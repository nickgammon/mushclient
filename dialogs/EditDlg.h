#if !defined(AFX_EDITDLG_H__2B40250A_6DE4_11D4_BB16_0080AD7972EF__INCLUDED_)
#define AFX_EDITDLG_H__2B40250A_6DE4_11D4_BB16_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditDlg dialog

class CEditDlg : public CDialog
{
// Construction
public:
	CEditDlg(CWnd* pParent = NULL);   // standard constructor

  ~CEditDlg () { delete m_font; };

// Dialog Data
	//{{AFX_DATA(CEditDlg)
	enum { IDD = IDD_EDIT };
	CButton	m_ctlRegexpButton;
	CButton	m_ctlOK;
	CButton	m_ctlCancel;
	CEdit	m_ctlText;
	CString	m_strText;
	//}}AFX_DATA

  CFont * m_font;

  CString m_strTitle;

  map<int, string> m_regexp_specials_map;
  map<int, int> m_char_offset;

  int m_next_item;

  bool m_bRegexp;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  void InsertRegexpItem (CMenu* pPopup, 
                         const string & sLabel, 
                         const string & sInsert,
                         const int iBack = 0);

	// Generated message map functions
	//{{AFX_MSG(CEditDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnRegexpButton();
	afx_msg void OnPopupHelp();
	//}}AFX_MSG
  afx_msg void OnRegexpMenu(UINT nID); 
	afx_msg void OnRemoveSelection();

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITDLG_H__2B40250A_6DE4_11D4_BB16_0080AD7972EF__INCLUDED_)
