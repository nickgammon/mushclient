// ButtonColour.cpp : implementation file
//

// nb. buttons are painted as "owner draw" and "disabled"

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "ButtonColour.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColourButton

CColourButton::CColourButton()
{
	m_colour = 0;
}

CColourButton::~CColourButton()
{
}


BEGIN_MESSAGE_MAP(CColourButton, CButton)
	//{{AFX_MSG_MAP(CColourButton)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_POPUP_COPYCOLOUR, OnPopupCopycolour)
	ON_COMMAND(ID_POPUP_PASTECOLOUR, OnPopupPastecolour)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColourButton message handlers

void CColourButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDC*	pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect	rc = lpDrawItemStruct->rcItem;

	if (lpDrawItemStruct->CtlType == ODT_BUTTON)
	{
		// Draw 3D frame
   if (lpDrawItemStruct->itemState & (ODS_SELECTED | ODS_DISABLED))
  		pDC->DrawEdge(rc, EDGE_SUNKEN, BF_RECT|BF_ADJUST);
   else
  		pDC->DrawEdge(rc, EDGE_RAISED, BF_RECT|BF_ADJUST);

		// Draw colour
		pDC->FillSolidRect(rc, m_colour);


  // if it has the focus, draw a dotted line
  if (lpDrawItemStruct->itemState  & ODS_FOCUS)
    {
    CRect rcFocus (rc);
    rcFocus.DeflateRect (1, 1);
     if (lpDrawItemStruct->itemState & ODS_SELECTED)
       rcFocus.OffsetRect (-1, -1);  // move text as button moves
    pDC->DrawFocusRect (rcFocus); 
    }

    // Draw button text
   CString strText;
   GetWindowText(strText);

   COLORREF crOldColor;

   // if colour is dark, use white, otherwise use black
   if (((GetRValue (m_colour) & 0xFF) +
       (GetGValue (m_colour) & 0xFF) +
       (GetBValue (m_colour) & 0xFF) ) < (128 * 3))
     crOldColor = pDC->SetTextColor(RGB(255,255,255));
   else
     crOldColor = pDC->SetTextColor(RGB(0,0,0));
   if (lpDrawItemStruct->itemState & ODS_SELECTED)
     rc.OffsetRect (-1, -1);  // move text as button moves
   pDC->DrawText (strText, rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
   pDC->SetTextColor(crOldColor);


	}
}

          
void CColourButton::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if (point.x == -1 && point.y == -1)
    {
		//keystroke invocation
		CRect rect;
		GetClientRect(rect);
		ClientToScreen(rect);

		point = rect.TopLeft();
		point.Offset(5, 5);
	  }
  else
    {
		CRect rect;
		GetClientRect(rect);
		ClientToScreen(rect);
    int oldx = point.x,
        oldy = point.y;
		point = rect.TopLeft();
		point.Offset(oldx, oldy);

    }

	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_COLOUR_POPUP));

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

  bool bEnable = GetClipboardColour (m_clipboardColour);

  // disable paste if no colour there
  pPopup->EnableMenuItem (ID_POPUP_PASTECOLOUR, 
                          MF_BYCOMMAND | (bEnable ? MF_ENABLED :MF_GRAYED));

  // display menu
  pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
                          point.x, 
                          point.y,
		                      this);
}

void CColourButton::OnPopupCopycolour() 
{
// to save a lot of mucking around I will copy colours in text format, compatible with VB
  putontoclipboard (CFormat ("&h%X", m_colour & 0xFFFFFF)); 
}



void CColourButton::OnPopupPastecolour() 
{
// we already worked out the colour as part of enabling/disabling the menu item

  m_colour = m_clipboardColour;
  Invalidate ();	
}


void CColourButton::OnContextMenu(CWnd* pWnd, CPoint point) 
{
  OnRButtonDown (0, point);	
}
