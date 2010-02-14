// CGlobalPrefsSheet.h : header file
//
// This class defines custom modal property sheet 
// CGlobalPrefsSheet.
 
#ifndef __CGlobalPREFSSHEET_H__
#define __CGlobalPREFSSHEET_H__

#include "GlobalPrefs.h"
#include "..\..\MUSHclient.h"
#include "..\..\mainfrm.h"
#include "..\..\doc.h"
#include "..\world_prefs\genpropertypage.h"

/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsSheet

class CGlobalPrefsSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CGlobalPrefsSheet)

// Construction
public:
	CGlobalPrefsSheet(CWnd* pWndParent = NULL);

// Attributes
public:
	CGlobalPrefsP1 m_Page1;
	CGlobalPrefsP2 m_Page2;
	CGlobalPrefsP3 m_Page3;
	CGlobalPrefsP4 m_Page4;
	CGlobalPrefsP5 m_Page5;
	CGlobalPrefsP6 m_Page6;
	CGlobalPrefsP7 m_Page7;
//	CMudListDlg m_Page8;
	CGlobalPrefsP9 m_Page9;
	CGlobalPrefsP10 m_Page10;
	CGlobalPrefsP11 m_Page11;
	CGlobalPrefsP12 m_Page12;
	CGlobalPrefsP13 m_Page13;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGlobalPrefsSheet)
	public:
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGlobalPrefsSheet();

// Generated message map functions
protected:
	//{{AFX_MSG(CGlobalPrefsSheet)
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif	// __CGlobalPREFSSHEET_H__
