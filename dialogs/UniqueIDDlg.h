#if !defined(AFX_UNIQUEIDDLG_H__A3149331_56DC_11D9_9957_00008C012785__INCLUDED_)
#define AFX_UNIQUEIDDLG_H__A3149331_56DC_11D9_9957_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UniqueIDDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUniqueIDDlg dialog

class CUniqueIDDlg : public CDialog
{
// Construction
public:
	CUniqueIDDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUniqueIDDlg)
	enum { IDD = IDD_GENERATE_UNIQUE_ID };
	CString	m_strUniqueID;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUniqueIDDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUniqueIDDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNIQUEIDDLG_H__A3149331_56DC_11D9_9957_00008C012785__INCLUDED_)
