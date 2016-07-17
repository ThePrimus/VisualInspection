#ifndef CIRCLE_DETECTION
#define CIRCLE_DETECTION


#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "detect_quad.h"

class CircleDetection {
public:
	/**
	Constructor for a circle detection object 
	*/
	CircleDetection();


	/**
	Constructor for a circle detection object. 
	@param sizeToleranceInMM tolerance value for the size of a circle
	@param positionToleranceInMM tolerance value for the position of a circle
	*/
	CircleDetection(double sizeToleranceInMM, double positionToleranceInMM);

	/**
	Sets the image which should be used for the circle detection
	@param img Image which should be used
	*/
	void setImage(cv::Mat img);

	/**
	Finds the circles on a image
	*/
	void findCircles();

	/**
	Draws the detected circles on a image
	@param drawCorrectCircles 
	@return the image on which the circles are drawn
	*/
	cv::Mat drawCircles(bool drawCorrectCircles);

	/**
	Sets the rotated rectangle
	@param rotatedRect the rotated rectangle object
	*/
	void setRotatedRect(cv::RotatedRect rotetedRect);
	/**
	Returns the circles which are detected
	@return vector of the detected circles
	*/
	std::vector<cv::Vec3f>  getCircles();

	/**
	Convers given pixels to millimeter
	@param pixels Number of pixels which should be converted to mm
	@return mm value of the given pixels
	*/
	double pixelsToMM(int pixels);

	/**
	Convers given millimeter to pixels
	@param mm Number of mm which should be converted to pixels
	@return pixel value of the given mm
	*/
	double mmToPixels(double mm);

	/**
	Finds circles which is close to a given position
	@param calculatedPosition which should be used to find a circle which is the closest
	@return ID of the circle which is the closest. -1 if none is closer then the position tolerance value
	*/
	int findClosestCirlce(cv::Point2f calculatedPosition);

	/**
	@return true if model is correct
	*/
	bool isModelCorrect();

	/**
	Calculates the expected positions for the circles
	*/
	void calculateExpectedCirclePositions();

	/**
	checks if detected circles are correct
	*/
	void checkCircles();
	
	/**
	Draws detected errors on image
	@return image with detected errors
	*/
	cv::Mat drawErrors();

	/**
	Sets the px2cm value
	@param pixelConversion px2cm value
	*/
	void setPixelConversion(double pixelConversion);
private:
	/**
	Indicates if the object is in vertical orientation
	*/
	bool isVertical_;

	/**
	Indicates if everything is correct
	*/
	bool isCorrect_;

	/**
	Amount of circles which should be checked
	*/
	int circleAmount_;

	/**
	px2cm conversion value
	*/
	double pixelConversion_;

	/**
	Size for the big circles
	*/
	double bigCirlceSize_;
	
	/**
	Size for the small circles
	*/
	double smallCirlceSize_;
	
	/**
	Tolerance value of the cirlce size
	*/
	double circleSizeTolerance_;
	
	/**
	Tolerance value of the circle position
	*/
	double circlePositionTolerance_;
	
	/**
	Vector of the central circle positions
	*/
	std::vector<cv::Point2f> centralCircles_;
	
	/**
	Vector of the outside circle positions
	*/
	std::vector<cv::Point2f> outsideCircles_;
	
	/**
	Image which should be checked
	*/
	cv::Mat img_;
	
	/**
	All detected circles of the image
	*/
	std::vector<cv::Vec3f> circles_;
	
	/**
	Rotated rectangle which is used to calculate the possible positions
	*/
	cv::RotatedRect rotetedRect_;

	/**
	Returns the position of the outer Circle
	@param center center of the rotated rectangle
	@param circleCenter Center of the detected Circle
	@return Calculated position
	*/
	cv::Point2f getPositionOfOuterCircle(cv::Point2f center, cv::Point2f circleCenter);
	
	/**
	Returns the position of the center Circle
	@param x direction for the center circle
	@param y direction for the center circle
	@return Calculated position of the inner circle
	*/
	cv::Point2f getPositionOfInnerCircles(int x, int y);

	/**
	Euklidian distance from a to b
	@param a 
	@param b
	@return euclidian distance from a to b
	*/
	double euclidianDistance(cv::Point2f a, cv::Point2f b);

	/**
	Masks the given point
	*/
	void masking(cv::Point2f, int thresholdValue, int minSizeCircles, int maxSizeCircles, int minDist, int mmMask);
};


#endif