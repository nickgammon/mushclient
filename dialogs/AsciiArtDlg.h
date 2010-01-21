#if !defined(AFX_ASCIIARTDLG_H__2B40250B_6DE4_11D4_BB16_0080AD7972EF__INCLUDED_)
#define AFX_ASCIIARTDLG_H__2B40250B_6DE4_11D4_BB16_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AsciiArtDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAsciiArtDlg dialog

class CAsciiArtDlg : public CDialog
{
// Construction
public:
	CAsciiArtDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAsciiArtDlg)
	enum { IDD = IDD_ASCII_ART };
	CEdit	m_ctlText;
	CEdit	m_ctlFont;
	CString	m_strText;
	CString	m_strFont;
	int		m_iLayout;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAsciiArtDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAsciiArtDlg)
	afx_msg void OnBrowse();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedThings(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ASCIIARTDLG_H__2B40250B_6DE4_11D4_BB16_0080AD7972EF__INCLUDED_)
