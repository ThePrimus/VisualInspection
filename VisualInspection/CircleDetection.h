#ifndef CIRCLE_DETECTION
#define CIRCLE_DETECTION


#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


class CircleDetection {
public:
	CircleDetection();
	void setImage(cv::Mat img);
	void findCircles();
	cv::Mat drawCircles();
	void setRotatedRect(cv::RotatedRect rotetedRect);
	std::vector<cv::Vec3f>  getCircles();

private:
	cv::Mat img_;
	std::vector<cv::Vec3f> circles_;
	cv::RotatedRect rotetedRect_;
};


#endif