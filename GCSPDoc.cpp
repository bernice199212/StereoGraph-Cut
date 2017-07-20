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

// GCSPDoc.cpp : implementation of the CGCSPDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "GCSPInteractiveSegmentation.h"
#endif

//added by myself
#include "Dib.h"
//
#include "ConfigManager.h"
#include "GCSPDoc.h"
#include "MainFrm.h"
#include <propkey.h>
#include "LazySnapping.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include "Stroke.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CGCSPDoc
IMPLEMENT_DYNCREATE(CGCSPDoc, CDocument)

	BEGIN_MESSAGE_MAP(CGCSPDoc, CDocument)
		ON_COMMAND(ID_BUTTON_IMAGE_OPEN, &CGCSPDoc::OnButtonImageOpen)
		ON_COMMAND(ID_BUTTON_IMAGE_OPEN2, &CGCSPDoc::OnButtonImageOpen2)
		ON_COMMAND(ID_BUTTON_IMAGE_DEPTH, &CGCSPDoc::OnButtonImageDepth)
		ON_COMMAND(ID_BUTTON_SEG_SAVEm_dibImageWithStrokes, &CGCSPDoc::OnButtonSegSavem_dibImageWithStrokes)
		ON_COMMAND(ID_BUTTON_SEG_OPENOVERSEGIMAGE, &CGCSPDoc::OnButtonSegOpenoversegimage)
		ON_COMMAND(ID_BUTTON_SAVE_STROKE, &CGCSPDoc::OnButtonSaveStroke)
		//ON_COMMAND(ID_BUTTON_READ_STROKE, &CGCSPDoc::OnButtonReadStroke)
		ON_COMMAND(ID_BUTTON_Auto_Segment, &CGCSPDoc::OnButtonAutoSegment)
	END_MESSAGE_MAP()

	// CGCSPDoc construction/destruction

	CGCSPDoc::CGCSPDoc()
	{
		// TODO: add one-time construction code here
		m_dibImage = NULL;
		m_iSelectedObject = 0;
		m_iSamplingInterval = 10;
		m_bPixelsinGroundCues = NULL;
		m_pClusters = NULL;
		m_iImageLabels = NULL;

		m_dibSegedImage = NULL;
		m_dibSegedImage_back = NULL;
		m_depthImage = NULL;

		m_dibImageWithStrokes = NULL;
		m_dibWatershedflagImg = NULL;

		m_dibImage2 = NULL;

		imgOri = CPoint(50,50);
		clientExtend = CPoint(50,50);
	}

	CGCSPDoc::~CGCSPDoc()
	{
		if (m_dibImage)
		{
			delete m_dibImage;
			m_dibImage = NULL;
		}

		if (m_bPixelsinGroundCues)
		{
			delete []m_bPixelsinGroundCues;
			m_bPixelsinGroundCues = NULL;
		}

		if(m_pClusters)
		{
			for(int i = 0; i<m_iClustersNum; i++)
			{
				delete []m_pClusters[i];
			}
			delete []m_pClusters;
			m_pClusters = NULL;
		}

		if(m_iImageLabels)
		{
			delete []m_iImageLabels;
			m_iImageLabels = NULL;
		}

		if(m_dibSegedImage)
		{
			delete []m_dibSegedImage;
			m_dibSegedImage = NULL;
		}

		if(m_dibImageWithStrokes)
		{
			delete []m_dibImageWithStrokes;
			m_dibImageWithStrokes = NULL;
		}

		if(m_dibWatershedflagImg)
		{
			delete []m_dibWatershedflagImg;
			m_dibWatershedflagImg = NULL;
		}

	}

	BOOL CGCSPDoc::OnNewDocument()
	{
		if (!CDocument::OnNewDocument())
			return FALSE;

		// TODO: add reinitialization code here
		// (SDI documents will reuse this document)

		return TRUE;
	}

	// CGCSPDoc serialization

	void CGCSPDoc::Serialize(CArchive& ar)
	{
		if (ar.IsStoring())
		{
			// TODO: add storing code here
		}
		else
		{
			// TODO: add loading code here
		}
	}

#ifdef SHARED_HANDLERS

	// Support for thumbnails
	void CGCSPDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
	{
		// Modify this code to draw the document's data
		dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

		CString strText = _T("TODO: implement thumbnail drawing here");
		LOGFONT lf;

		CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
		pDefaultGUIFont->GetLogFont(&lf);
		lf.lfHeight = 36;

		CFont fontDraw;
		fontDraw.CreateFontIndirect(&lf);

		CFont* pOldFont = dc.SelectObject(&fontDraw);
		dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
		dc.SelectObject(pOldFont);
	}

	// Support for Search Handlers
	void CGCSPDoc::InitializeSearchContent()
	{
		CString strSearchContent;
		// Set search contents from document's data. 
		// The content parts should be separated by ";"

		// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
		SetSearchContent(strSearchContent);
	}

	void CGCSPDoc::SetSearchContent(const CString& value)
	{
		if (value.IsEmpty())
		{
			RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
		}
		else
		{
			CMFCFilterChunkValueImpl *pChunk = NULL;
			ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
			if (pChunk != NULL)
			{
				pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
				SetChunkValue(pChunk);
			}
		}
	}

#endif // SHARED_HANDLERS

	// CGCSPDoc diagnostics

#ifdef _DEBUG
	void CGCSPDoc::AssertValid() const
	{
		CDocument::AssertValid();
	}

	void CGCSPDoc::Dump(CDumpContext& dc) const
	{
		CDocument::Dump(dc);
	}
#endif //_DEBUG

	// CGCSPDoc commands

	void CGCSPDoc::OnButtonImageOpen()
	{
		// TODO: Add your command handler code here；

		CFileDialog dlg( TRUE, NULL, NULL,
			OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
			_T("bmp (*.bmp)|*.bmp|"));
		if(dlg.DoModal() != IDOK)
			return;

		CString filename=dlg.GetPathName();
		CFile file;
		CFileException fe;

		// Set work space
		char* pFileName = NULL;
		CONVERT_CHAR(filename, pFileName);

		string path(pFileName);
		ConfigManager::Instance()->set_work_path(path);
		ConfigManager::Instance()->set_temp_path(path);

		if( !file.Open(filename, CFile::modeRead | CFile::shareDenyWrite, &fe))
		{
			ReportSaveLoadException(filename, &fe, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
			return;
		}
		if (m_dibImage)
		{
			delete m_dibImage;
			m_dibImage = NULL;
		}

		if (m_dibImageWithStrokes)
		{
			delete m_dibImageWithStrokes;
			m_dibImageWithStrokes = NULL;
		}

		if (m_dibSegedImage)
		{
			delete m_dibSegedImage;
			m_dibSegedImage = NULL;
		}

		if (m_dibSegedImage_back)
		{
			delete m_dibSegedImage_back;
			m_dibSegedImage_back = NULL;
		}

		if (m_strokeList.GetSize()>0)
		{
			m_strokeList.RemoveAll();
		}

		CGCSPApp *app = (CGCSPApp *)AfxGetApp();
		app->count1=0;
	    app->count2=0;

		m_dibImage = new CDib;
		if(!m_dibImage->Read(&file)){	 		
			return;
		}
		file.Close();
		UpdateAllViews(NULL);
	}


	void CGCSPDoc::OnButtonImageOpen2()
	{
		// TODO: Add your command handler code here；

		CFileDialog dlg(TRUE, NULL, NULL,
			OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			_T("bmp (*.bmp)|*.bmp|"));
		if (dlg.DoModal() != IDOK)
			return;

		CString filename = dlg.GetPathName();
		CFile file;
		CFileException fe;

		// Set work space
		char* pFileName = NULL;
		CONVERT_CHAR(filename, pFileName);

		string path(pFileName);
		ConfigManager::Instance()->set_work_path(path);
		ConfigManager::Instance()->set_temp_path(path);

		if (!file.Open(filename, CFile::modeRead | CFile::shareDenyWrite, &fe))
		{
			ReportSaveLoadException(filename, &fe, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
			return;
		}

		if (m_dibImage2 != NULL)
		{
			delete m_dibImage2;
			m_dibImage2 = NULL;
		}
		if (m_dibSegedImage_back)
		{
			delete m_dibSegedImage_back;
			m_dibSegedImage_back = NULL;
		}

		CGCSPApp *app = (CGCSPApp *)AfxGetApp();
		app->count1 = 0;
		app->count2 = 0;

		m_dibImage2 = new CDib;
		if (!m_dibImage2->Read(&file)){
			return;
		}

		file.Close();
		UpdateAllViews(NULL);
	}

	void CGCSPDoc::OnButtonImageDepth()
	{
		// TODO: Add your command handler code here；

		CFileDialog dlg(TRUE, NULL, NULL,
			OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			_T("bmp (*.bmp)|*.bmp|"));
		if (dlg.DoModal() != IDOK)
			return;

		CString filename = dlg.GetPathName();
		CFile file;
		CFileException fe;
		if (!file.Open(filename, CFile::modeRead | CFile::shareDenyWrite, &fe))
		{
			ReportSaveLoadException(filename, &fe, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
			return;
		}

		if (m_depthImage != NULL)
		{
			delete m_depthImage;
			m_depthImage = NULL;
		}


		CGCSPApp *app = (CGCSPApp *)AfxGetApp();
		app->count1 = 0;
		app->count2 = 0;

		m_depthImage = new CDib;
		if (!m_depthImage->Read(&file)){
			return;
		}

		file.Close();
		UpdateAllViews(NULL);
	}

void CGCSPDoc::CollectCurrentLebelledPixelsNew(CString foreOrBackGround,CStroke *stroke)
	{
		//原先是收集到m_pixLazySnappingBgInputColors和 m_pixLazySnappingFgInputColors并且修改m_dibImageWithStrokes。
		//现在不收集了，基于像素和基于watershed过分割的收集结果不一样，在lazysnapping中再收集

		//启动像素收集，把勾勒到的像素在m_dibImageWithStrokes更改颜色
		//原来只是简单保存stoke内存储的点，结果发现虽然程序显示的画笔很浓并连续，但保存下来的像素很少，是mousemove的关键点，非连续
		//所以此处除了保存stroke上的关键点之外，还要按照其宽度扩展一些，同时加入链接关键点的那些。
		//后续加入的生成方向场的那个只需要记录单点就可以。

		vector<C2DPoint> localCuePt;
		for(int i=0; i<stroke->m_pointArray.GetSize()-1; i++)
		{
			//被坐标变换搞懵了，cOrionImg得加上才行，虽然不清楚为什么了。但结果验证就是这样，
			float pixX0 = (float)((stroke->m_pointArray.GetAt(i).x + stroke->clientOrionImg.x));
			float pixY0 = (float)((stroke->m_pointArray.GetAt(i).y + stroke->clientOrionImg.y)); 

	/*		if(foreOrBackGround == "foreground") 
			{
				if(i==(stroke->m_pointArray.GetSize()-2)/2)
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
				if(i==stroke->m_pointArray.GetSize()-2)
				{
					CGCSPApp *app = (CGCSPApp *)AfxGetApp();
					app->maxX[app->count2]=pixX0;
					app->maxY[app->count2]=pixY0;
					app->count2++;
				}
			}*/

			//用于生成方向场的，只记录关键点
			/*if(foreOrBackGround == "microadjustment")
			{
				//用于传给别的类计算
				LONG posStore = pDoc->m_dibImage->GetPixelOffset((int)pixX0,(int)pixY0);
				pDoc->m_dibImageWithStrokes->SetPixel(posStore,RGB(255,0,255));

			}*/
			//用于提取前背景颜色模型的，多取一些点
			float pixX1 = (float)((stroke->m_pointArray.GetAt(i+1).x + stroke->clientOrionImg.x));
			float pixY1 = (float)((stroke->m_pointArray.GetAt(i+1).y + stroke->clientOrionImg.y));

			C2DPoint dir;
			dir.x = pixX1 - pixX0;
			dir.y = pixY1 - pixY0;

			int radius = (int)(stroke->GetStrokeWidth()/2);//忽略奇数还是偶数，如线宽为3或2其半径皆为1

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
						LONG posStore = m_dibImage->GetPixelOffset(m,n);

						//RGBQUAD pixColor = pDoc->m_dibImage->GetPixel(posStore);

						if(foreOrBackGround == "foreground") 
						{

							m_dibImageWithStrokes->SetPixel(posStore,RGB(255,0,0));
						}
						else if(foreOrBackGround == "background")
						{
							m_dibImageWithStrokes_back->SetPixel(posStore,RGB(0,0,255));
						}
					}
				}
			}
		}
	}
void CGCSPDoc::FirstHumanStroke(CString &or_path,CString &imagename)
{
	CString stroke_path =  or_path + "\\" + imagename + ".txt";
	ifstream st_file(stroke_path);
	m_strokeList.RemoveAll();
	float imageScale;
	int a,b;
	UINT PenWidth;
	int R,G,B;
	int x,y;
	CStroke *m_StrokeCur;
	while(!st_file.eof())  //画笔
	{
		m_StrokeCur = new CStroke;
		st_file>>imageScale;
		m_StrokeCur->imageScaleWhenMarked=imageScale;
		st_file>>a>>b;
		m_StrokeCur->clientOrionImg.x=a;
		m_StrokeCur->clientOrionImg.y=b;
		st_file>>PenWidth;
		m_StrokeCur->m_nPenWidth=PenWidth;
		st_file>>R>>G>>B;
		m_StrokeCur->m_cPenColor=RGB(R,G,B);
		st_file>>x>>y;
		while(!(x==66666&&y==66666))
		{
			m_StrokeCur->m_pointArray.Add(CPoint(x,y));
			st_file>>x>>y;
		}
		if(R==255&&G==0&&B==0)
			CollectCurrentLebelledPixelsNew(_T("foreground"),m_StrokeCur);
		else
			CollectCurrentLebelledPixelsNew(_T("background"),m_StrokeCur);
			//	delete m_StrokeCur;
			//	m_StrokeCur = NULL;
	}
	SegmentationByCues(NULL);
	st_file.close();
}

void CGCSPDoc::Readimage(CFile &or_file,CFile &gr_file,CFile &er_file)
{
	m_grImage = new CDib();
	if(!m_grImage->Read(&gr_file))		
		return;
	m_dibImage = new CDib();
	if(!m_dibImage->Read(&or_file))
		return;
	m_erImage = new CDib();
	if(!m_erImage->Read(&er_file))
		return;
	m_dibSegedImage = new CDib();
	m_dibSegedImage->CopyImageInMemory(m_dibImage);
	m_dibImageWithStrokes = new CDib();
	m_dibImageWithStrokes->CopyImageInMemory(m_dibImage);

	m_dibImageWithStrokes_back = new CDib();
	m_dibImageWithStrokes_back->CopyImageInMemory(m_dibImage);

	// Iterate over all rows
	for (int i = 0; i < m_dibImageWithStrokes->m_lpBMIH->biHeight; i++)
	{
		for (int k = 0; k < m_dibImageWithStrokes->m_lpBMIH->biWidth; k++)
		{
			LONG posOffset = m_dibImageWithStrokes->GetPixelOffset(k, i);//傻帽，原来是这里i,k反了
			m_dibImageWithStrokes->SetPixel(posOffset, RGB(0, 0, 0));
			m_dibImageWithStrokes_back->SetPixel(posOffset, RGB(0, 0, 0));
		}
	}

}
void CGCSPDoc::GetDifferentImage(int **gr_Binary,int **or_Binary,int **dr_Binary)
{
	//生成真值的二值数组,在GroundTruth图像当中边缘处的像素似是而非，目前处理为背景
	for(int i = 0;i < m_grImage->m_lpBMIH->biHeight; i++)
	{
		for(int k = 0; k < m_grImage->m_lpBMIH->biWidth; k++)
		{
			long gr_off = m_grImage->GetPixelOffset(k,i);
			if(m_grImage->GetPixel(gr_off).rgbBlue >0 && m_grImage->GetPixel(gr_off).rgbGreen >0 && m_grImage->GetPixel(gr_off).rgbRed >0)
				//m_grImage->GetPixel(gr_off).rgbBlue ==255 && m_grImage->GetPixel(gr_off).rgbGreen ==255 && m_grImage->GetPixel(gr_off).rgbRed ==255)
			{
				gr_Binary[i][k] = 1;
			}
			else
			{
				gr_Binary[i][k] = 0;
			}
		}
	}
	//生成用户划线后的分割图的二值数组
	for(int i = 0;i < m_dibImage->m_lpBMIH->biHeight; i++)
	{
		for(int k = 0; k < m_dibImage->m_lpBMIH->biWidth; k++)
		{
			if(k==99 && i==113)
			{
				int stp=1;
			}
			long pixel_off = m_dibImage->GetPixelOffset(k,i);
			if(m_dibImage->GetPixel(pixel_off).rgbBlue == m_dibSegedImage->GetPixel(pixel_off).rgbBlue &&
				m_dibImage->GetPixel(pixel_off).rgbGreen == m_dibSegedImage->GetPixel(pixel_off).rgbGreen && 
				m_dibImage->GetPixel(pixel_off).rgbRed == m_dibSegedImage->GetPixel(pixel_off).rgbRed)
				or_Binary[i][k] = 1;
			else
				or_Binary[i][k] = 0;
		}
	}
	//两图异或得差异图
	for(int i = 0;i < m_dibImage->m_lpBMIH->biHeight; i++)
	{
		for(int k = 0; k < m_dibImage->m_lpBMIH->biWidth; k++)
		{
			if(k==99 && i==113)
			{
				int stp=1;
			}
			long gr_off = m_dibSegedImage->GetPixelOffset(k,i);
			dr_Binary[i][k] = gr_Binary[i][k] ^ or_Binary[i][k];
		}
	}
}
void CGCSPDoc::OutputImage(CDib *test,CString &filename,int **dr_Binary)
{
	test->CopyImageInMemory(m_dibSegedImage);
	for(int i = 0;i < m_dibImage->m_lpBMIH->biHeight; i++)
	{
		for(int k = 0; k < m_dibImage->m_lpBMIH->biWidth; k++)
		{
			long gr_off = m_dibSegedImage->GetPixelOffset(k,i);
			if(dr_Binary[i][k] == 1)
				test->SetPixel(gr_off,RGB(255,255,255));
			else
				test->SetPixel(gr_off,RGB(0,0,0));
		}
	}
	CFile mfile;
	if( !mfile.Open(filename, CFile::modeCreate | CFile::modeWrite))
	{
		return;
	}
	test->Write(&mfile);
	mfile.Close();
}
void CGCSPDoc::GetConnectionImage(int **dr_Binary,int **Label,int &label_count)
{
	for(int i = 0;i < m_dibImage->m_lpBMIH->biHeight; i++)  //通过查询左、左上、上、右上邻域的值和Label来生成自身Label
	{
		for(int k = 0; k < m_dibImage->m_lpBMIH->biWidth; k++)
		{
			//左上角点单独处理
			if( i == 0 && k == 0)
			{
				if(dr_Binary[i][k] == 1)
					Label[i][k] = ++label_count;
				else
					Label[i][k] = 0;
			}
			//X轴单独处理
			else if(i == 0 && k != 0)
			{
				if(dr_Binary[i][k] == 1 && dr_Binary[i][k-1] == 1)
				{
					Label[i][k] = Label[i][k-1];
				}
				else if(dr_Binary[i][k] == 1 && dr_Binary[i][k-1] == 0)
					Label[i][k] = ++label_count;
				else
					Label[i][k] = 0;
			}
			//最右边的边单独处理
			else if(i !=0 && k == m_dibImage->m_lpBMIH->biWidth-1)
			{
				if(dr_Binary[i][k] == 1 &&
					(dr_Binary[i-1][k-1] == 1 || 
					dr_Binary[i-1][k] == 1 ||
					dr_Binary[i][k-1] == 1))
					Label[i][k] = (Label[i-1][k-1] | Label[i-1][k] | Label[i][k-1]);
				else if(dr_Binary[i][k] == 1 &&
					(dr_Binary[i-1][k-1] == 0 || 
					dr_Binary[i-1][k] == 0 ||
					dr_Binary[i][k-1] == 0))
				Label[i][k] = ++label_count;
				else
					Label[i][k] = 0;
			}
			//Y轴单独处理
			else if(i !=0 && k == 0)
			{
				if(dr_Binary[i][k] == 1 &&
				( dr_Binary[i-1][k] == 1 ||
				dr_Binary[i-1][k+1] == 1))
					Label[i][k] = (Label[i-1][k] | Label[i-1][k+1]);
				else if(dr_Binary[i][k] == 1 &&
					( dr_Binary[i-1][k] == 0 ||
					dr_Binary[i-1][k+1] == 0))
					Label[i][k] = ++label_count;
				else
					Label[i][k] = 0;
			}


			//其他情况
			else
			{
				//处理左邻域和右上邻域Label不同的情况
				if(dr_Binary[i][k] == 1 && dr_Binary[i][k-1] ==1 && dr_Binary[i-1][k+1] ==1 && Label[i][k-1] != Label[i-1][k+1] )
				{
					int temp;
					//将label较小的保存下来
					if(Label[i-1][k+1] < Label[i][k-1])
					{
						Label[i][k] = Label[i-1][k+1];
						temp= Label[i][k-1];
						for(int a = 0;a<i;a++)
						{
							for(int b=0;b<=m_dibImage->m_lpBMIH->biWidth-1;b++)
							{	
								if(Label[a][b] == temp)
									Label[a][b] = Label[i-1][k+1];
							}
						}
						for(int b=0;b<=k;b++)
						{	
							if(Label[i][b] == temp)
								Label[i][b] = Label[i-1][k+1];
						}

					}
					else
					{
						Label[i][k] = Label[i][k-1];
						temp = Label[i-1][k+1];
						for(int a = 0;a<i;a++)
						{
							for(int b=0;b<=m_dibImage->m_lpBMIH->biWidth-1;b++)
							{	
								if(Label[a][b] == temp)
									Label[a][b] = Label[i][k-1];
							}
						}
						for(int b=0;b<=k;b++)
						{	
							if(Label[i][b] == temp)
								Label[i][b] = Label[i][k-1];
						}
					}
					for(int a = temp+1;a<=label_count;a++) //把消失位置label之后的label都向前移动
					{
						for(int b=0;b<i;b++)
						{
							for(int c=0;c<=m_dibImage->m_lpBMIH->biWidth-1;c++)
							{
								if(Label[b][c] == a)
									Label[b][c] = a-1;
							}
						}
						for(int b=0;b<=k;b++)
						{	
							if(Label[i][b] == a)
								Label[i][b] = a-1;
						}
					}
					label_count--; //label总数减一
				}
				//处理左上邻域和右上邻域Label相同的情况
				else if(dr_Binary[i][k] == 1 && dr_Binary[i-1][k-1] ==1 && dr_Binary[i-1][k+1] ==1 && Label[i-1][k-1] != Label[i-1][k+1] )
				{
					int temp;
					//将label较小的保存下来
					if(Label[i-1][k+1] < Label[i-1][k-1])
					{
						Label[i][k] = Label[i-1][k+1];
						temp= Label[i-1][k-1];
						for(int a = 0;a<i;a++)
						{
							for(int b=0;b<=m_dibImage->m_lpBMIH->biWidth-1;b++)
							{	
								if(Label[a][b] == temp)
									Label[a][b] = Label[i-1][k+1];
							}
						}
						for(int b=0;b<=k;b++)
						{	
							if(Label[i][b] == temp)
								Label[i][b] = Label[i-1][k+1];
						}
					}
					else
					{
						Label[i][k] = Label[i-1][k-1];
						temp = Label[i-1][k+1];
						for(int a = 0;a<i;a++)
						{
							for(int b=0;b<=m_dibImage->m_lpBMIH->biWidth-1;b++)
							{	
								if(Label[a][b] == temp)
									Label[a][b] = Label[i-1][k-1];
							}
						}
						for(int b=0;b<=k;b++)
						{	
							if(Label[i][b] == temp)
								Label[i][b] = Label[i-1][k-1];
						}
					}
					for(int a = temp+1;a<=label_count;a++) //把消失位置label之后的label都向前移动
					{
						for(int b=0;b<i;b++)
						{
							for(int c=0;c<=m_dibImage->m_lpBMIH->biWidth-1;c++)
							{
								if(Label[b][c] == a)
									Label[b][c] = a-1;
							}
						}
						for(int b=0;b<=k;b++)
						{	
							if(Label[i][b] == a)
								Label[i][b] = a-1;
						}
					}
					label_count--; //label总数减一
				}
				//邻域Label都相同的情况
				else 
				{
					//本身值得1，且4邻域至少有一个为1，判定为连通
					if(dr_Binary[i][k] == 1 &&
					(dr_Binary[i-1][k-1] == 1 || 
					dr_Binary[i-1][k] == 1 ||
					dr_Binary[i][k-1] == 1 ||
					dr_Binary[i-1][k+1] == 1))
						Label[i][k] = (Label[i-1][k-1] | Label[i-1][k] | Label[i][k-1] | Label[i-1][k+1]);
					//本身值得1，且4邻域均为0，判定为不连通，增加label的数量
					else if(dr_Binary[i][k] == 1 &&
					(dr_Binary[i-1][k-1] == 0 && 
					dr_Binary[i-1][k] == 0 &&
					dr_Binary[i][k-1] == 0 &&
					dr_Binary[i-1][k+1] == 0))
						Label[i][k] = ++label_count;
					else
						Label[i][k] = 0;
				}
			}
		}

	}
}
int CGCSPDoc::GetMaxRegion(int **Label,int &label_count,int *Label_Count,long &max,long &sum,int *max_label)
{
	for(int i = 0;i<=label_count;i++)
	{
		Label_Count[i] = 0;
		max_label[i] = 0;
	}
	for(int j = 0;j <= label_count; j++)
	{
		for(int i = 0;i < m_dibImage->m_lpBMIH->biHeight; i++)
		{
			for(int k = 0; k < m_dibImage->m_lpBMIH->biWidth; k++)
			{
				//测试Label有没有漏掉的
				if(Label[i][k]>label_count || Label[i][k]<0)
				{
					int a=i;
					int b=k;
				}
				if(Label[i][k] == j)
					Label_Count[j]++;
			}
		}
	}
	int a;
	for(int i = 0;i<=label_count;i++)
	{
		sum += Label_Count[i];
		if(i == 0)
			continue;
		if(i == 1)
		{
			a = 0;
			max = Label_Count[1];
			max_label[a] = i;
			continue;
		}
		if(max < Label_Count[i])
		{
			a = 0;
			max = Label_Count[i];
			memset(max_label,0,(label_count+1)*sizeof(int));
			max_label[a] = i;
			continue;
		}
		if(max == Label_Count[i])
		{
			max_label[++a]=i;
			continue;
		}
	}
	return a;
}
long CGCSPDoc::BeginErosion(int **gr_Binary,int *y_max,int *x_max,long &max,int **Label,int &ml,CDib *test)
{
	long sum_ero=0;
	long s;
	//在最大连通区域中分别寻找前景占有的像素和背景占有的像素 取最大的区域
	long fore_count=0; //前景占的像素数
	for(int i = 0;i < m_dibImage->m_lpBMIH->biHeight; i++)
	{
		for(int k = 0; k < m_dibImage->m_lpBMIH->biWidth; k++)
		{
			if(Label[i][k] == ml)
			{
				if(gr_Binary[i][k] == 1)
					fore_count++;
			}
		}
	}
	if(fore_count != 0 && fore_count != max)   //如果存在最大连通区域中，既有前景又有背景像素的情况
	{
		if(fore_count >= (max-fore_count))   //如果前景比较多，则把背景像素的标签+100
		{
			for(int i = 0;i < m_dibImage->m_lpBMIH->biHeight; i++)
			{
				for(int k = 0; k < m_dibImage->m_lpBMIH->biWidth; k++)
				{
					if(Label[i][k] == ml)
					{
						if(gr_Binary[i][k] == 0)
							Label[i][k] = ml+100;
					}
				}
			}
			max = fore_count;
		}
		else     //否则把前景像素的标签+100
		{
			for(int i = 0;i < m_dibImage->m_lpBMIH->biHeight; i++)
			{
				for(int k = 0; k < m_dibImage->m_lpBMIH->biWidth; k++)
				{
					if(Label[i][k] == ml)
					{
						if(gr_Binary[i][k] == 1)
							Label[i][k] = ml+100;
					}
				}
			}
			max = max - fore_count;
		}
	}
	//开始腐蚀，找到最后一次腐蚀前得状态
	while(max != sum_ero)
	{
		s = 0;
		for(int i = 0;i < m_dibImage->m_lpBMIH->biHeight; i++)
		{
			for(int k = 0; k < m_dibImage->m_lpBMIH->biWidth; k++)
			{
				if(Label[i][k] == ml)
				{
					if(i == 0 && k == 0)
					{
						//if(Label[i+1][k] !=ml || Label[i+1][k+1] != ml || Label[i][k+1] != ml)
						//{
								y_max[s] = i;
								x_max[s] = k;
								s++;
						//}
					}
					else if(i == 0 && k== m_dibImage->m_lpBMIH->biWidth-1)
					{
						//if(Label[i][k-1] !=ml || Label[i+1][k-1] !=ml || Label[i+1][k]!=ml)
						//{
							y_max[s] = i;
							x_max[s] = k;
							s++;
						//}
					}
					else if(i == m_dibImage->m_lpBMIH->biHeight-1 && k== m_dibImage->m_lpBMIH->biWidth-1)
					{
						//if(Label[i][k-1] !=ml || Label[i-1][k-1]!=ml || Label[i-1][k]!=ml )
						//{
							y_max[s] = i;
							x_max[s] = k;
							s++;
						//}
					}
					else if(i == m_dibImage->m_lpBMIH->biHeight-1 && k== 0)
					{
						//if(Label[i-1][k]!=ml || Label[i-1][k+1]!=ml ||Label[i][k+1]!=ml  )
						//{
							y_max[s] = i;
							x_max[s] = k;
							s++;
						//}
					}
					else if(i == 0 && k!= m_dibImage->m_lpBMIH->biWidth-1)
					{
						//if(Label[i][k-1] !=ml || Label[i+1][k-1] !=ml || Label[i+1][k]!=ml || Label[i+1][k+1]!=ml || Label[i][k+1]!=ml)
						//{
							y_max[s] = i;
							x_max[s] = k;
							s++;
						//}
					}
					else if(i != 0 && k== m_dibImage->m_lpBMIH->biWidth-1)
					{
						//if(Label[i][k-1] !=ml || Label[i+1][k-1] !=ml || Label[i+1][k]!=ml || Label[i-1][k-1]!=ml || Label[i-1][k]!=ml )
						//{
							y_max[s] = i;
							x_max[s] = k;
							s++;
						//}
					}

					else if(i == m_dibImage->m_lpBMIH->biHeight-1 && k!= 0)
					{
						//if(Label[i][k-1] !=ml || Label[i-1][k-1]!=ml || Label[i-1][k]!=ml || Label[i-1][k+1]!=ml ||Label[i][k+1]!=ml  )
						//{
							y_max[s] = i;
							x_max[s] = k;
							s++;
						//}
					}
					else if(i !=0 && k== 0)
					{
						//if(Label[i-1][k]!=ml || Label[i-1][k+1]!=ml ||Label[i][k+1]!=ml || Label[i+1][k+1]!=ml ||Label[i+1][k]!=ml )
						//{
							y_max[s] = i;
							x_max[s] = k;
							s++;
						//}
					}
					else
					{
						if(Label[i-1][k]!=ml || Label[i-1][k+1]!=ml ||Label[i][k+1]!=ml || Label[i+1][k+1]!=ml ||Label[i+1][k]!=ml ||
							Label[i+1][k-1]!=ml || Label[i][k-1]!=ml || Label[i-1][k-1]!=ml)
						{
							y_max[s] = i;
							x_max[s] = k;
							s++;
						}
					}
				}	
			}
		}
		sum_ero += s;
		//腐蚀位置标签设为-1，并用红色标注
		for(int n = 0;n<s;n++)
		{
			Label[y_max[n]][x_max[n]] = -1;
			long gr_off = m_dibSegedImage->GetPixelOffset(x_max[n],y_max[n]);
			test->SetPixel(gr_off,RGB(255,0,0));
		}
		/*CFile mfile1;
		CString filename1 = _T("erosion.bmp");
		if( !mfile1.Open(filename1, CFile::modeCreate | CFile::modeWrite))
		{
			return;
		}
		test->Write(&mfile1);
		mfile1.Close();*/
	}
	//还原最后一次腐蚀前得状态
	for(int n = 0;n<s;n++)
	{
		Label[y_max[n]][x_max[n]] = ml;
		long gr_off = m_dibSegedImage->GetPixelOffset(x_max[n],y_max[n]);
		test->SetPixel(gr_off,RGB(255,255,255));
	}
	CFile mfile1;
	CString filename1 = _T("erosion.bmp");
	if( !mfile1.Open(filename1, CFile::modeCreate | CFile::modeWrite))
	{
		return 0;
	}
	test->Write(&mfile1);
	mfile1.Close();
	return s;
}

void CGCSPDoc::ProcessLeftErosion(int *y_max_left,int *x_max_left,long &label_minus,int **Label,long &max,long &s,int &ml)
{
	long save_now =0;
	long save_obv =0;
	int  maxIndexX=m_dibImage->m_lpBMIH->biWidth;
	int  maxIndexY=m_dibImage->m_lpBMIH->biHeight;
	do
	{
		Label[y_max_left[0]][x_max_left[0]] = --label_minus;
		save_obv = save_now;
		save_now++;
		int *y_max_temp = new int[max];
		int *x_max_temp = new int[max];
		int temp;
		int left = s-save_now;
		int save_temp = 0;
		do
		{
			temp = 0;
			save_temp = save_now;
			for(int n = 1;n<=left;n++)
			{
				if(Label[y_max_left[n]][x_max_left[n]] == ml && 
					(Label[y_max_left[n]][max(x_max_left[n]-1,0)]==label_minus 
					|| Label[max(y_max_left[n]-1,0)][x_max_left[n]]==label_minus 
					|| Label[max(y_max_left[n]-1,0)][max(x_max_left[n]-1,0)]==label_minus
					|| Label[max(y_max_left[n]-1,0)][min(x_max_left[n]+1,maxIndexX-1)]==label_minus
					|| Label[y_max_left[n]][min(x_max_left[n]+1,maxIndexX-1)]==label_minus
					|| Label[min(y_max_left[n]+1,maxIndexY-1)][min(x_max_left[n]+1,maxIndexX-1)]==label_minus
					|| Label[min(y_max_left[n]+1,maxIndexY-1)][x_max_left[n]]==label_minus
					|| Label[min(y_max_left[n]+1,maxIndexY-1)][max(x_max_left[n]-1,0)]==label_minus))
				{
					Label[y_max_left[n]][x_max_left[n]] = label_minus;
					save_now++;
				}
				else if(Label[y_max_left[n]][x_max_left[n]] == label_minus)
					continue;
				else
				{
					y_max_temp[temp] = y_max_left[n];
					x_max_temp[temp] = x_max_left[n];
					temp++;
				}
			}
		}while(save_temp != save_now);
		if(save_obv == save_now)
		{		
			delete []y_max_temp;
			delete []x_max_temp;
			break;
		}
		else
		{
			memcpy(y_max_left,y_max_temp,sizeof(int)*max);
			memcpy(x_max_left,x_max_temp,sizeof(int)*max);
			delete []y_max_temp;
			delete []x_max_temp;
		}
	}while(save_now < s);
}
void CGCSPDoc::FindDesPoint(int &x_des,int &y_des,long &s,int *pen_y,int *pen_x,long &label_minus,long &max_label_minus,int **Label,int *y_max,int *x_max)
{
	long max_temp=0;
	for(long i=label_minus;i<-1;i++)
	{
		long m=0;
		for(long j=0;j<s;j++)
		{
			if(Label[y_max[j]][x_max[j]] == i)
				m++;
		}
		if(m>max_temp)
		{
			max_temp=m;
			max_label_minus = i;
		}
	}
	int n=0;
	for(long j=0;j<s;j++)
	{
		if(Label[y_max[j]][x_max[j]] == max_label_minus)
		{
			pen_y[n]=y_max[j];
			pen_x[n]=x_max[j];
			n++;
		}
	}
	//寻找中心点
	int x_avg=0;
	int y_avg=0;
	for(long i=0;i<n;i++)
	{
		x_avg += pen_x[i];
		y_avg += pen_y[i];
	}
	x_avg = x_avg/n;
	y_avg = y_avg/n;
	int min_dis;
	for(long i=0;i<n;i++)
	{
		int min_temp=30000;
		if(i ==0)
		{
			min_dis = pow((double)(x_avg-pen_x[i]),2)+pow((double)(y_avg-pen_y[i]),2);
			x_des = pen_x[i];
			y_des = pen_y[i];
		}
		else
			min_temp= pow((double)(x_avg-pen_x[i]),2)+pow((double)(y_avg-pen_y[i]),2);
		if(min_temp < min_dis)
		{
			min_dis = min_temp;
			x_des = pen_x[i];
			y_des = pen_y[i];
		}
	}
}
void CGCSPDoc::DrawRedOrBlue(int **gr_Binary,int *pen_y,int *pen_x,int &x_des,int &y_des)
{
	COLORREF rgbColor;
	RGBQUAD binaryColor;
	int grvalue=gr_Binary[y_des][x_des];

	if(grvalue == 1)
	{
		rgbColor=RGB(255,0,0);
		binaryColor.rgbRed=255;
		binaryColor.rgbGreen=255;
		binaryColor.rgbBlue=255;
	}
	else
	{
		rgbColor=RGB(0,0,255);
		binaryColor.rgbRed=0;
		binaryColor.rgbGreen=0;
		binaryColor.rgbBlue=0;
	}
	long gr_off = m_dibImageWithStrokes->GetPixelOffset(x_des,y_des);
	m_dibImageWithStrokes->SetPixel(gr_off,rgbColor);

	if(x_des==99 && y_des==113)//for test
	{
		int stp=1;
	}
	gr_off = m_grImage->GetPixelOffset(x_des,y_des);//for test
	RGBQUAD rgbColorGR =m_grImage->GetPixel(gr_off);

	if(x_des>0)
	{
		gr_off = m_dibImageWithStrokes->GetPixelOffset(x_des-1,y_des);
		if(gr_Binary[y_des][x_des-1]==grvalue)
		{
			m_dibImageWithStrokes->SetPixel(gr_off,rgbColor);
		}
	}
	if(x_des>0&&y_des>0)
	{
		gr_off = m_dibImageWithStrokes->GetPixelOffset(x_des-1,y_des-1);
		if(gr_Binary[y_des-1][x_des-1]==grvalue)
		{
			m_dibImageWithStrokes->SetPixel(gr_off,rgbColor);
		}
	}
	if(x_des>0&&y_des<m_dibImageWithStrokes->m_lpBMIH->biHeight-1)
	{
		gr_off = m_dibImageWithStrokes->GetPixelOffset(x_des-1,y_des+1);
		if(gr_Binary[y_des+1][x_des-1]==grvalue)
		{
			m_dibImageWithStrokes->SetPixel(gr_off,rgbColor);
		}
	}
	if(y_des>0)
	{			
		gr_off = m_dibImageWithStrokes->GetPixelOffset(x_des,y_des-1);
		if(gr_Binary[y_des-1][x_des]==grvalue)
		{
			m_dibImageWithStrokes->SetPixel(gr_off,rgbColor);
		}
	}
	if(x_des<m_dibImageWithStrokes->m_lpBMIH->biWidth-1&&y_des>0)
	{
		gr_off = m_dibImageWithStrokes->GetPixelOffset(x_des+1,y_des-1);
		if(gr_Binary[y_des-1][x_des+1]==grvalue)
		{
			m_dibImageWithStrokes->SetPixel(gr_off,rgbColor);
		}
	}
	if(x_des<m_dibImageWithStrokes->m_lpBMIH->biWidth-1)
	{
		gr_off = m_dibImageWithStrokes->GetPixelOffset(x_des+1,y_des);
		if(gr_Binary[y_des][x_des+1]==grvalue)
		{
			m_dibImageWithStrokes->SetPixel(gr_off,rgbColor);
		}
	}
	if(x_des<m_dibImageWithStrokes->m_lpBMIH->biWidth-1&&y_des<m_dibImageWithStrokes->m_lpBMIH->biHeight-1)
	{
		gr_off = m_dibImageWithStrokes->GetPixelOffset(x_des+1,y_des+1);
		if(gr_Binary[y_des+1][x_des+1]==grvalue)
		{
			m_dibImageWithStrokes->SetPixel(gr_off,rgbColor);
		}
	}
	if(y_des<m_dibImageWithStrokes->m_lpBMIH->biHeight-1)
	{
		gr_off = m_dibImageWithStrokes->GetPixelOffset(x_des,y_des+1);
		if(gr_Binary[y_des+1][x_des]==grvalue)
		{
			m_dibImageWithStrokes->SetPixel(gr_off,rgbColor);
		}
	}
	SegmentationByCues(NULL);
}
double CGCSPDoc::CountRate(int **or_Binary,int **gr_Binary,double &falut_rate)
{
	falut_rate = 0.00;
	for(int i = 0;i < m_grImage->m_lpBMIH->biHeight; i++)
	{
		for(int k = 0; k < m_grImage->m_lpBMIH->biWidth; k++)
		{
			long pixel_off = m_dibImage->GetPixelOffset(k,i);
			long er_off = m_erImage->GetPixelOffset(k,i);
			if(m_dibImage->GetPixel(pixel_off).rgbBlue == m_dibSegedImage->GetPixel(pixel_off).rgbBlue &&
				m_dibImage->GetPixel(pixel_off).rgbGreen == m_dibSegedImage->GetPixel(pixel_off).rgbGreen && 
				m_dibImage->GetPixel(pixel_off).rgbRed == m_dibSegedImage->GetPixel(pixel_off).rgbRed)
				or_Binary[i][k] = 1;
			else 
				or_Binary[i][k] = 0;
			if((gr_Binary[i][k]^or_Binary[i][k]) == 1)
			{
				if(m_erImage->GetPixel(er_off).rgbBlue == 127 && 
				   m_erImage->GetPixel(er_off).rgbGreen == 127 &&
				   m_erImage->GetPixel(er_off).rgbRed == 127)
				{
					falut_rate += 0.5;//这个看情况定
				}
				else
				{
					falut_rate++;
				}
			}
		}
	}
	falut_rate = falut_rate/(double)(m_grImage->m_lpBMIH->biHeight*m_grImage->m_lpBMIH->biWidth);
	return falut_rate;
}
void CGCSPDoc::OnButtonAutoSegment()
{
	// TODO: 在此添加命令处理程序代码
	CFileDialog script( TRUE,NULL,NULL,
		OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		_T("txt)*.txt)|*.txt|"));
	if(script.DoModal() != IDOK)
		return;
	CString filenames=script.GetPathName();
	CStdioFile files;
	CFileException fes;
	if( !files.Open(filenames, CFile::modeRead | CFile::shareDenyWrite, &fes))
	{
		ReportSaveLoadException(filenames, &fes, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		return;
	}
		
	CString mbuffer,or_path,gr_path,er_path,imagename;
	files.ReadString(mbuffer);
	or_path = mbuffer;
	files.ReadString(mbuffer);
	gr_path = mbuffer;
	files.ReadString(mbuffer);
	er_path = mbuffer;
	CString resultname = or_path + "\\result.txt";
	ofstream result(resultname);
	while(files.ReadString(mbuffer))
	{
		imagename = mbuffer;
		//连接源图像地址
		CString orimage_path = or_path + "\\" + imagename + ".bmp";
		//连接真值图像地址
		CString grimage_path = gr_path + "\\" + imagename + ".bmp";
		//连接7-7腐蚀膨胀图像地址
		CString erimage_path = er_path + "\\" + imagename + "trimap.bmp";
		//给源图像真值图像赋值
		CFile or_file,gr_file,er_file;
		CFileException fe1,fe2,fe3;
		if( !or_file.Open(orimage_path, CFile::modeRead | CFile::shareDenyWrite, &fe1))
		{
			ReportSaveLoadException(orimage_path, &fe1, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
			return;
		}
		if( !gr_file.Open(grimage_path, CFile::modeRead | CFile::shareDenyWrite, &fe2))
		{
			ReportSaveLoadException(grimage_path, &fe2, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
			return;
		}
		if( !er_file.Open(erimage_path, CFile::modeRead | CFile::shareDenyWrite, &fe3))
		{
			ReportSaveLoadException(erimage_path, &fe2, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
			return;
		}
		Readimage(or_file,gr_file,er_file);
		long gr_off = m_grImage->GetPixelOffset(108,202); //for test
		RGBQUAD rgbColorTemp =m_grImage->GetPixel(gr_off);

		//读已存Stroke
		FirstHumanStroke(or_path,imagename);
		//开始自动分割
		double falut_rate =0.00;
		int pen_count =1;
		std::string str = CStringA(imagename);
		result<<str.c_str()<<".bmp"<<endl;
		do
		{
			pen_count++;
			if(m_grImage->m_lpBMIH->biHeight == m_dibImage->m_lpBMIH->biHeight && m_grImage->m_lpBMIH->biWidth == m_dibImage->m_lpBMIH->biWidth)
			{	
				int** gr_Binary=new int*[m_grImage->m_lpBMIH->biHeight];
				int** or_Binary=new int*[m_dibImage->m_lpBMIH->biHeight];
				int** dr_Binary=new int*[m_dibImage->m_lpBMIH->biHeight];
				int** Label=new int*[m_dibImage->m_lpBMIH->biHeight];
				for(int i=0;i<m_grImage->m_lpBMIH->biHeight;i++)
					gr_Binary[i]=new int [m_dibImage->m_lpBMIH->biWidth]; 
				for(int i=0;i<m_dibImage->m_lpBMIH->biHeight;i++)
					or_Binary[i]=new int [m_dibImage->m_lpBMIH->biWidth];
				for(int i=0;i<m_dibImage->m_lpBMIH->biHeight;i++)
					dr_Binary[i]=new int [m_dibImage->m_lpBMIH->biWidth]; 
				for(int i=0;i<m_dibImage->m_lpBMIH->biHeight;i++)
					Label[i]=new int [m_dibImage->m_lpBMIH->biWidth]; 
				//得到差异图
				GetDifferentImage(gr_Binary,or_Binary,dr_Binary);
	
				//测试生成的差异图像
				CDib *test = new CDib();
				CString filename = _T("Different.bmp");
				OutputImage(test,filename,dr_Binary);
				if(pen_count == 45)
					int asdad=0;
				//生成连通图像
				int label_count = 0;
				GetConnectionImage(dr_Binary,Label,label_count);

				//统计Label图中各Label的像素数以及最大像素数的Label值
				int* Label_Count=new int[label_count+1];
				long max = 0; //最大连通区域的像素个数
				long sum = 0; //总像素个数
				int *max_label = new int[label_count+1]; //最大连通区域Label值
				//memset(Label_Count,0,(label_count+1)*sizeof(int));
				//memset(max_label,0,(label_count+1)*sizeof(int));
				int MaxLabelCount;
				MaxLabelCount = GetMaxRegion(Label,label_count,Label_Count,max,sum,max_label);

				//记录最大标签的元素当前状态

				int *y_max = new int[max];
				int *x_max = new int[max];
				int ml = max_label[0];  //选定第一个被存的的最大像素数的label为目标
				long s;//最后一次腐蚀的像素个数
				if(pen_count == 45)
					int asdad=0;
				s = BeginErosion(gr_Binary,y_max,x_max,max,Label,ml,test);

				//判断区域离散还是连续，并保存下剩余最大的区域
				int *y_max_left = new int[max];
				int *x_max_left = new int[max];
				long label_minus = -1;
				memcpy(y_max_left,y_max,sizeof(int)*max);
				memcpy(x_max_left,x_max,sizeof(int)*max);
				ProcessLeftErosion(y_max_left,x_max_left,label_minus,Label,max,s,ml);//将剩余区域标记为-2,-3,-4,-5....
				
				long max_label_minus;
				int *pen_y = new int[s];
				int *pen_x = new int[s];
				int x_des,y_des;
				FindDesPoint(x_des,y_des,s,pen_y,pen_x,label_minus,max_label_minus,Label,y_max,x_max);//找出所画目标点坐标
				
				DrawRedOrBlue(gr_Binary,pen_y,pen_x,x_des,y_des);//判断所画区域是前景还是背景同时画目标点

				falut_rate = CountRate(or_Binary,gr_Binary,falut_rate); //计算错误率
				//falut_rate = falut_rate/(double)gr_objcount;
				for(int j=0;j<m_grImage->m_lpBMIH->biHeight;j++)
				{ 	
					delete []gr_Binary[j];
					delete []or_Binary[j];
					delete []dr_Binary[j];
					delete []Label[j];
				}
				delete []gr_Binary;
				delete []or_Binary;
				delete []dr_Binary;
				delete []Label;
				delete []Label_Count;
				delete []max_label;
				delete []y_max;
				delete []x_max;
				delete []y_max_left;
				delete []x_max_left;
				delete []pen_x;
				delete []pen_y;
				delete test;
				test = NULL;
			}
			result<<falut_rate<<endl;
		}while(falut_rate >= 0.01 && pen_count<=50);
		result<<pen_count<<"笔"<<endl;
		//输出分割结果和带有Stroke的图像
		CFile Seged;
		CFile SegedWithStroke;
		CFileException feSeged;
		CString Segedimage = imagename + ".bmp";
		CString SegedWithStrokeImage = imagename + "strokes.bmp";
		if( !Seged.Open(Segedimage, CFile::modeCreate | CFile::modeWrite))
		{
			ReportSaveLoadException(Segedimage, &feSeged, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
			return;
		}
		if( !SegedWithStroke.Open(SegedWithStrokeImage, CFile::modeCreate | CFile::modeWrite))
		{
			ReportSaveLoadException(SegedWithStrokeImage, &feSeged, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
			return;
		}
		m_dibSegedImage->Write(&Seged);
		m_dibImageWithStrokes->Write(&SegedWithStroke);
		Seged.Close();
		SegedWithStroke.Close();
		or_file.Close();
		gr_file.Close();
		delete m_dibImage;
		m_dibImage = NULL;
		delete m_grImage;
		m_grImage = NULL;
		delete m_erImage;
		m_erImage = NULL;
		delete m_dibImageWithStrokes;
		m_dibImageWithStrokes = NULL;
		delete m_dibSegedImage;
		m_dibSegedImage = NULL;
	}
	files.Close();
	result.close();
	
	UpdateAllViews(NULL);
}

	void CGCSPDoc::SegmentationByCues(CStroke *ls)
	{

		if(m_dibSegedImage == NULL)
		{
			m_dibSegedImage = new CDib();
			m_dibSegedImage->CopyImageInMemory(m_dibImage);
			m_dibSegedImage_back = new CDib();
			m_dibSegedImage_back->CopyImageInMemory(m_dibImage2);
		}

		uchar *foregroundUchar = new uchar[3];
		uchar *backgroundUchar = new uchar[3];
		foregroundUchar[0] = 255;
		foregroundUchar[1] = 0;
		foregroundUchar[2] = 0;
		backgroundUchar[0] = 0;
		backgroundUchar[1] = 0;
		backgroundUchar[2] = 255;

		//第二个参数是过分割后的图像指针，如果是NULL，即做基于像素级别的graph cut
		LazySnapping lsnap(m_dibImage, m_dibImage2, m_dibImageWithStrokes, m_dibSegedImage, m_dibSegedImage_back, foregroundUchar, backgroundUchar, ls);
		lsnap.run();

		delete []foregroundUchar;
		delete []backgroundUchar;

		UpdateAllViews(NULL);
	}

	void CGCSPDoc::OnButtonSegSavem_dibImageWithStrokes()
	{
		if(m_dibImageWithStrokes->m_lpImage == NULL)
		{
			::AfxMessageBox(_T("No labelled image exists!"));
			return;
		}
		CFile file;
		CFileException fe;
		CString filename = _T("ImageWithStrokes.bmp");
		if( !file.Open(filename, CFile::modeCreate | CFile::modeWrite))
		{
			ReportSaveLoadException(filename, &fe, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
			return;
		}
		m_dibImageWithStrokes->Write(&file);
		file.Close();
	}

	void CGCSPDoc::OnButtonSegOpenoversegimage()
	{
		CFileDialog dlg( TRUE, NULL, NULL,
			OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
			_T("bmp (*.bmp)|*.bmp|"));
		if(dlg.DoModal() != IDOK)
			return;

		CString filename=dlg.GetPathName();
		CFile file;
		CFileException fe;
		if( !file.Open(filename, CFile::modeRead | CFile::shareDenyWrite, &fe))
		{
			ReportSaveLoadException(filename, &fe, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
			return;
		}
		if (m_dibWatershedflagImg)
		{
			delete m_dibWatershedflagImg;
			m_dibWatershedflagImg = NULL;
		}

		m_dibWatershedflagImg = new CDib;
		if(!m_dibWatershedflagImg->Read(&file)){	 		
			return;
		}
		file.Close();
		if(m_dibImage->m_lpBMIH->biHeight!=m_dibWatershedflagImg->m_lpBMIH->biHeight||
			m_dibImage->m_lpBMIH->biWidth!=m_dibWatershedflagImg->m_lpBMIH->biWidth)
		{
			::AfxMessageBox(_T("Might be the wrong oversegmentation image, try to reload one"));
		}
	}


	void CGCSPDoc::OnButtonSaveStroke()
	{
		if (m_dibImageWithStrokes->m_lpImage == NULL)
		{
			::AfxMessageBox(_T("No labelled image exists!"));
			return;
		}
		CFile file;
		CFileException fe;
		string path = ConfigManager::Instance()->work_path() + "ImageWithStrokes.bmp";
		CString filename(path.c_str());
		if (!file.Open(filename, CFile::modeCreate | CFile::modeWrite))
		{
			ReportSaveLoadException(filename, &fe, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
			return;
		}
		m_dibImageWithStrokes->Write(&file);
		file.Close();

	}

