#include "main.h"

//////////////////////////////////////////lines detect////////////////////////////////////////
double proposals_total_time = 0;
double total_time = 0;

int main(void)
{
	int recNum = 0;
	int recRecall = 0;
	int objectTotal = 0;
	vector<string> error_image;
	vector<string> error_data;

	int gray = 1;
	//////////////////////////////////////*read path.txt*///////////////////////////
	string imageFile = "test_image_path";
	vector<string> image_path;
	string dataFile = "test_data_path.txt";
	vector<string> data_path;
	loadFile(image_path, data_path, imageFile, dataFile);
	int imageCount = image_path.size();
	int dataCount = data_path.size();
	if (imageCount != dataCount)
	{
		cout << "imageCount != dataCount,Something wrong !" << endl;
	//	return -1;
	}
	/////////////////////////////////////* load clsssfier *//////////////////////////
	CvSVM svm_stage_0, svm_stage_1, svm_stage_2;
	string svm_filename = "hog_svm-l.xml";
	cout << "stage_0 SVM file: " << svm_filename << endl;
	svm_stage_0.load(svm_filename.c_str());

	svm_filename = "CLBP_false_svm-l.xml";
	cout << "stage_1 SVM file: " << svm_filename << endl;
	svm_stage_1.load(svm_filename.c_str());

	svm_filename = "bow_svm-l.xml";
	cout << "stage_2 SVM file: " << svm_filename << endl;
	svm_stage_2.load(svm_filename.c_str());


	/////////////////////////////////////* load codebook *//////////////////////////
	Mat codebook;
	string codebookFile = "codebook_HOG_HSV_12_2_8_1_1_0_512_only_posi_64.xml";
	readCodebook(codebookFile, codebook);
	/////////////////////////////////////////////////////////////////////////////////
	for (int i = 0; i != image_path.size(); ++i)
	{
		cout << i + 1 << ">" << endl;
		string tempImg = image_path[i];
		Mat imgRGB = imread(tempImg);
		Mat imgGray;
		cvtColor(imgRGB, imgGray, CV_BGR2GRAY);
		Mat image_rects = imgRGB.clone();

		double t = (double)getTickCount();

		vector<Rect> recBag;    
		getProposals(imgGray, recBag);                  //候选区域提取

		t = ((double)getTickCount() - t) / getTickFrequency();
		cout << "proposals  in milliseconds: " << t * 1000 << endl;
		proposals_total_time += t;
		//////////////////////////////////////////////* classify *////////////////////////////////////////////////
		t = (double)getTickCount();


		vector<double> HOG_scores;
		classifyHOG(imgGray, recBag, HOG_scores, svm_stage_0, -1);         //第一级分类
		supress(recBag, HOG_scores, 0.9);
		//supressContain(recBag, HOG_scores, 0.95);

		vector<double> lbp_scores;
		classifyLBP(imgGray, recBag, lbp_scores,svm_stage_1,-1.75);        //第二级分类
		//supress(recBag, lbp_scores, 0.6);
		//supressContain(recBag, lbp_scores, 0.9);
	/*	vector<Rect> recBagEx(recBag);
		rect_extend(recBagEx, 1.2, imgRGB.cols, imgRGB.rows);
		for (int i = 0; i < recBag.size(); ++i)
		{
			Rect r = recBagEx[i];
			Mat recImg(imgRGB, r);
			Mat rimg = recImg.clone();
			vector<Vec4i> lineV, lineH;
			getLines(rimg, lineV, lineH);
			Mat show=recImg.clone();
			drawLines(show, lineV, Scalar(0,0,255));
			drawLines(show, lineH, Scalar(0, 255, 255));
		}*/

		vector<double> bow_scores;
		classifyBow(imgRGB, recBag, bow_scores,svm_stage_2, codebook, -1);   //第三级分类

		//cutRects(recBag, imgGray, svm_stage_1, -1.75);

		t = ((double)getTickCount() - t) / getTickFrequency();
		cout << "classified times passed in milliseconds: " << t * 1000 << endl;
		total_time += t;

		for (int i = 0; i < recBag.size(); ++i) { rectangle(image_rects, recBag[i], Scalar(0, 0, 255)); }
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		string tempData = data_path[i];
		vector<string> rects_str;
		int object_num = readImageFile(tempData, rects_str);

		int *xmin = new int[object_num];
		int *xmax = new int[object_num];
		int *ymin = new int[object_num];
		int *ymax = new int[object_num];

		int *f_recall = new int[object_num];

		for (int i = 0; i < object_num; ++i)
		{
			string tempObject = rects_str[i];
			getRangeFromTxt(tempObject, &xmin[i], &xmax[i], &ymin[i], &ymax[i]);
			f_recall[i] = 0;
		}

		vector<Rect> p_rects;
		vector<Rect> n_rects;
		vector<Rect> hard_rects;
		vector<Rect> in_rects;

		for (int i = 0; i < recBag.size(); ++i)
		{
			int f_n = 1;
			Rect r = recBag[i];
			int testx = r.x;
			for (int i = 0; i < object_num; ++i)
			{
				int f_s = similarity(r, xmin[i], ymin[i], xmax[i], ymax[i], 0.8);
				if (f_s == 1)
				{
					p_rects.push_back(r);
					f_recall[i] = 1;
					f_n = 0;
					break;
				}
			}

			if (f_n && !overlap_object(r, xmin, ymin, xmax, ymax, object_num, 0.7))
				n_rects.push_back(r);
		}

		int c_recall = 0;
		for (int i = 0; i < object_num; ++i)
		{
			if (f_recall[i])
				c_recall++;
			else
			{
				Rect r_object(xmin[i], ymin[i], xmax[i] - xmin[i], ymax[i] - ymin[i]);
				p_rects.push_back(r_object);
			}
		}

		if (c_recall != object_num)
		{
			error_image.push_back(tempImg);
			error_data.push_back(tempData);
		}

		recRecall += c_recall;
		objectTotal += object_num;
		recNum += recBag.size();

		/*  string recFile = imageFile+"_recImage";
		  if (_access(recFile.c_str(), 0) == -1)
		  {
		  _mkdir(recFile.c_str());
		  }
		  rect2image(imgRGB, p_rects, tempImg, recFile, 1);
		  rect2image(imgRGB, n_rects, tempImg, recFile, 0);*/

		string rectFolder = "positive_stage_1";
		if (_access(rectFolder.c_str(), 0) == -1)
		{
			_mkdir(rectFolder.c_str());
		}
	//	rectsSaveAsImage(imgRGB, p_rects, tempImg, rectFolder);

		rectFolder = "negative_stage_1";
		if (_access(rectFolder.c_str(), 0) == -1)
		{
			_mkdir(rectFolder.c_str());
		}
//		rectsSaveAsImage(imgRGB, n_rects, tempImg, rectFolder);

		/////////////////////////////////////////////////////////////////////
		recBag.clear();
		p_rects.clear();
		n_rects.clear();

		delete[] xmin;
		delete[] ymin;
		delete[] xmax;
		delete[] ymax;
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		string folderName = imageFile;
		if (_access(folderName.c_str(), 0) == -1)
		{
			_mkdir(folderName.c_str());
		}
		string image_name = getFileName(tempImg);

		char srcToSaved[50];
		sprintf(srcToSaved, "%s/%s.png", folderName.c_str(), image_name.c_str());
		imwrite(srcToSaved, imgRGB);

		char recToSaved[50];
		sprintf(recToSaved, "%s/%s_rec.png", folderName.c_str(), image_name.c_str());
		imwrite(recToSaved, image_rects);
	}

	cout << "\nrecRecall : " << recRecall << endl;
	cout << "objectTotal : " << objectTotal << endl;
	cout << "recall : " << (double)recRecall / objectTotal * 100 << "%" << endl;
	cout << "recNum : " << recNum << endl;
	cout << "average recNum :" << recNum / imageCount << endl;

	cout << "\nlsd total time : " << proposals_total_time << " s" << endl;
	cout << "lsd average time : " << proposals_total_time / imageCount * 1000 << " ms" << endl;
	cout << "classify total time : " << total_time << " s" << endl;
	cout << "classify average time : " << total_time / imageCount * 1000 << " ms\n" << endl;

	ofstream out_error("error_image_path.txt");
	for (int i = 0; i < error_image.size(); ++i)
	{
		out_error << error_image[i] << endl;
	}
	out_error.close();

	ofstream out_error_data("error_data_path.txt");
	for (int i = 0; i < error_data.size(); ++i)
	{
		out_error_data << error_data[i] << endl;
	}
	out_error_data.close();

	return EXIT_SUCCESS;
}
