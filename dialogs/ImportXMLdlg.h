#if !defined(AFX_IMPORTXMLDLG_H__346ECA36_684E_11D6_BBB6_0080AD7972EF__INCLUDED_)
#define AFX_IMPORTXMLDLG_H__346ECA36_684E_11D6_BBB6_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImportXMLdlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImportXMLdlg dialog

class CImportXMLdlg : public CDialog
{
// Construction
public:
	CImportXMLdlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CImportXMLdlg)
	enum { IDD = IDD_IMPORT_XML };
	CButton	m_ctlPrinting;
	CButton	m_ctlKeypad;
	CButton	m_ctlColours;
	CButton	m_ctlVariables;
	CButton	m_ctlMacros;
	CButton	m_ctlTimers;
	CButton	m_ctlAliases;
	CButton	m_ctlTriggers;
	CButton	m_ctlGeneral;
	BOOL	m_bGeneral;
	BOOL	m_bTriggers;
	BOOL	m_bAliases;
	BOOL	m_bTimers;
	BOOL	m_bMacros;
	BOOL	m_bVariables;
	BOOL	m_bColours;
	BOOL	m_bKeypad;
	BOOL	m_bPrinting;
	//}}AFX_DATA

  CMUSHclientDoc * m_pDoc;

  void ImportArchive (CArchive & ar);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImportXMLdlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CImportXMLdlg)
	afx_msg void OnOk();
	afx_msg void OnClipboard();
	afx_msg void OnPluginsList();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedXMLClipboard(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPORTXMLDLG_H__346ECA36_684E_11D6_BBB6_0080AD7972EF__INCLUDED_)
