// Stroke.cpp : implementation file
//

#include "stdafx.h"
#include "GCSPInteractiveSegmentation.h"
#include "Stroke.h"
#include "2DPoint.h"
#include <math.h>
#include <iostream>
#include <fstream>


// CStroke

CStroke::CStroke()
{
	//m_nPenWidth = 1;
}

CStroke::CStroke(UINT nPenWidth, COLORREF cPenColor)
{
	m_nPenWidth = nPenWidth;
	m_cPenColor = cPenColor;
	m_fPtSlope = NULL;
	m_bBoxInPixel = NULL;
	m_iWeight = 0;
	nodeProperty = NULL;
}

CStroke::~CStroke()
{
	if (m_fPtSlope)
	{
		delete[]m_fPtSlope;
		m_fPtSlope = NULL;
	}
	if (m_bBoxInPixel)
	{
		delete[]m_bBoxInPixel;
		m_bBoxInPixel = NULL;
	}
	if (nodeProperty)
	{
		delete[]nodeProperty;
		nodeProperty = NULL;
	}
}


// CStroke member functions


void CStroke::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{	// storing code
		ar << (WORD)m_nPenWidth;
		m_pointArray.Serialize(ar);
	}
	else
	{	// loading code
		WORD w;
		ar >> w;
		m_nPenWidth = w;
		m_pointArray.Serialize(ar);
	}
}
BOOL CStroke::DrawStrokeVector(CDC* pDC, CPoint imgOri, float imgScale, C2DPoint clientOri)
{
	if (m_pointArray.GetSize() == 0 || m_fPtSlope == NULL)
	{
		return false;
	}
	CPen penStroke;
	COLORREF vectorColor = RGB(128, 0, 255);

	if (!penStroke.CreatePen(PS_SOLID, m_nPenWidth, RGB(GetRValue(vectorColor), GetGValue(vectorColor), GetBValue(vectorColor))))
	{
		return FALSE;
	}
	CPen * pOldPen = pDC->SelectObject(&penStroke);
	int arrowMLen = 16;
	int arrowSLen = 3;
	float tempx, tempy;
	float temptipx, temptipy;
	for (int i = 0; i<m_pointArray.GetSize(); i += 1)
	{
		tempx = (m_pointArray[i].x + clientOrionImg.x + imgOri.x*imageScaleWhenMarked)*imgScale / imageScaleWhenMarked;
		tempy = (m_pointArray[i].y + clientOrionImg.y + imgOri.x*imageScaleWhenMarked)*imgScale / imageScaleWhenMarked;
		pDC->MoveTo(CPoint((int)tempx, (int)tempy));
		//箭头,因为本程序跟矢量方向无关，所以箭头朝向统一定为朝向右方，即deltaX>0,deltaY看slope的正负
		temptipx = tempx + arrowMLen*m_fPtSlope[i].x;
		temptipy = tempy + arrowMLen*m_fPtSlope[i].y;
		pDC->LineTo(CPoint((int)temptipx, (int)temptipy));
	}

	pDC->SelectObject(pOldPen);
	return TRUE;
}

BOOL CStroke::DrawStroke(CDC* pDC, CPoint imgOri, float imgScale, C2DPoint clientOri)
{
	//(LONG)((tempcontour->m_p2DPolygonContour.GetPoint(0)->x+tempcontour->GetClientOrionImg()->x + pDoc->imgOri.x*tempcontour->GetImageScaleWhenMarked())* m_fImageScale/tempcontour->GetImageScaleWhenMarked());
	if (m_pointArray.GetSize() == 0)
		return false;
	CPen penStroke;
	if (!penStroke.CreatePen(PS_SOLID, m_nPenWidth, RGB(GetRValue(m_cPenColor), GetGValue(m_cPenColor), GetBValue(m_cPenColor))))
	{
		return FALSE;
	}
	CPen * pOldPen = pDC->SelectObject(&penStroke);
	float tempx = (m_pointArray[0].x + clientOrionImg.x + imgOri.x*imageScaleWhenMarked)*imgScale / imageScaleWhenMarked;
	float tempy = (m_pointArray[0].y + clientOrionImg.y + imgOri.x*imageScaleWhenMarked)*imgScale / imageScaleWhenMarked;
	pDC->MoveTo(CPoint((int)tempx, (int)tempy));
	for (int i = 1; i<m_pointArray.GetSize(); i++)
	{
		tempx = (m_pointArray[i].x + clientOrionImg.x + imgOri.x*imageScaleWhenMarked)*imgScale / imageScaleWhenMarked;
		tempy = (m_pointArray[i].y + clientOrionImg.y + imgOri.x*imageScaleWhenMarked)*imgScale / imageScaleWhenMarked;
		pDC->LineTo(CPoint((int)tempx, (int)tempy));
	}

	pDC->SelectObject(pOldPen);
	return TRUE;
}



COLORREF CStroke::GetStrokeColor()
{
	return m_cPenColor;
}

UINT CStroke::GetStrokeWidth()
{
	return m_nPenWidth;
}

void CStroke::ComputeBoundingBoxInPixel()
{
	//像素坐标的boundingbox
	float minX = 10000, minY = 10000;
	float maxX = -1, maxY = -1;
	if (m_bBoxInPixel)
	{
		delete[]m_bBoxInPixel;
		m_bBoxInPixel = NULL;
	}
	m_bBoxInPixel = new float[4];
	for (int i = 0; i<m_pointArray.GetSize(); i++)
	{
		float pixX = (float)((m_pointArray.GetAt(i).x + clientOrionImg.x) / imageScaleWhenMarked);
		float pixY = (float)((m_pointArray.GetAt(i).y + clientOrionImg.y) / imageScaleWhenMarked);
		//float pixX1 = (float)((m_pointArray.GetAt(m_pointArray.GetSize()-1).x + clientOrionImg.x)/imageScaleWhenMarked);
		//float pixY1 = (float)((m_pointArray.GetAt(m_pointArray.GetSize()-1).y + clientOrionImg.y)/imageScaleWhenMarked);
		if (pixX<minX)
		{
			minX = pixX;
		}
		if (pixX>maxX)
		{
			maxX = pixX;
		}
		if (pixY<minY)
		{
			minY = pixY;
		}
		if (pixY>maxY)
		{
			maxY = pixY;
		}
	}
	m_bBoxInPixel[0] = minX;
	m_bBoxInPixel[1] = maxX;
	m_bBoxInPixel[2] = minY;
	m_bBoxInPixel[3] = maxY;
}

float * CStroke::GetBoundingBoxInPixel()
{
	if (m_bBoxInPixel == NULL)//像素坐标
	{
		ComputeBoundingBoxInPixel();
	}
	return m_bBoxInPixel;
}

bool CStroke::AppDistance2Stroke(int posx, int posy, int relax)
{
	//估计stroke在relax条件下，大致的影响范围
	//注意：posx与posy是图像像素，stroke是直接从屏幕获得，需要事先做变换

	if (m_bBoxInPixel == NULL)//像素坐标
	{
		ComputeBoundingBoxInPixel();
	}
	float minX = m_bBoxInPixel[0], minY = m_bBoxInPixel[2];
	float maxX = m_bBoxInPixel[1], maxY = m_bBoxInPixel[3];
	if (posx>minX - relax&& posx<maxX + relax&& posy>minY - relax&& posy<maxY + relax)
	{
		return true;
	}
	else
	{
		return false;
	}
}
int PtDistance2LineSegment(C2DPoint PA, C2DPoint PB, C2DPoint P3, float &dis)
{
	//返回值情况如下,0：与PA重合；1：与PB重合；2：两个端点重合取与PA的距离
	//3：距离PA为长边，钝角，取与PB的距离；4：距离PB为长边，钝角，取与PA的距离
	//5：在PA与PB的范围内，取垂直距离
	//dis已经加入正负，在线段右侧为正，左侧为负
	float a, b, c;

	a = Distance(PA, P3);
	if (a <= 0.00001)
	{
		dis = 0.0f;
		return 0;
	}
	b = Distance(PB, P3);
	if (b <= 0.00001)
	{
		dis = 0.0f;
		return 1;
	}

	c = Distance(PA, PB);
	//两个端点重合，距离为其中一个,stroke已经处理过，可以忽略这种情况
	if (c <= 0.00001)
	{
		dis = a;
		return 2;
	}

	//     P3
	//  (a)   (b)
	//PA----(c)----PB
	//判断左右
	C2DPoint pab(PB.x - PA.x, PB.y - PA.y);
	C2DPoint pa3(P3.x - PA.x, P3.y - PA.y);
	float crossx = pab.x*pa3.y - pa3.x*pab.y;//利用叉乘判断左右，为正则图像上点P1在线段左侧，为负则右，为零则在线上，之所以
	//与右手螺旋式正负相反，是因为图像上Y坐标轴朝下
	if (crossx<0) crossx = 1;
	else if (crossx >= 0) crossx = -1; //反过来了，右侧为正，左侧为负
	if (a*a >= b*b + c*c)
	{
		dis = crossx*b;      //如果a长边，钝角情况下距离为b   
		return 3;
	}
	if (b*b >= a*a + c*c)
	{
		dis = crossx*a;      //如果b长边，钝角情况下距离为a  
		return 4;
	}

	float l = (a + b + c) / 2;     //周长的一半   
	float s = sqrt(abs(l*(l - a)*(l - b)*(l - c)));  //海伦公式求面积，也可以用矢量求   
	dis = crossx * 2 * s / c;
	return 5;
}
void CStroke::ComputeAnklePointProperty()
{
	//给当前折点赋予属性，
	//         /
	//        /
	// nodeN /  \
			//       \  /  angle a
	//        \
			//         \
			//如果a<180，N取另一侧的属性（即+，-），即大角侧的属性。那么所有最小距离为到N的，取为N的属性
	nodeProperty = new bool[m_pointArray.GetSize()];
	nodeProperty[0] = false;
	nodeProperty[m_pointArray.GetSize() - 1] = false;//第一个和最后一个不用，非折点
	for (int i = 1; i< m_pointArray.GetSize() - 1; i++)
	{
		//因为只是做角度判断，跟坐标是图像坐标还是屏幕坐标无关，不需要做坐标变换
		float pixX0 = m_pointArray.GetAt(i - 1).x;
		float pixY0 = m_pointArray.GetAt(i - 1).y;
		float pixX1 = m_pointArray.GetAt(i).x;
		float pixY1 = m_pointArray.GetAt(i).y;
		float pixX2 = m_pointArray.GetAt(i + 1).x;
		float pixY2 = m_pointArray.GetAt(i + 1).y;

		C2DPoint PA(pixX0, pixY0);
		C2DPoint PB(pixX1, pixY1);
		C2DPoint PC(pixX2, pixY2);


		//    PA
		//   /
		//  / 
		//PB--------PC
		//判断左右
		C2DPoint pbc(PC.x - PB.x, PC.y - PB.y);
		C2DPoint pba(PA.x - PB.x, PA.y - PB.y);
		float crossx = pbc.x*pba.y - pbc.y*pba.x;//利用叉乘判断左右，为负则PA在右，为正则左，为零则在线上，
		//之所以与右手螺旋式判断法相反是因为图像上Y坐标轴朝下
		if (crossx<0)
		{
			//crossx = -1;//PA-PB-PC 从右侧来说为小角度，折点属性随左侧，为负
			nodeProperty[i] = false;
		}
		if (crossx >= 0)
		{
			//crossx = 1;//PA-PB-PC 相对右侧来说，为大角度，如图所示，折点属性随左右侧，为+
			nodeProperty[i] = true;
		}


	}
}

float CStroke::SideDistance2Stroke(int posx, int posy)
{
	//通过找点到每个线段的距离找到点到stroke的距离,注意，两个端点末端的半圆区域舍去
	//另外，在大角（>180）的折线部分，有一段区域是距离折点最小的，要根据折点属性来判断左右，而非线段的左右。
	//后者不同线段间会有不同结果，造成错误判断。
	RemoveRepetitivePts();//去除重复的点

	if (!nodeProperty)
	{
		ComputeAnklePointProperty();
	}

	float dismin = 10000;
	int disminI = -1;
	int disminType = -1;
	for (int i = 0; i< m_pointArray.GetSize() - 1; i++)
	{
		//注意：posx与posy是图像像素，stroke是直接从屏幕获得，需要事先做变换
		float pixX0 = (float)((m_pointArray.GetAt(i).x + clientOrionImg.x) / imageScaleWhenMarked);
		float pixY0 = (float)((m_pointArray.GetAt(i).y + clientOrionImg.y) / imageScaleWhenMarked);
		float pixX1 = (float)((m_pointArray.GetAt(i + 1).x + clientOrionImg.x) / imageScaleWhenMarked);
		float pixY1 = (float)((m_pointArray.GetAt(i + 1).y + clientOrionImg.y) / imageScaleWhenMarked);

		C2DPoint PA(pixX0, pixY0);
		C2DPoint PB(pixX1, pixY1);

		//只要左右侧向的影响范围内，两个端点所控制的半圆不在范围内。
		C2DPoint P3((float)posx, (float)posy);
		float dis = 10000;
		int distype = PtDistance2LineSegment(PA, PB, P3, dis);

		//暂不考虑首位相接的，等距离长的计算调试好了再看这一部分
		if (fabs(fabs(dis) - fabs(dismin))<0.001)
		{//此项设置主要是用于首位相叠时，避免因为最初的端点，和最后的端点而造成一部分区域损失
			//在距离相等时，判断是否首末点，是则更改	
			if ((i == 0 && i == 4) || (i == m_pointArray.GetSize() - 2 && i == 3)) //始末端点判断时，不置换
			{
				continue;
			}
			else
			{
				dismin = dis;
				disminI = i;
				disminType = distype;
			}
		}
		else if (fabs(dis)<fabs(dismin))
		{
			dismin = dis;
			disminI = i;
			disminType = distype;
		}
	}
	//根据折点属性修正
	if (disminType == 3 || disminType == 4)
	{
		//去掉两个端点末端的半圆区域,即位于该区域的点都给予大的距离值
		if (disminI == 0 && disminType == 4)
			dismin = 1000; //算了，..........去掉的时候末点处总有一点缺失
		else if (disminI == m_pointArray.GetSize() - 2 && disminType == 3)
			dismin = 1000;
		else
		{ //修正最小距离为到折点的正负属性
			if (disminType == 3)
			{
				//跟线段末点相关
				if (nodeProperty[disminI + 1] == true)
				{
					dismin = fabs(dismin);
				}
				else
				{
					dismin = -fabs(dismin);
				}
			}
			if (disminType == 4)
			{
				//跟线段起点相关
				if (nodeProperty[disminI] == true)
				{
					dismin = fabs(dismin);
				}
				else
				{
					dismin = -fabs(dismin);
				}
			}
		}
	}

	return dismin;
}

void CStroke::RemoveRepetitivePts()
{
	CArray<CPoint, CPoint> ptarrTemp;
	for (int i = 0; i< m_pointArray.GetSize(); i++)
	{
		CPoint pt = m_pointArray.GetAt(i);
		ptarrTemp.Add(pt);
	}
	m_pointArray.RemoveAll();
	m_pointArray.Add(ptarrTemp.GetAt(0));
	for (int i = 1; i<ptarrTemp.GetSize(); i++)
	{
		float deltaX = (float)(ptarrTemp.GetAt(i).x - ptarrTemp.GetAt(i - 1).x);
		float deltaY = (float)(ptarrTemp.GetAt(i).y - ptarrTemp.GetAt(i - 1).y);

		if (!(deltaX == 0 && deltaY == 0))
		{
			//重复的不加入
			m_pointArray.Add(ptarrTemp.GetAt(i));
		}
	}
	ptarrTemp.RemoveAll();
}

float CStroke::ComputePtSlope()
{
	//计算点的方向，假定点已经排序，返回线条长度。此函数所得点的方向脱离点位置意义。
	float curveLength = 0.0f;
	RemoveRepetitivePts();//去除重复的点
	int vecNum = m_pointArray.GetSize();
	if (m_fPtSlope)
	{
		delete[]m_fPtSlope;
		m_fPtSlope = NULL;
	}
	m_fPtSlope = new C2DPoint[vecNum];
	//计算方向场，最后一个点的方向不需计算，直接沿用上一个的
	for (int i = 0; i< vecNum - 1; i++)
	{
		//计算方向
		float deltaX = (float)(m_pointArray.GetAt(i + 1).x - m_pointArray.GetAt(i).x);
		float deltaY = (float)(m_pointArray.GetAt(i + 1).y - m_pointArray.GetAt(i).y);

		//前面已经排除了双零的情况
		m_fPtSlope[i].SetPoint(deltaX, deltaY);
		m_fPtSlope[i] = m_fPtSlope[i].Unify();
		curveLength += sqrt(deltaX*deltaX + deltaY*deltaY);
	}
	m_fPtSlope[vecNum - 1] = m_fPtSlope[vecNum - 2];
	//光滑方向
	int smoothStp = 8;//每smoothStp个像素做一次平均,是奇数
	int halfStp = smoothStp / 2;
	for (int i = halfStp; i< vecNum - halfStp; i++)
	{
		C2DPoint vecS;
		vecS.SetPoint(0.0f, 0.0f);
		float vecY = 0.0f;
		for (int j = -halfStp; j <= halfStp; j++)
		{
			vecS.x += m_fPtSlope[i + j].x;
			vecS.y += m_fPtSlope[i + j].y;
		}
		m_fPtSlope[i] = vecS.Unify();
	}
	for (int i = 0; i<halfStp; i++)//把头尾的都拉直
	{
		m_fPtSlope[i] = m_fPtSlope[halfStp];
	}
	for (int i = vecNum - halfStp; i<vecNum; i++)
	{
		m_fPtSlope[i] = m_fPtSlope[vecNum - halfStp - 1];
	}
	return curveLength;
}

//void CStroke::SaveStroke(ofstream *fp){

//}