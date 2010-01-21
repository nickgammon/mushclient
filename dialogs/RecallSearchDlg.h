#if !defined(AFX_RECALLSEARCHDLG_H__3414F5A7_2628_11D4_BB0D_0080AD7972EF__INCLUDED_)
#define AFX_RECALLSEARCHDLG_H__3414F5A7_2628_11D4_BB0D_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RecallSearchDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRecallSearchDlg dialog

class CRecallSearchDlg : public CDialog
{
// Construction
public:
	CRecallSearchDlg(CStringList & strFindStringList,
                   CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRecallSearchDlg)
	enum { IDD = IDD_RECALL };
	CComboBox	m_ctlFindText;
	BOOL	m_bMatchCase;
	BOOL	m_bRegexp;
	CString	m_strFindText;
	int		m_iLines;
	BOOL	m_bOutput;
	BOOL	m_bCommands;
	BOOL	m_bNotes;
	CString	m_strRecallLinePreamble;
	//}}AFX_DATA

  CString m_strTitle;

  CStringList & m_strFindStringList;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecallSearchDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRecallSearchDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSubsitutionHelp();
	afx_msg void OnRegexpHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECALLSEARCHDLG_H__3414F5A7_2628_11D4_BB0D_0080AD7972EF__INCLUDED_)
