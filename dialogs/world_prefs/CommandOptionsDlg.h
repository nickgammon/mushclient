#if !defined(AFX_COMMANDOPTIONSDLG_H__7FB4E346_5865_11D5_BB6E_0080AD7972EF__INCLUDED_)
#define AFX_COMMANDOPTIONSDLG_H__7FB4E346_5865_11D5_BB6E_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CommandOptionsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCommandOptionsDlg dialog

class CCommandOptionsDlg : public CDialog
{
// Construction
public:
	CCommandOptionsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCommandOptionsDlg)
	enum { IDD = IDD_COMMAND_OPTIONS };
	CButton	m_ctlEscapeDeletesTypeing;
	CButton	m_ctlDoubleClickSends;
	CButton	m_ctlDoubleClickPastesWord;
	BOOL	m_bDoubleClickInserts;
	BOOL	m_bDoubleClickSends;
	BOOL	m_bArrowKeysWrap;
	BOOL	m_bArrowsChangeHistory;
	BOOL	m_bArrowRecallsPartial;
	BOOL	m_bAltArrowRecallsPartial;
	BOOL	m_bEscapeDeletesInput;
	BOOL	m_bSaveDeletedCommand;
	BOOL	m_bConfirmBeforeReplacingTyping;
	BOOL	m_bCtrlZGoesToEndOfBuffer;
	BOOL	m_bCtrlPGoesToPreviousCommand;
	BOOL	m_bCtrlNGoesToNextCommand;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCommandOptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCommandOptionsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDoubleClickSends();
	afx_msg void OnDoubleClickInserts();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedEscapeDeletesTyping(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMMANDOPTIONSDLG_H__7FB4E346_5865_11D5_BB6E_0080AD7972EF__INCLUDED_)
