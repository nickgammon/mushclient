#if !defined(AFX_REGEXPPROBLEMDLG_H__7404107A_94A0_11D8_9937_00008C012785__INCLUDED_)
#define AFX_REGEXPPROBLEMDLG_H__7404107A_94A0_11D8_9937_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RegexpProblemDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRegexpProblemDlg dialog

class CRegexpProblemDlg : public CDialog
{
// Construction
public:
	CRegexpProblemDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRegexpProblemDlg)
	enum { IDD = IDD_REGEXP_PROBLEM };
	CStatic	m_ctlErrorMessage;
	CEdit	m_ctlRegexpText;
	CString	m_strColumn;
	CString	m_strErrorMessage;
	CString	m_strText;
	//}}AFX_DATA

  CFont m_font;
  int m_iColumn;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRegexpProblemDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRegexpProblemDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSetfocusRegexpText();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REGEXPPROBLEMDLG_H__7404107A_94A0_11D8_9937_00008C012785__INCLUDED_)
