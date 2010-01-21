#if !defined(AFX_EMOTETOALLDLG_H__AA5D5A74_7541_11D7_A77E_0080AD7972EF__INCLUDED_)
#define AFX_EMOTETOALLDLG_H__AA5D5A74_7541_11D7_A77E_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmoteToAllDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEmoteToAllDlg dialog

class CEmoteToAllDlg : public CDialog
{
// Construction
public:
	CEmoteToAllDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEmoteToAllDlg)
	enum { IDD = IDD_CHAT_EMOTE_ALL };
	CString	m_strText;
	//}}AFX_DATA


  CString m_strTitle;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmoteToAllDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEmoteToAllDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMOTETOALLDLG_H__AA5D5A74_7541_11D7_A77E_0080AD7972EF__INCLUDED_)
