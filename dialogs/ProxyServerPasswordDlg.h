#if !defined(AFX_PROXYSERVERPASSWORDDLG_H__0A7832A6_783C_11D7_A77E_0080AD7972EF__INCLUDED_)
#define AFX_PROXYSERVERPASSWORDDLG_H__0A7832A6_783C_11D7_A77E_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProxyServerPasswordDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProxyServerPasswordDlg dialog

class CProxyServerPasswordDlg : public CDialog
{
// Construction
public:
	CProxyServerPasswordDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProxyServerPasswordDlg)
	enum { IDD = IDD_PROXY_PASSWORD };
	CString	m_strProxyUserName;
	CString	m_strProxyPassword;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProxyServerPasswordDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProxyServerPasswordDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROXYSERVERPASSWORDDLG_H__0A7832A6_783C_11D7_A77E_0080AD7972EF__INCLUDED_)
