
#include "stdafx.h"
#include "AlertFromDB.h"

CAlertFromDB::CAlertFromDB(AFX_THREADPROC pfnThreadProc, LPVOID param)
: CMyThread(pfnThreadProc, param)
{
}

CAlertFromDB::~CAlertFromDB()
{
}

/*virtual*/ void CAlertFromDB::Go()
{		
	struct sockaddr_in addr; 
	struct sockaddr_in client_addr;
	int len = sizeof(client_addr);
	int err = 0;

	memset(&addr, NULL, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8787);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	while (!IsStop())
	{
		// Create Socket
		if ((m_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
		{
			err = GetLastError();
			LogSend("[AlertFromDB]SocketError", ID_SYSLOG_SEVERITY_DEBUG);
			closesocket(m_socket);
			Sleep(10000);
			continue;
		}

		if (bind(m_socket, (SOCKADDR*)&addr, sizeof(struct sockaddr_in)) < 0)
		{
			err = GetLastError();
			LogSend("[AlertFromDB]BindError", ID_SYSLOG_SEVERITY_DEBUG);
			closesocket(m_socket);
			Sleep(10000);
			continue;
		}

		break;
	}

	HANDLE hEvent = CreateEvent(0, TRUE, FALSE, "update_elise_udp_event");

	while (!IsStop())
	{
		char buf[256] = {0};
		int n = 0;
		if ((n=recvfrom(m_socket, buf, 256, 0, (struct sockaddr*)&client_addr, &len)) < 0)
		{
			LogSend("[AlertFromDB]RecvError", ID_SYSLOG_SEVERITY_DEBUG);
			continue;
		}
	
		if (memcmp(buf, "SyncNginx", 9) == 0)
			SetEvent(hEvent);
	}
}

void CAlertFromDB::StopThread()
{
	closesocket(m_socket);
	CMyThread::StopThread();
}
