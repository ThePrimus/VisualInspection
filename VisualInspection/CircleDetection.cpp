#include "CircleDetection.h"


using namespace cv;
using namespace std;


CircleDetection::CircleDetection()
{
}

void CircleDetection::setImage(cv::Mat img)
{
	img_ = img;
}

void CircleDetection::setRotatedRect(cv::RotatedRect rotetedRect)
{
	rotetedRect_ = rotetedRect;
}

std::vector<cv::Vec3f> CircleDetection::getCircles()
{
	return circles_;
}


void CircleDetection::findCircles()
{
	cv::Mat imgTemp;

	/// Convert it to gray
	cvtColor(img_, imgTemp, CV_BGR2GRAY);

	// Filter picture
	cv::threshold(imgTemp, imgTemp, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

	// Blur it for better Hough Transformation
	GaussianBlur(imgTemp, imgTemp, cv::Size(9, 9), 2, 2);

	// Apply the Hough Transform to find the circles
	HoughCircles(imgTemp, circles_, HOUGH_GRADIENT, 1, 20, 100, 25, 1, 0);
}

cv::Mat CircleDetection::drawCircles()
{
	cv::Mat imgTemp = img_;

	/// Draw the circles detected
	for (size_t i = 0; i < circles_.size(); i++)
	{
		cv::Point center(cvRound(circles_[i][0]), cvRound(circles_[i][1]));
		int radius = cvRound(circles_[i][2]);
		// circle center
		circle(imgTemp, center, 3, cv::Scalar(0, 230, 0), -1, 8, 0);
		// circle outline
		circle(imgTemp, center, radius, cv::Scalar(0, 230, 0), 2, 8, 0);
	}
	return imgTemp;
}