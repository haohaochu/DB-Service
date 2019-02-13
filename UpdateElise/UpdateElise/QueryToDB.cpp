
#include "stdafx.h"
#include "QueryToDB.h"
#include "JsonParser.h"
#include "UpdateElise.h"
#include "UpdateEliseDlg.h"
#include <curl/curl.h>
#include "MD5.h"

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
	HANDLE hUdpEvent = CreateEvent(NULL, TRUE, FALSE, "update_elise_udp_event");
	
	CUpdateEliseDlg* master = (CUpdateEliseDlg*)GetParentsHandle();

	m_packet = new CPacket();
	memset(m_packet, NULL, sizeof(CPacket));
	
	curl_global_init(CURL_GLOBAL_ALL);
	
	int n=0;
	char chLog[256] = {0};

	char chDBUrl[256] = {0};
	sprintf_s(chDBUrl, "http://%s/EliseGet", theApp.m_chDB);

	char chEliseUrl[256] = {0};
	sprintf_s(chEliseUrl, "http://%s/%s", theApp.m_chElise, theApp.m_chEliseApi);

	char chUpdateUrl[256] = {0};
	sprintf_s(chUpdateUrl, "http://%s/EliseUpdate", theApp.m_chDB);

	DWORD dw = 0;
	while (!IsStop())
	{
		//DWORD dw = WaitForSingleObject(hUdpEvent, 30000);
		DWORD dw = WaitForSingleObject(hUdpEvent, theApp.m_uFreq);
	
		InterlockedIncrement(&master->m_CheckTimes);
		memset(m_packet, NULL, sizeof(CPacket));

		if (dw == WAIT_OBJECT_0)
		{
			curl = curl_easy_init();
			curl_easy_setopt(curl, CURLOPT_URL, chDBUrl);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CQueryToDB::RecvBody); 
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)m_packet); 
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);

			ResetEvent(hUdpEvent);

			LogSend("[Query2DB]Event", ID_SYSLOG_SEVERITY_DEBUG);
		}
		else if (dw == WAIT_TIMEOUT)
		{
			curl = curl_easy_init();
			curl_easy_setopt(curl, CURLOPT_URL, chDBUrl);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CQueryToDB::RecvBody); 
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)m_packet);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);

			ResetEvent(hUdpEvent);

			LogSend("[Query2DB]Timeout", ID_SYSLOG_SEVERITY_DEBUG);
		}
		else
		{
			LogSend("[Query2DB]Error", ID_SYSLOG_SEVERITY_DEBUG);
			break;
		}

		if (m_packet->contentlen <= 0)
			continue;

		CJsonParser jsp;
		if (jsp.Parse(m_packet->content, m_packet->contentlen))
		{
			CString strcmd = CString("root.root.cmd");
			for (int i=0; i<jsp.GetCount(strcmd); i++)
			{
				InterlockedIncrement(&master->m_ProcessTimes);

				CString stridx(strcmd);
				stridx.AppendFormat(_T(".%d"), i);
			
				CString strsno(jsp[stridx+_T(".sno")]);
				CString strfn(jsp[stridx+_T(".fn")]);
				CString strurl(jsp[stridx+_T(".url")]);

				strurl.Replace("\\", "");
				strurl.Replace(_T("127.0.0.1:8081"), theApp.m_chDB);
			
				CPacket *pkt = new CPacket();
				pkt->content = new char[1];
				pkt->contentlen = 0;

				curl = curl_easy_init();
				curl_easy_setopt(curl, CURLOPT_URL, strurl.GetBuffer(0));
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CQueryToDB::RecvBody);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)pkt);
				res = curl_easy_perform(curl);

				char chlog[256] = {0};
				sprintf_s(chlog, "[Query2DB:Done]len=%d", pkt->contentlen);
				LogSend(chlog, ID_SYSLOG_SEVERITY_DEBUG);
			
				CJsonParser jsp;
				char* find = NULL;
				char* datastart = NULL;
				char* dataend = NULL;

				if (strfn.Right(4) == "pats") // 海期設定
				{
					if ((find = strstr((char*)pkt->content, "\"setting\":")) == 0)
					{
						if (pkt->content) delete pkt->content;
						if (pkt) delete pkt;

						sprintf_s(chLog, "[Upload]這個檔案沒有資料喔(%s)", strurl);
						LogSend(chLog, ID_SYSLOG_SEVERITY_DEBUG);
						
						curl_easy_cleanup(curl);
						continue;
					}

					datastart = find + 10;
				}
				else if (strfn.Right(4) == "tree") // 商品清單
				{
					if ((find = strstr((char*)pkt->content, "\"tree\":")) == 0)
					{
						if (pkt->content) delete pkt->content;
						if (pkt) delete pkt;

						sprintf_s(chLog, "[Upload]這個檔案沒有資料喔(%s)", strurl);
						LogSend(chLog, ID_SYSLOG_SEVERITY_DEBUG);
						
						curl_easy_cleanup(curl);
						continue;
					}

					datastart = find + 7;
				}
				else
				{
					sprintf_s(chLog, "[Upload]這個檔案很奇怪喔(%s)", strurl);
					LogSend(chLog, ID_SYSLOG_SEVERITY_DEBUG);
					
					curl_easy_cleanup(curl);
					continue;
				}

				dataend = (char*)pkt->content + pkt->contentlen - 2;
				int datalen = dataend - datastart;

				if (jsp.Parse(pkt->content, pkt->contentlen))
				{
					// FOR UI
					UINT idx = master->GetRecord();
					
					CString strrc = jsp[CString("root.root.rc")];
					CString strfunc = jsp[CString("root.root.func")];
					CString strtreeid = jsp[CString("root.root.tree_id")];
					CString strfn = jsp[CString("root.root.fn")];
					CString strtype = jsp[CString("root.root.type")];
					CString strver = jsp[CString("root.root.ver")];
					CString strenv = CString(theApp.m_chEnv);
					
					// FOR UI
					master->SetRecord(idx, strver, strfn, 2);

					struct curl_httppost *formpost = 0;
					struct curl_httppost *lastptr  = 0;
					curl_formadd(&formpost, &lastptr, CURLFORM_PTRNAME, "func", CURLFORM_PTRCONTENTS, strfunc.GetBuffer(0), CURLFORM_END);
					curl_formadd(&formpost, &lastptr, CURLFORM_PTRNAME, "tree_id", CURLFORM_PTRCONTENTS, strtreeid.GetBuffer(0), CURLFORM_END);
					curl_formadd(&formpost, &lastptr, CURLFORM_PTRNAME, "fn", CURLFORM_PTRCONTENTS, strfn.GetBuffer(0), CURLFORM_END);
					curl_formadd(&formpost, &lastptr, CURLFORM_PTRNAME, "type", CURLFORM_PTRCONTENTS, strtype.GetBuffer(0), CURLFORM_END);
					curl_formadd(&formpost, &lastptr, CURLFORM_PTRNAME, "ver", CURLFORM_PTRCONTENTS, strver.GetBuffer(0), CURLFORM_END);
					curl_formadd(&formpost, &lastptr, CURLFORM_PTRNAME, "env", CURLFORM_PTRCONTENTS, strenv.GetBuffer(0), CURLFORM_END);
					curl_formadd(&formpost, &lastptr, CURLFORM_PTRNAME, "fp", CURLFORM_PTRCONTENTS, "", CURLFORM_END);
					curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "file01", CURLFORM_BUFFER, strfn, CURLFORM_BUFFERPTR, datastart, CURLFORM_BUFFERLENGTH, datalen, CURLFORM_END);

					curl = curl_easy_init();
					curl_easy_setopt(curl, CURLOPT_URL, chEliseUrl);
					//curl_easy_setopt(curl, CURLOPT_URL, "http://10.1.4.181:8888/UploadTest/test.php");
					//curl_easy_setopt(curl, CURLOPT_URL, "10.1.4.132:8518/syncTree");
					curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

					char instring[32] = {0};
					char outstring[32] = {0};
					char md5code[128] = {0};
					sprintf(instring, "@Mitake-$AC78k2Y^#-%s", strver.Mid(0,8));
					MD5(instring, outstring, 32);
					sprintf(md5code, "MT: %s", outstring);
					struct curl_slist *list = NULL;
					list = curl_slist_append(list, md5code);
					curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

					CPacket *recvpkt = new CPacket();
					recvpkt->content = new char[1];
					recvpkt->contentlen = 0;
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CQueryToDB::RecvBody);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)recvpkt);
					
					res = curl_easy_perform(curl);
		
					long retcode = 0;
					curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retcode);
					if (retcode == 200 && res != CURLE_ABORTED_BY_CALLBACK)
					{
						if (recvpkt->contentlen > 0)
						{
							InterlockedIncrement(&master->m_SuccessTimes);
							
							CJsonParser jp;
							jp.Parse(recvpkt->content, recvpkt->contentlen);

							if (UpdateToDB(strsno))
							{
								master->SetRecord(idx, strver, strfn, atoi(jp[CString("root.Code")]));
								sprintf_s(chLog, "[Update]傳送成功:%s DB更新成功", jp[CString("root.Code")]);
								LogSend(chLog, ID_SYSLOG_SEVERITY_DEBUG);
							}
							else
							{
								master->SetRecord(idx, strver, strfn, atoi(jp[CString("root.Code")]));
								sprintf_s(chLog, "[Update]傳送成功:%s DB更新失敗", jp[CString("root.Code")]);
								LogSend(chLog, ID_SYSLOG_SEVERITY_DEBUG);
							}
						}
						else
						{
							master->SetRecord(idx, strver, strfn, 2);
							sprintf_s(chLog, "[Update]傳送失敗:%d", retcode);
							LogSend(chLog, ID_SYSLOG_SEVERITY_DEBUG);
						}
					//	// UPDATE ELISE
					//	InterlockedIncrement(&master->m_SuccessTimes);

					//	if (UpdateToDB(strsno))
					//	{
					//		master->SetRecord(idx, strver, strfn, 0);
					//		sprintf_s(chLog, "[Update]DB更新成功喔");
					//		LogSend(chLog, ID_SYSLOG_SEVERITY_DEBUG);
					//	}
					//	else
					//	{
					//		master->SetRecord(idx, strver, strfn, 2);
					//		sprintf_s(chLog, "[Update]DB更新失敗喔");
					//		LogSend(chLog, ID_SYSLOG_SEVERITY_DEBUG);
					//	}
					}
					else
					{
						master->SetRecord(idx, strver, strfn, 1);
						sprintf_s(chLog, "[SyncTree]商品樹更新失敗,EliseServer無回應(%s)", chEliseUrl);
						LogSend(chLog, ID_SYSLOG_SEVERITY_ERROR);
					}

					curl_formfree(formpost);
					curl_easy_cleanup(curl);

					if (recvpkt->content)
						delete recvpkt->content;

					if (recvpkt)
						delete recvpkt;
				}
				else
				{
					sprintf_s(chLog, "[Upload]JSON格式不對喔(%s)", chEliseUrl);
					LogSend(chLog, ID_SYSLOG_SEVERITY_DEBUG);
				}

				if (pkt->content)
				{
					delete pkt->content;
					pkt->content = NULL;
				}

				if (pkt)
				{
					delete pkt;
					pkt = NULL;
				}

				curl_easy_cleanup(curl);
				strurl.ReleaseBuffer();
			}
		}
		else
		{
			LogSend("[Query2DB]JsonParseError", ID_SYSLOG_SEVERITY_DEBUG);
		}
	}

	if (m_packet->content)
	{
		delete m_packet->content;
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
	char chUpdateUrl[256] = {0};
	sprintf_s(chUpdateUrl, "http://%s/EliseUpdate?sno=%s", theApp.m_chDB, sno);

	CURL* curl;
	CURLcode res;

	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, chUpdateUrl);
	res = curl_easy_perform(curl);
		
	long retcode = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retcode);

	if (retcode == 200 && res != CURLE_ABORTED_BY_CALLBACK)
		return TRUE;
	else
		return FALSE;
}

size_t CQueryToDB::RecvBody(void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t realsize = size * nmemb;

	//(*(CString*)stream).Append(CString((char*)ptr));
	//strcat((char*)stream, (char*)ptr);
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
