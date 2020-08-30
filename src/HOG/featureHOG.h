#ifndef _FEATUREHOG_H
#define _FEATUREHOG_H

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\objdetect\objdetect.hpp"

using namespace cv;
using namespace std;


void calEdge(Mat &img, vector<double> &edgeF);
void calculateHOG(Mat &src, vector<float> &descriptors);

#endif