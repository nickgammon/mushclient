#if !defined(AFX_TABDEFAULTSDLG_H__9F3FE613_59C2_11D3_A6C9_0000B4595568__INCLUDED_)
#define AFX_TABDEFAULTSDLG_H__9F3FE613_59C2_11D3_A6C9_0000B4595568__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabDefaultsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTabDefaultsDlg dialog

class CTabDefaultsDlg : public CDialog
{
// Construction
public:
	CTabDefaultsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTabDefaultsDlg)
	enum { IDD = IDD_TAB_COMPLETION_DEFAULTS_DLG };
	CString	m_strWordList;
	UINT	m_iTabCompletionLines;
	BOOL	m_bTabCompletionSpace;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabDefaultsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTabDefaultsDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABDEFAULTSDLG_H__9F3FE613_59C2_11D3_A6C9_0000B4595568__INCLUDED_)
