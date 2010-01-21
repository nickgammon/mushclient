#if !defined(AFX_CHATCALLDLG_H__9B3793C7_7539_11D7_A77E_0080AD7972EF__INCLUDED_)
#define AFX_CHATCALLDLG_H__9B3793C7_7539_11D7_A77E_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChatCallDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChatCallDlg dialog

class CChatCallDlg : public CDialog
{
// Construction
public:
	CChatCallDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChatCallDlg)
	enum { IDD = IDD_CHAT_CALL };
	long	m_iPort;
	CString	m_strAddress;
	BOOL	m_bzChat;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChatCallDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChatCallDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHATCALLDLG_H__9B3793C7_7539_11D7_A77E_0080AD7972EF__INCLUDED_)
