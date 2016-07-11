#pragma once

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <array>

using namespace cv;
using namespace std;

void draw_rotated_rect(Mat out, const RotatedRect& rect, Scalar color);
void draw_contour(Mat out, const vector<Point>& contour, Scalar color);
void draw_quad_info(Mat out, RotatedRect* rect, Scalar rect_color, vector<Point>* cont, Scalar cont_color);

auto rotatedrect_area(RotatedRect rect);
bool check_quad_size(const RotatedRect& rect, const double px2cm, const double thresh_accuracy, const double side_length);
Mat image_preprocessing(Mat input);
bool detect_quad(Mat in, double thresh_accuracy, double alpha, double beta, double side_length, RotatedRect* out_rect = NULL, vector<Point>* out_cont = NULL);