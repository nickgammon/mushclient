#if !defined(AFX_CHOOSENOTEPADDLG_H__CF599423_3B2F_11D4_BB11_0080AD7972EF__INCLUDED_)
#define AFX_CHOOSENOTEPADDLG_H__CF599423_3B2F_11D4_BB11_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChooseNotepadDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseNotepadDlg dialog

class CChooseNotepadDlg : public CDialog
{
// Construction
public:
	CChooseNotepadDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChooseNotepadDlg)
	enum { IDD = IDD_CHOOSE_NOTEPAD };
	CListBox	m_ctlNotepadList;
	//}}AFX_DATA

  CMUSHclientDoc * m_pWorld;
  CTextDocument  * m_pTextDocument;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseNotepadDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChooseNotepadDlg)
	afx_msg void OnOpenExisting();
	afx_msg void OnDblclkNotepadList();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedSelection(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSENOTEPADDLG_H__CF599423_3B2F_11D4_BB11_0080AD7972EF__INCLUDED_)
