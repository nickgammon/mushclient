// aliasdlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAliasDlg dialog

class CAliasDlg : public CDialog
{
	DECLARE_DYNAMIC(CAliasDlg)

    // Construction
public:
	CAliasDlg(CWnd* pParent = NULL);   // standard constructor

  ~CAliasDlg () { delete m_font; delete m_font2; };

// Dialog Data
	//{{AFX_DATA(CAliasDlg)
	enum { IDD = IDD_EDIT_ALIAS };
	CComboBox	m_ctlSendTo;
	CEdit	m_ctlVariable;
	CButton	m_ctlRegexp;
	CEdit	m_ctlAliasContents;
	CEdit	m_ctlAliasName;
	CString	m_contents;
	CString	m_name;
	CString	m_strLabel;
	BOOL	m_bEnabled;
	CString	m_strProcedure;
	BOOL	m_bIgnoreCase;
	BOOL	m_bExpandVariables;
	BOOL	m_bOmitFromLogFile;
	BOOL	m_bRegexp;
	BOOL	m_bOmitFromOutput;
	BOOL	m_bMenu;
	BOOL	m_bTemporary;
	CString	m_strGroup;
	CString	m_strVariable;
	int		m_iSendTo;
	BOOL	m_bKeepEvaluating;
	short	m_iSequence;
	BOOL	m_bEchoAlias;
	BOOL	m_bOmitFromCommandHistory;
	CString	m_strIncluded;
	CString	m_strRegexpError;
	BOOL	m_bOneShot;
	//}}AFX_DATA

  CAliasMap * m_pAliasMap;
  CAlias * m_current_alias;

  CFont * m_font;
  CFont * m_font2;
  CMUSHclientDoc * m_pDoc;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAliasDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAliasDlg)
	afx_msg void OnHelpbutton();
	afx_msg void OnReverse();
	afx_msg void OnEdit2();
	afx_msg void OnConvertToRegexp();
	afx_msg void OnEditSend();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateOK(CCmdUI* pCmdUI);
  afx_msg void OnUpdateInvocationCount(CCmdUI* pCmdUI);
  afx_msg void OnUpdateMatchCount(CCmdUI* pCmdUI);
  afx_msg void OnUpdateProcedure(CCmdUI* pCmdUI);
  afx_msg void OnUpdateReverse(CCmdUI* pCmdUI);
//  afx_msg void OnUpdateDelayed(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedNoRegexp(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
