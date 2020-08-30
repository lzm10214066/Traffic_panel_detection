#ifndef _FEATURE_H
#define _FEATURE_H

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"

using namespace cv;

double simiGauss(vector<float> &h1, vector<float> &h2);

void calculateColorHist(Mat &src, vector<float> &hist, int h_bins, int s_bins);
void calculateColorHist(Mat &src, vector<float> &hist, int h_bins, int s_bins, double r);
void calculateColorHistRect(Mat &src, vector<float> &hist, int h_bins, int s_bins, double r);

void calculateColorHistSpm(Mat &src, vector<float> &hist, int h_bins, int s_bins, int nlevels);

void calculateColorHistMask(Mat &src, Mat &mask, vector<float> &hist, int h_bins, int s_bins);


#endif