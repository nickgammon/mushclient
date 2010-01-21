#if !defined(AFX_MAPMOVEDLG_H__3494CB78_61D5_11D4_BB13_0080AD7972EF__INCLUDED_)
#define AFX_MAPMOVEDLG_H__3494CB78_61D5_11D4_BB13_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MapMoveDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMapMoveDlg dialog

class CMapMoveDlg : public CDialog
{
// Construction
public:
	CMapMoveDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMapMoveDlg)
	enum { IDD = IDD_MAP_MOVE };
	CEdit	m_ctlReverse;
	CEdit	m_ctlAction;
	CString	m_strAction;
	CString	m_strReverse;
	BOOL	m_bSendToMUD;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapMoveDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMapMoveDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedAction(CCmdUI* pCmdUI);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPMOVEDLG_H__3494CB78_61D5_11D4_BB13_0080AD7972EF__INCLUDED_)
