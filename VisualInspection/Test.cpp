#include "CircleDetection.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {

	CircleDetection cd;

	cv::RotatedRect rRect = cv::RotatedRect(cv::Point2f(512, 383), cv::Size2f(650, 648.003), 0);

	// Don't forget to change the path ;-)
	cv::Mat img = cv::imread("C:\\Users\\Patrick\\Documents\\visual studio 2015\\Projects\\VisualInspection\\VisualInspection\\Renderings\\Werkstück perfekt.png", CV_LOAD_IMAGE_UNCHANGED);
	cd.setRotatedRect(rRect);
	cd.setImage(img);
	cd.findCircles();

	std::vector<cv::Vec3f> circles = cd.getCircles();
	cv::Mat imgWithDrawnCircles = cd.drawCircles();


	cv::namedWindow("Picture", CV_WINDOW_AUTOSIZE);
	cv::imshow("Picture", imgWithDrawnCircles);
	cv::waitKey(0);
	
	return 0;
}