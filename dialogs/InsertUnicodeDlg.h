#if !defined(AFX_INSERTUNICODEDLG_H__84B8CA6E_EEB9_11DB_99AC_00008C012785__INCLUDED_)
#define AFX_INSERTUNICODEDLG_H__84B8CA6E_EEB9_11DB_99AC_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InsertUnicodeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInsertUnicodeDlg dialog

class CInsertUnicodeDlg : public CDialog
{
// Construction
public:
	CInsertUnicodeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInsertUnicodeDlg)
	enum { IDD = IDD_INSERT_UNICODE };
	CString	m_strCharacter;
	BOOL	m_bHex;
	//}}AFX_DATA

  __int64 m_iCode;  // converted code

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInsertUnicodeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInsertUnicodeDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INSERTUNICODEDLG_H__84B8CA6E_EEB9_11DB_99AC_00008C012785__INCLUDED_)
