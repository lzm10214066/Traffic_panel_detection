#include "classifierColor.h"


double colorHist_classify_one(Mat &img, CvSVM &svm, int h_bins,int s_bins)
{

	vector<float> hist(h_bins + s_bins, 0);
	calculateColorHist(img, hist, h_bins, s_bins);

	Mat hist_mat(1, hist.size(), CV_32FC1);
	for (int i = 0; i < hist.size(); ++i)
	{
		hist_mat.at<float>(0, i) = hist[i];
	}
	return(svm.predict(hist_mat,true));
}
double colorHist_classify_one(Mat &img, CvSVM &svm, int h_bins,int s_bins,int nlevels)
{
	vector<float> hist;
	calculateColorHistSpm(img, hist, h_bins,s_bins,nlevels);

	Mat hist_mat(1, hist.size(), CV_32FC1);
	for (int i = 0; i < hist.size(); ++i)
	{
		hist_mat.at<float>(0, i) = hist[i];
	}
	return(svm.predict(hist_mat, true));
}

int colorHist_classify_Mask(Mat &img, Mat &mask,CvSVM &svm, int h_bins, int s_bins,double s_th)
{
	vector<float> hist(h_bins + s_bins, 0);
	calculateColorHistMask(img, mask,hist, h_bins, s_bins);

	Mat hist_mat(1, hist.size(), CV_32FC1);
	for (int i = 0; i < hist.size(); ++i)
	{
		hist_mat.at<float>(0, i) = hist[i];
	}
	double s=-svm.predict(hist_mat, true);
	if (s > s_th)
		return 1;
	return 0;
}
