#include "opencv2\core\core.hpp"
#include "opencv2\ml\ml.hpp"

#include "featureHOG.h"

using namespace cv;
using namespace std;

double edge_classify_one_svm(Mat &img, CvSVM &svm);
void classifyHOG(Mat &img, vector<Rect> &recBag, vector<double> & scores, CvSVM &svm, double s_th);
