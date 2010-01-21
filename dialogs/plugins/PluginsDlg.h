#if !defined(AFX_PLUGINSDLG_H__51112DD6_738E_11D6_BBB9_0080AD7972EF__INCLUDED_)
#define AFX_PLUGINSDLG_H__51112DD6_738E_11D6_BBB9_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PluginsDlg.h : header file
//

  enum 
  { eColumnName,
    eColumnPurpose,
    eColumnAuthor,
    eColumnLanguage,
    eColumnFile,
    eColumnEnabled,
    eColumnVersion,
    eColumnCount      // this must be last!
  };


/////////////////////////////////////////////////////////////////////////////
// CPluginsDlg dialog

class CPluginsDlg : public CDialog
{
// Construction
public:
	CPluginsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPluginsDlg)
	enum { IDD = IDD_PLUGINS };
	CButton	m_ctlDisable;
	CButton	m_ctlEnable;
	CButton	m_ctlCancel;
	CButton	m_ctlShowDescription;
	CButton	m_ctlReload;
	CButton	m_ctlEdit;
	CButton	m_ctlDelete;
	CButton	m_ctlAdd;
	CListCtrl	m_ctlPluginList;
	//}}AFX_DATA

  CMUSHclientDoc * m_pDoc;

  // for sorting the list

  int m_last_col;
  BOOL m_reverse;

  void LoadList (void);
  void EditPlugin (const CString strName);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPluginsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPluginsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnColumnclickPluginsList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowDescription();
	afx_msg void OnAddPlugin();
	afx_msg void OnDeletePlugin();
	afx_msg void OnReload();
	afx_msg void OnDestroy();
	afx_msg void OnEdit();
	afx_msg void OnDblclkPluginsList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRdblclkPluginsList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnable();
	afx_msg void OnDisable();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedSelection(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedDescription(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLUGINSDLG_H__51112DD6_738E_11D6_BBB9_0080AD7972EF__INCLUDED_)
