#if !defined(AFX_MAPDLG_H__3494CB77_61D5_11D4_BB13_0080AD7972EF__INCLUDED_)
#define AFX_MAPDLG_H__3494CB77_61D5_11D4_BB13_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MapDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMapDlg dialog

class CMapDlg : public CDialog
{
// Construction
public:
	CMapDlg(CWnd* pParent = NULL);   // standard constructor

  ~CMapDlg () { delete m_font; };

// Dialog Data
	//{{AFX_DATA(CMapDlg)
	enum { IDD = IDD_MAPPER };
	CButton	m_ctlRegexp;
	CEdit	m_ctlFailure;
	CEdit	m_ctlForwards;
	CEdit	m_ctlBackwards;
	BOOL	m_bEnable;
	BOOL	m_bRemoveMapReverses;
	CString	m_strMappingFailure;
	BOOL	m_bMapFailureRegexp;
	//}}AFX_DATA

  CMUSHclientDoc * m_pDoc;
  int m_iItemCount;
  CFont * m_font;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  void CalculateSpeedWalks (void);
  void AddAlias (CString strDirection);

	// Generated message map functions
	//{{AFX_MSG(CMapDlg)
	afx_msg void OnRemoveAll();
	afx_msg void OnRemoveLast();
	virtual BOOL OnInitDialog();
	afx_msg void OnSpecialMove();
	afx_msg void OnMakeAlias();
	afx_msg void OnMakeReverseAlias();
	afx_msg void OnTakeReverse();
	afx_msg void OnEdit();
	afx_msg void OnConvertToRegexp();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedEntries(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedNoRegexp(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPDLG_H__3494CB77_61D5_11D4_BB13_0080AD7972EF__INCLUDED_)
