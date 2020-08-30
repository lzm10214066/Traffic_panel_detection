#include "lines.h"


void analysisLinesH(vector<Vec4i> &l_h, int &s, int &e);
void analysisLinesV(vector<Vec4i> &l_v, int &s, int &e);


typedef struct LINE
{
	Vec4i l;
	int label;
}line_label;

bool lla_comp_x_t(line_label a, line_label b)
{
	return a.l[0] < b.l[0];
}

bool lla_comp_x_b(line_label a, line_label b)
{
	return a.l[0] > b.l[0];
}

bool lla_comp_y_l(line_label a, line_label b)
{
	return a.l[1] > b.l[1];
}

bool lla_comp_y_r(line_label a, line_label b)
{
	return a.l[1] < b.l[1];
}

bool comp_x(Vec4i a, Vec4i b)
{
	return a[0] < b[0];

}

bool comp_y(Vec4i a, Vec4i b)
{
	return a[1] < b[1];   //升序排列，如果改为return a>b，则为降序
}

bool comp_length_v(Vec4i a, Vec4i b)
{
	int len_a = abs(a[3] - a[1]);
	int len_b = abs(b[3] - b[1]);
	return len_a > len_b;
}

bool comp_length_h(Vec4i a, Vec4i b)
{
	int len_a = abs(a[2] - a[0]);
	int len_b = abs(b[2] - b[0]);
	return len_a > len_b;
}

///////////////////////////////////////////////*xÐ¡µÄµã×÷ÎªÆðµã*//////////////////////////////

void fixLines_start_end(vector<Vec4i> &lines_in, int lineType)
{
	if (lines_in.empty())
	{
		return;
	}
	if (lineType)
	{
		for (int i = 0; i != lines_in.size(); ++i)
		{
			Vec4i l = lines_in[i];

			if (l[1] > l[3])
			{
				int t = l[1];
				l[1] = l[3];
				l[3] = t;

				t = l[0];
				l[0] = l[2];
				l[2] = t;
			}
			lines_in[i] = l;
		}
	}

	else
	{
		for (int i = 0; i != lines_in.size(); ++i)
		{

			Vec4i l = lines_in[i];
			if (l[0] > l[2])
			{
				int t = l[0];
				l[0] = l[2];
				l[2] = t;

				t = l[1];
				l[1] = l[3];
				l[3] = t;

			}
			lines_in[i] = l;
		}
	}
}


void drawLines(Mat &dst, vector<Vec4i> &lines, Scalar color)
{
	for (int i = 0; i != lines.size(); ++i)
	{
		Vec4i l_y = lines[i];
		line(dst, Point(l_y[0], l_y[1]), Point(l_y[2], l_y[3]), color, 1);
	}
}

////////////////////////////////* 校正方向，为水平和垂直*//////////////////////////////
void fixDirtion(vector<Vec4i> &lines_in, int lineType)
{
	if (lineType)
	{
		for (int i = 0; i != lines_in.size(); ++i)
		{
			Vec4i l = lines_in[i];
			if (l[0] != l[2])
			{
				int x = cvRound((l[0] + l[2]) / 2.0);
				lines_in[i][0] = x;
				lines_in[i][2] = x;
			}
		}
	}
	else
	{
		for (int i = 0; i != lines_in.size(); ++i)
		{
			Vec4i l = lines_in[i];
			if (l[1] != l[3])
			{
				int y = cvRound((l[1] + l[3]) / 2.0);
				lines_in[i][1] = y;
				lines_in[i][3] = y;
			}
		}
	}

}

////////////////////////////////*合并距离较近的直线*////////////////////
void fixCloseLines(vector<Vec4i> &lines, int lineType, int posi_th)
{
	if (lines.empty())
		return;
	switch (lineType)
	{

#pragma region case_0:lines_h
	case 0:
	{
		vector<Vec4i> lines_tmp;
		vector<int> used(lines.size(), 0);

		sort(lines.begin(), lines.end(), comp_length_h);

		for (int i = 0; i != lines.size(); ++i)
		{
			if (used[i] == 1)
				continue;

			int seed_y = lines[i][1];
			int sx = lines[i][0];
			int ex = lines[i][2];

			vector<line_label> lines_label;
			line_label lla;

			for (int j = i + 1; j != lines.size(); ++j)
			{
				int y = lines[j][1];

				if (used[j] == 0 && y >= seed_y - posi_th && y <= seed_y + posi_th)
				{
					if (lines[j][0] >= sx && lines[j][2] <= ex)
					{
						used[j] = 1;
					}

					else
					{
						lla.l = lines[j];
						lla.label = j;
						lines_label.push_back(lla);
					}
				}
			}

			if (lines_label.empty())
				continue;

			lla.l = lines[i];
			lla.label = i;
			lines_label.push_back(lla);

			sort(lines_label.begin(), lines_label.end(), lla_comp_x_t);

			int k = 0;
			while (k < lines_label.size() && lines_label[k].l[0] != sx) k++;
			int end = ex;
			int start = sx;

			for (int i = k + 1; i < lines_label.size(); ++i)
			{
				if (lines_label[i].l[0] - end <= posi_th)
				{
					used[lines_label[i].label] = 1;
					if (lines_label[i].l[2] > end)
					{
						end = lines_label[i].l[2];
					}
				}
				else
					break;
			}

			for (int i = k - 1; i >= 0; --i)
			{
				if (start - lines_label[i].l[2] <= posi_th)
				{
					used[lines_label[i].label] = 1;
					if (lines_label[i].l[0] < start)
					{
						start = lines_label[i].l[0];
					}
				}
				else
					break;
			}

			lines_label.clear();
			lines[i][0] = start;
			lines[i][2] = end;
		}

		for (int i = 0; i != used.size(); ++i)
		{
			if (used[i] == 0)
			{
				int len = lines[i][2] - lines[i][0];
				if (len > 10)
					lines_tmp.push_back(lines[i]);
			}
		}
		lines.clear();
		lines.assign(lines_tmp.begin(), lines_tmp.end());
		lines_tmp.clear();

	}; break;

#pragma endregion

#pragma region case_1:lines_v
	case 1:
	{
		vector<Vec4i> lines_tmp;
		vector<int> used(lines.size(), 0);

		sort(lines.begin(), lines.end(), comp_length_v);

		for (int i = 0; i != lines.size(); ++i)
		{
			if (used[i] == 1)
				continue;

			int seed_x = lines[i][0];
			int ey = lines[i][3];
			int sy = lines[i][1];

			vector<line_label> lines_label;
			line_label lla;

			for (int j = i + 1; j != lines.size(); ++j)
			{
				int x = lines[j][0];

				if (used[j] == 0 && x >= seed_x - posi_th && x <= seed_x + posi_th)
				{
					if (lines[j][3] <= ey && lines[j][1] >= sy)
					{
						used[j] = 1;
					}

					else
					{
						lla.l = lines[j];
						lla.label = j;
						lines_label.push_back(lla);
					}
				}
			}

			if (lines_label.empty())
				continue;

			lla.l = lines[i];
			lla.label = i;
			lines_label.push_back(lla);

			sort(lines_label.begin(), lines_label.end(), lla_comp_y_r);

			int k = 0;
			while (k < lines_label.size() && lines_label[k].l[1] != sy) k++;
			int end = ey;
			int start = sy;

			for (int i = k + 1; i < lines_label.size(); ++i)
			{
				if (lines_label[i].l[1] - end <= posi_th)
				{
					used[lines_label[i].label] = 1;
					if (lines_label[i].l[3] > end);
					{
						end = lines_label[i].l[3];
					}
				}
				else
					break;
			}

			for (int i = k - 1; i >= 0; --i)
			{
				if (start - lines_label[i].l[3] <= posi_th)
				{
					used[lines_label[i].label] = 1;
					if (lines_label[i].l[1] < start)
					{
						start = lines_label[i].l[1];
					}
				}
				else
					break;
			}

			lines_label.clear();
			lines[i][1] = start;
			lines[i][3] = end;
		}

		for (int i = 0; i != used.size(); ++i)
		{
			if (used[i] == 0)
			{
				int len = lines[i][3] - lines[i][1];
				if (len > 10)
					lines_tmp.push_back(lines[i]);
			}
		}
		lines.clear();
		lines.assign(lines_tmp.begin(), lines_tmp.end());
		lines_tmp.clear();

	}; break;

#pragma endregion

	}
}

void fixSameLines(vector<Vec4i> &lines, int lineType, int posi_th)
{
	if (lines.empty())
		return;
	switch (lineType)
	{

#pragma region case_0:lines_h
	case 0:
	{
		vector<Vec4i> lines_tmp;
		vector<int> used(lines.size(), 0);

		sort(lines.begin(), lines.end(), comp_length_h);

		for (int i = 0; i != lines.size(); ++i)
		{
			if (used[i] == 1)
				continue;

			int seed_y = lines[i][1];
			int sx = lines[i][0];
			int ex = lines[i][2];

			for (int j = i + 1; j != lines.size(); ++j)
			{
				int y = lines[j][1];

				if (used[j] == 0 && y >= seed_y - posi_th && y <= seed_y + posi_th)
				{
					if (abs(lines[j][0] - sx)<=10 && abs(lines[j][2] - ex)<=  10)
					{
						used[j] = 1;
					}
				}
			}

		}

		for (int i = 0; i != used.size(); ++i)
		{
			if (used[i] == 0)
			{
				int len = lines[i][2] - lines[i][0];
				if (len >= 10)
					lines_tmp.push_back(lines[i]);
			}
		}
		lines.clear();
		lines.assign(lines_tmp.begin(), lines_tmp.end());
		lines_tmp.clear();

	}; break;

#pragma endregion

#pragma region case_1:lines_v
	case 1:
	{
		vector<Vec4i> lines_tmp;
		vector<int> used(lines.size(), 0);

		sort(lines.begin(), lines.end(), comp_length_v);

		for (int i = 0; i != lines.size(); ++i)
		{
			if (used[i] == 1)
				continue;

			int seed_x = lines[i][0];
			int ey = lines[i][3];
			int sy = lines[i][1];

			vector<line_label> lines_label;
			line_label lla;

			for (int j = i + 1; j != lines.size(); ++j)
			{
				int x = lines[j][0];

				if (used[j] == 0 && x >= seed_x - posi_th && x <= seed_x + posi_th)
				{
					if (abs(lines[j][3] - ey) <= 10 && abs(lines[j][1] - sy) <= 10)
					{
						used[j] = 1;
					}
				}
			}

		}

		for (int i = 0; i != used.size(); ++i)
		{
			if (used[i] == 0)
			{
				int len = lines[i][3] - lines[i][1];
				if (len > 10)
					lines_tmp.push_back(lines[i]);
			}
		}
		lines.clear();
		lines.assign(lines_tmp.begin(), lines_tmp.end());
		lines_tmp.clear();

	}; break;

#pragma endregion




	}
}


/////////////////////////////*矩形检测*///////////////////////////////////////////
//void findRectTwoLines_vertical(vector<Vec4i> &lines_l, vector <Vec4i> &lines_r, Mat &dst, vector<Rect> &recBag)
//{
//	int minArea = 500;
//	int maxArea = 480*360/2;
//////////////////*竖直线扫描*////////////////////////////////////////
//	for (int i = 0; i != lines_l.size(); ++i)
//	{
//		Vec4i li = lines_l[i];
//		int len_i = li[1] - li[3];
//		for (int j = 0; j != lines_r.size(); ++j)
//		{
//			Vec4i lj = lines_r[j];
//			int len_j = lj[3] - lj[1];
//
//			double len_r = (double)len_i / len_j;
//			if (len_r < 0.7 || len_r>1.5)
//			{
//				continue;
//			}
//
//			int dx = abs(lj[0] - li[0]);
//			if (dx >= 20 && dx<=400)
//			{
//				int dy_t = abs(li[3] - lj[1]);
//				int dy_b = abs(li[1] - lj[3]);
//
//				if (dy_t <= 10 && dy_b <= 10)
//				{
//					int s_top = li[3], b_top = lj[1];
//					int s_bottom = li[1], b_bottom = lj[3];
//					if (s_top>b_top)
//					{
//						s_top = lj[1];
//						b_top = li[3];
//					}
//					if (li[1] > lj[3])
//					{
//						s_bottom = lj[3];
//						b_bottom = li[1];
//					}
//
//					int x = 0, y = 0;
//					x = li[0]<lj[0]?li[0]:lj[0];
//					y = s_top;
//					int width = dx;
//					int height = b_bottom - s_top;
//				
//					Rect rec(x, y, width, height);
//					double r = (double)width/height;
//					if (rec.area() > minArea && rec.area() < maxArea && (r<w_h_ratio || r>1/w_h_ratio))
//					{
//						rectangle(dst, rec, Scalar(0,0,255));
//						recBag.push_back(rec);
//					}
//					
//				}
//			}
//		}
//	}
//
////////////////////////*水平线扫描*//////////////////////////////////////////////////////
//	//for (int i = 0; i != lines_h.size() - 1; ++i)
//	//{
//	//	Vec4i li = lines_h[i];
//	//	int len_i = li[2] - li[0];
//	//	for (int j = i + 1; j != lines_h.size(); ++j)
//	//	{
//	//		Vec4i lj = lines_h[j];
//	//		int len_j = lj[2] - lj[0];
//	//		double len_r = (double)len_i / len_j;
//	//		if (len_r < 0.25 || len_r>4)
//	//		{
//	//			continue;
//	//		}
//
//	//		int dy = lj[1] - li[1];
//	//		if (dy > 20 && dy<300)
//	//		{
//	//			int dx_s = abs(lj[0] - li[0]);
//	//			int dx_e = abs(lj[2] - li[2]);
//
//	//			if (dx_s < 10 || dx_e < 10)
//	//			{
//	//				int s_left = li[0], b_left = lj[0];
//	//				int s_right = li[2], b_right = lj[2];
//
//	//				if (li[0] > lj[0])
//	//				{
//	//					s_left = lj[0];
//	//					b_left = li[0];
//	//				}
//	//				if (li[2] > lj[2])
//	//				{
//	//					s_right = lj[2];
//	//					b_right = li[2];
//	//				}
//
//	//				int x = 0, y = 0;
//	//				x = s_left;
//	//				y = li[1];
//
//	//				int height = dy;
//	//				int width = b_right- s_left;
//
//	//				Rect rec(x, y, width, height);
//	//				double r = (double)width / height;
//	//				if (rec.area() > minArea && rec.area() < maxArea && (r<4 || r>1 / 4))
//	//				{
//	//					rectangle(dst, rec, 255);
//	//					recBag.push_back(rec);
//	//				}
//	//			}
//
//	//		}
//	//	}
//	//}
//}

//void findRecTest(vector<Vec4i> &lines_v, vector<Vec4i> &lines_h, Mat &dst, vector<Rect> &recBag)
//{
//	int minArea = 500;
//	int maxArea = 480 * 360 / 2;
//	////////////////*竖直线扫描*////////////////////////////////////////
//	sort(lines_v.begin(), lines_v.end(), comp_x);
//	sort(lines_h.begin(), lines_h.end(), comp_y);
//
//	for (int i = 0; i < lines_v.size()-1; ++i)
//	{
//		Vec4i li = lines_v[i];
//		int len_i = li[3] - li[1];
//	
//		for (int j = i+1; j < lines_v.size(); ++j)
//		{
//			Vec4i lj = lines_v[j];
//			int len_j = lj[3] - lj[1];
//
//			double len_r = (double)len_i / len_j;
//			if (len_r <= 0.5 || len_r>=2)
//			{
//				continue;
//			}
//
//			int distance_H = lj[0] - li[0];
//			if (distance_H > 10 && distance_H<400)
//			{
//				int dy_top = abs(lj[1] - li[1]);
//				int dy_bottom = abs(lj[3] - li[3]);
//
//				int width = distance_H;
//				int height = 0;
//
//				int s_top = li[1], b_top = lj[1];
//				int s_bottom = li[3], b_bottom = lj[3];
//				if (li[1] > lj[1])
//				{
//					s_top = lj[1];
//					b_top = li[1];
//				}
//				if (li[3] > lj[3])
//				{
//					s_bottom = lj[3];
//					b_bottom = li[3];
//				}
//
//				if (dy_top <= 10 || dy_bottom <= 10)
//				{
//					int up_top = b_top + 10;
//					int down_top = s_top - 10;
//
//					int up_bottom = b_bottom + 10;
//					int down_bottom = s_bottom - 10;
//
//					int ls = li[0];
//					int rs = lj[0];
//					vector<Vec4i> l_h_top;
//					vector<Vec4i> l_h_bottom;
//
//					int f_top = 0;
//					int f_bottom = 0;
//
//					f_top = findHorizonLine(lines_h, up_top, down_top, ls, rs, l_h_top);
//					f_bottom = findHorizonLine(lines_h, up_bottom, down_bottom, ls, rs, l_h_bottom);
//
//					if (f_top == 1 && f_bottom == 1)
//					{
//						int y = l_h_top[1];
//						int x = li[0];
//						height = l_h_bottom[1] - l_h_top[1];
//						Rect rec(x, y, width, height);
//						double ratio = (double)width / height;
//						if (rec.area() > minArea && rec.area()<maxArea && ratio<4 && ratio>1 / 4)
//						{
//							rectangle(dst, rec, Scalar(0,0,255));
//							recBag.push_back(rec);
//						}
//
//					}
//					if (f_top == 1 && f_bottom == 0)
//					{
//						int y = l_h_top[1];
//						int x = li[0];
//						height = b_bottom - l_h_top[1];
//						Rect rec(x, y, width, height);
//						double ratio = (double)width / height;
//						if (rec.area() > minArea && rec.area()<maxArea && ratio<4 && ratio>1 / 4)
//						{
//							rectangle(dst, rec, Scalar(0,0,255));
//							recBag.push_back(rec);
//						}
//					}
//					if (f_top == 0 && f_bottom == 1)
//					{
//						int y = s_top;
//						int x = li[0];
//						height = l_h_bottom[1] - s_top;
//						Rect rec(x, y, width, height);
//						double ratio = (double)width / height;
//						if (rec.area() > minArea && rec.area()<maxArea && ratio<4 && ratio>1 / 4)
//						{
//							rectangle(dst, rec, Scalar(0,0,255));
//							recBag.push_back(rec);
//						}
//					}
//				}
//
//			}
//		}
//	}
//
//	//////////////////////*水平线扫描*//////////////////////////////////////////////////////
//	k = 0;
//	for (int i = 0; i != lines_h.size()-1; ++i)
//	{
//		Vec4i li = lines_h[i];
//
//		for (int j = i+1; j < lines_h.size(); ++j)
//		{
//			Vec4i lj = lines_h[j];
//			int distance_V = lj[1] - li[1];
//
//			if (distance_V >= 20)
//			{
//				int dx_left = abs(lj[0] - li[0]);
//				int dx_right = abs(lj[2] - li[2]);
//
//				int height = distance_V;
//				int width =0;
//
//				int s_left = li[0], b_left = lj[0];
//				int s_right = li[2], b_right = lj[2];
//
//				if (li[0] > lj[0])
//				{
//					s_left = lj[0];
//					b_left = li[0];
//				}
//				if (li[2] > lj[2])
//				{
//					s_right = lj[2];
//					b_right = li[2];
//				}
//
//				if (dx_left <= 10 || dx_right <= 10)
//				{
//					int up_left = b_left + 10;
//					int down_left = s_left - 10;
//
//					int up_right = b_right + 10;
//					int down_right = s_right - 10;
//
//					int ts = li[1];
//					int ds = lj[1];
//
//					Vec4i l_v_left;
//					Vec4i l_v_right;
//
//					int f_left = 0;
//					int f_right = 0;
//
//					f_left = findVerticalLine(lines_v, up_left, down_left, ts, ds, l_v_left);
//					f_right = findVerticalLine(lines_v, up_right, down_right, ts, ds, l_v_right);
//
//					if (f_left == 1 && f_right == 1)
//					{
//						int x = l_v_left[0];
//						int y = li[1];
//						width = l_v_right[0] - l_v_left[0];
//						Rect rec(x, y, width, height);
//						double ratio = (double)width / height;
//						if (rec.area() > minArea && rec.area()<maxArea && ratio<4 && ratio>1 / 4)
//						{
//							rectangle(dst, rec, Scalar(0,255,255));
//							recBag.push_back(rec);
//						}
//					}
//					if (f_left == 1 && f_right == 0)
//					{
//						int x = l_v_left[0];
//						int y = li[1];
//						width = b_right - l_v_left[0];
//						Rect rec(x, y, width, height);
//						double ratio = (double)width / height;
//						if (rec.area() > minArea && rec.area()<maxArea && ratio<4 && ratio>1 / 4)
//						{
//							rectangle(dst, rec, Scalar(0, 255, 255));
//							recBag.push_back(rec);
//						}
//					}
//					if (f_left == 0 && f_right == 1)
//					{
//						int x = s_left;
//						int y = li[1];
//						width = l_v_right[0] - s_left;
//						Rect rec(x, y, width, height);
//						double ratio = (double)width / height;
//						if (rec.area() > minArea && rec.area()<maxArea && ratio<4 && ratio>1 / 4)
//						{
//							rectangle(dst, rec, Scalar(0, 255, 255));
//							recBag.push_back(rec);
//						}
//					}
//				}
//
//			}
//		}
//	}
//}

Point getCrossPoint(Vec4i lv, Vec4i lh)
{
	Point p;
	p.y = lh[1];
	p.x = lv[0];
	return p;
}

int findHorizonLine(vector<Vec4i> lines_h, int up, int dowm, int ls, int rs, vector<Vec4i> &l_result)
{
	int width = rs - ls;

	for (int i = 0; i < lines_h.size(); ++i)
	{
		Vec4i l = lines_h[i];
		int y = l[1];
		if (y < dowm)
			continue;
		if (y > up)
			break;
		if (y >= dowm && y <= up)
		{
			int length = l[2] - l[0];
			if (l[0] >= ls-10 && l[2] <= rs+10)
			{
				if (length >= width*0.4)
				{
					l_result.push_back(l);
				}
			}
		/*	else if (l[0] < ls && l[2] <= rs && l[2]>ls)
			{
				double r = (double)(l[2] - ls) / width;
				if (r > 0.5)
				{
					l_result.push_back(l);
				}
			}
			else if (l[0] >= ls && l[0]< rs && l[2]>rs)
			{
				double r = (double)(rs - l[0]) / width;
				if (r > 0.5)
				{
					l_result.push_back(l);
				}
			}
			else if (l[0]<ls && l[2]>rs)
			{
				l_result.push_back(l);
			}*/
		}
	}

	if (l_result.empty())
	{
		return 0;
	}
	return 1;
}
int findVerticalLine(vector<Vec4i> lines_v, int up, int dowm, int ts, int ds, vector<Vec4i> &l_result)
{
	int height = ds - ts;

	for (int i = 0; i < lines_v.size(); ++i)
	{
		Vec4i l = lines_v[i];
		int x = l[0];
		if (x < dowm)
			continue;
		if (x > up)
			break;

		if (x >= dowm && x <= up)
		{
			int length = l[3] - l[1];
			if (l[1] >= ts-10 && l[3] <= ds+10)
			{
				if (length >= height*0.4)
				{
					l_result.push_back(l);
				}
			}
			/*else if (l[1] < ts && l[3] <= ds && l[3]>ts)
			{
				double r = (double)(l[3] - ts) / height;
				if (r > 0.5)
				{
					l_result.push_back(l);
				}
			}
			else if (l[1] >= ts && l[1]< ds && l[2]>ds)
			{
				double r = (double)(ds - l[1]) / height;
				if (r > 0.5)
				{
					l_result.push_back(l);
				}
			}
			else if (l[1]<ts && l[3]>ds)
			{
				l_result.push_back(l);
			}*/
		}
	}
	if (l_result.empty())
	{
		return 0;
	}
	return 1;
}


int findHorizonLine(vector<Vec4i> lines_h, int up, int dowm, int ls, int rs, vector<Vec4i> &l_result,double l_th)
{
	int width = rs - ls;

	for (int i = 0; i < lines_h.size(); ++i)
	{
		Vec4i l = lines_h[i];
		int y = l[1];
		if (y < dowm)
			continue;
		if (y > up)
			break;
		if (y >= dowm && y <= up)
		{
			int length = l[2] - l[0];
			if (l[0] >= ls - 10 && l[2] <= rs + 10)
			{
				if (length >= width*l_th)
				{
					l_result.push_back(l);
				}
			}
		}
	}

	if (l_result.empty())
	{
		return 0;
	}
	return 1;
}
int findVerticalLine(vector<Vec4i> lines_v, int up, int dowm, int ts, int ds, vector<Vec4i> &l_result,double l_th)
{
	int height = ds - ts;

	for (int i = 0; i < lines_v.size(); ++i)
	{
		Vec4i l = lines_v[i];
		int x = l[0];
		if (x < dowm)
			continue;
		if (x > up)
			break;

		if (x >= dowm && x <= up)
		{
			int length = l[3] - l[1];
			if (l[1] >= ts - 10 && l[3] <= ds + 10)
			{
				if (length >= height*l_th)
				{
					l_result.push_back(l);
				}
			}
		}
	}
	if (l_result.empty())
	{
		return 0;
	}
	return 1;
}

int overlap2lines(Vec4i l1, Vec4i l2, int lineType, double th)
{
	if (lineType)
	{
		double len1 = (double)(l1[3] - l1[1]);
		double len2 = (double)(l2[3] - l2[1]);

		int sy = max(l1[1], l2[1]);
		int ey = min(l1[3], l2[3]);

		double overl = ey > sy ? ey - sy : 0;
		double r1 = overl / len1;
		double r2 = overl / len2;

		if (r1 >= th && r2 >= th)
			return 1;
		return 0;
	}

	else
	{
		double len1 = (double)(l1[2] - l1[0]);
		double len2 = (double)(l2[2] - l2[0]);

		int sx = max(l1[0], l2[0]);
		int ex = min(l1[2], l2[2]);

		double overl = ex > sx ? ex - sx : 0;
		double r1 = overl / len1;
		double r2 = overl / len2;

		if (r1 >= th && r2 >= th)
			return 1;
		return 0;
	}
	
}

void findRectangles(vector<Vec4i> &lines_v, vector<Vec4i> &lines_h, Mat &dst, vector<Rect> &recBag)
{
	double w_h_ratio = 3.5;
	double len_th = 2;

	int minArea = 500;
	int maxArea = 480 * 360 / 2;
	////////////////*竖直线扫描*////////////////////////////////////////
	sort(lines_v.begin(), lines_v.end(), comp_x);
	sort(lines_h.begin(), lines_h.end(), comp_y);

	for (int i = 0; i < lines_v.size() - 1; ++i)
	{
		Vec4i li = lines_v[i];
		int len_i = li[3] - li[1];

		for (int j = i + 1, c_v = 0; j < lines_v.size() && c_v <= 1; ++j)
		{
			Vec4i lj = lines_v[j];
			int len_j = lj[3] - lj[1];

			double len_r = (double)len_i / len_j;
			if (len_r <= 1/len_th || len_r >= len_th)
			{
				continue;
			}

			int distance_H = lj[0] - li[0];
			if (distance_H > 10 && distance_H < 400)
			{
				int dy_top = abs(lj[1] - li[1]);
				int dy_bottom = abs(lj[3] - li[3]);

				int width = distance_H;
				int x = li[0];
				int ls = li[0];
				int rs = lj[0];

				int s_top = li[1], b_top = lj[1];
				int s_bottom = li[3], b_bottom = lj[3];
				if (li[1] > lj[1])
				{
					s_top = lj[1];
					b_top = li[1];
				}
				if (li[3] > lj[3])
				{
					s_bottom = lj[3];
					b_bottom = li[3];
				}

				if (dy_top <= 10 && dy_bottom <= 10)
				{
					c_v++;

					int y = s_top;
					int height = b_bottom - s_top;
					Rect rec(x, y, width, height);
					double ratio = (double)width / height;
					if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
					{
						rectangle(dst, rec, Scalar(0, 0, 255));
						recBag.push_back(rec);
					}
				}

				if (dy_top <= 10 && dy_bottom > 10)
				{
					c_v++;

					int y = s_top;

					int up_bottom = b_bottom + 10;
					int down_bottom = s_bottom - 10;

					vector<Vec4i> l_h_bottom;
					int f_bottom = 0;
					f_bottom = findHorizonLine(lines_h, up_bottom, down_bottom, ls, rs, l_h_bottom);

					if (f_bottom)
					{
						for (int k = 0; k < l_h_bottom.size(); ++k)
						{
							int height = l_h_bottom[k][1] - s_top;
							Rect rec(x, y, width, height);
							double ratio = (double)width / height;
							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec, Scalar(0, 0, 255));
								recBag.push_back(rec);
							}
						}
						l_h_bottom.clear();
					}
					else
					{
						int up_top = b_top + 10;
						int down_top = s_top - 10;
						vector<Vec4i> l_h_top;
						int f_top = 0;
						f_top = findHorizonLine(lines_h, up_top, down_top, ls, rs, l_h_top);
						if (f_top)
						{
							int height = s_bottom - y;
							Rect rec_1(x, y, width, height);
							double ratio = (double)width / height;
							if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_1, Scalar(0, 0, 255));
								recBag.push_back(rec_1);
							}

							height = b_bottom - y;
							Rect rec_2(x, y, width, height);
							ratio = (double)width / height;
							if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_2, Scalar(0, 0, 255));
								recBag.push_back(rec_2);
							}
						}
					}

				}

				if (dy_top > 10 && dy_bottom <= 10)
				{
					c_v++;

					int up_top = b_top + 10;
					int down_top = s_top - 10;

					vector<Vec4i> l_h_top;
					int f_top = 0;
					f_top = findHorizonLine(lines_h, up_top, down_top, ls, rs, l_h_top);

					if (f_top)
					{
						for (int k = 0; k < l_h_top.size(); ++k)
						{
							int y = l_h_top[k][1];
							int height = b_bottom - y;
							Rect rec(x, y, width, height);
							double ratio = (double)width / height;
							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec, Scalar(0, 0, 255));
								recBag.push_back(rec);
							}
						}
						l_h_top.clear();
					}

					else
					{
						int up_bottom = b_bottom + 10;
						int down_bottom = s_bottom - 10;

						vector<Vec4i> l_h_bottom;
						int f_bottom = 0;
						f_bottom = findHorizonLine(lines_h, up_bottom, down_bottom, ls, rs, l_h_bottom);
						if (f_bottom)
						{
							int y = s_top;
							int height = b_bottom - y;

							Rect rec_1(x, y, width, height);
							double ratio = (double)width / height;
							if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_1, Scalar(0, 0, 255));
								recBag.push_back(rec_1);
							}

							y = b_top;
							height = b_bottom - y;
							Rect rec_2(x, y, width, height);
							ratio = (double)width / height;
							if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_2, Scalar(0, 0, 255));
								recBag.push_back(rec_2);
							}
						}
					}

				}

			}
		}
	}

	//////////////////////*水平线扫描*//////////////////////////////////////////////////////
	for (int i = 0; i != lines_h.size() - 1; ++i)
	{
		Vec4i li = lines_h[i];
		int len_i = lines_h[i][2] - lines_h[i][0];

		for (int j = i + 1, c_h = 0; j < lines_h.size() && c_h <= 1; ++j)
		{
			Vec4i lj = lines_h[j];
			int len_j = lj[2] - lj[0];

			double len_r = (double)len_i / len_j;
			if (len_r <= 1 / len_th || len_r >= len_th)
			{
				continue;
			}

			int distance_V = lj[1] - li[1];
			if (distance_V >= 10)
			{
				int dx_left = abs(lj[0] - li[0]);
				int dx_right = abs(lj[2] - li[2]);

				int height = distance_V;
				int width = 0;
				int y = li[1];
				int ts = li[1];
				int ds = lj[1];

				int s_left = li[0], b_left = lj[0];
				int s_right = li[2], b_right = lj[2];

				if (li[0] > lj[0])
				{
					s_left = lj[0];
					b_left = li[0];
				}
				if (li[2] > lj[2])
				{
					s_right = lj[2];
					b_right = li[2];
				}

				if (dx_left <= 10 && dx_right <= 10)
				{
					c_h++;

					int x = s_left;
					width = b_right - s_left;
					Rect rec(x, y, width, height);
					double ratio = (double)width / height;
					if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
					{
						rectangle(dst, rec, Scalar(0, 255, 255));
						recBag.push_back(rec);
					}
				}

				if (dx_left > 10 && dx_right <= 10)
				{
					c_h++;

					int up_left = b_left + 10;
					int down_left = s_left - 10;

					int f_left = 0;
					vector<Vec4i> l_v_left;
					f_left = findVerticalLine(lines_v, up_left, down_left, ts, ds, l_v_left);
					if (f_left)
					{
						for (int k = 0; k < l_v_left.size(); ++k)
						{
							int x = l_v_left[k][0];
							width = b_right - l_v_left[k][0];
							Rect rec(x, y, width, height);
							double ratio = (double)width / height;
							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec, Scalar(0, 255, 255));
								recBag.push_back(rec);
							}
						}
						l_v_left.clear();
					}
					else
					{
						int up_right = b_right + 10;
						int down_right = s_right - 10;

						vector<Vec4i> l_v_right;
						int f_right = 0;
						f_right = findVerticalLine(lines_v, up_right, down_right, ts, ds, l_v_right);
						if (f_right)
						{
							int x = s_left;
							int width = b_right - x;
							Rect rec_1(x, y, width, height);
							double ratio = (double)width / height;
							if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_1, Scalar(0, 255, 255));
								recBag.push_back(rec_1);
							}

							x = b_left;
							width = b_right - x;
							Rect rec_2(x, y, width, height);
							ratio = (double)width / height;
							if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_2, Scalar(0, 255, 255));
								recBag.push_back(rec_2);
							}
						}

					}

				}

				if (dx_left <= 10 && dx_right > 10)
				{
					c_h++;

					int up_right = b_right + 10;
					int down_right = s_right - 10;

					vector<Vec4i> l_v_right;
					int f_right = 0;
					f_right = findVerticalLine(lines_v, up_right, down_right, ts, ds, l_v_right);

					if (f_right)
					{
						for (int k = 0; k < l_v_right.size(); ++k)
						{
							int x = s_left;
							int y = li[1];
							width = l_v_right[k][0] - s_left;
							Rect rec(x, y, width, height);
							double ratio = (double)width / height;
							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec, Scalar(0, 255, 255));
								recBag.push_back(rec);
							}
						}
						l_v_right.clear();
					}
					else
					{
						int up_left = b_left + 10;
						int down_left = s_left - 10;

						int f_left = 0;
						vector<Vec4i> l_v_left;
						f_left = findVerticalLine(lines_v, up_left, down_left, ts, ds, l_v_left);
						if (f_left)
						{
							int x = s_left;
							int width = b_right - x;
							Rect rec_1(x, y, width, height);
							double ratio = (double)width / height;
							if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_1, Scalar(0, 255, 255));
								recBag.push_back(rec_1);
							}

							width = s_right - x;
							Rect rec_2(x, y, width, height);
							ratio = (double)width / height;
							if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_2, Scalar(0, 255, 255));
								recBag.push_back(rec_2);
							}
						}
					}

				}

			}
		}
	}
}

void findRectangles_2(vector<Vec4i> &lines_v, vector<Vec4i> &lines_h, Mat &dst, vector<Rect> &recBag)
{
	double w_h_ratio = 3.5;
	double len_th = 4;
	int numCons = 3;

	int minArea = 400;
	int maxArea = 480 * 360 / 2;
	////////////////*竖直线扫描*///////////////////////////////////////
	sort(lines_v.begin(), lines_v.end(), comp_x);
	sort(lines_h.begin(), lines_h.end(), comp_y);

	for (int i = 0; i < lines_v.size() - 1; ++i)
	{
		Vec4i li = lines_v[i];
		int len_i = li[3] - li[1];

		for (int j = i + 1, c_v = 0; j < lines_v.size() && c_v <= numCons; ++j)
		{
			Vec4i lj = lines_v[j];
			int xj = lj[0];
			int len_j = lj[3] - lj[1];

			double len_r = (double)len_i / len_j;
			
			if (len_r <= 1 / len_th || len_r >= len_th)
			{
				continue;
			}

			int distance_H = abs(lj[0] - li[0]);
			if (distance_H >= 10)
			{
				int dy_top = abs(lj[1] - li[1]);
				int dy_bottom = abs(lj[3] - li[3]);

				int width = distance_H;
				int x = li[0];
				int ls = li[0];
				int rs = lj[0];

				int s_top = li[1], b_top = lj[1];
				int s_bottom = li[3], b_bottom = lj[3];
				if (li[1] > lj[1])
				{
					s_top = lj[1];
					b_top = li[1];
				}
				if (li[3] > lj[3])
				{
					s_bottom = lj[3];
					b_bottom = li[3];
				}

				if (dy_top <= 10 && dy_bottom <= 10)
				{
					int y = s_top;
					int height = b_bottom - s_top;
					Rect rec(x, y, width, height);
					double ratio = (double)width / height;
					if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
					{
						c_v++;

						rectangle(dst, rec, Scalar(0, 0, 255));
						recBag.push_back(rec);
					}
				}

				if (dy_top <= 10 && dy_bottom > 10)
				{
					int y = s_top;

					int up_bottom = b_bottom + 10;
					int down_bottom = s_bottom - 10;

					vector<Vec4i> l_h_bottom;
					int f_bottom = 0;
					f_bottom = findHorizonLine(lines_h, up_bottom, down_bottom, ls, rs, l_h_bottom);

					if (f_bottom)
					{
						c_v++;
						for (int k = 0; k < l_h_bottom.size(); ++k)
						{
							int height = l_h_bottom[k][1] - s_top;
							Rect rec(x, y, width, height);
							double ratio = (double)width / height;
							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec, Scalar(0, 0, 255));
								recBag.push_back(rec);
							}
						}
						l_h_bottom.clear();
					}
					else
					{
						int up_top = b_top + 10;
						int down_top = s_top - 10;
						vector<Vec4i> l_h_top;
						int f_top = 0;
						f_top = findHorizonLine(lines_h, up_top, down_top, ls, rs, l_h_top);
						if (f_top)
						{
							c_v++;

							int height = s_bottom - y;
							Rect rec_1(x, y, width, height);
							double ratio = (double)width / height;
							if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_1, Scalar(0, 0, 255));
								recBag.push_back(rec_1);
							}

							height = b_bottom - y;
							Rect rec_2(x, y, width, height);
							ratio = (double)width / height;
							if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_2, Scalar(0, 0, 255));
								recBag.push_back(rec_2);
							}
						}
					}

				}

				if (dy_top > 10 && dy_bottom <= 10)
				{
					int up_top = b_top + 10;
					int down_top = s_top - 10;

					vector<Vec4i> l_h_top;
					int f_top = 0;
					f_top = findHorizonLine(lines_h, up_top, down_top, ls, rs, l_h_top);

					if (f_top)
					{
						c_v++;

						for (int k = 0; k < l_h_top.size(); ++k)
						{
							int y = l_h_top[k][1];
							int height = b_bottom - y;
							Rect rec(x, y, width, height);
							double ratio = (double)width / height;
							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec, Scalar(0, 0, 255));
								recBag.push_back(rec);
							}
						}
						l_h_top.clear();
					}

					else
					{
						int up_bottom = b_bottom + 10;
						int down_bottom = s_bottom - 10;

						vector<Vec4i> l_h_bottom;
						int f_bottom = 0;
						f_bottom = findHorizonLine(lines_h, up_bottom, down_bottom, ls, rs, l_h_bottom);
						if (f_bottom)
						{
							c_v++;

							int y = s_top;
							int height = b_bottom - y;

							Rect rec_1(x, y, width, height);
							double ratio = (double)width / height;
							if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_1, Scalar(0, 0, 255));
								recBag.push_back(rec_1);
							}

							y = b_top;
							height = b_bottom - y;
							Rect rec_2(x, y, width, height);
							ratio = (double)width / height;
							if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_2, Scalar(0, 0, 255));
								recBag.push_back(rec_2);
							}
						}
					}

				}

			}
		}
	}

	//////////////////////*水平线扫描*//////////////////////////////////////////////////////
	for (int i = 0; i != lines_h.size() - 1; ++i)
	{
		Vec4i li = lines_h[i];
		int len_i = lines_h[i][2] - lines_h[i][0];

		for (int j = i + 1, c_h = 0; j < lines_h.size() && c_h <= numCons; ++j)
		{
			Vec4i lj = lines_h[j];
			int len_j = lj[2] - lj[0];

			double len_r = (double)len_i / len_j;
	
			if (len_r <= 1 / len_th || len_r >= len_th)
			{
				continue;
			}

			int distance_V = abs(lj[1] - li[1]);
			if (distance_V >= 10)
			{
				int dx_left = abs(lj[0] - li[0]);
				int dx_right = abs(lj[2] - li[2]);

				int height = distance_V;
				int width = 0;
				int y = li[1];
				int ts = li[1];
				int ds = lj[1];

				int s_left = li[0], b_left = lj[0];
				int s_right = li[2], b_right = lj[2];

				if (li[0] > lj[0])
				{
					s_left = lj[0];
					b_left = li[0];
				}
				if (li[2] > lj[2])
				{
					s_right = lj[2];
					b_right = li[2];
				}

				if (dx_left <= 10 && dx_right <= 10)
				{
					c_h++;

					int x = s_left;
					width = b_right - s_left;
					Rect rec(x, y, width, height);
					double ratio = (double)width / height;
					if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
					{
						rectangle(dst, rec, Scalar(0, 255, 255));
						recBag.push_back(rec);
					}
				}

				if (dx_left > 10 && dx_right <= 10)
				{
					int up_left = b_left + 10;
					int down_left = s_left - 10;

					int f_left = 0;
					vector<Vec4i> l_v_left;
					f_left = findVerticalLine(lines_v, up_left, down_left, ts, ds, l_v_left);
					if (f_left)
					{
						c_h++;

						for (int k = 0; k < l_v_left.size(); ++k)
						{
							int x = l_v_left[k][0];
							width = b_right - l_v_left[k][0];
							Rect rec(x, y, width, height);
							double ratio = (double)width / height;
							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec, Scalar(0, 255, 255));
								recBag.push_back(rec);
							}
						}
						l_v_left.clear();
					}
					else
					{
						int up_right = b_right + 10;
						int down_right = s_right - 10;

						vector<Vec4i> l_v_right;
						int f_right = 0;
						f_right = findVerticalLine(lines_v, up_right, down_right, ts, ds, l_v_right);
						if (f_right)
						{
							c_h++;

							int x = s_left;
							int width = b_right - x;
							Rect rec_1(x, y, width, height);
							double ratio = (double)width / height;
							if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_1, Scalar(0, 255, 255));
								recBag.push_back(rec_1);
							}

							x = b_left;
							width = b_right - x;
							Rect rec_2(x, y, width, height);
							ratio = (double)width / height;
							if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_2, Scalar(0, 255, 255));
								recBag.push_back(rec_2);
							}
						}

					}

				}

				if (dx_left <= 10 && dx_right > 10)
				{
					int up_right = b_right + 10;
					int down_right = s_right - 10;

					vector<Vec4i> l_v_right;
					int f_right = 0;
					f_right = findVerticalLine(lines_v, up_right, down_right, ts, ds, l_v_right);

					if (f_right)
					{
						c_h++;

						for (int k = 0; k < l_v_right.size(); ++k)
						{
							int x = s_left;
							int y = li[1];
							width = l_v_right[k][0] - s_left;
							Rect rec(x, y, width, height);
							double ratio = (double)width / height;
							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec, Scalar(0, 255, 255));
								recBag.push_back(rec);
							}
						}
						l_v_right.clear();
					}
					else
					{
						int up_left = b_left + 10;
						int down_left = s_left - 10;

						int f_left = 0;
						vector<Vec4i> l_v_left;
						f_left = findVerticalLine(lines_v, up_left, down_left, ts, ds, l_v_left);
						if (f_left)
						{
							c_h++;

							int x = s_left;
							int width = b_right - x;
							Rect rec_1(x, y, width, height);
							double ratio = (double)width / height;
							if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_1, Scalar(0, 255, 255));
								recBag.push_back(rec_1);
							}

							width = s_right - x;
							Rect rec_2(x, y, width, height);
							ratio = (double)width / height;
							if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_2, Scalar(0, 255, 255));
								recBag.push_back(rec_2);
							}
						}
					}

				}

			}
		}
	}
}

void findRectangles_3(vector<Vec4i> &lines_v, vector<Vec4i> &lines_h, Mat &dst, vector<Rect> &recBag)
{
	double w_h_ratio = 3.5;
	double len_th = 4;
	int numCons = 3;
	double overlenth = 0.75;

	int minArea = 400;
	int maxArea = 480 * 360 / 2;
	////////////////*竖直线扫描*///////////////////////////////////////
	sort(lines_v.begin(), lines_v.end(), comp_x);
	sort(lines_h.begin(), lines_h.end(), comp_y);

	for (int i = 0; i < lines_v.size() - 1; ++i)
	{
		Vec4i li = lines_v[i];
		int len_i = li[3] - li[1];

		for (int j = i + 1, c_v = 0; j < lines_v.size() && c_v <= numCons; ++j)
		{
			Vec4i lj = lines_v[j];
			int xj = lj[0];
			int len_j = lj[3] - lj[1];

			double len_r = (double)len_i / len_j;

			if (len_r <= 1 / len_th || len_r >= len_th)
			{
				continue;
			}

			int distance_H = abs(lj[0] - li[0]);
			if (distance_H >= 10)
			{
				int dy_top = abs(lj[1] - li[1]);
				int dy_bottom = abs(lj[3] - li[3]);

				int width = distance_H;
				int x = li[0];

				int ls = li[0];
				int rs = lj[0];

				int s_top = li[1], b_top = lj[1];
				int s_bottom = li[3], b_bottom = lj[3];
				if (li[1] > lj[1])
				{
					s_top = lj[1];
					b_top = li[1];
				}
				if (li[3] > lj[3])
				{
					s_bottom = lj[3];
					b_bottom = li[3];
				}

				if (dy_top <= 10 && dy_bottom <= 10)
				{
					int y = s_top;
					int height = b_bottom - y;
					Rect rec(x, y, width, height);
					double ratio = (double)width / height;
					if (rec.area() >= minArea && rec.area() <= maxArea && ratio <= w_h_ratio && ratio >= 1 / w_h_ratio)
					{
						c_v++;

						rectangle(dst, rec, Scalar(0, 0, 255));
						recBag.push_back(rec);
					}
				}

				else if (dy_top <= 10 && dy_bottom > 10)
				{			
#pragma region "(dy_top <= 10 && dy_bottom > 10"	

					int y = s_top;

					int up_bottom = b_bottom + 10;
					int down_bottom = s_bottom - 10;

					vector<Vec4i> l_h_bottom;
					int f_bottom = 0;
					f_bottom = findHorizonLine(lines_h, up_bottom, down_bottom, ls, rs, l_h_bottom);

					if (f_bottom)
					{
						c_v++;

						for (int k = 0; k < l_h_bottom.size(); ++k)
						{
							int height = l_h_bottom[k][1] - s_top;
							Rect rec(x, y, width, height);
							double ratio = (double)width / height;
							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec, Scalar(0, 0, 255));
								recBag.push_back(rec);
							}
						}
						l_h_bottom.clear();
					}
					else
					{
						int up_top = b_top + 10;
						int down_top = s_top - 10;
						vector<Vec4i> l_h_top;
						int f_top = 0;
						f_top = findHorizonLine(lines_h, up_top, down_top, ls, rs, l_h_top);
						if (f_top)
						{
							c_v++;

							int height = s_bottom - y;
							Rect rec_1(x, y, width, height);
							double ratio = (double)width / height;
							if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								//rectangle(dst, rec_1, Scalar(0, 0, 255));
								recBag.push_back(rec_1);
							}

							height = b_bottom - y;
							Rect rec_2(x, y, width, height);
							ratio = (double)width / height;
							if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_2, Scalar(0, 0, 255));
								recBag.push_back(rec_2);
							}
						}
					}
#pragma endregion
				}

				else if (dy_top > 10 && dy_bottom <= 10)
				{
#pragma region "dy_top > 10 && dy_bottom <= 10"					
					
					int up_top = b_top + 10;
					int down_top = s_top - 10;

					vector<Vec4i> l_h_top;
					int f_top = 0;
					f_top = findHorizonLine(lines_h, up_top, down_top, ls, rs, l_h_top);

					if (f_top)
					{
						c_v++;

						for (int k = 0; k < l_h_top.size(); ++k)
						{
							int y = l_h_top[k][1];
							int height = b_bottom - y;
							Rect rec(x, y, width, height);
							double ratio = (double)width / height;
							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec, Scalar(0, 0, 255));
								recBag.push_back(rec);
							}
						}
						l_h_top.clear();
					}
					else
					{
						int up_bottom = b_bottom + 10;
						int down_bottom = s_bottom - 10;

						vector<Vec4i> l_h_bottom;
						int f_bottom = 0;
						f_bottom = findHorizonLine(lines_h, up_bottom, down_bottom, ls, rs, l_h_bottom);
						if (f_bottom)
						{
							c_v++;

							int y = s_top;
							int height = b_bottom - y;

							Rect rec_1(x, y, width, height);
							double ratio = (double)width / height;
							if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_1, Scalar(0, 0, 255));
								recBag.push_back(rec_1);
							}

							y = b_top;
							height = b_bottom - y;
							Rect rec_2(x, y, width, height);
							ratio = (double)width / height;
							if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								//rectangle(dst, rec_2, Scalar(0, 0, 255));
								recBag.push_back(rec_2);
							}
						}
					}
					
#pragma endregion
				}

				else if (overlap2lines(li,lj,1,overlenth))
				{
#pragma region "overlap"

					int up_top = b_top + 10;
					int down_top = s_top - 10;

					vector<Vec4i> l_h_top;
					int f_top = 0;
					f_top = findHorizonLine(lines_h, up_top, down_top, ls, rs, l_h_top);

					if (f_top)
					{
						c_v++;

						for (int k = 0; k < l_h_top.size(); ++k)
						{
							int y = l_h_top[k][1];
							int height = b_bottom - y;
							Rect rec(x, y, width, height);
							double ratio = (double)width / height;
							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec, Scalar(0, 0, 255));
								recBag.push_back(rec);
							}
						}
						l_h_top.clear();
					}

					{
						int up_bottom = b_bottom + 10;
						int down_bottom = s_bottom - 10;

						vector<Vec4i> l_h_bottom;
						int f_bottom = 0;
						f_bottom = findHorizonLine(lines_h, up_bottom, down_bottom, ls, rs, l_h_bottom);
						if (f_bottom)
						{
							c_v++;

							for (int k = 0; k < l_h_bottom.size(); ++k)
							{
								int y = s_top;
								int height = l_h_bottom[k][1] - s_top;
								Rect rec(x, y, width, height);
								double ratio = (double)width / height;
								if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
								{
									rectangle(dst, rec, Scalar(0, 0, 255));
									recBag.push_back(rec);
								}
							}
							l_h_bottom.clear();
						}
					}

#pragma endregion		
				}

			}
		}
	}

	//////////////////////*水平线扫描*//////////////////////////////////////////////////////
	for (int i = 0; i != lines_h.size() - 1; ++i)
	{
		Vec4i li = lines_h[i];
		int len_i = lines_h[i][2] - lines_h[i][0];

		for (int j = i + 1, c_h = 0; j < lines_h.size() && c_h <= numCons; ++j)
		{
			Vec4i lj = lines_h[j];
			int len_j = lj[2] - lj[0];

			double len_r = (double)len_i / len_j;

			if (len_r <= 1 / len_th || len_r >= len_th)
			{
				continue;
			}

			int distance_V = abs(lj[1] - li[1]);
			if (distance_V >= 10)
			{
				int dx_left = abs(lj[0] - li[0]);
				int dx_right = abs(lj[2] - li[2]);

				int height = distance_V;
				int y = li[1];

				int ts = li[1];
				int ds = lj[1];

				int s_left = li[0], b_left = lj[0];
				int s_right = li[2], b_right = lj[2];

				if (li[0] > lj[0])
				{
					s_left = lj[0];
					b_left = li[0];
				}
				if (li[2] > lj[2])
				{
					s_right = lj[2];
					b_right = li[2];
				}

				if (dx_left <= 10 && dx_right <= 10)
				{
					c_h++;

					int x = s_left;
					int width = b_right - x;
					Rect rec(x, y, width, height);
					double ratio = (double)width / height;
					if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
					{
						rectangle(dst, rec, Scalar(0, 255, 255));
						recBag.push_back(rec);
					}
				}

				else if (dx_left > 10 && dx_right <= 10)
				{
#pragma region "dx_left > 10 && dx_right <= 10"

					int up_left = b_left + 10;
					int down_left = s_left - 10;

					int f_left = 0;
					vector<Vec4i> l_v_left;
					f_left = findVerticalLine(lines_v, up_left, down_left, ts, ds, l_v_left);
					if (f_left)
					{
						c_h++;

						for (int k = 0; k < l_v_left.size(); ++k)
						{
							int x = l_v_left[k][0];
							int width = b_right - l_v_left[k][0];
							Rect rec(x, y, width, height);
							double ratio = (double)width / height;
							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec, Scalar(0, 255, 255));
								recBag.push_back(rec);
							}
						}
						l_v_left.clear();
					}
					else
					{
						int up_right = b_right + 10;
						int down_right = s_right - 10;

						vector<Vec4i> l_v_right;
						int f_right = 0;
						f_right = findVerticalLine(lines_v, up_right, down_right, ts, ds, l_v_right);
						if (f_right)
						{
							c_h++;

							int x = s_left;
							int width = b_right - x;
							Rect rec_1(x, y, width, height);
							double ratio = (double)width / height;
							if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_1, Scalar(0, 255, 255));
								recBag.push_back(rec_1);
							}

							x = b_left;
							width = b_right - x;
							Rect rec_2(x, y, width, height);
							ratio = (double)width / height;
							if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								//rectangle(dst, rec_2, Scalar(0, 255, 255));
								recBag.push_back(rec_2);
							}
						}

					}
#pragma endregion
				}

				else if (dx_left <= 10 && dx_right > 10)
				{
#pragma region "dx_left <= 10 && dx_right > 10"

					int up_right = b_right + 10;
					int down_right = s_right - 10;

					vector<Vec4i> l_v_right;
					int f_right = 0;
					f_right = findVerticalLine(lines_v, up_right, down_right, ts, ds, l_v_right);

					if (f_right)
					{
						c_h++;

						for (int k = 0; k < l_v_right.size(); ++k)
						{
							int x = s_left;
							int y = li[1];
							int width = l_v_right[k][0] - s_left;
							Rect rec(x, y, width, height);
							double ratio = (double)width / height;
							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec, Scalar(0, 255, 255));
								recBag.push_back(rec);
							}
						}
						l_v_right.clear();
					}
					
					else
					{
						int up_left = b_left + 10;
						int down_left = s_left - 10;

						int f_left = 0;
						vector<Vec4i> l_v_left;
						f_left = findVerticalLine(lines_v, up_left, down_left, ts, ds, l_v_left);
						if (f_left)
						{
							c_h++;

							int x = s_left;
							int width = b_right - x;
							Rect rec_1(x, y, width, height);
							double ratio = (double)width / height;
							if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec_1, Scalar(0, 255, 255));
								recBag.push_back(rec_1);
							}

							width = s_right - x;
							Rect rec_2(x, y, width, height);
							ratio = (double)width / height;
							if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								//rectangle(dst, rec_2, Scalar(0, 255, 255));
								recBag.push_back(rec_2);
							}
						}
					}
#pragma endregion
				}

				else if (overlap2lines(li, lj, 0, overlenth))
				{
#pragma region "overlap"

					int up_right = b_right + 10;
					int down_right = s_right - 10;

					vector<Vec4i> l_v_right;
					int f_right = 0;
					f_right = findVerticalLine(lines_v, up_right, down_right, ts, ds, l_v_right);

					if (f_right)
					{
						c_h++;

						for (int k = 0; k < l_v_right.size(); ++k)
						{
							int x = s_left;
							int y = li[1];
							int width = l_v_right[k][0] - s_left;
							Rect rec(x, y, width, height);
							double ratio = (double)width / height;
							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec, Scalar(0, 255, 255));
								recBag.push_back(rec);
							}
						}
						l_v_right.clear();
					}

					int up_left = b_left + 10;
					int down_left = s_left - 10;

					int f_left = 0;
					vector<Vec4i> l_v_left;
					f_left = findVerticalLine(lines_v, up_left, down_left, ts, ds, l_v_left);
					if (f_left)
					{
						c_h++;

						for (int k = 0; k < l_v_left.size(); ++k)
						{
							int x = l_v_left[k][0];
							int width = b_right - l_v_left[k][0];
							Rect rec(x, y, width, height);
							double ratio = (double)width / height;
							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
							{
								rectangle(dst, rec, Scalar(0, 255, 255));
								recBag.push_back(rec);
							}
						}
						l_v_left.clear();
					}
#pragma endregion 
				}

			}
		}
	}
}

void findRectsTwoLines(vector<Vec4i> &lines_v, vector<Vec4i> &lines_h, Mat &dst, vector<Rect> &recBag)
{
	double w_h_ratio = 3.5;
	double len_th = 4;

	int minArea = 500;
	int maxArea = 480 * 360 / 2;

	////////////////*竖直线扫描*////////////////////////////////////////
	sort(lines_v.begin(), lines_v.end(), comp_x);
	sort(lines_h.begin(), lines_h.end(), comp_y);

	for (int i = 0; i < lines_v.size() - 1; ++i)
	{
		Vec4i li = lines_v[i];
		int len_i = li[3] - li[1];

		for (int j = i + 1; j < lines_v.size(); ++j)
		{
			Vec4i lj = lines_v[j];
			int len_j = lj[3] - lj[1];

			double len_r = (double)len_i / len_j;
			if (len_r <= 1 / len_th || len_r >= len_th)
			{
				continue;
			}

			int distance_H = lj[0] - li[0];
			if (distance_H > 10 && distance_H < 400)
			{
				int dy_top = abs(lj[1] - li[1]);
				int dy_bottom = abs(lj[3] - li[3]);

				int width = distance_H;
				int x = li[0];

				int s_top = li[1], b_top = lj[1];
				int s_bottom = li[3], b_bottom = lj[3];
				if (li[1] > lj[1])
				{
					s_top = lj[1];
					b_top = li[1];
				}
				if (li[3] > lj[3])
				{
					s_bottom = lj[3];
					b_bottom = li[3];
				}

				if (dy_top <= 10 && dy_bottom <= 10)
				{
					int y = s_top;
					int height = b_bottom - s_top;
					Rect rec(x, y, width, height);
					double ratio = (double)width / height;
					if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
					{
						rectangle(dst, rec, Scalar(0, 0, 255));
						recBag.push_back(rec);
					}
				}

				if (dy_top <= 10 && dy_bottom > 10)
				{

					int y = s_top;
					int height = s_bottom - y;
					Rect rec_1(x, y, width, height);
					double ratio = (double)width / height;
					if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
					{
						rectangle(dst, rec_1, Scalar(0, 0, 255));
						recBag.push_back(rec_1);
					}

					height = b_bottom - y;
					Rect rec_2(x, y, width, height);
					ratio = (double)width / height;
					if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
					{
						rectangle(dst, rec_2, Scalar(0, 0, 255));
						recBag.push_back(rec_2);
					}

				}

				if (dy_top > 10 && dy_bottom <= 10)
				{
					int y = s_top;
					int height = b_bottom - y;

					Rect rec_1(x, y, width, height);
					double ratio = (double)width / height;
					if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
					{
						rectangle(dst, rec_1, Scalar(0, 0, 255));
						recBag.push_back(rec_1);
					}

					y = b_top;
					height = b_bottom - y;
					Rect rec_2(x, y, width, height);
					ratio = (double)width / height;
					if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
					{
						rectangle(dst, rec_2, Scalar(0, 0, 255));
						recBag.push_back(rec_2);
					}
			
				}

			}
		}
	}

	//////////////////////*水平线扫描*//////////////////////////////////////////////////////
	for (int i = 0; i != lines_h.size() - 1; ++i)
	{
		Vec4i li = lines_h[i];
		int len_i = lines_h[i][2] - lines_h[i][0];

		for (int j = i + 1; j < lines_h.size(); ++j)
		{
			Vec4i lj = lines_h[j];
			int len_j = lj[2] - lj[0];

			double len_r = (double)len_i / len_j;
			if (len_r <= 1 / len_th || len_r >= len_th)
			{
				continue;
			}

			int distance_V = lj[1] - li[1];
			if (distance_V >= 10)
			{
				int dx_left = abs(lj[0] - li[0]);
				int dx_right = abs(lj[2] - li[2]);

				int height = distance_V;
				int y = li[1];
			
				int s_left = li[0], b_left = lj[0];
				int s_right = li[2], b_right = lj[2];

				if (li[0] > lj[0])
				{
					s_left = lj[0];
					b_left = li[0];
				}
				if (li[2] > lj[2])
				{
					s_right = lj[2];
					b_right = li[2];
				}

				if (dx_left <= 10 && dx_right <= 10)
				{
					int x = s_left;
					int width = b_right - s_left;
					Rect rec(x, y, width, height);
					double ratio = (double)width / height;
					if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
					{
						rectangle(dst, rec, Scalar(0, 255, 255));
						recBag.push_back(rec);
					}
				}

				if (dx_left > 10 && dx_right <= 10)
				{
					int x = s_left;
					int width = b_right - x;
					Rect rec_1(x, y, width, height);
					double ratio = (double)width / height;
					if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
					{
						rectangle(dst, rec_1, Scalar(0, 255, 255));
						recBag.push_back(rec_1);
					}

					x = b_left;
					width = b_right - x;
					Rect rec_2(x, y, width, height);
					ratio = (double)width / height;
					if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
					{
						rectangle(dst, rec_2, Scalar(0, 255, 255));
						recBag.push_back(rec_2);
					}

				}

				if (dx_left <= 10 && dx_right > 10)
				{
					
					int x = s_left;
					int width = b_right - x;
					Rect rec_1(x, y, width, height);
					double ratio = (double)width / height;
					if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
					{
						rectangle(dst, rec_1, Scalar(0, 255, 255));
						recBag.push_back(rec_1);
					}

					width = s_right - x;
					Rect rec_2(x, y, width, height);
					ratio = (double)width / height;
					if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
					{
						rectangle(dst, rec_2, Scalar(0, 255, 255));
						recBag.push_back(rec_2);
					}

				}

			}
		}
	}
}

void fixCloseLinesButSave(vector<Vec4i> &lines, int lineType, int posi_th)
{
	if (lines.empty())
		return;
	switch (lineType)
	{

#pragma region case_0:lines_h
	case 0:
	{
		vector<Vec4i> lines_new;
		vector<int> used(lines.size(), 0);

		sort(lines.begin(), lines.end(), comp_length_h);

		for (int i = 0; i != lines.size(); ++i)
		{
			if (used[i] == 1)
				continue;

			int seed_y = lines[i][1];
			int sx = lines[i][0];
			int ex = lines[i][2];

			vector<line_label> lines_label;
			line_label lla;

			for (int j = i + 1; j != lines.size(); ++j)
			{
				int y = lines[j][1];

				if (used[j] == 0 && y >= seed_y - posi_th && y <= seed_y + posi_th)
				{
					if (lines[j][0] >= sx && lines[j][2] <= ex)
					{
						used[j] = 1;
					}

					else
					{
						lla.l = lines[j];
						lla.label = j;
						lines_label.push_back(lla);
					}
				}
			}

			if (lines_label.empty())
				continue;

			lla.l = lines[i];
			lla.label = i;
			lines_label.push_back(lla);

			sort(lines_label.begin(), lines_label.end(), lla_comp_x_t);

			int k = 0;
			while (k < lines_label.size() && lines_label[k].l[0] != sx) k++;
			int end = ex;
			int start = sx;

			for (int i = k + 1; i < lines_label.size(); ++i)
			{
				if (lines_label[i].l[0] - end <= posi_th)
				{
					used[lines_label[i].label] = 1;
					if (lines_label[i].l[2] > end)
					{
						end = lines_label[i].l[2];
					}
				}
				else
					break;
			}

			for (int i = k - 1; i >= 0; --i)
			{
				if (start - lines_label[i].l[2] <= posi_th)
				{
					used[lines_label[i].label] = 1;
					if (lines_label[i].l[0] < start)
					{
						start = lines_label[i].l[0];
					}
				}
				else
					break;
			}

			lines_label.clear();
			if (lines[i][0] != start || lines[i][2] != end)
			{
				Vec4i l;
				l[0] = start;
				l[1] = lines[i][1];
				l[2] = end;
				l[3] = lines[i][3];
				lines_new.push_back(l);
			}		
		}

		for (int i = 0; i < lines_new.size(); ++i)
		{
			lines.push_back(lines_new[i]);
		}

	}; break;

#pragma endregion

#pragma region case_1:lines_v
	case 1:
	{
		vector<Vec4i> lines_new;
		vector<int> used(lines.size(), 0);

		sort(lines.begin(), lines.end(), comp_length_v);

		for (int i = 0; i != lines.size(); ++i)
		{
			if (used[i] == 1)
				continue;

			int seed_x = lines[i][0];
			int ey = lines[i][3];
			int sy = lines[i][1];

			vector<line_label> lines_label;
			line_label lla;

			for (int j = i + 1; j != lines.size(); ++j)
			{
				int x = lines[j][0];

				if (used[j] == 0 && x >= seed_x - posi_th && x <= seed_x + posi_th)
				{
					if (lines[j][3] <= ey && lines[j][1] >= sy)
					{
						used[j] = 1;
					}

					else
					{
						lla.l = lines[j];
						lla.label = j;
						lines_label.push_back(lla);
					}
				}
			}

			if (lines_label.empty())
				continue;

			lla.l = lines[i];
			lla.label = i;
			lines_label.push_back(lla);

			sort(lines_label.begin(), lines_label.end(), lla_comp_y_r);

			int k = 0;
			while (k < lines_label.size() && lines_label[k].l[1] != sy) k++;
			int end = ey;
			int start = sy;

			for (int i = k + 1; i < lines_label.size(); ++i)
			{
				if (lines_label[i].l[1] - end <= posi_th)
				{
					used[lines_label[i].label] = 1;
					if (lines_label[i].l[3] > end);
					{
						end = lines_label[i].l[3];
					}
				}
				else
					break;
			}

			for (int i = k - 1; i >= 0; --i)
			{
				if (start - lines_label[i].l[3] <= posi_th)
				{
					used[lines_label[i].label] = 1;
					if (lines_label[i].l[1] < start)
					{
						start = lines_label[i].l[1];
					}
				}
				else
					break;
			}

			lines_label.clear();

			if (lines[i][1] != start || lines[i][3] != end)
			{
				Vec4i l;
				l[0] = lines[i][0];
				l[1] = start;
				l[2] = lines[i][2];
				l[3] = end;
				lines_new.push_back(l);
			}
		}

		for (int i = 0; i < lines_new.size(); ++i)
		{
			lines.push_back(lines_new[i]);
		}
	}; break;

#pragma endregion

	}
}

void fixSameLines_2(vector<Vec4i> &lines, int lineType, double over_th,double d_th)
{
	if (lines.empty())
		return;
	switch (lineType)
	{

#pragma region case_0:lines_h
	case 0:
	{
		vector<Vec4i> lines_tmp;
		vector<int> used(lines.size(), 0);

		sort(lines.begin(), lines.end(), comp_length_h);

		for (int i = 0; i != lines.size(); ++i)
		{
			if (used[i] == 1)
				continue;
			Vec4i li = lines[i];
			int len_i = li[2] - li[0];
			int seed_y = li[1];

			for (int j = i + 1; j != lines.size(); ++j)
			{
				Vec4i lj = lines[j];
				int y = lj[1];

				int d = abs(y - seed_y);
				double r = (double)d / len_i;

				if (used[j] == 0 && overlap2lines(li,lj,0,over_th) && r<=d_th )
				{				
					used[j] = 1;				
				}
			}

		}

		for (int i = 0; i != used.size(); ++i)
		{
			if (used[i] == 0)
			{
				int len = lines[i][2] - lines[i][0];
				if (len >= 10)
					lines_tmp.push_back(lines[i]);
			}
		}
		lines.clear();
		lines.assign(lines_tmp.begin(), lines_tmp.end());
		lines_tmp.clear();

	}; break;

#pragma endregion

#pragma region case_1:lines_v
	case 1:
	{
		vector<Vec4i> lines_tmp;
		vector<int> used(lines.size(), 0);

		sort(lines.begin(), lines.end(), comp_length_v);

		for (int i = 0; i != lines.size(); ++i)
		{
			if (used[i] == 1)
				continue;

			Vec4i li = lines[i];
			int seed_x = lines[i][0];
			int len_i = li[3] - li[1];

			for (int j = i + 1; j != lines.size(); ++j)
			{
				Vec4i lj = lines[j];
				int x = lines[j][0];
				int d = abs(x - seed_x);
				double r = (double)d / len_i;

				if (used[j] == 0 && overlap2lines(li,lj,1,over_th) && r<=d_th)
				{
					
					used[j] = 1;
					
				}
			}

		}

		for (int i = 0; i != used.size(); ++i)
		{
			if (used[i] == 0)
			{
				int len = lines[i][3] - lines[i][1];
				if (len > 10)
					lines_tmp.push_back(lines[i]);
			}
		}
		lines.clear();
		lines.assign(lines_tmp.begin(), lines_tmp.end());
		lines_tmp.clear();

	}; break;

#pragma endregion

	}
}

//void findRectangles_4(vector<Vec4i> &lines_v, vector<Vec4i> &lines_h, Mat &dst, vector<Rect> &recBag)
//{
//	double w_h_ratio = 3.5;
//	double len_th = 4;
//	int numCons = 3;
//	double overlenth = 0.75;
//
//	int minArea = 400;
//	int maxArea = 480 * 360 / 2;
//	////////////////*竖直线扫描*///////////////////////////////////////
//	sort(lines_v.begin(), lines_v.end(), comp_x);
//	sort(lines_h.begin(), lines_h.end(), comp_y);
//
//	for (int i = 0; i < lines_v.size() - 1; ++i)
//	{
//		Vec4i li = lines_v[i];
//		int len_i = li[3] - li[1];
//
//		for (int j = i + 1, c_v = 0; j < lines_v.size() && c_v <= numCons; ++j)
//		{
//			Vec4i lj = lines_v[j];
//			int xj = lj[0];
//			int len_j = lj[3] - lj[1];
//
//			double len_r = (double)len_i / len_j;
//
//			if (len_r <= 1 / len_th || len_r >= len_th)
//			{
//				continue;
//			}
//
//			int distance_H = abs(lj[0] - li[0]);
//			if (distance_H >= 10)
//			{
//				int dy_top = abs(lj[1] - li[1]);
//				int dy_bottom = abs(lj[3] - li[3]);
//
//				int width = distance_H;
//				int x = li[0];
//
//				int ls = li[0];
//				int rs = lj[0];
//
//				int s_top = li[1], b_top = lj[1];
//				int s_bottom = li[3], b_bottom = lj[3];
//				if (li[1] > lj[1])
//				{
//					s_top = lj[1];
//					b_top = li[1];
//				}
//				if (li[3] > lj[3])
//				{
//					s_bottom = lj[3];
//					b_bottom = li[3];
//				}
//
//				if (dy_top <= 10 && dy_bottom <= 10)
//				{
//#pragma region
//					int up_top = b_top + 10;
//					int down_top = s_top - 10;
//					vector<Vec4i> l_h_top;
//					int f_top = 0;
//					f_top = findHorizonLine(lines_h, up_top, down_top, ls, rs, l_h_top);
//
//					int up_bottom = b_bottom + 10;
//					int down_bottom = s_bottom - 10;
//
//					vector<Vec4i> l_h_bottom;
//					int f_bottom = 0;
//					f_bottom = findHorizonLine(lines_h, up_bottom, down_bottom, ls, rs, l_h_bottom);
//
//					if (f_top && f_bottom)
//					{
//						int st, et, sb, eb;
//						analysisLinesH(l_h_top, st, et);
//						analysisLinesV(l_h_bottom, sb, eb);
//						int s = min(st, sb);
//						int e = max(et, eb);
//
//						int y = s_top;
//						int height = b_bottom - y;
//						int x = s;
//						int width = e - s;
//						Rect rec(x, y, width, height);
//						double ratio = (double)width / height;
//						if (rec.area() >= minArea && rec.area() <= maxArea && ratio <= w_h_ratio && ratio >= 1 / w_h_ratio)
//						{
//							c_v++;
//
//							rectangle(dst, rec, Scalar(0, 0, 255));
//							recBag.push_back(rec);
//						}
//
//					}
//					else if (f_top && !f_bottom)
//					{
//
//					}
//					int y = s_top;
//					int height = b_bottom - y;
//					Rect rec(x, y, width, height);
//					double ratio = (double)width / height;
//					if (rec.area() >= minArea && rec.area() <= maxArea && ratio <= w_h_ratio && ratio >= 1 / w_h_ratio)
//					{
//						c_v++;
//
//						rectangle(dst, rec, Scalar(0, 0, 255));
//						recBag.push_back(rec);
//					}
//
//#pragma endregion
//				}
//
//				else if (dy_top <= 10 && dy_bottom > 10)
//				{
//#pragma region "(dy_top <= 10 && dy_bottom > 10"	
//
//					int y = s_top;
//
//					int up_bottom = b_bottom + 10;
//					int down_bottom = s_bottom - 10;
//
//					vector<Vec4i> l_h_bottom;
//					int f_bottom = 0;
//					f_bottom = findHorizonLine(lines_h, up_bottom, down_bottom, ls, rs, l_h_bottom);
//
//					if (f_bottom)
//					{
//						c_v++;
//
//						for (int k = 0; k < l_h_bottom.size(); ++k)
//						{
//							int height = l_h_bottom[k][1] - s_top;
//							Rect rec(x, y, width, height);
//							double ratio = (double)width / height;
//							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
//							{
//								rectangle(dst, rec, Scalar(0, 0, 255));
//								recBag.push_back(rec);
//							}
//						}
//						l_h_bottom.clear();
//					}
//					else
//					{
//						int up_top = b_top + 10;
//						int down_top = s_top - 10;
//						vector<Vec4i> l_h_top;
//						int f_top = 0;
//						f_top = findHorizonLine(lines_h, up_top, down_top, ls, rs, l_h_top);
//						if (f_top)
//						{
//							c_v++;
//
//							int height = s_bottom - y;
//							Rect rec_1(x, y, width, height);
//							double ratio = (double)width / height;
//							if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
//							{
//								//rectangle(dst, rec_1, Scalar(0, 0, 255));
//								recBag.push_back(rec_1);
//							}
//
//							height = b_bottom - y;
//							Rect rec_2(x, y, width, height);
//							ratio = (double)width / height;
//							if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
//							{
//								rectangle(dst, rec_2, Scalar(0, 0, 255));
//								recBag.push_back(rec_2);
//							}
//						}
//					}
//#pragma endregion
//				}
//
//				else if (dy_top > 10 && dy_bottom <= 10)
//				{
//#pragma region "dy_top > 10 && dy_bottom <= 10"					
//
//					int up_top = b_top + 10;
//					int down_top = s_top - 10;
//
//					vector<Vec4i> l_h_top;
//					int f_top = 0;
//					f_top = findHorizonLine(lines_h, up_top, down_top, ls, rs, l_h_top);
//
//					if (f_top)
//					{
//						c_v++;
//
//						for (int k = 0; k < l_h_top.size(); ++k)
//						{
//							int y = l_h_top[k][1];
//							int height = b_bottom - y;
//							Rect rec(x, y, width, height);
//							double ratio = (double)width / height;
//							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
//							{
//								rectangle(dst, rec, Scalar(0, 0, 255));
//								recBag.push_back(rec);
//							}
//						}
//						l_h_top.clear();
//					}
//					else
//					{
//						int up_bottom = b_bottom + 10;
//						int down_bottom = s_bottom - 10;
//
//						vector<Vec4i> l_h_bottom;
//						int f_bottom = 0;
//						f_bottom = findHorizonLine(lines_h, up_bottom, down_bottom, ls, rs, l_h_bottom);
//						if (f_bottom)
//						{
//							c_v++;
//
//							int y = s_top;
//							int height = b_bottom - y;
//
//							Rect rec_1(x, y, width, height);
//							double ratio = (double)width / height;
//							if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
//							{
//								rectangle(dst, rec_1, Scalar(0, 0, 255));
//								recBag.push_back(rec_1);
//							}
//
//							y = b_top;
//							height = b_bottom - y;
//							Rect rec_2(x, y, width, height);
//							ratio = (double)width / height;
//							if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
//							{
//								//rectangle(dst, rec_2, Scalar(0, 0, 255));
//								recBag.push_back(rec_2);
//							}
//						}
//					}
//
//#pragma endregion
//				}
//
//				else if (overlap2lines(li, lj, 1, overlenth))
//				{
//#pragma region "overlap"
//
//					int up_top = b_top + 10;
//					int down_top = s_top - 10;
//
//					vector<Vec4i> l_h_top;
//					int f_top = 0;
//					f_top = findHorizonLine(lines_h, up_top, down_top, ls, rs, l_h_top);
//
//					if (f_top)
//					{
//						c_v++;
//
//						for (int k = 0; k < l_h_top.size(); ++k)
//						{
//							int y = l_h_top[k][1];
//							int height = b_bottom - y;
//							Rect rec(x, y, width, height);
//							double ratio = (double)width / height;
//							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
//							{
//								rectangle(dst, rec, Scalar(0, 0, 255));
//								recBag.push_back(rec);
//							}
//						}
//						l_h_top.clear();
//					}
//
//					{
//						int up_bottom = b_bottom + 10;
//						int down_bottom = s_bottom - 10;
//
//						vector<Vec4i> l_h_bottom;
//						int f_bottom = 0;
//						f_bottom = findHorizonLine(lines_h, up_bottom, down_bottom, ls, rs, l_h_bottom);
//						if (f_bottom)
//						{
//							c_v++;
//
//							for (int k = 0; k < l_h_bottom.size(); ++k)
//							{
//								int y = s_top;
//								int height = l_h_bottom[k][1] - s_top;
//								Rect rec(x, y, width, height);
//								double ratio = (double)width / height;
//								if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
//								{
//									rectangle(dst, rec, Scalar(0, 0, 255));
//									recBag.push_back(rec);
//								}
//							}
//							l_h_bottom.clear();
//						}
//					}
//
//#pragma endregion		
//				}
//
//			}
//		}
//	}
//
//	//////////////////////*水平线扫描*//////////////////////////////////////////////////////
//	for (int i = 0; i != lines_h.size() - 1; ++i)
//	{
//		Vec4i li = lines_h[i];
//		int len_i = lines_h[i][2] - lines_h[i][0];
//
//		for (int j = i + 1, c_h = 0; j < lines_h.size() && c_h <= numCons; ++j)
//		{
//			Vec4i lj = lines_h[j];
//			int len_j = lj[2] - lj[0];
//
//			double len_r = (double)len_i / len_j;
//
//			if (len_r <= 1 / len_th || len_r >= len_th)
//			{
//				continue;
//			}
//
//			int distance_V = abs(lj[1] - li[1]);
//			if (distance_V >= 10)
//			{
//				int dx_left = abs(lj[0] - li[0]);
//				int dx_right = abs(lj[2] - li[2]);
//
//				int height = distance_V;
//				int y = li[1];
//
//				int ts = li[1];
//				int ds = lj[1];
//
//				int s_left = li[0], b_left = lj[0];
//				int s_right = li[2], b_right = lj[2];
//
//				if (li[0] > lj[0])
//				{
//					s_left = lj[0];
//					b_left = li[0];
//				}
//				if (li[2] > lj[2])
//				{
//					s_right = lj[2];
//					b_right = li[2];
//				}
//
//				if (dx_left <= 10 && dx_right <= 10)
//				{
//					c_h++;
//
//					int x = s_left;
//					int width = b_right - x;
//					Rect rec(x, y, width, height);
//					double ratio = (double)width / height;
//					if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
//					{
//						rectangle(dst, rec, Scalar(0, 255, 255));
//						recBag.push_back(rec);
//					}
//				}
//
//				else if (dx_left > 10 && dx_right <= 10)
//				{
//#pragma region "dx_left > 10 && dx_right <= 10"
//
//					int up_left = b_left + 10;
//					int down_left = s_left - 10;
//
//					int f_left = 0;
//					vector<Vec4i> l_v_left;
//					f_left = findVerticalLine(lines_v, up_left, down_left, ts, ds, l_v_left);
//					if (f_left)
//					{
//						c_h++;
//
//						for (int k = 0; k < l_v_left.size(); ++k)
//						{
//							int x = l_v_left[k][0];
//							int width = b_right - l_v_left[k][0];
//							Rect rec(x, y, width, height);
//							double ratio = (double)width / height;
//							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
//							{
//								rectangle(dst, rec, Scalar(0, 255, 255));
//								recBag.push_back(rec);
//							}
//						}
//						l_v_left.clear();
//					}
//					else
//					{
//						int up_right = b_right + 10;
//						int down_right = s_right - 10;
//
//						vector<Vec4i> l_v_right;
//						int f_right = 0;
//						f_right = findVerticalLine(lines_v, up_right, down_right, ts, ds, l_v_right);
//						if (f_right)
//						{
//							c_h++;
//
//							int x = s_left;
//							int width = b_right - x;
//							Rect rec_1(x, y, width, height);
//							double ratio = (double)width / height;
//							if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
//							{
//								rectangle(dst, rec_1, Scalar(0, 255, 255));
//								recBag.push_back(rec_1);
//							}
//
//							x = b_left;
//							width = b_right - x;
//							Rect rec_2(x, y, width, height);
//							ratio = (double)width / height;
//							if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
//							{
//								//rectangle(dst, rec_2, Scalar(0, 255, 255));
//								recBag.push_back(rec_2);
//							}
//						}
//
//					}
//#pragma endregion
//				}
//
//				else if (dx_left <= 10 && dx_right > 10)
//				{
//#pragma region "dx_left <= 10 && dx_right > 10"
//
//					int up_right = b_right + 10;
//					int down_right = s_right - 10;
//
//					vector<Vec4i> l_v_right;
//					int f_right = 0;
//					f_right = findVerticalLine(lines_v, up_right, down_right, ts, ds, l_v_right);
//
//					if (f_right)
//					{
//						c_h++;
//
//						for (int k = 0; k < l_v_right.size(); ++k)
//						{
//							int x = s_left;
//							int y = li[1];
//							int width = l_v_right[k][0] - s_left;
//							Rect rec(x, y, width, height);
//							double ratio = (double)width / height;
//							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
//							{
//								rectangle(dst, rec, Scalar(0, 255, 255));
//								recBag.push_back(rec);
//							}
//						}
//						l_v_right.clear();
//					}
//
//					else
//					{
//						int up_left = b_left + 10;
//						int down_left = s_left - 10;
//
//						int f_left = 0;
//						vector<Vec4i> l_v_left;
//						f_left = findVerticalLine(lines_v, up_left, down_left, ts, ds, l_v_left);
//						if (f_left)
//						{
//							c_h++;
//
//							int x = s_left;
//							int width = b_right - x;
//							Rect rec_1(x, y, width, height);
//							double ratio = (double)width / height;
//							if (rec_1.area() > minArea && rec_1.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
//							{
//								rectangle(dst, rec_1, Scalar(0, 255, 255));
//								recBag.push_back(rec_1);
//							}
//
//							width = s_right - x;
//							Rect rec_2(x, y, width, height);
//							ratio = (double)width / height;
//							if (rec_2.area() > minArea && rec_2.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
//							{
//								//rectangle(dst, rec_2, Scalar(0, 255, 255));
//								recBag.push_back(rec_2);
//							}
//						}
//					}
//#pragma endregion
//				}
//
//				else if (overlap2lines(li, lj, 0, overlenth))
//				{
//#pragma region "overlap"
//
//					int up_right = b_right + 10;
//					int down_right = s_right - 10;
//
//					vector<Vec4i> l_v_right;
//					int f_right = 0;
//					f_right = findVerticalLine(lines_v, up_right, down_right, ts, ds, l_v_right);
//
//					if (f_right)
//					{
//						c_h++;
//
//						for (int k = 0; k < l_v_right.size(); ++k)
//						{
//							int x = s_left;
//							int y = li[1];
//							int width = l_v_right[k][0] - s_left;
//							Rect rec(x, y, width, height);
//							double ratio = (double)width / height;
//							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
//							{
//								rectangle(dst, rec, Scalar(0, 255, 255));
//								recBag.push_back(rec);
//							}
//						}
//						l_v_right.clear();
//					}
//
//					int up_left = b_left + 10;
//					int down_left = s_left - 10;
//
//					int f_left = 0;
//					vector<Vec4i> l_v_left;
//					f_left = findVerticalLine(lines_v, up_left, down_left, ts, ds, l_v_left);
//					if (f_left)
//					{
//						c_h++;
//
//						for (int k = 0; k < l_v_left.size(); ++k)
//						{
//							int x = l_v_left[k][0];
//							int width = b_right - l_v_left[k][0];
//							Rect rec(x, y, width, height);
//							double ratio = (double)width / height;
//							if (rec.area() > minArea && rec.area() < maxArea && ratio < w_h_ratio && ratio>1 / w_h_ratio)
//							{
//								rectangle(dst, rec, Scalar(0, 255, 255));
//								recBag.push_back(rec);
//							}
//						}
//						l_v_left.clear();
//					}
//#pragma endregion 
//				}
//
//			}
//		}
//	}
//}

void analysisLinesH(vector<Vec4i> &l_h, int &s, int &e)
{
	int mins = 1000;
	int maxe = 0;

	for (int i = 0; i < l_h.size(); ++i)
	{
		if (l_h[i][0] < mins)
			mins = l_h[i][0];

		if (l_h[i][2] > maxe)
			maxe = l_h[i][2];
	}
	s = mins;
	e = maxe;
}

void analysisLinesV(vector<Vec4i> &l_v, int &s, int &e)
{
	int mins = 1000;
	int maxe = 0;

	for (int i = 0; i < l_v.size(); ++i)
	{
		if (l_v[i][1] < mins)
			mins = l_v[i][1];

		if (l_v[i][3] > maxe)
			maxe = l_v[i][3];
	}
	s = mins;
	e = maxe;
}