#pragma once

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "guidedfilter.h"
#include "CircleDetection.h"
#include "detect_quad.h"


using namespace cv;
using namespace std;

void bilateral_filter(Mat in, Mat* out) {
	int MAX_KERNEL_LENGTH = 31;
	for (int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2)
	{
		bilateralFilter(in, *out, i, i * 2, i / 2);
	}
}

void guided_filter(Mat in, Mat* out) {
	*out = guidedFilter(in, in, 8, 0.11 * 0.11 * 255 * 255);
}

void canny_detection(Mat* in, Mat* out, int threshold, bool dilate_output = false) {
	Canny(*in, *out, threshold, 3 * threshold, 3);

	if (dilate_output) {
		int dilate_size = 1;
		Mat element = getStructuringElement(MORPH_RECT,
			Size(2 * dilate_size + 1, 2 * dilate_size + 1),
			Point(dilate_size, dilate_size));

		dilate(*out, *out, element);
	}

}

void remove_edges(Mat in, Mat* out, RotatedRect rect, int rectangle_line_size) {
	Mat drawn_rect(in.rows, in.cols, CV_8UC1, Scalar(0, 0, 0));

	Point2f points[4];
	rect.points(points);
	for (int i = 0; i < 4; i++) {
		line(drawn_rect, points[i], points[(i + 1) % 4], Scalar(255,255,255), rectangle_line_size);
	}

	subtract(in, drawn_rect, *out);
}

void remove_circles(Mat in, Mat* out, vector<Vec3f> circles, int radius_extension) {
	Mat drawn_circles(in.rows, in.cols, CV_8UC1, Scalar(0, 0, 0));

	// Draw the circles detected
	for (int i = 0; i < circles.size(); i++)
	{
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]) + radius_extension;
		// circle outline
		circle(drawn_circles, center, radius, Scalar(255, 255, 255), -1);
	}

	subtract(in, drawn_circles, *out);

}

int clusters(Mat* in, Mat* out, int min_size) {
	int cluster_count = 0;
	vector<Point> pts;
	findNonZero(*in, pts);

	if (pts.size() == 0) {
		return 0;
	}

	int distance = 40;

	vector<int> labels;

	int distance_2 = distance * distance;
	int labels_count = partition(pts, labels, [distance_2](const Point& lhs, const Point& rhs) {
		return ((lhs.x - rhs.x)*(lhs.x - rhs.x) + (lhs.y - rhs.y)*(lhs.y - rhs.y)) < distance_2;
	});

	vector<vector<Point>> contours(labels_count);
	for (int i = 0; i < pts.size(); ++i)
	{
		contours[labels[i]].push_back(pts[i]);
	}

	Mat cluster;

	for (int i = 0; i < labels_count; i++) {
		Rect boundedRect = boundingRect(contours[i]);
		Point2f center = Point2f(boundedRect.x + boundedRect.width / 2, boundedRect.y + boundedRect.height / 2);
		

		getRectSubPix(*in, boundedRect.size(), center, cluster);
		threshold(cluster, cluster, 2, 255, THRESH_BINARY);
		int whitePixel = (int)sum(cluster)[0] / 255;

		//cout << "cluster " << i << " whitePixel: " << whitePixel << endl;
		if (whitePixel >= min_size) {	
			boundedRect.x -= 10;
			boundedRect.y -= 10;
			boundedRect.width += 20;
			boundedRect.height += 20;
			rectangle(*out, boundedRect, Vec3b(0, 255, 0), 2);
			cluster_count++;
		}
			
	}

	return cluster_count;
}

bool broken_bridge(Mat* image, RotatedRect rect) {
	RotatedRect bridgeArea = RotatedRect(rect.center, Point2f(15, 15), rect.angle);

	// matrices we'll use
	Mat M, rotated, bridge, bridgeThreshold;
	// get angle and size from the bounding box
	float angle = bridgeArea.angle;
	Size rect_size = bridgeArea.size;
	if (bridgeArea.angle < -45.) {
		angle += 90.0;
		swap(rect_size.width, rect_size.height);
	}
	// get the rotation matrix
	M = getRotationMatrix2D(bridgeArea.center, angle, 1.0);
	// perform the affine transformation
	warpAffine(*image, rotated, M, image->size(), INTER_CUBIC);
	// crop the resulting image
	getRectSubPix(rotated, rect_size, bridgeArea.center, bridge);
	
	threshold(bridge, bridgeThreshold, 50, 255, THRESH_BINARY);

	//namedWindow("bridge", WINDOW_NORMAL);
	//imshow("bridge", bridgeThreshold);


	int whitePixel = (int)sum(bridgeThreshold)[0] / 255;
	int blackPixel = bridgeThreshold.rows * bridgeThreshold.cols - whitePixel;

	//cout << "pixel: " << whitePixel << " " << blackPixel << endl;
	
	if (whitePixel < blackPixel) {
		cout << "steg kaputt!" << endl;
		//draw error rect
		Rect errorRect = Rect(Point2f(rect.center.x - 20, rect.center.y - 20), Size2f(40,40));
		rectangle(*image, errorRect, Scalar(0, 255, 0), 2);
		return true;
	}
	else
		return false;
}

bool detect_damage(Mat* image, RotatedRect rect, vector<Vec3f> circles, int threshold, int min_cluster_size, int rectangle_line_size, int radius_extension) {
	Mat workpiece, workpiece_filter, workpiece_canny, workpiece_dilate, output_image;
	bool error = false;

	image->convertTo(workpiece_filter, -1, 1.5, 0);
	image->convertTo(workpiece, -1, 1.5, 0);

	//namedWindow("brightness", WINDOW_NORMAL);
	//imshow("brightness", workpiece);

	error = broken_bridge(&workpiece, rect);

	//GaussianBlur(workpiece_filter, workpiece_filter, Size(7,7), 10);
	guided_filter(workpiece_filter, &workpiece_filter);


	namedWindow("filter", WINDOW_NORMAL);
	imshow("filter", workpiece_filter);

	/*
	//draw rectangle (testing)
	Mat rectangle;
	image->copyTo(rectangle);

	Point2f points[4];
	rect.points(points);
	for (int i = 0; i < 4; i++) {
		line(rectangle, points[i], points[(i + 1) % 4], Scalar(255, 255, 255), 2);
	}
	
	namedWindow("rectangle", WINDOW_NORMAL);
	imshow("rectangle", rectangle);*/

	

	canny_detection(&workpiece_filter, &workpiece_canny, threshold);
	namedWindow("canny", WINDOW_NORMAL);
	imshow("canny", workpiece_canny);

	remove_edges(workpiece_canny, &workpiece_canny, rect, rectangle_line_size);

	remove_circles(workpiece_canny, &workpiece_canny, circles, radius_extension);

	namedWindow("removed", WINDOW_NORMAL);
	imshow("removed", workpiece_canny);

	int errors = clusters(&workpiece_canny, image, min_cluster_size);

	if (errors > 0)
		error = true;
	
	return error;

}



