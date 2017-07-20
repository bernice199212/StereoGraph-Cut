// 2DPoint.h: interface for the C2DPoint class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_2DPOINT_H__6DF301ED_C157_43AE_96C2_37D8314D4E3A__INCLUDED_)
#define AFX_2DPOINT_H__6DF301ED_C157_43AE_96C2_37D8314D4E3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class C2DPoint  
{
public:
	float sqrt2d();
	void SetPoint(C2DPoint *pt);
	void SetPoint(float tx, float ty);
	float Moduleof2PointCross(C2DPoint pt);
	float Cos2DPoint(C2DPoint &pt);
	void Rotate(int angle);
	C2DPoint Unify();
	double C2DPoint::operator ^(C2DPoint& pt);
	friend const float Distance(C2DPoint p1,C2DPoint p2);
	float x;
	float y;
	int index;//作为一般用途的二维浮点数，该值没用。但在对应其他序列时可以用
	RGBQUAD color;
	C2DPoint();
	C2DPoint(float tx,float ty);
	virtual ~C2DPoint();

};

#endif // !defined(AFX_2DPOINT_H__6DF301ED_C157_43AE_96C2_37D8314D4E3A__INCLUDED_)
