#ifndef LINES_H
#define LINES_H

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/features2d/features2d.hpp"

#include <algorithm>
#include <iostream>

using namespace cv;
using namespace std;

int lineDirection(vector<Point> &line, Mat &angle);
void fixLines_vertical(vector<Vec4i> &lines_in);
void fixLines_horizon(vector<Vec4i> &lines_in);
void repairRectangle_x(vector<Vec4i> &l_x, vector<Vec4i> &l_y, Mat &image);
void repairRectangle_y(vector<Vec4i> &l_x, vector<Vec4i> &l_y, Mat &image);
void myLine(Mat &image, Point p1, Point p2, uchar v);
int isIntersection(Point p, Mat &image);
void fixLines_start_end(vector<Vec4i> &lines_in, int lineType);
void fixLines_close(vector<Vec4i> &lines_in, bool v_or_h);
void fixLines_intersection(vector<Vec4i> &lines_same, bool v_or_h);

void repairEdge(Mat &edge);
void fixDirtion(vector<Vec4i> &lines_in, int lineType);
void fixCloseLines(vector<Vec4i> &lines, int lineType, int posi_th);
void fixCloseLinesButSave(vector<Vec4i> &lines, int lineType, int posi_th);

void fixSameLines(vector<Vec4i> &lines, int lineType, int posi_th);
void fixSameLines_2(vector<Vec4i> &lines, int lineType, double over_th, double d_th);

void drawLines(Mat &dst, vector<Vec4i> &lines_v, Scalar color);

Point getCrossPoint(Vec4i lv, Vec4i lh);

int findHorizonLine(vector<Vec4i> lines_h, int up, int dowm, int ls, int rs, vector<Vec4i> &l_result);
int findVerticalLine(vector<Vec4i> lines_v, int up, int dowm, int ts, int ds, vector<Vec4i> &l_result);

int overlap2lines(Vec4i l1, Vec4i l2, int lineType, double th);

void sortLines(vector<Vec4i> &lines_in, bool v_or_h);

void findRectsTwoLines(vector<Vec4i> &lines_v, vector<Vec4i> &lines_h, Mat &dst, vector<Rect> &recBag);

void findRectangles(vector<Vec4i> &lines_v, vector<Vec4i> &lines_h, Mat &dst, vector<Rect> &recBag);
void findRectangles_2(vector<Vec4i> &lines_v, vector<Vec4i> &lines_h, Mat &dst, vector<Rect> &recBag);
void findRectangles_3(vector<Vec4i> &lines_v, vector<Vec4i> &lines_h, Mat &dst, vector<Rect> &recBag);

#endif