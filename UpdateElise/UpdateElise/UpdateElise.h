// UpdateElise.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CUpdateEliseApp:
// See UpdateElise.cpp for the implementation of this class
//

class CUpdateEliseApp : public CWinApp
{
public:
	CUpdateEliseApp();

	char	m_chTitle[100];

	char	m_chDB[50];
	char	m_chElise[50];
	char	m_chEliseApi[50];
	UINT	m_uFreq;
	char	m_chEnv[4];

// Overrides
public:
	virtual BOOL InitInstance();

	void GetVersion();
	void GetIni();
	void SetIni();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CUpdateEliseApp theApp;
