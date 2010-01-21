#if !defined(AFX_MYSPLITTERWND_H__8731F757_5508_11D3_A6C8_0000B4595568__INCLUDED_)
#define AFX_MYSPLITTERWND_H__8731F757_5508_11D3_A6C8_0000B4595568__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MySplitterWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMySplitterWnd window

class CMySplitterWnd : public CSplitterWnd
{
// Construction
public:
	CMySplitterWnd();

// Attributes
public:

  CMUSHclientDoc * m_pDoc;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMySplitterWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMySplitterWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMySplitterWnd)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYSPLITTERWND_H__8731F757_5508_11D3_A6C8_0000B4595568__INCLUDED_)
