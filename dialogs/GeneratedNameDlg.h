#if !defined(AFX_GENERATEDNAMEDLG_H__C14D6353_34F9_11D4_BB0E_0080AD7972EF__INCLUDED_)
#define AFX_GENERATEDNAMEDLG_H__C14D6353_34F9_11D4_BB0E_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GeneratedNameDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGeneratedNameDlg dialog

class CGeneratedNameDlg : public CDialog
{
// Construction
public:
	CGeneratedNameDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGeneratedNameDlg)
	enum { IDD = IDD_GENERATED_NAME };
	CStatic	m_ctlFileName;
	CEdit	m_ctlName;
	CString	m_strName;
	CString	m_strFileName;
	//}}AFX_DATA


  CMUSHclientDoc * m_pDoc;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeneratedNameDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGeneratedNameDlg)
	afx_msg void OnTryAgain();
	afx_msg void OnCopy();
	afx_msg void OnSendToWorld();
	afx_msg void OnBrowseName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENERATEDNAMEDLG_H__C14D6353_34F9_11D4_BB0E_0080AD7972EF__INCLUDED_)
