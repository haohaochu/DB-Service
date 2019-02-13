
#include "stdafx.h"
#include "QueryToDB.h"
#include "JsonParser.h"
#include "Pa.h"
#include "PaDlg.h"
#include <curl/curl.h>
//#include "MD5.h"
#include <iostream>
#include <string>
using namespace std;

CQueryToDB::CQueryToDB(AFX_THREADPROC pfnThreadProc, LPVOID param)
: CMyThread(pfnThreadProc, param)
, m_packet(NULL)
{

}

CQueryToDB::~CQueryToDB()
{

}

void CQueryToDB::Go(void)
{
	CURL* curl;
	CURLcode res;
	HANDLE hUdpEvent = CreateEvent(NULL, TRUE, FALSE, "update_papapa_udp_event");
	
	CPaDlg* master = (CPaDlg*)GetParentsHandle();

	m_packet = new CPacket();
	memset(m_packet, NULL, sizeof(CPacket));
	
	curl_global_init(CURL_GLOBAL_ALL);
	
	int n=0;
	char chLog[256] = {0};

	DWORD dw = 0;
	while (!IsStop())
	{
		DWORD dw = WaitForSingleObject(hUdpEvent, theApp.m_uFreq);
	
		// UI 檢查次數
		InterlockedIncrement(&master->m_CheckTimes);

		// 清除舊紀錄
		memset(m_packet, NULL, sizeof(CPacket));
		
		if (dw == WAIT_OBJECT_0)
		{
			m_packet->content = new char[1];
			m_packet->contentlen = 0;

			curl = curl_easy_init();
			curl_easy_setopt(curl, CURLOPT_URL, theApp.m_chSendURL);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CQueryToDB::RecvBody); 
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)m_packet); 
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);

			ResetEvent(hUdpEvent);
			LogSend("[Query2DB]Event", ID_SYSLOG_SEVERITY_DEBUG);
		}
		else if (dw == WAIT_TIMEOUT)
		{
			m_packet->content = new char[1];
			m_packet->contentlen = 0;

			curl = curl_easy_init();
			curl_easy_setopt(curl, CURLOPT_URL, theApp.m_chSendURL);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CQueryToDB::RecvBody); 
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)m_packet);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);

			ResetEvent(hUdpEvent);
			LogSend("[Query2DB]Timeout", ID_SYSLOG_SEVERITY_DEBUG);
		}
		else
		{
			ResetEvent(hUdpEvent);
			LogSend("[Query2DB]Error", ID_SYSLOG_SEVERITY_DEBUG);
			break;
		}

		if (m_packet->contentlen <= 0)
		{
			delete[] m_packet->content;
			m_packet->content = NULL;
			continue;
		}

		char type; 
		char* idx = m_packet->content;
		char* find = NULL;
		UINT index = 0;
		CContext context;
		while (idx < m_packet->content+m_packet->contentlen)
		{
			m_context[index].strsno = "";
			m_context[index].strfn = "";
			m_context[index].strurl = "";

			// CMDTYPE
			if (idx[0]=='0' && idx[1]=='1')
				type = 1;
			else if (idx[0]=='0' && idx[1]=='2')
				type = 2;
			else
				type = 0;
			
			idx+=2;

			// KEY
			find = strstr(idx, "]\n");
			if (idx[0]=='[' && find)
			{
				find[0] = 0;
				m_context[index].strsno = CString(&idx[1]);
				idx=find+2;
			}
			else
			{
				idx++;
				continue;
			}

			// URL
			find = strstr(idx, "\r\n\n");
			if (find)
			{
				find[0] = 0;
				m_context[index].strurl = CString(&idx[0]);
				idx=find+3;
			}
			else
			{
				idx++;
				continue;
			}

			SYSTEMTIME time;
			GetLocalTime(&time);
			m_context[index].strfn.Format("%03d%02d%04d%02d%02d%02d%02d%02d%03d", index, type, time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

			CString strt;
			strt.Format("%04d%02d%02d%02d%02d%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

			//
			switch (type)
			{
			case 0:
				// ERROR
				break;
			case 1:
				m_context[index].uid = master->GetRecord();
				m_context[index].pmaster = master;
				master->SetRecord(m_context[index].uid, 1, strt, m_context[index].strsno.Left(3), m_context[index].strurl, 2);
				QueueUserWorkItem(&CQueryToDB::cbProcURL, &m_context[index], WT_EXECUTEDEFAULT);
				break;
			case 2:
				m_context[index].uid = master->GetRecord();
				m_context[index].pmaster = master;
				master->SetRecord(m_context[index].uid, 2, strt, m_context[index].strsno.Left(3), m_context[index].strurl, 2);
				QueueUserWorkItem(&CQueryToDB::cbProcURL2, &m_context[index], WT_EXECUTEDEFAULT);
				break;
			}
			
			index = (index+1)%1000;
		}
	}

	if (m_packet->content)
	{
		delete[] m_packet->content;
		m_packet->content = NULL;
	}

	if (m_packet)
	{
		delete m_packet;
		m_packet = NULL;
	}

	EndThread();
	return;
}

BOOL CQueryToDB::UpdateToDB(CString& sno)
{
	//char chUpdateUrl[256] = {0};
	//sprintf_s(chUpdateUrl, "%sClientID=%s", theApp.m_chDB, sno);

	//CURL* curl;
	//CURLcode res;

	//curl = curl_easy_init();
	//curl_easy_setopt(curl, CURLOPT_URL, chUpdateUrl);
	//res = curl_easy_perform(curl);
	//	
	//long retcode = 0;
	//curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retcode);

	//if (retcode == 200 && res != CURLE_ABORTED_BY_CALLBACK)
	//	return TRUE;
	//else
		return FALSE;
}

size_t CQueryToDB::RecvBody(void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t realsize = size * nmemb;

	CPacket* pkt = (CPacket*)stream;
	pkt->content = (char*)realloc(pkt->content, pkt->contentlen+realsize+1);
	if (pkt->content == NULL)
	{
		return 0;
	}

	memcpy(&(pkt->content[pkt->contentlen]), ptr, realsize);
	pkt->contentlen += realsize;
	pkt->content[pkt->contentlen] = 0;

	return realsize;
}


/*static */DWORD WINAPI	CQueryToDB::cbProcURL(PVOID pContext)
{
	CURL* curl;
	CURLcode res;

	CContext* context = (CContext*)pContext;
	
	InterlockedIncrement(&((CPaDlg*)(context->pmaster))->m_Proc1Times);

	// Step2.取得網頁
	CPacket* packet = new CPacket;
	packet->content = new char[1];
	packet->contentlen = 0;

	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, context->strurl);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CQueryToDB::RecvBody);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)packet);
	res = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	
	// Step3.POST Update
	char chUpdateUrl[256] = {0};
	//sprintf_s(chUpdateUrl, "http://10.99.0.30:8080/SpHttpPush_PaWeb_Update?ClientID=%s&filein=%s", context->strsno, context->strfn);
	sprintf_s(chUpdateUrl, "%sClientID=%s&filein=%s", theApp.m_chPostURL, context->strsno, context->strfn);

	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, chUpdateUrl);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, packet->content);
	res = curl_easy_perform(curl);
	
	long retcode = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retcode);

	if (retcode == 200 && res != CURLE_ABORTED_BY_CALLBACK)
	{
		// LOG
		((CPaDlg*)(context->pmaster))->SetRecord(context->uid, 0);
		InterlockedIncrement(&((CPaDlg*)(context->pmaster))->m_PostTimes);
	}
	else
	{
		// ERROR
		((CPaDlg*)(context->pmaster))->SetRecord(context->uid, retcode);
		InterlockedIncrement(&((CPaDlg*)(context->pmaster))->m_ErrorTimes);

		char chLog[256] = {0};
		sprintf_s(chLog, "[ERR:%d][TYPE1]%s", retcode, context->strurl);
		LogSend(chLog, ID_SYSLOG_SEVERITY_ERROR);
	}

	curl_easy_cleanup(curl);
	delete[] packet->content;
	delete packet;

	return 0;
}

/*static */DWORD WINAPI	CQueryToDB::cbProcURL2(PVOID pContext)
{
	CContext* context = (CContext*)pContext;
	
	InterlockedIncrement(&((CPaDlg*)(context->pmaster))->m_Proc2Times);

	// Step2.取得網頁
	CString sposturl;
	//sposturl.Format("http://10.99.0.30:8080/SpHttpPush_PaWeb_Update?ClientID=%s&filein=%s", context->strsno, context->strfn);
	sposturl.Format("%sClientID=%s&filein=%s", theApp.m_chPostURL, context->strsno, context->strfn);

	char command[512] = {0};
	sprintf_s(command, "phantomjs.exe %s %s %s", theApp.m_chJs, context->strurl, sposturl);

	CStringA strResult;
    HANDLE hPipeRead, hPipeWrite;

	SECURITY_ATTRIBUTES saAttr = {sizeof(SECURITY_ATTRIBUTES)};
    saAttr.bInheritHandle = TRUE;   //Pipe handles are inherited by child process.
    saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0))
        return 0;

	STARTUPINFO si = {sizeof(STARTUPINFO)};
    si.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.hStdOutput  = hPipeWrite;
    si.hStdError   = hPipeWrite;
    si.wShowWindow = SW_HIDE;       // Prevents cmd window from flashing. Requires STARTF_USESHOWWINDOW in dwFlags.

    PROCESS_INFORMATION pi = {0};

	BOOL fSuccess = CreateProcess(NULL, command, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    if (!fSuccess)
    {
		int err = GetLastError();
		TRACE("%d\n", GetLastError());
        CloseHandle(hPipeWrite);
        CloseHandle(hPipeRead);
        return 0;
    }

	bool bProcessEnded = false;
    for (;!bProcessEnded;)
    {
        // Give some timeslice (50ms), so we won't waste 100% cpu.
        bProcessEnded = WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0;

        // Even if process exited - we continue reading, if there is some data available over pipe.
        for (;;)
        {
            char buf[1024];
            DWORD dwRead = 0;
            DWORD dwAvail = 0;

            if (!::PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL))
                break;

            if (!dwAvail) // no data available, return
                break;

            if (!::ReadFile(hPipeRead, buf, min(sizeof(buf) - 1, dwAvail), &dwRead, NULL) || !dwRead)
                // error, the child process might ended
                break;

            buf[dwRead] = 0;
            strResult += buf;
        }
    } // for

	if (strResult.Find("000 Success") >= 0)
	{
		// LOG
		((CPaDlg*)(context->pmaster))->SetRecord(context->uid, 0);
		InterlockedIncrement(&((CPaDlg*)(context->pmaster))->m_PostTimes);
	}
	else
	{
		// ERROR
		((CPaDlg*)(context->pmaster))->SetRecord(context->uid, strResult);
		InterlockedIncrement(&((CPaDlg*)(context->pmaster))->m_ErrorTimes);

		char chLog[256] = {0};
		sprintf_s(chLog, "[ERR:%s][TYPE2]%s", strResult, context->strurl);
		LogSend(chLog, ID_SYSLOG_SEVERITY_ERROR);
	}

    CloseHandle(hPipeWrite);
    CloseHandle(hPipeRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

	if (strResult != "")
	{
		TRACE(strResult);
	}

    return 0;
}

