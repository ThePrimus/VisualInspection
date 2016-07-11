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

bool IS_CALIBRATED = true;
double PX2CM = 0.00664624;

const bool test_from_filepath = false;
const bool test_test_routine = false;
const string filepath = "Images/Neue Beleuchtung/KaputterSteg2.png"; 
//const string filepath = "Images/Neue Beleuchtung/Perfekt2.png";
//const string filepath = "Images/Neue Beleuchtung/circles_test.png";
//const string filepath = "Images/Neue Beleuchtung/KaputteEcke3.png";
const string calibration_image = "Images/Neue Beleuchtung/Kalibrierung1.png"; 
const double calibration_value = 2.0;
const string IMAGE_FOLDER = "./Images/Neue Beleuchtung/resized";


void test_image(Mat img, bool show = true);
double calibrate_px2cm(Mat in, double calibration_length = 1.0);

vector<string> get_all_files_names_within_folder(string folder);
void test_routine(string image_folder, string output_path = "tests.txt");




int main(int argc, char* argv[]) {

	if (test_test_routine) {
		test_routine(IMAGE_FOLDER);
		return 0;
	}

	namedWindow(window_name, WINDOW_NORMAL);
	resizeWindow(window_name, 1024, 765);

	HIDS hCam = 1; // 0
	char* imgMem;
	int memId;
	int img_width = 2048, img_height = 1536, img_bpp = 24, img_step, img_data_size;

	if (!test_from_filepath && (is_InitCamera(&hCam, NULL) != IS_SUCCESS)) {
		return 0;
	}
	if(!test_from_filepath)
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

	cout << "Controls: <c> to calibrate the system" << endl << "<t> to test the image displayed in the window" << endl << "<e> to exit" << endl;
	cout << "After testing / calibration is done, press any key to read the next image (only works for webcam build)" << endl;

	while (true) {
		Mat image_to_test;
		if (test_from_filepath) {

			image_to_test = imread(filepath);
			cv::cvtColor(image_to_test, image_to_test, CV_BGR2GRAY);
			if (image_to_test.data == NULL) {
				cout << "Error: can't open file <" << filepath << ">" << endl;
				break;
			}

		} else if (is_FreezeVideo(hCam, IS_WAIT) == IS_SUCCESS) {
			void *pMemVoid; //pointer to where the image is stored
			is_GetImageMem(hCam, &pMemVoid);
			cv::Mat img(img_height, img_width, CV_8UC3, pMemVoid);
			cv::cvtColor(img, image_to_test, CV_RGB2GRAY);
		}

		if (image_to_test.rows > 0) {
			imshow(window_name, image_to_test);
		}
		int key = waitKey(30);

		if (key == 'e')
			break;

		if (key == 's')
			continue;

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

void test_image(Mat img, bool show) {
	RotatedRect rect;
	CircleDetection cd;
	std::vector<Point> contour;

	bool result_quad_detect = false;
	bool result_circle_detection = false;
	bool result_damage_detection = false;

	bool check_everything = true;

	cout << "==========" << endl;
	//quad detection
	Mat workpiece = img;
	result_quad_detect = detect_quad(workpiece, 0.25, 2, 50, 8, &rect, &contour);
	cout << "Quad detection: " << (result_quad_detect ? "OK" : "FAILED") << endl;

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
		/*
		Mat circles1 = cd.drawCircles();
		namedWindow("circles", WINDOW_NORMAL);
		imshow("circles", circles1);
		*/

		std::vector<cv::Vec3f> circles = cd.getCircles();
		cout << "Circle detection: " << (result_circle_detection ? "OK" : "FAILED") << endl;

		if (result_circle_detection || check_everything) {
			//damage detection
			bool result_damage_detection = !detect_damage(&workpiece, rect, circles, 50, 30, 25, 15);
			cout << "Damage detection: " << (result_damage_detection ? "OK" : "FAILED") << endl;
		}
	}

	if (show) {
		if (result_quad_detect) {
			if (!result_circle_detection) {
				Mat circle_error = cd.drawErrors();
				imshow(window_name, circle_error);
				waitKey(0);
			}
			//else { // true!
				if (!result_damage_detection) {
					namedWindow("damage", WINDOW_NORMAL);
					imshow("damage", workpiece);
					waitKey(0);
				}
			//}
		} else {
			draw_quad_info(workpiece, &rect, Scalar(255, 0, 0), &contour, Scalar(0, 255, 0));
			imshow(window_name, workpiece);
			waitKey(0);
		}
	}

	bool result_overall = result_quad_detect && result_circle_detection && result_damage_detection;
	cout << "Overall result: " << (result_overall ? "OK" : "FAILED") << endl;
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