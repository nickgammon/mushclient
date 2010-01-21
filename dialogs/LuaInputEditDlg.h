#if !defined(AFX_LUAINPUTEDITDLG_H__8014884E_A636_11DA_996F_00008C012785__INCLUDED_)
#define AFX_LUAINPUTEDITDLG_H__8014884E_A636_11DA_996F_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LuaInputEditDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLuaInputEditDlg dialog

class CLuaInputEditDlg : public CDialog
{
// Construction
public:
	CLuaInputEditDlg(CWnd* pParent = NULL);   // standard constructor

  ~CLuaInputEditDlg () { delete m_font; }

// Dialog Data
	//{{AFX_DATA(CLuaInputEditDlg)
	enum { IDD = IDD_LUA_INPUTEDIT };
	CButton	m_ctlCancel;
	CButton	m_ctlOK;
	CEdit	m_ctlReply;
	CString	m_strMessage;
	CString	m_strReply;
	//}}AFX_DATA

  CString m_strTitle;
  CString m_strFont;
  int m_iFontSize;

  CFont * m_font;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLuaInputEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLuaInputEditDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg void OnRemoveSelection();

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LUAINPUTEDITDLG_H__8014884E_A636_11DA_996F_00008C012785__INCLUDED_)
