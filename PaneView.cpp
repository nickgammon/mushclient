// PaneView.cpp : implementation file
//

#include "stdafx.h"
#ifdef PANE

#include "mushclient.h"
#include "doc.h"
#include "PaneView.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPaneView

IMPLEMENT_DYNCREATE(CPaneView, CView)

CPaneView::CPaneView()
{

  m_iMaxLines = 1000;
  m_iMaxWidth = 80;
  m_bUTF8 = false;

  m_bShowBold       = false;
  m_bShowItalic     = false;
  m_bShowUnderline  = false;
  m_nHeight = App.m_iFixedPitchFontSize;
  m_strFont = App.m_strFixedPitchFont;

  m_cText = RGB_WHITE;
  m_cBack = RGB_BLACK;

  for (int i = 0; i < 8; i++)  
    m_font [i] = NULL;

}

CPaneView::~CPaneView()
{

  for (int i = 0; i < 8; i++)  
    {
    delete m_font [i];         // get rid of old font
    m_font [i] = NULL;
    }

  // delete line deque
  for (deque <CPaneLine *>::iterator line_it = m_lines.begin (); 
       line_it != m_lines.end (); 
       line_it++)
    delete *line_it;

}


BEGIN_MESSAGE_MAP(CPaneView, CView)
	//{{AFX_MSG_MAP(CPaneView)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_TEST_END, OnGotoEnd)
	ON_COMMAND(ID_TEST_LINEDOWN, OnLinedown)
	ON_COMMAND(ID_TEST_LINEUP, OnLineup)
	ON_COMMAND(ID_TEST_PAGEDOWN, OnPagedown)
	ON_COMMAND(ID_TEST_PAGEUP, OnPageup)
	ON_COMMAND(ID_TEST_START, OnGotoStart)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPaneView drawing

void CPaneView::OnInitialUpdate()
{
	CDocument* pDoc = GetDocument();

  CView::OnInitialUpdate();

  /*
	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);

  */

  CDC dc;

  dc.CreateCompatibleDC (NULL);

  for (int i = 0; i < 8; i++)  
    {
     m_font [i] = new CFont;    // create new font

   
     // if height is zero, default to 10 so it doesn't look stupid
     int lfHeight = -MulDiv(m_nHeight ? m_nHeight : 10, dc.GetDeviceCaps(LOGPIXELSY), 72);

     m_font [i]->CreateFont(lfHeight, // int m_nHeight, 
				    0, // int nWidth, 
				    0, // int nEscapement, 
				    0, // int nOrientation, 
            m_bShowBold ? ((i & HILITE) ? FW_BOLD : FW_NORMAL) : FW_NORMAL, // int nWeight, 
            m_bShowItalic ? (i & BLINK) != 0 : 0, // BYTE bItalic, 
            m_bShowUnderline ? (i & UNDERLINE) != 0 : 0, // BYTE bUnderline, 
            0, // BYTE cStrikeOut, 
            MUSHCLIENT_FONT_CHARSET, // BYTE nCharSet, 
            0, // BYTE nOutPrecision, 
            0, // BYTE nClipPrecision, 
            0, // BYTE nQuality, 
            MUSHCLIENT_FONT_FAMILY, // BYTE nPitchAndFamily,     // was FF_DONTCARE
            m_strFont);// LPCTSTR lpszFacename );

    }   // end of allocating 8 fonts

  dc.SelectObject(m_font [HILITE]);
  
  TEXTMETRIC tm;
  dc.GetTextMetrics(&tm);

  m_iLineHeight = tm.tmHeight; 

  m_iFontWidth = tm.tmAveCharWidth; 

  GetParent ()->SetWindowText(m_sTitle.c_str ());


}

void CPaneView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();


COLORREF clrBackground = m_cBack;

RECT r;

  GetClientRect (&r);

  pDC->FillSolidRect (&r, clrBackground);


  // pDC->SetBkMode (TRANSPARENT);
  pDC->SetBkMode(OPAQUE);

int iVpixel = 0,
    iHpixel;
int iFirstLine,
    iLastLine,
    iLines;

  // where are we in the scrolled view?
  SCROLLINFO ScrollInfo;
  GetScrollInfo (SB_VERT, &ScrollInfo, SIF_POS);

  int y = ScrollInfo.nPos;

  iFirstLine = y;

  // scrolled past end - no drawing
  if (iFirstLine >= (int) m_lines.size ())
    return;

  // how big is our client window, eh? (in lines

  GetClientRect (&r);
  iLines = ((r.bottom - r.top) / m_iLineHeight);

  iLastLine = iFirstLine + iLines;

  // don't overshoot buffer
  if (iLastLine >= (int) m_lines.size ())
     iLastLine = m_lines.size ();

  // find first visible line
  deque <CPaneLine *>::iterator it_first_line = m_lines.begin ();
  it_first_line += iFirstLine;

  // find last visible line
  deque <CPaneLine *>::iterator it_last_line = m_lines.begin ();
  it_last_line += iLastLine;

//  iVpixel = iFirstLine * m_iLineHeight;
  iVpixel = 0;

  // draw each line
  for (deque <CPaneLine *>::iterator line_it = it_first_line ; 
       line_it != it_last_line; 
       line_it++)
    {

    iHpixel = 0;

    // draw each style in the line

    for (CPaneStyleVector::iterator style_it = (*line_it)->m_vStyles.begin (); 
         style_it != (*line_it)->m_vStyles.end (); 
         style_it++)
      {

      // set the colours
      pDC->SetTextColor((*style_it)->m_cText);
      pDC->SetBkColor  ((*style_it)->m_cBack);
      // choose correct font style (eg. bold)
      pDC->SelectObject (*m_font [(*style_it)->m_iStyle & 0x07]);        

      // note, if you use  ETO_OPAQUE and a NULL rectangle, it seems to
      // decide not to draw every 7th line or so, for some reason. ;)

      // draw the text
      pDC->ExtTextOut( iHpixel, 
                       iVpixel, 
                       0, //ETO_OPAQUE,
                       NULL,
                      (*style_it)->m_sText.c_str (),
                      (*style_it)->m_sText.length (),
                      NULL);  // lpDxWidths
      
    // find exact width of this text 

      CSize textsize = pDC->GetTextExtent (
                        (*style_it)->m_sText.c_str (),
                        (*style_it)->m_sText.length ());

      // the next style needs to be so many pixels further on ...
      iHpixel += textsize.cx;

      }   // end of doing each style

    // find next line start pixel
    iVpixel += m_iLineHeight;

    }  // end of doing each line

}

/////////////////////////////////////////////////////////////////////////////
// CPaneView diagnostics

#ifdef _DEBUG
void CPaneView::AssertValid() const
{
	CView::AssertValid();
}

void CPaneView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPaneView message handlers

// add a line to the output window
CPaneLine * CPaneView::AddLine ()
  {

  // add to output buffer
  CPaneLine * line = new CPaneLine;
  m_lines.push_back (line);
  
  // keep lines within buffer limit
  if (m_lines.size () > m_iMaxLines)
    m_lines.pop_front ();

  /*
  // how big is our client window, eh?
  int w, h;
  GetClientSize (&w, &h);

  // we will scroll to the last line, except the last screenfull
  Scroll (-1, m_lines.size () - (h / m_iLineHeight));

  */

  Invalidate ();    // TESTING

//  ScrollWindow (0, - m_iLineHeight);

  /*
  SCROLLINFO ScrollInfo;
  GetScrollInfo (SB_VERT, &ScrollInfo, SIF_ALL);
  ScrollInfo.nMin = 0;
  ScrollInfo.nMax = m_lines.size ();

  RECT r;
  GetClientRect (&r);
  int iLines = ((r.bottom - r.top) / m_iLineHeight) + 2;
  ScrollInfo.nPage = iLines;
  SetScrollInfo (SB_VERT, &ScrollInfo, SIF_ALL);

*/

  RECT r;
  GetClientRect (&r);
  int iLines = ((r.bottom - r.top) / m_iLineHeight);

  int iPos = m_lines.size () - iLines;
  if (iPos < 0)
    iPos = 0;

  SetScrollRange (SB_VERT, 0, iPos, FALSE);
  SetScrollPos (SB_VERT, iPos);

  return line;
  }  // end of CPaneView::::AddLine


// put text in the output window, start a new line if required

/*

The complexity here is that we want to wrap at a space, however the space might
be a few styles back. eg. if R represents Red and G represents Green, and we have
a line like this:

  test RGRGRGRGRGRGRGRGRGRGRG

We need to "unwind" a heap of styles from the rear of the line until we find a space,
if indeed we ever do.

This routine has preliminary UTF-8 support, in that it counts (or tries to) characters
rather than bytes.

*/

void CPaneView::AddText (const string sText,    // what to add
                         const COLORREF & cText, // text colour
                         const COLORREF & cBack, // background colour
                         const int iStyle,       // style (eg. italic)
                         const bool bNewLine)    // end with newline
  {
  string sTextToGo = sText;

  // find current line
  CPaneLine * line = GetLastLine ();

  // if ended with newline, or new type of line, start new one
  if (!line || line->GetNewline ())
    line = AddLine ();

  while (!sTextToGo.empty ())
    {

    // find current line
    CPaneLine * line = GetLastLine ();

    // how much room have we got?
    int iCharactersToGo = m_iMaxWidth - line->m_iWidth;

    // can't add a negative amount
    if (iCharactersToGo < 0)
      iCharactersToGo = 0;

    // find how many *characters* to go
    int iTextToGoSize = sTextToGo.size ();
    if (m_bUTF8)
       iTextToGoSize = utf8_strlen (sTextToGo.c_str ());

    // for substr etc. keep within range of what we actually have
    if (iCharactersToGo > iTextToGoSize)
       iCharactersToGo =  iTextToGoSize;

    // find which byte this UTF8 character starts at
    unsigned int iOffset = CharacterOffset (sTextToGo.c_str (), iCharactersToGo, m_bUTF8);
    
    // add the new text - do this first in case we can fit some of it
    line->AddStyle (CPaneStyle (sTextToGo.substr (0, iOffset), cText, cBack, iStyle)); 

    // compute new width
    line->m_iWidth += iCharactersToGo;
    
    // this much less next time
    sTextToGo.erase (0, iOffset);

    // less than max width? no problem :)
    if (line->m_iWidth < m_iMaxWidth)
      break;

    // if too wide with the new text we will need a line break somewhere
    string::size_type iSpacePos = string::npos;

    CPaneStyleVector::reverse_iterator style_it;

    // work backwards through styles until we find a space
    for (style_it = line->m_vStyles.rbegin ();
         style_it != line->m_vStyles.rend ();
         style_it++)
           {
           // get last space
           iSpacePos = (*style_it)->m_sText.find_last_of (' ');

           if (iSpacePos != string::npos)
             break; // found one!

           }  // end of working backwards through styles


    CPaneLine * new_line = AddLine (); // this will be the overflow line

    // we need at least one space, or there is no point in splitting the line ...

    if (iSpacePos != string::npos)
      {
      iSpacePos++;    // include the space, or we get into infinite loops

      // copy styles to new line
      // we subtract one to go back to the correct iterator spot
      //   see Josuttis p 267.

      copy (style_it.base () - 1, line->m_vStyles.end (), 
            back_inserter (new_line->m_vStyles));

      // remove from original line
      line->m_vStyles.erase (style_it.base () - 1, line->m_vStyles.end ());

      // now the new line has one style that needs splitting

      // get a copy
      CPaneStyle last_style = *(new_line->m_vStyles.front ());

      // erase past space
      last_style.m_sText.erase (iSpacePos, last_style.m_sText.size ());

      // put back partial run
      line->AddStyle (last_style); 

      // remove front bit from new line

      (new_line->m_vStyles.front ())->m_sText.erase (0, iSpacePos);

      // recompute widths of both lines
      new_line->CalculateWidth ();
      line->CalculateWidth ();

      }   // end of having a space somewhere in the line

    // new line for the rest of the text
    line = new_line;

    }; // end of text to be added

  if (bNewLine)
    MarkLastLineNewLine ();

  };  // end of CPaneView::AddText 

void CPaneView::OnDestroy() 
{
	CView::OnDestroy();
	
	CMUSHclientDoc* pDoc = (CMUSHclientDoc*) GetDocument();

  PaneMapIterator pit = pDoc->m_PaneMap.find (m_sName);
  if (pit == pDoc->m_PaneMap.end ())
    return;

  // remove from document's pane map, if possible
  pDoc->m_PaneMap.erase (pit);
  
}

BOOL CPaneView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style &= ~(FWS_ADDTOTITLE | FWS_PREFIXTITLE);
  cs.style	|= WS_VSCROLL;

  BOOL ret = CView::PreCreateWindow(cs);

  return ret;
}

BOOL CPaneView::OnEraseBkgnd(CDC* pDC) 
{


RECT r;
  pDC->GetClipBox (&r);
  pDC->FillSolidRect (&r, m_cBack);
  
  return TRUE;
	
}


void CPaneView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{

  RECT r;
  GetClientRect (&r);
  int iLines = ((r.bottom - r.top) / m_iLineHeight);

  int iMaxPos = m_lines.size () - iLines;
  if (iMaxPos < 0)
    iMaxPos = 0;

  // find where we are if required
  if (nSBCode != SB_THUMBTRACK &&
      nSBCode != SB_THUMBPOSITION)
    nPos = GetScrollPos (SB_VERT);

  switch (nSBCode)
    {
    case SB_THUMBTRACK:   // Drag scroll box to specified position. The current position is provided in nPos.
    case SB_THUMBPOSITION:// Scroll to the absolute position. The current position is provided in nPos.
          break;          // all done below

    case SB_BOTTOM:       //  Scroll to bottom - take max position
          nPos = iMaxPos;
          break;

    case SB_ENDSCROLL:    //  End scroll.
          // what does this do? nothing?
          break;

    case SB_LINEDOWN:     //  Scroll one line down.
          if (nPos < iMaxPos)
            nPos++;
          break;

    case SB_LINEUP:       //  Scroll one line up.
          if (nPos > 0)
            nPos--;
          break;

    case SB_PAGEDOWN:     //  Scroll one page down.
          nPos += iLines;
          if (nPos > iMaxPos)
            nPos = iMaxPos;
          break;

    case SB_PAGEUP:       //  Scroll one page up.
          nPos -= iLines;
          if (nPos < 0)
            nPos = 0;
          break;

    case SB_TOP:          //  Scroll to top - take mininum position
          nPos = 0;
          break;

    } // end of switch


  // set to new position
  SetScrollPos (SB_VERT, nPos);
  // redraw eventually
  Invalidate ();

}

void CPaneView::Clear ()
  {
  // delete line deque
  for (deque <CPaneLine *>::iterator line_it = m_lines.begin (); 
       line_it != m_lines.end (); 
       line_it++)
    delete *line_it;

  m_lines.clear ();
  Invalidate ();
  }


BOOL CPaneView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{

unsigned int iScrollLines;


    /*
     * Don't handle zoom and datazoom.
     */
    
  if (nFlags & MK_CONTROL || nFlags & MK_SHIFT)	
	  return CView::OnMouseWheel(nFlags, zDelta, pt);

  SystemParametersInfo (SPI_GETWHEELSCROLLLINES, 0, &iScrollLines, 0);

  if (iScrollLines == 0)
    return 0;   // scrolling not enabled

  if (iScrollLines == WHEEL_PAGESCROLL)
    {
    if (zDelta < 0)   // page down
      OnVScroll(SB_PAGEDOWN, 0, NULL);
    else
      OnVScroll(SB_PAGEUP, 0, NULL);

    return 1;
    }

  // scroll lines not pages

  if (zDelta < 0) // line down
    {
    zDelta = 0 - zDelta;  // make positive
    zDelta /= WHEEL_DELTA;
    zDelta *= iScrollLines;

    for (int i = 0; i < zDelta; i++)
      OnVScroll(SB_LINEDOWN, 0, NULL);

    return 1;
    }

  // line up

  zDelta /= WHEEL_DELTA;
  zDelta *= iScrollLines;


  for (int i = 0; i < zDelta; i++)
      OnVScroll(SB_LINEUP, 0, NULL);

  return 1;
}


void CPaneView::OnGotoEnd() 
{
  OnVScroll(SB_BOTTOM, 0, NULL);	
}

void CPaneView::OnLinedown() 
{
  OnVScroll(SB_LINEDOWN, 0, NULL);	
}

void CPaneView::OnLineup() 
{
  OnVScroll(SB_LINEUP, 0, NULL);
	
}

void CPaneView::OnPagedown() 
{
  OnVScroll(SB_PAGEDOWN, 0, NULL);	
}

void CPaneView::OnPageup() 
{
  OnVScroll(SB_PAGEUP, 0, NULL);	
}

void CPaneView::OnGotoStart() 
{
  OnVScroll(SB_TOP, 0, NULL);	
}
#endif