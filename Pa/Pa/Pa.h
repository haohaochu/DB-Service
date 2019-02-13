// Pa.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CPaApp:
// See Pa.cpp for the implementation of this class
//

class CPaApp : public CWinApp
{
public:
	CPaApp();

	char	m_chTitle[100];

	char	m_chSendURL[100];
	char	m_chPostURL[100];
	char	m_chJs[50];
	UINT	m_uFreq;
	UINT	m_uUdpPort;

// Overrides
public:
	virtual BOOL InitInstance();

	void GetVersion();
	void GetIni();
	void SetIni();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CPaApp theApp;
