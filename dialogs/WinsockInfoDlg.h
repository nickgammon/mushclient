#if !defined(AFX_WINSOCKINFODLG_H__488873B7_4CDF_11D4_BB13_0080AD7972EF__INCLUDED_)
#define AFX_WINSOCKINFODLG_H__488873B7_4CDF_11D4_BB13_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WinsockInfoDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWinsockInfoDlg dialog

class CWinsockInfoDlg : public CDialog
{
// Construction
public:
	CWinsockInfoDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWinsockInfoDlg)
	enum { IDD = IDD_WINSOCK_INFO };
	CString	m_szDescription;
	CString	m_szSystemStatus;
	short	m_iMaxSockets;
	CString	m_strVersion;
	CString	m_strHighVersion;
	CString	m_strHostName;
	CString	m_strAddresses;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinsockInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWinsockInfoDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINSOCKINFODLG_H__488873B7_4CDF_11D4_BB13_0080AD7972EF__INCLUDED_)
