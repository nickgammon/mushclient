#if !defined(AFX_GLOBALCHANGEDLG_H__32AAE166_5E05_11D3_A6CB_0000B4595568__INCLUDED_)
#define AFX_GLOBALCHANGEDLG_H__32AAE166_5E05_11D3_A6CB_0000B4595568__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GlobalChangeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGlobalChangeDlg dialog

class CGlobalChangeDlg : public CDialog
{
// Construction
public:
	CGlobalChangeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGlobalChangeDlg)
	enum { IDD = IDD_GLOBAL_CHANGE };
	CString	m_strChangeTo;
	CString	m_strChangeFrom;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGlobalChangeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGlobalChangeDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GLOBALCHANGEDLG_H__32AAE166_5E05_11D3_A6CB_0000B4595568__INCLUDED_)
