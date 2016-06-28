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

int main(int argc, char* argv[]) {

	Mat workpiece;
	bool error = false;
	RotatedRect rect;
	CircleDetection cd;

	//quad detection
	cout << "Quad detection" << endl;
	loadImage(&workpiece);
	detect_quad(workpiece, 0.0125, 0.2, 2, 50, 8, &rect);
	draw_quad_info(workpiece, &rect, Scalar(0, 255, 0), NULL, Scalar(0, 0, 0));

	namedWindow(window_name, WINDOW_NORMAL);
	imshow(window_name, workpiece);
	waitKey(0);

	//circle detection
	cout << "Circle detection" << endl;
	loadImage(&workpiece);
	cd.setRotatedRect(rect);
	cd.setImage(workpiece);
	cd.findCircles();

	std::vector<cv::Vec3f> circles = cd.getCircles();
	cv::Mat imgWithDrawnCircles = cd.drawCircles();

	namedWindow(window_name, WINDOW_NORMAL);
	imshow(window_name, imgWithDrawnCircles);
	waitKey(0);

	//damage detection
	cout << "Damage detection" << endl;
	loadImage(&workpiece);
	error = detect_damage(&workpiece, &rect, circles, 25, 8, 2, 3);

	namedWindow(window_name, WINDOW_NORMAL);
	imshow(window_name, workpiece);
	waitKey(0);
	
	return 0;
}