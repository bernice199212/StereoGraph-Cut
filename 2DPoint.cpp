// 2DPoint.cpp: implementation of the C2DPoint class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GCSPInteractiveSegmentation.h"
#include "2DPoint.h"
#include <math.h>


#define SQR(x)  ((x)*(x))
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

C2DPoint::C2DPoint()
{
	index=-1;
}
C2DPoint::C2DPoint(float tx,float ty)
{
	x=tx;
	y=ty;
}
C2DPoint::~C2DPoint()
{

}

const float Distance(C2DPoint p1, C2DPoint p2)
{
	return ((float)sqrt(SQR(p1.x-p2.x)+SQR(p1.y-p2.y)));
}

C2DPoint C2DPoint::Unify()
{
	C2DPoint unitp;
	if(x*x+y*y!=0){
		unitp.x=(float)(x/sqrt(x*x+y*y));
		unitp.y=(float)(y/sqrt(x*x+y*y));
		return unitp;
	}
	else{
		unitp.x=0.0f;
		unitp.y=0.0f;
		return unitp;
	}
}
double C2DPoint::operator ^(C2DPoint &pt)
{
	pt=pt.Unify();
	//由于向量已经单位化，两向量的夹角A满足cosA=两向量的点乘操作！
	//这里并不返回cosA或A，而返回一个1-cosA，其单调性与两向量交角相同！
	if((pt.x*pt.x+pt.y*pt.y)==0||(x*x+y*y)==0) return -1;
	return (1-fabs(x*pt.x+y*pt.y));
}

void C2DPoint::Rotate(int angle)
{
	//逆时针旋转，angle为角度
	float da=angle*3.14159f/180;
	float xt=(float)(x*cos(da)-y*sin(da));
	float yt=(float)(x*sin(da)+y*cos(da));
	x=xt;
	y=yt;

}

float C2DPoint::Cos2DPoint(C2DPoint &pt)
{
	pt=pt.Unify();
	return x*pt.x+y*pt.y;
}

float C2DPoint::Moduleof2PointCross(C2DPoint pt)
{
	float modu=x*pt.y-y*pt.x;
	return modu;
}

void C2DPoint::SetPoint(C2DPoint *pt)
{
	x=pt->x;
	y=pt->y;
}


float C2DPoint::sqrt2d()
{
	return (float)sqrt(x*x+y*y);
}

void C2DPoint::SetPoint(float tx, float ty)
{
	x = tx;
	y = ty;
}


