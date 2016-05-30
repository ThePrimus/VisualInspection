#include <iostream>
#include "opencv2/opencv.hpp"
using namespace cv;

int main() {
	//short camera test in opencv
	Mat img;


	VideoCapture cap;
	cap.open(0);

	namedWindow("window", 1);
	while (1) {
		cap >> img;
		imshow("window", img);
		waitKey(33);
	}
}