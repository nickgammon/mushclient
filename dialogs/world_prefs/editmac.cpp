// editmac.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "editmac.h"
#include "..\EditMultiLine.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditMacro dialog


CEditMacro::CEditMacro(CWnd* pParent /*=NULL*/)
	: CDialog(CEditMacro::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditMacro)
	m_macro = _T("");
	m_macro_type = -1;
	m_macro_desc = _T("");
	//}}AFX_DATA_INIT
}


void CEditMacro::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditMacro)
	DDX_Control(pDX, IDC_MACRO_TEXT, m_ctlSend);
	DDX_Text(pDX, IDC_MACRO_TEXT, m_macro);
	DDX_Radio(pDX, IDC_SEND_TYPE, m_macro_type);
	DDX_Text(pDX, IDC_MACRO_TYPE, m_macro_desc);
	//}}AFX_DATA_MAP

}


BEGIN_MESSAGE_MAP(CEditMacro, CDialog)
	//{{AFX_MSG_MAP(CEditMacro)
	ON_BN_CLICKED(IDC_HELPBUTTON, OnHelpbutton)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CEditMacro message handlers

void CEditMacro::OnHelpbutton() 
{
//	  OnCmdMsg(ID_HELP, CN_COMMAND, NULL, NULL);	
	App.WinHelp(m_nIDHelp + HID_BASE_RESOURCE);
}

void CEditMacro::OnEdit() 
{
CEditMultiLine dlg;

  dlg.m_strText = GetText (m_ctlSend);

  dlg.m_strTitle = "Edit macro 'send' text for " + m_macro_desc;

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlSend.SetWindowText (dlg.m_strText);
	
}

BOOL CEditMacro::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  if (!m_strTitle.IsEmpty ())
    SetWindowText (m_strTitle);	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
