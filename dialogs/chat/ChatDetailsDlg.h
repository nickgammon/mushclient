#if !defined(AFX_CHATDETAILSDLG_H__AA5D5A78_7541_11D7_A77E_0080AD7972EF__INCLUDED_)
#define AFX_CHATDETAILSDLG_H__AA5D5A78_7541_11D7_A77E_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChatDetailsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChatDetailsDlg dialog

class CChatDetailsDlg : public CDialog
{
// Construction
public:
	CChatDetailsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChatDetailsDlg)
	enum { IDD = IDD_CHAT_DETAILS };
	CString	m_strRemoteUserName;
	CString	m_strGroup;
	CString	m_strChatID;
	CString	m_strRemoteVersion;
	BOOL	m_bIncoming;
	BOOL	m_bCanSnoop;
	BOOL	m_bCanSendFiles;
	BOOL	m_bPrivate;
	BOOL	m_bIgnore;
	BOOL	m_bCanSendCommands;
	CString	m_strAllegedAddress;
	CString	m_strAllegedPort;
	CString	m_strActualIPaddress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChatDetailsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChatDetailsDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHATDETAILSDLG_H__AA5D5A78_7541_11D7_A77E_0080AD7972EF__INCLUDED_)
