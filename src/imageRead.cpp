#include "imageRead.h"

int readImageFile(string &imageFile, vector<string> &pathOfImage)
{
	string buf;
	int imageCount = 0;

	ifstream img_list(imageFile);

	if (img_list)
	{
		cout << "file is : " << imageFile << endl;

	}
	else
	{
		cout << "cannot open the imagelist file." << endl;
	}

	while (img_list)
	{
		if (getline(img_list, buf))
		{
			pathOfImage.push_back(buf);
			imageCount++;
		}
	}
	img_list.close();
	cout << imageCount << " things have been read" << endl;

	return imageCount;
}

void rect2image(Mat &image, vector<Rect> recBag, string imagePath,string &folder,int type)
{
	int m = imagePath.find_last_of('\\');
	int n = imagePath.find_last_of('.');

	string image_name = imagePath.substr(m + 1, n - m - 1);

	Rect rec;
	char dir[50];
	
	sprintf(dir, "%s/%s", folder.c_str(), image_name.c_str());
	_mkdir(dir);

	if (type)
	{
		char dir_p[50];
		sprintf(dir_p, "%s/%s", dir, "p");
		_mkdir(dir_p);

		for (int k = 0; k != recBag.size(); ++k)
		{
			rec = recBag[k];
			Mat recImage(image, rec);
			char recToSaved[50];
			sprintf(recToSaved, "%s/%s_%d_rec.png", dir_p, image_name.c_str(), k);
			imwrite(recToSaved, recImage);
		}
	}

	else
	{
		char dir_n[50];
		sprintf(dir_n, "%s/%s", dir, "n");
		_mkdir(dir_n);

		for (int k = 0; k != recBag.size(); ++k)
		{
			rec = recBag[k];
			Mat recImage(image, rec);
			char recToSaved[50];
			sprintf(recToSaved, "%s/%s_%d_rec.png", dir_n, image_name.c_str(), k);
			imwrite(recToSaved, recImage);
		}
	}
}

void rectsSaveAsImage(Mat &image, vector<Rect> recBag, string imagePath, string &folder)
{
	int m = imagePath.find_last_of('\\');
	int n = imagePath.find_last_of('.');

	string image_name = imagePath.substr(m + 1, n - m - 1);

	for (int k = 0; k != recBag.size(); ++k)
	{
		Rect rec = recBag[k];
		Mat recImage(image, rec);
		char recToSaved[50];
		sprintf(recToSaved, "%s/%s_%d_rec.png", folder.c_str(), image_name.c_str(), k);
		imwrite(recToSaved, recImage);
	}
}

void saveErrImg(vector<string> &error_images, string &folder)
{
	for (int i = 0; i < error_images.size(); ++i)
	{
		Mat img = imread(error_images[i]);
		int m = error_images[i].find_last_of('\\');
		string imgName = error_images[i].substr(m + 1);
		char recToSaved[50];
		sprintf(recToSaved, "%s/%s", folder.c_str(), imgName.c_str());
		imwrite(recToSaved, img);
	}
}
	