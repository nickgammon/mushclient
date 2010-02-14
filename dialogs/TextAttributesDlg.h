#if !defined(AFX_TEXTATTRIBUTESDLG_H__CF599416_3B2F_11D4_BB11_0080AD7972EF__INCLUDED_)
#define AFX_TEXTATTRIBUTESDLG_H__CF599416_3B2F_11D4_BB11_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextAttributesDlg.h : header file
//

#include "world_prefs\ButtonColour.h"

/////////////////////////////////////////////////////////////////////////////
// CTextAttributesDlg dialog

class CMUSHclientDoc;
class CLine;

class CTextAttributesDlg : public CDialog
{
// Construction
public:
	CTextAttributesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTextAttributesDlg)
	enum { IDD = IDD_TEXT_ATTRIBUTES };
	CColourButton	m_ctlTextSwatch;
	CColourButton	m_ctlBackgroundSwatch;
	CString	m_strBackColour;
	CString	m_strLetter;
	BOOL	m_bBold;
	BOOL	m_bInverse;
	BOOL	m_bItalic;
	CString	m_strModified;
	CString	m_strTextColour;
	CString	m_strCustomColour;
	CString	m_strTextColourRGB;
	CString	m_strBackgroundColourRGB;
	//}}AFX_DATA

  CMUSHclientDoc * m_pDoc;
  int m_iLine;
  CLine * m_pLine; 


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextAttributesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTextAttributesDlg)
	afx_msg void OnLineInfo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTATTRIBUTESDLG_H__CF599416_3B2F_11D4_BB11_0080AD7972EF__INCLUDED_)
