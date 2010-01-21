#if !defined(AFX_CREDITSDLG_H__71365423_C53B_11D2_A279_0000B4595568__INCLUDED_)
#define AFX_CREDITSDLG_H__71365423_C53B_11D2_A279_0000B4595568__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CreditsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCreditsDlg dialog

class CCreditsDlg : public CDialog
{
// Construction
public:
	CCreditsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCreditsDlg)
	enum { IDD = IDD_CREDITS };
	CEdit	m_ctlCreditsText;
	//}}AFX_DATA

  bool m_first_time;
  int  m_iResourceID;
  CString m_strTitle;

  CFont m_font;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreditsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreditsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSetfocusCreditsList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CREDITSDLG_H__71365423_C53B_11D2_A279_0000B4595568__INCLUDED_)
