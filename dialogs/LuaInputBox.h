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

  int m_iBoxWidth;
  int m_iBoxHeight;
  int m_iPromptWidth;
  int m_iPromptHeight;
  int m_iReplyWidth;
  int m_iReplyHeight;
  int m_iMaxReplyLength;
  bool m_bReadOnly;
  CString m_strOKbuttonLabel;
  CString m_strCancelbuttonLabel;
  int m_iOKbuttonWidth;
  int m_iCancelbuttonWidth;
  bool m_bNoDefault;

  lua_State *m_L;   // for validating
  int m_iValidationIndex;  // where validation function is

  void Calculate_Button_Positions ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLuaInputBox)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLuaInputBox)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg void OnRemoveSelection();

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LUAINPUTBOX_H__6E9EEA72_586E_11DA_9963_00008C012785__INCLUDED_)
