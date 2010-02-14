#if !defined(AFX_HIGHLIGHTPHRASEDLG_H__9697FDCF_56B7_11D9_9957_00008C012785__INCLUDED_)
#define AFX_HIGHLIGHTPHRASEDLG_H__9697FDCF_56B7_11D9_9957_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HighlightPhraseDlg.h : header file
//

#include "world_prefs\ButtonColour.h"
#include "ColourComboBox.h"

/////////////////////////////////////////////////////////////////////////////
// CHighlightPhraseDlg dialog

class CHighlightPhraseDlg : public CDialog
{
// Construction
public:
	CHighlightPhraseDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CHighlightPhraseDlg)
	enum { IDD = IDD_HIGHLIGHT };
	CEdit	m_ctlText;
	CColourButton	m_ctlTriggerSwatchBgnd;
	CColourButton	m_ctlTriggerSwatchText;
	CColourComboBox	m_ctlTriggerColour;
	int		m_iColour;
	CString	m_strText;
	BOOL	m_bWord;
	BOOL	m_bMatchCase;
	//}}AFX_DATA

  // custom (user-defined) colours for triggers etc.
  COLORREF m_customtext [MAX_CUSTOM];
  COLORREF m_customback [MAX_CUSTOM];
  CString m_strCustomName [MAX_CUSTOM];
  COLORREF m_iOtherForeground;
  COLORREF m_iOtherBackground;

  COLORREF m_iOtherForeground_orig;
  COLORREF m_iOtherBackground_orig;

  void FixSwatch (CColourButton & m_ctlSwatch, 
                  CColourButton & m_ctlSwatch2,
                  int iNewColour);


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHighlightPhraseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CHighlightPhraseDlg)
	afx_msg void OnTriggerSwatchText();
	afx_msg void OnTriggerSwatchBgnd();
	afx_msg void OnSelchangeTriggerColour();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HIGHLIGHTPHRASEDLG_H__9697FDCF_56B7_11D9_9957_00008C012785__INCLUDED_)
