#include "colorFeature.h"

bool comp(double i, double j)
{
	return i > j;
}
double simiInterKer(vector<float> &h1, vector<float> &h2)
{
	int dim = h1.size();
	double s = 0;
	for (int i = 0; i < dim; ++i)
	{
		s += min(h1[i], h2[i]);
	}
	return s;
}

double simiGauss(vector<float> &h1, vector<float> &h2)
{
	int dim = h1.size();
	double d = 0;
	for (int i = 0; i < dim; ++i)
	{
		d += (h1[i] - h2[i])*(h1[i] - h2[i]);
	}
	return exp(-d);
}

void calculateColorHist(Mat &src, vector<float> &hist, int h_bins, int s_bins)
{
	int nRows = src.rows;
	int nCols = src.cols;
	int pixels = nRows*nCols;

	Mat src_hsv;
	cvtColor(src, src_hsv, CV_BGR2HSV);
	vector<Mat> hsv;
	split(src_hsv, hsv);

	Mat h = hsv.at(1);
	Mat s = hsv.at(2);

	int h_bin_value = 180 / h_bins;
	int s_bin_value = 256 / s_bins;

	uchar* p_h;
	uchar *p_s;

	hist.resize(h_bins + s_bins, 0);
	for (int i = 0; i < nRows; ++i)
	{
		p_h = h.ptr<uchar>(i);
		p_s = s.ptr<uchar>(i);

		for (int j = 0; j < nCols; ++j)
		{
			p_h[j] /= h_bin_value;
			p_s[j] /= s_bin_value;

			hist[p_h[j]]++;
			hist[p_s[j] + h_bins]++;
		}
	}

	for (int i = 0; i < h_bins + s_bins; ++i)
	{
		hist[i] /= pixels;
	}
}
void calculateColorHistSpm(Mat &src, vector<float> &hist, int h_bins, int s_bins,int nlevels)
{
	int nRows = src.rows;
	int nCols = src.cols;
	int pixels = nRows*nCols;

	for (int scale = 1; scale <= nlevels; ++scale)
	{
		int dw = floor((double)nCols / scale);
		int dh = floor(double(nRows) / scale);
		int tot = dw*dh;

		int offsetx = (nCols%scale - 1) / 2;
		int offsety = (nRows%scale - 1) / 2;

		for (int m = 0; m < scale; ++m)
		{
			for (int n = 0; n < scale; ++n)
			{
				int y = m*dh + offsety;
				int x = n*dw + offsetx;
				Rect r(x, y, dw, dh);
				Mat recImg(src, r);
				vector<float> recHist(h_bins + s_bins, 0);
				calculateColorHist(recImg, recHist, h_bins, s_bins);
				for (int k = 0; k < recHist.size(); ++k)
				{
					hist.push_back(recHist[k]);
				}
			}
		}

	}
}

void findTextPixel(Mat &src, Mat &mask)
{
	Mat gray;
	cvtColor(src, gray, CV_BGR2GRAY);
	int blockSize = 11;
	int constValue = 0;
	adaptiveThreshold(gray, mask, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, blockSize, constValue);
}

void calculateColorHistMask(Mat &src, Mat &mask,vector<float> &hist, int h_bins, int s_bins)
{
	int nRows = src.rows;
	int nCols = src.cols;
	int pixels = nRows*nCols;

	Mat src_hsv;
	cvtColor(src, src_hsv, CV_BGR2HSV);
	vector<Mat> hsv;
	split(src_hsv, hsv);

	Mat h = hsv.at(0);
	Mat s = hsv.at(1);

	int h_bin_value = 180 / h_bins;
	int s_bin_value = 256 / s_bins;

	uchar *p_m;
	uchar* p_h;
	uchar *p_s;
	int num = 0;
	int num_b = 0;
	int bins = h_bins + s_bins;
	hist.resize(bins, 0);

	for (int i = 0; i < nRows; ++i)
	{
		p_m = mask.ptr<uchar>(i);
		p_h = h.ptr<uchar>(i);
		p_s = s.ptr<uchar>(i);

		for (int j = 0; j < nCols; ++j)
		{
			p_h[j] /= h_bin_value;
			p_s[j] /= s_bin_value;
			if (p_m[j])
			{
				hist[p_h[j]]++;
				hist[p_s[j] + h_bins]++;
				num++;
			}
		}
	}
	for (int i = 0; i < bins; ++i)
	{
		hist[i] /= num;
	}
	for (int i = bins; i < hist.size(); ++i)
	{
		hist[i] /= num_b;
	}
/////////////////////////////////////////////////////////////////////////
}
