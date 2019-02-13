
#pragma once

#include <tool\mythread.h>

typedef struct CPacket
{
	char* content;
	size_t contentlen;
} CPacket;

typedef struct CContext
{
	//char sno[16];
	//char fn[16];
	//char url[480];
	UINT uid;
	void* pmaster;
	CString strsno;
	CString strfn;
	CString strurl;
} CContext;

class CQueryToDB : public CMyThread
{
public:
	CQueryToDB(AFX_THREADPROC pfnThreadProc, LPVOID param);
	~CQueryToDB(void);

public:
	CPacket* m_packet;
	BOOL UpdateToDB(CString&);
	static DWORD WINAPI	cbProcURL(PVOID pContext);
	static DWORD WINAPI	cbProcURL2(PVOID pContext);
	CContext m_context[1000];

public:

public:
	virtual void Go(void);
	static size_t RecvBody(void *ptr, size_t size, size_t nmemb, void *stream);
};
