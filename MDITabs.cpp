/****************************************************************************\
Datei  : MDITabs.h
Projekt: MDITabs, a tabcontrol for switching between MDI-views
Inhalt : CMDITabs implementation
Datum  : 03.10.2001
Autor  : Christian Rodemeyer
Hinweis: 2001 by Christian Rodemeyer
\****************************************************************************/

#include "stdafx.h"
#include "MDITabs.h"
#include <AFXPRIV.H>
#include <algorithm>
#include <vector>


#include "MUSHclient.h"
#include "TextDocument.h"
#include "TextView.h"
#include "doc.h"
#include "childfrm.h"
#include "textchildfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CMDITabs

CMDITabs::CMDITabs()
{
  m_mdiClient = NULL;
  m_minViews = 0;
  m_bImages = false;
  m_bTop    = false;
}

BEGIN_MESSAGE_MAP(CMDITabs, CTabCtrl)
  //{{AFX_MSG_MAP(CMDITabs)
  ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnSelChange)
  ON_WM_PAINT()
  ON_WM_NCPAINT()
	ON_WM_CONTEXTMENU()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMDITabs message handlers

afx_msg LRESULT CMDITabs::OnSizeParent(WPARAM, LPARAM lParam)
  {
  if (GetItemCount() < m_minViews) 
    {
    ShowWindow(SW_HIDE);
    }
  else 
    {  
    AFX_SIZEPARENTPARAMS* pParams = reinterpret_cast<AFX_SIZEPARENTPARAMS*>(lParam);
    
    const int height = 26 + (m_bImages ? 1 : 0);
    const int offset = 2;
    
    m_height = height + offset;
    m_width  = pParams->rect.right - pParams->rect.left;
    
    if (m_bTop)
      {
      pParams->rect.top += height;
      MoveWindow(pParams->rect.left, pParams->rect.top - height, m_width, m_height, true);
      }
    else
      {
      pParams->rect.bottom -= height;
      MoveWindow(pParams->rect.left, pParams->rect.bottom - offset, m_width, m_height, true);
      }
    ShowWindow(SW_NORMAL);
    }
  return 0;
  }

void CMDITabs::OnSelChange(NMHDR* pNMHDR, LRESULT* pResult)
  {
  TCITEM item;
  item.mask = TCIF_PARAM;
  GetItem(GetCurSel(), &item);
  ::BringWindowToTop(HWND(item.lParam));
  *pResult = 0;
  }

CString GetTabText (HWND hwnd)
  {
  CString strName;
  char text[256];

  CWnd * pWnd = CWnd::FromHandle  (hwnd);

  ::GetWindowText(hwnd, text, sizeof text);
  strName = text;

  if (pWnd->IsKindOf(RUNTIME_CLASS(CChildFrame)))
    {
    CChildFrame * pWorldFrame = (CChildFrame *) pWnd;
    CMUSHclientDoc * pDoc = pWorldFrame->m_pDoc;
    strName = pDoc->m_mush_name;
    if (pDoc->m_new_lines && !pDoc->m_bDoNotShowOutstandingLines)
      strName += CFormat (" (%i)", pDoc->m_new_lines);

    }
  else if (pWnd->IsKindOf(RUNTIME_CLASS(CTextChildFrame)))
    {
    CTextChildFrame * pNotepadFrame = (CTextChildFrame *) pWnd;
    CTextDocument * pDoc = pNotepadFrame->m_pDoc;
    if (!pDoc->m_strTitle.IsEmpty ())
      strName = pDoc->m_strTitle;
    if (pDoc->IsModified () && !pDoc->m_bReadOnly)
       strName += " *";
    }

  return strName;
  }

void CMDITabs::Update()
  {
  SetRedraw(false);
  
  HWND active = ::GetTopWindow(m_mdiClient); // get active view window (actually the frame of the view)
  
  typedef std::vector<HWND> TWndVec;
  typedef TWndVec::iterator TWndIter;
  
  TWndVec vChild; // put all child windows in a list (actually a vector)
  for (HWND child = active; child; child = ::GetNextWindow(child, GW_HWNDNEXT))
    {
    vChild.push_back(child);
    }
  
  TCITEM item;
//  char text[256];
//  item.pszText = text;
  
  int i;
  for (i = GetItemCount(); i--;)  // for each tab
    {
    item.mask = TCIF_PARAM;
    GetItem(i, &item);
    
    TWndIter it = std::find(vChild.begin(), vChild.end(), HWND(item.lParam));
    if (it == vChild.end()) // associatete view does no longer exist, so delete the tab
      {
      DeleteItem(i);
      if (m_bImages) RemoveImage(i);
      }
    else // update the tab's text, image and selection state
      {
      item.mask = TCIF_TEXT;
//      ::GetWindowText(*it, text, sizeof text);
      CString sText = GetTabText (*it);
      item.pszText = (char *) (LPCTSTR) sText;
      if (m_bImages) 
        m_images.Replace(i, (HICON)::GetClassLong(*it, GCL_HICONSM));
      SetItem(i, &item);
      if (*it == active) 
        SetCurSel(i); // associated view is active => make it the current selection
      vChild.erase(it);                // remove view from list
      }
    }
  
  // all remaining views in vChild have to be added as new tabs
  i = GetItemCount();
  for (TWndIter it = vChild.begin(), end = vChild.end(); it != end; ++it)
    {
    item.mask = TCIF_TEXT|TCIF_PARAM|TCIF_IMAGE;
//    ::GetWindowText(*it, text, sizeof text);
    CString sText = GetTabText (*it);
    item.pszText = (char *) (LPCTSTR) sText;
    if (m_bImages) 
      m_images.Add((HICON)::GetClassLong(*it, GCL_HICONSM));
    item.iImage = i;
    item.lParam = LPARAM(*it);
    InsertItem(i, &item);
    if (*it == active) SetCurSel(i);
    ++i;
    }
  
  // this removes the control when there are no tabs and shows it when there is at least one tab
  bool bShow = GetItemCount() >= m_minViews;
  if ((!bShow && IsWindowVisible()) || (bShow && !IsWindowVisible())) 
    {
    static_cast<CMDIFrameWnd*>(FromHandlePermanent(::GetParent(m_mdiClient)))->RecalcLayout();
    }
  
  RedrawWindow(NULL, NULL, RDW_FRAME|RDW_INVALIDATE|RDW_ERASE);
  SetRedraw(true);
  }

void CMDITabs::OnPaint()
  {
  CPaintDC dc(this);
  
  if (GetItemCount() == 0) return; // do nothing
  
  int dcState = dc.SaveDC();
  
  // windows should draw the control as usual
  _AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
  pThreadState->m_lastSentMsg.wParam = WPARAM(HDC(dc));
  Default();
  
  dc.RestoreDC(dcState);
  
  DWORD face = ::GetSysColor(COLOR_3DFACE);
  if (m_bTop)
    {
    CRect rect(0, m_height - 7, m_width, m_height);
    dc.FillSolidRect(&rect, face);
    }
  else
    {
    CRect rect(0, 0, m_width, 3);
    dc.FillSolidRect(&rect, face);
    }
  }

void CMDITabs::OnNcPaint()
  {
  HDC hdc = ::GetWindowDC(m_hWnd);
  
  CRect rect;
  rect.left = 0;
  rect.top = m_bTop ? 0 : -2;
  rect.right = m_width;
  rect.bottom = m_height;
  
  HPEN pen = ::CreatePen(PS_SOLID, 0, ::GetSysColor(COLOR_3DFACE));
  HGDIOBJ old = ::SelectObject(hdc, pen);
  if (m_bTop)
    {
    DrawEdge(hdc, rect, EDGE_SUNKEN, BF_LEFT|BF_RIGHT|BF_TOP);
    ::MoveToEx(hdc, 2, m_height - 1, NULL);
    ::LineTo(hdc, m_width - 2, m_height - 1);
    ::MoveToEx(hdc, 2, m_height - 2, NULL);
    ::LineTo(hdc, m_width - 2, m_height - 2);
    }
  else
    {
    DrawEdge(hdc, rect, EDGE_SUNKEN, BF_LEFT|BF_RIGHT|BF_BOTTOM);
    ::MoveToEx(hdc, 2, 0, NULL);
    ::LineTo(hdc, m_width - 2, 0);
    ::MoveToEx(hdc, 2, 1, NULL);
    ::LineTo(hdc, m_width - 2, 1);
    }
  ::SelectObject(hdc, old);
  ::DeleteObject(pen);
  ::ReleaseDC(m_hWnd, hdc);
  }

void CMDITabs::Create(CFrameWnd* pMainFrame, DWORD dwStyle)
  {
  m_bTop = (dwStyle & MT_TOP);
  m_minViews = (dwStyle & MT_HIDEWLT2VIEWS) ? 2 : 1;
  
  CTabCtrl::Create(WS_CHILD|WS_VISIBLE|(m_bTop?0:TCS_BOTTOM)|TCS_SINGLELINE|TCS_FOCUSNEVER|TCS_FORCEICONLEFT|WS_CLIPSIBLINGS, CRect(0, 0, 0, 0), pMainFrame, 42);
  ModifyStyleEx(0, WS_EX_CLIENTEDGE);
  SendMessage(WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)), 0);
  
  HWND wnd;
  for (wnd = ::GetTopWindow(*pMainFrame); wnd; wnd = ::GetNextWindow(wnd, GW_HWNDNEXT))
    {
    char wndClass[32];
    ::GetClassName(wnd, wndClass, 32);
    if (strncmp(wndClass, "MDIClient", 32) == 0) break;
    }
  m_mdiClient = wnd;
  
  ASSERT(m_mdiClient); // Ooops, no MDIClient window?
  
  // manipulate Z-order so, that our tabctrl is above the mdi client, but below any status bar
  ::SetWindowPos(m_hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
  ::SetWindowPos(m_mdiClient, m_hWnd, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
  m_bImages = (dwStyle & MT_IMAGES) != 0;
  if (m_bImages)
    {
    if (m_images.GetSafeHandle()) 
      {
      m_images.SetImageCount(0);
      }
    else    
      {
      m_images.Create(16, 16, ILC_COLORDDB|ILC_MASK, 1, 1);
      }
    SetImageList(&m_images);
    }
  
  //SetItemSize(CSize(50, 0)); // Fixed Width Experiment
  }

void CMDITabs::OnContextMenu(CWnd* pWnd, CPoint point) 
  {
  TCHITTESTINFO hit;
  hit.pt = point;
  ScreenToClient(&hit.pt);
  int i = HitTest(&hit);
  if (i >= 0) 
    {
    TCITEM item;
    item.mask = TCIF_PARAM;
    GetItem(i, &item);
    
    HWND hWnd = HWND(item.lParam);
    SetCurSel(i);
    ::BringWindowToTop(hWnd);
    
    HMENU menu = HMENU(::SendMessage(::GetTopWindow(hWnd), WM_GETTABSYSMENU, 0, 0));
    if (menu == 0) menu = ::GetSystemMenu(hWnd, FALSE);
    UINT cmd = ::TrackPopupMenu(menu, TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_VCENTERALIGN, point.x, point.y, 0, m_hWnd, NULL);
    ::SendMessage(hWnd, WM_SYSCOMMAND, cmd, 0);
    }
  }

void CMDITabs::OnLButtonDblClk(UINT nFlags, CPoint point) 
  {
  int i = GetCurSel();
  if (i >= 0) 
    {
    TCITEM item;
    item.mask = TCIF_PARAM;
    GetItem(i, &item);
    HWND hWnd = HWND(item.lParam);
    ::ShowWindow(hWnd, SW_MAXIMIZE);
    }
  }
