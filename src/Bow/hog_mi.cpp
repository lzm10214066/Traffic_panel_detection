#include "hog_mi.h"

void HOGFeature::maskBuild(vector<vector<double>> &gaussM, double beta)
{
	vector<vector<double> > gaussMask(blockSize.height, vector<double>(blockSize.width));

	double sigma2;
	int win = gaussMask[0].size();
	int r = win / 2;

	sigma2 = (double)(r*r / (beta*beta));
	double xc = r - 0.5;
	double yc = r - 0.5;

	for (int i = 0; i < win; i++)
	{
		for (int j = 0; j < win; j++)
		{
			double d2 = (i - yc)*(i - yc) + (j - xc)*(j - xc);
			double gauss = exp(-d2 / (2 * sigma2));
			gaussMask[i][j] = gauss;
		}
	}
	gaussM = gaussMask;
}

HOGFeature::HOGFeature(Size &imgSize, HOG_opt &hog_opt)
{
	NormSize = imgSize;
	blockSize = hog_opt.blockSize;
	blockStride = hog_opt.blockStride;
	cellSize = hog_opt.cellSize;
	nbins = hog_opt.nbins;

	blur = hog_opt.blur;
	gradTh = hog_opt.gradTh;
	sw = hog_opt.sw;
	orien_dir = hog_opt.orien_dir;

	/*for opencv hog*/
	gammaCorrection = hog_opt.gammaCorrection;
	winSigma = hog_opt.winSigma;
	L2HysThreshold = hog_opt.L2HysThreshold;

	wight_map[0] = new double[NormSize.area()];
	wight_map[1] = new double[NormSize.area()];
	wight_bin[0] = new uchar[NormSize.area()];
	wight_bin[1] = new uchar[NormSize.area()];

	featureDim = nbins * (blockSize.width / cellSize.width*blockSize.height / cellSize.height) *
		((NormSize.width - blockSize.width) / blockStride.width + 1) * ((NormSize.height - blockSize.height) / blockStride.height + 1);

	beta = hog_opt.beta;
	maskBuild(gaussM, beta);
}
HOGFeature::~HOGFeature()
{
	delete wight_bin[0];
	delete wight_bin[1];
	delete wight_map[0];
	delete wight_map[1];
}
// HOG feature by block stride
void HOGFeature::computeHOG(Mat &img, vector<vector<double> >& ftr)
{
	// direction decomposition of Sobel gradient
	int modx = (NormSize.width - blockSize.width) % blockStride.width;
	int mody = (NormSize.height - blockSize.height) % blockStride.height;

	int offsetx = cvFloor((modx*1.0) / 2.);
	int offsety = cvFloor((mody*1.0) / 2.);

	if (img.channels() == 1) { calGrad(img, nbins); }
	else { calGradRgb(img, nbins); }

	int m_oneblock = nbins*blockSize.width / cellSize.width*blockSize.height / cellSize.height;

	if (blur == 0)
	{
		vector<double> grd_hist(m_oneblock, 0);

		int count_block_h = (NormSize.height - blockSize.height) / blockStride.height + 1;
		int count_block_w = (NormSize.width - blockSize.width) / blockStride.width + 1;
		int count_cell_h = blockSize.height / cellSize.height;
		int count_cell_w = blockSize.width / cellSize.width;

		for (int i = 0; i < count_block_h; ++i)
		{
			for (int j = 0; j < count_block_w; ++j)
			{
				grd_hist.assign(m_oneblock, 0);

				for (int m = 0; m < count_cell_h; ++m)
					for (int n = 0; n < count_cell_w; ++n)
					{
						int yl = i*blockStride.height + m*cellSize.height + offsety;
						int yu = yl + cellSize.height;
						int xl = j*blockStride.width + n*cellSize.width + offsetx;
						int xu = xl + cellSize.width;

						for (int y = yl; y < yu; ++y)
							for (int x = xl; x < xu; ++x)
							{
								int k = wight_bin[0][y*NormSize.width + x];
								grd_hist[k + (m*count_cell_w + n)*nbins] += wight_map[0][y*NormSize.width + x];

								k = wight_bin[1][y*NormSize.width + x];
								grd_hist[k + (m*count_cell_w + n)*nbins] += wight_map[1][y*NormSize.width + x];
							}
					}
				cv::normalize(grd_hist, grd_hist);
				ftr.push_back(grd_hist);
			}
		}
	}
	else
	{
		fblurEven(NormSize, ftr);
	}
}
void HOGFeature::computeHOG_opencv(Mat &img, vector<vector<double> >& ftr)
{

	Size winSize(img.cols, img.rows);
	int modx = 0, mody = 0;
	modx = (winSize.width - blockSize.width) % blockStride.width;
	if (modx != 0)
	{
		winSize.width -= modx;
	}
	mody = (winSize.height - blockSize.height) % blockStride.height;
	if (mody != 0)
	{
		winSize.height -= mody;
	}
	Mat imgROI(img, Rect(modx / 2, mody / 2, winSize.width, winSize.height));
	HOGDescriptor hog(winSize, blockSize, blockStride, cellSize, nbins, 1, winSigma,
		HOGDescriptor::L2Hys, L2HysThreshold, gammaCorrection);

	hog.compute(imgROI, ftr, Size(8, 8), Size(0, 0));
}
void HOGFeature::computeHOGThres(Mat &img, vector<vector<double> >& ftr)
{
	// direction decomposition of Sobel gradient
	int modx = (NormSize.width - blockSize.width) % blockStride.width;
	int mody = (NormSize.height - blockSize.height) % blockStride.height;

	int offsetx = cvFloor((modx*1.0) / 2.);
	int offsety = cvFloor((mody*1.0) / 2.);

	if (img.channels() == 1) { calGrad(img, nbins); }
	else { calGradRgb(img, nbins); }

	int m_oneblock = nbins*blockSize.width / cellSize.width*blockSize.height / cellSize.height;
	int pixels_b = blockSize.area();
	if (blur == 0)
	{
		int count_block_h = (NormSize.height - blockSize.height) / blockStride.height + 1;
		int count_block_w = (NormSize.width - blockSize.width) / blockStride.width + 1;
		int count_cell_h = blockSize.height / cellSize.height;
		int count_cell_w = blockSize.width / cellSize.width;

		for (int i = 0; i < count_block_h; ++i)
		{
			for (int j = 0; j < count_block_w; ++j)
			{
				vector<double> grd_hist(m_oneblock, 0);
				double g_meanb = 0;
				double s_mb = 0;

				int ylb = i*blockStride.height;
				int yub = ylb + blockSize.height;
				int xlb = j*blockStride.width;
				int xub = xlb + blockSize.width;

				for (int y = ylb; y < yub; ++y)
					for (int x = xlb; x < xub; ++x)
					{
						s_mb += grad.at<float>(y, x);
					}
				g_meanb = s_mb / pixels_b*0.5;

				for (int m = 0; m < count_cell_h; ++m)
					for (int n = 0; n < count_cell_w; ++n)
					{
						int yl = i*blockStride.height + m*cellSize.height + offsety;
						int yu = yl + cellSize.height;
						int xl = j*blockStride.width + n*cellSize.width + offsetx;
						int xu = xl + cellSize.width;

						/*		double s_m = 0;
						for (int y = yl; y < yu; ++y)
						for (int x = xl; x < xu; ++x)
						{
						s_m += grad.at<float>(y, x);
						}

						double g_mean = s_m / pixels_b;*/

						for (int y = yl; y < yu; ++y)
							for (int x = xl; x < xu; ++x)
							{
								if (grad.at<float>(y, x) < g_meanb)
									continue;
								int k = wight_bin[0][y*NormSize.width + x];
								grd_hist[k + (m*count_cell_w + n)*nbins] += wight_map[0][y*NormSize.width + x];

								k = wight_bin[1][y*NormSize.width + x];
								grd_hist[k + (m*count_cell_w + n)*nbins] += wight_map[1][y*NormSize.width + x];
							}
					}
				cv::normalize(grd_hist, grd_hist);
				ftr.push_back(grd_hist);
			}
		}
	}
	else
	{
		fblurEven(NormSize, ftr);
	}
}
void HOGFeature::computeHOGColor(Mat &img, vector<vector<double>> &ftr)
{
	CV_Assert(img.channels() == 3);
	vector<Mat> channels;
	split(img, channels);
	Mat one = channels.at(0);
	Mat two = channels.at(1);
	Mat three = channels.at(2);

	vector<vector<double>> hog_one;
	computeHOG(one, hog_one);
	vector<vector<double>> hog_two;
	computeHOG(two, hog_two);
	vector<vector<double>> hog_three;
	computeHOG(three, hog_three);

	for (int i = 0; i < hog_one.size(); ++i)
	{
		vector<double> hog_channles;
		for (int j = 0; j < hog_one[0].size(); ++j)
		{
			hog_channles.push_back(hog_one[i][j]);
			hog_channles.push_back(hog_two[i][j]);
			hog_channles.push_back(hog_three[i][j]);
		}
		ftr.push_back(hog_channles);
	}
}

void HOGFeature::computeHOGColor_opencv(Mat &img, vector<vector<double>> &ftr)
{
	CV_Assert(img.channels() == 3);
	vector<Mat> channels;
	split(img, channels);
	Mat one = channels.at(0);
	Mat two = channels.at(1);
	Mat three = channels.at(2);

	vector<vector<double>> hog_one;
	computeHOG_opencv(one, hog_one);
	vector<vector<double>> hog_two;
	computeHOG_opencv(two, hog_two);
	vector<vector<double>> hog_three;
	computeHOG_opencv(three, hog_three);

	for (int i = 0; i < hog_one.size(); ++i)
	{
		vector<double> hog_channles;
		for (int j = 0; j < hog_one[0].size(); ++j)
		{
			hog_channles.push_back(hog_one[i][j]);
			hog_channles.push_back(hog_two[i][j]);
			hog_channles.push_back(hog_three[i][j]);
		}
		ftr.push_back(hog_channles);
	}
}

void HOGFeature::calGradRgb(Mat &img, int dn)
{
	CV_Assert(img.channels() == 3);
	int wid = img.cols;
	int hei = img.rows;

	memset(wight_bin[0], 0, sizeof(uchar)*wid*hei);
	memset(wight_bin[1], 0, sizeof(uchar)*wid*hei);

	memset(wight_map[0], 0, sizeof(double)*wid*hei);
	memset(wight_map[1], 0, sizeof(double)*wid*hei);

	vector<Mat> bgr;
	split(img, bgr);
	Mat b = bgr[0];
	Mat g = bgr[1];
	Mat r = bgr[2];

	copyMakeBorder(b, b, 1, 1, 1, 1, BORDER_REFLECT_101);
	copyMakeBorder(g, g, 1, 1, 1, 1, BORDER_REFLECT_101);
	copyMakeBorder(r, r, 1, 1, 1, 1, BORDER_REFLECT_101);

	grad.create(img.size(), CV_32FC1);

	for (int i = 1; i < hei + 1; i++)
	{
		uchar *pb = b.ptr(i), *pb_a = b.ptr(i + 1), *pb_s = b.ptr(i - 1);
		uchar *pg = g.ptr(i), *pg_a = g.ptr(i + 1), *pg_s = g.ptr(i - 1);
		uchar *pr = r.ptr(i), *pr_a = r.ptr(i + 1), *pr_s = r.ptr(i - 1);

		float *p_grad = (float*)grad.ptr(i - 1);

		for (int j = 1; j < wid + 1; j++)
		{
			double b_gradx = (pb[j + 1] - pb[j - 1]);
			double b_grady = (pb_a[j] - pb_s[j]);
			double b_grad = sqrt(b_gradx*b_gradx + b_grady*b_grady);

			double dx = b_gradx;
			double dy = b_grady;
			double grad = b_grad;

			double g_gradx = pg[j + 1] - pg[j - 1];
			double g_grady = pg_a[j] - pg_s[j];
			double g_grad = sqrt(g_gradx*g_gradx + g_grady*g_grady);

			if (g_grad > grad)
			{
				dx = g_gradx;
				dy = g_grady;
				grad = g_grad;
			}

			double r_gradx = pr[j + 1] - pr[j - 1];
			double r_grady = pr_a[j] - pr_s[j];
			double r_grad = sqrt(r_gradx*r_gradx + r_grady*r_grady);

			if (r_grad > grad)
			{
				dx = r_gradx;
				dy = r_grady;
				grad = r_grad;
			}

			if (grad <= gradTh)
			{
				dx = 0;
				dy = 0;
				grad = 0;
			}
			p_grad[j - 1] = grad;
			if (dx == 0 && dy == 0)
				continue;
			gradDir(wid, i - 1, j - 1, dx, dy, dn);
		}
	}
}
void HOGFeature::calGrad(Mat &img, int dn)
{
	if (img.channels() != 1) { cvtColor(img, img, CV_BGR2GRAY); }
	int wid = img.cols;
	int hei = img.rows;

	memset(wight_bin[0], 0, sizeof(uchar)*wid*hei);
	memset(wight_bin[1], 0, sizeof(uchar)*wid*hei);

	memset(wight_map[0], 0, sizeof(double)*wid*hei);
	memset(wight_map[1], 0, sizeof(double)*wid*hei);

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

			if (dx == 0 && dy == 0)
				continue;
			gradDir(wid, i - 1, j - 1, dx, dy, dn);
		}
	}
}

void HOGFeature::gradSobelPoint(Mat &img, int i, int j, double *dx, double *dy)
{
	double sobelw = sw;	// sobel weight, 2 or 1.414
	int d[8];

	uchar *p = img.ptr(i);
	uchar *p_a = img.ptr(i + 1);
	uchar *p_s = img.ptr(i - 1);


	d[0] = p[j + 1];		// east
	d[2] = p_a[j];	// south
	d[4] = p[j - 1];		// west
	d[6] = p_s[j];	// north
	d[1] = p_a[j + 1];	// south-east
	d[3] = p_a[j - 1];		// south-west
	d[5] = p_s[j - 1];		// north-west
	d[7] = p_s[j + 1];		// north-east

	*dx = double(d[7] + sobelw*d[0] + d[1] - d[3] - sobelw*d[4] - d[5]);
	*dy = double(d[1] + sobelw*d[2] + d[3] - d[5] - sobelw*d[6] - d[7]);
}
void HOGFeature::gradSobelHsv(Mat &img, int dn)
{
	CV_Assert(img.channels() == 3);
	Mat hsvMat;
	cvtColor(img, hsvMat, CV_BGR2HSV);

	int wid = img.cols;
	int hei = img.rows;

	memset(wight_bin[0], 0, sizeof(uchar)*wid*hei);
	memset(wight_bin[1], 0, sizeof(uchar)*wid*hei);

	memset(wight_map[0], 0, sizeof(double)*wid*hei);
	memset(wight_map[1], 0, sizeof(double)*wid*hei);

	vector<Mat> hsv;
	split(hsvMat, hsv);
	Mat h = hsv[0];
	Mat s = hsv[1];
	Mat v = hsv[2];

	double sobelw = sw;	// sobel weight, 2 or 1.414
	copyMakeBorder(h, h, 1, 1, 1, 1, BORDER_REFLECT_101);
	copyMakeBorder(s, s, 1, 1, 1, 1, BORDER_REFLECT_101);
	copyMakeBorder(v, v, 1, 1, 1, 1, BORDER_REFLECT_101);

	for (int i = 1; i < hei + 1; i++)
	{
		for (int j = 1; j < wid + 1; j++)
		{
			double h_gradx, h_grady;
			gradSobelPoint(h, i, j, &h_gradx, &h_grady);
			h_gradx *= 255. / 180;
			h_grady *= 255. / 180;
			double h_grad = h_gradx*h_gradx + h_grady*h_grady;

			double dx = h_gradx;
			double dy = h_grady;
			double grad = h_grad;

			double s_gradx, s_grady;
			gradSobelPoint(s, i, j, &s_gradx, &s_grady);
			double s_grad = s_gradx*s_gradx + s_grady*s_grady;

			if (s_grad > grad)
			{
				dx = s_gradx;
				dy = s_grady;
				grad = s_grad;
			}

			double v_gradx, v_grady;
			gradSobelPoint(v, i, j, &v_gradx, &v_grady);
			double v_grad = v_gradx*v_gradx + v_grady*v_grady;

			if (v_grad > grad)
			{
				dx = v_gradx;
				dy = v_grady;
				grad = v_grad;
			}

			if (sqrt(grad) <= gradTh)
			{
				dx = 0;
				dy = 0;
			}

			if (dx == 0 && dy == 0)
				continue;
			gradDir(wid, i - 1, j - 1, dx, dy, dn);
		}
	}
}
void HOGFeature::SobelDec(Mat &img, int dn)
{
	if (img.channels() != 1) { cvtColor(img, img, CV_BGR2GRAY); }
	int i, j;
	int d[8];
	double dx, dy;
	int wid = img.cols;
	int hei = img.rows;

	memset(wight_bin[0], 0, sizeof(uchar)*wid*hei);
	memset(wight_bin[1], 0, sizeof(uchar)*wid*hei);

	memset(wight_map[0], 0, sizeof(double)*wid*hei);
	memset(wight_map[1], 0, sizeof(double)*wid*hei);

	double sobelw = sw;	// sobel weight, 2 or 1.414
	Mat timg = img.clone();
	copyMakeBorder(timg, timg, 1, 1, 1, 1, BORDER_REFLECT_101);

	for (i = 1; i < hei + 1; i++)
	{
		uchar *p = timg.ptr(i);
		uchar *p_a = timg.ptr(i + 1);
		uchar *p_s = timg.ptr(i - 1);

		for (j = 1; j < wid + 1; j++)
		{
			d[0] = p[j + 1];		// east
			d[2] = p_a[j];	// south
			d[4] = p[j - 1];		// west
			d[6] = p_s[j];	// north
			d[1] = p_a[j + 1];	// south-east
			d[3] = p_a[j - 1];		// south-west
			d[5] = p_s[j - 1];		// north-west
			d[7] = p_s[j + 1];		// north-east

			dx = double(d[7] + sobelw*d[0] + d[1] - d[3] - sobelw*d[4] - d[5]);
			dy = double(d[1] + sobelw*d[2] + d[3] - d[5] - sobelw*d[6] - d[7]);

			if (sqrt(dx*dx + dy*dy) <= gradTh)
			{
				dx = 0;
				dy = 0;
			}

			if (dx == 0 && dy == 0)
				continue;
			gradDir(wid, i - 1, j - 1, dx, dy, dn);
		}
	}
}
void HOGFeature::gradDir(int wid, int i, int j, double dx, double dy, int dn)
{
	double theta = 0;
	double theta_step = 0;
	double sin_step = 0;

	if (orien_dir == 0)
	{
		theta_step = double(2 * CV_PI / dn);
		sin_step = sin(theta_step);
		theta = atan2(dy, dx);
		if (theta < 0)
			theta += 2 * CV_PI;
	}
	else
	{
		theta_step = double(CV_PI / dn);
		sin_step = sin(theta_step);
		theta = atan(dy / dx);
		if (theta < 0)
			theta += CV_PI;
	}

	int n = (int)floor(theta / theta_step);
	double r = sqrt(dx*dx + dy*dy);


	wight_map[1][i*wid + j] = r*sin(theta - n*theta_step) / sin_step;
	wight_bin[1][i*wid + j] = (n + 1) % 8;

	wight_map[0][i*wid + j] = r*sin((n + 1)*theta_step - theta) / sin_step;
	wight_bin[0][i*wid + j] = n;
}

void HOGFeature::fblurEven(Size NormSize, vector<vector<double> >& ftr)
{
	int m_oneblock = nbins*blockSize.width / cellSize.width*blockSize.height / cellSize.height;
	vector<double> grd_hist(m_oneblock, 0);

	int count_block_h = (NormSize.height - blockSize.height) / blockStride.height + 1;
	int count_block_w = (NormSize.width - blockSize.width) / blockStride.width + 1;

	int count_cell_h = blockSize.height / cellSize.height;
	int count_cell_w = blockSize.width / cellSize.width;

	for (int i = 0; i < count_block_h; ++i)
	{
		for (int j = 0; j < count_block_w; ++j)
		{
			grd_hist.assign(m_oneblock, 0);

			for (int m = 0; m < count_cell_h; ++m)
			{
				int yl = i*blockStride.height + m*cellSize.height;
				int yu = yl + cellSize.height;

				for (int n = 0; n < count_cell_w; ++n)
				{
					int xl = j*blockStride.width + n*cellSize.width;
					int xu = xl + cellSize.width;

					for (int y = yl; y < yu; ++y)
					{
						int my = y - blockStride.height*i;;

						for (int x = xl; x < xu; ++x)
						{
							int mx = x - blockStride.width*j;

							int k = wight_bin[0][y*NormSize.width + x];
							grd_hist[k + (m*count_cell_w + n)*nbins] += wight_map[0][y*NormSize.width + x] * gaussM[my][mx];

							k = wight_bin[1][y*NormSize.width + x];
							grd_hist[k + (m*count_cell_w + n)*nbins] += wight_map[1][y*NormSize.width + x] * gaussM[my][mx];
						}
					}
				}
			}

			cv::normalize(grd_hist, grd_hist);
			ftr.push_back(grd_hist);
		}
	}
}
