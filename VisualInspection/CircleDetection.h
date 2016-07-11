#ifndef CIRCLE_DETECTION
#define CIRCLE_DETECTION


#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "detect_quad.h"

class CircleDetection {
public:
	CircleDetection();
	CircleDetection(double toleranceInMM);
	void setImage(cv::Mat img);
	void findCircles();
	void masking(cv::Point2f, int thresholdValue, int minSizeCircles, int maxSizeCircles, int minDist, int mmMask);
	cv::Mat drawCircles();
	void setRotatedRect(cv::RotatedRect rotetedRect);
	std::vector<cv::Vec3f>  getCircles();
	double pixelsToMM(int pixels);
	double mmToPixels(double mm);
	cv::Point2f getPositionOfOuterCircle(cv::Point2f center, cv::Point2f circleCenter);
	cv::Point2f getPositionOfInnerCircles(int x, int y);
	int findClosestCirlce(cv::Point2f calculatedPosition);
	bool isModelCorrect();
	void calculateExpectedCirclePositions();
	void checkCircles();
	cv::Mat drawErrors();
	void setPixelConversion(double pixelConversion);
	double euclidianDistance(cv::Point2f a, cv::Point2f b);

private:
	bool isVertical_;
	std::vector<bool> correctCircles_;
	bool isCorrect_;
	int circleAmount_;
	double pixelConversion_;
	double bigCirlceSize_;
	double smallCirlceSize_;
	double toleranceValue_;
	std::vector<cv::Point2f> centralCircles_;
	std::vector<cv::Point2f> outsideCircles_;
	cv::Mat img_;
	std::vector<cv::Vec3f> circles_;
	cv::RotatedRect rotetedRect_;
};


#endif