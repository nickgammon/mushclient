// LuaChooseList.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "LuaChooseList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLuaChooseList dialog


CLuaChooseList::CLuaChooseList(CWnd* pParent /*=NULL*/)
	: CDialog(CLuaChooseList::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLuaChooseList)
	m_strMessage = _T("");
	//}}AFX_DATA_INIT

  m_iDefault = -1;
  m_iResult = CB_ERR;

}


void CLuaChooseList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLuaChooseList)
	DDX_Control(pDX, IDC_CHOOSE_LIST, m_ctlListBox);
	DDX_Text(pDX, IDC_CHOOSE_MSG, m_strMessage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLuaChooseList, CDialog)
	//{{AFX_MSG_MAP(CLuaChooseList)
	ON_NOTIFY(NM_DBLCLK, IDC_CHOOSE_LIST, OnDblclkChooseList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLuaChooseList message handlers

BOOL CLuaChooseList::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  SetWindowText (m_strTitle);	
	
  m_ctlListBox.InsertColumn(0, TranslateHeading ("Main column"), LVCFMT_LEFT, 340);

  m_ctlListBox.SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, 0, 
                        m_ctlListBox.SendMessage (LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) 
                                                  | LVS_EX_FULLROWSELECT);

  for (int i = 0; i < m_data.size (); i++)
    {
    int iPos = m_ctlListBox.InsertItem (i, m_data [i].sValue_.c_str ());

    if (iPos != -1)
      {
      m_ctlListBox.SetItemData (iPos, i);
      if (i == m_iDefault)
         m_ctlListBox.SetItemState (iPos,
                                    LVIS_FOCUSED | LVIS_SELECTED,
                                    LVIS_FOCUSED | LVIS_SELECTED);

      } // end of added OK
    }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLuaChooseList::OnOK() 
{

  int iWhich = m_ctlListBox.GetNextItem(-1, LVNI_SELECTED);

  if (iWhich != -1)
    m_iResult = m_ctlListBox.GetItemData (iWhich);
	
	CDialog::OnOK();
}

void CLuaChooseList::OnDblclkChooseList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnOK ();	
	*pResult = 0;
}
