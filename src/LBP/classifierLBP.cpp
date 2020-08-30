#include "classifierrLBP.h"

using namespace lbp;
double lbp_classify_one_svm(Mat &img, CvSVM &svm,LBP_Option lbp_opt)
{
	vector<double> hist;
	if (img.channels() == 1) 
	{ 
		//calLBPMultiScale(img, hist, lbp_opt);
		//calLBPHFSM_MultiScale(img, hist, lbp_opt); 
		calCLBP_MultiScale(img, hist, lbp_opt);
		//calLBPHFSMC_MultiScale(img, hist, lbp_opt);
	}
	else 
	{ 
		//calLBPMultiScaleChannels(img, hist, lbp_opt); 
		//calLBPHFSM_MultiScaleChannels(img, hist, lbp_opt);
		calCLBPMultiScaleChannels(img, hist, lbp_opt);
	}
	
	Mat hist_mat(1,hist.size(),CV_32FC1);
	for (int i = 0; i < hist.size(); ++i)
	{
		hist_mat.at<float>(0, i) = hist[i];
	}
	return(svm.predict(hist_mat,true));
}

void classifyLBP(Mat &img, vector<Rect> &recBag, vector<double> & scores,CvSVM &svm,double s_th)
{
	if (img.channels() == 3) { cvtColor(img, img, CV_BGR2GRAY); }
	
	LBP_Option lbp_opt;
	lbp_opt.type = LBP_MAPPING_RIU2;
	lbp_opt.radius.push_back(1); lbp_opt.radius.push_back(3); lbp_opt.radius.push_back(5);
	lbp_opt.samples.push_back(8); lbp_opt.samples.push_back(8); lbp_opt.samples.push_back(8);

	vector<Rect> recBag_filter;
	for (int k = 0; k != recBag.size(); ++k)
	{
		Rect rec = recBag[k];
		Mat recImage(img, rec);
		Mat normImage = recImage.clone();
		normSize(normImage,64);

		double score = -lbp_classify_one_svm(normImage, svm, lbp_opt);
		if (score > s_th)
		{
			recBag_filter.push_back(rec);
			scores.push_back(score);
		}
	}
	recBag.clear();
	recBag.assign(recBag_filter.begin(), recBag_filter.end());
}


int classifyLBP_RecImg(Mat &img, CvSVM &svm, double s_th)
{
	if (img.channels() == 3) { cvtColor(img, img, CV_BGR2GRAY); }

	LBP_Option lbp_opt;
	lbp_opt.type = LBP_MAPPING_RIU2;
	lbp_opt.radius.push_back(1); lbp_opt.radius.push_back(3); lbp_opt.radius.push_back(5);
	lbp_opt.samples.push_back(8); lbp_opt.samples.push_back(8); lbp_opt.samples.push_back(8);

	Mat normImage = img.clone();
	normSize(normImage, 64);

	double score = -lbp_classify_one_svm(normImage, svm, lbp_opt);
	if (score > s_th)
	{
		return 1;
	}
	return 0;
}
