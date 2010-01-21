#if !defined(AFX_SCRIPTERRORDLG_H__CF599418_3B2F_11D4_BB11_0080AD7972EF__INCLUDED_)
#define AFX_SCRIPTERRORDLG_H__CF599418_3B2F_11D4_BB11_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScriptErrorDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScriptErrorDlg dialog

class CScriptErrorDlg : public CDialog
{
// Construction
public:
	CScriptErrorDlg(CWnd* pParent = NULL);   // standard constructor

  ~CScriptErrorDlg () { delete m_font; }

// Dialog Data
	//{{AFX_DATA(CScriptErrorDlg)
	enum { IDD = IDD_SCRIPT_ERROR };
	CEdit	m_ctlRaisedByString;
	CStatic	m_ctlRaisedByLabel;
	CButton	m_ctlUseOutputWindow;
	CEdit	m_ctlDescription;
	long	m_iError;
	CString	m_strEvent;
	CString	m_strDescription;
	CString	m_strCalledBy;
	CString	m_strRaisedBy;
	BOOL	m_bUseOutputWindow;
	//}}AFX_DATA

  CFont * m_font;
  bool m_bHaveDoc;    // is there a document?

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScriptErrorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CScriptErrorDlg)
	afx_msg void OnCopy();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCRIPTERRORDLG_H__CF599418_3B2F_11D4_BB11_0080AD7972EF__INCLUDED_)
