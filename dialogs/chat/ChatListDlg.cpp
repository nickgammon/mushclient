// ChatListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\mushclient.h"
#include "..\..\doc.h"
#include "ChatListDlg.h"

#include "ChatCallDlg.h"
#include "ChatToAllDlg.h"
#include "EmoteToAllDlg.h"
#include "ChatDetailsDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef struct t_chat_sort_param
  {  
  // initialise via constructor - makes sure we don't leave something out!
  t_chat_sort_param (const CMUSHclientDoc * arg_pDoc,
                    const int arg_sortkey,
                    const int arg_reverse) :
                    pDoc (arg_pDoc),
                    sortkey (arg_sortkey),
                    reverse (arg_reverse) {};

  const CMUSHclientDoc * pDoc;        // document
  const int        sortkey;           // which key to use
  const int        reverse;           // reverse sort or not
  }   t_chat_sort_param;

/////////////////////////////////////////////////////////////////////////////
// CChatListDlg dialog


CChatListDlg::CChatListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChatListDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChatListDlg)
	m_strStatus = _T("");
	//}}AFX_DATA_INIT

  m_pDoc = NULL;

// default to sorting in name order

  m_last_col = eColumnChatName;
  m_reverse = FALSE;

}


void CChatListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChatListDlg)
	DDX_Control(pDX, IDC_CHAT_SESSIONS, m_ctlChatList);
	DDX_Text(pDX, IDC_STATUS, m_strStatus);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChatListDlg, CDialog)
	//{{AFX_MSG_MAP(CChatListDlg)
	ON_BN_CLICKED(IDC_CALL, OnCall)
	ON_BN_CLICKED(IDC_CHAT_ALL, OnChatAll)
	ON_BN_CLICKED(IDC_EMOTE_ALL, OnEmoteAll)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_BN_CLICKED(IDC_PING, OnPing)
	ON_BN_CLICKED(IDC_SNOOP, OnSnoop)
	ON_BN_CLICKED(IDC_SENDFILE, OnSendfile)
	ON_BN_CLICKED(IDC_HANGUP, OnHangup)
	ON_BN_CLICKED(IDC_CHAT, OnChat)
	ON_BN_CLICKED(IDC_EMOTE, OnEmote)
	ON_BN_CLICKED(IDC_PEEK, OnPeek)
	ON_BN_CLICKED(IDC_REQUEST, OnRequest)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_DBLCLK, IDC_CHAT_SESSIONS, OnDblclkChatSessions)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_CHAT_SESSIONS, OnColumnclickChatSessions)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_EDIT, OnUpdateNeedOneSelection)
  ON_UPDATE_COMMAND_UI(IDC_PING, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_SNOOP, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_SENDFILE, OnUpdateNeedOneSelection)
  ON_UPDATE_COMMAND_UI(IDC_HANGUP, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_CHAT, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_EMOTE, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_PEEK, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_REQUEST, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_CHAT_SESSIONS, OnUpdateCheckList)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChatListDlg message handlers

void CChatListDlg::OnCall() 
{
CChatCallDlg dlg;

  // default to what they did last time
  dlg.m_iPort       = m_pDoc->m_iLastOutgoingChatPort;
  dlg.m_strAddress  = m_pDoc->m_strLastOutgoingChatAddress;
  
  if (dlg.DoModal () != IDOK)
    return;

  // remember for next time so they don't need to re-key
  m_pDoc->m_iLastOutgoingChatPort =         (unsigned short) dlg.m_iPort       ;
  m_pDoc->m_strLastOutgoingChatAddress =    dlg.m_strAddress  ;

  if (dlg.m_bzChat)
    m_pDoc->ChatCallzChat (dlg.m_strAddress, dlg.m_iPort);
  else
    m_pDoc->ChatCall (dlg.m_strAddress, dlg.m_iPort);

}

void CChatListDlg::OnChatAll() 
{
CChatToAllDlg dlg;

  dlg.m_strTitle = "Chat to all";

  if (dlg.DoModal () != IDOK)
    return;

  m_pDoc->ChatEverybody (dlg.m_strText, false);   // say it
	
}

void CChatListDlg::OnEmoteAll() 
{
CEmoteToAllDlg dlg;

  dlg.m_strTitle = "Emote to all";

  if (dlg.DoModal () != IDOK)
    return;

  m_pDoc->ChatEverybody (dlg.m_strText, true);   // emote it

  }


BOOL CChatListDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    // update window title
  SetWindowText (TFormat ("Chat sessions for %s", m_pDoc->m_mush_name));
	
int iColOrder [eColumnChatCount] = {0, 1, 2, 3, 4, 5},
    iColWidth [eColumnChatCount] = {90, 66, 81, 81, 55, 50};

// set list control to display whole row when selected and to allow column drag/drop

  App.RestoreColumnConfiguration ("Chat List", eColumnChatCount, m_ctlChatList,
                                  iColOrder, iColWidth, m_last_col, m_reverse);
 
 m_ctlChatList.InsertColumn(eColumnChatName, TranslateHeading ("Name"), LVCFMT_LEFT, iColWidth [eColumnChatName]);
 m_ctlChatList.InsertColumn(eColumnGroup, TranslateHeading ("Group"), LVCFMT_LEFT, iColWidth [eColumnGroup]);
 m_ctlChatList.InsertColumn(eColumnFromIP, TranslateHeading ("From IP"), LVCFMT_LEFT, iColWidth [eColumnFromIP]);
 m_ctlChatList.InsertColumn(eColumnCallIP, TranslateHeading ("Call IP"), LVCFMT_LEFT, iColWidth [eColumnCallIP]);
 m_ctlChatList.InsertColumn(eColumnCallPort, TranslateHeading ("Call Port"), LVCFMT_LEFT, iColWidth [eColumnCallPort]);
 m_ctlChatList.InsertColumn(eColumnFlags, TranslateHeading ("Flags"), LVCFMT_LEFT, iColWidth [eColumnFlags]);
                                                       
// recover column sequence

  m_ctlChatList.SendMessage (LVM_SETCOLUMNORDERARRAY, eColumnChatCount, (DWORD) iColOrder);
	
  // the update handler will auto-refresh the list ;)

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CChatListDlg::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CChatListDlg::OnKickIdle

void CChatListDlg::OnUpdateNeedSelection(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable (m_ctlChatList.GetSelectedCount () > 0);
}  // end of CChatListDlg::OnUpdateNeedSelection

void CChatListDlg::OnUpdateNeedOneSelection(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable (m_ctlChatList.GetSelectedCount () == 1);
}  // end of CChatListDlg::OnUpdateNeedOneSelection

void CChatListDlg::OnEdit() 
{
// iterate through list 
for (int nItem = -1;
      (nItem = m_ctlChatList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
  {

  long iChatID = m_ctlChatList.GetItemData (nItem);
	  
  CChatSocket * pSock = m_pDoc->GetChatSocket (iChatID);

  if (!pSock)   // must have gone
    continue;

  // edit it here
  CChatDetailsDlg dlg;


  dlg.m_strRemoteUserName   = pSock->m_strRemoteUserName  ;
	dlg.m_strGroup            = pSock->m_strGroup           ;
	dlg.m_strActualIPaddress  = inet_ntoa (pSock->m_ServerAddr.sin_addr);
	dlg.m_strAllegedAddress   = pSock->m_strAllegedAddress  ;
	dlg.m_strAllegedPort      = CFormat ("%d", pSock->m_iAllegedPort);
	dlg.m_strChatID           = CFormat ("%ld", pSock->m_iChatID);
	dlg.m_strRemoteVersion    = pSock->m_strRemoteVersion   ;
	dlg.m_bIncoming           = pSock->m_bIncoming          ;
	dlg.m_bCanSnoop           = pSock->m_bCanSnoop          ;
	dlg.m_bCanSendFiles       = pSock->m_bCanSendFiles      ;
	dlg.m_bPrivate            = pSock->m_bPrivate           ;
	dlg.m_bIgnore             = pSock->m_bIgnore            ;
  dlg.m_bCanSendCommands    = pSock->m_bCanSendCommands   ;

  if (dlg.DoModal () != IDOK)
    continue;

  // check still exists :)
  pSock = m_pDoc->GetChatSocket (iChatID);

  if (!pSock)  
    continue;    // must have gone while we were thinking

  // we do it this way so they get the message about the option changing
  m_pDoc->SetChatOption (iChatID, "group",              dlg.m_strGroup);
  // boolean options
  m_pDoc->SetChatOption (iChatID, "served",             dlg.m_bIncoming ? "1" : "0");
  m_pDoc->SetChatOption (iChatID, "can_snoop",          dlg.m_bCanSnoop ? "1" : "0");
  m_pDoc->SetChatOption (iChatID, "can_send_files",     dlg.m_bCanSendFiles ? "1" : "0");
  m_pDoc->SetChatOption (iChatID, "private",            dlg.m_bPrivate ? "1" : "0");
  m_pDoc->SetChatOption (iChatID, "ignore",             dlg.m_bIgnore ? "1" : "0");
  m_pDoc->SetChatOption (iChatID, "can_send_commands",  dlg.m_bCanSendCommands ? "1" : "0");
  
  } // end of loop
	
}

void CChatListDlg::OnPing() 
{
// iterate through list 
  for (int nItem = -1;
        (nItem = m_ctlChatList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
    m_pDoc->ChatPing (m_ctlChatList.GetItemData (nItem));
	
}

void CChatListDlg::OnSnoop() 
{
// iterate through list 
  for (int nItem = -1;
        (nItem = m_ctlChatList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
          m_pDoc->ChatMessage (m_ctlChatList.GetItemData (nItem), CHAT_SNOOP, "");
	
}

void CChatListDlg::OnSendfile() 
{
// iterate through list 
  for (int nItem = -1;
        (nItem = m_ctlChatList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
          m_pDoc->ChatSendFile (m_ctlChatList.GetItemData (nItem), "");
	
}

void CChatListDlg::OnHangup() 
{
// iterate through list 
  for (int nItem = -1;
        (nItem = m_ctlChatList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
    m_pDoc->ChatDisconnect (m_ctlChatList.GetItemData (nItem));
	
}

void CChatListDlg::OnChat() 
{
CChatToAllDlg dlg;

  dlg.m_strTitle = "Chat";

  if (dlg.DoModal () != IDOK)
    return;

// iterate through list 
  for (int nItem = -1;
        (nItem = m_ctlChatList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
    m_pDoc->ChatID (m_ctlChatList.GetItemData (nItem), dlg.m_strText, false);
	
}

void CChatListDlg::OnEmote() 
{
CChatToAllDlg dlg;

  dlg.m_strTitle = "Chat";

  if (dlg.DoModal () != IDOK)
    return;

// iterate through list 
  for (int nItem = -1;
        (nItem = m_ctlChatList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
    m_pDoc->ChatID (m_ctlChatList.GetItemData (nItem), dlg.m_strText, true);
	
}

void CChatListDlg::OnPeek() 
{
// iterate through list 
  for (int nItem = -1;
        (nItem = m_ctlChatList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
    m_pDoc->ChatPeekConnections (m_ctlChatList.GetItemData (nItem));
	
}

void CChatListDlg::OnRequest() 
{
// iterate through list 
  for (int nItem = -1;
        (nItem = m_ctlChatList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
    m_pDoc->ChatRequestConnections (m_ctlChatList.GetItemData (nItem));
	
}

static CString MakeFlags (CChatSocket * pChat)
  {
  CString strFlags;

  if (pChat->m_bCanSendCommands)
     strFlags += "A";

  if (pChat->m_bIgnore)
     strFlags += "I";
  
  if (pChat->m_bPrivate)
     strFlags += "P";

  if (pChat->m_bCanSnoop)
     strFlags += "n";

  if (pChat->m_bHeIsSnooping)
     strFlags += "N";

  if (pChat->m_bIncoming)
     strFlags += "S";

  if (pChat->m_bCanSendFiles)
     strFlags += "F";
    
  if (pChat->m_iChatConnectionType == eChatZMud)
     strFlags += "z";
      
  return strFlags;

  }

static int CALLBACK CompareFunc ( LPARAM lParam1, 
                                 LPARAM lParam2,
                                 LPARAM lParamSort)
  { 

 ASSERT (lParamSort != NULL);

t_chat_sort_param * psort_param = (t_chat_sort_param *) lParamSort;

 CChatSocket * item1 = psort_param->pDoc->GetChatSocket (lParam1);
 CChatSocket * item2 = psort_param->pDoc->GetChatSocket (lParam2);

 if (item1 == NULL || item2 == NULL)
   return 0;

int iResult;

  switch (psort_param->sortkey)   // which sort key
    {
    case eColumnChatName:
      iResult = item1->m_strRemoteUserName.CompareNoCase (item2->m_strRemoteUserName); 
      break;

    case eColumnGroup:
      iResult = item1->m_strGroup.CompareNoCase (item2->m_strGroup); 
      break;

    case eColumnFromIP:
      if (item1->m_ServerAddr.sin_addr.S_un.S_addr <  item2->m_ServerAddr.sin_addr.S_un.S_addr)
        iResult = -1;
      else 
      if (item1->m_ServerAddr.sin_addr.S_un.S_addr > item2->m_ServerAddr.sin_addr.S_un.S_addr)
        iResult = 1;
      else 
        iResult = 0;
      break;

    case eColumnCallIP:
      iResult = item1->m_strAllegedAddress.CompareNoCase (item2->m_strAllegedAddress); 
      break;

    case eColumnCallPort:
      if (item1->m_iAllegedPort < item2->m_iAllegedPort)
        iResult = -1;
      else
      if (item1->m_iAllegedPort > item2->m_iAllegedPort)
        iResult = 1;
      else 
        iResult = 0;
      break;

    case eColumnFlags:
      iResult = MakeFlags (item1).CompareNoCase (MakeFlags (item2)); 
      break;

    default: return 0;
    } // end of switch

// if reverse sort wanted, reverse sense of result

  if (psort_param->reverse)
    iResult *= -1;

  return iResult;

  } // end of CompareFunc

void CChatListDlg::LoadList (void)
  {

int nItem;

  // remember which chat IDs were previously selected
  m_SelectedList.RemoveAll ();
  // and focussed
  m_FocussedList.RemoveAll ();

  // do selected ones
  for (nItem = -1;
        (nItem = m_ctlChatList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
     m_SelectedList.AddTail (m_ctlChatList.GetItemData (nItem));
  // do focussed ones
  for (nItem = -1;
        (nItem = m_ctlChatList.GetNextItem(nItem, LVNI_FOCUSED)) != -1;)
     m_FocussedList.AddTail (m_ctlChatList.GetItemData (nItem));

  nItem = 0;
  m_ctlChatList.DeleteAllItems ();

  for (POSITION pos = m_pDoc->m_ChatList.GetHeadPosition(); pos; nItem++)
    {
    CChatSocket * p = m_pDoc->m_ChatList.GetNext (pos);

    // skip ones not yet connected
    if (p->m_bDeleteMe  || 
        p->m_iChatStatus != eChatConnected)
        continue;

    CString strFlags = MakeFlags (p);

 	  m_ctlChatList.InsertItem (nItem, p->m_strRemoteUserName);    // eColumnChatName

	  m_ctlChatList.SetItemText (nItem, eColumnGroup, p->m_strGroup);
	  m_ctlChatList.SetItemText (nItem, eColumnFromIP, inet_ntoa (p->m_ServerAddr.sin_addr));
	  m_ctlChatList.SetItemText (nItem, eColumnCallIP, p->m_strAllegedAddress);
	  m_ctlChatList.SetItemText (nItem, eColumnCallPort, CFormat ("%ld", p->m_iAllegedPort));
	  m_ctlChatList.SetItemText (nItem, eColumnFlags, strFlags);
    m_ctlChatList.SetItemData (nItem, p->m_iChatID);

    if (m_SelectedList.Find (p->m_iChatID))
     m_ctlChatList.SetItemState (nItem,  LVIS_SELECTED, 
                                         LVIS_SELECTED);

    if (m_FocussedList.Find (p->m_iChatID))
     m_ctlChatList.SetItemState (nItem,  LVIS_FOCUSED, 
                                         LVIS_FOCUSED);

    }

  // we have redone the list, redo the hash
  m_strListHash = MakeListHash ();

  t_chat_sort_param sort_param (m_pDoc, m_last_col, m_reverse);
  m_ctlChatList.SortItems (CompareFunc, (LPARAM) &sort_param); 

// set the 1st item to be selected - we do this here because sorting the
// list means our first item is not necessarily the 1st item in the list

  if (m_SelectedList.IsEmpty ())
     m_ctlChatList.SetItemState (0, LVIS_SELECTED, 
                                    LVIS_SELECTED);

  if (m_FocussedList.IsEmpty ())
     m_ctlChatList.SetItemState (0, LVIS_FOCUSED , 
                                     LVIS_FOCUSED );

  } // end of CChatListDlg::LoadList

void CChatListDlg::OnDestroy() 
{
  App.SaveColumnConfiguration ("Chat List",  eColumnChatCount, m_ctlChatList,
                                m_last_col, m_reverse);

  CDialog::OnDestroy();
	
}


// we use this hash to determine if the list has changed

CString CChatListDlg::MakeListHash (void)
  {
  CString strResult;

  for (POSITION pos = m_pDoc->m_ChatList.GetHeadPosition(); pos; )
    {
    CChatSocket * p = m_pDoc->m_ChatList.GetNext (pos);

    // skip ones not yet connected
    if (p->m_bDeleteMe  || 
        p->m_iChatStatus != eChatConnected)
        continue;

    strResult += CFormat ("%ul,%s,%s,%s",
                          p->m_iChatID,
                          (LPCTSTR) p->m_strRemoteUserName,
                          (LPCTSTR) p->m_strGroup,
                          (LPCTSTR) MakeFlags (p));
    }


  return strResult;
  } // end of CChatListDlg::MakeListHash


void CChatListDlg::OnUpdateCheckList(CCmdUI* pCmdUI) 
{
CString strNewHash = MakeListHash ();

  if (strNewHash != m_strListHash)
     LoadList ();

  pCmdUI->Enable (true);
}  // end of CChatListDlg::OnUpdateNeedSelection

void CChatListDlg::OnDblclkChatSessions(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnEdit ();
	
	*pResult = 0;
}

void CChatListDlg::OnColumnclickChatSessions(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  int col = pNMListView->iSubItem;

  if (col == m_last_col)
    m_reverse = !m_reverse;
  else
    m_reverse = FALSE;

  m_last_col = col;
    
  t_chat_sort_param sort_param (m_pDoc, m_last_col, m_reverse);
  m_ctlChatList.SortItems (CompareFunc, (LPARAM) &sort_param); 
	
	*pResult = 0;

}
