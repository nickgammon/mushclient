#if !defined(AFX_IMMEDIATEDLG_H__11DFC5FF_AD6F_11D0_8EAE_00A0247B3BFD__INCLUDED_)
#define AFX_IMMEDIATEDLG_H__11DFC5FF_AD6F_11D0_8EAE_00A0247B3BFD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ImmediateDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImmediateDlg dialog

class CImmediateDlg : public CDialog
{
// Construction
public:
	CImmediateDlg(CWnd* pParent = NULL);   // standard constructor

  CMUSHclientDoc * m_pDoc;    // owning document

// Dialog Data
	//{{AFX_DATA(CImmediateDlg)
	enum { IDD = IDD_IMMEDIATE };
	CEdit	m_ctlExpression;
	CString	m_strExpression;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImmediateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  virtual void OnCancel( );

	// Generated message map functions
	//{{AFX_MSG(CImmediateDlg)
	afx_msg void OnRun();
	afx_msg void OnEdit();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	afx_msg void OnRemoveSelection();
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMMEDIATEDLG_H__11DFC5FF_AD6F_11D0_8EAE_00A0247B3BFD__INCLUDED_)
