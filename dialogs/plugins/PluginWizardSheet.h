// PluginWizardSheet.h : header file
//
// This class defines custom modal property sheet 
// CPluginWizardSheet.
 
#ifndef __PLUGINWIZARDSHEET_H__
#define __PLUGINWIZARDSHEET_H__

#include "..\..\MUSHclient.h"
#include "..\..\mainfrm.h"
#include "..\..\doc.h"
#include "PluginWizard.h"

/////////////////////////////////////////////////////////////////////////////
// CPluginWizardSheet

class CPluginWizardSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CPluginWizardSheet)

// Construction
public:
	CPluginWizardSheet(CWnd* pWndParent = NULL);

// Attributes
public:
	CPluginWizardPage1 m_Page1;
	CPluginWizardPage2 m_Page2;
	CPluginWizardPage3 m_Page3;
	CPluginWizardPage4 m_Page4;
	CPluginWizardPage5 m_Page5;
	CPluginWizardPage6 m_Page6;
	CPluginWizardPage7 m_Page7;
	CPluginWizardPage8 m_Page8;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPluginWizardSheet)
	public:
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPluginWizardSheet();

// Generated message map functions
protected:
	//{{AFX_MSG(CPluginWizardSheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif	// __PLUGINWIZARDSHEET_H__
