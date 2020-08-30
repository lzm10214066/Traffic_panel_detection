#pragma once
#include <iostream>

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"

#include "Lines/lines.h"
#include "Lines/lsd.h"
#include "LBP/classifierrLBP.h"

using namespace std;
using namespace cv;

void getProposals(Mat &img,vector<Rect> &recBag);

string getFileName(string str);
void getRangeFromTxt(string tempObject, int *xmin, int *xmax, int *ymin, int *ymax);
int similarity(Rect r, int xmin, int ymin, int xmax, int ymax, double th);
int overlap_object(Rect r, int *xmin, int *ymin, int *xmax, int *ymax, int n, double th);
void rect_extend(vector<Rect> &rects, double r, int width, int height);

void loadFile(vector<string> &image_path, vector<string> &data_path, string imageFile, string dataFile);

void supress(vector<Rect> &recBag, vector<double> &scores, double th);
double getBoxScoreOne(Rect &recBag, Mat &lineMap);
void supressContain(vector<Rect> &recBag, vector<double> &scores, double r);
void normSize(Mat &recImage, double length);

void getLines(Mat &img, vector<Vec4i> &lines_v, vector<Vec4i> &line_h);
void cutRects(vector<Rect> &recBag, Mat &img, CvSVM &svm, double s_th);

