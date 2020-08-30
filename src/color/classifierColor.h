#include "opencv2\core\core.hpp"
#include "opencv2\ml\ml.hpp"

#include "colorFeature.h"

using namespace cv;
using namespace std;

double colorHist_classify_one(Mat &img, CvSVM &svm, int h_bins, int s_bins);
double colorHist_classify_one(Mat &img, CvSVM &svm, int h_bins, int s_bins, int nlevels);
int colorHist_classify_Mask(Mat &img, Mat &mask, CvSVM &svm, int h_bins, int s_bins, double s_th);
