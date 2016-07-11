#include "CircleDetection.h"
#include <iostream>

#define PI 3.14159265



bool correctCircles_[];
bool correctCenterCircles_[4] = { true, true, true, true };
bool correctOutsideCircles_[4] = { true, true, true, true };


CircleDetection::CircleDetection()
{
	bigCirlceSize_ = 7 / 2; // in mm
	smallCirlceSize_ = 5 / 2; // in mm
	toleranceValue_ = 3; // in mm
	circleAmount_ = 6;
	isVertical_ = false;
}


CircleDetection::CircleDetection(double toleranceInMM)
{
	bigCirlceSize_ = 7 / 2; // in mm
	smallCirlceSize_ = 5 / 2; // in mm
	toleranceValue_ = toleranceInMM; // in mm
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
	//cv::HoughCircles(imgTemp, circTemp, cv::HOUGH_GRADIENT, 1, 40, 100, 25, 20, 300);
	/*
	int minSizeCircles = 20;
	int maxSizeCircles = 300;
	int minDist = 40;
	cv::HoughCircles(imgTemp, circTemp, cv::HOUGH_GRADIENT, 1, minDist, 100, 25, minSizeCircles, maxSizeCircles);
	*/
	int minSizeCircles = 1;
	int maxSizeCircles = 0;
	int minDist = 100; // 20
	//cv::HoughCircles(imgTemp, circTemp, cv::HOUGH_GRADIENT, 1, minDist, 100, 25, minSizeCircles, maxSizeCircles);


	double mmMask = mmToPixels(bigCirlceSize_) * 2;
	//oben links
	/*
	cv::Point2f pos = outsideCircles_[0];
	cv::Point center(pos.x, pos.y);
	int radius = cvRound(mmMask); 
	cv::circle(mask, center, radius, cv::Scalar(0, 0, 0), -1, 8, 0);
	cv::GaussianBlur(imgTemp, imgTemp, cv::Size(9, 9), 2, 2);
	cv::threshold(imgTemp, imgTemp, 40, 255, CV_THRESH_BINARY);
	cv::bitwise_or(imgTemp, mask, imgTemp);

	cv::GaussianBlur(imgTemp, imgTemp, cv::Size(9, 9), 2, 2);
	circTemp.clear();
	cv::HoughCircles(imgTemp, circTemp, cv::HOUGH_GRADIENT, 1, minDist, 100, 25, minSizeCircles, maxSizeCircles);
	circles_.insert(circles_.end(), circTemp.begin(), circTemp.end());
	*/

	int thresholdValue = 105;
	masking(outsideCircles_[0], thresholdValue, minSizeCircles, maxSizeCircles, minDist, mmMask);
	


	// unten rechts
	thresholdValue = 20;
	masking(outsideCircles_[1], thresholdValue, minSizeCircles, maxSizeCircles, minDist, mmMask);



	//oben rechts
	thresholdValue = 60;
	masking(outsideCircles_[2], thresholdValue, minSizeCircles, maxSizeCircles, minDist, mmMask);

	// unten links
	thresholdValue = 50;
	masking(outsideCircles_[3], thresholdValue, minSizeCircles, maxSizeCircles, minDist, mmMask);






	mmMask = mmToPixels(smallCirlceSize_) * 1.2;
	isVertical_= true;
	if(isVertical_)
	{
	//oben 
	thresholdValue = 50;
	masking(centralCircles_[0], thresholdValue, minSizeCircles, maxSizeCircles, minDist, mmMask);

	// unten 
	thresholdValue = 50;
	masking(centralCircles_[1], thresholdValue, minSizeCircles, maxSizeCircles, minDist, mmMask);
	} 
	else
	{

	//links 
	thresholdValue = 40;
	masking(centralCircles_[2], thresholdValue, minSizeCircles, maxSizeCircles, minDist, mmMask);


	// rechts 
	thresholdValue = 40;
	masking(centralCircles_[3], thresholdValue, minSizeCircles, maxSizeCircles, minDist, mmMask);
	}

	for (int i = 0; i < circles_.size(); i++)
	{
		std::cout << pixelsToMM(circles_[i][2] * 2) << std::endl;
	}

	/*

	for(int i = 0; i < outsideCircles_.size(); i++)                          // iterate through all detected Circles
	{
		double mmMask = mmToPixels(mmToPixels(bigCirlceSize_)*3);
		cv::Point2f pos = outsideCircles_[i];
		cv::Point center(pos.x, pos.y); // CVRound converts floating numbers to integer
		int radius = cvRound(mmMask);                              // Radius is the third parameter [i][0] = x [i][1]= y [i][2] = radius
		cv::circle(mask, center, radius, cv::Scalar(0, 0, 0), -1, 8, 0);    // Circle(img, center, radius, color, thickness=1, lineType=8, shift=0)	
}

	for (int i = 0; i < centralCircles_.size(); i++)                          // iterate through all detected Circles
	{
		if (correctCenterCircles_ == false) {
			continue;
		}
		double mmMask = mmToPixels(mmToPixels(smallCirlceSize_)*3);
		cv::Point2f pos = centralCircles_[i];
		cv::Point center(pos.x, pos.y); // CVRound converts floating numbers to integer
		int radius = cvRound(mmMask);                              // Radius is the third parameter [i][0] = x [i][1]= y [i][2] = radius
		cv::circle(mask, center, radius, cv::Scalar(0, 0, 0), -1, 8, 0);    // Circle(img, center, radius, color, thickness=1, lineType=8, shift=0)	
	}

	cv::bitwise_or(imgTemp, mask, imgTemp);

	// Blur it for better Hough Transformation
	cv::GaussianBlur(imgTemp, imgTemp, cv::Size(9, 9), 2, 2); // vorher ImgTemp
	//cv::namedWindow("Bla", CV_WINDOW_NORMAL);
	//cv::imshow("Bla",imgTemp);
	//cv::waitKey(0);

	// Apply the Hough Transform to find the circles
	//cv::HoughCircles(imgTemp, circles_, cv::HOUGH_GRADIENT, 1, 20, 100, 25, 1, 0);
	cv::HoughCircles(imgTemp, circles_, cv::HOUGH_GRADIENT, 1, 40, 100, 25, 20, 300);
	*/

	int size = circles_.size();

	correctCircles_.clear();
	for (int i = 0; i < size; i++) {
		correctCircles_.push_back(false);
		//std::cout << pixelsToMM(circles_[i][2]) << std::endl;
	}
	//std::cout << correctCircles_.size() << std::endl;
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
	cv::namedWindow("Thresh", CV_WINDOW_NORMAL);
	cv::imshow("Thresh", imgTemp);
	cv::bitwise_or(imgTemp, mask, imgTemp);
	cv::namedWindow("bitwise", CV_WINDOW_NORMAL);
	cv::imshow("bitwise", imgTemp);

	cv::GaussianBlur(imgTemp, imgTemp, cv::Size(9, 9), 2, 2);
	cv::HoughCircles(imgTemp, circTemp, cv::HOUGH_GRADIENT, 1, minDist, 100, 25, minSizeCircles, maxSizeCircles);
	circles_.insert(circles_.end(), circTemp.begin(), circTemp.end());
	cv::Mat img = drawCircles();

	draw_rotated_rect(img, rotetedRect_, cv::Scalar(255, 255, 255));

	cv::namedWindow("Circles", CV_WINDOW_NORMAL);
	cv::imshow("Circles", img);
}

cv::Mat CircleDetection::drawCircles()
{
	cv::Mat imgTemp = img_.clone();

	/// Draw the circles detected
	for (size_t i = 0; i < circles_.size(); i++)
	{
		cv::Point center(cvRound(circles_[i][0]), cvRound(circles_[i][1]));
		int radius = cvRound(circles_[i][2]);
		// circle center
		cv::circle(imgTemp, center, 3, cv::Scalar(255, 255, 255), -1, 8, 0);
		// circle outline
		cv::circle(imgTemp, center, radius, cv::Scalar(255, 255, 255), 2, 8, 0);
	}
	
	for (size_t i = 0; i < outsideCircles_.size(); i++)
	{
		cv::Point center(cvRound(outsideCircles_[i].x), cvRound(outsideCircles_[i].y));
	//	int radius = cvRound(3);
		// circle center
		cv::circle(imgTemp, center, 3, cv::Scalar(120, 120, 120), -1, 8, 0);
		// circle outline
	//	cv::circle(imgTemp, center, radius, cv::Scalar(120, 120, 120), 2, 8, 0);
	}

	for (size_t i = 0; i < centralCircles_.size(); i++)
	{
		cv::Point center(cvRound(centralCircles_[i].x), cvRound(centralCircles_[i].y));
		//int radius = cvRound(3);
		// circle center
		cv::circle(imgTemp, center, 3, cv::Scalar(120, 120, 120), -1, 8, 0);
		// circle outline
	//	cv::circle(imgTemp, center, radius, cv::Scalar(120, 120, 120), 2, 8, 0);
	}
	

	return imgTemp;
}


cv::Mat  CircleDetection::drawErrors()
{

	cv::Mat imgTemp = img_.clone();
	/// Draw the circles detected
	for (size_t i = 0; i < circles_.size(); i++)
	{
		cv::Point center(cvRound(circles_[i][0]), cvRound(circles_[i][1]));
		int radius = cvRound(circles_[i][2]);

		// draw box around circle if circle isn't correct
		if (!correctCircles_[i]) 
		{
			rectangle(imgTemp, cv::Point(center.x + radius + 5, center.y + radius + 5), cv::Point(center.x - radius - 5, center.y - radius - 5), cv::Scalar(0, 0, 255), 2, 8, 0);
		}
	}

	if (circles_.size() < circleAmount_) 
	{
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
	double toleranceValueInPixels = mmToPixels(toleranceValue_);
	double distance = std::numeric_limits<double>::max();
	int indexOfClosestCircle = -1;
	for (int i = 0; i < circles_.size(); i++) 
	{
		cv::Point2f circle(circles_[i][0], circles_[i][1]);
		double distTemp = euclidianDistance(calculatedPosition, circle);
		if (distTemp < distance && distTemp <= toleranceValueInPixels) 
		{
			
			distance = distTemp;
			indexOfClosestCircle = i;
		}
	}

	std::cout << pixelsToMM(distance) << std::endl;
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


	// matrices we'll use
	Mat M, rotated, bridge, bridgeThreshold;
	// get angle and size from the bounding box
	float angle = rotetedRect_.angle;
	Size rect_size = rotetedRect_.size;
	if (rotetedRect_.angle < -45.) {
		angle += 90.0;
		swap(rect_size.width, rect_size.height);
	}
	// get the rotation matrix
	M = getRotationMatrix2D(rotetedRect_.center, angle, 1.0);
	// perform the affine transformation
	warpAffine(img_, rotated, M, img_.size(), INTER_CUBIC);
	// crop the resulting image
	getRectSubPix(rotated, rect_size, rotetedRect_.center, bridge);


	double pixels = mmToPixels(1);
	pos = vertices[tl] - cv::Point2f(pixels, pixels);

	cv::circle(rotated, pos, 3, cv::Scalar(255, 255, 255), -1, 8, 0);
	cv::namedWindow("rotated", CV_WINDOW_NORMAL);
	cv::imshow("rotated", rotated);


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

	double bigCirlceSizeInPixels = mmToPixels(bigCirlceSize_);
	double smallCirlceSizeinPixels = mmToPixels(smallCirlceSize_);
	double toleranceValueInPixles = mmToPixels(toleranceValue_);
	bool horizontalOrientation = true;

	// checking orientation of model
	int closestCirlceTop = findClosestCirlce(centralCircles_[0]);
	int closestCirlceButtom = findClosestCirlce(centralCircles_[1]);
	int closestCirlceLeft = findClosestCirlce(centralCircles_[2]);
	int closestCirlceRight = findClosestCirlce(centralCircles_[3]);

	int start = 0;
	// if a circle is on top or buttom then its not horizontel
	if (closestCirlceTop != -1 || closestCirlceButtom != -1) 
	{
		horizontalOrientation = false;
	}

	// change back tohorizontal if both are horizontal  <<< IF 3 circles are detected (two horizontal) or  if 4 then use horizontal
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
			if (radius - smallCirlceSizeinPixels <= toleranceValueInPixles / 2)
			{
				correctCircles_[id] = true;
				correctCenterCircles_[i] = true;
			}
			else {
				correctCircles_[id] = false;
				correctCenterCircles_[i] = false;
				isCorrect_ = false;
			}
		} 
		else
		{
			isCorrect_ = false;
		}

	}

	int closestCirlceLeftTop = findClosestCirlce(outsideCircles_[0]);
	int closestCirlceRightButtom = findClosestCirlce(outsideCircles_[1]);
	int closestCirlceRightTop = findClosestCirlce(outsideCircles_[2]);
	int closestCirlceLeftButtom = findClosestCirlce(outsideCircles_[3]);

	std::vector<int> foundCircles;
	foundCircles.push_back(closestCirlceLeftTop);
	foundCircles.push_back(closestCirlceRightButtom);
	foundCircles.push_back(closestCirlceRightTop);
	foundCircles.push_back(closestCirlceLeftButtom);







	double  sizeLeftTop = bigCirlceSizeInPixels;
	double sizeRightTop = smallCirlceSizeinPixels;

	for (int i = 0; i < foundCircles.size(); i++) {
		int id = foundCircles[i];
		if (id == -1) {
			correctOutsideCircles_[0] = false;
			isCorrect_ = false;
			continue;
		}
		double radius = circles_[id][2];
		if (radius - bigCirlceSizeInPixels <= toleranceValueInPixles /2) {
			if (i < 2) {
				sizeLeftTop = bigCirlceSizeInPixels;
				sizeRightTop = smallCirlceSizeinPixels;
			}
			else {
				sizeLeftTop = smallCirlceSizeinPixels;
				sizeRightTop = bigCirlceSizeInPixels;
			}
			break;
		}

		if (radius - smallCirlceSizeinPixels <= toleranceValueInPixles /2)
		{
			if (i < 2) {
				sizeLeftTop = smallCirlceSizeinPixels;
				sizeRightTop = bigCirlceSizeInPixels;
			} 
			else
			{
				sizeLeftTop = bigCirlceSizeInPixels;
				sizeRightTop = smallCirlceSizeinPixels;
			}
			break;
		}

	}



	for (int i = 0; i < foundCircles.size(); i++) {
		int id = foundCircles[i];
		if (id == -1) {
			correctOutsideCircles_[i] = false;
			isCorrect_ = false;
			continue;
		}

		double radius = circles_[id][2];


		if (i < 2) {
			if (radius - sizeLeftTop <= toleranceValueInPixles /2)
			{
				correctCircles_[id] = true;
			}
			else
			{
				isCorrect_ = false;
			}
		}
		else {
			if (radius - sizeRightTop <= toleranceValueInPixles /2)
			{
				correctCircles_[id] = true;
			}
			else
			{
				isCorrect_ = false;
			}
		}

	}

}


