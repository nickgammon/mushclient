#if !defined(AFX_PREFSPROPERTYSHEET_H__11FEF0C3_AA75_11D0_8EA7_00A0247B3BFD__INCLUDED_)
#define AFX_PREFSPROPERTYSHEET_H__11FEF0C3_AA75_11D0_8EA7_00A0247B3BFD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PrefsPropertySheet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPrefsPropertySheet

class CPrefsPropertySheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CPrefsPropertySheet)

// Construction
public:
	CPrefsPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CPrefsPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsPropertySheet)
	public:
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPrefsPropertySheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPrefsPropertySheet)
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREFSPROPERTYSHEET_H__11FEF0C3_AA75_11D0_8EA7_00A0247B3BFD__INCLUDED_)
