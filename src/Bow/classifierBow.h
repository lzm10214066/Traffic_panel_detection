#include "opencv2\core\core.hpp"
#include "opencv2\ml\ml.hpp"

#include "featureBow.h"
#include "../utility.h"

using namespace cv;
using namespace std;

float bow_classify_one_svm_similarity(Mat &img, CvSVM &svm, Mat &codebook, HOG_opt &hog_opt, double length);
float bow_classify_one_svm_bank(Mat &img, vector<Ptr<CvSVM>> &svm_bank, vector<double> &th_bank,
	Mat &codebook, HOG_opt &hog_opt, int length, int num_stage);

void classifyBow(Mat &img, vector<Rect> &recBag, vector<double> & scores, CvSVM &svm, Mat &codebook, double s_th);


