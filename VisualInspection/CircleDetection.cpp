#include "CircleDetection.h"
#include <iostream>

#define PI 3.14159265


bool correctCenterCircles_[4] = { true, true, true, true };
bool correctOutsideCircles_[4] = { true, true, true, true };


CircleDetection::CircleDetection()
{
	bigCirlceSize_ = 7 / 2.0f; // in mm
	smallCirlceSize_ = 5 / 2.0f; // in mm
	circleSizeTolerance_ = 1.5f; // 1.5 // in mm
	circlePositionTolerance_ = 2.0f; // 2.0f // in mm
	circleAmount_ = 6;
	isVertical_ = false;
}


CircleDetection::CircleDetection(double sizeToleranceInMM, double positionToleranceInMM)
{
	bigCirlceSize_ = 7 / 2; // in mm
	smallCirlceSize_ = 5 / 2; // in mm
	circleSizeTolerance_ = sizeToleranceInMM; // in mm
	circlePositionTolerance_ = positionToleranceInMM; // in mm
	circleAmount_ = 6;
}

void CircleDetection::setImage(cv::Mat img)
{
	img_ = img;
	isCorrect_ = true;
	for (int i = 0; i < 4; i++)
	{
		correctCenterCircles_[i] = true;
		correctOutsideCircles_[i] = true;
	}
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
	int minSizeCircles = 1;
	int maxSizeCircles = 0;
	int minDist = 100; 

	double mmMask = mmToPixels(bigCirlceSize_) * 2;
	//corner holes
	//top left
	int thresholdValue = 70;
	masking(outsideCircles_[0], thresholdValue, minSizeCircles, maxSizeCircles, minDist, mmMask);

	// bottom right
	thresholdValue = 40;
	masking(outsideCircles_[1], thresholdValue, minSizeCircles, maxSizeCircles, minDist, mmMask);

	//top right
	thresholdValue = 40;
	masking(outsideCircles_[2], thresholdValue, minSizeCircles, maxSizeCircles, minDist, mmMask);

	// buttom left
	thresholdValue = 50;
	masking(outsideCircles_[3], thresholdValue, minSizeCircles, maxSizeCircles, minDist, mmMask);


	//center holes
	mmMask = mmToPixels(smallCirlceSize_) * 2;
	isVertical_= true;

	std::vector<cv::Vec3f> circTemp;
	cv::Mat imgTemp;
	img_.convertTo(imgTemp, -1);
	cv::Mat maskedImage;
	cv::Mat mask(imgTemp.size(), imgTemp.type());
	mask.setTo(cv::Scalar(255, 255, 255));

	cv::GaussianBlur(imgTemp, imgTemp, cv::Size(9, 9), 2, 2);

	for (int i = 0; i < centralCircles_.size(); i++)   // mask all circles in center
	{
		if (correctCenterCircles_ == false) {
			continue;
		}
		cv::Point2f pos = centralCircles_[i];
		cv::Point center(pos.x, pos.y); 
		int radius = cvRound(mmMask);                            
		cv::circle(mask, center, radius, cv::Scalar(0, 0, 0), -1, 8, 0);
	}
	
	cv::threshold(imgTemp, imgTemp, thresholdValue, 255, CV_THRESH_BINARY);
	cv::bitwise_or(imgTemp, mask, imgTemp);

	cv::GaussianBlur(imgTemp, imgTemp, cv::Size(9, 9), 2, 2);
	cv::HoughCircles(imgTemp, circTemp, cv::HOUGH_GRADIENT, 1, minDist-50, 100, 25, minSizeCircles, maxSizeCircles);
	circles_.insert(circles_.end(), circTemp.begin(), circTemp.end());

}


void CircleDetection::masking(cv::Point2f pos, int thresholdValue, int minSizeCircles, int maxSizeCircles, int minDist, int mmMask)
{

	std::vector<cv::Vec3f> circTemp;
	cv::Mat imgTemp;
	img_.convertTo(imgTemp, -1);
	cv::Mat maskedImage;
	cv::Mat mask(imgTemp.size(), imgTemp.type());
	mask.setTo(cv::Scalar(255, 255, 255));
	int radius = cvRound(mmMask);

	cv::GaussianBlur(imgTemp, imgTemp, cv::Size(9, 9), 2, 2);
	cv::circle(mask, pos, radius, cv::Scalar(0, 0, 0), -1, 8, 0);
	cv::threshold(imgTemp, imgTemp, thresholdValue, 255, CV_THRESH_BINARY);
	cv::bitwise_or(imgTemp, mask, imgTemp);
	cv::GaussianBlur(imgTemp, imgTemp, cv::Size(9, 9), 2, 2);
	cv::HoughCircles(imgTemp, circTemp, cv::HOUGH_GRADIENT, 1, minDist, 100, 25, minSizeCircles, maxSizeCircles);
	circles_.insert(circles_.end(), circTemp.begin(), circTemp.end());
}

cv::Mat CircleDetection::drawCircles(bool drawCorrectCircles)
{
	cv::Mat imgTemp = img_.clone();
	cv::cvtColor(imgTemp, imgTemp, CV_GRAY2RGB);

	/// Draw the circles detected
	for (size_t i = 0; i < circles_.size(); i++)
	{
		cv::Point center(cvRound(circles_[i][0]), cvRound(circles_[i][1]));
		int radius = cvRound(circles_[i][2]);
		// circle center
		cv::circle(imgTemp, center, 3, cv::Scalar(0, 0, 255), -1, 8, 0);
		// circle outline
		cv::circle(imgTemp, center, radius, cv::Scalar(0, 0, 255), 2, 8, 0);
	}
	if(drawCorrectCircles)
	{ 
	for (size_t i = 0; i < outsideCircles_.size(); i++)
	{
		cv::Point center(cvRound(outsideCircles_[i].x), cvRound(outsideCircles_[i].y));
		cv::circle(imgTemp, center, 3, cv::Scalar(0, 0, 120), -1, 8, 0);
	}

	for (size_t i = 0; i < centralCircles_.size(); i++)
	{
		cv::Point center(cvRound(centralCircles_[i].x), cvRound(centralCircles_[i].y));
		cv::circle(imgTemp, center, 3, cv::Scalar(0, 0, 120), -1, 8, 0);
	}
	
	cv::Point2f vertices[4];
	rotetedRect_.points(vertices);

	cv::line(imgTemp, vertices[0], vertices[2], cv::Scalar(0, 0, 120), 1);
	cv::line(imgTemp, vertices[1], vertices[3], cv::Scalar(0, 0, 120), 1);
	}
	return imgTemp;
}


cv::Mat  CircleDetection::drawErrors()
{

	cv::Mat imgTemp = img_.clone();
	cv::cvtColor(imgTemp, imgTemp, CV_GRAY2RGB);

	/// Draw the circles detected
	for (int i = 0; i < centralCircles_.size(); i++) 
		{
			double radius = mmToPixels(smallCirlceSize_);
			if (!correctCenterCircles_[i])
			{
				rectangle(imgTemp, cv::Point(centralCircles_[i].x + radius + 5, centralCircles_[i].y + radius + 5), cv::Point(centralCircles_[i].x - radius - 5, centralCircles_[i].y - radius - 5), cv::Scalar(0, 0, 255), 2, 8, 0);
			}
			radius = mmToPixels(bigCirlceSize_);
			if (!correctOutsideCircles_[i])
			{
				rectangle(imgTemp, cv::Point(outsideCircles_[i].x + radius + 5, outsideCircles_[i].y + radius + 5), cv::Point(outsideCircles_[i].x - radius - 5, outsideCircles_[i].y - radius - 5), cv::Scalar(0, 0, 255), 2, 8, 0);
			}
		}

	return imgTemp;
}

void CircleDetection::setPixelConversion(double pixelConversion)
{
	pixelConversion_ = pixelConversion;
}

double CircleDetection::euclidianDistance(cv::Point2f a, cv::Point2f b)
{
	cv::Point2f diff = a - b;
	return cv::sqrt(diff.x*diff.x + diff.y*diff.y);
}


double CircleDetection::pixelsToMM(int pixels) {
	return pixels / pixelConversion_;
}

double CircleDetection::mmToPixels(double mm) {
	return mm*pixelConversion_;
}



cv::Point2f CircleDetection::getPositionOfOuterCircle(cv::Point2f center, cv::Point2f circleCenter)
{
	double dir_x = center.x - circleCenter.x;
	double dir_y = center.y - circleCenter.y;
	double norm = sqrt(dir_x*dir_x + dir_y*dir_y);
	double eigen_dir_x = dir_x / norm;
	double eigen_dir_y = dir_y / norm;
	double hyp = sqrt(80 * 80 + 80 * 80) / 2;
	double hyp_2 = sqrt(10 * 10 + 10 * 10);
	double dist = mmToPixels(hyp - hyp_2);
	return cv::Point2f(rotetedRect_.center - cv::Point2f(eigen_dir_x, eigen_dir_y)*dist);
}

cv::Point2f CircleDetection::getPositionOfInnerCircles(int x, int y)
{
	double angle = rotetedRect_.angle * 2 * PI / 360;
	double px = x * cos(angle) - y * sin(angle);
	double py = x * sin(angle) + y * cos(angle);
	double dist = mmToPixels(0.5 + 2.5);
	return cv::Point2f(rotetedRect_.center + cv::Point2f(px, py)*dist);
}

int CircleDetection::findClosestCirlce(cv::Point2f calculatedPosition)
{
	double toleranceValueInPixels = mmToPixels(circlePositionTolerance_);
	double distance = std::numeric_limits<double>::max();
	int indexOfClosestCircle = -1;
	for (int i = 0; i < circles_.size(); i++) 
	{
		cv::Point2f circle(circles_[i][0], circles_[i][1]);
		double distTemp = euclidianDistance(calculatedPosition, circle);
		distTemp = pixelsToMM(distTemp);
		if (distTemp < distance && distTemp <= circlePositionTolerance_)
		{
			
			distance = distTemp;
			indexOfClosestCircle = i;
		}
	}
	return indexOfClosestCircle;
}

bool CircleDetection::isModelCorrect()
{
	return isCorrect_;
}


void CircleDetection::calculateExpectedCirclePositions() 
{
	cv::Point2f vertices[4];
	rotetedRect_.points(vertices);

	// Circle in corners
	// order of vertices!!
	int tl;
	int tr;
	int bl;
	int br;

	for (int i = 0; i < 4; i++)
	{
		//top left
		if (vertices[i].x <= rotetedRect_.center.x && vertices[i].y <= rotetedRect_.center.y)
		{
			tl = i;
		}

		//buttom right
		if (vertices[i].x >= rotetedRect_.center.x && vertices[i].y >= rotetedRect_.center.y)
		{
			br = i;
		}

		//top right
		if (vertices[i].x >= rotetedRect_.center.x && vertices[i].y <= rotetedRect_.center.y)
		{
			tr = i;
		}

		//buttom left
		if (vertices[i].x <= rotetedRect_.center.x && vertices[i].y >= rotetedRect_.center.y)
		{
			bl = i;
		}
	}


	//top left
	cv::Point2f pos = getPositionOfOuterCircle(rotetedRect_.center, vertices[tl]); // 2 // 1
	outsideCircles_.push_back(pos);


	//buttom right
	pos = getPositionOfOuterCircle(rotetedRect_.center, vertices[br]); // 0 // 3
	outsideCircles_.push_back(pos);

	//top right
	pos = getPositionOfOuterCircle(rotetedRect_.center, vertices[tr]); // 3 // 2
	outsideCircles_.push_back(pos);

	//buttom left
	pos = getPositionOfOuterCircle(rotetedRect_.center, vertices[bl]); // 1 // 0
	outsideCircles_.push_back(pos);


	// center circles
	//top
	pos = getPositionOfInnerCircles(0, -1);
	centralCircles_.push_back(pos);

	//buttom
	pos = getPositionOfInnerCircles(0, 1);
	centralCircles_.push_back(pos);

	// left
	pos = getPositionOfInnerCircles(-1, 0);
	centralCircles_.push_back(pos);

	//right
	pos = getPositionOfInnerCircles(1, 0);
	centralCircles_.push_back(pos);
}



void CircleDetection::checkCircles()
{

	bool horizontalOrientation = true;

	// checking orientation of model
	//std::cout << "Detected Distances: " << std::endl;
	//std::cout << "Center Top: ";
	int closestCirlceTop = findClosestCirlce(centralCircles_[0]);
	//std::cout << "Center Buttom: ";
	int closestCirlceButtom = findClosestCirlce(centralCircles_[1]);
	//std::cout << "Center Left: ";
	int closestCirlceLeft = findClosestCirlce(centralCircles_[2]);
	//std::cout << "Center Right: ";
	int closestCirlceRight = findClosestCirlce(centralCircles_[3]);

	int start = 0;
	// if a circle is on top or buttom then its not horizontel
	if (closestCirlceTop != -1 || closestCirlceButtom != -1) 
	{
		horizontalOrientation = false;
	}

	// change back to horizontal if both are horizontal  <<< IF 3 circles are detected (two horizontal) or  if 4 then use horizontal
	if (closestCirlceLeft != -1 && closestCirlceRight != -1) 
	{
		horizontalOrientation = true;
		start = 2;
	}

	std::vector<int> foundCircles2;
	foundCircles2.push_back(closestCirlceTop);
	foundCircles2.push_back(closestCirlceButtom);
	foundCircles2.push_back(closestCirlceLeft);
	foundCircles2.push_back(closestCirlceRight);

	for (int i = start; i < start + 2; i++)
	{
		if (foundCircles2[i] != -1)
		{
			int id = foundCircles2[i];
			double radius = circles_[id][2];
			radius = pixelsToMM(radius);
			double diff = abs(radius - smallCirlceSize_);
			diff = diff*2;
			if (diff <= circleSizeTolerance_)
			{
				correctCenterCircles_[i] = true;
			}
			else 
			{
				correctCenterCircles_[i] = false;
				isCorrect_ = false;
			}
		} 
		else
		{
			isCorrect_ = false;
		}

	}

	//std::cout << "Top Left: ";
	int closestCirlceLeftTop = findClosestCirlce(outsideCircles_[0]);
	//std::cout << "Buttom Right: ";
	int closestCirlceRightButtom = findClosestCirlce(outsideCircles_[1]);
	//std::cout << "Top Right: ";
	int closestCirlceRightTop = findClosestCirlce(outsideCircles_[2]);
	//std::cout << "Buttom Left: " ;
	int closestCirlceLeftButtom = findClosestCirlce(outsideCircles_[3]);

	std::vector<int> foundCircles;
	foundCircles.push_back(closestCirlceLeftTop);
	foundCircles.push_back(closestCirlceRightButtom);
	foundCircles.push_back(closestCirlceRightTop);
	foundCircles.push_back(closestCirlceLeftButtom);


	double circleSizes[4];

	for (int i = 0; i < foundCircles.size(); i++) {
		int id = foundCircles[i];
		if (id == -1) {
			circleSizes[i] = -1;
			correctOutsideCircles_[i] = false;
			isCorrect_ = false;
			continue;
		}
		double radius = circles_[id][2];
		radius = pixelsToMM(radius);
		double diffSmall = abs(radius - smallCirlceSize_);
		diffSmall = diffSmall * 2;

		double diffBig = abs(radius - bigCirlceSize_);
		diffBig = diffBig * 2;

		if (diffSmall <= circleSizeTolerance_ && diffSmall <= diffBig)
		{
			circleSizes[i] = smallCirlceSize_;
		}
		else if (diffBig <= circleSizeTolerance_)
		{
			circleSizes[i] = bigCirlceSize_;
		}
		else
		{
			circleSizes[i] = -1;
			correctOutsideCircles_[i] = false;
			isCorrect_ = false;
		}
	}
	/*
	std::cout << "Detected sizes: " << std::endl;
	if (closestCirlceLeftTop != -1) {
	std::cout << "Top Left: " << circleSizes[0] << " Radius: " << pixelsToMM(circles_[closestCirlceLeftTop][2]) << std::endl;
	}
	if (closestCirlceRightButtom != -1) {
	std::cout << "Buttom Rright: " << circleSizes[1] << " Radius: " << pixelsToMM(circles_[closestCirlceRightButtom][2] ) << std::endl;
	}
	if (closestCirlceRightTop != -1) {
	std::cout << "Top Right: " << circleSizes[2] << " Radius: " << pixelsToMM(circles_[closestCirlceRightTop][2] ) << std::endl;
	}
	if (closestCirlceLeftButtom != -1) {
	std::cout << "Buttom Left: " << circleSizes[3] << " Radius: " << pixelsToMM(circles_[closestCirlceLeftButtom][2]) << std::endl;
	}
	*/
	if (isCorrect_ == false)
	{
		return;
	}

	if (circleSizes[0] == circleSizes[1]) // top left == buttom right
	{

		if (circleSizes[2] == circleSizes[3]) //  top right == buttom left
		{
			if (circleSizes[0] == circleSizes[2]) // top right == top left
			{// all are same size so mark top right and buttom left as wrong
				correctOutsideCircles_[2] = false;
				correctOutsideCircles_[3] = false;
				isCorrect_ = false;
			} 
		} 
		else
		{ // top right and buttom left arent same size so check which is wrong
			if (circleSizes[0] == circleSizes[2])
			{
				correctOutsideCircles_[2] = false;
				isCorrect_ = false;
			}
			else
			{
				correctOutsideCircles_[3] = false;
				isCorrect_ = false;
			}
		}
	}
	else // top left != buttom right
	{
		isCorrect_ = false;
		if (circleSizes[2] != circleSizes[3]) // top right != buttom left
		{ // mark all as false cause can't decide which is correct
				correctOutsideCircles_[0] = false;
				correctOutsideCircles_[1] = false;
				correctOutsideCircles_[2] = false;
				correctOutsideCircles_[3] = false;
				isCorrect_ = false;
		} 
		else // top right == buttom left
		{
			if (circleSizes[0] != circleSizes[2]) // top left != top left
			{
				// therefor buttom right must be false
				correctOutsideCircles_[1] = false;
				isCorrect_ = false;
			} 
			else // top left == top right
			{ // therefor top left must be false
				correctOutsideCircles_[0] = false;
				isCorrect_ = false;

			}
		}
	}

}


