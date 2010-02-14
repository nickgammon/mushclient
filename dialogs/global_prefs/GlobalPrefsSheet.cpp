// CGlobalPrefsSheet.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\resource.h"
#include "GlobalPrefsSheet.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsSheet

IMPLEMENT_DYNAMIC(CGlobalPrefsSheet, CPropertySheet)

CGlobalPrefsSheet::CGlobalPrefsSheet(CWnd* pWndParent)
	 : CPropertySheet(IDS_PROPSHT_CAPTION1, pWndParent)
{

	AddPage(&m_Page1);
	AddPage(&m_Page2);
	AddPage(&m_Page3);
	AddPage(&m_Page4);
	AddPage(&m_Page5);
	AddPage(&m_Page6);
	AddPage(&m_Page7);
//	AddPage(&m_Page8);
	AddPage(&m_Page9);
	AddPage(&m_Page10);
	AddPage(&m_Page11);
	AddPage(&m_Page12);
	AddPage(&m_Page13);
  
  m_Page1.m_pCurrentDoc = NULL;

}

CGlobalPrefsSheet::~CGlobalPrefsSheet()
{
}


BEGIN_MESSAGE_MAP(CGlobalPrefsSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CGlobalPrefsSheet)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsSheet message handlers


LRESULT CGlobalPrefsSheet::OnKickIdle(WPARAM, LPARAM)
  {
  GetActivePage ()->SendMessage (WM_KICKIDLE, 0, 0);
  return 0;
  } // end of CGlobalPrefsSheet::OnKickIdle

BOOL CGlobalPrefsSheet::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	
  // remove apply button
  CWnd* pTemp;
  pTemp = GetDlgItem(ID_APPLY_NOW);
  if (pTemp)
      pTemp->ShowWindow (SW_HIDE);
	
	return bResult;
}

