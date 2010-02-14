// PrefsPropertySheet.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\mushclient.h"
#include "PrefsPropertySheet.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPrefsPropertySheet

IMPLEMENT_DYNAMIC(CPrefsPropertySheet, CPropertySheet)

CPrefsPropertySheet::CPrefsPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CPrefsPropertySheet::CPrefsPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CPrefsPropertySheet::~CPrefsPropertySheet()
{
}


BEGIN_MESSAGE_MAP(CPrefsPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CPrefsPropertySheet)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsPropertySheet message handlers

LRESULT CPrefsPropertySheet::OnKickIdle(WPARAM, LPARAM)
  {
  GetActivePage ()->SendMessage (WM_KICKIDLE, 0, 0);
  return 0;
  } // end of CPrefsPropertySheet::OnKickIdle

BOOL CPrefsPropertySheet::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	
  // remove apply button
  CWnd* pTemp;
  pTemp = GetDlgItem(ID_APPLY_NOW);
  if (pTemp)
      pTemp->ShowWindow (SW_HIDE);

	return bResult;
}


