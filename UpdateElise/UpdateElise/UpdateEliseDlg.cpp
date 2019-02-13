// UpdateEliseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UpdateElise.h"
#include "UpdateEliseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CUpdateEliseDlg dialog

CGrid1Record::CGrid1Record()
{
}

CGrid1Record::CGrid1Record(CUpdateEliseDlg* master, CString& name, CString& value)
{
	AddItem(new CXTPReportRecordItemText(name));
	AddItem(new CXTPReportRecordItemText(value));
}

CGrid1Record::CGrid1Record(CUpdateEliseDlg* master, CString& name, UINT value)
{
	AddItem(new CXTPReportRecordItemText(name));
	
	CString str("");
	str.Format("%d", value);
	AddItem(new CXTPReportRecordItemText(str));
}

/*virtual */CGrid1Record::~CGrid1Record(void)
{
}

/*virtual */void CGrid1Record::CreateItems(void)
{
}

/*virtual */void CGrid1Record::GetItemMetrics(XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs, XTP_REPORTRECORDITEM_METRICS* pItemMetrics)
{
	CXTPReportRecord::GetItemMetrics(pDrawArgs, pItemMetrics);
}

BOOL CGrid1Record::UpdateRecord(int idx, CString& data)
{
	if (idx < this->GetItemCount()) 
	{
		((CXTPReportRecordItemText*)GetItem(idx))->SetValue(data);
		return TRUE;
	}

	return FALSE;
}

BOOL CGrid1Record::UpdateRecord(int idx, UINT data)
{
	if (idx < this->GetItemCount()) 
	{
		CString strData("");
		strData.Format("%d", data);

		((CXTPReportRecordItemText*)GetItem(idx))->SetValue(strData);
		return TRUE;
	}

	return FALSE;
}

CGrid2Record::CGrid2Record(CUpdateEliseDlg* master)
: m_master(master)
{
	CreateItems();
}

CGrid2Record::~CGrid2Record(void)
{
}

/*virtual */void CGrid2Record::CreateItems()
{
	AddItem(new CXTPReportRecordItemText());
	AddItem(new CXTPReportRecordItemText());
	AddItem(new CXTPReportRecordItemText());
	AddItem(new CXTPReportRecordItemText());
}

/*virtual */void CGrid2Record::GetItemMetrics(XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs, XTP_REPORTRECORDITEM_METRICS* pItemMetrics)
{
	CString strColumn = pDrawArgs->pColumn->GetCaption();
	int nIndexCol = pDrawArgs->pColumn->GetItemIndex();
	int nIndexRow = pDrawArgs->pRow->GetIndex();
	int nCount = pDrawArgs->pControl->GetRows()->GetCount();

	if (m_master)
	{
		AcquireSRWLockExclusive(&m_master->m_RecordLock);
		
		UINT idx = (m_master->m_RecordIndex-nIndexRow-1)%100;

		CRecord* rec = &m_master->m_Record[idx];

		if (rec == NULL)
		{
			pItemMetrics->strText.Format("%s", "NULL");
			ReleaseSRWLockExclusive(&m_master->m_RecordLock);
			return;
		}
		
		switch (nIndexCol)
		{
		case 0: // 編號
			{
				pItemMetrics->strText.Format("%d", m_master->m_RecordCount-nIndexRow-1);
				break;
			}
		case 1: // 時間
			{
				pItemMetrics->strText.Format("%s", rec->time);
				break;
			}
		case 2: // 檔案
			{
				pItemMetrics->strText.Format("%s", rec->name);
				break;
			}
		case 3: // 狀態
			{
				switch(rec->status)
				{
				case 0:
					{
						pItemMetrics->nItemIcon = 0; // DONE
						pItemMetrics->strText = "00000";
						break;
					}
				case 1:
					{
						pItemMetrics->nItemIcon = 1; // ERR
						pItemMetrics->strText = "失敗";
						break;
					}
				case 2:
					{
						pItemMetrics->nItemIcon = 2; // DOING
						pItemMetrics->strText = "處理";
						break;
					}
				default:
					{
						pItemMetrics->nItemIcon = 1; // ERR
						pItemMetrics->strText.Format("%05d", rec->status);
						break;
					}
				}
				break;
			}
		default:
			{
				pItemMetrics->strText = "???";
				break;
			}
		}

		ReleaseSRWLockExclusive(&m_master->m_RecordLock);
	}

	return;


}

CUpdateEliseDlg::CUpdateEliseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUpdateEliseDlg::IDD, pParent)
, m_RecordIndex(0)
, m_RecordCount(0)
, m_RecordRefresh(TRUE)
, m_CheckTimes(0)
, m_ProcessTimes(0)
, m_SuccessTimes(0)
, m_Setting(FALSE)
, m_Status(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
	
	memset(m_Record, NULL, sizeof(CRecord)*100);
	InitializeSRWLock(&m_RecordLock);
}

void CUpdateEliseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CUpdateEliseDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_TOOLBARBTN01, &CUpdateEliseDlg::OnStartWork)
	ON_BN_CLICKED(IDC_TOOLBARBTN02, &CUpdateEliseDlg::OnStopWork)
	ON_BN_CLICKED(IDC_TOOLBARBTN03, &CUpdateEliseDlg::OnSetting)
	ON_NOTIFY(XTP_NM_REPORT_VALUECHANGED, IDC_GRID1, &CUpdateEliseDlg::OnChangeSettings)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CUpdateEliseDlg message handlers

BOOL CUpdateEliseDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);         // Set big icon
	SetIcon(m_hIcon, FALSE);        // Set small icon

	// TODO: Add extra initialization here
	theApp.GetIni();
	SetWindowText(theApp.m_chTitle);

	// Dlg Set
	CRect rect;
	CBitmap bmp;
	GetClientRect(&rect);

	m_Font.CreateFont(
		16,							// nHeight(Min 8)
		0,							// nWidth(min 4)
		0,							// nEscapement
		0,							// nOrientation
		FW_BOLD,					// nWeight
		FALSE,						// bItalic
		FALSE,						// bUnderline
		0,							// cStrikeOut
		ANSI_CHARSET,				// nCharSet
		OUT_DEFAULT_PRECIS,			// nOutPrecision
		CLIP_DEFAULT_PRECIS,		// nClipPrecision
		DEFAULT_QUALITY,			// nQuality
		DEFAULT_PITCH | FF_SWISS,	// nPitchAndFamily
		//"Consolas");				// lpszFacename
		//"Courier New");			// lpszFacename
		//"Arial");					// lpszFacename
		"微軟正黑體");				// lpszFacename

	// Toolbar
	m_ToolBarImageList.Create(16, 16, ILC_COLOR16 | ILC_MASK, 1, 1);
	UINT arToolBarImg[] = {IDB_BITMAP4, IDB_BITMAP5, IDB_BITMAP6};
	UINT arToolBarBtn[] = {IDC_TOOLBARBTN01, IDC_TOOLBARBTN02, IDC_TOOLBARBTN03};
	for (int i=0; i<3; i++)
	{
		bmp.LoadBitmap(arToolBarImg[i]);
		m_ToolBarImageList.Add(&bmp, RGB(255, 255, 255));
		bmp.DeleteObject();
	}

	m_ToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(0, 0, 0, 0));
	m_ToolBar.SetButtons(arToolBarBtn, 3);
	m_ToolBar.SetSizes(CSize(24, 24), CSize(16, 16));
	m_ToolBar.GetToolBarCtrl().SetImageList(&m_ToolBarImageList);
	m_ToolBar.MoveWindow(0, 0, rect.Width(), 40, 1);
	
	m_ToolBar.SetButtonStyle(0, TBBS_AUTOSIZE);
	m_ToolBar.SetButtonText(0, _T("開始"));
	
	m_ToolBar.SetButtonStyle(1, TBBS_AUTOSIZE);
	m_ToolBar.SetButtonText(1, _T("停止"));

	m_ToolBar.SetButtonStyle(2, TBBS_AUTOSIZE | TBSTYLE_CHECK);
	m_ToolBar.SetButtonText(2, _T("設定"));

	// Grid
	m_GridImageList.Create(16, 16, ILC_COLOR24 | ILC_MASK, 1, 1);
	UINT arGridImg[] = {IDB_BITMAP2, IDB_BITMAP3};
	for (int i=0; i<2; i++)
	{
		bmp.LoadBitmap(arGridImg[i]);
		m_GridImageList.Add(&bmp, RGB(0, 0, 0));
		bmp.DeleteObject();
	}

	m_Grid1.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_OVERLAPPED | WS_TABSTOP | WS_BORDER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(0, 40, 180, rect.Height()), this, IDC_GRID1, NULL);
	m_Grid1.GetPaintManager()->SetColumnStyle(xtpReportColumnFlat);
	//m_Grid1.GetPaintManager()->SetGridStyle(FALSE, xtpReportGridNoLines);
	m_Grid1.GetReportHeader()->AllowColumnRemove(FALSE);
	
	m_Grid1.SetImageList(&m_GridImageList);
	m_Grid1.SetFont(&m_Font);
	m_Grid1.ShowHeader(FALSE);
	
	m_Grid1.AddColumn(new CXTPReportColumn(0, _T("項目"), 70, 1, -1, 0));
	m_Grid1.AddColumn(new CXTPReportColumn(1, _T("內容"), 100, 1, -1, 0));

	m_Grid1.GetColumns()->GetAt(0)->SetAlignment(DT_CENTER);
	m_Grid1.GetColumns()->GetAt(1)->SetAlignment(DT_CENTER);
	
	m_Grid1.AddRecord(new CGrid1Record(this, CString("程式狀態"), m_Status));
	m_Grid1.AddRecord(new CGrid1Record(this, CString("資料庫位置"), CString(theApp.m_chDB)));
	m_Grid1.AddRecord(new CGrid1Record(this, CString("目標位置"), CString(theApp.m_chElise)));
	m_Grid1.AddRecord(new CGrid1Record(this, CString("目標名稱"), CString(theApp.m_chEliseApi)));
	m_Grid1.AddRecord(new CGrid1Record(this, CString("運作環境"), CString(theApp.m_chEnv)));
	m_Grid1.AddRecord(new CGrid1Record(this, CString("檢查頻率"), theApp.m_uFreq));
	m_Grid1.AddRecord(new CGrid1Record(this, CString("檢查次數"), m_CheckTimes));
	m_Grid1.AddRecord(new CGrid1Record(this, CString("處理次數"), m_ProcessTimes));
	m_Grid1.AddRecord(new CGrid1Record(this, CString("上傳次數"), m_SuccessTimes));
	
	m_Grid1.GetColumns()->GetAt(0)->SetEditable(0);
	
	m_Grid1.GetRecords()->GetAt(0)->GetItem(1)->SetEditable(0);
	m_Grid1.GetRecords()->GetAt(1)->GetItem(1)->SetEditable(1);
	m_Grid1.GetRecords()->GetAt(2)->GetItem(1)->SetEditable(1);
	m_Grid1.GetRecords()->GetAt(3)->GetItem(1)->SetEditable(1);
	m_Grid1.GetRecords()->GetAt(4)->GetItem(1)->SetEditable(1);
	m_Grid1.GetRecords()->GetAt(5)->GetItem(1)->SetEditable(1);
	m_Grid1.GetRecords()->GetAt(6)->GetItem(1)->SetEditable(0);
	m_Grid1.GetRecords()->GetAt(7)->GetItem(1)->SetEditable(0);
	m_Grid1.GetRecords()->GetAt(8)->GetItem(1)->SetEditable(0);

	switch (m_Status)
	{
	case 0:
		{
			m_Grid1.GetRecords()->GetAt(0)->GetItem(1)->SetIconIndex(2);
			((CXTPReportRecordItemText*)(m_Grid1.GetRecords()->GetAt(0)->GetItem(1)))->SetValue("初始設定...");
			break;
		}
	case 1: // 正常
		{
			m_Grid1.GetRecords()->GetAt(0)->GetItem(1)->SetIconIndex(0);
			((CXTPReportRecordItemText*)(m_Grid1.GetRecords()->GetAt(0)->GetItem(1)))->SetValue("正常");
			break;
		}
	case 2: // 停止
		{
			m_Grid1.GetRecords()->GetAt(0)->GetItem(1)->SetIconIndex(1);
			((CXTPReportRecordItemText*)(m_Grid1.GetRecords()->GetAt(0)->GetItem(1)))->SetValue("停止");
			break;
		}
	case 3: // 更新設定...
		{
			m_Grid1.GetRecords()->GetAt(0)->GetItem(1)->SetIconIndex(2);
			((CXTPReportRecordItemText*)(m_Grid1.GetRecords()->GetAt(0)->GetItem(1)))->SetValue("更新設定...");
			break;
		}
	default:
		{
			break;
		}
	}

	m_Grid1.Populate();
	
	m_Grid2.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_OVERLAPPED | WS_TABSTOP | WS_BORDER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(180, 40, rect.Width(), rect.Height()), this, IDC_GRID2, NULL);
	m_Grid2.GetPaintManager()->SetColumnStyle(xtpReportColumnFlat);
	m_Grid2.GetPaintManager()->SetGridStyle(FALSE, xtpReportGridNoLines);
	m_Grid2.GetReportHeader()->AllowColumnRemove(FALSE);
	
	m_Grid2.SetImageList(&m_GridImageList);
	m_Grid2.SetFont(&m_Font);
	m_Grid2.ShowHeader(FALSE);

	m_Grid2.AddColumn(new CXTPReportColumn(0, _T("編號"), 30, 1, -1, 0));
	m_Grid2.AddColumn(new CXTPReportColumn(1, _T("時間"), 80, 1, -1, 0));
	m_Grid2.AddColumn(new CXTPReportColumn(2, _T("檔案"), 60, 1, -1, 0));
	m_Grid2.AddColumn(new CXTPReportColumn(3, _T("狀態"), 40, 1, -1, 0));

	m_Grid2.GetColumns()->GetAt(0)->SetAlignment(DT_LEFT);
	m_Grid2.GetColumns()->GetAt(1)->SetAlignment(DT_LEFT);
	m_Grid2.GetColumns()->GetAt(2)->SetAlignment(DT_LEFT);
	m_Grid2.GetColumns()->GetAt(3)->SetAlignment(DT_LEFT);

	// Work Begin
	LoadSysLogClientDll();
	BeginApp();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUpdateEliseDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		if ((nID & 0xFFF0) == SC_CLOSE)
		{
//			CNotifyDlg dlgNotify;

//			dlgNotify.m_strMessage.Format("確定要關閉程式?");
//			if (dlgNotify.DoModal() == IDOK)
//			{
				EndApp();
				theApp.SetIni();
				CDialog::OnCancel();
//			}
		}

		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CUpdateEliseDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CUpdateEliseDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CUpdateEliseDlg::BeginApp()
{
	if (m_Status == 1)
		return;

	m_qdb = new CQueryToDB(CQueryToDB::ThreadFunc, this);
	VERIFY(m_qdb->CreateThread());

	m_adb = new CAlertFromDB(CAlertFromDB::ThreadFunc, this);
	VERIFY(m_adb->CreateThread());

	SetTimer(IDC_TIMER, 1000, NULL);

	m_Status = 1;
	m_Grid1.GetRecords()->GetAt(0)->GetItem(1)->SetIconIndex(0);
	((CXTPReportRecordItemText*)(m_Grid1.GetRecords()->GetAt(0)->GetItem(1)))->SetValue("正常");
	m_Grid1.RedrawControl();

	return;
}

void CUpdateEliseDlg::EndApp()
{
	if (m_Status == 2)
		return;

	DWORD nThreadID;

	KillTimer(IDC_TIMER);
	Sleep(100);

	if (m_adb)
	{
		nThreadID = m_adb->m_nThreadID;
		m_adb->StopThread();
		WaitForSingleObject(m_adb->m_hThread, INFINITE);
		delete m_adb;
		m_adb = NULL;
	}
	Sleep(100);
	
	if (m_qdb)
	{
		nThreadID = m_qdb->m_nThreadID;
		m_qdb->StopThread();
		WaitForSingleObject(m_qdb->m_hThread, INFINITE);
		delete m_qdb;
		m_qdb = NULL;
	}
	Sleep(100);

	m_Status = 2;
	m_Grid1.GetRecords()->GetAt(0)->GetItem(1)->SetIconIndex(1);
	((CXTPReportRecordItemText*)(m_Grid1.GetRecords()->GetAt(0)->GetItem(1)))->SetValue("停止");
	m_Grid1.RedrawControl();

	return;
}

UINT CUpdateEliseDlg::GetRecord()
{
	AcquireSRWLockExclusive(&m_RecordLock);

	UINT ret = m_RecordIndex;
	m_RecordIndex = (m_RecordIndex+1)%100;
	m_RecordCount ++;
	m_RecordRefresh = TRUE;

	ReleaseSRWLockExclusive(&m_RecordLock);
	
	return ret;
}

void CUpdateEliseDlg::SetRecord(UINT idx, CString& time, CString& name, UINT status)
{
	if (idx >= 100)
		return;

	AcquireSRWLockExclusive(&m_RecordLock);

	sprintf_s(m_Record[idx].time, "%s", time);
	sprintf_s(m_Record[idx].name, "%s", name);
	m_Record[idx].status = status;
	m_RecordRefresh = TRUE;

	ReleaseSRWLockExclusive(&m_RecordLock);

	return;
}

/*afx_msg */void CUpdateEliseDlg::OnTimer(UINT_PTR nIDEvent)
{
	AcquireSRWLockExclusive(&m_RecordLock);
	
	BOOL bRefresh = m_RecordRefresh;
	m_RecordRefresh = FALSE;
	
	ReleaseSRWLockExclusive(&m_RecordLock);
	
	if (m_RecordCount<=100 && m_RecordCount!=m_Grid2.GetRecords()->GetCount())
	{
		m_Grid2.SetVirtualMode(new CGrid2Record(this), m_RecordCount);
		m_Grid2.Populate();
	}
	
	if (bRefresh)
		m_Grid2.RedrawControl();

	((CGrid1Record*)m_Grid1.GetRecords()->GetAt(6))->UpdateRecord(1, m_CheckTimes);
	((CGrid1Record*)m_Grid1.GetRecords()->GetAt(7))->UpdateRecord(1, m_ProcessTimes);
	((CGrid1Record*)m_Grid1.GetRecords()->GetAt(8))->UpdateRecord(1, m_SuccessTimes);
	m_Grid1.RedrawControl();
	
	return;
}

/*afx_msg */void CUpdateEliseDlg::OnOK()
{
}

/*afx_msg */void CUpdateEliseDlg::OnCancel()
{
}

/*afx_msg */void CUpdateEliseDlg::OnDestroy()
{
}

/*afx_msg */void CUpdateEliseDlg::OnStartWork()
{
	BeginApp();
}

/*afx_msg */void CUpdateEliseDlg::OnStopWork()
{
	EndApp();
}

/*afx_msg */void CUpdateEliseDlg::OnSetting()
{
	m_Setting = (m_Setting+1) % 2;
	m_Grid1.AllowEdit(m_Setting);
	
	if (!m_Setting)
	{
		m_Grid1.GetRecords()->GetAt(0)->GetItem(1)->SetIconIndex(2);
		((CXTPReportRecordItemText*)(m_Grid1.GetRecords()->GetAt(0)->GetItem(1)))->SetValue("更新設定...");
		m_Grid1.RedrawControl();

		EndApp();
		BeginApp();
	}
}

/*afx_msg */void CUpdateEliseDlg::OnChangeSettings(NMHDR* pNotifyStruct, LRESULT* pResult)
{
	XTP_NM_REPORTRECORDITEM* pItemNotify = (XTP_NM_REPORTRECORDITEM*)pNotifyStruct;
	if(!pItemNotify) return;
	
	if(pItemNotify->pItem == NULL || pItemNotify->pItem->GetRecord() == NULL) return;
	
	CString value = ((CXTPReportRecordItemText*)(pItemNotify->pItem))->GetValue();
	int select = m_Grid1.GetSelectedRows()->GetAt(0)->GetIndex();
	int count = m_Grid1.GetRecords()->GetCount();

	switch (select)
	{
	case 1:
		{
			((CXTPReportRecordItemText*)(m_Grid1.GetRecords()->GetAt(1)->GetItem(1)))->SetValue(value);
			memset(theApp.m_chDB, NULL, 50);
			memcpy(theApp.m_chDB, value, value.GetLength());
			break;
		}
	case 2:
		{
			((CXTPReportRecordItemText*)(m_Grid1.GetRecords()->GetAt(2)->GetItem(1)))->SetValue(value);
			memset(theApp.m_chElise, NULL, 50);
			memcpy(theApp.m_chElise, value, value.GetLength());
			break;
		}
	case 3:
		{
			((CXTPReportRecordItemText*)(m_Grid1.GetRecords()->GetAt(3)->GetItem(1)))->SetValue(value);
			memset(theApp.m_chEliseApi, NULL, 50);
			memcpy(theApp.m_chEliseApi, value, value.GetLength());
			break;
		}
	case 4:
		{
			((CXTPReportRecordItemText*)(m_Grid1.GetRecords()->GetAt(4)->GetItem(1)))->SetValue(value);
			memset(theApp.m_chEnv, NULL, 4);
			memcpy(theApp.m_chEnv, value, value.GetLength());
			break;
		}
	case 5:
		{
			((CXTPReportRecordItemText*)(m_Grid1.GetRecords()->GetAt(5)->GetItem(1)))->SetValue(value);
			theApp.m_uFreq = atoi(value);
			break;
		}
	default:
		{
			break;
		}
	}

	m_Grid1.RedrawControl();
}
