#if !defined(AFX_MXPSCRIPTROUTINESDLG_H__7FB4E347_5865_11D5_BB6E_0080AD7972EF__INCLUDED_)
#define AFX_MXPSCRIPTROUTINESDLG_H__7FB4E347_5865_11D5_BB6E_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MXPscriptRoutinesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMXPscriptRoutinesDlg dialog

class CMXPscriptRoutinesDlg : public CDialog
{
// Construction
public:
	CMXPscriptRoutinesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMXPscriptRoutinesDlg)
	enum { IDD = IDD_MXP_SCRIPTS };
	CString	m_strOnMXP_Start;
	CString	m_strOnMXP_Stop;
	CString	m_strOnMXP_OpenTag;
	CString	m_strOnMXP_CloseTag;
	CString	m_strOnMXP_SetVariable;
	CString	m_strOnMXP_Error;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMXPscriptRoutinesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMXPscriptRoutinesDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MXPSCRIPTROUTINESDLG_H__7FB4E347_5865_11D5_BB6E_0080AD7972EF__INCLUDED_)
