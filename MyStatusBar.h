#if !defined(AFX_MYSTATUSBAR_H__040BE3BE_7AD7_11D8_9931_00008C012785__INCLUDED_)
#define AFX_MYSTATUSBAR_H__040BE3BE_7AD7_11D8_9931_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyStatusBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMyStatusBar window

class CMyStatusBar : public CStatusBar
{
// Construction
public:
	CMyStatusBar();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyStatusBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMyStatusBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMyStatusBar)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYSTATUSBAR_H__040BE3BE_7AD7_11D8_9931_00008C012785__INCLUDED_)
