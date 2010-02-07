#if !defined(AFX_FUNCTIONLISTDLG_H__6E9EEA77_586E_11DA_9963_00008C012785__INCLUDED_)
#define AFX_FUNCTIONLISTDLG_H__6E9EEA77_586E_11DA_9963_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FunctionListDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFunctionListDlg dialog

class CFunctionListDlg : public CDialog
{
// Construction
public:
	CFunctionListDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFunctionListDlg)
	enum { IDD = IDD_FUNCTION_LIST };
	CEdit	m_ctlFilter;
	CListCtrl	m_ctlFunctions;
	CString	m_strFilter;
	//}}AFX_DATA

  CString m_strResult;

  bool m_bLua;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFunctionListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

  BOOL ReloadList ();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFunctionListDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblclkFunctionsList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLuaFunctions();
	afx_msg void OnCopyName();
	afx_msg void OnChangeFilter();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedSelection(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FUNCTIONLISTDLG_H__6E9EEA77_586E_11DA_9963_00008C012785__INCLUDED_)
