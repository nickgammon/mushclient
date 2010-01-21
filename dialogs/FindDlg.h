#if !defined(AFX_FINDDLG_H__84E45A73_B390_11D0_8EBA_00A0247B3BFD__INCLUDED_)
#define AFX_FINDDLG_H__84E45A73_B390_11D0_8EBA_00A0247B3BFD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// FindDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFindDlg dialog

class CFindDlg : public CDialog
{
// Construction
public:
	CFindDlg(CStringList & strFindStringList,
           CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFindDlg)
	enum { IDD = IDD_FIND };
	CComboBox	m_ctlFindText;
	BOOL	m_bMatchCase;
	int		m_bForwards;
	BOOL	m_bRegexp;
	CString	m_strFindText;
	//}}AFX_DATA

  CString m_strTitle;

  CStringList & m_strFindStringList;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFindDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRegexpHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDDLG_H__84E45A73_B390_11D0_8EBA_00A0247B3BFD__INCLUDED_)
