// LuaInputBox.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "LuaInputBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLuaInputBox dialog

#define CLEAR_SELECTION 10200

CLuaInputBox::CLuaInputBox(CWnd* pParent /*=NULL*/)
	: CDialog(CLuaInputBox::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLuaInputBox)
	m_strMessage = _T("");
	m_strReply = _T("");
	//}}AFX_DATA_INIT

  m_font = NULL;

}


void CLuaInputBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLuaInputBox)
	DDX_Control(pDX, IDC_INPUT_BOX_REPLY, m_ctlReply);
	DDX_Text(pDX, IDC_INPUT_BOX_MESSAGE, m_strMessage);
	DDX_Text(pDX, IDC_INPUT_BOX_REPLY, m_strReply);
	//}}AFX_DATA_MAP

 if(!pDX->m_bSaveAndValidate)
   {
   if (!m_strFont.IsEmpty () && m_iFontSize > 3)
      FixFont (m_font, m_ctlReply, m_strFont, m_iFontSize, FW_NORMAL, DEFAULT_CHARSET);
   }

}


BEGIN_MESSAGE_MAP(CLuaInputBox, CDialog)
	//{{AFX_MSG_MAP(CLuaInputBox)
	//}}AFX_MSG_MAP

  ON_COMMAND(CLEAR_SELECTION, OnRemoveSelection)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLuaInputBox message handlers

BOOL CLuaInputBox::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  SetWindowText (m_strTitle);	
	
  PostMessage (WM_COMMAND, CLEAR_SELECTION);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLuaInputBox::OnRemoveSelection()
  {

//  m_ctlReply.SetSel (m_strReply.GetLength (), m_strReply.GetLength ());

  }
