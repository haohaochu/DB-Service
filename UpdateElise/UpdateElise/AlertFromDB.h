
#pragma once

#include <tool\mythread.h>

class CAlertFromDB : public CMyThread
{
public:
	CAlertFromDB(AFX_THREADPROC pfnThreadProc, LPVOID param);
	~CAlertFromDB();

protected:
	SOCKET m_socket;

public:
	virtual void Go(void);
	void StopThread(void);
};
