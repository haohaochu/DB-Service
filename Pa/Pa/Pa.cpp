// Pa.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Pa.h"
#include "PaDlg.h"

#pragma comment(lib, "Version.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPaApp

BEGIN_MESSAGE_MAP(CPaApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CPaApp construction

CPaApp::CPaApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CPaApp object

CPaApp theApp;


// CPaApp initialization

BOOL CPaApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	//SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CPaDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

void CPaApp::GetIni()
{
	char szIniPath[261]={0}, szFolder[260]={0}, szAppName[256]={0}, szDrive[3]={0};
	CString	strbuf;
	CString strExecutePath;
	// then try current working dir followed by app folder
	GetModuleFileName(NULL, szIniPath, sizeof(szIniPath)-1);
	_splitpath_s(szIniPath, szDrive, 3, szFolder, 260, szAppName, 256, NULL, 0);
	// chNowDirectory
	GetCurrentDirectory(sizeof(szIniPath)-1, szIniPath);
	strExecutePath.Format("%s\\", szIniPath);

	// SYSTEM
	_makepath_s(szIniPath, NULL, szIniPath, szAppName, ".ini");
	free((void*)m_pszProfileName);
	m_pszProfileName = _strdup(szIniPath);

	memset(m_chSendURL, NULL, 100);
	memset(m_chPostURL, NULL, 100);
	memset(m_chJs, NULL, 50);
	m_uFreq = 0;
	m_uUdpPort = 0;

	GetPrivateProfileString("SETTINGS", "DBSENDURL", "", m_chSendURL, 100, szIniPath);
	GetPrivateProfileString("SETTINGS", "DBPOSTURL", "", m_chPostURL, 100, szIniPath);
	GetPrivateProfileString("SETTINGS", "JAVASCRIPT", "", m_chJs, 50, szIniPath);
	m_uFreq = GetPrivateProfileInt("SETTINGS", "FREQUENCY", 10000, szIniPath);
	m_uUdpPort = GetPrivateProfileInt("SETTINGS", "UDPPORT", 8787, szIniPath);

	GetVersion();
}

void CPaApp::SetIni()
{
	//m_pszRegistryKey = NULL;

	WriteProfileString("SETTINGS", "DBSENDURL", m_chSendURL);
	WriteProfileString("SETTINGS", "DBPOSTURL", m_chPostURL);
	WriteProfileString("SETTINGS", "JAVASCRIPT", m_chJs);
	WriteProfileInt("SETTINGS", "FREQUENCY", m_uFreq);
	WriteProfileInt("SETTINGS", "UDPPORT", m_uUdpPort);
}

// 取得版本別
void CPaApp::GetVersion()
{
	char szFileName[128]={0};
	char chLogbuf[128]={0};
	
	// 取程式路徑
	if (GetModuleFileName(NULL, szFileName, sizeof(szFileName)) == 0)
	{
//		sprintf(chLogbuf,"[CPats2mtkApp::GetVersion()]GetModuleFileName() Error(%d)",GetLastError());
//		LogSend(chLogbuf,ID_SYSLOG_SEVERITY_ERROR);
		return;
	}

	DWORD dwInfoSize;
	if ((dwInfoSize=GetFileVersionInfoSize(szFileName,NULL)) == 0)
	{
//		sprintf(chLogbuf,"[CPats2mtkApp::GetVersion()]GetFileVersionInfoSize(%s) Error(%d)",szFileName,GetLastError());
//		LogSend(chLogbuf,ID_SYSLOG_SEVERITY_ERROR);
		return;
	}
    
	char* pVersionInfo = NULL;

	if ((pVersionInfo=new char[dwInfoSize+1]) == NULL)
	{
//		LogSend("[CPats2mtkApp::GetVersion()]New Memory Failure!",ID_SYSLOG_SEVERITY_ERROR);
		return;
	}

	ZeroMemory(pVersionInfo,dwInfoSize+1);

	if (!GetFileVersionInfo(szFileName,NULL,dwInfoSize,pVersionInfo))
	{
		delete [] pVersionInfo;
//		sprintf(chLogbuf,"[CPats2mtkApp::GetVersion()]GetFileVersionInfo(%s) Error(%d)",szFileName,GetLastError());
//		LogSend(chLogbuf,ID_SYSLOG_SEVERITY_ERROR);
		return;
	}

	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	}*lpTranslate;
	
	UINT cbTranslate;
	// Read the list of languages and code pages.

    if (!VerQueryValue(pVersionInfo,TEXT("\\"),(LPVOID*)&lpTranslate,&cbTranslate) || cbTranslate == 0)
	{
		delete [] pVersionInfo;
//		sprintf(chLogbuf,"[CPats2mtkApp::GetVersion()]VerQueryValue(%s) Error",szFileName);
//		LogSend(chLogbuf,ID_SYSLOG_SEVERITY_ERROR);
		return;
	}

	//m_strVersion.Format("Ver. %d.%d.%d.%d", lpTranslate[2].wCodePage, lpTranslate[2].wLanguage, lpTranslate[3].wCodePage, lpTranslate[3].wLanguage);

	delete [] pVersionInfo;
}
