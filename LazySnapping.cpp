#include "StdAfx.h"
#include "ConfigManager.h"
#include "lazysnapping.h"
#include "GCSPInteractiveSegmentation.h"
#include <cmath>
#include <fstream>
#include <string>

#define UNIFORM 0
#define PDF 0
#define DELTA 1f

#define infinity	100000000

static uchar* bg_centroids;
static uchar* fg_centroids;

static int *bg_disCentroids;
static int *fg_disCentroids;


LazySnapping::LazySnapping()
{

}

LazySnapping::LazySnapping(CDib* img, CDib* rightimg, CDib* labelledImg, CDib* result, CDib* right_result,uchar* foreground, uchar* background, CStroke *lAdjustStroke):
srcImage(img),
srcImage2(rightimg),
markUpImage(labelledImg),
destImage(result),
destImage2(right_result)
{
	if (img == NULL || result == NULL)
	{
		::AfxMessageBox(_T("No original or result image data exists."));
		return;
	}
	graph = NULL;
	nodes = NULL;

	memcpy(fg_color, foreground, 3);
	memcpy(bg_color, background, 3);

	int imWidth = srcImage->m_lpBMIH->biWidth;
	int imHeight = srcImage->m_lpBMIH->biHeight;

	ReadMatchPoint(ConfigManager::Instance()->work_path() + "match.txt");
}

LazySnapping::~LazySnapping(void)
{
	fg_input.clear();
	bg_input.clear();
	if (nodes)
	{
		delete nodes;
		nodes = NULL;
	}
	if (graph)
	{
		delete graph;
		graph = NULL;
	}
}

void  LazySnapping::run()
{
	string timer_path = ConfigManager::Instance()->work_path() + "sparseCutTimer.txt";
	FILE *fp = fopen(timer_path.c_str(), "w");

	//收集scribble所穿过的像素，存储在fg_input和bg_input中
	bool collectedFandB = collectLabelledPixels();
	if (!collectedFandB)
		return;
	getCentroids();

	clock_t start = clock();
	build_stereograph();
	clock_t end = clock();
	fprintf(fp, "#Build:%ld\n", end - start);
	
	start = clock();
	computeMaxFlow();
	end = clock();
	fprintf(fp, "#ComputeFlow:%ld\n", end - start);
	fclose(fp);

	extractResult();

	delete[]fg_centroids;
	fg_centroids = NULL;
	delete[]bg_centroids;
	bg_centroids = NULL;
}

void LazySnapping::getCentroids()
{
	if (fg_centroids)
	{
		delete[]fg_centroids;
		fg_centroids = NULL;
	}
	if (bg_centroids)
	{
		delete[]bg_centroids;
		bg_centroids = NULL;
	}

	if (fg_disCentroids)
	{
		delete[]fg_disCentroids;
		fg_disCentroids = NULL;
	}
	if (bg_disCentroids)
	{
		delete[]bg_disCentroids;
		bg_disCentroids = NULL;
	}

	bg_centroids = new uchar[_dim * _k];
	fg_centroids = new uchar[_dim * _k];
	fg_disCentroids = new int[16];
	bg_disCentroids = new int[16];

	vector<float> bg_centroidVec = vector<float>(_k*_dim, 0);
	vector<float> fg_centroidsVec = vector<float>(_k*_dim, 0);

	vector<int> fg_disCentroidsVec = vector<int>(16, 0);
	vector<int> bg_disCentroidsVec = vector<int>(16, 0);

	if (bg_input.size() != 0)
		kmean.computeKMeans(bg_input, bg_centroidVec);

	if (fg_input.size() != 0)
		kmean.computeKMeans(fg_input, fg_centroidsVec);

	int i = 0;
	for (Iterator it = bg_centroidVec.begin(); it != bg_centroidVec.end(); it++, i++)
		bg_centroids[i] = (uchar)*it;

	i = 0;
	for (Iterator it = fg_centroidsVec.begin(); it != fg_centroidsVec.end(); it++, i++)
		fg_centroids[i] = (uchar)*it;

	i = 0;
	for (IntIterator it = bg_disCentroidsVec.begin(); it != bg_disCentroidsVec.end(); it++, i++)
		bg_disCentroids[i] = (int)*it;

	i = 0;
	for (IntIterator it = fg_disCentroidsVec.begin(); it != fg_disCentroidsVec.end(); it++, i++)
		fg_disCentroids[i] = (int)*it;

}

void LazySnapping::build_stereograph()
{
	//这个是基于像素基础上建立图
	int imWidth = srcImage->m_lpBMIH->biWidth;
	int imHeight = srcImage->m_lpBMIH->biHeight;
	unsigned int graphDimension = imWidth * imHeight * 2;
	nodes = new Graph::node_id[graphDimension];
	graph = new Graph();
	for (unsigned int i = 0; i < graphDimension; i++)
		nodes[i] = graph->add_node();

	// Add interconnections
	int numChan = 3;
	Graph::captype cap;
	Graph::captype cap2;

	uchar** dataLineCache = new uchar*[imWidth];
	uchar** dataColumnCache = new uchar*[imHeight];

	uchar** dataLineCache2 = new uchar*[imWidth];
	uchar** dataColumnCache2 = new uchar*[imHeight];

	for (int i = 0; i < imWidth; i++){
		dataLineCache[i] = NULL;
		dataLineCache2[i] = NULL;
	}

	for (int i = 0; i < imHeight; i++){
		dataColumnCache[i] = NULL;
		dataColumnCache2[i] = NULL;
	}

	int numPixel = imWidth * imHeight;
	int posX, posY;

	uchar* pixColorOnSrcImageUnchar = new uchar[3];
	uchar* pixColorOnSrcImage2Unchar = new uchar[3];
	
	for (int i = 0; i < numPixel; i++)
	{
		int indexAtRight = numPixel + i;

		posX = i % imWidth;
		posY = i / imWidth;

		// Data cost
		LONG posOffset = srcImage->GetPixelOffset(posX, posY);

		RGBQUAD pixColorOnMarkUpImage = markUpImage->GetPixel(posOffset);
		RGBQUAD pixColorOnSrcImage = srcImage->GetPixel(posOffset);
		RGBQUAD pixColorOnSrcImage2 = srcImage2->GetPixel(posOffset);

		// Copy RGB color of pixel
		pixColorOnSrcImageUnchar[0] = pixColorOnSrcImage.rgbRed;
		pixColorOnSrcImageUnchar[1] = pixColorOnSrcImage.rgbGreen;
		pixColorOnSrcImageUnchar[2] = pixColorOnSrcImage.rgbBlue;

		pixColorOnSrcImage2Unchar[0] = pixColorOnSrcImage2.rgbRed;
		pixColorOnSrcImage2Unchar[1] = pixColorOnSrcImage2.rgbGreen;
		pixColorOnSrcImage2Unchar[2] = pixColorOnSrcImage2.rgbBlue;

		if ((pixColorOnMarkUpImage.rgbRed == fg_color[0]) &&
			(pixColorOnMarkUpImage.rgbGreen == fg_color[1]) &&
			(pixColorOnMarkUpImage.rgbBlue == fg_color[2]))
		{
			graph->set_tweights(nodes[i], (Graph::captype)infinity, (Graph::captype)0);
		}
		else if (((pixColorOnMarkUpImage.rgbRed == bg_color[0]) &&
			(pixColorOnMarkUpImage.rgbGreen == bg_color[1]) &&
			(pixColorOnMarkUpImage.rgbBlue == bg_color[2])))//||AreaID(posX,posY))//直连通约束
		{
			graph->set_tweights(nodes[i], (Graph::captype)0, (Graph::captype)infinity);
		}
		else
		{	
			float sDist = min_fg(pixColorOnSrcImageUnchar);
			float dDist = min_bg(pixColorOnSrcImageUnchar);
			float dDenom = sDist + dDist;

			float sDist2 = min_fg(pixColorOnSrcImage2Unchar);
			float dDist2 = min_bg(pixColorOnSrcImage2Unchar);
			float dDenom2 = sDist2 + dDist2;

			//if (abs(sDist - dDist) < 50)   //可注释
			//{
			//	graph->set_tweights(nodes[i], (Graph::captype)0.5, (Graph::captype)0.5);//可注释
			//}
			//else
			//{
				Graph::captype sEnergy = 0;
				Graph::captype dEnergy = 0;

				if (dDenom != 0)
				{
					sEnergy = (Graph::captype)(dDist / dDenom);
					dEnergy = (Graph::captype)(sDist / dDenom);
				}

				graph->set_tweights(nodes[i], sEnergy, dEnergy);
		/*	}*/

			//if (abs(sDist2 - dDist2) < 50)   //可注释
			//{
			//	graph->set_tweights(nodes[indexAtRight], (Graph::captype)0.5, (Graph::captype)0.5);//可注释
			//}
			//else
			//{
				Graph::captype sEnergy2 = 0;
				Graph::captype dEnergy2 = 0;

				if (dDenom2 != 0)
				{
					sEnergy2 = (Graph::captype)(dDist2 / dDenom2);
					dEnergy2 = (Graph::captype)(sDist2 / dDenom2);
				}
				graph->set_tweights(nodes[indexAtRight], sEnergy2, dEnergy2);
			/*}*/

		}

		// Smoothness term
		float lamda = 4000;
		float lamdaC = 6000;

		// Link to upper node
		if (dataLineCache[posX])
		{
			cap = (Graph::captype)(lamda / (1 + colorDistanceSq(pixColorOnSrcImageUnchar, dataLineCache[posX], numChan)));
			graph->add_edge(nodes[i], nodes[i - imWidth], cap, cap);
		}
		else
		{
			dataLineCache[posX] = new unsigned char[numChan];
		}

		if (dataLineCache2[posX])
		{
			cap2 = (Graph::captype)(lamda / (1 + colorDistanceSq(pixColorOnSrcImage2Unchar, dataLineCache2[posX], numChan)));
			graph->add_edge(nodes[indexAtRight], nodes[indexAtRight - imWidth], cap2, cap2);
		}
		else
		{
			dataLineCache2[posX] = new unsigned char[numChan];
		}

		// Link to left node
		if (dataColumnCache[posY])
		{
			cap = (Graph::captype)(lamda / (1 + colorDistanceSq(pixColorOnSrcImageUnchar, dataColumnCache[posY], numChan)));
			graph->add_edge(nodes[i], nodes[i - 1], cap, cap);
		}
		else
		{
			dataColumnCache[posY] = new unsigned char[numChan];
		}

		if (dataColumnCache2[posY])
		{
			cap2 = (Graph::captype)(lamda / (1 + colorDistanceSq(pixColorOnSrcImage2Unchar, dataColumnCache2[posY], numChan)));
			graph->add_edge(nodes[indexAtRight], nodes[indexAtRight - 1], cap2, cap2);
		}
		else
		{
			dataColumnCache2[posY] = new unsigned char[numChan];
		}

		memcpy(dataLineCache[posX], pixColorOnSrcImageUnchar, numChan);
		memcpy(dataColumnCache[posY], pixColorOnSrcImageUnchar, numChan);
		memcpy(dataLineCache2[posX], pixColorOnSrcImage2Unchar, numChan);
		memcpy(dataColumnCache2[posY], pixColorOnSrcImage2Unchar, numChan);

	}

	//Process match 
	int matchCount = left_features.size();
	for (size_t i = 0; i < matchCount; i++)
	{
		pair<int, int> left = left_features[i];
		pair<int, int> right = right_features[i];

		//Add cap
		int leftIndex = left.second*imWidth + left.first;
		int rightIndex = numPixel + right.second*imWidth + right.first;

		graph->add_edge(nodes[leftIndex], nodes[rightIndex], (Graph::captype)infinity, (Graph::captype)infinity);
		graph->add_edge(nodes[rightIndex], nodes[leftIndex], (Graph::captype)infinity, (Graph::captype)infinity);

	}

	//Release allocated
	delete[]pixColorOnSrcImageUnchar;
	delete[]pixColorOnSrcImage2Unchar;
	pixColorOnSrcImageUnchar = NULL;
	pixColorOnSrcImage2Unchar = NULL;

	for (int i = 0; i < imWidth; i++){
		delete[] dataLineCache[i];
		delete[] dataLineCache2[i];
	}
	
	for (int i = 0; i < imHeight; i++){
		delete[] dataColumnCache[i];
		delete[] dataColumnCache2[i];
	}

	delete[] dataLineCache;
	delete[] dataLineCache2;
	delete[] dataColumnCache;
	delete[] dataColumnCache2;

}


// Compute the maxflow and add extract the solution 
void LazySnapping::computeMaxFlow()
{
	graph->maxflow();
}

void LazySnapping::extractResult()
{
	CDib leftSeg;
	CDib rightSeg;
	leftSeg.CopyImageInMemory(srcImage);
	rightSeg.CopyImageInMemory(srcImage2);

	// Define iterator pointers
	Graph::node_id* nodeIt = nodes;

	// Iterate over all rows
	for (int i = 0; i < destImage->m_lpBMIH->biHeight; i++)
	{
		for (int k = 0; k < destImage->m_lpBMIH->biWidth; k++, nodeIt++)
		{
			LONG posOffset = destImage->GetPixelOffset(k, i);//傻帽，原来是这里i,k反了

			if (graph->what_segment(*nodeIt) == Graph::SINK) // background,SINK or SOURCE?
			{
				RGBQUAD srcColor = srcImage->GetPixel(posOffset);

				srcColor.rgbRed = (BYTE)((float)srcColor.rgbRed * alpha);
				srcColor.rgbGreen = (BYTE)((float)srcColor.rgbGreen * alpha);
				srcColor.rgbBlue = (BYTE)((float)srcColor.rgbBlue * alpha + 255.0f * (1 - alpha));
				destImage->SetPixel(posOffset, RGB(srcColor.rgbRed, srcColor.rgbGreen, srcColor.rgbBlue));
				leftSeg.SetPixel(posOffset, RGB(0, 0, 0));
			}
			else
			{
				RGBQUAD srcColor = srcImage->GetPixel(posOffset);
				destImage->SetPixel(posOffset, RGB(srcColor.rgbRed, srcColor.rgbGreen, srcColor.rgbBlue));
				leftSeg.SetPixel(posOffset, RGB(255, 255, 255));
			}
		}
	}

	CFile mfile;
	string file_path = ConfigManager::Instance()->work_path() + "leftResSIFT.bmp";
	CString filename(file_path.c_str());

	if (!mfile.Open(filename, CFile::modeCreate | CFile::modeWrite))
	{
		return;
	}
	leftSeg.Write(&mfile);

	//For right picture
	for (int i = 0; i < destImage->m_lpBMIH->biHeight; i++)
	{
		for (int k = 0; k < destImage->m_lpBMIH->biWidth; k++, nodeIt++)
		{
			LONG posOffset = destImage2->GetPixelOffset(k, i);

			if (graph->what_segment(*nodeIt) == Graph::SINK)
			{
				RGBQUAD srcColor = srcImage2->GetPixel(posOffset);

				srcColor.rgbRed = (BYTE)((float)srcColor.rgbRed * alpha);
				srcColor.rgbGreen = (BYTE)((float)srcColor.rgbGreen * alpha);
				srcColor.rgbBlue = (BYTE)((float)srcColor.rgbBlue * alpha + 255.0f * (1 - alpha));

				destImage2->SetPixel(posOffset, RGB(srcColor.rgbRed, srcColor.rgbGreen, srcColor.rgbBlue));
				rightSeg.SetPixel(posOffset, RGB(0, 0, 0));

			}
			else
			{
				RGBQUAD srcColor = srcImage2->GetPixel(posOffset);

				destImage2->SetPixel(posOffset, RGB(srcColor.rgbRed, srcColor.rgbGreen, srcColor.rgbBlue));
				rightSeg.SetPixel(posOffset, RGB(255, 255, 255));
			}
		}

		CFile mfile2;
		string file_path2 = ConfigManager::Instance()->work_path() + "rightResSIFT.bmp";
		CString filename2(file_path2.c_str());

		if (!mfile2.Open(filename2, CFile::modeCreate | CFile::modeWrite))
		{
			return;
		}
		rightSeg.Write(&mfile2);
	}

}

// Compute min distance to FOREGROUND CENTROIDS
// color: the rgb color to which the distance should be computed
// return: minimum distance
float LazySnapping::min_fg(uchar* color)
{
	if (fg_centroids != 0)
		return minCentroidDistance(color, fg_centroids, 3);
	else
		return 0;
}

// Compute min distance to BACKGROUND CENTROIDS
// color: the rgb color, to which the distance should be computed
// return: minimum distance
float LazySnapping::min_bg(uchar* color)
{
	if (bg_centroids != 0)
		return minCentroidDistance(color, bg_centroids, 3);
	else
		return 0;
}

// Compute min distance to the centroids, given a color
// color: a rgb color 
// centroids: a rgb color vector
// retrun: the minimum distance betweeb the rgb color and the centroid
float LazySnapping::minCentroidDistance(uchar* color, uchar* centroids, int dim)
{
	float min = distance(color, centroids, dim);
	float tmpMin;
	for (unsigned int i = dim; i < _dim * _k; i += dim)
	{
		tmpMin = distance(color, centroids + i, dim);
		if (tmpMin < min)
			min = tmpMin;
	}
	return sqrt(min);
}

float LazySnapping::distance(const uchar* c1, const uchar* c2, int dim)
{
	float result = 0;

	for (int i = 0; i < dim; i++)
	{
		result += ((float)c1[i] - c2[i])*((float)c1[i] - c2[i]);
	}

	return result;
}

float LazySnapping::colorDistanceSq(uchar* a, uchar* b, int dim)
{
	float t = 0;
	for (int i = 0; i < dim; i++)
	{
		float d = (float)(a[i] - b[i]);
		t += d*d;
	}
	return t;
}

bool LazySnapping::collectLabelledPixels()
{
	int imWidth = markUpImage->m_lpBMIH->biWidth;
	int offsetIndex = 0;

	for (int i = 0; i < markUpImage->m_lpBMIH->biHeight; i++)
	{
		for (int k = 0; k < markUpImage->m_lpBMIH->biWidth; k++)
		{
			LONG posOffset = markUpImage->GetPixelOffset(k, i);
			RGBQUAD pixColor = markUpImage->GetPixel(posOffset);
			RGBQUAD pixColor_src = srcImage->GetPixel(posOffset);

			if ((pixColor.rgbRed == fg_color[0]) &&
				(pixColor.rgbGreen == fg_color[1]) &&
				(pixColor.rgbBlue == fg_color[2]))
			{
				fg_input.push_back(pixColor_src.rgbRed);
				fg_input.push_back(pixColor_src.rgbGreen);
				fg_input.push_back(pixColor_src.rgbBlue);
			}
			else if ((pixColor.rgbRed == bg_color[0]) &&
				(pixColor.rgbGreen == bg_color[1]) &&
				(pixColor.rgbBlue == bg_color[2]))
			{
				bg_input.push_back(pixColor_src.rgbRed);
				bg_input.push_back(pixColor_src.rgbGreen);
				bg_input.push_back(pixColor_src.rgbBlue);
			}

			offsetIndex++;
		}
	}
	if (bg_input.size() == 0 || fg_input.size() == 0)
	{
		return false;
	}
	else
	{
		return true;
	}

}

void LazySnapping::ReadLeftDispairity(string path)
{
	int imWidth = srcImage->m_lpBMIH->biWidth;
	int imHeight = srcImage->m_lpBMIH->biHeight;

	int numPixels = (imWidth - 2)*(imHeight - 2);

	FILE *fp = fopen(path.c_str(), "r");

	int value = 0;
	this->Disparity = (float*)malloc(sizeof(float)*numPixels);

	int count = 0;

	while (1)
	{
		if (fscanf(fp, "%d\n", &value) != 1)
		{
			break;
		}
		else
		{
			this->Disparity[count] = (float)value;
			count++;
		}
	}

	fclose(fp);

	int numPixels2 = imWidth*imHeight;
	//Here we start generate dispairity;
	this->leftDispairites = (int*)malloc(sizeof(int)*numPixels2);

	count = 0;

	//For left image
	for (size_t y = 1; y < imHeight - 1; y++)
	{
		for (size_t x = 1; x < imWidth - 1; x++)
		{
			int offset = y*imWidth + x;

			int disValue = (int)this->Disparity[count];

			leftDispairites[offset] = disValue;

			count++;
		}
	}

	//For hor margin:
	for (size_t y = 1; y < imHeight - 1; y++)
	{
		int offset = y*imWidth;
		int offset2 = (y + 1)*imWidth - 1;

		int disValue1 = leftDispairites[offset + 1];
		int disValue2 = leftDispairites[offset2 - 1];
		leftDispairites[offset] = disValue1;
		leftDispairites[offset2] = disValue2;
	}

	//For Vor margin:
	for (size_t x = 0; x < imWidth; x++)
	{
		int offset = x;
		int offset2 = (imHeight - 1)*imWidth + x;

		int disValue1 = leftDispairites[offset + imWidth];
		int disValue2 = leftDispairites[offset2 - imWidth];

		leftDispairites[offset] = disValue1;
		leftDispairites[offset2] = disValue2;
	}
}

void LazySnapping::ReadRightDispairity(string path)
{
	int imWidth = srcImage->m_lpBMIH->biWidth;
	int imHeight = srcImage->m_lpBMIH->biHeight;

	int numPixels = (imWidth - 2)*(imHeight - 2);

	FILE *fp = fopen(path.c_str(), "r");

	int value = 0;
	this->Disparity = (float*)malloc(sizeof(float)*numPixels);

	int count = 0;

	while (1)
	{
		if (fscanf(fp, "%d\n", &value) != 1)
		{
			break;
		}
		else
		{
			this->Disparity[count] = (float)value;
			count++;
		}
	}

	fclose(fp);

	int numPixels2 = imWidth*imHeight;
	//Here we start generate dispairity;
	this->rightDispairities = (int*)malloc(sizeof(int)*numPixels2);

	count = 0;

	//For left image
	for (size_t y = 1; y < imHeight - 1; y++)
	{
		for (size_t x = 1; x < imWidth - 1; x++)
		{
			int offset = y*imWidth + x;

			int disValue = (int)this->Disparity[count];

			rightDispairities[offset] = disValue;

			count++;
		}
	}

	//For hor margin:
	for (size_t y = 1; y < imHeight - 1; y++)
	{
		int offset = y*imWidth;
		int offset2 = (y + 1)*imWidth - 1;

		int disValue1 = rightDispairities[offset + 1];
		int disValue2 = rightDispairities[offset2 - 1];
		rightDispairities[offset] = disValue1;
		rightDispairities[offset2] = disValue2;
	}

	//For Vor margin:
	for (size_t x = 0; x < imWidth; x++)
	{
		int offset = x;
		int offset2 = (imHeight - 1)*imWidth + x;

		int disValue1 = rightDispairities[offset + imWidth];
		int disValue2 = rightDispairities[offset2 - imWidth];

		rightDispairities[offset] = disValue1;
		rightDispairities[offset2] = disValue2;
	}
}

void LazySnapping::SaveDispairityImage()
{
	CDib leftImage;
	CDib rightImage;
	leftImage.CopyImageInMemory(srcImage);
	rightImage.CopyImageInMemory(srcImage);

	int imWidth = srcImage->m_lpBMIH->biWidth;
	int imHeight = srcImage->m_lpBMIH->biHeight;

	int scale = 4;

	for (size_t y = 0; y < imHeight; y++)
	{
		for (size_t x = 0; x < imWidth; x++)
		{
			int offset = y*imWidth + x;
			int leftDisValue = 4 * leftDispairites[offset];
			int rightDisValue = 4 * rightDispairities[offset];

			offset = leftImage.GetPixelOffset(x, y);

			BYTE l = (BYTE)leftDisValue;
			BYTE r = (BYTE)rightDisValue;
			leftImage.SetPixel(offset, RGB(l, l, l));
			rightImage.SetPixel(offset, RGB(r, r, r));
		}
	}

	{
		CFile mfile;
		CString filename = _T("leftDis.bmp");
		if (!mfile.Open(filename, CFile::modeCreate | CFile::modeWrite))
		{
			return;
		}
		leftImage.Write(&mfile);
		mfile.Close();
	}
	{
		CFile mfile;
		CString filename = _T("rightDis.bmp");
		if (!mfile.Open(filename, CFile::modeCreate | CFile::modeWrite))
		{
			return;
		}
		rightImage.Write(&mfile);
		mfile.Close();
	}

}

void LazySnapping::ReadMatchPoint(string path)
{
	FILE *fp = fopen(path.c_str(), "r");

	int c[4] = { 0 };
	float distance = 0;

	while (1)
	{
		if (fscanf(fp, "%d %d %d %d %f", &c[0], &c[1], &c[2], &c[3], &distance) != 5)
		{
			break;
		}
		else
		{
			pair<int, int> newPair;
			newPair.first = c[0];
			newPair.second = c[1];

			pair<int, int> newPair2;
			newPair2.first = c[2];
			newPair2.second = c[3];

			left_features.push_back(newPair);
			right_features.push_back(newPair2);
		}
	}

	// Close the file
	fclose(fp);
}