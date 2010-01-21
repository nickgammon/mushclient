#if !defined(AFX_DEBUGWORLDINPUTDLG_H__7FB4E348_5865_11D5_BB6E_0080AD7972EF__INCLUDED_)
#define AFX_DEBUGWORLDINPUTDLG_H__7FB4E348_5865_11D5_BB6E_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DebugWorldInputDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDebugWorldInputDlg dialog

class CDebugWorldInputDlg : public CDialog
{
// Construction
public:
	CDebugWorldInputDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDebugWorldInputDlg)
	enum { IDD = IDD_DEBUG_INPUT };
	CComboBox	m_ctlSpecials;
	CEdit	m_ctlText;
	CString	m_strText;
	//}}AFX_DATA


  void AddAnsiSpecial (const char * sName, const int iCode);
  void AddMXPSpecial (const char * sName, const int iCode);
  void AddOtherSpecial (const char * sName, const int iCode);

  CFont m_font;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDebugWorldInputDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDebugWorldInputDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSpecial();
	afx_msg void OnInsertUnicode();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedSelection(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEBUGWORLDINPUTDLG_H__7FB4E348_5865_11D5_BB6E_0080AD7972EF__INCLUDED_)
