// LuaChooseBox.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "LuaChooseBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLuaChooseBox dialog


CLuaChooseBox::CLuaChooseBox(CWnd* pParent /*=NULL*/)
	: CDialog(CLuaChooseBox::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLuaChooseBox)
	m_strMessage = _T("");
	//}}AFX_DATA_INIT

  m_iDefault = -1;
  m_iResult = CB_ERR;
}


void CLuaChooseBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLuaChooseBox)
	DDX_Control(pDX, IDC_CHOOSE_COMBO, m_ctlCombo);
	DDX_Text(pDX, IDC_CHOOSE_MSG, m_strMessage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLuaChooseBox, CDialog)
	//{{AFX_MSG_MAP(CLuaChooseBox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLuaChooseBox message handlers

BOOL CLuaChooseBox::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  SetWindowText (m_strTitle);	
	
  for (int i = 0; i < m_data.size (); i++)
    {
    int iPos = m_ctlCombo.AddString (m_data [i].sValue_.c_str ());

    if (iPos != CB_ERR)
      {
      m_ctlCombo.SetItemData (iPos, i);
      if (i == m_iDefault)
        m_ctlCombo.SetCurSel (iPos);
      } // end of added OK
    }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLuaChooseBox::OnOK() 
{
  int iWhich = m_ctlCombo.GetCurSel ();

  if (iWhich != CB_ERR)
    m_iResult = m_ctlCombo.GetItemData (iWhich);
  
	CDialog::OnOK();
}
