#if !defined(AFX_CHATLISTDLG_H__9B3793C1_7539_11D7_A77E_0080AD7972EF__INCLUDED_)
#define AFX_CHATLISTDLG_H__9B3793C1_7539_11D7_A77E_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChatListDlg.h : header file
//

class CMUSHclientDoc;

  enum 
  { 
    eColumnChatName,
    eColumnGroup,
    eColumnFromIP,
    eColumnCallIP,
    eColumnCallPort,
    eColumnFlags,
    eColumnChatCount      // this must be last!
  };

/////////////////////////////////////////////////////////////////////////////
// CChatListDlg dialog

class CChatListDlg : public CDialog
{
// Construction
public:
	CChatListDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChatListDlg)
	enum { IDD = IDD_CHAT_LIST };
	CListCtrl	m_ctlChatList;
	CString	m_strStatus;
	//}}AFX_DATA


  CMUSHclientDoc * m_pDoc;

  // for sorting the list

  int m_last_col;
  BOOL m_reverse;

  void LoadList (void);
  CString MakeListHash (void);

  CString m_strListHash;

  CList<int, int> m_SelectedList;   // which ones were selected
  CList<int, int> m_FocussedList;   // which ones were focussed

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChatListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChatListDlg)
	afx_msg void OnCall();
	afx_msg void OnChatAll();
	afx_msg void OnEmoteAll();
	virtual BOOL OnInitDialog();
	afx_msg void OnEdit();
	afx_msg void OnPing();
	afx_msg void OnSnoop();
	afx_msg void OnSendfile();
	afx_msg void OnHangup();
	afx_msg void OnChat();
	afx_msg void OnEmote();
	afx_msg void OnPeek();
	afx_msg void OnRequest();
	afx_msg void OnDestroy();
	afx_msg void OnDblclkChatSessions(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickChatSessions(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedSelection(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedOneSelection(CCmdUI* pCmdUI);
  afx_msg void OnUpdateCheckList(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHATLISTDLG_H__9B3793C1_7539_11D7_A77E_0080AD7972EF__INCLUDED_)
