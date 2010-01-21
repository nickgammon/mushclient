#if !defined(AFX_EDITMULTILINE_H__0219A5F6_A9BC_11D6_BBBE_0080AD7972EF__INCLUDED_)
#define AFX_EDITMULTILINE_H__0219A5F6_A9BC_11D6_BBBE_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditMultiLine.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditMultiLine dialog

class CEditMultiLine : public CDialog
{
// Construction
public:
	CEditMultiLine(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditMultiLine)
	enum { IDD = IDD_EDIT_MULTILINE };
	CButton	m_ctlGoToLineButton;
	CButton	m_ctlCompleteFunctionButton;
	CButton	m_ctlFunctionListButton;
	CButton	m_ctlOK;
	CButton	m_ctlCancel;
	CEdit	m_ctlText;
	CString	m_strText;
	//}}AFX_DATA

  CFont * m_font;

  CString m_strTitle;

  bool m_bScript;
  bool m_bLua;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditMultiLine)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditMultiLine)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnFunctionList();
	afx_msg void OnCompleteWord();
	afx_msg void OnDestroy();
	afx_msg void OnGotoLine();
	//}}AFX_MSG
	afx_msg void OnRemoveSelection();
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITMULTILINE_H__0219A5F6_A9BC_11D6_BBBE_0080AD7972EF__INCLUDED_)
