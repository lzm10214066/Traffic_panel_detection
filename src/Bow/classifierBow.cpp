#include "classifierBow.h"

float bow_classify_one_svm_similarity(Mat &img, CvSVM &svm, Mat &codebook, HOG_opt &hog_opt,double length)
{
	vector<double> hist;
	calculateBowSimilarity(img, hist, codebook, hog_opt,length);

	Mat hist_mat(1, hist.size(), CV_32FC1);
	for (int i = 0; i < hist.size(); ++i)
	{
		hist_mat.at<float>(0, i) = hist[i];
	}
	return(svm.predict(hist_mat,true));
}
float bow_classify_one_svm_bank(Mat &img, vector<Ptr<CvSVM>> &svm_bank, vector<double> &th_bank, Mat &codebook, HOG_opt &hog_opt, int length, int num_stage)
{
	double ep = 1e-5;
	vector<double> hist;
	calculateBowSimilarity(img, hist, codebook, hog_opt, 64);

	Mat hist_mat(1, hist.size(), CV_32FC1);
	for (int i = 0; i < hist.size(); ++i)
	{
		hist_mat.at<float>(0, i) = hist[i];
	}

	for (int i = 0; i < num_stage && i < svm_bank.size(); ++i)
	{
		double score = -svm_bank[i]->predict(hist_mat, true);
		if (score < th_bank[i]-ep)
			return -1;
	}
	return 1.;
}

void classifyBow(Mat &img, vector<Rect> &recBag, vector<double> & scores, CvSVM &svm,Mat &codebook,double s_th)
{
	HOG_opt hog_opt;

	hog_opt.blockSize = Size(12, 12);
	hog_opt.blockStride = Size(2, 2);
	int cell = hog_opt.blockSize.width / 2;
	hog_opt.cellSize = Size(cell, cell);
	hog_opt.nbins = 8;
	hog_opt.blur = 1;
	hog_opt.gradTh = 0;
	hog_opt.beta = 1;
	hog_opt.orien_dir = 1;  //1为180度范围，无方向,0为有方向
	hog_opt.sw = 2;

	hog_opt.gammaCorrection = true;
	hog_opt.winSigma = hog_opt.blockSize.width / 2;  // -1表示采用默认参数，(blockSize.width+blockSize.height)/8;
	hog_opt.L2HysThreshold = 0.2;

	int gray_f = 0;

	vector<Rect> recBag_filter;
	for (int k = 0; k != recBag.size(); ++k)
	{
		Rect rec = recBag[k];
		Mat recImage(img, rec);
		Mat normImage = recImage.clone();
		normSize(normImage, 64);

		if (gray_f) { cvtColor(normImage, normImage, CV_BGR2GRAY); }
		else { cvtColor(normImage, normImage, CV_BGR2HSV); }

		double score = -bow_classify_one_svm_similarity(normImage, svm,codebook, hog_opt,64);
		if (score > s_th)
		{
			recBag_filter.push_back(rec);
			scores.push_back(score);
		}
	}
	recBag.clear();
	recBag.assign(recBag_filter.begin(), recBag_filter.end());
}

