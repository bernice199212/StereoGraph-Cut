#pragma once

// CStroke command target
#include "2DPoint.h"
class CStroke : public CObject
{
public:
	CStroke();
	CStroke(UINT nPenWidth, COLORREF cPenColor);
	virtual ~CStroke();
	virtual void Serialize(CArchive& ar);
protected:
	//	int m_nPenWidth;
public:
	BOOL DrawStroke(CDC* pDC, CPoint imgOri, float imgScale, C2DPoint clientOri);
	BOOL DrawStrokeVector(CDC* pDC, CPoint imgOri, float imgScale, C2DPoint clientOri);

public:
	UINT m_nPenWidth;
	COLORREF m_cPenColor;
	float *m_bBoxInPixel;
	bool *nodeProperty;//存放当前折点赋予属性，为true，则从右侧属性，即目标物，为false，则从左侧属性，为背景
public:
	CArray<CPoint, CPoint> m_pointArray;
	float imageScaleWhenMarked;//对应的被记录下时的图像的尺度，用于显示时用。
	C2DPoint clientOrionImg;//对应的被记录下时客户坐标系的原点在图像上的位置。

	C2DPoint *m_fPtSlope;//存放点的方向，最简单的是将点列排序后，相邻点相减得到，如果点列中存放的是粗线条，则需要拟合。

	int m_iWeight;//权重，用于表示该stroke用于运算时的重要性，主要是用在微调时，作用影响范围

	COLORREF GetStrokeColor();
	UINT GetStrokeWidth();
	float ComputePtSlope();
	//去除线条中重复的点。
	void RemoveRepetitivePts();
	//通过找点到每个线段的距离找到点到stroke的距离,注意，两个端点末端的半圆区域舍去
	float SideDistance2Stroke(int posx, int posy);
	//计算在relax下情况下是否在stroke的包围框内。
	bool AppDistance2Stroke(int posx, int posy, int relax);
	//计算stroke的boundingbox，都是图像坐标
	void ComputeBoundingBoxInPixel();
	//返回stroke的boundingbox
	float * GetBoundingBoxInPixel();
	void ComputeAnklePointProperty();
};


