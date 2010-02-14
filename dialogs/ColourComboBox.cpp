// ColourComboBox.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "ColourComboBox.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColourComboBox

CColourComboBox::CColourComboBox()
{

// default to black on white in case they never set them
  for (int i = 0; i < MAX_CUSTOM; i++)
    {
    m_customtext [i] = RGB (0, 0, 0);;
    m_customback [i] = RGB (255, 255, 255);
    }

  m_iOtherForeground = RGB (0, 0, 0); // black 
  m_iOtherBackground = RGB (255, 255, 255); // on white

}

CColourComboBox::~CColourComboBox()
{
}


BEGIN_MESSAGE_MAP(CColourComboBox, CComboBox)
	//{{AFX_MSG_MAP(CColourComboBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColourComboBox message handlers

void CColourComboBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{

	if (lpDrawItemStruct->CtlType != ODT_COMBOBOX)
    return;


	CDC*	pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect	rc = lpDrawItemStruct->rcItem;
  CString strText;

  int iItem = lpDrawItemStruct->itemID - 1;   // 0 is actually "no change" here

  if (lpDrawItemStruct->itemState  & ODS_DISABLED)
    {
		pDC->FillSolidRect(rc, RGB (192, 192, 192));  // grey background
    pDC->SetTextColor (RGB (0, 0, 0));    // black text
    }
  else
    {
    if (iItem >= 0 && iItem < MAX_CUSTOM)
      {
      // if same colour on itself, just show black on white
      if (m_customtext [iItem] == m_customback [iItem])
        {
		    pDC->FillSolidRect(rc, RGB (255, 255, 255));  // white background
        pDC->SetTextColor (RGB (0, 0, 0));    // black text
        }
      else
        {
		    // Draw colour
		    pDC->FillSolidRect(rc, m_customback [iItem]);
        pDC->SetTextColor (m_customtext [iItem]);
        }
      }    // end of custom colour (0 to 15)
    else if (iItem == OTHER_CUSTOM)
      {
      // if same colour on itself, just show black on white
      if (m_iOtherForeground == m_iOtherBackground)
        {
		    pDC->FillSolidRect(rc, RGB (255, 255, 255));  // white background
        pDC->SetTextColor (RGB (0, 0, 0));    // black text
        }
      else
        {
		    // Draw colour
		    pDC->FillSolidRect(rc, m_iOtherBackground);
        pDC->SetTextColor (m_iOtherForeground);
        }
      }  // end of "other" custom
    else
      {
		  pDC->FillSolidRect(rc, RGB (255, 255, 255));  // white background
      pDC->SetTextColor (RGB (0, 0, 0));    // black text
      }   // something else? (no change) for instance
    }   // not disabled

  // sometimes itemID is 0xFFFFFFFF it seems
  if (lpDrawItemStruct->itemID != UINT_MAX)
    {
    GetLBText(lpDrawItemStruct->itemID, strText);
    rc.left += 2;    // draw two pixels in
    pDC->DrawText (strText, rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    rc.left -= 2;    // undo add above
    }

  // if it has the focus, draw a dotted line
  if (lpDrawItemStruct->itemState  & ODS_FOCUS)
    {
    CRect rcFocus (rc);
    rcFocus.DeflateRect (1, 1);
    pDC->DrawFocusRect (rcFocus); 
    }

	
}
