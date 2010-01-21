#if !defined(AFX_MULTILINETRIGGERDLG_H__A3149338_56DC_11D9_9957_00008C012785__INCLUDED_)
#define AFX_MULTILINETRIGGERDLG_H__A3149338_56DC_11D9_9957_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MultiLineTriggerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMultiLineTriggerDlg dialog

class CMultiLineTriggerDlg : public CDialog
{
// Construction
public:
	CMultiLineTriggerDlg(CWnd* pParent = NULL);   // standard constructor

  ~CMultiLineTriggerDlg () { delete m_font; };

// Dialog Data
	//{{AFX_DATA(CMultiLineTriggerDlg)
	enum { IDD = IDD_MULTILINE_TRIGGER };
	CEdit	m_ctlTriggerText;
	CString	m_strTriggerText;
	BOOL	m_bMatchCase;
	//}}AFX_DATA

  CFont * m_font;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiLineTriggerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMultiLineTriggerDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	afx_msg void OnRemoveSelection();
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTILINETRIGGERDLG_H__A3149338_56DC_11D9_9957_00008C012785__INCLUDED_)
