#if !defined(AFX_KEYNAMEDLG_H__6CC9D657_FD01_11DA_9974_00008C012785__INCLUDED_)
#define AFX_KEYNAMEDLG_H__6CC9D657_FD01_11DA_9974_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// KeyNameDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CKeyNameDlg dialog

class CKeyNameDlg : public CDialog
{
// Construction
public:
	CKeyNameDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CKeyNameDlg)
	enum { IDD = IDD_KEY_NAME };
	CHotKeyCtrl	m_ctlHotkeyValue;
	CEdit	m_ctlTextName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyNameDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CKeyNameDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateKeyName(CCmdUI* pCmdUI);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYNAMEDLG_H__6CC9D657_FD01_11DA_9974_00008C012785__INCLUDED_)
