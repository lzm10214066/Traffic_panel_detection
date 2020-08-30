#include "featureBow.h"

int readCodebook(string codebookFile, Mat &codebook)
{
	FileStorage fs_codebook;
	fs_codebook.open(codebookFile, FileStorage::READ);
	if (!fs_codebook.isOpened()) { return -1; }
	else
		fs_codebook["codebook"] >> codebook;
	fs_codebook.release();
	return 1;
}

double similarity(Mat &s1, Mat &s2)
{
	double dotC = s1.dot(s2);
	return (dotC) / (norm(s1, NORM_L2)*norm(s2, NORM_L2));
}
double similarityGauss(Mat &s1, Mat &s2)
{
	double dis = norm(s1, s2, NORM_L2);
	return exp(-dis*dis);
}
double simiInterKer(Mat &h1, Mat &h2)
{
	int dim = h1.cols;
	double s = 0;
	float *p1 = (float *)h1.ptr(0);
	float *p2 = (float *)h2.ptr(0);
	for (int i = 0; i < dim; ++i)
	{
		s += min(p1[i], p2[i]);
	}
	return s;
}

void calAve(vector<double> &hist, int patchNum)
{
	for (int i = 0; i < hist.size(); ++i)
	{
		hist[i] = hist[i] / (double)patchNum * 100;
	}
}
void pooling(vector<double> &hist, vector<int> bin_num)
{
	for (int i = 0; i < hist.size(); ++i)
	{
		if (bin_num[i])
		{
			hist[i] = hist[i] / (double)bin_num[i];
		}
	}
}
void vector2mat(vector<vector<double> >& des, Mat &desMat)
{
	int dim = des[0].size();
	for (int i = 0; i < des.size(); ++i)
	{
		Mat one(1, dim, CV_32FC1);
		for (int j = 0; j < dim; ++j)
		{
			one.at<float>(0, j) = des[i][j];
		}
		desMat.push_back(one);
	}
}

void calculateBowSimilarity(Mat &src, vector<double> &hist, Mat& codebook, HOG_opt &hog_opt, double length)
{
	Mat c_src = src.clone();
	double len = c_src.cols > c_src.rows ? c_src.cols : c_src.rows;
	if (len != length) { normSize(c_src, length); }

	Size NormSize(c_src.cols, c_src.rows);
	HOGFeature hog(NormSize, hog_opt);

	vector<vector<double> > descriptor;

	if (c_src.channels() == 3)
	{
		//hog.computeHOGColor_opencv(c_src, descriptor);
		hog.computeHOGColor(c_src, descriptor);
	}
	else
	{
		//hog.computeHOG_opencv(c_src, descriptor);
		//hog.computeHOG(c_src, descriptor);
	}

	Mat desMat;
	vector2mat(descriptor, desMat);
	hist.resize(codebook.rows, 0);
	for (int i = 0; i < desMat.rows; ++i)
	{
		double max_simi = 0;
		int label = 0;
		for (int j = 0; j < codebook.rows; ++j)
		{
			//double simi = similarity(desMat.row(i), codebook.row(j));
			double simi = similarityGauss(desMat.row(i), codebook.row(j));
		//	double simi = simiInterKer(desMat.row(i), codebook.row(j));
			if (simi > max_simi)
			{
				max_simi = simi;
				label = j;
			}
		}
		hist[label] += max_simi;
	}
	//normalize(hist, hist, 1, 0, NORM_L1);
	calAve(hist, desMat.rows);
}
void calculateBowHistHOG(Mat &src, vector<double> &histHOG, Mat& codebook, HOG_opt &hog_opt, double length)
{
	Mat c_src = src.clone();
	double len = c_src.cols > c_src.rows ? c_src.cols : c_src.rows;
	if (len != length) { normSize(c_src, length); }

	Size winSize(c_src.cols, c_src.rows);
	HOGFeature hog(winSize, hog_opt);

	vector<vector<double> > descriptor;
	//hog.computeHOG(c_src, descriptor);
	hog.computeHOG_opencv(c_src, descriptor);

	Mat desMat;
	vector2mat(descriptor, desMat);
	histHOG.resize(codebook.rows*codebook.cols);

	for (int i = 0; i < desMat.rows; ++i)
	{
		double max_simi = 0;
		int label = 0;
		for (int j = 0; j < codebook.rows; ++j)
		{
			//double simi = similarity(desMat.row(i), codebook.row(j));
			double simi = similarityGauss(desMat.row(i), codebook.row(j));
			if (simi > max_simi)
			{
				max_simi = simi;
				label = j;
			}
		}

		for (int j = 0; j < codebook.cols; ++j)
		{
			histHOG[label*codebook.cols + j] += descriptor[i][j];
		}
	}
	calAve(histHOG, descriptor.size());
}
void calculateBowHOG(Mat &src, vector<double> &histHOG, Mat& codebook, HOG_opt &hog_opt, double length)
{
	Mat c_src = src.clone();
	double len = c_src.cols > c_src.rows ? c_src.cols : c_src.rows;
	if (len != length) { normSize(c_src, length); }

	Size winSize(c_src.cols, c_src.rows);
	HOGFeature hog(winSize, hog_opt);

	vector<vector<double> > descriptor;
	hog.computeHOG(c_src, descriptor);
	//hog.computeHOG_opencv(c_src, descriptor);

	Mat desMat;
	vector2mat(descriptor, desMat);
	vector<double> hist_maxSimi(codebook.rows, 0);
	vector<int> hog_label(codebook.rows, 0);

	for (int i = 0; i < desMat.rows; ++i)
	{
		double max_simi = 0;
		int label = 0;
		for (int j = 0; j < codebook.rows; ++j)
		{
			//double simi = similarity(desMat.row(i), codebook.row(j));
			double simi = similarityGauss(desMat.row(i), codebook.row(j));
			if (simi > max_simi)
			{
				max_simi = simi;
				label = j;
			}
		}
		if (max_simi > hist_maxSimi[label])
		{
			hist_maxSimi[label] = max_simi;
			hog_label[label] = i;
		}
	}

	for (int i = 0; i < codebook.rows; ++i)
	{
		for (int j = 0; j < codebook.cols; ++j)
		{
			histHOG.push_back(descriptor[hog_label[i]][j]);
		}
	}
}

void calGrad(Mat &img, Mat &grad,double gradTh)
{
	int wid = img.cols;
	int hei = img.rows;

	Mat cimg = img.clone();
	copyMakeBorder(cimg, cimg, 1, 1, 1, 1, BORDER_REFLECT_101);

	grad.create(img.size(), CV_32FC1);

	for (int i = 1; i < hei + 1; i++)
	{
		uchar *p = cimg.ptr(i), *p_a = cimg.ptr(i + 1), *p_s = cimg.ptr(i - 1);
		float *p_grad = (float*)grad.ptr(i - 1);

		for (int j = 1; j < wid + 1; j++)
		{
			double dx = p[j + 1] - p[j - 1];
			double dy = p_a[j] - p_s[j];

			double grad = sqrt(dx*dx + dy*dy);

			if (grad <= gradTh)
			{
				dx = 0;
				dy = 0;
				grad = 0;
			}
			p_grad[j - 1] = grad;
		}
	}
}
