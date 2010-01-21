#if !defined(AFX_DEBUGLUADLG_H__66BF9180_3B92_11DB_997F_00008C012785__INCLUDED_)
#define AFX_DEBUGLUADLG_H__66BF9180_3B92_11DB_997F_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DebugLuaDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDebugLuaDlg dialog

class CDebugLuaDlg : public CDialog
{
// Construction
public:
	CDebugLuaDlg(CWnd* pParent = NULL);   // standard constructor

  ~CDebugLuaDlg () { delete m_font; };

// Dialog Data
	//{{AFX_DATA(CDebugLuaDlg)
	enum { IDD = IDD_LUA_DEBUG };
	CEdit	m_ctlCommand;
	CString	m_strCommand;
	CString	m_strCurrentLine;
	CString	m_strFunctionName;
	CString	m_strFunction;
	CString	m_strLines;
	CString	m_strNups;
	CString	m_strSource;
	CString	m_strWhat;
	//}}AFX_DATA

  lua_State *L;
  bool m_bAbort;
  CFont * m_font;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDebugLuaDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDebugLuaDlg)
	afx_msg void OnDoCommand();
	afx_msg void OnEditCommand();
	virtual void OnCancel();
	afx_msg void OnShowUps();
	afx_msg void OnShowLocals();
	afx_msg void OnTraceback();
	afx_msg void OnAbort();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEBUGLUADLG_H__66BF9180_3B92_11DB_997F_00008C012785__INCLUDED_)
