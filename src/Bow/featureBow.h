#ifndef _FEATUREBOW_H
#define _FEATUREBOW_H

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\objdetect\objdetect.hpp"

#include "hog_mi.h"
#include "../utility.h"

using namespace cv;
using namespace std;

int readCodebook(string codebookFile, Mat &codebook);
void vector2mat(vector<vector<double> >& des, Mat &desMat);

void calculateBowSimilarity(Mat &src, vector<double> &hist, Mat& codebook, HOG_opt &hog_opt, double length);
void calculateBowHistHOG(Mat &src, vector<double> &histHOG, Mat& codebook, HOG_opt &hog_opt, double length);
void calculateBowHOG(Mat &src, vector<double> &histHOG, Mat& codebook, HOG_opt &hog_opt, double length);

#endif