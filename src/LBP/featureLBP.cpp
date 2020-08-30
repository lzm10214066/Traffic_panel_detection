#include "featureLBP.h"

void calLBPMultiScale(Mat &src, vector<double> &hist, LBP_Option &lbp_opt)
{
	Mat img;
	src.convertTo(img, CV_64F);

	int num_s = lbp_opt.radius.size();
	for (int i = 0; i < num_s; ++i)
	{
		LBP lbp(lbp_opt.samples[i], lbp_opt.type);
		lbp.calcLBP(img, lbp_opt.radius[i], false);
		vector<double> hist_one = lbp.calcHist().getHist();
		hist.insert(hist.end(), hist_one.begin(), hist_one.end());
	}
}
void calLBPMultiScaleChannels(Mat &src, vector<double> &hist, LBP_Option &lbp_opt)
{
	cvtColor(src, src, CV_BGR2HSV);
	CV_Assert(src.channels() == 3);
	vector<Mat> channels;
	split(src, channels);
	calLBPMultiScale(channels.at(0), hist, lbp_opt);
	calLBPMultiScale(channels.at(1), hist, lbp_opt);
	calLBPMultiScale(channels.at(2), hist, lbp_opt);
}

void calLBPHFSM_MultiScale(Mat &src, vector<double> &hist, LBP_Option &lbp_opt)
{
	Mat img;
	src.convertTo(img, CV_64F);

	int num_s = lbp_opt.radius.size();
	for (int i = 0; i < num_s; ++i)
	{
		LBP lbp(lbp_opt.samples[i], lbp_opt.type);
		lbp.calcLBP(img, lbp_opt.radius[i], false);
		vector<double> hist_one = lbp.calcHist().getHist();
		hist.insert(hist.end(), hist_one.begin(), hist_one.end());

		lbp.calcLBP_M(img, lbp_opt.radius[i], false);
		hist_one = lbp.calcHist().getHist();
		hist.insert(hist.end(), hist_one.begin(), hist_one.end());
	}
}
void calLBPHFSM_MultiScaleChannels(Mat &src, vector<double> &hist, LBP_Option &lbp_opt)
{
	cvtColor(src, src, CV_BGR2HSV);
	CV_Assert(src.channels() == 3);
	vector<Mat> channels;
	split(src, channels);
	calLBPHFSM_MultiScale(channels.at(0), hist, lbp_opt);
	calLBPHFSM_MultiScale(channels.at(1), hist, lbp_opt);
	calLBPHFSM_MultiScale(channels.at(2), hist, lbp_opt);
}


void calLBPHFSMC_MultiScale(Mat &src, vector<double> &hist, LBP_Option &lbp_opt)
{
	CV_Assert(lbp_opt.type == LBP_MAPPING_HF);
	CV_Assert(src.channels() == 1);
	Mat img;
	src.convertTo(img, CV_64F);
	double meanG = mean(src)[0];

	Mat c_map,c_map_inv;
	threshold(src, c_map, meanG, 1, THRESH_BINARY);
	threshold(src, c_map_inv, meanG, 1, THRESH_BINARY_INV);

	int num_s = lbp_opt.radius.size();
	for (int i = 0; i < num_s; ++i)
	{
		LBP lbp(lbp_opt.samples[i], lbp_opt.type);
		lbp.calcLBP(img, lbp_opt.radius[i], true);
		vector<double> hist_one = lbp.calcHist(c_map).getHist();
		hist.insert(hist.end(), hist_one.begin(), hist_one.end());
		hist_one = lbp.calcHist(c_map_inv).getHist();
		hist.insert(hist.end(), hist_one.begin(), hist_one.end());

		lbp.calcLBP_M(img, lbp_opt.radius[i], true);
		hist_one = lbp.calcHist(c_map).getHist();
		hist.insert(hist.end(), hist_one.begin(), hist_one.end());

		hist_one = lbp.calcHist(c_map_inv).getHist();
		hist.insert(hist.end(), hist_one.begin(), hist_one.end());
	}
}

void hist3D(Mat &s_map, Mat &m_map, Mat &c_map, vector<double> &hist)
{
	hist.resize(10 * 10 * 2,0);
	int h = s_map.rows;
	int w = s_map.cols;
	for (int i = 0; i < h; ++i)
	{
		uchar *p_s = s_map.ptr(i);
		uchar *p_m = m_map.ptr(i);
		uchar *p_c = c_map.ptr(i);

		for (int j = 0; j < w; ++j)
		{
			uchar k = p_c[j];
			uchar m = p_s[j];
			uchar n = p_m[j];

			hist[k * 100 + m * 10 + n]++;
		}
	}
	normalize(hist, hist, 1, 0, NORM_L1);
}
void calCLBP_MultiScale(Mat &src, vector<double> &hist, LBP_Option &lbp_opt)
{
	CV_Assert(lbp_opt.type==LBP_MAPPING_RIU2);
	CV_Assert(src.channels() == 1);
	Mat img;
	src.convertTo(img, CV_64F);
	double meanG = mean(src)[0];
	int num_s = lbp_opt.radius.size();
	for (int i = 0; i < num_s; ++i)
	{
		LBP lbp(lbp_opt.samples[i], lbp_opt.type);
		lbp.calcLBP(img, lbp_opt.radius[i], false);
		Mat s_map = lbp.getLBPImage();

		lbp.calcLBP_M(img, lbp_opt.radius[i], false);
		Mat m_map = lbp.getLBPImage();

		Mat c_map;
		threshold(src, c_map, meanG, 1, THRESH_BINARY);

		vector<double> hist_one;
		hist3D(s_map, m_map, c_map, hist_one);
		hist.insert(hist.end(), hist_one.begin(), hist_one.end());
	}
}
void calCLBPMultiScaleChannels(Mat &src, vector<double> &hist, LBP_Option &lbp_opt)
{
	CV_Assert(src.channels() == 3);
	cvtColor(src, src, CV_BGR2HSV);
	
	vector<Mat> channels;
	split(src, channels);
	calCLBP_MultiScale(channels.at(0), hist, lbp_opt);
	calCLBP_MultiScale(channels.at(1), hist, lbp_opt);
	calCLBP_MultiScale(channels.at(2), hist, lbp_opt);
}