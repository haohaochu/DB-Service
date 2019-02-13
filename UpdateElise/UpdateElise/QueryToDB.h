
#pragma once

#include <tool\mythread.h>

typedef struct CPacket
{
	char* content;
	size_t contentlen;
} CPacket;

class CQueryToDB : public CMyThread
{
public:
	CQueryToDB(AFX_THREADPROC pfnThreadProc, LPVOID param);
	~CQueryToDB(void);

public:
	CPacket* m_packet;
	BOOL UpdateToDB(CString&);

public:
	virtual void Go(void);
	static size_t RecvBody(void *ptr, size_t size, size_t nmemb, void *stream);
};
