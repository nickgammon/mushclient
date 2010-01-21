#if !defined(AFX_EDITVARIABLE_H__D34279E3_B2C3_11D0_8EB9_00A0247B3BFD__INCLUDED_)
#define AFX_EDITVARIABLE_H__D34279E3_B2C3_11D0_8EB9_00A0247B3BFD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// EditVariable.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditVariable dialog

class CEditVariable : public CDialog
{
	DECLARE_DYNAMIC(CEditVariable)

// Construction
public:
	CEditVariable(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditVariable)
	enum { IDD = IDD_EDIT_VARIABLE };
	CEdit	m_ctlContents;
	CEdit	m_ctlName;
	CString	m_strContents;
	CString	m_strName;
	//}}AFX_DATA

  CVariableMap * m_pVariableMap;
  bool m_bDoingChange;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditVariable)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditVariable)
	afx_msg void OnHelpbutton();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditContents();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITVARIABLE_H__D34279E3_B2C3_11D0_8EB9_00A0247B3BFD__INCLUDED_)
