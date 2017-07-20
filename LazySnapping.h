#pragma once

#include <iostream>
#include <vector>
#include <limits>

#include <assert.h>
#include <time.h>

#include "kmeans.h"
#include "graph.h"
#include "Stroke.h"
#include "Histogram.h"
#include "Dib.h"

using namespace std;

// Definitions used for k-means
#define _k 64
#define _dim 3
#define alpha 0.6f //用于生成输出图像的alpha值

typedef unsigned char uchar;

class LazySnapping
{
public:
	float *Disparity;

	int *leftDispairites;
	int *rightDispairities;

	LazySnapping();
	LazySnapping::LazySnapping(CDib* img, CDib* rightimg, CDib* labelledImg, CDib* result, CDib* right_result, uchar* foreground, uchar* background, CStroke *lAdjustStroke);
	~LazySnapping(void);
	void run();

private:

	// Images
	CDib* srcImage; 
	CDib* destImage;
	CDib* destImage2;
	CDib* srcImage2;

	CDib* leftDepthImage;
	CDib* rightDepthImage;

	CDib* markUpImage;

	// Node for minCut
	Graph::node_id* nodes;
	Graph* graph;

	// Colors used to mark the back, foreground
	uchar bg_color[3];
	uchar fg_color[3];

	vector<float> bg_input; //依据labelledImg进行计算，如果是基于像素的，收集像素，如果是基于过分割结果的，收集每个区域的均值
	vector<float> fg_input;
	KMeans<_dim, _k, float> kmean;

	typedef vector<float>::iterator Iterator;
	typedef vector<int>::iterator IntIterator;

	static void activate_fg(void);
	static void activate_bg(void);

	// Those are storage of features in left and right, note that the size need to be equal
	vector< pair<int, int>> left_features;
	vector< pair<int, int>> right_features;
	
	// Routine for compute max flow
	void computeMaxFlow();

	// Process both pair
	void ReadLeftDispairity(string path);
	void ReadRightDispairity(string path);
	void ReadMatchPoint(string path);
	void SaveDispairityImage();

	// Compute min distance to FOREGROUND CENTROIDS
	float min_fg(uchar* color);

	// Compute min distance to BACKGROUND CENTROIDS
	float min_bg(uchar* color);

	// Compute min distance to centroids
	float minCentroidDistance(uchar* color, uchar* centroids, int dim);

	// Compute the square of the euclidien distance between two points
	// c1: rgb color as array of uchar
	// c2: rgb color as vector
	float distance(const uchar* c1, const uchar* c2, int dim);

	float colorDistanceSq(uchar* a, uchar* b, int dim);

	// extract the result 
	void extractResult();

	// Computes the centroids of foreground 
	// and background
	void getCentroids();

	// Get user input from layer
	void build_stereograph();

	// 收集labelledImg中的已经标注的前背景像素，如果没有收集到，返回false
	bool collectLabelledPixels();
};


