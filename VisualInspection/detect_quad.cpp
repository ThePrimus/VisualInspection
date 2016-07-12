#pragma once

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <array>
#include <iostream>

using namespace cv;
using namespace std;

extern double PX2CM;
extern char* window_name;

void draw_rotated_rect(Mat out, const RotatedRect& rect, Scalar color) {
	Point2f points[4];
	rect.points(points);
	for (int i = 0; i < 4; i++) {
		line(out, points[i], points[(i + 1) % 4], color, 3);
	}
}

void draw_contour(Mat out, const vector<Point>& contour, Scalar color) {
	vector<vector<Point> > contour_list;
	contour_list.push_back(contour);
	drawContours(out, contour_list, 0, color, 3);
}

void draw_quad_info(Mat out, RotatedRect* rect, Scalar rect_color, vector<Point>* cont, Scalar cont_color) {
	if (rect)
		draw_rotated_rect(out, *rect, rect_color);
	/*if (cont)
		draw_contour(out, *cont, cont_color);*/

}

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
		auto abs_length_diff = abs(length_cm - side_length);
		std::cout << "detect rect length difference: " << abs_length_diff << std::endl;
		if (abs_length_diff > thresh_accuracy) {
			return false;
		}
	}
	return true;
}

Mat image_preprocessing(Mat img) {
	Mat result;

	img.convertTo(result, -1, 1.5, 0);
	cv::threshold(result, result, 200, 0, THRESH_TOZERO_INV);
	//cv::namedWindow("test", WINDOW_NORMAL);
	//cv::imshow("test", result);
	//waitKey(0);

	cv::threshold(result, result, 70, 255, THRESH_BINARY);
	//cv::imshow(window_name, result);
	//waitKey(0);

	int erosion_size = 3;
	Mat element = getStructuringElement(MORPH_RECT,
		Size(2 * erosion_size + 1, 2 * erosion_size + 1),
		Point(erosion_size, erosion_size));

	/// Apply the erosion operation
	erode(result, result, element);
	//imshow(window_name, result);
	//waitKey(0);



	//blur(img, img, Size(3, 3));
	//cv::imshow("test", img);
	//waitKey(0);
	
	int dilation_size = 2;
	element = getStructuringElement(MORPH_RECT,
	Size(2 * dilation_size + 1, 2 * dilation_size + 1),
	Point(dilation_size, dilation_size));
	/// Apply the dilation operation
	dilate(result, result, element);
	//cv::imshow(window_name, result);
	//waitKey(0);
	

	/// Detect edges using canny
	auto tresh = 100;
	Mat canny_result;
	Canny(result, canny_result, tresh, tresh * 2, 3);
	
	//cv::imshow(window_name, result);
	//waitKey(0);

	return canny_result;
}

bool detect_quad(Mat in, double thresh_accuracy, double alpha, double beta, double side_length, RotatedRect* out_rect = NULL, vector<Point>* out_cont = NULL) {
	using Pvec = vector<Point>;
	Mat canny_output;
	vector<Pvec> contours;
	vector<Vec4i> hierarchy;

	Mat img = image_preprocessing(in);
	//cv::cvtColor(in, img, CV_BGR2GRAY);

	

	

	canny_output = img;

	/// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	std::sort(contours.begin(), contours.end(), [](const Pvec& a, const Pvec& b) { return minAreaRect(a).size.area() > minAreaRect(b).size.area(); });

	if (contours.size() > 0) {
		auto cont = contours[0];
		auto rect = minAreaRect(cont);

		if (out_rect)
			*out_rect = minAreaRect(cont);
		if (out_cont)
			*out_cont = cont;

		//draw_quad_info(in, &rect, Scalar(255, 0, 0), NULL, Scalar(255, 0, 0));
		//cv::imshow(window_name, in);
		//waitKey(0);

		if (check_quad_size(rect, PX2CM, thresh_accuracy, side_length)) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}