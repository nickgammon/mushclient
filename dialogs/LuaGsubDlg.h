#if !defined(AFX_LUAGSUBDLG_H__CFFC37B4_6534_11DA_9964_00008C012785__INCLUDED_)
#define AFX_LUAGSUBDLG_H__CFFC37B4_6534_11DA_9964_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LuaGsubDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLuaGsubDlg dialog

class CLuaGsubDlg : public CDialog
{
// Construction
public:
	CLuaGsubDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLuaGsubDlg)
	enum { IDD = IDD_LUA_GSUB };
	CEdit	m_ctlReplacementText;
	CButton	m_ctlCallFunction;
	CEdit	m_ctlFindText;
	BOOL	m_bEachLine;
	CString	m_strFindPattern;
	CString	m_strReplacement;
	BOOL	m_bEscapeSequences;
	BOOL	m_bCallFunction;
	CString	m_strSelectionSize;
	//}}AFX_DATA

  CString m_strFunctionText;
  lua_State           * L;                  // Lua state

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLuaGsubDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLuaGsubDlg)
	afx_msg void OnEditFunction();
	afx_msg void OnEditFind();
	afx_msg void OnEditReplace();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedFindText(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LUAGSUBDLG_H__CFFC37B4_6534_11DA_9964_00008C012785__INCLUDED_)
