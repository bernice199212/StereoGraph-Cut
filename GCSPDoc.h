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

// GCSPDoc.h : interface of the CGCSPDoc class
//
#include "Dib.h"
#include "Stroke.h"


#pragma once


class CGCSPDoc : public CDocument
{
protected: // create from serialization only
	CGCSPDoc();
	DECLARE_DYNCREATE(CGCSPDoc)

public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

public:
	virtual ~CGCSPDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	void FirstHumanStroke(CString &or_path,CString &imagename);
	void Readimage(CFile &or_file,CFile &gr_file,CFile &er_file);
	void GetDifferentImage(int **gr_Binary,int **or_Binary,int **dr_Binary);
	void OutputImage(CDib *test,CString &filename,int **dr_Binary);
	void GetConnectionImage(int **dr_Binary,int **Label,int &label_count);
	int GetMaxRegion(int **Label,int &label_count,int *Label_Count,long &max,long &sum,int *max_label);
	long BeginErosion(int **gr_Binary,int *y_max,int *x_max,long &max,int **Label,int &ml,CDib *test);
	void ProcessLeftErosion(int *y_max_left,int *x_max_left,long &label_minus,int **Label,long &max,long &s,int &ml);
	void FindDesPoint(int &x_des,int &y_des,long &s,int *pen_y,int *pen_x,long &label_minus,long &max_label_minus,int **Label,int *y_max,int *x_max);
	void DrawRedOrBlue(int **gr_Binary,int *pen_y,int *pen_x,int &x_des,int &y_des);
	double CountRate(int **or_Binary,int **gr_Binary,double &falut_rate);
public:
	CDib *m_dibImage;
	CDib *m_dibImage2;

	CDib *m_grImage;
	CDib *m_erImage;

	CDib *m_dibImageWithStrokes;//存放带有stroke的图像
	CDib *m_dibImageWithStrokes_back;
	CDib *m_dibWatershedflagImg;//存放过分割的图像，读入进来的,其实是watershed程序中的flag
	
	int m_iSelectedObject; 
	void SetIndObjectsName();

	void Compute3DGroundDepthbyGroundCues();
	int *ComputeSlantingAngles(int w, int h, bool &hasBlank, int &maxDirDeri);
	void FillBlankSlantingAnglesAlongY(int *dirDerivativeAlongY, int w, int h);
	void SaveGroundCues();
	void ReadinGroundCues();	
	void ReadinGroundDepth();
	void SaveGreyMap(int *m_pfDepthBuffer, int w, int h, CString filename);
	void RecordPixelsinGroundCues();
	bool IsInGroundCues(int pix);

	bool *m_bPixelsinGroundCues;
	void PropagateGroundCues();

	int m_iSamplingInterval;

	int **m_pClusters;//存放对应分割结果的显示颜色值，共有m_iClustersNum个Cluster，每一个有RGB三色分量
	int **m_pClusters2;

	int m_iClustersNum;

	int *m_iImageLabels; //存放每个像素对应的label值
	int *m_iImageLabels2;
	
	CDib *m_dibSegedImage;//仅仅用于显示标注图像，与m_iImageLabels信息重复
	CDib *m_dibSegedImage_back;

	CDib *m_depthImage;	//Store the dispairity of depth info

	//图像相对整个客户窗口的偏移,与扩充
	CPoint  imgOri;
	CPoint clientExtend;
	void CollectCurrentLebelledPixelsNew(CString foreOrBackGround,CStroke *stroke);
	void SegmentationByCues(CStroke *ls);
	afx_msg void OnButtonSegSavem_dibImageWithStrokes();
	afx_msg void OnButtonSegOpenoversegimage();

	CTypedPtrList<CObList, CStroke*> m_strokeList;
	afx_msg void OnButtonSaveStroke();
	afx_msg void OnButtonAutoSegment();
	afx_msg void OnButtonImageOpen();
	afx_msg void OnButtonImageOpen2();
	afx_msg void OnButtonImageDepth();
};
