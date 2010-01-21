#if !defined(AFX_RECALLDLG_H__3414F5A6_2628_11D4_BB0D_0080AD7972EF__INCLUDED_)
#define AFX_RECALLDLG_H__3414F5A6_2628_11D4_BB0D_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RecallDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRecallDlg dialog

class CRecallDlg : public CDialog
{
// Construction
public:
	CRecallDlg(COLORREF back_colour,
             CWnd* pParent = NULL);   // standard constructor

  ~CRecallDlg () { delete m_font; };

// Dialog Data
	//{{AFX_DATA(CRecallDlg)
	enum { IDD = ID_RECALL_WINDOW };
	CEdit	m_ctlText;
	CString	m_strText;
	//}}AFX_DATA


  CFont * m_font;

  bool m_first_time;

  CString m_strTitle;

  CString m_strFilename;

  BOOL m_bReadOnly;

  CString m_strFontName;
  LONG     m_iFontSize;
  LONG     m_iFontWeight;
  DWORD    m_iFontCharset;
  COLORREF m_textColour;
  COLORREF m_backColour;


  // Add a CBrush* to store the new background brush for edit controls.
  CBrush* m_pEditBkBrush;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecallDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

  bool SaveModified (const bool bSaveAs);
  virtual void OnOK (void);
  virtual void OnCancel (void);

	// Generated message map functions
	//{{AFX_MSG(CRecallDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	afx_msg void OnSetfocusText();
	//}}AFX_MSG
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECALLDLG_H__3414F5A6_2628_11D4_BB0D_0080AD7972EF__INCLUDED_)
