#if !defined(AFX_COMPLETEWORDDLG_H__802B86BC_824C_11DA_996C_00008C012785__INCLUDED_)
#define AFX_COMPLETEWORDDLG_H__802B86BC_824C_11DA_996C_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CompleteWordDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCompleteWordDlg dialog

class CCompleteWordDlg : public CDialog
{
// Construction
public:
	CCompleteWordDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCompleteWordDlg)
	enum { IDD = IDD_COMPLETE_WORD };
	CListBox	m_ctlFunctions;
	//}}AFX_DATA

  const char * m_sFunctionsPtr;
  int m_nFunctions;
  CString m_strResult;
	CString	m_strFilter;
  CPoint m_pt;  // where to put it

  bool m_bLua;
  bool m_bFunctions;
  set<string> * m_extraItems;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCompleteWordDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  void ReloadList ();
  virtual void OnOK();

	// Generated message map functions
	//{{AFX_MSG(CCompleteWordDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkWordList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMPLETEWORDDLG_H__802B86BC_824C_11DA_996C_00008C012785__INCLUDED_)
