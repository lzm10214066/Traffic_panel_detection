#ifndef _GRADIENTDEC_H
#define _GRADIENTDEC_H

#include "opencv2\core\core.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\objdetect\objdetect.hpp"

using namespace cv;

struct HOG_opt
{
	int nbins;		// direction resolution ()
	Size blockSize;
	Size blockStride;
	Size cellSize;
	int blur;
	double gradTh;
	double sw;
	double beta;
	int orien_dir;

	bool gammaCorrection;
	double winSigma;  // -1表示采用默认参数，(blockSize.width+blockSize.height)/8;
	double L2HysThreshold;
};


class HOGFeature
{
	Size NormSize;
	int nbins;		// direction resolution ()
	Size blockSize;
	Size blockStride;
	Size cellSize;

	bool gammaCorrection;
	double winSigma;  // -1表示采用默认参数，(blockSize.width+blockSize.height)/8;
	double L2HysThreshold;

	vector<vector<double> > gaussM;

	int featureDim;
	int blur;
	double gradTh;
	double sw;
	double beta;
	int orien_dir;

	double *wight_map[2];
	uchar *wight_bin[2];
	
	// Blurring of continuous features
	void fblurOdd(Size NormSize, vector<vector<double> >& ftr);
	void fblurEven(Size NormSize, vector<vector<double> >& ftr);

	void calGradRgb(Mat &img, int dn);
	void calGrad(Mat &img, int dn);
	void SobelDec(Mat &img, int dn);

	void gradSobelHsv(Mat &img, int dn);
	void gradSobelPoint(Mat &img, int i,int j,double *dx, double *dy);
	void gradDir(int, int, int, double, double,int dn);

public:
	Mat grad;

	HOGFeature(Size &imgSize, HOG_opt &hog_opt);
	~HOGFeature();

	void maskBuild(vector<vector<double>> &gaussMask, double beta);

	void computeHOG(Mat &img, vector<vector<double> >& ftr);
	void computeHOG_opencv(Mat &img, vector<vector<double> >& ftr);
	void computeHOGThres(Mat &img, vector<vector<double> >& ftr);
	void computeHOGColor(Mat &img, vector<vector<double> >& ftr);
	void computeHOGColor_opencv(Mat &img, vector<vector<double>> &ftr);
};

#endif
