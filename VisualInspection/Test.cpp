#pragma once
#include <iostream>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "damage_detection.h"
#include "detect_quad.h"
#include "CircleDetection.h"

using namespace std;
using namespace cv;

char* window_name = "Visual Inspection";

bool IS_CALIBRATED = true;
double PX2CM = 0.0125;

void test_image(Mat img);
double calibrate_px2cm(Mat in, double calibration_length = 1.0);

int main(int argc, char* argv[]) {

	namedWindow(window_name, WINDOW_NORMAL);

	bool test_from_filepath = true;
	//string filepath = "Renderings/Werkst�ck kaputter Steg.png";
	//string filepath = "Renderings/Calibration-1.png";
	string filepath = "Images/Neue Beleuchtung/resized/Perfekt2.png";
	string calibration_image = "Images/Neue Beleuchtung/resized/Kalibrierung1.png";
	Mat source_image = imread(filepath);


	int CAMERA_INDEX = 0;
	VideoCapture cap(CAMERA_INDEX);

	cout << "Controls: <c> to calibrate the system" << endl << "<t> to test the image displayed in the window" << endl << "<e> to exit" << endl;
	cout << "After testing / calibration is done, press any key to read the next image (only works for webcam build)" << endl;

	while (cap.isOpened() || test_from_filepath) {
		Mat image_to_test;
		if (test_from_filepath) {

			image_to_test = imread(filepath);
			if (image_to_test.data == NULL) {
				cout << "Error: can't open file <" << filepath << ">" << endl;
				break;
			}

		}
		else if (cap.isOpened()) {
			cap >> image_to_test;
		}
		else {
			cout << "Error: webcam unavailable!" << endl;
			break;
		}

		imshow(window_name, image_to_test);
		int key = waitKey(0);

		if (key == 'e') {
			break;
		}

		if (key == 'c') {
			// set calibration image
			if (test_from_filepath)
				image_to_test = imread(calibration_image);

			PX2CM = calibrate_px2cm(image_to_test, 2.0);
			if (PX2CM > 0) {
				cout << "==========" << endl;
				cout << "System calibrated with value: " << PX2CM << endl;
				IS_CALIBRATED = true;
			}
			else {
				cout << "==========" << endl;
				cout << "Warning: calibration failed!" << endl;
			}
		}
		if (key == 't') {
			if (!IS_CALIBRATED) {
				cout << "==========" << endl;
				cout << "Warning: System not calibrated!" << endl;
				continue;
			}
			test_image(image_to_test);
		}

	}

	cout << "==========" << endl;
	cout << "PRESS ANY KEY IN CONSOLE TO SHUTDOWN THE PROGRAM!" << endl;
	cin.get();

	return 0;
}

void test_image(Mat img) {
	RotatedRect rect;
	CircleDetection cd;
	std::vector<Point> contour;

	cout << "==========" << endl;
	//quad detection
	Mat workpiece = img;
	bool result_quad_detect = detect_quad(workpiece, 0.25, 2, 50, 8, &rect, &contour);
	cout << "Quad detection: " << (result_quad_detect ? "OK" : "FAILED") << endl;

	//circle detection
	bool result_circle_detection = true;
	cd.setRotatedRect(rect);
	cd.setImage(workpiece);
	cd.findCircles();

	std::vector<cv::Vec3f> circles = cd.getCircles();
	//cv::Mat imgWithDrawnCircles = cd.drawCircles();
	cout << "Circle detection: " << (result_circle_detection ? "OK" : "FAILED") << endl;

	//damage detection
	bool result_damage_detection = !detect_damage(&workpiece, &rect, circles, 25, 8, 2, 3);
	cout << "Damage detection: " << (result_damage_detection ? "OK" : "FAILED") << endl;

	bool result_overall = result_quad_detect && result_circle_detection && result_damage_detection;
	cout << "Overall result: " << (result_overall ? "OK" : "FAILED") << endl;

	imshow(window_name, workpiece);
	waitKey(0);
}

double calibrate_px2cm(Mat img, double calibration_length) {
	using Pvec = vector<Point>;
	Mat canny_output;
	vector<Pvec> contours;
	vector<Vec4i> hierarchy;

	//img.convertTo(img, -1, 2.0, beta);
	/// Detect edges using canny
	auto tresh = 100;
	Canny(img, canny_output, tresh, tresh * 2, 3);
	/// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	std::sort(contours.begin(), contours.end(), [](const Pvec& a, const Pvec& b) { return minAreaRect(a).size.area() > minAreaRect(b).size.area(); });

	if (contours.empty()) {
		cout << "Error: no contours found during calibration process!" << endl;
		return -1.0;
	}

	Point2f rect_points[4];
	minAreaRect(contours[0]).points(rect_points);

	double max_line = 0;
	for (int i = 0; i < 4; i++) {
		Point2f line_vec = rect_points[i] - rect_points[(i + 1) % 4];
		double length = sqrt(line_vec.x *line_vec.x + line_vec.y * line_vec.y);

		max_line = max(length, max_line);
	}

	double px2cm = calibration_length / max_line;

	cout << "==========" << endl;
	cout << "Calibration result: " << px2cm << endl;
	return px2cm;
}