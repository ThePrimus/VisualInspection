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
			rectangle(*out, boundedRect, Scalar(0, 0, 255), 2);
			cluster_count++;
		}
			
	}

	return cluster_count;
}

bool broken_bridge(Mat* in, Mat* out, RotatedRect rect) {
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
	warpAffine(*in, rotated, M, in->size(), INTER_CUBIC);
	// crop the resulting image
	getRectSubPix(rotated, rect_size, bridgeArea.center, bridge);

	//namedWindow("bridge", WINDOW_NORMAL);
	//imshow("bridge", bridge);
	
	threshold(bridge, bridgeThreshold, 50, 255, THRESH_BINARY);




	int whitePixel = (int)sum(bridgeThreshold)[0] / 255;
	int blackPixel = bridgeThreshold.rows * bridgeThreshold.cols - whitePixel;

	//cout << "pixel: " << whitePixel << " " << blackPixel << endl;
	
	if (whitePixel < blackPixel) {
		cout << "steg kaputt!" << endl;
		//draw error rect
		Rect errorRect = Rect(Point2f(rect.center.x - 20, rect.center.y - 20), Size2f(40,40));
		rectangle(*out, errorRect, Scalar(0, 0, 255), 2);
		return true;
	}
	else
		return false;
}

bool detect_damage(Mat* in, Mat* out, RotatedRect rect, vector<Vec3f> circles, int threshold, int min_cluster_size, int rectangle_line_size, int radius_extension) {
	Mat workpiece, masked_workpiece, output_image, mask;
	bool error = false;
	cvtColor(*in, *out, CV_GRAY2RGB);
	in->convertTo(workpiece, -1, 1.5, 0);
	//in->convertTo(workpiece, -1, 1.5, 0);

	mask = Mat(workpiece.rows, workpiece.cols, workpiece.type(), Scalar(0));
	Point2f points[4];
	rect.points(points);
	vector<Point> vertices;
	for (int i = 0; i < 4; i++) {
		vertices.push_back(points[i]);
	}
	fillConvexPoly(mask, vertices, Scalar(255));
	workpiece.copyTo(masked_workpiece, mask);

	//namedWindow("mask", WINDOW_NORMAL);
	//imshow("mask", masked_workpiece);

	error = broken_bridge(&masked_workpiece, out, rect);

	//GaussianBlur(workpiece_filter, workpiece_filter, Size(7,7), 10);
	guided_filter(masked_workpiece, &masked_workpiece);

	//namedWindow("filter", WINDOW_NORMAL);
	//imshow("filter", masked_workpiece);

	cv::threshold(masked_workpiece, masked_workpiece, 65, 0, THRESH_TOZERO);





	canny_detection(&masked_workpiece, &masked_workpiece, threshold);
	//namedWindow("canny", WINDOW_NORMAL);
	//imshow("canny", masked_workpiece);

	remove_edges(masked_workpiece, &masked_workpiece, rect, rectangle_line_size);

	remove_circles(masked_workpiece, &masked_workpiece, circles, radius_extension);

	//namedWindow("removed", WINDOW_NORMAL);
	//imshow("removed", masked_workpiece);

	int errors = clusters(&masked_workpiece, out, min_cluster_size);
	//namedWindow("out", WINDOW_NORMAL);
	//imshow("out", output_image);
	//in = &output_image;

	if (errors > 0)
		error = true;
	
	return error;

}



