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

// GCSPView.cpp : implementation of the CGCSPView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "GCSPInteractiveSegmentation.h"
#endif

#include "ConfigManager.h"
#include "GCSPDoc.h"
#include "GCSPView.h"
#include "MainFrm.h"
#include "2DPoint.h"
#include <math.h>
#include <iostream>
#include <fstream>

using namespace std;

#include "Stroke.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CGCSPView, CScrollView)

	BEGIN_MESSAGE_MAP(CGCSPView, CScrollView)
		ON_COMMAND(ID_FILE_PRINT, &CScrollView::OnFilePrint)
		ON_COMMAND(ID_FILE_PRINT_DIRECT, &CScrollView::OnFilePrint)
		ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CGCSPView::OnFilePrintPreview)
		ON_WM_CONTEXTMENU()
		ON_WM_RBUTTONUP()

		ON_COMMAND(ID_BUTTON_IMAGE_ZOOMIN, &CGCSPView::OnButtonImageZoomin)
		ON_COMMAND(ID_BUTTON_IMAGE_ZOOMOUT, &CGCSPView::OnButtonImageZoomout)
		ON_WM_LBUTTONDOWN()

		ON_WM_MOUSEMOVE()
		ON_WM_RBUTTONDOWN()
		ON_WM_ERASEBKGND()

		ON_WM_MOUSEMOVE()
		ON_COMMAND(ID_CHECK_SEG_SHOWLABELINFO, &CGCSPView::OnCheckSegShowlabelinfo)
		ON_UPDATE_COMMAND_UI(ID_CHECK_SEG_SHOWLABELINFO, &CGCSPView::OnUpdateCheckSegShowlabelinfo)

		ON_WM_LBUTTONUP()
		ON_COMMAND(ID_BUTTON_SEG_LAZYSNAPPING, &CGCSPView::OnButtonSegLazySnapping)

		ON_COMMAND(ID_BUTTON_READ_STROKE, &CGCSPView::OnButtonReadStroke)
	END_MESSAGE_MAP()

	CGCSPView::CGCSPView()
	{
		m_fImageScale = 1.0f;
		m_bImageSegLazySnapping = false;
		m_eOperationType = view;
		m_bShowLabelInfo = true;
		m_nPenWidth = 3;
		m_cPenColor = RGB(0,0,0);
		m_pStrokeCur = NULL;
		m_penCur.CreatePen(PS_SOLID,m_nPenWidth,m_cPenColor);
	}

	CGCSPView::~CGCSPView()
	{
		m_penCur.DeleteObject();
	}

	BOOL CGCSPView::PreCreateWindow(CREATESTRUCT& cs)
	{
		return CScrollView::PreCreateWindow(cs);
	}

	void CGCSPView::OnDraw(CDC *pDC)
	{
		CGCSPDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		if (!pDoc)
			return;

		//改变窗口标题
		CMainFrame *m_pMainWnd;
		m_pMainWnd = (CMainFrame *)AfxGetMainWnd();
		m_pMainWnd->SetWindowText(_T("GCSPInteractiveSegmentation"));

		//show the image//不加&&后面那个，在某些zoom值的时候会闪屏，可能是先后画两幅图像的原因吧
		if (pDoc->m_dibImage2 == NULL && pDoc->m_dibImage != NULL && (pDoc->m_dibSegedImage == NULL || m_bShowLabelInfo == false))
		{	
			if(!pDoc->m_dibImage->IsEmpty())
			{
				CSize sizeDibDisplay = pDoc->m_dibImage->GetDimensions(); 
				CSize sizeClient = sizeDibDisplay;

				sizeDibDisplay.cx = (LONG)(sizeDibDisplay.cx * m_fImageScale);//加上imgOri是想让客户区大一点，方便选择下面的像素
				sizeDibDisplay.cy = (LONG)(sizeDibDisplay.cy * m_fImageScale);

				sizeClient.cx = (LONG)((sizeClient.cx + pDoc->clientExtend.x + pDoc->imgOri.x)  * m_fImageScale);
				sizeClient.cy = (LONG)((sizeClient.cy  + pDoc->clientExtend.y + pDoc->imgOri.y) * m_fImageScale);

				CSize sizePage(sizeClient.cx / 10, sizeClient.cy / 10);
				CSize sizeLine(sizeClient.cx / 50, sizeClient.cy / 50);

				SetScrollSizes(MM_TEXT, sizeClient, sizePage, sizeLine);
				CPoint imgori;
				imgori.x= (int)(pDoc->imgOri.x * m_fImageScale);
				imgori.y= (int)(pDoc->imgOri.y * m_fImageScale);

				pDoc->m_dibImage->Draw(pDC,imgori,sizeDibDisplay);	
			}
		}
		if(pDoc->m_dibSegedImage != NULL && m_bShowLabelInfo == true)
		{
			if(!pDoc->m_dibSegedImage->IsEmpty())
			{

				CSize sizeDibDisplay = pDoc->m_dibSegedImage->GetDimensions(); 
				CSize sizeClient = sizeDibDisplay;
				sizeDibDisplay.cx = (LONG)(sizeDibDisplay.cx * m_fImageScale);//加上imgOri是想让客户区大一点，方便选择下面的像素
				sizeDibDisplay.cy = (LONG)(sizeDibDisplay.cy * m_fImageScale);
				sizeClient.cx = (LONG)2*(sizeClient.cx * m_fImageScale + pDoc->clientExtend.x);
				sizeClient.cy = (LONG)2*(sizeClient.cy * m_fImageScale + pDoc->clientExtend.y);
				CSize sizePage(sizeClient.cx / 10, sizeClient.cy / 10);
				CSize sizeLine(sizeClient.cx / 50, sizeClient.cy / 50);
				SetScrollSizes(MM_TEXT, sizeClient, sizePage, sizeLine);

				CPoint imgori;
				imgori.x = (int)(sizeDibDisplay.cx * m_fImageScale + pDoc->clientExtend.x + pDoc->imgOri.x * m_fImageScale);
				imgori.y = (int)(pDoc->imgOri.y * m_fImageScale);

				CPoint imgori2;
				imgori2.x = (int)(pDoc->imgOri.x * m_fImageScale);
				imgori2.y = (int)(pDoc->imgOri.y * m_fImageScale);

				pDoc->m_dibSegedImage->Draw(pDC,imgori2,sizeDibDisplay);	
				pDoc->m_dibSegedImage_back->Draw(pDC, imgori, sizeDibDisplay);
			}
		}
		if (pDoc->m_dibImage2 != NULL && pDoc->m_dibImage != NULL && (pDoc->m_dibSegedImage == NULL || m_bShowLabelInfo == false))
		{
			CSize sizeDibDisplay = pDoc->m_dibImage2->GetDimensions();
			CSize sizeClient = sizeDibDisplay;
			sizeDibDisplay.cx = (LONG)(sizeDibDisplay.cx * m_fImageScale);//加上imgOri是想让客户区大一点，方便选择下面的像素
			sizeDibDisplay.cy = (LONG)(sizeDibDisplay.cy * m_fImageScale);
			sizeClient.cx = (LONG)2*(sizeClient.cx * m_fImageScale + pDoc->clientExtend.x);
			sizeClient.cy = (LONG)2*(sizeClient.cy * m_fImageScale + pDoc->clientExtend.y);

			CSize sizePage(sizeClient.cx / 10, sizeClient.cy / 10);
			CSize sizeLine(sizeClient.cx / 50, sizeClient.cy / 50);

			SetScrollSizes(MM_TEXT, sizeClient, sizePage, sizeLine);

			CPoint imgori;
			imgori.x = (int)(sizeDibDisplay.cx * m_fImageScale + pDoc->clientExtend.x + pDoc->imgOri.x * m_fImageScale);
			imgori.y = (int)(pDoc->imgOri.y * m_fImageScale);

			CPoint imgori2;
			imgori2.x = (int)(pDoc->imgOri.x * m_fImageScale);
			imgori2.y = (int)(pDoc->imgOri.y * m_fImageScale);

			pDoc->m_dibImage->Draw(pDC, imgori2, sizeDibDisplay);
			pDoc->m_dibImage2->Draw(pDC, imgori, sizeDibDisplay);
		}

		//获取滚动条的位置，以connect图像坐标与当前所见（受滚动条控制）客户区坐标。所有在客户区获取的信息都是在当前看到的客户区坐标下的，如cues，bounding boxes of Objects
		int scrollClientPosX = GetScrollPos(SB_HORZ) ;
		int scrollClientPosY = GetScrollPos(SB_VERT) ;


		POSITION pos = pDoc->m_strokeList.GetHeadPosition();
		while(pos!=NULL)
		{
			CStroke *pStroke = pDoc->m_strokeList.GetNext(pos);
			pStroke->DrawStroke(pDC,pDoc->imgOri,m_fImageScale,C2DPoint((float)scrollClientPosX,(float)scrollClientPosY));
			pStroke->DrawStrokeVector(pDC,pDoc->imgOri,m_fImageScale,C2DPoint((float)scrollClientPosX,(float)scrollClientPosY));

		}


		CPen pen(PS_SOLID,1,RGB(234,23,53));
		CPen *pOldPen=pDC->SelectObject(&pen);
		CPen pen2(PS_SOLID,1,RGB(250,0,250));

		pDC->SelectObject(pOldPen);
	}


	void CGCSPView::OnInitialUpdate()
	{
		CScrollView::OnInitialUpdate();

		// TODO: Initialize the scrollview 做一个初始化，实际上是在ondraw函数中真正设置的
		CSize sizeTotal(800, 600); 
		CSize sizePage(sizeTotal.cx / 10, sizeTotal.cy / 10);
		CSize sizeLine(sizeTotal.cx /50, sizeTotal.cy / 50);
		SetScrollSizes(MM_TEXT, sizeTotal, sizePage, sizeLine);

	}

	// CGCSPView printing

	void CGCSPView::OnFilePrintPreview()
	{
#ifndef SHARED_HANDLERS
		AFXPrintPreview(this);
#endif
	}

	BOOL CGCSPView::OnPreparePrinting(CPrintInfo* pInfo)
	{
		// default preparation
		return DoPreparePrinting(pInfo);
	}

	void CGCSPView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
	{
		// TODO: add extra initialization before printing
	}

	void CGCSPView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
	{
		// TODO: add cleanup after printing
	}

	void CGCSPView::OnRButtonUp(UINT /* nFlags */, CPoint point)
	{

		CGCSPDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		if (!pDoc)
			return;
		if(GetCapture()!=this)
			return;//如果当前窗口没有捕捉鼠标，不画图
		CClientDC dc(this);

		m_penCur.DeleteObject();
		m_penCur.CreatePen(PS_SOLID,m_nPenWidth,m_cPenColor);
		CPen *pOldPen = dc.SelectObject(&m_penCur);
		dc.MoveTo(m_ptPrev);
		dc.LineTo(point);
		dc.SelectObject(pOldPen);
		m_pStrokeCur->m_pointArray.Add(CPoint((int)(point.x - pDoc->imgOri.x*m_fImageScale), (int)(point.y- pDoc->imgOri.y*m_fImageScale)));
		m_pStrokeCur->imageScaleWhenMarked = m_fImageScale;
		int scrollClientPosX = GetScrollPos(SB_HORZ);
		int scrollClientPosY = GetScrollPos(SB_VERT);
		m_pStrokeCur->clientOrionImg = C2DPoint((float)scrollClientPosX,(float)scrollClientPosY);
		ReleaseCapture();
		//启动像素收集
		if(m_bImageSegLazySnapping)
		{
			CollectCurrentLebelledPixels(_T("background"));
		}
		//如果既有前景又有背景线索了，启动分割过程。
		if(HavingBothFGandBGCues())
		{
			pDoc->SegmentationByCues(NULL);
		}

	}

	bool CGCSPView::HavingBothFGandBGCues()
	{
		CGCSPDoc* pDoc = GetDocument();
		POSITION pos = pDoc->m_strokeList.GetHeadPosition();
		bool hasFgCue = false;
		bool hasBgCue = false;
		while(pos!=NULL)
		{
			CStroke *pStroke = pDoc->m_strokeList.GetNext(pos);
			COLORREF strokecolor = pStroke->GetStrokeColor();
			if(hasFgCue == false)
			{
				if(GetRValue(strokecolor) == 255 && GetGValue(strokecolor) == 0 && GetBValue(strokecolor) == 0)
				{
					hasFgCue = true;
				}
			}
			if(hasBgCue == false)
			{
				if(GetRValue(strokecolor) == 0 && GetGValue(strokecolor) == 0 && GetBValue(strokecolor) == 255)
				{
					hasBgCue = true;
				}
			}
			if(hasBgCue == true && hasFgCue == true)
			{
				break;
			}
		}
		if(hasBgCue == true && hasFgCue == true)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void CGCSPView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
	{
#ifndef SHARED_HANDLERS
		theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
	}

	// CGCSPView diagnostics

#ifdef _DEBUG
	void CGCSPView::AssertValid() const
	{
		CScrollView::AssertValid();
	}

	void CGCSPView::Dump(CDumpContext& dc) const
	{
		CScrollView::Dump(dc);
	}

	CGCSPDoc* CGCSPView::GetDocument() const // non-debug version is inline
	{
		ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGCSPDoc)));
		return (CGCSPDoc*)m_pDocument;
	}
#endif //_DEBUG

	void CGCSPView::OnButtonImageZoomin()
	{
		CGCSPDoc *pDoc=GetDocument();
		ASSERT_VALID(pDoc);
		if (!pDoc)
			return;
		if(pDoc->m_dibImage == NULL)
		{
			::AfxMessageBox(_T("Please open an image first!"));
			return;
		}
		//操作类型设置
		m_eOperationType = view;
		m_fImageScale = m_fImageScale * 1.1f;
		Invalidate();
	}


	void CGCSPView::OnButtonImageZoomout()
	{
		CGCSPDoc *pDoc=GetDocument();
		ASSERT_VALID(pDoc);
		if (!pDoc)
			return;
		if(pDoc->m_dibImage == NULL)
		{
			::AfxMessageBox(_T("Please open an image first!"));
			return;
		}
		//操作类型设置
		m_eOperationType = view;

		m_fImageScale = m_fImageScale * 0.8f;
		Invalidate();
	}

	void CGCSPView::OnLButtonDown(UINT nFlags, CPoint point)
	{
		CGCSPDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		if (!pDoc)
			return;

		//勾勒前景线索
		if (m_bImageSegLazySnapping)
		{		
			m_cPenColor = RGB(255,0,0);
			m_pStrokeCur = NewStroke();
			m_pStrokeCur->m_pointArray.Add(CPoint((int)(point.x - pDoc->imgOri.x*m_fImageScale), (int)(point.y- pDoc->imgOri.y*m_fImageScale)));
			SetCapture();//捕捉鼠标直到button up
			m_ptPrev = point;//
			m_penCur.DeleteObject();
			m_penCur.CreatePen(PS_SOLID,m_nPenWidth,m_cPenColor);

		}

		CScrollView::OnLButtonDown(nFlags, point);
	}

	void CGCSPView::OnRButtonDown(UINT nFlags, CPoint point)
	{
		// 右键落下时
		CGCSPDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		if (!pDoc)
			return;

		int scrollClientPosX = GetScrollPos(SB_HORZ);
		int scrollClientPosY = GetScrollPos(SB_VERT);

		//勾勒背景线索
		if (m_bImageSegLazySnapping)
		{
			m_cPenColor = RGB(0,0,255);
			m_pStrokeCur = NewStroke();
			m_pStrokeCur->m_pointArray.Add(CPoint((int)(point.x - pDoc->imgOri.x*m_fImageScale), (int)(point.y- pDoc->imgOri.y*m_fImageScale)));
			SetCapture();//捕捉鼠标直到button up
			m_ptPrev = point;//
			m_penCur.DeleteObject();
			m_penCur.CreatePen(PS_SOLID,m_nPenWidth,m_cPenColor);

		}

		CScrollView::OnRButtonDown(nFlags, point);
		m_eOperationType = view;
	}


	BOOL CGCSPView::OnEraseBkgnd(CDC* pDC)
	{
		//当前操作类型影响背景刷新与否
		if (m_eOperationType == view) 
		{
			return CScrollView::OnEraseBkgnd(pDC);
		}
		else
		{
			return TRUE;
		}
	}

	void CGCSPView::OnMouseMove(UINT nFlags, CPoint point)
	{
		// TODO: Add your message handler code here and/or call default	
		CGCSPDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		if (!pDoc)
			return;
		if(GetCapture()!=this)
			return;//如果当前窗口没有捕捉鼠标，不画图

		m_pStrokeCur->m_pointArray.Add(CPoint((int)(point.x - pDoc->imgOri.x*m_fImageScale), (int)(point.y- pDoc->imgOri.y*m_fImageScale)));	
		CClientDC dc(this);
		CPen *pOldPen = dc.SelectObject(&m_penCur);
		dc.MoveTo(m_ptPrev);
		dc.LineTo(point);
		dc.SelectObject(pOldPen);
		m_ptPrev = point;

		CScrollView::OnMouseMove(nFlags, point);
	}

	void CGCSPView::OnCheckSegShowlabelinfo()
	{
		// TODO: Add your command handler code here
		m_bShowLabelInfo = !m_bShowLabelInfo;
		Invalidate();
	}


	void CGCSPView::OnUpdateCheckSegShowlabelinfo(CCmdUI *pCmdUI)
	{
		// TODO: Add your command update UI handler code here
		pCmdUI->SetCheck(m_bShowLabelInfo);
		//Invalidate();
	}

	CStroke* CGCSPView::NewStroke()
	{
		CGCSPDoc* pDoc = GetDocument();
		CStroke * pStrokeItem = new CStroke(m_nPenWidth,m_cPenColor);
		pDoc->m_strokeList.AddTail(pStrokeItem);
		return pStrokeItem;
	}

	void CGCSPView::OnLButtonUp(UINT nFlags, CPoint point)
	{
		// TODO: Add your message handler code here and/or call default
		if(GetCapture()!=this)
			return;//如果当前窗口没有捕捉鼠标，不画图

		CGCSPDoc *pDoc=GetDocument();
		ASSERT_VALID(pDoc);
		if (!pDoc)
			return;
		CClientDC dc(this);

		m_penCur.DeleteObject();
		m_penCur.CreatePen(PS_SOLID,m_nPenWidth,m_cPenColor);
		CPen *pOldPen = dc.SelectObject(&m_penCur);
		dc.MoveTo(m_ptPrev);
		dc.LineTo(point);
		dc.SelectObject(pOldPen);
		m_pStrokeCur->m_pointArray.Add(CPoint((int)(point.x - pDoc->imgOri.x*m_fImageScale), (int)(point.y- pDoc->imgOri.y*m_fImageScale)));
		m_pStrokeCur->imageScaleWhenMarked = m_fImageScale;
		int scrollClientPosX = GetScrollPos(SB_HORZ);
		int scrollClientPosY = GetScrollPos(SB_VERT);
		m_pStrokeCur->clientOrionImg = C2DPoint((float)scrollClientPosX,(float)scrollClientPosY);
		ReleaseCapture();

		//启动像素收集
		if(m_bImageSegLazySnapping)
		{
			CollectCurrentLebelledPixels(_T("foreground"));
			//如果既有前景又有背景线索了，启动分割过程。
			if(HavingBothFGandBGCues())
			{
				pDoc->SegmentationByCues(NULL);
			}
		}
	}

	void CGCSPView::OnButtonSegLazySnapping()
	{
		// 开始lazysnapping分割过程。尚没有停止状态的操作，今后如需接续其他操作，可通过改变m_bImageSegLazySnapping状态来终止
		//labellImage分配内存。

		CGCSPDoc *pDoc=GetDocument();
		ASSERT_VALID(pDoc);
		if (!pDoc)
			return;
		if(pDoc->m_dibImageWithStrokes == NULL)
		{
			if(pDoc->m_dibImage == NULL)
			{
				::AfxMessageBox(_T("Please open an image first!"));
				return;
			}
			pDoc->m_dibImageWithStrokes = new CDib();
			pDoc->m_dibImageWithStrokes->CopyImageInMemory(pDoc->m_dibImage);
		}
		m_bImageSegLazySnapping = true;
	}


	void CGCSPView::CollectCurrentLebelledPixels(CString foreOrBackGround)
	{
		//原先是收集到m_pixLazySnappingBgInputColors和 m_pixLazySnappingFgInputColors并且修改m_dibImageWithStrokes。
		//现在不收集了，基于像素和基于watershed过分割的收集结果不一样，在lazysnapping中再收集
		CGCSPDoc *pDoc=GetDocument();
		ASSERT_VALID(pDoc);
		if (!pDoc)
			return;

		//此处加一个判断，主要是因为，在做完一张图像时，再打开新的图像，往往不去点击并激发
		//OnButtonSegLazySnapping函数，以分配m_dibImageWithStrokes的空间。
		if(pDoc->m_dibImageWithStrokes == NULL)
		{
			if(pDoc->m_dibImage == NULL)
			{
				::AfxMessageBox(_T("Please open an image first!"));
				return;
			}
			pDoc->m_dibImageWithStrokes = new CDib();
			pDoc->m_dibImageWithStrokes->CopyImageInMemory(pDoc->m_dibImage);
		}

		//启动像素收集，把勾勒到的像素在m_dibImageWithStrokes更改颜色
		//原来只是简单保存stoke内存储的点，结果发现虽然程序显示的画笔很浓并连续，但保存下来的像素很少，是mousemove的关键点，非连续
		//所以此处除了保存stroke上的关键点之外，还要按照其宽度扩展一些，同时加入链接关键点的那些。
		//后续加入的生成方向场的那个只需要记录单点就可以。
		vector<C2DPoint> localCuePt;
		for(int i=0; i<m_pStrokeCur->m_pointArray.GetSize()-1; i++)
		{
			//被坐标变换搞懵了，cOrionImg得加上才行，虽然不清楚为什么了。但结果验证就是这样，
			float pixX0 = (float)((m_pStrokeCur->m_pointArray.GetAt(i).x + m_pStrokeCur->clientOrionImg.x)/m_fImageScale);
			float pixY0 = (float)((m_pStrokeCur->m_pointArray.GetAt(i).y + m_pStrokeCur->clientOrionImg.y)/m_fImageScale); 

			if(foreOrBackGround == "foreground") 
			{
				if(i==(m_pStrokeCur->m_pointArray.GetSize()-2)/2)
				{
					CGCSPApp *app = (CGCSPApp *)AfxGetApp();
					app->midX[app->count1]=pixX0;
					app->midY[app->count1]=pixY0;
					app->count1++;
				}
			}
			if(foreOrBackGround == "background") 
			{
				if(i==0)
				{
					CGCSPApp *app = (CGCSPApp *)AfxGetApp();
					app->minX[app->count2]=pixX0;
					app->minY[app->count2]=pixY0;
				}
				if(i==m_pStrokeCur->m_pointArray.GetSize()-2)
				{
					CGCSPApp *app = (CGCSPApp *)AfxGetApp();
					app->maxX[app->count2]=pixX0;
					app->maxY[app->count2]=pixY0;
					app->count2++;
				}
			}

			//用于生成方向场的，只记录关键点
			/*if(foreOrBackGround == "microadjustment")
			{
				//用于传给别的类计算
				LONG posStore = pDoc->m_dibImage->GetPixelOffset((int)pixX0,(int)pixY0);
				pDoc->m_dibImageWithStrokes->SetPixel(posStore,RGB(255,0,255));

			}*/
			//用于提取前背景颜色模型的，多取一些点
			float pixX1 = (float)((m_pStrokeCur->m_pointArray.GetAt(i+1).x + m_pStrokeCur->clientOrionImg.x)/m_fImageScale);
			float pixY1 = (float)((m_pStrokeCur->m_pointArray.GetAt(i+1).y + m_pStrokeCur->clientOrionImg.y)/m_fImageScale);

			C2DPoint dir;
			dir.x = pixX1 - pixX0;
			dir.y = pixY1 - pixY0;

			int radius = (int)(m_pStrokeCur->GetStrokeWidth()/2);//忽略奇数还是偶数，如线宽为3或2其半径皆为1

			int minX = int(min(pixX0 - radius,pixX1 - radius)+0.5);//四舍五入
			int maxX = int(max(pixX0 + radius,pixX1 + radius)+0.5);
			int minY = int(min(pixY0 - radius,pixY1 - radius)+0.5);
			int maxY = int(max(pixY0 + radius,pixY1 + radius)+0.5);
			for (int m = minX; m< maxX; m ++)
			{
				for (int n = minY; n < maxY; n ++)
				{
					C2DPoint dis2Orivector(m - pixX0, n - pixY0);
					float modDis2Orivector = dis2Orivector.sqrt2d();
					bool toBeAdded = false;
					if(modDis2Orivector == 0)
					{
						toBeAdded = true;
					}
					else
					{
						float dotcross = dis2Orivector.Cos2DPoint(dir)/modDis2Orivector;
						float dis2Line = sqrt(1 - dotcross*dotcross) * modDis2Orivector ;
						if (dis2Line <= radius)
						{
							toBeAdded = true;
						}
					}
					if(toBeAdded == true)
					{
						LONG posStore = pDoc->m_dibImage->GetPixelOffset(m,n);

						//RGBQUAD pixColor = pDoc->m_dibImage->GetPixel(posStore);

						if(foreOrBackGround == "foreground") 
						{

							pDoc->m_dibImageWithStrokes->SetPixel(posStore,RGB(255,0,0));//改写标注图像
						}
						else if(foreOrBackGround == "background")
						{
							pDoc->m_dibImageWithStrokes->SetPixel(posStore,RGB(0,0,255));
						}
				}
			}
		}
	}
}

void CGCSPView::OnButtonReadStroke()
{
	CGCSPDoc* pDoc = GetDocument();
	if (pDoc->m_dibImage == NULL)
	{
		::AfxMessageBox(_T("please open original image first"));
		return;
	}

	string file_path_str = ConfigManager::Instance()->work_path()+"ImageWithStrokes.bmp";
	CString filename(file_path_str.c_str());
	CFile file;
	CFileException fe;
	if (!file.Open(filename, CFile::modeRead | CFile::shareDenyWrite, &fe))
	{
		return;
	}

	pDoc->m_dibImageWithStrokes = new CDib;
	if (!pDoc->m_dibImageWithStrokes->Read(&file)){
		return;
	}
	file.Close();

	pDoc->SegmentationByCues(NULL);
}
