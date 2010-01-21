#if !defined(AFX_SPELLCHECKDLG_H__EC26C143_568A_11DB_9984_00008C012785__INCLUDED_)
#define AFX_SPELLCHECKDLG_H__EC26C143_568A_11DB_9984_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpellCheckDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpellCheckDlg dialog

class CSpellCheckDlg : public CDialog
{
// Construction
public:
	CSpellCheckDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSpellCheckDlg)
	enum { IDD = IDD_SPELLCHECK_DIALOG };
	CEdit	m_ctlMisspeltWord;
	CListBox	m_ctlSuggestions;
	CString	m_strMisspeltWord;
	//}}AFX_DATA

  vector<string> m_suggestions;

  CString m_strOriginalWord;

  string m_sAction;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellCheckDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	virtual void OnOK();

	// Generated message map functions
	//{{AFX_MSG(CSpellCheckDlg)
	afx_msg void OnIgnore();
	afx_msg void OnIgnoreAll();
	afx_msg void OnAdd();
	afx_msg void OnChange();
	afx_msg void OnChangeAll();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkSuggestions();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPELLCHECKDLG_H__EC26C143_568A_11DB_9984_00008C012785__INCLUDED_)
