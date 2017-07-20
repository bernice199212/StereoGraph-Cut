// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// GCSPView.h : interface of the CGCSPView class
//

#pragma once

#include "Stroke.h"
#include <vector>

class CGCSPView : public CScrollView
{
protected: // create from serialization only
	CGCSPView();
	DECLARE_DYNCREATE(CGCSPView)

	// Attributes
public:
	CGCSPDoc* GetDocument() const;

	// Operations
public:

	// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	CStroke* NewStroke();
protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	// Implementation
public:
	virtual ~CGCSPView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	//int static midX;
	//int static midY;

private:
	float m_fImageScale;

	bool  m_b3DStartGroundEdit;
	bool m_b3DStartObjPolyBox;
	bool m_b3DStartGroundPolyBox;

	int m_iCurrentEditedObject;

	bool m_bShowLabelInfo;
	bool m_bShowObjectBBox;

	enum {view, edit} m_eOperationType; //标志操作类型
	int m_iCurrentObject;
	CPoint m_pGroundCueTrackTemp[2];//存放当前正在画的ground cue的轨迹

	//Stroke相关
	UINT m_nPenWidth;//m_nPenWidth 代表当前画笔的宽度，因为ui不提供画笔的宽度选择，所以在构造函数中固定其值
	COLORREF m_cPenColor;////m_cPenColor代表当前画笔的颜色，在构造函数中初始化过，但在左键事件为前景红色，右键为背景蓝色 
	CPen m_penCur;
	CStroke* m_pStrokeCur;
	CPoint m_ptPrev;
	//存放stroke列表
	//CTypedPtrList<CObList, CStroke*> m_strokeList;
	CPen * GetCurrentPen() {return &m_penCur;}
	bool  m_bImageSegLazySnapping;

public:
	afx_msg void OnButtonImageZoomin();
	afx_msg void OnButtonImageZoomout();
	afx_msg void OnButtonSegRegularbbox();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnComb3delements();
	afx_msg void OnButtonSegPolygonbbox();
	afx_msg void OnButtonObjectsDelete();
	afx_msg void OnButtonSegPolishcontour();
	afx_msg void OnButtonEdit3dIndicateground();
	afx_msg void OnButton3deditComputeground();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnButton3deditReadinGroundCues();
	afx_msg void OnButton3deditReadingrounddepth();
	afx_msg void OnButton3deditCuespropagation();
	afx_msg void OnCheckSegShowlabelinfo();
	afx_msg void OnUpdateCheckSegShowlabelinfo(CCmdUI *pCmdUI);
	afx_msg void OnCheckSegShowbbox();
	afx_msg void OnUpdateCheckSegShowbbox(CCmdUI *pCmdUI);
	afx_msg void OnButtonEdit3dObjpolybox();
	afx_msg void OnButtonEdit3dGroundpolybox();

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnButtonSegLazySnapping();
	void CollectCurrentLebelledPixels(CString foreOrBackGround);
	bool HavingBothFGandBGCues();
	//float ComputeDirectionInVectorCue();
	afx_msg void OnButtonSegMicroadjust();
	afx_msg void OnButtonSegMicroadjustbrokenline();
	afx_msg void OnButtonSegMicroadjustbrokenlineweightadjust();
	afx_msg void OnButtonReadStroke();
};

#ifndef _DEBUG  // debug version in GCSPView.cpp
inline CGCSPDoc* CGCSPView::GetDocument() const
{ return reinterpret_cast<CGCSPDoc*>(m_pDocument); }
#endif

