// PluginWizardSheet.cpp : implementation file
//

#include "stdafx.h"
#include "PluginWizardSheet.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPluginWizardSheet

IMPLEMENT_DYNAMIC(CPluginWizardSheet, CPropertySheet)

CPluginWizardSheet::CPluginWizardSheet(CWnd* pWndParent)
	 : CPropertySheet(IDS_PROPSHT_CAPTION2, pWndParent)
{
	// Add all of the property pages here.  Note that
	// the order that they appear in here will be
	// the order they appear in on screen.  By default,
	// the first page of the set is the active one.
	// One way to make a different property page the 
	// active one is to call SetActivePage().

	AddPage(&m_Page1);
	AddPage(&m_Page2);
	AddPage(&m_Page3);
	AddPage(&m_Page4);
	AddPage(&m_Page5);
	AddPage(&m_Page6);
	AddPage(&m_Page7);
	AddPage(&m_Page8);

  // in case we forget to change them ...

  m_Page1.m_doc = NULL;
  m_Page1.m_pPage2 = &m_Page2;
  m_Page2.m_doc = NULL;
  m_Page3.m_doc = NULL;
  m_Page4.m_doc = NULL;
  m_Page5.m_doc = NULL;
  m_Page6.m_doc = NULL;
  m_Page7.m_doc = NULL;
  m_Page8.m_doc = NULL;

}

CPluginWizardSheet::~CPluginWizardSheet()
{
}


BEGIN_MESSAGE_MAP(CPluginWizardSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CPluginWizardSheet)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPluginWizardSheet message handlers



BOOL CPluginWizardSheet::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	
  // remove apply button
  CWnd* pTemp;
  pTemp = GetDlgItem(ID_APPLY_NOW);
  if (pTemp)
      pTemp->ShowWindow (SW_HIDE);
           
  // OK is a bit wishy-washy, make it clear that clicking this button
  // creates the plugin (once you specify a file name)
  pTemp = GetDlgItem(IDOK);
  if (pTemp)
      pTemp->SetWindowText ("&Create...");

	return bResult;
}


LRESULT CPluginWizardSheet::OnKickIdle(WPARAM, LPARAM)
  {
  GetActivePage ()->SendMessage (WM_KICKIDLE, 0, 0);
  return 0;
  } // end of CPluginWizardSheet::OnKickIdle
