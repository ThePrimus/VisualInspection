#pragma once

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std;

void bilateral_filter(Mat in, Mat* out);
void guided_filter(Mat in, Mat* out);
void canny_detection(Mat* in, Mat* out, int threshold, bool dilate_output = false);
int clusters(Mat* in, Mat* out, int min_size);
bool detect_damage(Mat* image, RotatedRect* rect, vector<Vec3f> circles, int threshold, int min_cluster_size, int rectangle_line_size, int radius_extension);
void remove_edges(Mat in, Mat* out, RotatedRect* rect, int rectangle_line_size);
void remove_circles(Mat in, Mat* out, vector<Vec3f> circles, int radius_extension);


