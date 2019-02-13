// UpdateEliseDlg.h : header file
//

#pragma once
#include "QueryToDB.h"
#include "AlertFromDB.h"

typedef struct CRecord
{
	char time[16];
	char name[40];
	UINT status;
} CRecord;

class CUpdateEliseDlg;
class CGrid1Record : public CXTPReportRecord
{
public:
	CGrid1Record(void);
	CGrid1Record(CUpdateEliseDlg* master, CString& name, CString& value);
	CGrid1Record(CUpdateEliseDlg* master, CString& name, UINT value);
	virtual ~CGrid1Record(void);

	virtual void CreateItems();
	virtual void GetItemMetrics(XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs, XTP_REPORTRECORDITEM_METRICS* pItemMetrics);
	BOOL UpdateRecord(int idx, CString& data);
	BOOL UpdateRecord(int idx, UINT data);
	CUpdateEliseDlg* m_master;
};

class CGrid2Record : public CXTPReportRecord
{
public:
	CGrid2Record(CUpdateEliseDlg* master);
	virtual ~CGrid2Record(void);

	virtual void CreateItems();
	virtual void GetItemMetrics(XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs, XTP_REPORTRECORDITEM_METRICS* pItemMetrics);
	CUpdateEliseDlg* m_master;
};

// CUpdateEliseDlg dialog
class CUpdateEliseDlg : public CDialog 
{
// Construction
public:
	CUpdateEliseDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_UPDATEELISE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CQueryToDB*			m_qdb;
	CAlertFromDB*		m_adb;

	CFont				m_Font;
	CImageList			m_ToolBarImageList;
	CToolBar			m_ToolBar;

	CImageList			m_GridImageList;
	CXTPReportControl	m_Grid1;
	CXTPReportControl	m_Grid2;

	void BeginApp();
	void EndApp();

public:
	CRecord				m_Record[100];
	UINT				m_RecordIndex;
	UINT				m_RecordCount;
	SRWLOCK				m_RecordLock;
	BOOL				m_RecordRefresh;

	UINT				GetRecord();
	void				SetRecord(UINT, CString&, CString&, UINT);

	UINT				m_CheckTimes;
	UINT				m_ProcessTimes;
	UINT				m_SuccessTimes;

	BOOL				m_Setting;
	UINT				m_Status;
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnDestroy();

	afx_msg void OnStartWork();
	afx_msg void OnStopWork();
	afx_msg void OnSetting();

	afx_msg void OnChangeSettings(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};
