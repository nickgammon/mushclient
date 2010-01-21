// MyStatusBar.cpp : implementation file
//

#include "stdafx.h"
#include "mushclient.h"
#include "mainfrm.h"
#include "MyStatusBar.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyStatusBar

CMyStatusBar::CMyStatusBar()
{
}

CMyStatusBar::~CMyStatusBar()
{
}


BEGIN_MESSAGE_MAP(CMyStatusBar, CStatusBar)
	//{{AFX_MSG_MAP(CMyStatusBar)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMyStatusBar message handlers

void CMyStatusBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	
//	CStatusBar::OnLButtonDblClk(nFlags, point);


  RECT rect;

  // see which pane they double-clicked in
  if (GetStatusBarCtrl ().GetRect 
      (CommandToIndex (ID_STATUSLINE_TIME), &rect ))
    {
    // if in it, post message to clear the connected time
    if (PtInRect (&rect, point))
      {
      Frame.SendMessage (WM_COMMAND, ID_GAME_RESETCONNECTEDTIME, 0);
      return;
      }
    }

  if (GetStatusBarCtrl ().GetRect 
      (CommandToIndex (ID_STATUSLINE_FREEZE), &rect ))
    {
    // if in it, post message to clear the connected time
    if (PtInRect (&rect, point))
      {
      Frame.SendMessage (WM_COMMAND, ID_DISPLAY_FREEZEOUTPUT, 0);
      return;
      }
    }


  if (GetStatusBarCtrl ().GetRect 
      (CommandToIndex (ID_STATUSLINE_LINES), &rect ))
    {
    // if in it, post message to open the "go to line" dialog
    if (PtInRect (&rect, point))
      {
      Frame.SendMessage (WM_COMMAND, ID_DISPLAY_GOTOLINE, 0);
      return;
      }
    }


}

void CMyStatusBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	
//	CStatusBar::OnLButtonDown(nFlags, point);

  RECT rect;

  // see which pane they clicked in
  if (GetStatusBarCtrl ().GetRect (CommandToIndex (ID_STATUSLINE_MUSHNAME), &rect ))
    {
    if (PtInRect (&rect, point))
      {
      Frame.LeftTrayClick ();
      }

    }

}
