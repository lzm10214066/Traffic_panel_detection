#ifndef _FEATURELBP_H
#define _FEATURELBP_H

#include "opencv2\core\core.hpp"
#include "opencv2\highgui\highgui.hpp"

#include "LBP.hpp"

using namespace cv;
using namespace std;
using namespace lbp;

struct LBP_Option
{
	MappingType type;
	vector<double> radius;
	vector<unsigned int> samples;
};

void calLBPMultiScale(Mat &src, vector<double> &hist, LBP_Option &lbp_opt);
void calLBPMultiScaleChannels(Mat &src, vector<double> &hist, LBP_Option &lbp_opt);

void calLBPHFSM_MultiScale(Mat &src, vector<double> &hist, LBP_Option &lbp_opt);
void calLBPHFSM_MultiScaleChannels(Mat &src, vector<double> &hist, LBP_Option &lbp_opt);

void calCLBP_MultiScale(Mat &src, vector<double> &hist, LBP_Option &lbp_opt);
void calCLBPMultiScaleChannels(Mat &src, vector<double> &hist, LBP_Option &lbp_opt);

void calLBPHFSMC_MultiScale(Mat &src, vector<double> &hist, LBP_Option &lbp_opt);

#endif