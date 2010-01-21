// MySplitterWnd.cpp : implementation file
//

#include "stdafx.h"
#include "mushclient.h"
#include "doc.h"

#include "MySplitterWnd.h"
#include "childfrm.h"
#include "MUSHview.h"
#include "sendvw.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMySplitterWnd

CMySplitterWnd::CMySplitterWnd()
{
  m_pDoc = NULL;
}

CMySplitterWnd::~CMySplitterWnd()
{
}


BEGIN_MESSAGE_MAP(CMySplitterWnd, CSplitterWnd)
	//{{AFX_MSG_MAP(CMySplitterWnd)
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMySplitterWnd message handlers

void CMySplitterWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
  CSplitterWnd::OnLButtonUp(nFlags, point);

  // this hideous code is so that we know when the user has resized the
  // input pane, so we don't snap the size back to what it was
  if (m_pDoc)
    {
    int cyCur,
        cyMin;
    GetRowInfo (COMMAND_PANE, cyCur, cyMin);

    // snap to boundary stuff (version 4.16)
    if (((GetKeyState (VK_LCONTROL) & 0x8000) != 0 ||
        (GetKeyState (VK_RCONTROL) & 0x8000) != 0) && m_pDoc->m_input_font)
      {
      CDC dc;

      dc.CreateCompatibleDC (NULL);
      dc.SelectObject (m_pDoc->m_input_font);

      TEXTMETRIC tm;
      dc.GetTextMetrics(&tm);

      // need to know height of entire font (eg. 12 point FixedSys is 15)
      int m_iLineHeight = tm.tmHeight; 

      const int iFudge = 4;   // trial and error stuff, if anyone has a better idea let me know

      double fLines = (cyCur - iFudge) / m_iLineHeight;
      int iLines = floor (fLines + 0.5);   // round to nearest number of lines
      if (iLines)   
        cyCur =  m_iLineHeight * iLines + iFudge;  // calculate desired height for those lines
      else
        cyCur = 0;  // zero lines is still zero
      } // end of ctrl key held down

    if (!(m_pDoc->m_mush_name.IsEmpty ()))
      App.db_write_int ("worlds", (LPCTSTR) CFormat ("%s:%s", (LPCTSTR) m_pDoc->m_mush_name, "Bottom Height"), cyCur);

//    TRACE1 ("Bottom height (saved) = %i\n", cyCur);

    if (m_pDoc->m_pActiveCommandView)
      m_pDoc->m_pActiveCommandView->m_owner_frame->FixUpSplitterBar ();
    else if (m_pDoc->m_pActiveOutputView)
      m_pDoc->m_pActiveOutputView->m_owner_frame->FixUpSplitterBar ();

	  }

}

