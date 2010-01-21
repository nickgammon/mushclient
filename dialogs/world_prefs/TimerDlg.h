#if !defined(AFX_TIMERDLG_H__11FEF0C4_AA75_11D0_8EA7_00A0247B3BFD__INCLUDED_)
#define AFX_TIMERDLG_H__11FEF0C4_AA75_11D0_8EA7_00A0247B3BFD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TimerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTimerDlg dialog

class CTimerDlg : public CDialog
{
	DECLARE_DYNAMIC(CTimerDlg)

// Construction
public:
	CTimerDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTimerDlg)
	enum { IDD = IDD_EDIT_TIMER };
	CComboBox	m_ctlSendTo;
	CEdit	m_ctlProcedure;
	CEdit	m_ctlContents;
	CButton	m_ctlButtonAt;
	BOOL	m_bEnabled;
	CString	m_strLabel;
	int		m_iAtHour;
	int		m_iAtMinute;
	int		m_iEveryHour;
	int		m_iEveryMinute;
	int		m_iOffsetHour;
	int		m_iOffsetMinute;
	CString	m_strContents;
	int		m_iType;
	CString	m_strProcedure;
	BOOL	m_bOneShot;
	BOOL	m_bTemporary;
	BOOL	m_bActiveWhenClosed;
	CString	m_strGroup;
	int		m_iSendTo;
	CString	m_strVariable;
	CString	m_strIncluded;
	BOOL	m_bOmitFromOutput;
	BOOL	m_bOmitFromLog;
	double	m_fAtSecond;
	double	m_fEverySecond;
	double	m_fOffsetSecond;
	//}}AFX_DATA

  CTimerMap * m_pTimerMap;
  CTimer * m_current_timer;
  CMUSHclientDoc * m_pDoc;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTimerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTimerDlg)
	afx_msg void OnButtonAt();
	afx_msg void OnButtonEvery();
	afx_msg void OnHelpbutton();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditSend();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateOK(CCmdUI* pCmdUI);
  afx_msg void OnUpdateInvocationCount(CCmdUI* pCmdUI);
  afx_msg void OnUpdateMatchCount(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TIMERDLG_H__11FEF0C4_AA75_11D0_8EA7_00A0247B3BFD__INCLUDED_)
