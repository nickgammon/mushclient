// LuaChooseListMulti.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "LuaChooseListMulti.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLuaChooseListMulti dialog


CLuaChooseListMulti::CLuaChooseListMulti(CWnd* pParent /*=NULL*/)
	: CDialog(CLuaChooseListMulti::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLuaChooseListMulti)
	m_strMessage = _T("");
	//}}AFX_DATA_INIT
}


void CLuaChooseListMulti::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLuaChooseListMulti)
	DDX_Control(pDX, IDC_CHOOSE_LIST, m_ctlListBox);
	DDX_Text(pDX, IDC_CHOOSE_MSG, m_strMessage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLuaChooseListMulti, CDialog)
	//{{AFX_MSG_MAP(CLuaChooseListMulti)
	ON_NOTIFY(NM_DBLCLK, IDC_CHOOSE_LIST, OnDblclkChooseList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLuaChooseListMulti message handlers

BOOL CLuaChooseListMulti::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  SetWindowText (m_strTitle);	
	
  m_ctlListBox.InsertColumn(0, TranslateHeading ("Main column"), LVCFMT_LEFT, 340);

  m_ctlListBox.SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, 0, 
                        m_ctlListBox.SendMessage (LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) 
                                                  | LVS_EX_FULLROWSELECT);

  bool bHaveFocus = false;  // first selected item has the focus

  for (int i = 0; i < m_data.size (); i++)
    {
    int iPos = m_ctlListBox.InsertItem (i, m_data [i].sValue_.c_str ());

    if (iPos != -1)
      {
      m_ctlListBox.SetItemData (iPos, i);

      if (m_iDefaults.find (i) != m_iDefaults.end ())
        {
        int iGiveFocus = 0;
        if (!bHaveFocus)
          {
           iGiveFocus = LVIS_FOCUSED;
           bHaveFocus = true;
          }

        m_ctlListBox.SetItemState (iPos,
                                   iGiveFocus | LVIS_SELECTED,
                                   iGiveFocus | LVIS_SELECTED);
        }

      } // end of added OK
    }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLuaChooseListMulti::OnOK() 
{

// iterate through list remembering the item numbers of all selected items
for (int nItem = -1;
      (nItem = m_ctlListBox.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
  m_iResults.insert (m_ctlListBox.GetItemData (nItem));


	CDialog::OnOK();
	
}

void CLuaChooseListMulti::OnDblclkChooseList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnOK ();	
	
	*pResult = 0;
}
