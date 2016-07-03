#pragma once

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <array>

using namespace cv;
using namespace std;

extern double PX2CM;

auto rotatedrect_area(RotatedRect rect) {
	return rect.size.area();
}

bool check_quad_size(const RotatedRect& rect, const double px2cm, const double thresh_accuracy, const double side_length) {
	Point2f rect_points[4];
	rect.points(rect_points);

	for (int i = 0; i < 4; i++) {
		auto diff_p = rect_points[i] - rect_points[(i + 1) % 4];
		auto length_px = sqrt(diff_p.x * diff_p.x + diff_p.y * diff_p.y);
		auto length_cm = length_px * px2cm;

		if (abs(length_cm - side_length) < thresh_accuracy) {
			return false;
		}
	}
	return true;
}

bool detect_quad(Mat img, double px2cm, double thresh_accuracy, double alpha, double beta, double side_length, RotatedRect* out_rect = NULL, vector<Point>* out_cont = NULL) {
	using Pvec = vector<Point>;
	Mat canny_output;
	vector<Pvec> contours;
	vector<Vec4i> hierarchy;

	img.convertTo(img, -1, alpha, beta);
	/// Detect edges using canny
	auto tresh = 100;
	Canny(img, canny_output, tresh, tresh * 2, 3);
	/// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	std::sort(contours.begin(), contours.end(), [](const Pvec& a, const Pvec& b) { return minAreaRect(a).size.area() > minAreaRect(b).size.area(); });

	if (contours.size() > 0) {
		auto cont = contours[0];
		auto rect = minAreaRect(cont);

		if (check_quad_size(rect, PX2CM, thresh_accuracy, side_length)) {
			if (out_rect)
				*out_rect = minAreaRect(cont);
			if (out_cont)
				*out_cont = cont;

			return true;
		}
	}
	else {
		return false;
	}
}

void draw_rotated_rect(Mat out, const RotatedRect& rect, Scalar color) {
	Point2f points[4];
	rect.points(points);
	for (int i = 0; i < 4; i++) {
		line(out, points[i], points[(i + 1) % 4], color, 2);
	}
}

void draw_contour(Mat out, const vector<Point>& contour, Scalar color) {
	vector<vector<Point> > contour_list;
	contour_list.push_back(contour);
	drawContours(out, contour_list, 0, color);
}

void draw_quad_info(Mat out, RotatedRect* rect, Scalar rect_color, vector<Point>* cont, Scalar cont_color) {
	if (rect)
		draw_rotated_rect(out, *rect, rect_color);
	if (cont)
		draw_contour(out, *cont, cont_color);

}