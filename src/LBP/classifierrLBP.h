#include "opencv2\core\core.hpp"
#include "opencv2\ml\ml.hpp"

#include "featureLBP.h"
#include "../utility.h"

using namespace cv;
using namespace std;

double lbp_classify_one_svm(Mat &img, CvSVM &svm, LBP_Option lbp_opt);
void classifyLBP(Mat &img, vector<Rect> &recBag, CvSVM &svm, double s_th);
void classifyLBP(Mat &img, vector<Rect> &recBag, vector<double> & scores, CvSVM &svm, double s_th);
int classifyLBP_RecImg(Mat &img, CvSVM &svm, double s_th);

