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


	int distance = 25;

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

	for (int i = 0; i < labels_count; i++) {
		if (contours[i].size() >= min_size) {
			Rect boundedRect = boundingRect(contours[i]);
			//cout << "width: " << boundedRect.width << " height: " << boundedRect.height << endl;
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


bool detect_damage(Mat* image, RotatedRect rect, vector<Vec3f> circles, int threshold, int min_cluster_size, int rectangle_line_size, int radius_extension) {
	Mat workpiece_filter, workpiece_canny, workpiece_dilate, output_image;

	GaussianBlur(*image, workpiece_filter, Size(5,5), 10);
	guided_filter(workpiece_filter, &workpiece_filter);

	//namedWindow("filter", WINDOW_NORMAL);
	//imshow("filter", workpiece_filter);

	canny_detection(&workpiece_filter, &workpiece_canny, threshold);
	//namedWindow("canny", WINDOW_NORMAL);
	//imshow("canny", workpiece_canny);

	remove_edges(workpiece_canny, &workpiece_canny, rect, rectangle_line_size);

	remove_circles(workpiece_canny, &workpiece_canny, circles, radius_extension);

	//namedWindow("removed", WINDOW_NORMAL);
	//imshow("removed", workpiece_canny);

	int errors = clusters(&workpiece_canny, image, min_cluster_size);

	if (errors > 0)
		return true;
	else
		return false;

}

