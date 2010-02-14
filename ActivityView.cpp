// ActivityView.cpp : implementation file
//

#include "stdafx.h"
#include "mushclient.h"
#include "mainfrm.h"
#include "doc.h"
#include "ActivityView.h"
#include "sendvw.h"
#include "childfrm.h"

#include "winplace.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CActivityView

IMPLEMENT_DYNCREATE(CActivityView, CListView)

CActivityView::CActivityView()
{
App.m_pActivityView = this;
m_bUpdateLockout = FALSE;

// default to sorting in sequence order

  m_last_col = eColumnSeq;
  m_reverse = FALSE;

}

CActivityView::~CActivityView()
{
App.m_pActivityView = NULL;
}


BEGIN_MESSAGE_MAP(CActivityView, CListView)
	ON_WM_CONTEXTMENU()
	//{{AFX_MSG_MAP(CActivityView)
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(NM_RETURN, OnReturn)
	ON_COMMAND(ID_POPUP_CONFIGUREWORLD, OnPopupConfigureworld)
	ON_COMMAND(ID_POPUP_SWITCHTOWORLD, OnPopupSwitchtoworld)
	ON_UPDATE_COMMAND_UI(ID_POPUP_CONFIGUREWORLD, OnUpdatePopupConfigureworld)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_COMMAND(ID_CONNECTION_CONNECT, OnConnectionConnect)
	ON_COMMAND(ID_CONNECTION_DISCONNECT, OnConnectionDisconnect)
	ON_UPDATE_COMMAND_UI(ID_POPUP_SWITCHTOWORLD, OnUpdateNeedSelection)
	ON_UPDATE_COMMAND_UI(ID_CONNECTION_CONNECT, OnUpdateConnectionConnect)
	ON_UPDATE_COMMAND_UI(ID_CONNECTION_DISCONNECT, OnUpdateConnectionDisconnect)
	ON_COMMAND(ID_POPUP_FILE_CLOSE, OnPopupFileClose)
	ON_COMMAND(ID_POPUP_FILE_SAVE, OnPopupFileSave)
	ON_COMMAND(ID_GAME_PREFERENCES, OnPopupConfigureworld)
	ON_UPDATE_COMMAND_UI(ID_GAME_PREFERENCES, OnUpdatePopupConfigureworld)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_POPUP_FILE_CLOSE, OnUpdateNeedSelection)
	ON_UPDATE_COMMAND_UI(ID_POPUP_FILE_SAVE, OnUpdateNeedSelection)
	ON_UPDATE_COMMAND_UI(ID_POPUP_SAVEWORLDDETAILSAS, OnUpdateNeedSelection)
	ON_COMMAND(ID_POPUP_SAVEWORLDDETAILSAS, OnPopupSaveworlddetailsas)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CActivityView drawing

void CActivityView::OnDraw(CDC* pDC)
{
}

/////////////////////////////////////////////////////////////////////////////
// CActivityView diagnostics

#ifdef _DEBUG
void CActivityView::AssertValid() const
{
	CListView::AssertValid();
}

void CActivityView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CActivityView message handlers

void CActivityView::OnInitialUpdate() 
{

int iColOrder [eColumnCount] = {0, 1, 2, 3, 4, 5, 6},
    iColWidth [eColumnCount] = {40, 130, 55, 55, 80, 105, 70};

// set list control to display whole row when selected and to allow column drag/drop

  App.RestoreColumnConfiguration ("Activity List", eColumnCount, GetListCtrl (),
                                  iColOrder, iColWidth, m_last_col, m_reverse);
 
 GetListCtrl ().InsertColumn(eColumnSeq, TranslateHeading ("Seq"), LVCFMT_RIGHT, iColWidth [eColumnSeq]);
 GetListCtrl ().InsertColumn(eColumnMush, TranslateHeading ("World"), LVCFMT_LEFT, iColWidth [eColumnMush]);
 GetListCtrl ().InsertColumn(eColumnNew, TranslateHeading ("New"), LVCFMT_RIGHT, iColWidth [eColumnNew]);
 GetListCtrl ().InsertColumn(eColumnLines, TranslateHeading ("Lines"), LVCFMT_RIGHT, iColWidth [eColumnLines]);
 GetListCtrl ().InsertColumn(eColumnStatus, TranslateHeading ("Status"), LVCFMT_LEFT, iColWidth [eColumnStatus]);
 GetListCtrl ().InsertColumn(eColumnSince, TranslateHeading ("Since"), LVCFMT_LEFT, iColWidth [eColumnSince]);
 GetListCtrl ().InsertColumn(eColumnDuration, TranslateHeading ("Duration"), LVCFMT_RIGHT, iColWidth [eColumnDuration]);
                                                       
// recover column sequence

  GetListCtrl ().SendMessage (LVM_SETCOLUMNORDERARRAY, eColumnCount, (DWORD) iColOrder);

	CListView::OnInitialUpdate();
	
  GetParent()->SetWindowText(Translate ("Activity List"));

  CWindowPlacement wp;
  wp.Restore ("Activity List", GetParent (), false);
  
  m_imglIcons.Create (IDR_ACTIVITYIMAGES, 14, 1, RGB (255, 255, 255));
  GetListCtrl ().SetImageList (&m_imglIcons, LVSIL_SMALL);

}        // end of ActivityView::OnInitialUpdate

BOOL CActivityView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= LVS_REPORT | WS_BORDER | WS_TABSTOP;
	cs.style &= ~FWS_ADDTOTITLE;
	
	return CListView::PreCreateWindow(cs);
}

void CActivityView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
bool bInserting = false;
POSITION pos;
int nDocCount;
CListCtrl & pList = GetListCtrl ();

// if we don't want list updated right now, then exit

  if (m_bUpdateLockout)
    return;

//  TRACE ("Activity window being updated\n");

  m_bUpdateLockout = TRUE;

  App.m_bUpdateActivity = FALSE;

  App.m_timeLastActivityUpdate = CTime::GetCurrentTime();

// if the list has the same number of worlds (and they are the same
// worlds), then we can not bother deleting the list and re-adding it

// first, count worlds

 	pos = App.m_pWorldDocTemplate->GetFirstDocPosition();
  for (nDocCount = 0; pos; nDocCount++)
       App.m_pWorldDocTemplate->GetNextDoc(pos);

// if count is the same, check world is the same as in the list

  if (nDocCount == pList.GetItemCount ())
    {
   	pos = App.m_pWorldDocTemplate->GetFirstDocPosition();
    while (pos)
      {
       CMUSHclientDoc* pDoc = (CMUSHclientDoc*) App.m_pWorldDocTemplate->GetNextDoc(pos);
       int nItem = pDoc->m_view_number - 1;
       if (nItem < 0)
         {
         bInserting = true;
         break;
         }
       if ((DWORD) pDoc != pList.GetItemData (nItem))
         {
         bInserting = true;
         break;
         }
      }   // end of looping through each world
    }     // end of world count being same as list count
  else
    bInserting = true;    // different counts, must re-do list

  if (bInserting)
    pList.DeleteAllItems ();

// add all documents to the list

 	pos = App.m_pWorldDocTemplate->GetFirstDocPosition();

	for (int nItem = 0; pos != NULL; nItem++)
	{
    CMUSHclientDoc* pDoc = (CMUSHclientDoc*) App.m_pWorldDocTemplate->GetNextDoc(pos);

    if (bInserting)
      pDoc->m_view_number = nItem + 1;    // so we can use Ctrl+1 etc.
    else
      nItem = pDoc->m_view_number - 1;    // use existing item number

    CString strSeq;
    CString strLines;
    CString strNewLines;
    CString strStatus;
    CString strSince;
    CString strDuration;

    strSeq.Format   ("%ld", pDoc->m_view_number);
    strNewLines.Format ("%ld", pDoc->m_new_lines);
    strLines.Format ("%ld", pDoc->m_total_lines);

// work out world status

    strStatus = GetConnectionStatus (pDoc->m_iConnectPhase);

// when they connected

    if (pDoc->m_iConnectPhase == eConnectConnectedToMud)
      strSince = pDoc->FormatTime (pDoc->m_tConnectTime, "%#I:%M %p, %d %b");
    else
      strSince.Empty ();

// work out world connection duration

    // first time spent in previous connections
    CTimeSpan ts = pDoc->m_tsConnectDuration;
    
    // now time spent connected in this session, if we are connected
    if (pDoc->m_iConnectPhase == eConnectConnectedToMud)
      ts += CTime::GetCurrentTime() - pDoc->m_tConnectTime;
  
    if (ts.GetDays () > 0)
      strDuration = ts.Format ("%Dd %Hh %Mm %Ss");
    else
      if (ts.GetHours () > 0)
        strDuration = ts.Format ("%Hh %Mm %Ss");
      else
        if (ts.GetMinutes () > 0)
          strDuration = ts.Format ("%Mm %Ss");
        else
          strDuration = ts.Format ("%Ss");
	
    // sequence
    if (bInserting)
      pList.InsertItem (nItem, strSeq);  //  eColumnSeq
	  pList.SetItemText(nItem, eColumnMush, pDoc->m_mush_name);
		pList.SetItemText(nItem, eColumnNew, strNewLines);
 		pList.SetItemText(nItem, eColumnLines, strLines);
  	pList.SetItemText(nItem, eColumnStatus, strStatus);
		pList.SetItemText(nItem, eColumnSince, strSince);
		pList.SetItemText(nItem, eColumnDuration, strDuration);

    if (bInserting)
      pList.SetItemData(nItem, (DWORD) pDoc);

    LVITEM lvitem;

    memset (&lvitem, 0, sizeof lvitem);

    // update where tick goes
    lvitem.iImage = 0;
    
    if (pDoc->m_pActiveCommandView || pDoc->m_pActiveOutputView)
      lvitem.iImage = 1;  // show the tick

    lvitem.mask = LVIF_IMAGE;
    lvitem.iItem = nItem;

    pList.SetItem (&lvitem);



   }    // end of searching for all documents

// make sure in same order that we left them

  pList.SortItems (CompareFunc, m_reverse << 8 | m_last_col); 

  m_bUpdateLockout = FALSE;

}     // end of CActivityView::OnUpdate

void CActivityView::OnDestroy() 
{

	App.SaveColumnConfiguration ("Activity List", eColumnCount, GetListCtrl (),
                               m_last_col, m_reverse);

  CWindowPlacement wp;
  wp.Save ("Activity List", GetParent ());

	CListView::OnDestroy();
	
}

void CActivityView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{

	*pResult = 0;
	
  OnPopupSwitchtoworld ();

  }

void CActivityView::OnReturn(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;

  OnPopupSwitchtoworld ();
}

void CActivityView::OnContextMenu(CWnd*, CPoint point)
{

	// CG: This block was added by the Pop-up Menu component
	{
		if (point.x == -1 && point.y == -1){
			//keystroke invocation
			CRect rect;
			GetClientRect(rect);
			ClientToScreen(rect);

			point = rect.TopLeft();
			point.Offset(5, 5);
		}

		CMenu menu;
		VERIFY(menu.LoadMenu(CG_IDR_POPUP_ACTIVITY_VIEW));

		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);
		CWnd* pWndPopupOwner = this;

	  CMenu mainmenu;

    // in full-screen mode, give access to all menu items
    if (Frame.IsFullScreen ())
      {
		  VERIFY(mainmenu.LoadMenu(IDR_MUSHCLTYPE));

      pPopup->AppendMenu (MF_SEPARATOR, 0, ""); 
      pPopup->AppendMenu (MF_POPUP | MF_ENABLED, (UINT ) mainmenu.m_hMenu, 
                          "Main Menus");     

      }

		while (pWndPopupOwner->GetStyle() & WS_CHILD &&
          pWndPopupOwner != pWndPopupOwner->GetParent())
			pWndPopupOwner = pWndPopupOwner->GetParent();

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
			pWndPopupOwner);
	}
}      // end of CActivityView::OnContextMenu

void CActivityView::OnPopupConfigureworld() 
{

  CMUSHclientDoc* pDoc = GetSelectedWorld ();

  if (!pDoc)
    return;

  pDoc->OnGamePreferences ();

  }     // end of CActivityView::OnPopupConfigureworld

void CActivityView::OnUpdatePopupConfigureworld(CCmdUI* pCmdUI) 
{

  CMUSHclientDoc* pDoc = GetSelectedWorld ();

  if (!pDoc)
    {
    pCmdUI->Enable (FALSE);
    return;   // no item selected
    }

  pCmdUI->Enable (pDoc->m_iConnectPhase == eConnectNotConnected ||
                  pDoc->m_iConnectPhase == eConnectConnectedToMud);   // can't configure whilst connecting
}      // end of CActivityView::OnUpdatePopupConfigureworld

void CActivityView::OnPopupSwitchtoworld() 
{

  CMUSHclientDoc* pDoc = GetSelectedWorld ();

  if (!pDoc)
    return;

  for(POSITION pos=pDoc->GetFirstViewPosition();pos!=NULL;)
    {
    CView* pView = pDoc->GetNextView(pos);

    if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
      {
      CSendView* pmyView = (CSendView*)pView;

      if (pmyView->GetParentFrame ()->IsIconic ())
        pmyView->GetParentFrame ()->ShowWindow (SW_RESTORE);

      pmyView->GetParentFrame ()->ActivateFrame ();
      pmyView->m_owner_frame->SetActiveView(pmyView);

      break;

      }	
    }
}       // end of CActivityView::OnPopupSwitchtoworld 


void CActivityView::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable (FALSE);
}


void CActivityView::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  int col = pNMListView->iSubItem;

  if (col == m_last_col)
    m_reverse = !m_reverse;
  else
    m_reverse = FALSE;

  m_last_col = col;
    
  GetListCtrl ().SortItems (CompareFunc, m_reverse << 8 | m_last_col); 
	
	*pResult = 0;
}


int CALLBACK CActivityView::CompareFunc ( LPARAM lParam1, 
                                          LPARAM lParam2,
                                          LPARAM lParamSort)
  { 

 CMUSHclientDoc * item1 = (CMUSHclientDoc *) lParam1;
 CMUSHclientDoc * item2 = (CMUSHclientDoc *) lParam2;

int iResult = 0;

  switch (lParamSort & 0xFF)   // which sort key
    {
    case eColumnSeq: 
                if (item1->m_view_number < item2->m_view_number)
                  iResult = -1;
                else
                if (item1->m_view_number > item2->m_view_number)
                  iResult = 1;
                break;

    case eColumnMush: 
                iResult = item1->m_mush_name.CompareNoCase 
                          (item2->m_mush_name); 
                break;

    case eColumnNew: 
                if (item1->m_new_lines < item2->m_new_lines)
                  iResult = -1;
                else
                if (item1->m_new_lines > item2->m_new_lines)
                  iResult = 1;
                break;

    case eColumnLines: 
                if (item1->m_total_lines < item2->m_total_lines)
                  iResult = -1;
                else
                if (item1->m_total_lines > item2->m_total_lines)
                  iResult = 1;
                break;

  case eColumnStatus: 
                {
                CString strStatus1 = GetConnectionStatus (item1->m_iConnectPhase);
                CString strStatus2 = GetConnectionStatus (item2->m_iConnectPhase);

                iResult = strStatus1.CompareNoCase (strStatus2); 
                break;
                }

  case eColumnSince: 
                {
                if (item1->m_tConnectTime < item2->m_tConnectTime)
                  iResult = -1;
                else
                if (item1->m_tConnectTime > item2->m_tConnectTime)
                  iResult = 1;
                }
                break;

  case eColumnDuration: 
                {
                CTimeSpan ts1 = CTime::GetCurrentTime() - item1->m_tConnectTime;
                CTimeSpan ts2 = CTime::GetCurrentTime() - item2->m_tConnectTime;
                if (ts1 < ts2)
                  iResult = -1;
                else
                if (ts1 > ts2)
                  iResult = 1;
                }
                break;

    default: return 0;
    } // end of switch

// if reverse sort wanted, reverse sense of result

  if (lParamSort & 0xFF00)
    iResult *= -1;

  return iResult;

  } // end of CompareFunc

void CActivityView::OnConnectionConnect() 
{

  CMUSHclientDoc* pDoc = GetSelectedWorld ();

  if (!pDoc)
    return;
  
  pDoc->Connect ();

  }

void CActivityView::OnConnectionDisconnect() 
{
  CMUSHclientDoc* pDoc = GetSelectedWorld ();

  if (!pDoc)
    return;
  
  pDoc->Disconnect ();
}

void CActivityView::OnUpdateNeedSelection(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable (GetListCtrl ().GetSelectedCount () > 0);
}

void CActivityView::OnUpdateConnectionConnect(CCmdUI* pCmdUI) 
{
  CMUSHclientDoc* pDoc = GetSelectedWorld ();

  if (!pDoc)
    {
    pCmdUI->Enable (FALSE);
    return;   // no item selected
    }

	pCmdUI->Enable (pDoc->m_iConnectPhase == eConnectNotConnected);

}

void CActivityView::OnUpdateConnectionDisconnect(CCmdUI* pCmdUI) 
{
  CMUSHclientDoc* pDoc = GetSelectedWorld ();

  if (!pDoc)
    {
    pCmdUI->Enable (FALSE);
    return;   // no item selected
    }

	pCmdUI->Enable (pDoc->m_iConnectPhase != eConnectNotConnected);
  
}


void CActivityView::OnPopupFileClose() 
{
  CMUSHclientDoc* pDoc = GetSelectedWorld ();

  if (!pDoc)
    return;

  pDoc->OnFileClose ();
  
}

void CActivityView::OnPopupFileSave() 
{
  CMUSHclientDoc* pDoc = GetSelectedWorld ();

  if (!pDoc)
    return;

  pDoc->OnFileSave ();

}


CMUSHclientDoc * CActivityView::GetSelectedWorld (void)
  {

int nItem = GetListCtrl ().GetNextItem(-1, LVNI_SELECTED);

  if (nItem == -1)
    return NULL;   // no item selected

  return (CMUSHclientDoc*) GetListCtrl ().GetItemData (nItem);

  } // end of  CActivityView::GetSelectedWorld

void CActivityView::OnPopupSaveworlddetailsas() 
{
  CMUSHclientDoc* pDoc = GetSelectedWorld ();

  if (!pDoc)
    return;

  pDoc->OnFileSaveAs ();
	
}
