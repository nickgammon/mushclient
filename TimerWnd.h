#if !defined(AFX_TIMERWND_H__126EF919_6B46_11D4_BB14_0080AD7972EF__INCLUDED_)
#define AFX_TIMERWND_H__126EF919_6B46_11D4_BB14_0080AD7972EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TimerWnd.h : header file
//

class CMUSHclientDoc;

/////////////////////////////////////////////////////////////////////////////
// CTimerWnd window

class CTimerWnd : public CWnd
{
// Construction
public:
	CTimerWnd(CMUSHclientDoc * pDoc);

// Attributes
public:

  CMUSHclientDoc * m_pDoc;
  int m_iTimer;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTimerWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTimerWnd();

  void ChangeTimerRate (const int iRate);

	// Generated message map functions
protected:
	//{{AFX_MSG(CTimerWnd)
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TIMERWND_H__126EF919_6B46_11D4_BB14_0080AD7972EF__INCLUDED_)
