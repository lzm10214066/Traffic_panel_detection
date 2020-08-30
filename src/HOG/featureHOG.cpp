#include "featureHOG.h"

void calEdge(Mat &img, vector<double> &edgeF)
{
	if (img.channels() == 3) { cvtColor(img, img, CV_BGR2GRAY); }
	img.convertTo(img, CV_64F);

	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;
	/// Gradient X
	//Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
	Sobel(img, grad_x, -1, 1, 0, 3);
	/// Gradient Y
	//Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
	Sobel(img, grad_y, -1, 0, 1, 3);
	/// Total Gradient (approximate)
	Mat grad(img.size(), CV_64FC1);
	for (int i = 0; i < img.rows; ++i)
	{
		double *p_x = grad_x.ptr<double>(i);
		double *p_y = grad_y.ptr<double>(i);
		double *p_g = grad.ptr<double>(i);
		for (int j = 0; j < img.cols; ++j)
		{
			double m = abs(p_x[j]) + abs(p_y[j]);
			p_g[j] = m;
			edgeF.push_back(m);
		}
	}
	grad.convertTo(grad, CV_8UC1);
	normalize(edgeF, edgeF);
}

void calculateHOG(Mat &src, vector<float> &descriptors)
{
	int nbins = 8;
	Size blockSize(16, 16);
	Size blockStride(8, 8);
	Size cellSize(8, 8);
	Size NormSize(48, 48);

	bool gammaCorrection = false;
	double winSigma = -1;
	double L2HysThreshold = 0.2;

	int featureDim = nbins * (blockSize.width / cellSize.width*blockSize.height / cellSize.height) *
		((NormSize.width - blockSize.width) / blockStride.width + 1) * ((NormSize.height - blockSize.height) / blockStride.height + 1);

	HOGDescriptor *hog = new cv::HOGDescriptor(NormSize, blockSize, blockStride, cellSize, nbins, 1, winSigma,
		HOGDescriptor::L2Hys, L2HysThreshold, gammaCorrection);

	Mat src_gray;
	if (src.channels() != 1) { cvtColor(src, src_gray, CV_BGR2GRAY); }
	else src_gray = src.clone();
	
	Mat imgNorm(NormSize, CV_8UC1);
	resize(src_gray, imgNorm, NormSize);

	hog->compute(imgNorm, descriptors, Size(1, 1), Size(0, 0));
}