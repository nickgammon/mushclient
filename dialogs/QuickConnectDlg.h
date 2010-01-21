#if !defined(AFX_QUICKCONNECTDLG_H__8E59D505_D802_11D2_A27F_0000B4595568__INCLUDED_)
#define AFX_QUICKCONNECTDLG_H__8E59D505_D802_11D2_A27F_0000B4595568__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// QuickConnectDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CQuickConnectDlg dialog

class CQuickConnectDlg : public CDialog
{
// Construction
public:
	CQuickConnectDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CQuickConnectDlg)
	enum { IDD = IDD_QUICK_CONNECT };
	CString	m_strWorldName;
	CString	m_strAddress;
	int		m_iPort;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQuickConnectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CQuickConnectDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUICKCONNECTDLG_H__8E59D505_D802_11D2_A27F_0000B4595568__INCLUDED_)
