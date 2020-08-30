#include "utility.h"
#include "opencv2\ml\ml.hpp"

double dx = 0, dy = 0;
double angle_err = 10;
double th_90 = sin((90 - angle_err)*CV_PI / 180);
double th_270 = sin((270 + angle_err)*CV_PI / 180);

double th_0 = cos(angle_err*CV_PI / 180);
double th_180 = cos((180 - angle_err)*CV_PI / 180);

#include "imageRead.h"

struct Box_Score
{
	Rect r;
	double score;
};

bool bs_comp(Box_Score a, Box_Score b)
{
	return a.score > b.score;
}

void getProposals(Mat &img,vector<Rect> &recBag)
{
	if (img.channels() != 1) { cvtColor(img, img, CV_BGR2GRAY); }
	double * image;
	double * out;
	int width = 480;  /* x image size */
	int height = 360;  /* y image size */
	/* create a simple image: left half black, right half gray */
	image = (double *)malloc(width * height * sizeof(double));
	if (image == NULL)
	{
		fprintf(stderr, "error: not enough memory\n");
		exit(EXIT_FAILURE);
	}

	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			image[y*width + x] = img.at<uchar>(y, x);  /* image(x,y) */

	int nl;  //number of lines
	out = lsd(&nl, image, width, height);

	vector<Vec4i> lines_v, lines_h;
	for (int i = 0; i < nl; ++i)
	{
		Vec4i l;
		dx = out[i * 7 + 5];
		dy = out[i * 7 + 6];

		l[0] = cvRound(out[i * 7 + 0]);
		l[1] = cvRound(out[i * 7 + 1]);
		l[2] = cvRound(out[i * 7 + 2]);
		l[3] = cvRound(out[i * 7 + 3]);

		/*check premeters*/
		{
			l[0] = l[0] < 0 ? 0 : l[0]; l[0] = l[0]>width - 1 ? width - 1 : l[0];
			l[1] = l[1] < 0 ? 0 : l[1]; l[1] = l[1]>height - 1 ? height - 1 : l[1];

			l[2] = l[2] < 0 ? 0 : l[2]; l[2] = l[2]>width - 1 ? width - 1 : l[2];
			l[3] = l[3] < 0 ? 0 : l[3]; l[3] = l[3]>height - 1 ? height - 1 : l[3];
		}

		if (dy<th_270 || dy>th_90) { lines_v.push_back(l); }
		if (dx > th_0 || dx < th_180) { lines_h.push_back(l); }
		
	}

	fixDirtion(lines_v, 1);
	fixDirtion(lines_h, 0);

	fixLines_start_end(lines_v, 1);
	fixLines_start_end(lines_h, 0);

	fixCloseLines(lines_v, 1, 2);
	fixCloseLines(lines_h, 0, 2);

	fixCloseLinesButSave(lines_v, 1, 5);
	fixCloseLinesButSave(lines_h, 0, 5);

	fixSameLines(lines_v, 1, 8);
	fixSameLines(lines_h, 0, 8);
	//fixSameLines_2(lines_v, 1, 0.9, 0.1);
	//fixSameLines_2(lines_h, 0, 0.9, 0.1);
	Mat recImage = img.clone();
	findRectangles_3(lines_v, lines_h, recImage, recBag);
}
string getFileName(string str)
{
	int m = str.find_last_of('\\');
	int n = str.find_last_of('.');

	return str.substr(m + 1, n - m - 1);
}
void getRangeFromTxt(string tempObject, int *xmin, int *xmax, int *ymin, int *ymax)
{
	int posi[8];
	int k = 0;
	posi[0] = 0;
	for (int i = 0; i != tempObject.size(); ++i)
	{
		if (tempObject[i] == ',')
		{
			posi[++k] = i;
		}
	}

	int xy[8];

	xy[0] = atoi(tempObject.substr(0, posi[1]).c_str());
	for (k = 1; k < 8; k++)
	{
		xy[k] = atoi(tempObject.substr(posi[k] + 1, posi[k + 1] - posi[k] - 1).c_str());
	}

	/*check premeters*/
	{
		xy[0] = xy[0] < 0 ? 0 : xy[0]; xy[0] = xy[0] > 479 ? 479 : xy[0];
		xy[1] = xy[1] < 0 ? 0 : xy[1]; xy[1] = xy[1] > 359 ? 359 : xy[1];

		xy[4] = xy[4] < 0 ? 0 : xy[4]; xy[4] = xy[4] > 479 ? 479 : xy[4];
		xy[5] = xy[5] < 0 ? 0 : xy[5]; xy[5] = xy[5] > 359 ? 359 : xy[5];
	}

	*xmin = xy[0];
	*ymin = xy[1];
	*xmax = xy[4];
	*ymax = xy[5];
}
int similarity(Rect r, int xmin, int ymin, int xmax, int ymax, double th)
{
	int sx = max(xmin, r.x);
	int sy = max(ymin, r.y);

	int ex = min(xmax, r.x + r.width);
	int ey = min(ymax, r.y + r.height);

	int over_w = ex - sx > 0 ? ex - sx : 0;
	int over_h = ey - sy > 0 ? ey - sy : 0;

	double over_area = over_w*over_h;
	double r1 = over_area / double(r.area());
	double r2 = over_area / ((xmax - xmin)*(ymax - ymin));

	if (r1 >= th && r2 >= th)
		return 1;

	return 0;
}
int overlap_object(Rect r, int *xmin, int *ymin, int *xmax, int *ymax, int n, double th)
{
	double over_area_total = 0;
	for (int i = 0; i < n; ++i)
	{
		int sx = max(xmin[i], r.x);
		int sy = max(ymin[i], r.y);

		int ex = min(xmax[i], r.x + r.width);
		int ey = min(ymax[i], r.y + r.height);

		int over_w = ex - sx > 0 ? ex - sx : 0;
		int over_h = ey - sy > 0 ? ey - sy : 0;

		double over_area = over_w*over_h;
		over_area_total += over_area;
	}
	double ra = over_area_total / r.area();
	if (ra > th)
		return 1;
	return 0;
}
void rect_extend(vector<Rect> &rects, double r, int width, int height)
{
	for (int i = 0; i < rects.size(); ++i)
	{
		Rect pr = rects[i];
		int dx = cvRound(pr.width*(r - 1) / 2.0);
		int dy = cvRound(pr.height*(r - 1) / 2.0);
		int w = cvRound(pr.width*r);
		int h = cvRound(pr.height*r);
		int x = pr.x - dx;
		int y = pr.y - dy;
		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
		if (x + w>width - 1)
			w = width - 1 - x;
		if (y + h>height - 1)
			h = height - 1 - y;

		rects[i].x = x;
		rects[i].y = y;
		rects[i].width = w;
		rects[i].height = h;
	}
}

void loadFile(vector<string> &image_path, vector<string> &data_path, string imageFile, string dataFile)
{
	readImageFile(imageFile + ".txt", image_path);
	readImageFile(dataFile, data_path);
}
static float overlap(const cv::Rect &a, const cv::Rect &b)
{
	int w = std::min(a.x + a.width, b.x + b.width) - std::max(a.x, b.x);
	int h = std::min(a.y + a.height, b.y + b.height) - std::max(a.y, b.y);

	return (w < 0 || h < 0) ? 0.f : (float)(w * h);
}
void supress(vector<Rect> &recBag, vector<double> &scores, double r)
{
	vector<Box_Score> boxScores;
	Box_Score bs;
	for (int i = 0; i < recBag.size(); ++i)
	{
		bs.r = recBag[i];
		bs.score = scores[i];
		boxScores.push_back(bs);
	}
	sort(boxScores.begin(), boxScores.end(), bs_comp);
	vector<int> retain(boxScores.size(), 1);
	for (int i = 0; i < boxScores.size(); ++i)
	{
		for (int j = i + 1; j < boxScores.size(); ++j)
		{
			double ain = overlap(boxScores[i].r, boxScores[j].r);
			double aun = boxScores[i].r.area() + boxScores[j].r.area() - ain;
			if ((ain / aun) > r)
			{
				retain[j] = 0;
			}
		}
	}

	recBag.clear();
	scores.clear();
	for (int i = 0; i < boxScores.size(); ++i)
	{
		if (retain[i])
		{
			recBag.push_back(boxScores[i].r);
			scores.push_back(boxScores[i].score);
		}

	}
	boxScores.clear();
}

bool contain(Rect r, Rect rc)
{
	if (rc.x >= r.x && rc.y >= r.y &&
		rc.x + rc.width - 1 <= r.x + r.width - 1 && rc.y + rc.height <= r.y + r.height)
	{
		return true;
	}
	return false;
}
void supressContain(vector<Rect> &recBag, vector<double> &scores, double r)
{
	vector<Box_Score> boxScores;
	Box_Score bs;
	for (int i = 0; i < recBag.size(); ++i)
	{
		bs.r = recBag[i];
		bs.score = scores[i];
		boxScores.push_back(bs);
	}
	sort(boxScores.begin(), boxScores.end(), bs_comp);
	vector<int> retain(boxScores.size(), 1);
	for (int i = 0; i < boxScores.size(); ++i)
	{
		for (int j = i + 1; j < boxScores.size(); ++j)
		{
			double ain = overlap(boxScores[i].r, boxScores[j].r);
			double aun = boxScores[i].r.area() + boxScores[j].r.area() - ain;
			if ((ain / aun) > r)
			{
				retain[j] = 0;
			}
			else if (contain(boxScores[j].r, boxScores[i].r))
			{
				/*if ((double)boxScores[i].r.area() / boxScores[j].r.area()>0.8)*/
					retain[j] = 0;
				
			}
		}
	}

	recBag.clear();
	scores.clear();
	for (int i = 0; i < boxScores.size(); ++i)
	{
		if (retain[i])
		{
			recBag.push_back(boxScores[i].r);
			scores.push_back(boxScores[i].score);
		}

	}
	boxScores.clear();
}

 void normSize(Mat &recImage, double length)
{
	double w = recImage.cols;
	double h = recImage.rows;

	if (max(w, h) == length)
		return;

	if (w>h)
	{
		double s = length / w;
		resize(recImage, recImage, Size(), s, s);
	}
	else
	{
		double s = length / h;
		resize(recImage, recImage, Size(), s, s);
	}
}
 void getLines(Mat &img, vector<Vec4i> &lines_v, vector<Vec4i> &lines_h)
 {
	 if (img.channels() != 1) { cvtColor(img, img, CV_BGR2GRAY); }
	 double * image;
	 double * out;
	 int width = img.cols;  /* x image size */
	 int height = img.rows;  /* y image size */
	 /* create a simple image: left half black, right half gray */
	 image = (double *)malloc(width * height * sizeof(double));
	 if (image == NULL)
	 {
		 fprintf(stderr, "error: not enough memory\n");
		 exit(EXIT_FAILURE);
	 }

	 for (int y = 0; y < height; y++)
		 for (int x = 0; x < width; x++)
			 image[y*width + x] = img.at<uchar>(y, x);  /* image(x,y) */

	 int nl;  //number of lines
	 out = lsd(&nl, image, width, height);

	 for (int i = 0; i < nl; ++i)
	 {
		 Vec4i l;
		 dx = out[i * 7 + 5];
		 dy = out[i * 7 + 6];

		 l[0] = cvRound(out[i * 7 + 0]);
		 l[1] = cvRound(out[i * 7 + 1]);
		 l[2] = cvRound(out[i * 7 + 2]);
		 l[3] = cvRound(out[i * 7 + 3]);

		 /*check premeters*/
		 {
			 l[0] = l[0] < 0 ? 0 : l[0]; l[0] = l[0]>width - 1 ? width - 1 : l[0];
			 l[1] = l[1] < 0 ? 0 : l[1]; l[1] = l[1]>height - 1 ? height - 1 : l[1];

			 l[2] = l[2] < 0 ? 0 : l[2]; l[2] = l[2]>width - 1 ? width - 1 : l[2];
			 l[3] = l[3] < 0 ? 0 : l[3]; l[3] = l[3]>height - 1 ? height - 1 : l[3];
		 }

		 if (dy<th_270 || dy>th_90) { lines_v.push_back(l); }
		 if (dx > th_0 || dx < th_180) { lines_h.push_back(l); }

	 }

	 fixDirtion(lines_v, 1);
	 fixDirtion(lines_h, 0);

	 fixLines_start_end(lines_v, 1);
	 fixLines_start_end(lines_h, 0);
 }

 bool comp_a(Rect r1, Rect r2)
 {
	 return r1.area() > r2.area();
 }

 void cut(Mat &img,Rect &r, int sx, int sy, int ex, int ey, CvSVM &svm, double s_th)
 {
	 Rect r_c(r);
	 Rect r1, r2, r3, r4;
	 r1.x = r_c.x, r1.y = r_c.y, r1.width = r_c.width, r1.height = sy-r_c.y  + 1;
	 if ((double)r1.height / r1.width >= 0.2)
	 {
		 Mat recImg(img, r1);
		 if (!classifyLBP_RecImg(recImg, svm, s_th))
		 {
			 r.y = r1.y;
		 }
	 }
	 else
		 r.y = r1.y;

	 r2.x = ex, r2.y = r_c.y, r2.width = r_c.x+r_c.width-r2.x, r2.height = r_c.height;
	 if ((double)r2.width / r2.height >= 0.2)
	 {
		 Mat recImg(img, r2);
		 if (!classifyLBP_RecImg(recImg, svm, s_th))
		 {
			 r.width = ex - r.x + 1;
		 }
	 }
	 else
		 r.width = ex - r.x + 1;

	 r3.x = r_c.x, r3.y = ey, r3.width = r_c.width, r3.height = r_c.y+r_c.height-r3.y;

	 if ((double)r3.height / r3.width >= 0.2)
	 {
		 Mat recImg(img, r3);
		 if (!classifyLBP_RecImg(recImg, svm, s_th))
		 {
			 r.height = ey-r.y+1;
		 }
	 }
	 else
		 r.height = ey - r.y + 1;

	 r4.x = r_c.x, r4.y = r_c.y, r4.width = sx-r_c.x+1, r4.height = r_c.height;
	 if ((double)r4.width / r4.height >= 0.2)
	 {
		 Mat recImg(img, r4);
		 if (!classifyLBP_RecImg(recImg, svm, s_th))
		 {
			 r.x = sx;
		 }
	 }
	 else
		 r.x = sx;

 }
 void cutRects(vector<Rect> &recBag, Mat &img,CvSVM &svm,double s_th)
 {
	 sort(recBag.begin(),recBag.end(),comp_a);
	 for (int i = 0; i < recBag.size(); ++i)
	 {
		 Rect ri = recBag[i];
		 for (int j = 0; j < recBag.size(); ++j)
		 {
			 Rect rj = recBag[j];
			 int sx = max(ri.x, rj.x);
			 int sy = max(ri.y, rj.y);

			 int ex = min(ri.x + ri.width - 1, rj.x + rj.width - 1);
			 int ey = min(ri.y + ri.height - 1, rj.y+rj.height - 1);

			 if (ex <= sx || ey <= sy)
				 continue;

			 double r1 = double(ex - sx + 1) / ri.width;
			 double r2 = double(ey - sy + 1) / ri.height;

			 if (r1 > 0.8 || r2 > 0.8)
			 {
				 cut(img, ri, sx, sy, ex, ey, svm, s_th);
				 recBag[i] = ri;
			 }
			

		 }
	 }
 }