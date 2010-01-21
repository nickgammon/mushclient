#if !defined(AFX_CHATTOALLDLG_H__AA5D5A73_7541_11D7_A77E_0080AD7972EF__INCLUDED_)
#define AFX_CHATTOALLDLG_H__AA5D5A73_7541_11D7_A77E_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChatToAllDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChatToAllDlg dialog

class CChatToAllDlg : public CDialog
{
// Construction
public:
	CChatToAllDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChatToAllDlg)
	enum { IDD = IDD_CHAT_SAY_ALL };
	CString	m_strText;
	//}}AFX_DATA


  CString m_strTitle;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChatToAllDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChatToAllDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHATTOALLDLG_H__AA5D5A73_7541_11D7_A77E_0080AD7972EF__INCLUDED_)
