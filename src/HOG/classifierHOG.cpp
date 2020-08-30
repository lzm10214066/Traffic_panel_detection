#include "classifierHOG.h"
#include "../utility.h"

double HOG_classify_one_svm(Mat &img, CvSVM &svm)
{
	vector<float> hist;
	calculateHOG(img, hist);

	Mat hist_mat(1,hist.size(),CV_32FC1);
	for (int i = 0; i < hist.size(); ++i)
	{
		hist_mat.at<float>(0, i) = hist[i];
	}
	return(svm.predict(hist_mat,true));
}


void classifyHOG(Mat &img, vector<Rect> &recBag, vector<double> & scores, CvSVM &svm, double s_th)
{
	vector<Rect> recBag_ex(recBag);
	rect_extend(recBag_ex, 1.2, img.cols, img.rows);

	vector<Rect> recBag_filter;
	for (int k = 0; k != recBag.size(); ++k)
	{
		Rect rec = recBag[k];
		Rect rec_ex = recBag_ex[k];
		Mat recImage(img, rec_ex);
	
		double score = -HOG_classify_one_svm(recImage, svm);
		if (score > s_th)
		{
			recBag_filter.push_back(rec);
			scores.push_back(score);
		}
	}
	recBag.clear();
	recBag.assign(recBag_filter.begin(), recBag_filter.end());
}
