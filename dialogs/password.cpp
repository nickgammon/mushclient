// password.cpp : implementation file
//

#include "stdafx.h"
#include "..\MUSHclient.h"
#include "password.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPasswordDialog dialog


CPasswordDialog::CPasswordDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPasswordDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPasswordDialog)
	m_character = _T("");
	m_password = _T("");
	//}}AFX_DATA_INIT
}


void CPasswordDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPasswordDialog)
	DDX_Text(pDX, IDC_CHARACTER, m_character);
	DDX_Text(pDX, IDC_PASSWORD, m_password);
	//}}AFX_DATA_MAP

   if(pDX->m_bSaveAndValidate)
   {
      m_password.TrimLeft ();
      m_password.TrimRight ();
      if(m_password.IsEmpty ())
          {
          ::TMessageBox("Your password cannot be blank.");
          DDX_Text(pDX, IDC_PASSWORD, m_password);
          pDX->Fail();
          }     // end of password being blank
  
  } // end of saving and validating

}


BEGIN_MESSAGE_MAP(CPasswordDialog, CDialog)
	//{{AFX_MSG_MAP(CPasswordDialog)
	ON_BN_CLICKED(IDC_HELPBUTTON, OnHelpbutton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPasswordDialog message handlers

void CPasswordDialog::OnHelpbutton() 
{
//	  OnCmdMsg(ID_HELP, CN_COMMAND, NULL, NULL);	
	App.WinHelp(m_nIDHelp + HID_BASE_RESOURCE);
}
