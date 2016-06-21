#pragma once
#include <iostream>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "damage_detection.h"
#include "detect_quad.h"
#include "CircleDetection.h"

using namespace std;
using namespace cv;



using namespace std;

int main(int argc, char* argv[]) {

	Mat workpiece;
	char* window_name = "Damage Detection";
	bool error = false;
	RotatedRect rect;
	CircleDetection cd;

	//quad detection
	loadImage(&workpiece);
	detect_quad(workpiece, 0.0125, 0.2, 2, 50, 8, &rect);

	//circle detection
	loadImage(&workpiece);
	cd.setRotatedRect(rect);
	cd.setImage(workpiece);
	cd.findCircles();

	std::vector<cv::Vec3f> circles = cd.getCircles();
	cv::Mat imgWithDrawnCircles = cd.drawCircles();

	namedWindow("Circles", WINDOW_NORMAL);
	imshow("Circles", imgWithDrawnCircles);

	//damage detection
	loadImage(&workpiece);
	error = detect_damage(&workpiece, 30, &rect, circles);

	namedWindow(window_name, WINDOW_NORMAL);
	imshow(window_name, workpiece);


	waitKey(0);
	return 0;
	
	return 0;
}