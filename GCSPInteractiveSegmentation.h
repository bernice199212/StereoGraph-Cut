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

// GCSPInteractiveSegmentation.h : main header file for the GCSPInteractiveSegmentation application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CGCSPApp:
// See GCSPInteractiveSegmentation.cpp for the implementation of this class
//

class CGCSPApp : public CWinAppEx
{
public:
	CGCSPApp();
	

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	UINT  m_nAppLook;
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

public:
	float midX[10];
	float midY[10];

	float minX[10];
	float minY[10];

	float maxX[10];
	float maxY[10];

	int count1;
	int count2;
	int time;

};

extern CGCSPApp theApp;
