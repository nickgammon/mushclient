#if !defined(AFX_MISSINGENTRYPOINTS_H__CDF93193_C987_11D0_8EB9_00A0247B3BFD__INCLUDED_)
#define AFX_MISSINGENTRYPOINTS_H__CDF93193_C987_11D0_8EB9_00A0247B3BFD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MissingEntryPoints.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMissingEntryPoints dialog

class CMissingEntryPoints : public CDialog
{
// Construction
public:
	CMissingEntryPoints(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMissingEntryPoints)
	enum { IDD = IDD_MISSING_ENTRY_POINTS };
	CString	m_strErrorMessage;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMissingEntryPoints)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMissingEntryPoints)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MISSINGENTRYPOINTS_H__CDF93193_C987_11D0_8EB9_00A0247B3BFD__INCLUDED_)
