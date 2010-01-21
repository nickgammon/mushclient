#if !defined(AFX_LUAINPUTBOX_H__6E9EEA72_586E_11DA_9963_00008C012785__INCLUDED_)
#define AFX_LUAINPUTBOX_H__6E9EEA72_586E_11DA_9963_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LuaInputBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLuaInputBox dialog

class CLuaInputBox : public CDialog
{
// Construction
public:
	CLuaInputBox(CWnd* pParent = NULL);   // standard constructor
            
  ~CLuaInputBox () { delete m_font; }

// Dialog Data
	//{{AFX_DATA(CLuaInputBox)
	enum { IDD = IDD_LUA_INPUTBOX };
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
	//{{AFX_VIRTUAL(CLuaInputBox)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLuaInputBox)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	afx_msg void OnRemoveSelection();

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LUAINPUTBOX_H__6E9EEA72_586E_11DA_9963_00008C012785__INCLUDED_)
