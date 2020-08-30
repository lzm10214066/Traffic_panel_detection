#ifndef _IMAGEREAD_H
#define  _IMAGEREAD_H

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"

using namespace cv;

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <direct.h>

using namespace std;

int readImageFile(string &imageFile, vector<string> &pathOfImage);

void rect2image(Mat &image, vector<Rect> recBag, string imagePath, string &folder, int type);
void rectsSaveAsImage(Mat &image, vector<Rect> recBag, string imagePath, string &folder);

void saveErrImg(vector<string> &error_images, string &folder);


#endif