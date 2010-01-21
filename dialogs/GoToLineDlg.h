#if !defined(AFX_GOTOLINEDLG_H__D2527D3E_0F59_11D4_BB0A_0080AD7972EF__INCLUDED_)
#define AFX_GOTOLINEDLG_H__D2527D3E_0F59_11D4_BB0A_0080AD7972EF__INCLUDED_

/*
Copyright (C) 2000 Nick Gammon.

*/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GoToLineDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGoToLineDlg dialog

class CGoToLineDlg : public CDialog
{
// Construction
public:
	CGoToLineDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGoToLineDlg)
	enum { IDD = IDD_GOTO };
	int		m_iLineNumber;
	//}}AFX_DATA

  int m_iMaxLine;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGoToLineDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGoToLineDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GOTOLINEDLG_H__D2527D3E_0F59_11D4_BB0A_0080AD7972EF__INCLUDED_)
