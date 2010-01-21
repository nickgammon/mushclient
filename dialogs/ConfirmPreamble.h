#if !defined(AFX_CONFIRMPREAMBLE_H__5AC45117_A701_11D0_8EA1_00A0247B3BFD__INCLUDED_)
#define AFX_CONFIRMPREAMBLE_H__5AC45117_A701_11D0_8EA1_00A0247B3BFD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ConfirmPreamble.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfirmPreamble dialog

class CConfirmPreamble : public CDialog
{
// Construction
public:
	CConfirmPreamble(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfirmPreamble)
	enum { IDD = IDD_PASTE_PREAMBLES };
	CString	m_strPasteMessage;
	CString	m_strPostamble;
	CString	m_strPreamble;
	CString	m_strLinePostamble;
	CString	m_strLinePreamble;
	BOOL	m_bCommentedSoftcode;
	long	m_iLineDelay;
	BOOL	m_bEcho;
	long	m_nLineDelayPerLines;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfirmPreamble)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfirmPreamble)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIRMPREAMBLE_H__5AC45117_A701_11D0_8EA1_00A0247B3BFD__INCLUDED_)
