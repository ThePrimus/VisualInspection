#pragma once
#include <iostream>
#include <fstream>
#include <Windows.h>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "damage_detection.h"
#include "detect_quad.h"
#include "CircleDetection.h"



#include <uEye.h>
#include <uEye_tools.h>
#include <ueye_deprecated.h>
#include <wchar.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;
using namespace cv;

char* window_name = "Visual Inspection";

bool IS_CALIBRATED = false;
double PX2CM = 0.00664624;

const bool test_from_filepath = false;
const bool test_test_routine = false;
//const string filepath = "Images/Neue Beleuchtung/Perfekt3.png"; 
const string filepath = "Images/Neue Beleuchtung/Perfekt2.png";
//const string filepath = "Images/Neue Beleuchtung/circles_test.png";
const string calibration_image = "Images/Neue Beleuchtung/Kalibrierung1.png";
const double calibration_value = 2.0;
const string IMAGE_FOLDER = "./Images/Neue Beleuchtung/resized";


void test_image(Mat img, bool show = true);
double calibrate_px2cm(Mat in, double calibration_length = 1.0);

vector<string> get_all_files_names_within_folder(string folder);
void test_routine(string image_folder, string output_path = "tests.txt");
void callBackFunc(int event, int x, int y, int flags, void* userdata);

//GUI stuff
Mat3b gui;
Rect calibrate_button;
Rect test_button;
Rect video;
Rect exit_button;
Rect visual_result;

Size size_image;
Mat image_to_test;
Mat resized_image;

Mat test_button_image;
Mat test_button_inactive_image;
Mat calibrate_button_image;
Mat exit_button_image;
Mat new_image_button_image;
Mat x_symbol;
Mat check_mark;

const string test_button_path = "GUI/test_button.png";
const string test_button_inactive_path = "GUI/test_button_inactive.png";
const string calibrate_button_path = "GUI/calibrate_button.png";
const string exit_button_path = "GUI/exit_button.png";
const string new_image_button_path = "GUI/new_image_button.png";
const string x_symbol_path = "GUI/x_symbol.jpg";
const string check_mark_path = "GUI/haekchen_symbol.png";

bool exit_program = false;
bool test_finished = false;
bool overall_result = false;



int main(int argc, char* argv[]) {

	//GUI
	gui = Mat3b(600, 1005, Vec3b(255, 255, 255));
	calibrate_button = Rect(10, 10, 200, 40);
	test_button = Rect(10, 55, 200, 40);
	exit_button = Rect(10, 550, 200, 40);
	video = Rect(220, 10, 772, 579);
	visual_result = Rect(10, 345, 200, 200);

	size_image = Size(772, 579);

	test_button_image = imread(test_button_path);
	test_button_inactive_image = imread(test_button_inactive_path);
	calibrate_button_image = imread(calibrate_button_path);
	new_image_button_image = imread(new_image_button_path);
	x_symbol = imread(x_symbol_path);
	check_mark = imread(check_mark_path);

	exit_button_image = imread(exit_button_path);

	test_button_inactive_image.copyTo(gui(test_button));
	calibrate_button_image.copyTo(gui(calibrate_button));
	exit_button_image.copyTo(gui(exit_button));
	gui(visual_result) = Vec3b(250, 250, 250);

	namedWindow(window_name, WINDOW_FULLSCREEN);
	resizeWindow(window_name, 1024, 765);
	setMouseCallback(window_name, callBackFunc);
	//


	if (test_test_routine) {
		test_routine(IMAGE_FOLDER);
		return 0;
	}



	HIDS hCam = 1; // 0
	char* imgMem;
	int memId;
	int img_width = 2048, img_height = 1536, img_bpp = 24;

	if (!test_from_filepath && (is_InitCamera(&hCam, NULL) != IS_SUCCESS)) {
		return 0;
	}
	if (!test_from_filepath)
	{
		is_AllocImageMem(hCam, img_width, img_height, img_bpp, &imgMem, &memId);
		is_SetImageMem(hCam, imgMem, memId);
		is_SetDisplayMode(hCam, IS_SET_DM_DIB);
		is_SetColorMode(hCam, IS_CM_RGB8_PACKED);
		is_SetImageSize(hCam, img_width, img_height);
		double enable = 1;
		double disable = 0;
		is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_GAIN, &disable, 0);
		is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_WHITEBALANCE, &disable, 0);
		is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_FRAMERATE, &disable, 0);
		is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_SHUTTER, &disable, 0);
		is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_SENSOR_GAIN, &disable, 0);
		is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_SENSOR_WHITEBALANCE, &disable, 0);
		is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_SENSOR_SHUTTER, &disable, 0);
		is_SetGamma(hCam, 150);
	}
	Mat source_image = imread(filepath);


	while (!exit_program) {
		if (test_from_filepath) {

			image_to_test = imread(filepath);
			cv::cvtColor(image_to_test, image_to_test, CV_BGR2GRAY);
			if (image_to_test.data == NULL) {
				cout << "Error: can't open file <" << filepath << ">" << endl;
				break;
			}

		}
		else if (is_FreezeVideo(hCam, IS_WAIT) == IS_SUCCESS) {
			void *pMemVoid; //pointer to where the image is stored
			is_GetImageMem(hCam, &pMemVoid);
			cv::Mat img(img_height, img_width, CV_8UC3, pMemVoid);
			cv::cvtColor(img, image_to_test, CV_RGB2GRAY);
		}



		if (!test_finished) {
			resize(image_to_test, resized_image, size_image);
			cvtColor(resized_image, resized_image, CV_GRAY2BGR);
			resized_image.copyTo(gui(video));
			gui(visual_result) = Vec3b(250, 250, 250);
		}
		else {
			if (overall_result) {
				check_mark.copyTo(gui(visual_result));
			}
			else {
				x_symbol.copyTo(gui(visual_result));
			}
		}


		if (IS_CALIBRATED) {
			test_button_image.copyTo(gui(test_button));
			if (test_finished) {
				new_image_button_image.copyTo(gui(test_button));
			}
			else {
				test_button_image.copyTo(gui(test_button));
			}
		}
		else {
			test_button_inactive_image.copyTo(gui(test_button));
		}

		imshow(window_name, gui);

		waitKey(10);
	}

	return 0;
}

void test_image(Mat img, bool show) {
	RotatedRect rect;
	CircleDetection cd;
	std::vector<Point> contour;

	bool result_quad_detect = false;
	bool result_circle_detection = false;
	bool result_damage_detection = false;

	bool check_everything = false;

	//quad detection
	Mat workpiece = img;
	Mat output_damage;
	result_quad_detect = detect_quad(workpiece, 0.2, 2, 50, 8, &rect, &contour);

	if (result_quad_detect || check_everything) {


		//circle detection
		double conversion = 0.1 / PX2CM; // 1mm in pixel
		cd.setPixelConversion(conversion);
		cd.setRotatedRect(rect);
		cd.setImage(workpiece);
		cd.calculateExpectedCirclePositions();
		cd.findCircles();
		cd.checkCircles();
		result_circle_detection = cd.isModelCorrect();


		std::vector<cv::Vec3f> circles = cd.getCircles();

		if (result_circle_detection || check_everything) {
			//damage detection
			result_damage_detection = !detect_damage(&workpiece, &output_damage, rect, circles, 80, 30, 30, 25);
		}
	}

	if (show) {
		Mat resized_error_output;
		if (!result_quad_detect) {
			Mat color_workpiece;
			cv::cvtColor(workpiece, color_workpiece, CV_GRAY2RGB);
			draw_quad_info(color_workpiece, &rect, Scalar(0, 0, 255), NULL, Scalar(0, 255, 0));
			resize(color_workpiece, resized_error_output, size_image);
			resized_error_output.copyTo(gui(video));

		}
		else if (!result_circle_detection) {
			Mat circle_error = cd.drawErrors();
			resize(circle_error, resized_error_output, size_image);
			resized_error_output.copyTo(gui(video));
		}
		else if (!result_damage_detection) {
			resize(output_damage, resized_error_output, size_image);
			resized_error_output.copyTo(gui(video));
		}
		else {
			overall_result = true;
		}
		imshow(window_name, gui);
	}

}

double calibrate_px2cm(Mat img, double calibration_length) {
	using Pvec = vector<Point>;
	Mat canny_output;
	vector<Pvec> contours;
	vector<Vec4i> hierarchy;

	canny_output = image_preprocessing(img);
	/// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	std::sort(contours.begin(), contours.end(), [](const Pvec& a, const Pvec& b) { return minAreaRect(a).size.area() > minAreaRect(b).size.area(); });

	if (contours.empty()) {
		//cout << "Error: no contours found during calibration process!" << endl;
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

	return px2cm;
}

vector<string> get_all_files_names_within_folder(string folder)
{
	vector<string> names;
	char search_path[200];
	sprintf_s(search_path, "%s/*.*", folder.c_str());
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				string str = fd.cFileName;
				string file_type = str.substr(str.find_last_of('.') + 1);
				if (file_type == "png" && str.find("Kalibrierung") == string::npos)
					names.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}

void test_routine(string image_folder, string output_path) {
	auto file_names = get_all_files_names_within_folder(image_folder);

	std::ofstream out(output_path);
	std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
	std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
	cout << "START TEST ROUTINE OF FOLDER: " << image_folder << endl;
	cout << "------------------------------" << endl;
	// calibrate
	Mat image_to_test = imread(calibration_image);
	PX2CM = calibrate_px2cm(image_to_test, calibration_value);
	IS_CALIBRATED = true;
	try {
		for (auto file : file_names) {
			string file_path = image_folder + "/" + file;
			cout << "----------" << endl;
			cout << file_path << endl;
			test_image(imread(file_path), false);
		}
	}
	catch (cv::Exception e) {
		cout << "PROGRAMM UNEXPECTEDLY TERMINATED!!" << endl;
		cout << e.what() << endl;
		cout << "==================================" << endl;
	}
	cout << "TEST ROUTINE DONE, RESULT STORED IN: " << output_path << endl;
	std::cout.rdbuf(coutbuf); //redirect std::cout to origin
}

void callBackFunc(int event, int x, int y, int flags, void* userdata) {
	if (event == EVENT_LBUTTONDOWN) {
		//test image
		if (test_button.contains(Point(x, y)) && IS_CALIBRATED) {
			if (test_finished) {
				test_finished = false;
				overall_result = false;
			}
			else {
				test_image(image_to_test);
				test_finished = true;
			}

		}

		//calibrate
		else if (calibrate_button.contains(Point(x, y))) {
			if (test_from_filepath)
				image_to_test = imread(calibration_image);

			PX2CM = calibrate_px2cm(image_to_test, 2.0);
			if (PX2CM > 0) {
				//std::cout << "Calibrated with px2cm = " << PX2CM << std::endl;
				IS_CALIBRATED = true;
			}
			else {
				IS_CALIBRATED = false;
			}
		}


		//exit
		else if (exit_button.contains(Point(x, y))) {
			exit_program = true;
		}
	}
	if (event == EVENT_LBUTTONUP) {
	}

	imshow(window_name, gui);
}