#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <map>
#include <functional>

using namespace cv;
using namespace std;

bool CALIBRATED = false;
double CALIBRATION_FLOAT = -1.0f;
bool source_loaded = false;

string read_filename();
bool read_image_file(string path, Mat& result);
void find_edges(Mat in, Mat& out);
void find_circles(Mat in, Mat& out);
void calibrate(Mat in, Mat& out);
void corner_harris(Mat in, Mat& out);
void check_sides(Mat in, Mat& out);

map<char, pair<const char*, function<void(Mat, Mat&)> > > COMMANDS = {
	{ 'i',{ "Calibrate the system", calibrate } },
	{ 'c',{ "Detect circles", find_circles } },
	{ 'e',{ "Detect edges", find_edges } },
	{ 'h',{ "Corner harris", corner_harris } },
	{ 'p',{ "Check sides", check_sides } }
};

const char* WINDOW_NAME = "Visual-Inspection";



int main() {
	bool run = true;
	Mat source_image;
	Mat result_image;
	do {
		cout << ">> ";
		char input = 0;
		cin >> input;

		auto command = COMMANDS.find(input);
		if (command != COMMANDS.end()) {
			if (!source_loaded) {
				if (read_image_file(read_filename(), source_image)) {
					source_loaded = true;
					result_image = source_image;
				}
				else {
					run = false;
					continue;
				}
			}
			result_image = source_image;

			// <key, pair<description, command_func>>
			auto command_func = command->second.second;


			threshold(source_image, result_image, 45, 255, THRESH_BINARY);
			//GaussianBlur(result_image, result_image, Size(5, 5), 0, 0);
			//addWeighted(result_image, 1.5, result_image, -0.5, 0, result_image);

			//command_func(result_image, result_image);

			imshow(WINDOW_NAME, result_image);
			waitKey();
		}
		else {
			run = false;
		}
	} while (run);

	return 0;
}




string read_filename() {
	cout << "Enter path to file\n";
	cout << ">> ";
	string filepath = "";
	cin >> filepath;
	return filepath;
}

bool read_image_file(string path, Mat& result) {
	if (path.empty())
	{
		cout << "no image_name provided" << endl;
		return false;
	}
	result = imread(path, 0);
	if (result.empty())
	{
		cout << "can not open " << path << endl;
		return false;
	}

	cout << "Loaded " << path << " with " << result.channels() << " channels." << endl << endl;

	return true;
}

bool is_contour_rectangle(const vector<Point>& contour, float min_treshhold = 2.5f) {
	//double peri = arcLength(contour, true);
	//vector<Point> approx;
	//approxPolyDP(contour, approx, 0.02 * peri, true);

	if (contour.size() == 4) {

		float max = 0.0f;
		auto a = contour[0];
		for (int i = 2; i < 4; i++) {
			auto b = contour[i];

			auto p = a - b;
			float l = std::sqrt(p.x * p.x + p.y * p.y);

			if (l > min_treshhold)
				return true;
		}
	}

	return false;
}

void find_edges(Mat img, Mat& out)
{
	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	/// Detect edges using canny
	auto tresh = 100;
	Canny(img, canny_output, tresh, tresh * 2, 3);
	/// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	/// Draw contours
	Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	for (int i = 0; i< contours.size(); i++)
	{
		auto cont = contours[i];
		/*if (is_contour_rectangle(cont))*/ {
			Scalar color = Scalar(255, 0, 0);
			if (i == 1) color = Scalar(0, 255, 0);
			drawContours(drawing, contours, i, color, 1, 8, hierarchy, 0, Point());
			/*
			for (int u = 0; u < 4; u++) {
			circle(drawing, contours[i][u], 5, Scalar(0, 0, 255), 1, LINE_AA);
			}
			*/
		}
	}

	out = drawing;
}

void find_circles(Mat img, Mat& out) {
	Mat cimg;
	medianBlur(img, img, 5);
	cvtColor(img, cimg, COLOR_GRAY2BGR);
	vector<Vec3f> circles;
	HoughCircles(img, circles, HOUGH_GRADIENT, 1, 10,
		100, 30, 1, 30 // change the last two parameters
					   // (min_radius & max_radius) to detect larger circles
	);
	for (size_t i = 0; i < circles.size(); i++)
	{
		Vec3i c = circles[i];
		circle(cimg, Point(c[0], c[1]), c[2], Scalar(0, 0, 255), 1, LINE_AA);
		circle(cimg, Point(c[0], c[1]), 2, Scalar(0, 255, 0), 1, LINE_AA);
	}
	out = cimg;
}

void calibrate(Mat img, Mat& out) {
	// forward input
	out = img;

	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	/// Detect edges using canny
	auto tresh = 100;
	Canny(img, canny_output, tresh, tresh * 2, 3);
	/// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	/// Draw contours
	Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	for (auto cont : contours) {
		if (cont.size() == 4 && is_contour_rectangle(cont)) {
			for (int i = 2; i < 4; i++) {
				if (cont[0].x == cont[i].x) {
					CALIBRATION_FLOAT = abs(cont[0].y - cont[i].y);
					break;
				}
				else if (cont[0].y == cont[i].y) {
					CALIBRATION_FLOAT = abs(cont[0].x - cont[i].x);
					break;
				}
			}

			if (CALIBRATION_FLOAT > 0.0) {
				cout << "Calibrated: " << CALIBRATION_FLOAT << endl;
				CALIBRATED = true;
				// unload source image
				source_loaded = false;
			}
		}
	}

	if (!CALIBRATED)
		cout << "Calibration error!" << endl;
}

void corner_harris(Mat img, Mat& out)
{

	Mat dst, dst_norm, dst_norm_scaled;
	dst = Mat::zeros(img.size(), CV_32FC1);

	/// Detector parameters
	int blockSize = 2;
	int apertureSize = 3;
	double k = 0.04;

	/// Detecting corners
	cornerHarris(img, dst, blockSize, apertureSize, k, BORDER_DEFAULT);

	/// Normalizing
	normalize(dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
	convertScaleAbs(dst_norm, dst_norm_scaled);

	double min, max;
	minMaxLoc(dst_norm, &min, &max);

	threshold(dst_norm, dst_norm, 0.01f * max, 255.0f, THRESH_TRUNC);

	/*
	/// Drawing a circle around corners
	for (int j = 0; j < dst_norm.rows; j++)
	{
	for (int i = 0; i < dst_norm.cols; i++)
	{
	if ((int)dst_norm.at<float>(j, i) > 0.01 * max)
	{
	circle(dst_norm_scaled, Point(i, j), 5, Scalar(0), 2, 8, 0);
	}
	}
	}
	*/

	out = dst_norm_scaled;
}

void check_sides(Mat img, Mat& out) {
	// forward input
	out = img;

	if (CALIBRATED) {

		Mat canny_output;
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;

		/// Detect edges using canny
		auto tresh = 100;
		Canny(img, canny_output, tresh, tresh * 2, 3);
		/// Find contours
		findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		/// Draw contours
		Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
		for (auto cont : contours) {
			if (cont.size() == 4 && is_contour_rectangle(cont)) {
				for (int i = 0; i < 4; i++) {
					/*cout << "X-diff: " << cont[0].x - cont[i].x << endl;
					cout << "Y-diff: " << cont[0].y - cont[i].y << endl;
					if (cont[0].x == cont[i].x) {
					cout << "Edge with length: " << abs(cont[0].y - cont[i].y) << endl;
					}
					else if (cont[0].y == cont[i].y) {
					cout << "Edge with length: " << abs(cont[0].x - cont[i].x) << endl;
					}
					*/

					cout << "Point (" << cont[i].x << ", " << cont[i].y << ")" << endl;
				}
			}
		}
	}
	else {
		cout << "Not Calibrated, calibrate system first!" << endl;
	}
}