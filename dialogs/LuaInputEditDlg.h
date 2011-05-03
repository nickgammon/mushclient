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
	//{{AFX_VIRTUAL(CLuaInputEditDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
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
