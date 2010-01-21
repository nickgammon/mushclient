#if !defined(AFX_SENDTOALLDLG_H__3414F5B1_2628_11D4_BB0D_0080AD7972EF__INCLUDED_)
#define AFX_SENDTOALLDLG_H__3414F5B1_2628_11D4_BB0D_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SendToAllDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSendToAllDlg dialog

class CSendToAllDlg : public CDialog
{
// Construction
public:
	CSendToAllDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSendToAllDlg)
	enum { IDD = IDD_SEND_TO_ALL };
	CEdit	m_ctlSendText;
	CListBox	m_ctlWorldList;
	CString	m_strSendText;
	BOOL	m_bEcho;
	//}}AFX_DATA

  CFont m_font;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendToAllDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSendToAllDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectAll();
	afx_msg void OnSelectNone();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateCheckIfReady(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENDTOALLDLG_H__3414F5B1_2628_11D4_BB0D_0080AD7972EF__INCLUDED_)
