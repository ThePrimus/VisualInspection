#pragma once

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std;

void loadImage(Mat* image);
void bilateral_filter(Mat in, Mat* out);
void guided_filter(Mat in, Mat* out);
void canny_detection(Mat* in, Mat* out, int threshold, bool dilate_output = false);
int clusters(Mat* in, Mat* out);
bool detect_damage(Mat* image, int threshold, RotatedRect* rect, vector<Vec3f> circles);
void remove_edges(Mat in, Mat* out, RotatedRect* rect);


