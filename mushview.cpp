// MUSHview.cpp : implementation of the CMUSHView class
//

// ANSI test in PennMUSH: think [ansi(cR,Nick)][ansi(wB,Gammon)][ansi(u,Underscored)]

#include "stdafx.h"
#include "MUSHclient.h"

#include "doc.h"
#include "MUSHview.h"
#include "mainfrm.h"
#include "childfrm.h"
#include "sendvw.h"

#include "ActivityDoc.h"
#include "genprint.h"
#include "dialogs\TextAttributesDlg.h"
#include "dialogs\GoToLineDlg.h"
#include "dialogs\HighlightPhraseDlg.h"
#include "dialogs\MultiLineTriggerDlg.h"
#include "scripting\errors.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define REDRAW_DEBUG 0

#if REDRAW_DEBUG

  #define TYPE_OF_RECT_FILL FillRect
  // #define TYPE_OF_RECT_FILL FrameRect
  #define TYPE_OF_RGN_FILL FillRgn
  // #define TYPE_OF_RGN_FILL FrameRgn

  void ShowInvalidatedRect (CMUSHView * pView, RECT & r, 
                            COLORREF colour = RGB (0, 255, 255))  // cyan?
    {
    CClientDC dc(pView);
    CBrush br;
    br.CreateSolidBrush (colour);    
    dc.TYPE_OF_RECT_FILL (&r, &br); 
    Sleep (500);    // pause so we can see it  
    }  // end of ShowInvalidatedRect

  void ShowInvalidatedRgn (CMUSHView * pView, CRgn & r)
    {
    CClientDC dc(pView);
    CBrush br;
    br.CreateSolidBrush (RGB (0, 255, 255));    // cyan?
    dc.TYPE_OF_RGN_FILL (&r, &br); 
    Sleep (500);    // pause so we can see it  
    }  // end of ShowInvalidatedRgn

#endif  // REDRAW_DEBUG
 

#define TTI_NONE     0 // no icon
#define TTI_INFO     1 // information icon
#define TTI_WARNING  2 // warning icon
#define TTI_ERROR    3 // error icon 

/*
  JMC - "redefinition of macro" warnings fixed
    These four are already defined with the same values
	in commctrl.h (Platform SDK header)
    TTM_SETTITLEA, TTM_SETTITLEW,
	TTM_POPUP, TTM_GETTITLE
*/
#ifndef TTM_SETTITLEA
  #define TTM_SETTITLEA           (WM_USER+32)
#endif

#define TTM_SETTITLE            TTM_SETTITLEA
#define TTM_SETWINDOWTHEME      CCM_SETWINDOWTHEME


CString strMXP_menu_item [MXP_MENU_COUNT];

#define ACTION_ALIAS 1    // dummy action type for alias menus

// for conversion to Unicode
static WCHAR sUnicodeText [MAX_LINE_WIDTH];

int iAction = 0;

/////////////////////////////////////////////////////////////////////////////
// CMUSHView

IMPLEMENT_DYNCREATE(CMUSHView, CView)

BEGIN_MESSAGE_MAP(CMUSHView, CView)
  ON_WM_CONTEXTMENU()
  //{{AFX_MSG_MAP(CMUSHView)
  ON_COMMAND(ID_TEST_END, OnTestEnd)
  ON_COMMAND(ID_TEST_PAGEDOWN, OnTestPagedown)
  ON_COMMAND(ID_TEST_PAGEUP, OnTestPageup)
  ON_COMMAND(ID_TEST_START, OnTestStart)
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()
  ON_WM_SETCURSOR()
  ON_COMMAND(ID_TEST_LINEDOWN, OnTestLinedown)
  ON_COMMAND(ID_TEST_LINEUP, OnTestLineup)
  ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
  ON_WM_ERASEBKGND()
  ON_COMMAND(ID_KEYS_ACTIVATECOMMANDVIEW, OnKeysActivatecommandview)
  ON_WM_SIZE()
  ON_COMMAND(ID_DISPLAY_FREEZEOUTPUT, OnDisplayFreezeoutput)
  ON_UPDATE_COMMAND_UI(ID_DISPLAY_FREEZEOUTPUT, OnUpdateDisplayFreezeoutput)
  ON_UPDATE_COMMAND_UI(ID_STATUSLINE_FREEZE, OnUpdateStatuslineFreeze)
  ON_COMMAND(ID_WINDOW_MINIMIZE, OnWindowMinimize)
  ON_UPDATE_COMMAND_UI(ID_WINDOW_MINIMIZE, OnUpdateWindowMinimize)
  ON_WM_CHAR()
  ON_WM_SYSCHAR()
  ON_COMMAND(ID_DISPLAY_FIND, OnDisplayFind)
  ON_COMMAND(ID_DISPLAY_FINDAGAIN, OnDisplayFindagain)
  ON_COMMAND(ID_FILE_PRINT_WORLD, OnFilePrintWorld)
  ON_WM_VSCROLL()
  ON_COMMAND(ID_FILE_SAVESELECTION, OnFileSaveselection)
  ON_COMMAND(ID_FIND_AGAIN_BACKWARDS, OnFindAgainBackwards)
  ON_UPDATE_COMMAND_UI(ID_FIND_AGAIN_BACKWARDS, OnUpdateFindAgainBackwards)
  ON_COMMAND(ID_FIND_AGAIN_FORWARDS, OnFindAgainForwards)
  ON_UPDATE_COMMAND_UI(ID_FIND_AGAIN_FORWARDS, OnUpdateFindAgainForwards)
  ON_COMMAND(ID_DISPLAY_GOTOURL, OnDisplayGotourl)
  ON_UPDATE_COMMAND_UI(ID_DISPLAY_GOTOURL, OnUpdateNeedSelection)
  ON_COMMAND(ID_DISPLAY_SENDMAILTO, OnDisplaySendmailto)
  ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
  ON_WM_LBUTTONDBLCLK()
  ON_COMMAND(ID_DISPLAY_GOTOBOOKMARK, OnDisplayGotobookmark)
  ON_COMMAND(ID_DISPLAY_BOOKMARKSELECTION, OnDisplayBookmarkselection)
  ON_WM_MOUSEWHEEL()
  ON_COMMAND(ID_ALT_UPARROW, OnAltUparrow)
  ON_COMMAND(ID_ALT_DOWNARROW, OnAltDownarrow)
  ON_COMMAND(ID_DISPLAY_TEXTATTRIBUTES, OnDisplayTextattributes)
  ON_COMMAND(ID_EDIT_NOTESWORKAREA, OnEditNotesworkarea)
  ON_COMMAND(ID_DISPLAY_RECALLTEXT, OnDisplayRecalltext)
  ON_WM_KEYDOWN()
  ON_WM_KEYUP()
  ON_WM_SYSKEYDOWN()
  ON_WM_SYSKEYUP()
  ON_COMMAND(ID_DISPLAY_GOTOLINE, OnDisplayGotoline)
  ON_COMMAND(ID_GAME_COMMANDHISTORY, OnGameCommandhistory)
  ON_COMMAND(ID_FILE_CTRL_N, OnFileCtrlN)
  ON_COMMAND(ID_FILE_CTRL_P, OnFileCtrlP)
  ON_COMMAND(ID_EDIT_CTRL_Z, OnEditCtrlZ)
  ON_COMMAND(ID_EDIT_COPYASHTML, OnEditCopyashtml)
  ON_COMMAND(ID_DISPLAY_HIGHLIGHTPHRASE, OnDisplayHighlightphrase)
  ON_COMMAND(ID_DISPLAY_MULTILINETRIGGER, OnDisplayMultilinetrigger)
  ON_COMMAND(ID_WINDOW_MAXIMIZE, OnWindowMaximize)
  ON_COMMAND(ID_WINDOW_RESTORE, OnWindowRestore)
  ON_WM_SYSCOMMAND()
  ON_WM_RBUTTONUP()
  ON_WM_RBUTTONDOWN()
  ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateNeedSelectionForCopy)
  ON_UPDATE_COMMAND_UI(ID_FILE_SAVESELECTION, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(ID_DISPLAY_SENDMAILTO, OnUpdateNeedSelection)
  ON_COMMAND(ID_KEYS_TAB, OnKeysActivatecommandview)
  ON_COMMAND(ID_KEYS_ESCAPE, OnKeysActivatecommandview)
  ON_UPDATE_COMMAND_UI(ID_DISPLAY_TEXTATTRIBUTES, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(ID_EDIT_COPYASHTML, OnUpdateNeedSelectionForCopy)
  ON_WM_RBUTTONDBLCLK()
  //}}AFX_MSG_MAP

  // macros
  ON_COMMAND_EX(ID_MACRO_CTRL_F10, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_CTRL_F11, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_CTRL_F12, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_CTRL_F2, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_CTRL_F3, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_CTRL_F5, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_CTRL_F7, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_CTRL_F8, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_CTRL_F9, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_F10, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_F11, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_F12, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_F2, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_F3, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_F4, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_F5, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_F7, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_F8, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_F9, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_SHIFT_F10, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_SHIFT_F11, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_SHIFT_F12, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_SHIFT_F2, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_SHIFT_F3, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_SHIFT_F4, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_SHIFT_F5, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_SHIFT_F7, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_SHIFT_F8, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_SHIFT_F9, OnMacroCommand)
  ON_COMMAND_EX(ID_ALT_A, OnMacroCommand)
  ON_COMMAND_EX(ID_ALT_B, OnMacroCommand)
  ON_COMMAND_EX(ID_ALT_J, OnMacroCommand)
  ON_COMMAND_EX(ID_ALT_K, OnMacroCommand)
  ON_COMMAND_EX(ID_ALT_L, OnMacroCommand)
  ON_COMMAND_EX(ID_ALT_M, OnMacroCommand)
  ON_COMMAND_EX(ID_ALT_N, OnMacroCommand)
  ON_COMMAND_EX(ID_ALT_O, OnMacroCommand)
  ON_COMMAND_EX(ID_ALT_P, OnMacroCommand)
  ON_COMMAND_EX(ID_ALT_Q, OnMacroCommand)
  ON_COMMAND_EX(ID_ALT_R, OnMacroCommand)
  ON_COMMAND_EX(ID_ALT_S, OnMacroCommand)
  ON_COMMAND_EX(ID_ALT_T, OnMacroCommand)
  ON_COMMAND_EX(ID_ALT_U, OnMacroCommand)
  ON_COMMAND_EX(ID_ALT_X, OnMacroCommand)
  ON_COMMAND_EX(ID_ALT_Y, OnMacroCommand)
  ON_COMMAND_EX(ID_ALT_Z, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_F1, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_CTRL_F1, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_SHIFT_F1, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_F6, OnMacroCommand)
  ON_COMMAND_EX(ID_MACRO_CTRL_F6, OnMacroCommand)

  // numeric keypad  
  ON_COMMAND_EX(ID_KEYPAD_0, OnKeypadCommand)
  ON_COMMAND_EX(ID_KEYPAD_1, OnKeypadCommand)
  ON_COMMAND_EX(ID_KEYPAD_2, OnKeypadCommand)
  ON_COMMAND_EX(ID_KEYPAD_3, OnKeypadCommand)
  ON_COMMAND_EX(ID_KEYPAD_4, OnKeypadCommand)
  ON_COMMAND_EX(ID_KEYPAD_5, OnKeypadCommand)
  ON_COMMAND_EX(ID_KEYPAD_6, OnKeypadCommand)
  ON_COMMAND_EX(ID_KEYPAD_7, OnKeypadCommand)
  ON_COMMAND_EX(ID_KEYPAD_8, OnKeypadCommand)
  ON_COMMAND_EX(ID_KEYPAD_9, OnKeypadCommand)
  ON_COMMAND_EX(ID_KEYPAD_DASH, OnKeypadCommand)
  ON_COMMAND_EX(ID_KEYPAD_DOT, OnKeypadCommand)
  ON_COMMAND_EX(ID_KEYPAD_PLUS, OnKeypadCommand)
  ON_COMMAND_EX(ID_KEYPAD_SLASH, OnKeypadCommand)
  ON_COMMAND_EX(ID_KEYPAD_STAR, OnKeypadCommand)
  ON_COMMAND_EX(ID_CTRL_KEYPAD_0, OnKeypadCommand)
  ON_COMMAND_EX(ID_CTRL_KEYPAD_1, OnKeypadCommand)
  ON_COMMAND_EX(ID_CTRL_KEYPAD_2, OnKeypadCommand)
  ON_COMMAND_EX(ID_CTRL_KEYPAD_3, OnKeypadCommand)
  ON_COMMAND_EX(ID_CTRL_KEYPAD_4, OnKeypadCommand)
  ON_COMMAND_EX(ID_CTRL_KEYPAD_5, OnKeypadCommand)
  ON_COMMAND_EX(ID_CTRL_KEYPAD_6, OnKeypadCommand)
  ON_COMMAND_EX(ID_CTRL_KEYPAD_7, OnKeypadCommand)
  ON_COMMAND_EX(ID_CTRL_KEYPAD_8, OnKeypadCommand)
  ON_COMMAND_EX(ID_CTRL_KEYPAD_9, OnKeypadCommand)
  ON_COMMAND_EX(ID_CTRL_KEYPAD_DASH, OnKeypadCommand)
  ON_COMMAND_EX(ID_CTRL_KEYPAD_DOT, OnKeypadCommand)
  ON_COMMAND_EX(ID_CTRL_KEYPAD_PLUS, OnKeypadCommand)
  ON_COMMAND_EX(ID_CTRL_KEYPAD_SLASH, OnKeypadCommand)
  ON_COMMAND_EX(ID_CTRL_KEYPAD_STAR, OnKeypadCommand)
  // Standard printing commands
  ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
  ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)

  // MXP pop-up menu
  ON_COMMAND_RANGE(MXP_FIRST_MENU, 
                    MXP_FIRST_MENU + MXP_MENU_COUNT - 1, 
                    OnMXPMenu)

  // Accelerator commands
  ON_COMMAND_RANGE(ACCELERATOR_FIRST_COMMAND, 
                   ACCELERATOR_FIRST_COMMAND + ACCELERATOR_COUNT - 1, 
                   OnAcceleratorCommand)

  END_MESSAGE_MAP()

//#define new DEBUG_NEW 

/////////////////////////////////////////////////////////////////////////////
// CMUSHView construction/destruction

CMUSHView::CMUSHView()
{

  m_selstart_line = 0;  
  m_selstart_col = 0;
  m_selend_line = 0;
  m_selend_col = 0;
  m_freeze = FALSE;
  m_bAtBufferEnd = false;
  m_last_line_drawn = 0;
  m_last_line = 0;
  m_nLastToolTipLine = 0;
  m_nLastToolTipColumn = 0;
  m_scroll_position = 0;
  m_mousedover = false;

  m_iPauseStatus = ePauseUninitialised;

}

CMUSHView::~CMUSHView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CMUSHView drawing

// for extracting parts of the style out - see copy in doc.cpp

#if 0
static inline short get_foreground (int style)
  {

  if (style & CUSTOM)
    {
    int iColour = (style >> 4) & 0x0FF;  // we have up to 256 custom colours
    if (iColour >= MAX_CUSTOM)
      iColour = 0;
    return iColour;   
    }
  else
    return (style >> 4) & 0x07;


  } // end of get_foreground

static inline short get_background (int style)
  {

  if (style & CUSTOM)
    return 0;   // custom colour backgrounds are defined as part of the colour
  else
    return (style >> 8) & 0x07;

  } // end of get_background

#endif

long CMUSHView::calculate_width (const int & line, 
                       const int len, 
                       CMUSHclientDoc* pDoc, 
                       CClientDC & dc)
  {
int thiscol = 0;
int cols_to_go = len;
int thislen;
CSize textsize;
long pixel = 0;
POSITION pos;
CLine * pLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (line));

  if (len <= 0)
    return 0;

  CStyle * pStyle;

  // do each batch of styles separately
  for (pos = pLine->styleList.GetHeadPosition(); pos && cols_to_go > 0; )
    {
    pStyle = pLine->styleList.GetNext (pos);

    if (pStyle->iLength <= 0)
      continue;   // ignore zero-length runs

    // don't overshoot requested column
    thislen = MIN (cols_to_go, pStyle->iLength);

// select appropriate font

    if (pDoc->m_font [pStyle->iFlags & 7])
      dc.SelectObject(pDoc->m_font [pStyle->iFlags & 7]);   


    if (pDoc->m_bUTF_8)
      {
      int iUnicodeCharacters = MultiByteToWideChar (CP_UTF8, 0,     // do not use flag MB_PRECOMPOSED - doesn't work!
                            &pLine->text [thiscol], thislen,  // input
                            sUnicodeText, MAX_LINE_WIDTH);         // output

      // now calculate width of Unicode pixels
      GetTextExtentPoint32W(
          dc.m_hDC,           // handle to device context
          sUnicodeText,  // pointer to text string
          iUnicodeCharacters,      // number of characters in string
          &textsize      // pointer to structure for string size
        );
 
      }
    else
      {


    // find exact width of this text (Ascii)

        textsize = dc.GetTextExtent (
                          &pLine->text [thiscol],
                          thislen);
      }

    thiscol += thislen;     // new column
    cols_to_go -= thislen;  // fewer in next batch
    pixel += textsize.cx;   // count pixels

    }   // end of this style


  return pixel;

  } // end of calculate_width


/*
   So that italic text doesn't get chopped off at the RH boundary we will draw each
   line twice - once for the background, and then redraw over it transparently for the
   text.

  */
void CMUSHView::display_text (CDC* pDC, 
                             const CMUSHclientDoc* pDoc, 
                             const long line,
                             const CLine * pLine, 
                             const int col, 
                             const int len, 
                             const BOOL selected,
                             long & pixel,
                             const bool bBackground)
  {
RECT r;

COLORREF colour1,
         colour2,
         clrBackground;


  if (len <= 0)
    return;

  pDC->SetBkMode (bBackground ? OPAQUE : TRANSPARENT);

int thiscol = col;
int cols_to_go = len;
int thislen;
CSize textsize;
CStyle * pStyle;
int iCol = 0;
POSITION foundpos;


  if (!pDoc->FindStyle (pLine, col, iCol, pStyle, foundpos))
    return;

  // get next style in sequence
  pLine->styleList.GetNext (foundpos);  // skip one we had

  // how much do we do of this style?
  thislen = iCol - col;

  while (thiscol < (col + len))
    {

    // don't overshoot
    thislen = MIN (cols_to_go, thislen);

    if (pDoc->m_font [pStyle->iFlags & 7])
      pDC->SelectObject(pDoc->m_font [pStyle->iFlags & 7]);

    pDoc->GetStyleRGB (pStyle, colour1, colour2);

    if (selected)
      {
      pDC->SetTextColor (pDoc->TranslateColour (colour2));  
      clrBackground = colour1;
      }
    else
      {
      pDC->SetTextColor (pDoc->TranslateColour (colour1));  
      clrBackground = colour2;
      }


int iUnicodeCharacters = 0;

// unicode conversion from UTF-8

    if (pDoc->m_bUTF_8)
      {
      iUnicodeCharacters = MultiByteToWideChar (CP_UTF8, 0,     // do not use flag MB_PRECOMPOSED - doesn't work!
                            &pLine->text [thiscol], thislen,  // input
                            sUnicodeText, MAX_LINE_WIDTH);         // output

      // now calculate width of Unicode pixels
      GetTextExtentPoint32W(
          pDC->m_hDC,           // handle to device context
          sUnicodeText,  // pointer to text string
          iUnicodeCharacters,      // number of characters in string
          &textsize      // pointer to structure for string size
        );
 
      }
    else
      {


    // find exact width of this text (Ascii)

        textsize = pDC->GetTextExtent (
                          &pLine->text [thiscol],
                          thislen);
      }


    SetRect (&r, 
             pDoc->m_iPixelOffset + pixel,
             - pDoc->m_iPixelOffset + line * pDoc->m_FontHeight,
             pDoc->m_iPixelOffset + pixel + textsize.cx,
             - pDoc->m_iPixelOffset + (line + 1) * pDoc->m_FontHeight);
            
    OffsetRect (&r, -m_scroll_position.x, -m_scroll_position.y);

    OffsetRect (&r, pDoc->m_TextRectangle.left, pDoc->m_TextRectangle.top);

    if (bBackground)
      {
      COLORREF b;
      if (pDoc->m_bCustom16isDefaultColour)
        b = pDoc->m_customback [15];
      else
        b = pDoc->m_normalcolour [BLACK];

      if (clrBackground != b)
        pDC->FillSolidRect (&r, pDoc->TranslateColour (clrBackground));
      }
    else
      {
      if (pDoc->m_bUTF_8)     // Unicode output
        ExtTextOutW(          // W = wide
                  pDC->m_hDC,          
                  r.left,   // pDoc->m_iPixelOffset + pixel - m_scroll_position.x,  
                  r.top, // - pDoc->m_iPixelOffset + line * pDoc->m_FontHeight - m_scroll_position.y,            
                  0,      // transparent
                  &r, 
                  sUnicodeText, 
                  iUnicodeCharacters,     
                  NULL);
      else                   // Ascii output
        pDC->ExtTextOut (
                    r.left,  // pDoc->m_iPixelOffset + pixel - m_scroll_position.x, 
                   r.top,    //- pDoc->m_iPixelOffset + line * pDoc->m_FontHeight - m_scroll_position.y,
                   0,   // transparent
                   &r,  
                   &pLine->text [thiscol], 
                   thislen,
                   NULL);

      }
    thiscol += thislen;
    cols_to_go -= thislen;  // this many fewer to go
    pixel += textsize.cx;

    if (!foundpos)
      break;  // no more styles

    pStyle = pLine->styleList.GetNext (foundpos);  // and get next one
    thislen = pStyle->iLength;                      

    } // end of this group of the same colour


  }  // end of CMUSHView::display_text 


// see: http://www.codeguru.com/Cpp/misc/misc/flickerfreedrawing/article.php/c389

//////////////////////////////////////////////////
// CMemDC - memory DC
//
// Author: Keith Rule
// Email:  keithr@europa.com
// Copyright 1996-1997, Keith Rule
//
// You may freely use or modify this code provided this
// Copyright is included in all derived versions.
//
// History - 10/3/97 Fixed scrolling bug.
//                   Added print support.
//
// This class implements a memory Device Context

class CMyMemDC : public CDC {
private:
  CBitmap m_bitmap; // Offscreen bitmap
  CBitmap* m_oldBitmap; // bitmap originally found in CMyMemDC
  CDC* m_pDC; // Saves CDC passed in constructor
  CRect m_rect; // Rectangle of drawing area.
  BOOL m_bMemDC; // TRUE if CDC really is a Memory DC.
public:
  CMyMemDC(CDC* pDC, RECT rect) : CDC(), m_oldBitmap(NULL), m_pDC(pDC), m_rect (rect)
  {
    ASSERT(m_pDC != NULL); // If you asserted here, you passed in a NULL CDC.

    m_bMemDC = !pDC->IsPrinting();

    if (m_bMemDC){
      // Create a Memory DC
      CreateCompatibleDC(pDC);
      m_bitmap.CreateCompatibleBitmap(pDC, m_rect.Width(), m_rect.Height());
      m_oldBitmap = SelectObject(&m_bitmap);
      SetWindowOrg(m_rect.left, m_rect.top);
    } else {
      // Make a copy of the relevent parts of the current DC for printing
      m_bPrinting = pDC->m_bPrinting;
      m_hDC = pDC->m_hDC;
      m_hAttribDC = pDC->m_hAttribDC;
    }
  }

  ~CMyMemDC()
  {
    if (m_bMemDC) {
      // Copy the offscreen bitmap onto the screen.
      m_pDC->BitBlt(m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(),
        this, m_rect.left, m_rect.top, SRCCOPY);
      //Swap back the original bitmap.
      SelectObject(m_oldBitmap);
    } else {
      // All we need to do is replace the DC with an illegal value,
      // this keeps us from accidently deleting the handles associated with
      // the CDC that was passed to the constructor.
      m_hDC = m_hAttribDC = NULL;
    }
  }

  // Allow usage as a pointer
  CMyMemDC* operator->() {return this;}

  // Allow usage as a pointer
  operator CMyMemDC*() {return this;}
};

/* modes: 

  0 = strech to output view size 
  1 = stretch with aspect ratio

  2 = strech to owner size 
  3 = stretch with aspect ratio
  
  -- going clockwise here:

  -- top
  4 = top left
  5 = center left-right at top
  6 = top right

  -- rh side
  7 = on right, center top-bottom
  8 = on right, at bottom

  -- bottom
  9 = center left-right at bottom

  -- lh side
  10 = on left, at bottom  
  11 = on left, center top-bottom 

  -- middle
  12 = center all

  13 = tile


  */

void CMUSHView::Blit_Bitmap (CDC* pDestDC, CDC* pSrcDC, 
                             const long iWidth, const long iHeight, 
                             const short iMode,
                             CRect & rect,
                             const bool bUseRect,
                             const bool bTransparent,
                             const COLORREF TransparentColour)
  {

  if (iWidth <= 0 || iHeight <= 0)
    return;

  CChildFrame * pFrame = m_owner_frame;


  RECT r;

  if (iMode == 2 || iMode == 3)
    pFrame->GetClientRect (&r);
  else
    GetClientRect (&r);

  pDestDC->SetStretchBltMode (HALFTONE);  // looks better when squashed

  // they know where they want it to go
  if (bUseRect)
    {
    rect.right = iWidth;
    rect.bottom = iHeight;
    }
  else
    {
    switch (iMode)
      {
      case 0 :  // stretch
      case 2 :
         rect = CRect (0, 0, r.right, r.bottom);
         break;


      case 1:  // stretch with aspect ratio
      case 3:
        {
        double ratio = ((double) iWidth) / ((double) iHeight);
        rect = CRect (0, 0, r.bottom * ratio, r.bottom);
        break;

        }

      case 4: // top left
         rect = CRect (0, 0, iWidth, iHeight);
         break;


      case 5 :  // center left-right at top
         rect = CRect ((r.right - iWidth) / 2, 0, iWidth, iHeight);
         break;

      case 6 : // top right
         rect = CRect (r.right - iWidth, 0, iWidth, iHeight);
         break;


      case 7 :  // on right, center top-bottom
         rect = CRect (r.right - iWidth, (r.bottom - iHeight) / 2, iWidth, iHeight);
         break;


      case 8 :  // on right at bottom
         rect = CRect (r.right - iWidth, r.bottom - iHeight, iWidth, iHeight);
         break;

      case 9 :  // center left-right at bottom
         rect = CRect ((r.right - iWidth) / 2, r.bottom - iHeight, iWidth, iHeight);
         break;

      case 10 :  // on left, at bottom
         rect = CRect (0, r.bottom - iHeight, iWidth, iHeight);
         break;


      case 11 :  // on left, center top-bottom
         rect = CRect (0, (r.bottom - iHeight) / 2, iWidth, iHeight);

         break;

      case 12:  // center all
         rect = CRect ((r.right - iWidth) / 2, (r.bottom - iHeight) / 2, iWidth, iHeight);
         break;

      case 13 :  // tile
        {
        rect = r;
        int iAcross = (r.right / iWidth) + 1;
        int iDown = (r.bottom / iHeight) + 1;
        int x, y;

        for (x = 0; x < iAcross; x++)
          {
          for (y = 0; y < iDown; y++)
            {
             pDestDC->BitBlt (x * iWidth, y * iHeight, 
                          iWidth, iHeight, pSrcDC, 0, 0, SRCCOPY);

            }  // each y
          }  // each x
    
      

        }  //  tile

       return;  // don't blit again

      } // end switch
    } // end of not absolute position


  if (iMode <= 3 && !bUseRect)         // stretch (not for absolute rectangles)
     pDestDC->StretchBlt(rect.left, rect.top, rect.right, rect.bottom, pSrcDC, 0, 0, iWidth, iHeight, SRCCOPY);
  else                    
    {                    // normal
    if (bTransparent)
      {
      COLORREF crOldBack = pDestDC->SetBkColor (RGB (255, 255, 255));    // white
      COLORREF crOldText = pDestDC->SetTextColor (RGB (0, 0, 0));        // black
      CDC dcTrans;   // transparency mask


      // Create a memory dc for the mask
      dcTrans.CreateCompatibleDC(pDestDC);

      // Create the mask bitmap for the subset of the main image
      CBitmap bitmapTrans;
      bitmapTrans.CreateBitmap(iWidth, iHeight, 1, 1, NULL);

      // Select the mask bitmap into the appropriate dc
      CBitmap* pOldBitmapTrans = dcTrans.SelectObject(&bitmapTrans);

      // Set transparency colour
      COLORREF crOldBackground = pSrcDC->SetBkColor (TransparentColour);

      // Build mask based on transparent colour at location 0, 0
      dcTrans.BitBlt (0, 0, iWidth, iHeight, pSrcDC, 0, 0, SRCCOPY);

      // Do the work 
      pDestDC->BitBlt (rect.left, rect.top, iWidth, iHeight, pSrcDC, 0, 0, SRCINVERT);
      pDestDC->BitBlt (rect.left, rect.top, iWidth, iHeight, &dcTrans, 0, 0, SRCAND);
      pDestDC->BitBlt (rect.left, rect.top, iWidth, iHeight, pSrcDC, 0, 0, SRCINVERT);

      // Restore settings
      dcTrans.SelectObject(pOldBitmapTrans);
      pDestDC->SetBkColor(crOldBack);
      pDestDC->SetTextColor(crOldText);
      pSrcDC->SetBkColor(crOldBackground);
      }
    else    // not transparent
      pDestDC->BitBlt (rect.left, rect.top, rect.right, rect.bottom, pSrcDC, 0, 0, SRCCOPY);
    }
  
  // make into rectangle, not position and width/height
  rect.right += rect.left;
  rect.bottom += rect.top;

  } // end of CMUSHView::Blit_Bitmap


void CMUSHView::DrawImage (CDC* pDC, CBitmap & bitmap, const short iMode)
  {

  if ((HBITMAP) bitmap == NULL)
    return;


  CClientDC dc(this);
  CDC bmDC;
  bmDC.CreateCompatibleDC(&dc);
  CBitmap *pOldbmp = bmDC.SelectObject(&bitmap);

  BITMAP  bi;
  bitmap.GetBitmap(&bi);
  CRect r;

  Blit_Bitmap (pDC, &bmDC, bi.bmWidth, bi.bmHeight, iMode, r);

  bmDC.SelectObject(pOldbmp);


  } // end of CMUSHView::DrawImage


void CMUSHView::OnDraw(CDC* dc)
{

// this stuff stops the flicker when redawing stuff that is almost identical to last time
CRect rcBounds;
GetClientRect(&rcBounds);

CMyMemDC  pDC(dc, rcBounds);

int startline,
    endline,
    line,
    lastline,
    startcol,
    endcol;

long pixel;

CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

  if (!pDoc->m_FontHeight)
    return;

  // sigh. How many times did we do this?
  pDoc->m_iOutputWindowRedrawCount++;

  pDC->SelectClipRgn (NULL);

CBrush backbr;
CRect rect;

  // background colour
  if (pDoc->m_iBackgroundColour != NO_COLOUR)
    backbr.CreateSolidBrush (pDoc->m_iBackgroundColour);
  else if (pDoc->m_bCustom16isDefaultColour)
    backbr.CreateSolidBrush (pDoc->TranslateColour (pDoc->m_customback [15]));
  else
    backbr.CreateSolidBrush (pDoc->TranslateColour (pDoc->m_normalcolour [BLACK]));
  
RECT r;

  GetClientRect (&r);

  pDC->SetBkMode (OPAQUE);

  CRect TextRectangle = GetTextRectangle ();

  // do external fill
  if (HaveTextRectangle () && pDoc->m_TextRectangleOutsideFillStyle != BS_NULL)
    {
    CBrush br;

    ValidateBrushStyle (pDoc->m_TextRectangleOutsideFillStyle, 
                        pDoc->m_TextRectangleBorderColour, 
                        pDoc->m_TextRectangleOutsideFillColour, 
                        br);
    CPen pen;
    pen.CreatePen(PS_NULL, (int) 1, pDoc->m_TextRectangleBorderColour);
    CPen* oldPen = pDC->SelectObject(&pen);
    CBrush* oldBrush = pDC->SelectObject(&br);

    if (pDoc->m_TextRectangleOutsideFillStyle > 1 && pDoc->m_TextRectangleOutsideFillStyle <= 7)
      pDC->SetBkColor (pDoc->m_TextRectangleOutsideFillColour);      // for hatched brushes this is the background colour
    else
    if (pDoc->m_TextRectangleOutsideFillStyle > 7)  // pattern brushes
      {
      pDC->SetTextColor (pDoc->m_TextRectangleOutsideFillColour);      // for hatched brushes and patterned brushes
      pDC->SetBkColor (pDoc->m_TextRectangleBorderColour);      // for patterned brushes
      }

    pDC->Rectangle (&r);

    pDC->SelectObject (oldPen);
    pDC->SelectObject (oldBrush);

    CRect fillrect (GetTextRectangle ());
    fillrect.InflateRect (pDoc->m_TextRectangleBorderOffset, pDoc->m_TextRectangleBorderOffset);
    fillrect.InflateRect (pDoc->m_TextRectangleBorderWidth, pDoc->m_TextRectangleBorderWidth);
    pDC->FillRect (&fillrect, &backbr);

    }
  else
    pDC->FillRect (&r, &backbr);

  // background image 
  if ((HBITMAP) pDoc->m_BackgroundBitmap)
    DrawImage (pDC, pDoc->m_BackgroundBitmap, pDoc->m_iBackgroundMode); 

  if (HaveTextRectangle ())
    {

    if (pDoc->m_TextRectangleBorderWidth > 0) 
      {
      // create pen 
      CPen pen;
      pen.CreatePen (PS_SOLID, pDoc->m_TextRectangleBorderWidth, pDoc->m_TextRectangleBorderColour); 
  
      CBrush br;

      LOGBRUSH lb;
      lb.lbStyle = BS_NULL;
      br.CreateBrushIndirect (&lb);

      // select into DC
      CPen* oldPen = pDC->SelectObject(&pen);
      CBrush* oldBrush = pDC->SelectObject(&br);
      CRect r (TextRectangle);
      r.InflateRect (pDoc->m_TextRectangleBorderWidth, pDoc->m_TextRectangleBorderWidth);
      r.InflateRect (pDoc->m_TextRectangleBorderOffset, pDoc->m_TextRectangleBorderOffset);
      pDC->Rectangle (r);
      pDC->SelectObject (oldPen);
      pDC->SelectObject (oldBrush);
      }


    }    // end of text rectangle


  Calculate_MiniWindow_Rectangles (pDoc, true);

  MiniWindowMapIterator mwit;

  // mini windows  - on bottom
  for (mwit = pDoc->m_MiniWindows.begin (); 
       mwit != pDoc->m_MiniWindows.end ();
       mwit++)
         {
         CMiniWindow * mw = mwit->second;
         // not if not to be shown
         if (!mw->GetShow () || mw->m_bTemporarilyHide)
           continue;

         long iFlags = mw->GetFlags ();

         // not if wrong time
         if (!(iFlags & MINIWINDOW_DRAW_UNDERNEATH))
           continue;

         // blit onto screen
         Blit_Bitmap (pDC, mw->GetDC (), 
                      mw->GetWidth (), mw->GetHeight (), 
                      mw->GetPosition (), 
                      mw->m_rect, 
                      mw->GetPosition () != 13,  // absolute? - we precalculated so yes (except tile)
                      (iFlags & MINIWINDOW_TRANSPARENT) != 0,       // transparent?
                      mw->GetBackgroundColour ());

         }  // end for each window


  if (HaveTextRectangle ())
    {
    // clip to rectangle
    CRgn rgn;
    rgn.CreateRectRgn (TextRectangle.left, TextRectangle.top, TextRectangle.right, TextRectangle.bottom);
    pDC->SelectClipRgn (&rgn);
    }

  CPoint pt = GetScrollPosition ();
   

  startline = (pt.y - pDoc->m_iPixelOffset) / pDoc->m_FontHeight;
  endline =  ((pt.y + GetOutputWindowHeight ()) / pDoc->m_FontHeight);

  if (startline < 0)
    startline = 0;

// include partial last line if necessary

  lastline = pDoc->GetLastLine ();

  if (endline > lastline)
    endline = lastline;

  if (startline > lastline)
    startline = lastline - 20;

POSITION pos = pDoc->GetLinePosition (startline);
unsigned short style = 0;    // background (bleed) style
COLORREF iForeColour = WHITE;
COLORREF iBackColour = BLACK;

  if (pDoc->m_bCustom16isDefaultColour)
    {
    iForeColour = 15;   // use custom colour 16
    iBackColour = 15;
    style = COLOUR_CUSTOM;
    }

  for (line = startline; line < endline && pos; line++)
    {

// retrieve actual pointer to this line

    CLine * pLine = pDoc->m_LineList.GetNext (pos);

    // on user input or note, assume default background bleed if nothing there
    if (pLine->flags & NOTE_OR_COMMAND)
      {
      if (pDoc->m_bCustom16isDefaultColour)
        {
        iForeColour = 15;   // use custom colour 16
        iBackColour = 15;
        style = COLOUR_CUSTOM;
        }
      else
        {
        style = 0;
        iForeColour = WHITE;
        iBackColour = BLACK;
        }
      }

    startcol = MIN (m_selstart_col, pLine->len);
    endcol = MIN (m_selend_col, pLine->len);

    // Draw the background and then the text

    for (int iPass = 0; iPass < 2; iPass++)
      {
      bool bBackground = iPass == 0;

      pixel = 0;

  // show the selection in a different colour

      if (line == m_selstart_line && m_selstart_line == m_selend_line)
        {

  // selection starting and ending on the same line is a special case

          display_text (pDC, pDoc, line, pLine, 0, startcol, 
                        FALSE, pixel, bBackground);
          display_text (pDC, pDoc, line, pLine, startcol, endcol - startcol, 
                        TRUE, pixel, bBackground);
          display_text (pDC, pDoc, line, pLine, endcol, pLine->len - endcol, 
                        FALSE, pixel, bBackground);

        }
      else if (line == m_selstart_line)
          {

  // selection starts on this line

          display_text (pDC, pDoc, line, pLine, 0, startcol, 
                        FALSE, pixel, bBackground);
          display_text (pDC, pDoc, line, pLine, startcol, pLine->len - startcol, 
                        TRUE, pixel, bBackground);

          }
      else if (line == m_selend_line)
          {

  // selection ends on this line

          display_text (pDC, pDoc, line, pLine, 0, endcol, 
                        TRUE, pixel, bBackground);
          display_text (pDC, pDoc, line, pLine, endcol, pLine->len - endcol, 
                        FALSE, pixel, bBackground);

          }
      else
        {

        display_text (pDC, pDoc, line, pLine, 0, pLine->len, 
                      line >= m_selstart_line && line <= m_selend_line, pixel, bBackground);

        }  // end of selection not starting and ending on the same line

// bleed to edge if wanted
      
      if (bBackground && 
          App.m_bBleedBackground)
        {

        COLORREF b;
        if (pDoc->m_bCustom16isDefaultColour)
          b = pDoc->m_customback [15];
        else
          b = pDoc->m_normalcolour [BLACK];

        pDC->SetBkMode (OPAQUE);
        RECT r,
             cr;    // client rect
        COLORREF clrBackground;

        // -------- bleed to left --------

        // empty lines take background from previous line        
        if (pLine->len > 0)
          {
          CStyle * pStyle = pLine->styleList.GetHead ();
          style = pStyle->iFlags & STYLE_BITS;
          iForeColour = pStyle->iForeColour;
          iBackColour = pStyle->iBackColour;
          }

        // work out the colour
        if ((style & COLOURTYPE) == COLOUR_CUSTOM)
          {
          ASSERT (iForeColour >= 0 && iForeColour < MAX_CUSTOM);
          clrBackground = pDoc->m_customback [iForeColour];
          }
        else
        // RGB is just itself
        if ((style & COLOURTYPE) == COLOUR_RGB)
          if (style & INVERSE)    // inverse inverts foreground and background
            clrBackground = iForeColour;
          else
            clrBackground = iBackColour;
        else
          {
          ASSERT (iForeColour >= 0 && iForeColour < 8);
          ASSERT (iBackColour >= 0 && iBackColour < 8);
          if (style & INVERSE)    // inverse inverts foreground and background
            clrBackground = pDoc->m_normalcolour [iForeColour];
          else
            clrBackground = pDoc->m_normalcolour [iBackColour];
          }   // not custom

        SetRect (&r, 
                 0,
                 - pDoc->m_iPixelOffset + line * pDoc->m_FontHeight,
                 pDoc->m_iPixelOffset,
                 - pDoc->m_iPixelOffset + (line + 1) * pDoc->m_FontHeight);
            
        OffsetRect (&r, -m_scroll_position.x, -m_scroll_position.y);

        if (clrBackground != b)
          pDC->FillSolidRect (&r, pDoc->TranslateColour (clrBackground));

        // -------- bleed to right --------

        // empty lines take background from previous line        
        if (pLine->len > 0)
          {
          CStyle * pStyle = pLine->styleList.GetTail ();
          style = pStyle->iFlags & STYLE_BITS;
          iForeColour = pStyle->iForeColour;
          iBackColour = pStyle->iBackColour;
          }

        // work out the colour
        if ((style & COLOURTYPE) == COLOUR_CUSTOM)
          {
          ASSERT (iForeColour >= 0 && iForeColour < MAX_CUSTOM);
          clrBackground = pDoc->m_customback [iForeColour]; 
          }
        else
        if ((style & COLOURTYPE) == COLOUR_RGB)
          if (style & INVERSE)    // inverse inverts foreground and background
            clrBackground = iForeColour;
          else
            clrBackground = iBackColour;
        else
          {
          ASSERT (iForeColour >= 0 && iForeColour < 8);
          ASSERT (iBackColour >= 0 && iBackColour < 8);
          if (style & INVERSE)    // inverse inverts foreground and background
            clrBackground = pDoc->m_normalcolour [iForeColour];
          else
            clrBackground = pDoc->m_normalcolour [iBackColour];
          }   // not custom

        GetClientRect (&cr);


        SetRect (&r, 
                 pDoc->m_iPixelOffset + pixel,
                 - pDoc->m_iPixelOffset + line * pDoc->m_FontHeight,
                 cr.right,
                 - pDoc->m_iPixelOffset + (line + 1) * pDoc->m_FontHeight);
            
        OffsetRect (&r, -m_scroll_position.x, -m_scroll_position.y);

        if (clrBackground != b)
          pDC->FillSolidRect (&r, pDoc->TranslateColour (clrBackground));
        }   // end of bleed wanted

      // special drawing for horizontal rule
      if (pLine->flags & HORIZ_RULE)
        {
        pDC->SetBkMode (OPAQUE);
        RECT r,
             cr;    // client rect
        int iHalfFontHeight = pDoc->m_FontHeight / 2;
        if (iHalfFontHeight == 0)
          iHalfFontHeight = 1;

        GetClientRect (&cr);

        SetRect (&r, 
                 pDoc->m_iPixelOffset + 10,
                 - pDoc->m_iPixelOffset + line * pDoc->m_FontHeight + iHalfFontHeight,
                 cr.right - 10 - pDoc->m_iPixelOffset,
                 - pDoc->m_iPixelOffset + line * pDoc->m_FontHeight + iHalfFontHeight + 1);
            
        OffsetRect (&r, -m_scroll_position.x, -m_scroll_position.y);
        OffsetRect (&r, pDoc->m_TextRectangle.left, pDoc->m_TextRectangle.top);

        pDC->FillSolidRect (&r, pDoc->TranslateColour (RGB (132, 132, 132)));
        OffsetRect (&r, 0, 1);
        pDC->FillSolidRect (&r, pDoc->TranslateColour (RGB (198, 198, 198)));

        } // end of doing horizontal rule 

      }   // end of doing background and then text
    }   // end of doing each line

// make sure count (and title) is reset once we look at the view

  if (!GetParentFrame ()->IsIconic ())
    {
    m_last_line = pDoc->m_total_lines;
    FixupTitle ();
    }
  

  pDC->SelectClipRgn (NULL);

  // foreground image 

  if ((HBITMAP) pDoc->m_ForegroundBitmap)
    DrawImage (pDC, pDoc->m_ForegroundBitmap, pDoc->m_iForegroundMode); 

  Calculate_MiniWindow_Rectangles (pDoc, false);

  // mini windows  - on bottom
  for (mwit = pDoc->m_MiniWindows.begin (); 
       mwit != pDoc->m_MiniWindows.end ();
       mwit++)
         {
         CMiniWindow * mw = mwit->second;
         // not if not to be shown
         if (!mw->GetShow () || mw->m_bTemporarilyHide)
           continue;

         long iFlags = mw->GetFlags ();

         // not if wrong time
         if ((iFlags & MINIWINDOW_DRAW_UNDERNEATH))
           continue;

         // blit onto screen
         Blit_Bitmap (pDC, mw->GetDC (), 
                      mw->GetWidth (), mw->GetHeight (), 
                      mw->GetPosition (), 
                      mw->m_rect, 
                      mw->GetPosition () != 13,  // absolute? - we precalculated so yes (except tile)
                      (iFlags & MINIWINDOW_TRANSPARENT) != 0,        // transparent?
                      mw->GetBackgroundColour ());

         }  // end for each window

}   // end CMUSHView::OnDraw


void CMUSHView::OnInitialUpdate()
{
  CMUSHclientDoc* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  CView::OnInitialUpdate();

  CSize sizeTotal;

  sizeTotal.cx = sizeTotal.cy = 100;
  SetScrollSizes(sizeTotal, sizeTotal, sizeTotal);

  sizewindow ();

  FixupTitle ();

  if (m_ToolTip.Create(this, TTS_ALWAYSTIP | TTS_NOPREFIX | 0x40) && m_ToolTip.AddTool(this))
  {
    m_ToolTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX);
    m_ToolTip.SendMessage(TTM_SETDELAYTIME, TTDT_AUTOPOP, 5000);
    m_ToolTip.SendMessage(TTM_SETDELAYTIME, TTDT_INITIAL, 400);
    m_ToolTip.SendMessage(TTM_SETDELAYTIME, TTDT_RESHOW, 400);

    // add icon and title to tooltip
   // TTI_NONE     = 0 - no icon
   // TTI_INFO     = 1 - information icon
   // TTI_WARNING  = 2 - warning icon
   // TTI_ERROR    = 3 - error icon 

//   m_ToolTip.SendMessage (TTM_SETTITLE, TTI_INFO, (LPARAM) Translate ("Line information") );

   m_ToolTip.SetMaxTipWidth(500);
  }
  else
  {
    TRACE("Error in creating ToolTip");
  }

  } // end CMUSHView::OnInitialUpdate

/////////////////////////////////////////////////////////////////////////////
// CMUSHView printing

BOOL CMUSHView::OnPreparePrinting(CPrintInfo* pInfo)
{
  // default preparation
  return DoPreparePrinting(pInfo);
}

void CMUSHView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
  // TODO: add extra initialization before printing
}

void CMUSHView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
  // TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CMUSHView diagnostics

#ifdef _DEBUG
void CMUSHView::AssertValid() const
{
  CView::AssertValid();
}

void CMUSHView::Dump(CDumpContext& dc) const
{
  CView::Dump(dc);
}

CMUSHclientDoc* CMUSHView::GetDocument() // non-debug version is inline
{
  ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMUSHclientDoc)));
  return (CMUSHclientDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMUSHView message handlers

void CMUSHView::OnTestEnd() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);
int lastline;

CPoint pt;

// include partial last line if necessary

lastline = pDoc->GetLastLine ();

pt.y =  (lastline * pDoc->m_FontHeight) - GetOutputWindowHeight ();

pt.x = 0;

if (pt.y < 0)
  pt.y = 0;

ScrollToPosition (pt, App.m_bSmoothScrolling);


  Invalidate ();
  
  m_last_line_drawn = lastline;

#if REDRAW_DEBUG
  ShowInvalidatedRgn (this, oldrgn);
#endif

  m_selstart_line = 0;  
  m_selstart_col = 0;
  m_selend_line = 0;
  m_selend_col = 0;
  m_bAtBufferEnd = true;  // note we are at end

  SelectionChanged ();

  }

void CMUSHView::OnTestPagedown() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

CPoint pt = GetScrollPosition ();
int lastline;


pt.y += GetOutputWindowHeight () - pDoc->m_FontHeight;

// include partial last line if necessary

lastline = pDoc->GetLastLine ();

if (pt.y > ((lastline * pDoc->m_FontHeight) - GetOutputWindowHeight ()))
  pt.y =  (lastline * pDoc->m_FontHeight) - GetOutputWindowHeight ();

ScrollToPosition (pt, false);

}

void CMUSHView::OnTestPageup() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

CPoint pt = GetScrollPosition ();

pt.y -= GetOutputWindowHeight () - pDoc->m_FontHeight;

if (pt.y < 0)
  pt.y = 0;

ScrollToPosition (pt, false);

}

void CMUSHView::OnTestStart() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

POINT pt = {0, 0};

ScrollToPosition (pt, false);

}



/* +++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*                                                     */
/*       sizewindow                                      */
/*                                                     */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++ */

// Size the window to hold our "screen"
// 

void CMUSHView::sizewindow (void)
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

  long top,
       left;
  CRect statusrect,
        toolbarrect;
  Frame.ReturnStatusRect (statusrect);
  Frame.ReturnToolbarRect (toolbarrect);

  RECT rect;

  GetOwner()->GetOwner()->GetWindowRect (&rect);

  top = 0;
  left = 0;
  
// calculate width

  m_DefaultWidth  =  (pDoc->m_nWrapColumn * pDoc->m_FontWidth) +   // Client width of screen is 640 pixels
                    (GetSystemMetrics (SM_CXFRAME) * 2) +  // allow for border
                     GetSystemMetrics (SM_CXVSCROLL) +  // allow for vertical scroll bar
                     + pDoc->m_iPixelOffset  +                  // plus text offset from side
                    4;      // allow for Windows 95 frame

// calculate height

  m_DefaultHeight =  (24 * pDoc->m_FontHeight) +      // Client height of screen is 400 pixels
                    statusrect.Height() +     // plus the status bar
                    toolbarrect.Height () +   // plus the tool bar
                    2 +                       // enough to see the WP cursor
                    pDoc->m_iPixelOffset +            // plus text offset from bottom
                    GetSystemMetrics (SM_CYMENU) + // plus the menu
                    (GetSystemMetrics (SM_CYFRAME) * 2) +
                    GetSystemMetrics (SM_CYCAPTION) +  // plus the title
                    20;    // allow for command input area

  RECT framerect;
  ::GetClientRect (((CMDIFrameWnd *)&Frame)->m_hWndMDIClient, &framerect);

  if (m_DefaultHeight > framerect.bottom)
    m_DefaultHeight = framerect.bottom;

  WINDOWPLACEMENT wp;
  wp.length = sizeof (wp);
  GetOwner()->GetOwner()->GetWindowPlacement (&wp);

  // don't resize if maximised
  if (wp.showCmd != SW_MAXIMIZE)
    GetOwner()->GetOwner()->MoveWindow (left, 
                      top, 
                      m_DefaultWidth,
                      m_DefaultHeight);

  // now recalulate scroll bar information

// find page size


// find last line

int lastline;

  lastline = pDoc->GetLastLine ();

CSize sizeTotal (pDoc->m_nWrapColumn * pDoc->m_FontWidth, lastline * pDoc->m_FontHeight),
      sizePage  (GetOutputWindowWidth () - pDoc->m_iPixelOffset, GetOutputWindowHeight ()),
      sizeLine  (pDoc->m_FontWidth, pDoc->m_FontHeight);

  SetScrollSizes (sizeTotal, sizePage, sizeLine);

  pDoc->SendWindowSizes (pDoc->m_nWrapColumn);

  } // end of sizewindow



void CMUSHView::addedstuff (void)
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

// find page size

// find last line

int lastline;

  lastline = pDoc->GetLastLine ();

CSize sizeTotal (pDoc->m_nWrapColumn * pDoc->m_FontWidth, lastline * pDoc->m_FontHeight),
      sizePage  (GetOutputWindowWidth () - pDoc->m_iPixelOffset, GetOutputWindowHeight ()),
      sizeLine  (pDoc->m_FontWidth, pDoc->m_FontHeight);

  SetScrollSizes (sizeTotal, sizePage, sizeLine);

// pretend they pressed "End" to force the view to update.

  if (!m_freeze)
    OnTestEnd ();   

// if we are still on first screen (no scroll bars) invalidate whole view

CPoint pt = GetScrollPosition ();

  if (pt.y == 0)
    {
    Invalidate ();
#if REDRAW_DEBUG
    RECT r;
    GetClientRect (&r);
    ShowInvalidatedRect (this, r);
#endif
    }

// if iconized, show number of lines since it wasn't iconised (in title)

  if (GetParentFrame ()->IsIconic ())
    {
    CString str;
    long diff = pDoc->m_total_lines - m_last_line;

    str.Format ("%s [%ld]", (const char *) pDoc->m_mush_name, diff);

    GetParentFrame ()->SetWindowText (str);
    }
  else
    {

// not iconised, remember last line count and restore document name in title

    m_last_line = pDoc->m_total_lines;

    CString strOldTitle;

    GetParentFrame ()->GetWindowText (strOldTitle);

    // amend title if necessary (avoid flicker)
    if (strOldTitle != pDoc->m_mush_name)
      GetParentFrame ()->SetWindowText (pDoc->m_mush_name);

    }

  }

// returns true if outside range
bool CMUSHView::calculate_line_and_column (const CPoint & pt, CClientDC & dc,
                                           int & line, int & col,
                                           const bool bHalfWay)
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

CPoint point (pt);

int lastline;
long lastx = 0;
bool bOutside = true;
long pixel;

  dc.SelectObject(pDoc->m_font [0]);

  point.y -= pDoc->m_TextRectangle.top;
  point.x -= pDoc->m_TextRectangle.left;

  line = (point.y + pDoc->m_iPixelOffset + m_scroll_position.y) / pDoc->m_FontHeight;

// include partial last line if necessary

  lastline = pDoc->GetLastLine ();

// make sure our line is inside the file

  if (line < 0)
    line = 0;
  else
    if (line > lastline)
      line = lastline;
    else
      bOutside = false;

  CLine * pLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (line));

// calculate which column we must be at

  for (col = pixel = 0; 
      pixel < point.x && col <= pLine->len; 
      col++)
        {
        lastx = pixel;
        pixel = calculate_width (line, col, pDoc, dc) + pDoc->m_iPixelOffset;
        }

  col--;    // columns are zero-relative

// if we are 50% through this character, take the next one, otherwise take the previous one

  if (bHalfWay)     // if wanted (not for menus)
    {
    lastx += (pixel - lastx) / 2;

    if (point.x < lastx)
      col--;
    }

// make sure column is reasonable

  if (col < 0)
    col = 0;

  return bOutside;

  } // end of CMUSHView::calculate_line_and_column 

void CMUSHView::extend_selection (const int line, const int col)
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

  if (line > m_pin_line || (line == m_pin_line && col > m_pin_col))
    {
    m_selstart_line = m_pin_line;
    m_selstart_col =  m_pin_col;
    m_selend_line = line;
    m_selend_col = col;
    }
  else
    {
    m_selstart_line = line;
    m_selstart_col = col;
    m_selend_line = m_pin_line;
    m_selend_col =  m_pin_col;
    }

  SelectionChanged ();

  // auto-copy selection to clipboard
  if (pDoc->m_bCopySelectionToClipboard)
    if (pDoc->m_bAutoCopyInHTML)
      OnEditCopyashtml ();
    else
      OnEditCopy ();

  } // end of CMUSHView::extend_selection  

int CompareMenu (const void * elem1, const void * elem2)
  {
  CString string1 = (*((CString *) elem1));
  CString string2 = (*((CString *) elem2));

  // strip off plugin IDs
  CStringList strList;

  StringToList (string1, ":", strList);
  string1 = strList.GetHead ();
  
  StringToList (string2, ":", strList);
  string2 = strList.GetHead ();

  return string1.CompareNoCase (string2);

  }   // end of CompareMenu

void CMUSHView::AliasMenu (CPoint point)
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

CPoint menupoint = point;

  CMenu menu;
  VERIFY(menu.LoadMenu(IDR_MXP_MENU));

  CMenu* pPopup = menu.GetSubMenu(0);
  ASSERT(pPopup != NULL);
  CWnd* pWndPopupOwner = this;

  pPopup->DeleteMenu (0, MF_BYPOSITION);  // get rid of dummy item

  int i = 0;

  CAlias * pAlias;
  CString strAliasName;

  for (POSITION pos = pDoc->m_AliasMap.GetStartPosition(); 
       pos && i < MXP_MENU_COUNT; 
       )
     {
     pDoc->m_AliasMap.GetNextAssoc (pos, strAliasName, pAlias);
     if (pAlias->strLabel.IsEmpty () ||
        !pAlias->bMenu ||
        !pAlias->bEnabled)
        continue;

     // remember what to send if they click on it
     strMXP_menu_item [i] = pAlias->strLabel;
     i++;
    } // end of all aliases

  // do plugins
  for (POSITION plugin_pos = pDoc->m_PluginList.GetHeadPosition (); 
        plugin_pos && i < MXP_MENU_COUNT; )
    {
    pDoc->m_CurrentPlugin = pDoc->m_PluginList.GetNext (plugin_pos);

    if (pDoc->m_CurrentPlugin->m_bEnabled)
      for (POSITION pos = pDoc->GetAliasMap ().GetStartPosition (); 
            pos && i < MXP_MENU_COUNT; )
        {
        pDoc->GetAliasMap ().GetNextAssoc (pos, strAliasName, pAlias);

         if (pAlias->strLabel.IsEmpty () ||
            !pAlias->bMenu ||
            !pAlias->bEnabled)
            continue;

         // remember what to send if they click on it
         strMXP_menu_item [i] = pAlias->strLabel;
         strMXP_menu_item [i] += ":";
         strMXP_menu_item [i] += pDoc->m_CurrentPlugin->m_strID;  // need to know which plugin
         i++;

        }  // end of scanning plugin aliases
    } // end of doing plugins list
  pDoc->m_CurrentPlugin = NULL;

  if (i == 0)
    {
     pPopup->AppendMenu (MF_STRING | MF_GRAYED, MXP_FIRST_MENU, "(no alias menu items)");
     strMXP_menu_item [0].Empty ();
    }
  else
    {
        // sort the array - otherwise we'll be all over the map :)
    qsort (strMXP_menu_item, 
           i,
           sizeof (CString),
           CompareMenu);

    for (int j = 0; j < i; j++)
      {
      CString strMenu = Replace (strMXP_menu_item [j], "_", " ");

      // strip off plugin id
      CStringList strList;
      StringToList (strMenu, ":", strList);
      strMenu = strList.GetHead ();

      // add menu item
      pPopup->AppendMenu (MF_STRING | MF_ENABLED, MXP_FIRST_MENU + j, strMenu);     

      // alias map lookup must be lower case
      strMXP_menu_item [j].MakeLower ();
      } // end of building menu

    } // end of some menu items


  while (pWndPopupOwner->GetStyle() & WS_CHILD)
    pWndPopupOwner = pWndPopupOwner->GetParent();

  ClientToScreen(&point);

  // without this line the auto-enable always set "no items" to active
  Frame.m_bAutoMenuEnable  = FALSE;

  iAction = ACTION_ALIAS;
  pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
                        point.x, 
                        point.y,
                        pWndPopupOwner);

  // put things back how they were
  Frame.m_bAutoMenuEnable  = TRUE;


  } // end of CMUSHView::AliasMenu


void CMUSHView::OnLButtonDown(UINT nFlags, CPoint point) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

int line,
    col;

  if (!pDoc->m_FontHeight)
    return;

  CPoint orig_point = point;  // for AliasMenu


  // CView changes the viewport origin and mapping mode.
  // It's necessary to convert the point from device coordinates
  // to logical coordinates, such as are stored in the document.
  CClientDC dc(this);
  OnPrepareDC(&dc);
  dc.DPtoLP(&point);

  // if click in mini-window, don't continue
  if (Mouse_Down_MiniWindow (pDoc, point, 0x10))  // LH mouse, single click
    return;

  // check for the rectangle, now we are past the miniwindow stuff
  CRect ClientRect = GetTextRectangle (true);

  if (!ClientRect.PtInRect(point))
    return;

  if (GetKeyState (VK_CONTROL) < 0)  // ctrl+click
    {
    AliasMenu (orig_point);
    return;
    } // end of ctrl+click

  dc.SelectObject(pDoc->m_font [0]);


  m_last_mouse_position = point;


//  see if they clicked on a hyperlink

  // find which line and column the mouse position is at
            
  calculate_line_and_column (point, dc, line, col, false);

  // find line

  CLine * pLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (line));

  unsigned int iStyle;
  CStyle * pStyle;
  POSITION foundpos;
  int iCol;

  // don't do it *past* end of last word
  long pixel = calculate_width (line, pLine->len, pDoc, dc) + pDoc->m_iPixelOffset + pDoc->m_TextRectangle.left;

  if (point.x < pixel &&
     pDoc->FindStyle (pLine, col, iCol, pStyle, foundpos))
    {
    iStyle = pStyle->iFlags;
      
    if (pStyle->pAction &&
        !pStyle->pAction->m_strAction.IsEmpty () &&
        pStyle->pAction->m_strAction.Find ("&text;") == -1)
      if ((iStyle & ACTIONTYPE) == ACTION_SEND ||
          (iStyle & ACTIONTYPE) == ACTION_PROMPT)
        {

        CString strActions = pStyle->pAction->m_strAction;   // action
        CString strHints = pStyle->pAction->m_strHint;    // hints, if any

        CStringList actionsList;

        StringToList (strActions, POPUP_DELIMITER, actionsList);

        CString strAction = actionsList.GetHead ();

        // just send first action (default action)

        // unpause the output window if wanted
        if (pDoc->m_bUnpauseOnSend && m_freeze)
          {
          m_freeze = false;
          addedstuff ();   
          }

        if ((iStyle & ACTIONTYPE) == ACTION_SEND && 
            (pLine->flags & COMMENT) == COMMENT)
          {
          // action_send on a note line will be executed, not sent
          //  (for the world.Hyperlink function)
          // However an action in the form !!pluginID:script(arg)
          // eg. !!753ba7e011f3c8943a885f18:mysub(1234)   
          // will be passed the nominted sub in the nominated plugin

          // rather elaborate test ...

          // a. Must start with !!
          // b. Must end with )
          // c. Must have a "(" in it
          // d. After the !! there must be 24 hex characters
          // e. After that must be a colon (:)
          // f. After the colon must be a valid subroutine name
          if (strAction.GetLength () >= PLUGIN_UNIQUE_ID_LENGTH + 5 &&
              strAction.Left (2) == "!!" &&
              strAction.Right (1) == ")" &&
              strAction.Find ("(") != -1 &&
              strAction.Mid (PLUGIN_UNIQUE_ID_LENGTH + 2, 1) == ":" && 
              IsPluginID (strAction.Mid (2, PLUGIN_UNIQUE_ID_LENGTH)) &&
              IsSubName (strAction.Mid (PLUGIN_UNIQUE_ID_LENGTH + 3)))
            { // correct syntax for plugin call
            CString strPluginID = strAction.Mid (2, PLUGIN_UNIQUE_ID_LENGTH);
            strAction = strAction.Mid (PLUGIN_UNIQUE_ID_LENGTH + 3);
            int iBracket = strAction.Find ("(");
            if (iBracket != -1)
              {
              CString strScriptName = strAction.Left (iBracket);
              strScriptName.TrimRight ();
              CString strArg = strAction.Mid (iBracket + 1);
              strArg = strArg.Left (strArg.GetLength () - 1); // drop trailing )
              if (strPluginID == DEBUG_PLUGIN_ID)  // just made that up ;)
                pDoc->DebugHelper (strScriptName, strArg);
              else
                {
                long iResult = pDoc->CallPlugin (strPluginID, strScriptName, strArg);
                CString strName = strPluginID;  // default to ID
                if (iResult != eOK)
                  {
                  CPlugin * pPlugin = pDoc->GetPlugin (strPluginID);
                  if (pPlugin)
                    strName = pPlugin->m_strName;
                  } // end of finding plugin's name
                switch (iResult)
                  {
                  case eNoSuchPlugin:
                    pDoc->ColourNote ("white", "red", 
                          TFormat ("Plugin \"%s\" is not installed",
                                  (LPCTSTR) strName));
                    break;
                  case eNoSuchRoutine:
                    pDoc->ColourNote ("white", "red", 
                          TFormat ("Script routine \"%s\" is not in plugin %s",
                                  (LPCTSTR) strScriptName,
                                  (LPCTSTR) strName));
                    break;

                  case eErrorCallingPluginRoutine:
                    pDoc->ColourNote ("white", "red", 
                          TFormat ("An error occurred calling plugin %s",
                                  (LPCTSTR) strName));
                    break;

                  } // end of switch on errors
                }   // end of not debug helper ID
              }  // end of ( found (this should occur, really)
            }   // end of passing special syntax test
          else
            {  // plugin does not exist - just execute it
            pDoc->m_iExecutionDepth = 0;
            pDoc->Execute (strAction);
            }
          } // end of note hyperlink
        else
          {   // not note line

          if (pDoc->CheckConnected ())
            return;

          // send to command window?
          if ( (iStyle & ACTIONTYPE) == ACTION_PROMPT)
            {
            if (m_bottomview->CheckTyping (pDoc, strAction))
              return;             
            m_bottomview->SetCommand (strAction);
            }
          else
            {
            // send it
            pDoc->SendMsg (strAction, 
                           pDoc->m_bEchoHyperlinkInOutputWindow,
                           false,           // don't queue
                           pDoc->LoggingInput ());
            if (pDoc->m_bHyperlinkAddsToCommandHistory)
              m_bottomview->AddToCommandHistory (strAction);
            }

          // put the focus back in the send window

          OnKeysActivatecommandview ();
          }   // end of not execute wanted
        return;
        } // end of ACTION_SEND  or  ACTION_PROMPT
      else
      if ((iStyle & ACTIONTYPE) == ACTION_HYPERLINK)
        {
        CString strAction = pStyle->pAction->m_strAction;

        // don't let them slip in arbitrary OS commands
        if (strAction.Left (7).CompareNoCase ("http://") != 0 &&
            strAction.Left (8).CompareNoCase ("https://") != 0 &&
            strAction.Left (7).CompareNoCase ("mailto:") != 0)
          ::UMessageBox(TFormat ("Hyperlink action \"%s\" - permission denied.", 
                          (const char *) strAction), 
                          MB_ICONEXCLAMATION);
        else
          if ((long) ShellExecute (Frame, _T("open"), strAction, NULL, NULL, SW_SHOWNORMAL) <= 32)
            ::UMessageBox(TFormat ("Unable to open the hyperlink \"%s\"", 
                            (const char *) strAction), 
                            MB_ICONEXCLAMATION);
        return;
        }  // end of ACTION_HYPERLINK

    } // end of column found


  calculate_line_and_column (point, dc, line, col, true);


  CRgn oldrgn;
  CRgn newrgn;

// Get old selection region

  get_selection (oldrgn);

// if clicked *inside* old selection, leave selected  
  if (oldrgn.PtInRegion (point))
    {
    oldrgn.DeleteObject ();
    return;
    }

// if shift key down, extend (or shrink) the current selection

  if (nFlags & MK_SHIFT)
    extend_selection (line, col);
  else
    {     // shift key not down, just start a new selection
    m_selend_line = m_pin_line = m_selstart_line = line;
    m_selend_col = m_pin_col = m_selstart_col = col;
    SelectionChanged ();
    }     // end of shift key not down

  Invalidate ();

#if REDRAW_DEBUG
  ShowInvalidatedRgn (this, oldrgn);
#endif

  // Finished with regions
  newrgn.DeleteObject ();
  oldrgn.DeleteObject ();

  SetCapture();       // Capture the mouse until button up.

  return;
  
  // CView::OnLButtonDown(nFlags, point);
}
               
void CMUSHView::OnRButtonDown(UINT nFlags, CPoint point) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);


  // CView changes the viewport origin and mapping mode.
  // It's necessary to convert the point from device coordinates
  // to logical coordinates, such as are stored in the document.
  CPoint mwpoint = point;
  CClientDC dc(this);
  OnPrepareDC(&dc);
  dc.DPtoLP(&mwpoint);

  // if click in mini-window, don't continue
  if (Mouse_Down_MiniWindow (pDoc, mwpoint, 0x20))  // RH mouse, single click
    return;
  
  CView::OnRButtonDown(nFlags, point);
}


void CMUSHView::OnLButtonUp(UINT nFlags, CPoint point) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

int line,
    col;

  if (!pDoc->m_FontHeight)
    return;


  CClientDC dc(this);

  // CView changes the viewport origin and mapping mode.
  // It's necessary to convert the point from device coordinates
  // to logical coordinates, such as are stored in the document.
  OnPrepareDC(&dc);  // set up mapping mode and viewport origin
  dc.DPtoLP(&point);


  // if miniwindow handles it, don't continue
  if (Mouse_Up_MiniWindow (pDoc, point, 0x10))
    return;

  if (GetCapture() != this)
    return; // If this window (view) didn't capture the mouse,
        // then the user isn't selecting in this window.

  dc.SelectObject(pDoc->m_font [0]);

  CRgn oldrgn;
  CRgn newrgn;

// Get old selection region

  get_selection (oldrgn);

// find which line and column the mouse position is at
              
  calculate_line_and_column (point, dc, line, col);

// extend (or shrink) the current selection

  extend_selection (line, col);

// Get new selection region

  if (!get_selection (newrgn) && pDoc->m_bAutoFreeze)
     m_freeze = true;   // freeze output so they can copy or print it

  // Invalidate new stuff
  Invalidate ();

#if REDRAW_DEBUG
  ShowInvalidatedRgn (this, oldrgn);
#endif

  // Finished with regions
  newrgn.DeleteObject ();
  oldrgn.DeleteObject ();

  ReleaseCapture();   // Release the mouse capture established at
                    // the beginning of the mouse drag.

  return;
  
//  CView::OnLButtonUp(nFlags, point);
}


void CMUSHView::OnRButtonUp(UINT nFlags, CPoint point) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);


  CClientDC dc(this);

  // CView changes the viewport origin and mapping mode.
  // It's necessary to convert the point from device coordinates
  // to logical coordinates, such as are stored in the document.
  CPoint mwpoint = point;
  OnPrepareDC(&dc);  // set up mapping mode and viewport origin
  dc.DPtoLP(&mwpoint);


  // if miniwindow handles it, don't continue
  if (Mouse_Up_MiniWindow (pDoc, mwpoint, 0x20))
    return;

  
  CView::OnRButtonUp(nFlags, point);
}



void CMUSHView::OnMouseMove(UINT nFlags, CPoint point) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

int line,
    col;

  if (!pDoc->m_FontHeight)
    return;

  CClientDC dc(this);
  // CView changes the viewport origin and mapping mode.
  // It's necessary to convert the point from device coordinates
  // to logical coordinates, such as are stored in the document.
  OnPrepareDC(&dc);

  dc.DPtoLP(&point);

  // if over miniwindow, don't keep going
  if (Mouse_Move_MiniWindow (pDoc, point))
    return;

  // if we have a mini window ID, then we must have clicked in that
  //  - don't start selecting text
  if (!m_sPreviousMiniWindow.empty ())
    return;
  
  dc.SelectObject(pDoc->m_font [0]);

  // first work out the line/column the mouse is over

  calculate_line_and_column (point, dc, line, col, false);

  // tooltips stuff

  if (::IsWindow(m_ToolTip.m_hWnd))
  {

  // test for changed line or column here

    if (line != m_nLastToolTipLine || col != m_nLastToolTipColumn)
       RemoveToolTip ();

    m_ToolTip.Activate(TRUE);
    m_nLastToolTipLine = line;
    m_nLastToolTipColumn = col;
  }

  // first work out the line/column the mouse is over

  calculate_line_and_column (point, dc, line, col, true);

  // end of tooltips stuff

  // only if the user is currently drawing a new stroke by dragging
  // the captured mouse.

  if (GetCapture() != this)
    return; // If this window (view) didn't capture the mouse,
        // then the user isn't drawing in this window.


  m_last_mouse_position = point;

// handle the case where the mouse moves outside the client area

RECT r;

// see if the mouse is outside the visible window

  GetTextRect (&r);

  if (point.y > r.bottom)
    doLinedown ();
  else
  if (point.y < r.top)
    doLineup ();

  CRgn oldrgn;
  CRgn newrgn;

// Get old selection region

  get_selection (oldrgn);

// extend (or shrink) the current selection

  extend_selection (line, col);

// Get new selection region

  if (!get_selection (newrgn) && pDoc->m_bAutoFreeze)
     m_freeze = true;   // freeze output so they can copy or print it

  Invalidate ();

#if REDRAW_DEBUG
  ShowInvalidatedRgn (this, oldrgn);
#endif

  // Finished with regions
  newrgn.DeleteObject ();
  oldrgn.DeleteObject ();

  return;

}   // end CMUSHView::OnMouseMove

int CMUSHView::mouse_still_down (void)
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

int line,
    col;

  if (!pDoc->m_FontHeight)
    return 0;

  // only if the user is currently drawing a new stroke by dragging
  // the captured mouse.

  if (GetCapture() != this)
    return 0; // If this window (view) didn't capture the mouse,
        // then the user isn't drawing in this window.

  // if we have a mini window ID, then we must have clicked in that
  if (!m_sPreviousMiniWindow.empty ())
    return 0;

  CClientDC dc(this);
  // CView changes the viewport origin and mapping mode.
  // It's necessary to convert the point from device coordinates
  // to logical coordinates, such as are stored in the document.
  OnPrepareDC(&dc);
  dc.SelectObject(pDoc->m_font [0]);

// mouse will still be in the same spot, or we would have got a "mouse move"

CPoint point = m_last_mouse_position;

// handle the case where the mouse moves outside the client area

RECT r;

// see if the mouse is outside the visible window

  GetTextRect (&r);

  if (point.y > r.bottom)
    {
    doLinedown ();
    m_last_mouse_position.y += pDoc->m_FontHeight;
    }
  else 
    if (point.y < r.top)
      {
      doLineup ();
      m_last_mouse_position.y -= pDoc->m_FontHeight;
      }
    else
      return 0;   // don't want any more idle calls

  CRgn oldrgn;
  CRgn newrgn;

// Get old selection region

  get_selection (oldrgn);

// find which line and column the mouse position is at
              
  calculate_line_and_column (point, dc, line, col);

// extend (or shrink) the current selection

  extend_selection (line, col);
  
  // Invalidate new stuff
  Invalidate ();

#if REDRAW_DEBUG
  ShowInvalidatedRgn (this, oldrgn);
#endif

  // Finished with regions
  newrgn.DeleteObject ();
  oldrgn.DeleteObject ();


    return 1;   // want another idle loop

  } // end of CMUSHView::mouse_still_down

bool CMUSHView::get_selection (CRgn & rgn)
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);


RECT r;
long start_textsize,
      end_textsize;

long right;

GetTextRect (&r);
right = r.right;

CPoint pt = GetScrollPosition ();

  CClientDC dc(this);
  OnPrepareDC(&dc);

  // if selection past end of data return empty region
  if (m_selstart_line > pDoc->GetLastLine ())
    {
    rgn.CreateRectRgn (0, 0, 0, 0);
    return true;
    }

  start_textsize = calculate_width (m_selstart_line, m_selstart_col, pDoc, dc);
  end_textsize = calculate_width (m_selend_line, m_selend_col, pDoc, dc);

  if (start_textsize < 0)
    start_textsize = 0;

// starting and ending on the same line is a special case

  if (m_selstart_line == m_selend_line)
    {

    SetRect (&r, 
              pDoc->m_iPixelOffset + start_textsize,
              - pDoc->m_iPixelOffset + (m_selstart_line * pDoc->m_FontHeight) - pt.y,
              pDoc->m_iPixelOffset + end_textsize,
              - pDoc->m_iPixelOffset + ((m_selstart_line + 1) * pDoc->m_FontHeight) - pt.y);

    // end before or equal start means empty region
    // if we cannot create the region, create an empty one
    if (m_selend_col <= m_selstart_col ||
        rgn.CreateRectRgn (r.left, r.top, r.right, r.bottom) == 0)
      {
      rgn.CreateRectRgn (0, 0, 0, 0);
      return true;
      }
    return false;
    }

// do the first line, from startcol to the end

  SetRect (&r, 
            pDoc->m_iPixelOffset + start_textsize,
            - pDoc->m_iPixelOffset + (m_selstart_line * pDoc->m_FontHeight) - pt.y,
            pDoc->m_iPixelOffset + right,
            - pDoc->m_iPixelOffset + ((m_selstart_line + 1) * pDoc->m_FontHeight) - pt.y);

  CRgn firstrgn;
  CRgn middlergn;
  CRgn lastrgn;

  if (firstrgn.CreateRectRgn (r.left, r.top, r.right, r.bottom) == 0)
      firstrgn.CreateRectRgn (0, 0, 0, 0);

// do the middle lines, all columns

  if (m_selend_line > (m_selstart_line + 1))
    {
    SetRect (&r, 
              pDoc->m_iPixelOffset,
              - pDoc->m_iPixelOffset + ((m_selstart_line + 1) * pDoc->m_FontHeight) - pt.y,
              pDoc->m_iPixelOffset + right,
              - pDoc->m_iPixelOffset + ((m_selend_line) * pDoc->m_FontHeight) - pt.y);


    if (middlergn.CreateRectRgn (r.left, r.top, r.right, r.bottom) == 0)
      middlergn.CreateRectRgn (0, 0, 0, 0);

    }
  else
    middlergn.CreateRectRgn (0, 0, 0, 0);


// do the last line, from the start to endcol

  SetRect (&r, 
            pDoc->m_iPixelOffset,
            - pDoc->m_iPixelOffset + (m_selend_line * pDoc->m_FontHeight) - pt.y,
            pDoc->m_iPixelOffset + end_textsize,
            - pDoc->m_iPixelOffset + ((m_selend_line + 1) * pDoc->m_FontHeight) - pt.y);

  if (lastrgn.CreateRectRgn (r.left, r.top, r.right, r.bottom) == 0)
    lastrgn.CreateRectRgn (0, 0, 0, 0);


  // destination region must exist
  rgn.CreateRectRgn (0, 0, 0, 0);

  rgn.CombineRgn (&firstrgn, &middlergn, RGN_OR);
  bool bReturn = rgn.CombineRgn (&rgn, &lastrgn, RGN_OR) == NULLREGION;

  // delete old regions, not needed any more
  firstrgn.DeleteObject ();
  middlergn.DeleteObject ();
  lastrgn.DeleteObject ();

  return bReturn;   // true if empty region
  } // end of CMUSHView::get_selection

BOOL CMUSHView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);
  
  if (nHitTest != HTCLIENT)
    return CView::OnSetCursor(pWnd, nHitTest, message);

  // so where is the cursor
  CPoint point;
  ::GetCursorPos(&point);
  ScreenToClient(&point);

  // see if we moused over one of our miniwindows

  CMiniWindow * mw = NULL;
  CHotspot * pHotspot = NULL;
  string sHotspotId;
  string sMiniWindowId;

  mw = Mouse_Over_Miniwindow (pDoc, point, sHotspotId, pHotspot, sMiniWindowId);

  if (mw)
    {
    // no hotspot, switch to arrow cursor
    if (!pHotspot)
      {
      ::SetCursor (::LoadCursor (NULL, IDC_ARROW));
      return TRUE;
      }

    switch (pHotspot->m_Cursor)
      {
      default:
      case -1:  ::SetCursor (NULL);                              break;        // no cursor
      case  0:  ::SetCursor (::LoadCursor (NULL, IDC_ARROW));    break;        // arrow
      case  1:  ::SetCursor (CStaticLink::g_hCursorLink);        break;        // hand
      case  2:  ::SetCursor (App.g_hCursorIbeam);                break;        // I-beam
      case  3:  ::SetCursor (::LoadCursor (NULL, IDC_CROSS));    break;        // + (cross)
      case  4:  ::SetCursor (::LoadCursor (NULL, IDC_WAIT));     break;        // wait (hour-glass)
      case  5:  ::SetCursor (::LoadCursor (NULL, IDC_UPARROW));  break;        // up arrow
      case  6:  ::SetCursor (::LoadCursor (NULL, IDC_SIZENWSE)); break;        // arrow nw-se
      case  7:  ::SetCursor (::LoadCursor (NULL, IDC_SIZENESW)); break;        // arrow ne-sw
      case  8:  ::SetCursor (::LoadCursor (NULL, IDC_SIZEWE));   break;        // arrow e-w
      case  9:  ::SetCursor (::LoadCursor (NULL, IDC_SIZENS));   break;        // arrow n-s
      case 10:  ::SetCursor (::LoadCursor (NULL, IDC_SIZEALL));  break;        // arrow - all ways
      case 11:  ::SetCursor (::LoadCursor (NULL, IDC_NO));       break;        // (X) no, no, I won't do that, but ...
      case 12:  ::SetCursor (::LoadCursor (NULL, IDC_HELP));     break;        // help  (? symbol)
      } // end of switch


    return TRUE;
    }

  // turn cursor into hand over hyperlinks


  CClientDC dc(this);
  OnPrepareDC(&dc);
  dc.DPtoLP(&point);
  dc.SelectObject(pDoc->m_font [0]);

  int line,
      col;

    // another check for the rectangle, now we are past the miniwindow stuff
  CRect ClientRect = GetTextRectangle (true);

  if (!ClientRect.PtInRect(point))
    {
    ::SetCursor (::LoadCursor (NULL, IDC_ARROW));
    return TRUE;
    }

  // what line and column is that
  calculate_line_and_column (point, dc, line, col, false);

  // find the line data

  CLine * pLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (line));

  unsigned int iStyle;
  CStyle * pStyle;
  int iCol;
  POSITION foundpos;

  // don't show finger pointer *past* end of last word
  long pixel = calculate_width (line, pLine->len, pDoc, dc) + pDoc->m_iPixelOffset + pDoc->m_TextRectangle.left;

  if (pDoc->FindStyle (pLine, col, iCol, pStyle, foundpos))
    {
    iStyle = pStyle->iFlags;
    if ((iStyle & ACTIONTYPE) &&      // we have an action (send, hyperlink)
         pStyle->pAction &&
        !pStyle->pAction->m_strAction.IsEmpty () &&   // there is something to send
        pStyle->pAction->m_strAction.Find ("&text;") == -1 &&  // we know what &text; is
        CStaticLink::g_hCursorLink &&   // we have a finger cursor
        point.x < pixel)    // the mouse is not past the RH side of the line
      {
      ::SetCursor (CStaticLink::g_hCursorLink);
      return TRUE;
      } // we have an action!

    } // end of column found


// not a hyperlink - just display the I-beam

//  ::SetCursor (::LoadCursor (NULL, IDC_IBEAM));

  ::SetCursor (App.g_hCursorIbeam);

  return TRUE;

}


void CMUSHView::did_jump (void) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

m_selstart_line -= JUMP_SIZE;
m_selend_line -= JUMP_SIZE;
m_pin_line -= JUMP_SIZE;

if (m_selstart_line < 0)
  {
  m_selstart_line = 0;
  m_selstart_col = 0;
  SelectionChanged ();
  }

if (m_selend_line < 0)
  {
  m_selend_line = 0;
  m_selend_col = 0;
  SelectionChanged ();
  }

if (m_pin_line < 0)
  {
  m_pin_line = 0;
  m_pin_col = 0;
  }

// if frozen, adjust scroll position so same piece of text stays visible

if (m_freeze)
  {

  CPoint orig_top = GetScrollPosition ();
  
  orig_top.y -= JUMP_SIZE * pDoc->m_FontHeight;

  if (orig_top.y < 0)
    {
    m_freeze = FALSE;
    orig_top.y = 0;
    }


  ScrollToPosition (orig_top, App.m_bSmoothScrolling);
  if (orig_top.y == 0)
    {
    Invalidate ();
#if REDRAW_DEBUG
    RECT r;
    GetClientRect (&r);
    ShowInvalidatedRect (this, r);
#endif
    }

  }  // end of being frozen

}   // end of did_jump


void CMUSHView::doStart (void)
  {
  OnTestStart ();
  }


void CMUSHView::doEnd (void)
  {
  OnTestEnd ();
  }


void CMUSHView::doPageup (void)
  {
  OnTestPageup ();
  }


void CMUSHView::doPagedown (void)
  {
  OnTestPagedown ();
  }

void CMUSHView::OnTestLinedown() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);
int lastline;

CPoint pt = GetScrollPosition ();

pt.y += pDoc->m_FontHeight;

// include partial last line if necessary

lastline = pDoc->GetLastLine ();

//if (pt.y > (pDoc->m_iPixelOffset + (lastline * pDoc->m_FontHeight) - r.bottom))
//  pt.y = pDoc->m_iPixelOffset + (lastline * pDoc->m_FontHeight) - r.bottom;

if (pt.y > ((lastline * pDoc->m_FontHeight) - GetOutputWindowHeight ()))
  pt.y =  (lastline * pDoc->m_FontHeight) - GetOutputWindowHeight ();

ScrollToPosition (pt, App.m_bSmoothScrolling);

}

void CMUSHView::OnTestLineup() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

CPoint pt = GetScrollPosition ();

pt.y -= pDoc->m_FontHeight;

if (pt.y < 0)
  pt.y = 0;

ScrollToPosition (pt, App.m_bSmoothScrolling);

}


void CMUSHView::doLineup (void)
  {
  OnTestLineup ();
  }


void CMUSHView::doLinedown (void)
  {
  OnTestLinedown ();
  }

void CMUSHView::OnEditCopy() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

CString strSelection;

// put the focus back in the send window

  OnKeysActivatecommandview ();

  // if no selection here, try a selection in the command window
  if (GetSelection (strSelection))
    {
    m_bottomview->CEditView::OnEditCopy ();
    return;     // error in producing selection
    }

  putontoclipboard (strSelection, pDoc->m_bUTF_8);

}

void CMUSHView::OnUpdateNeedSelection(CCmdUI* pCmdUI) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

long startcol,
     endcol;

  CLine * pStartLine,
        * pEndLine;

  pStartLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (m_selstart_line));
  pEndLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (m_selend_line));
  
  startcol  = MIN (m_selstart_col, pStartLine->len);
  endcol    = MIN (m_selend_col, pEndLine->len);


  pCmdUI->Enable (m_selend_line > m_selstart_line || 
                  (m_selend_line == m_selstart_line && 
                   endcol > startcol));


}

void CMUSHView::OnUpdateNeedSelectionForCopy(CCmdUI* pCmdUI) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

long startcol,
     endcol;

  CLine * pStartLine,
        * pEndLine;

  pStartLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (m_selstart_line));
  pEndLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (m_selend_line));
  
  startcol  = MIN (m_selstart_col, pStartLine->len);
  endcol    = MIN (m_selend_col, pEndLine->len);

  if (m_selend_line > m_selstart_line || 
                (m_selend_line == m_selstart_line && 
                 endcol > startcol))
    pCmdUI->Enable (TRUE);
  else
    m_bottomview->CEditView::OnUpdateNeedSel (pCmdUI);


}


BOOL CMUSHView::OnEraseBkgnd(CDC* pDC) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);


  return FALSE;
}

void CMUSHView::OnKeysActivatecommandview() 
{
  m_owner_frame->SetActiveView((CView*) m_bottomview);
}


void CMUSHView::OnSize(UINT nType, int cx, int cy) 
{
  CView::OnSize(nType, cx, cy);
  
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);


// find page size


// find last line

int lastline;

  lastline = pDoc->GetLastLine ();

CSize sizeTotal (pDoc->m_nWrapColumn * pDoc->m_FontWidth, lastline * pDoc->m_FontHeight),
      sizePage  (GetOutputWindowWidth () - pDoc->m_iPixelOffset, GetOutputWindowHeight ()),
      sizeLine  (pDoc->m_FontWidth, pDoc->m_FontHeight);

  SetScrollSizes (sizeTotal, sizePage, sizeLine);

  // very bizarre bug - fixed in 4.39
  // we seem to get size messages if we are maximized and in the background

  bool bOldFreeze = m_freeze;  
  if (m_bAtBufferEnd)
    OnTestEnd ();   

  m_freeze = bOldFreeze;
  
  Frame.FixUpTitleBar ();   // in case we need to add the mud name to the title bar

    // tell each plugin we have resized. Hello, Worstje!

  for (POSITION pluginpos = pDoc->m_PluginList.GetHeadPosition(); pluginpos; )
    {
    CPlugin * pPlugin = pDoc->m_PluginList.GetNext (pluginpos);


    if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
      continue;

    // see what the plugin makes of this,
    pPlugin->ExecutePluginScript (ON_PLUGIN_WORLD_OUTPUT_RESIZED,
                                  pPlugin->m_dispid_plugin_on_world_output_resized); 

    }   // end of doing each plugin


// this is for the guy that wants to fit the max text he can in his window,
// after resizing it

WINDOWPLACEMENT wp;
wp.length = sizeof (wp);
GetOwner()->GetOwner()->GetWindowPlacement (&wp);

// don't resize if not active, or minimized, or closed
if (wp.showCmd != SW_MINIMIZE &&
    pDoc->m_iConnectPhase == eConnectConnectedToMud &&
    (
    pDoc->m_pActiveOutputView == this  ||
    pDoc->m_pActiveCommandView == m_bottomview
    ))

  if (pDoc->m_bAutoWrapWindowWidth && 
      !pDoc->m_font_name.IsEmpty () &&
      pDoc->m_font_height > 0)
    {

    CDC dc;

    dc.CreateCompatibleDC (NULL);

    int lfHeight = -MulDiv(pDoc->m_font_height, dc.GetDeviceCaps(LOGPIXELSY), 72);
    CFont font;

     font.CreateFont(lfHeight, // int nHeight, 
            0, // int nWidth, 
            0, // int nEscapement, 
            0, // int nOrientation, 
            pDoc->m_font_weight, // int nWeight, 
            0, // BYTE bItalic, 
            0, // BYTE bUnderline, 
            0, // BYTE cStrikeOut, 
            pDoc->m_font_charset, // BYTE nCharSet, 
            0, // BYTE nOutPrecision, 
            0, // BYTE nClipPrecision, 
            0, // BYTE nQuality, 
            MUSHCLIENT_FONT_FAMILY, // BYTE nPitchAndFamily,    // was FF_DONTCARE
            pDoc->m_font_name);// LPCTSTR lpszFacename );

      // Get the metrics of the font.

      dc.SelectObject(font);

      TEXTMETRIC tm;
      dc.GetTextMetrics(&tm);


    int iWidth = (GetOutputWindowWidth () - pDoc->m_iPixelOffset) / tm.tmAveCharWidth;

    // ensure in range that we allow
    if (iWidth < 20)
      iWidth = 20;
    if (iWidth > MAX_LINE_WIDTH)
      iWidth = MAX_LINE_WIDTH;

  // We must adjust the current line to allow for the new wrap size

    if (pDoc->m_pCurrentLine)     // a new world might not have a line yet
      {
      // save current line text
      CString strLine = CString (pDoc->m_pCurrentLine->text, pDoc->m_pCurrentLine->len);

  #ifdef USE_REALLOC
      pDoc->m_pCurrentLine->text  = (char *) realloc (pDoc->m_pCurrentLine->text, 
                                               MAX (pDoc->m_pCurrentLine->len, iWidth) 
                                               * sizeof (char));
  #else
      delete [] pDoc->m_pCurrentLine->text;
      pDoc->m_pCurrentLine->text = new char [MAX (pDoc->m_pCurrentLine->len, iWidth)];
  #endif

      // put text back
      memcpy (pDoc->m_pCurrentLine->text, (LPCTSTR) strLine, pDoc->m_pCurrentLine->len);
      ASSERT (pDoc->m_pCurrentLine->text);
    
      }   // end of having a current line

    pDoc->m_nWrapColumn = iWidth;


    }   // end of auto-wrap wanted
  pDoc->SendWindowSizes (pDoc->m_nWrapColumn);
  EnableScrollBarCtrl (SB_VERT, pDoc->m_bScrollBarWanted);

}

void CMUSHView::OnDisplayFreezeoutput() 
{
  m_freeze = !m_freeze;

  if (!m_freeze)
    OnTestEnd ();   
  
}

void CMUSHView::OnUpdateDisplayFreezeoutput(CCmdUI* pCmdUI) 
{

  pCmdUI->SetCheck (m_freeze);
  pCmdUI->Enable (TRUE);

}

void CMUSHView::OnUpdateStatuslineFreeze_helper (CCmdUI* pCmdUI) 
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

int iNewPauseStatus = eNotPaused;

  m_bAtBufferEnd = GetScrollPos (SB_VERT) >= GetScrollLimit(SB_VERT);

// calculate pause status

   if (pDoc->m_iConnectPhase != eConnectConnectedToMud)
     iNewPauseStatus = eClosed;
   else
    if (m_freeze)
      {
       iNewPauseStatus = ePaused;
       if (!m_bAtBufferEnd)
         iNewPauseStatus = eMore;
      }

// same pause status? just exit

  if (m_iPauseStatus == iNewPauseStatus)
    return;

  switch (iNewPauseStatus)
    {
    default: return;
    case eNotPaused:  Frame.m_wndStatusBar.SendMessage 
                        (SB_SETTEXT, Frame.m_nPauseItem, (LPARAM) "");
                      break;
    case ePaused:     Frame.m_wndStatusBar.SendMessage 
                        (SB_SETTEXT, Frame.m_nPauseItem, (LPARAM) "PAUSE");
                      break;
    case eClosed:     Frame.m_wndStatusBar.SendMessage 
                        (SB_SETTEXT, Frame.m_nPauseItem, (LPARAM) "CLOSED");
                      break;
    case eMore:       Frame.m_wndStatusBar.SendMessage 
                        (SB_SETTEXT, SBT_OWNERDRAW | Frame.m_nPauseItem, 
                                  (LPARAM) "MORE");
                      break;      // draw MORE in inverse
    } // end of switch

// remember for next time

  m_iPauseStatus = iNewPauseStatus;


  }

void CMUSHView::OnUpdateStatuslineFreeze(CCmdUI* pCmdUI) 
{
OnUpdateStatuslineFreeze_helper (pCmdUI);
}

void CMUSHView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

  m_iPauseStatus = ePauseUninitialised;

  if (bActivate)
    {

    pDoc->m_pActiveOutputView = this;
    Frame.FixUpTitleBar ();
    if (pDoc->m_new_lines && App.m_pActivityDoc)
      {
      pDoc->m_new_lines = 0;
      App.m_bUpdateActivity = TRUE;
      }
    else
      pDoc->m_new_lines = 0;    

// make sure title is MUSH name

    FixupTitle ();

    // execute "get focus" script
    if (pDeactiveView != m_bottomview) // don't worry about swapping with bottom view
      {
      if (pDoc->m_ScriptEngine)
        {
        if (pDoc->SeeIfHandlerCanExecute (pDoc->m_strWorldGetFocus))
          {

          DISPPARAMS params = { NULL, NULL, 0, 0 };
          long nInvocationCount = 0;

          pDoc->ExecuteScript (pDoc->m_dispidWorldGetFocus, 
                       pDoc->m_strWorldGetFocus,
                       eWorldAction,
                       "world get focus", 
                       "getting focus",
                       params, 
                       nInvocationCount); 
          }
        } // end of executing get focus script

      if (!pDoc->m_bWorldClosing)
        {

        // now do plugins "get focus"
        CPlugin * pSavedPlugin = pDoc->m_CurrentPlugin;
        pDoc->m_CurrentPlugin = NULL;

        // tell each plugin what we have received
        for (POSITION pluginpos = pDoc->m_PluginList.GetHeadPosition(); pluginpos; )
          {
          CPlugin * pPlugin = pDoc->m_PluginList.GetNext (pluginpos);

          if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
            continue;

          // see what the plugin makes of this,
          pPlugin->ExecutePluginScript (ON_PLUGIN_GETFOCUS, pPlugin->m_dispid_plugin_get_focus);
          }   // end of doing each plugin

        pDoc->m_CurrentPlugin = pSavedPlugin;

        } // end of world not closing
      
      }   // end of executing "get focus" scripts
    // make sure status line is updated
    pDoc->ShowStatusLine ();

    }
  else
    {
    pDoc->m_pActiveOutputView = NULL;
    Frame.FixUpTitleBar ();

    // execute "Lose focus" script
    if (pActivateView != m_bottomview) // don't worry about swapping with bottom view
      {
      if (pDoc->m_ScriptEngine)
        {
        if (pDoc->SeeIfHandlerCanExecute (pDoc->m_strWorldLoseFocus))
          {
          DISPPARAMS params = { NULL, NULL, 0, 0 };
          long nInvocationCount = 0;

          pDoc->ExecuteScript (pDoc->m_dispidWorldLoseFocus,  
                       pDoc->m_strWorldLoseFocus,
                       eWorldAction,
                       "world lose focus", 
                       "losing focus",
                       params, 
                       nInvocationCount); 
          }
        } // end of executing lose focus script

      if (!pDoc->m_bWorldClosing)
        {
        // now do plugins "lose focus"
        CPlugin * pSavedPlugin = pDoc->m_CurrentPlugin;
        pDoc->m_CurrentPlugin = NULL;

        // tell each plugin what we have received
        for (POSITION pluginpos = pDoc->m_PluginList.GetHeadPosition(); pluginpos; )
          {
          CPlugin * pPlugin = pDoc->m_PluginList.GetNext (pluginpos);

          if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
            continue;

          // see what the plugin makes of this,
          pPlugin->ExecutePluginScript (ON_PLUGIN_LOSEFOCUS, pPlugin->m_dispid_plugin_lose_focus);
          }   // end of doing each plugin

        pDoc->m_CurrentPlugin = pSavedPlugin;
        } // end of world not closing
      }  // end of executing "Lose focus" scripts
    // make sure status line is updated
    Frame.SetStatusNormal (); 
    }

  
  CView::OnActivateView(bActivate, pActivateView, pDeactiveView);

  // update which world has the tick
  if (App.m_pActivityView && App.m_pActivityDoc)
      App.m_pActivityDoc->UpdateAllViews (NULL);


}

void CMUSHView::OnWindowMinimize() 
{
  
  if (GetParentFrame ()->IsIconic ())
    GetParentFrame ()->ShowWindow(SW_RESTORE);
  else
    GetParentFrame ()->ShowWindow(SW_MINIMIZE);
    
}

void CMUSHView::OnUpdateWindowMinimize(CCmdUI* pCmdUI) 
{
  if (GetParentFrame ()->IsIconic ())
    pCmdUI->SetCheck (TRUE);
  else
    pCmdUI->SetCheck (FALSE);
}

void CMUSHView::FixupTitle (void)
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

CString strTitle = pDoc->m_mush_name;

// don't bother showing "[open]" if connected - it is kind of annoying
//  ditto for worlds that will never connect (0.0.0.0)

  if (pDoc->m_iConnectPhase != eConnectConnectedToMud && pDoc->m_server != "0.0.0.0")
    {
    strTitle += " [";
    strTitle += GetConnectionStatus (pDoc->m_iConnectPhase);

    switch (pDoc->m_iConnectPhase)
      {
      case eConnectMudNameLookup:        
        strTitle += ": ";
        strTitle += pDoc->m_server; 
        break; 
      case eConnectProxyNameLookup:      
        strTitle += ": ";
        strTitle += pDoc->m_strProxyServerName; 
        break; 
      case eConnectConnectingToMud:      
        strTitle += ": ";
        strTitle += inet_ntoa (pDoc->m_sockAddr.sin_addr);
        strTitle += CFormat (", port %d]", pDoc->m_port);
        break; 
      case eConnectConnectingToProxy:    
        strTitle += ": ";
        strTitle += inet_ntoa (pDoc->m_ProxyAddr.sin_addr);
        strTitle += CFormat (", port %d]", pDoc->m_iProxyServerPort);
        break; 
      } // end of switch

    strTitle += "]";
    } // end of not connected

  CString strOldTitle;

  GetParentFrame ()->GetWindowText (strOldTitle);

  // only change title if necessary, to avoid flicker
  if (strTitle != strOldTitle)
    {
    GetParentFrame ()->SetWindowText (strTitle);
    if (Frame.m_wndMDITabs.InUse ())
      Frame.m_wndMDITabs.Update ();
    }

  } // end of FixupTitle

void CMUSHView::OnContextMenu(CWnd*, CPoint point)
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

    if (point.x == -1 && point.y == -1)
      {
      //keystroke invocation
      CRect rect;
      GetClientRect(rect);
      ClientToScreen(rect);

      point = rect.TopLeft();
      point.Offset(5, 5);
    }


// new, for MXP - see if they clicked on a hyperlink

  CPoint menupoint = point;
  ScreenToClient (&point);


  if (pDoc->m_bMXP && pDoc->m_FontHeight)
    {
    int line,
        col;


    // CView changes the viewport origin and mapping mode.
    // It's necessary to convert the point from device coordinates
    // to logical coordinates, such as are stored in the document.
    CClientDC dc(this);
    OnPrepareDC(&dc);
    dc.DPtoLP(&point);
    dc.SelectObject(pDoc->m_font [0]);

  // find which line and column the mouse position is at
            
    calculate_line_and_column (point, dc, line, col, false);

    // find line

    CLine * pLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (line));

    unsigned int iStyle;
    CStyle * pStyle;
    POSITION foundpos;
    int iCol;

    // don't do it *past* end of last word
    long pixel = calculate_width (line, pLine->len, pDoc, dc) + pDoc->m_iPixelOffset;

    if (point.x < pixel &&
        pDoc->FindStyle (pLine, col, iCol, pStyle, foundpos))
      {
      iStyle = pStyle->iFlags;
      if (pStyle->pAction &&
          !pStyle->pAction->m_strAction.IsEmpty () &&
          pStyle->pAction->m_strAction.Find ("&text;") == -1)
        if ((iStyle & ACTIONTYPE) == ACTION_SEND ||
            (iStyle & ACTIONTYPE) == ACTION_PROMPT)
          {

          CString strActions = pStyle->pAction->m_strAction;   // action
          CString strHints = pStyle->pAction->m_strHint;     // hints, if any

          CStringList actionsList,
                      hintsList;

          StringToList (strActions, POPUP_DELIMITER, actionsList);
          StringToList (strHints,   POPUP_DELIMITER, hintsList);

          // more hints than actions? first one must be the tooltip text

          if (hintsList.GetCount () > actionsList.GetCount ())
             hintsList.RemoveHead ();

          CMenu menu;
          VERIFY(menu.LoadMenu(IDR_MXP_MENU));

          CMenu* pPopup = menu.GetSubMenu(0);
          ASSERT(pPopup != NULL);
          CWnd* pWndPopupOwner = this;

          pPopup->DeleteMenu (0, MF_BYPOSITION);  // get rid of dummy item

          int iCount = MIN (actionsList.GetCount (), MXP_MENU_COUNT);

          // build up menu
          for (int i = 0; i < iCount; i++)
            {
            CString strAction = actionsList.RemoveHead ();
            CString strHint;

            // if we have a hint, take it
            if (!hintsList.IsEmpty ())
              strHint = hintsList.RemoveHead ();
            else
              strHint = strAction;    // otherwise use the action

            // ampersands won't come out right
            strHint.Replace ("&", "&&");

            // add menu item
            pPopup->AppendMenu (MF_STRING | MF_ENABLED, MXP_FIRST_MENU + i, strHint);

            if (i == 0)
              SetMenuDefaultItem(pPopup->m_hMenu, 0, MF_BYPOSITION);

            // remember what to send if they click on it
            strMXP_menu_item [i] = strAction;
            }

          iAction = iStyle & ACTIONTYPE;
          while (pWndPopupOwner->GetStyle() & WS_CHILD)
            pWndPopupOwner = pWndPopupOwner->GetParent();


          pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
                                menupoint.x, 
                                menupoint.y,
                                pWndPopupOwner);

          return;
          } // we have ACTION_SEND or ACTION_PROMPT
        else
        if ((iStyle & ACTIONTYPE) == ACTION_HYPERLINK)
          {

          CMenu menu;
          VERIFY(menu.LoadMenu(IDR_MXP_MENU));

          CMenu* pPopup = menu.GetSubMenu(0);
          ASSERT(pPopup != NULL);
          CWnd* pWndPopupOwner = this;

          pPopup->DeleteMenu (0, MF_BYPOSITION);  // get rid of dummy item

          // add menu item
          pPopup->AppendMenu (MF_STRING | MF_ENABLED, MXP_FIRST_MENU, pStyle->pAction->m_strAction);

          SetMenuDefaultItem(pPopup->m_hMenu, 0, MF_BYPOSITION);

          // remember what to send if they click on it
          strMXP_menu_item [0] = pStyle->pAction->m_strAction;

          iAction = iStyle & ACTIONTYPE;
          while (pWndPopupOwner->GetStyle() & WS_CHILD)
            pWndPopupOwner = pWndPopupOwner->GetParent();

          pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
                                menupoint.x, 
                                menupoint.y,
                                pWndPopupOwner);

          return;
          }  // end of ACTION_HYPERLINK

      } // end of column found

    }   // end of MXP active

    CMenu menu;
    VERIFY(menu.LoadMenu(CG_IDR_POPUP_MUSHVIEW));

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

    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, menupoint.x, menupoint.y,
      pWndPopupOwner);
}

/* If AllTypingToCommandWindow is enabled we redirect character messages to 
 * the bottom view. */

void CMUSHView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  if (App.m_bAllTypingToCommandWindow) {
    OnKeysActivatecommandview();
    m_bottomview->SendMessage(WM_CHAR, nChar, nRepCnt | (nFlags << 16));
  } else
    CView::OnChar(nChar, nRepCnt, nFlags);
}

void CMUSHView::OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  if (App.m_bAllTypingToCommandWindow) {
    OnKeysActivatecommandview();
    m_bottomview->SendMessage(WM_SYSCHAR, nChar, nRepCnt | (nFlags << 16));
  } else
    CView::OnSysChar(nChar, nRepCnt, nFlags);
}


void CMUSHView::OnDisplayFind() 
{
m_bottomview->DoFind(false);
}

void CMUSHView::OnDisplayFindagain() 
{
m_bottomview->DoFind(true);
}


void CMUSHView::PrintWorld (void)
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

int pages = 0;
int line = 0;

POSITION pos;
CLine * pLine;

CLine * pStartLine,
      * pEndLine;

int thiscol,
    thislen,
    startcol,
    endcol;
short style;

  pStartLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (m_selstart_line));
  pEndLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (m_selend_line));

BOOL bHaveSelection = m_selend_line > m_selstart_line || 
                      (m_selend_line == m_selstart_line &&
                      m_selend_col > m_selstart_col);

// printer control block for printing

t_print_control_block pcb;

// count pages in print file - allow 4 lines for header and footer
  
  pages = (pDoc->m_LineList.GetCount () + (App.m_nPrinterLinesPerPage - 4) - 1) /  (App.m_nPrinterLinesPerPage - 4);
  
// attempt to open the printer

  if (print_start_document (pcb, 
                            pDoc->m_mush_name, 
                            1, 
                            pages,
                            App.m_nPrinterLeftMargin,
                            App.m_nPrinterTopMargin,
                            App.m_nPrinterLinesPerPage,
                            App.m_nPrinterFontSize,
                            App.m_nPrinterFontSize,   // line spacing
                            App.m_strPrinterFont,
                            bHaveSelection))
    return;

  Frame.SetStatusMessageNow (Translate ("Printing world..."));

  unsigned int current_line = App.m_nPrinterLinesPerPage;   // force new page immediately

  if (pcb.pd->m_pd.Flags & PD_SELECTION)
    {
    line = m_selstart_line;
    pos = pDoc->GetLinePosition (m_selstart_line);
    }
  else
    pos = pDoc->m_LineList.GetHeadPosition ();

  
  while (pos)
    {

// get the line data

    pLine = pDoc->m_LineList.GetNext (pos);

// do footer and header if required

    if (current_line > (App.m_nPrinterLinesPerPage - 2))
      {

// do footer if not first page

      if (pcb.current_page > 0)
        {
        if (print_printline (pcb, 1, ""))     // first do a blank line
          break;

        print_font (pcb, FONT_BOLD);

        if (print_printline (pcb, 1, "Page %ld", pcb.current_page))     // then print the page number
          break;

        print_font (pcb, FONT_NORMAL);

        print_end_page (pcb);

        }   // end of not first page

// now do page header

      print_start_page (pcb);

      print_font (pcb, FONT_BOLD | FONT_UNDERLINE);

      CString strTime;
      strTime = pLine->m_theTime.Format (TranslateTime ("%A, %B %d, %Y, %#I:%M %p"));

      if (print_printline (pcb, 2, "%s - %s", 
                            (LPCTSTR) pDoc->m_mush_name,
                            (LPCTSTR) strTime)) 
        break;

      print_font (pcb, FONT_NORMAL);

      current_line = 2;     // back to top of page
        
      }  // end of past end of page

 // finished header and footer, print the current line

// break line up into pieces which have the same style, and then print in that style

// start by calculating the start column - will be different for the first line

    startcol = 0;
    if ((pcb.pd->m_pd.Flags & PD_SELECTION) && 
      (line == m_selstart_line))
      startcol = m_selstart_col;

// and the end column - will be different for the last line

    endcol = pLine->len;
    if ((pcb.pd->m_pd.Flags & PD_SELECTION) && 
      (line == m_selend_line))
      endcol = m_selend_col;

    CStyle * pStyle = NULL;
    int i = 0;
    POSITION stylepos;

    // find first style
    for (stylepos = pLine->styleList.GetHeadPosition(); stylepos; )
      {
      pStyle = pLine->styleList.GetNext (stylepos);
      i += pStyle->iLength;
      if (i >= startcol)
        break;    // found first style
      }

    // how much do we do of this style?
    thislen = i - startcol;
    int len = endcol - startcol;
    int cols_to_go = len;

    for (thiscol = startcol; thiscol < endcol; )
      {
      if (!pStyle)
        break;

      // don't overshoot
      thislen = MIN (cols_to_go, thislen);

      style = pStyle->iFlags & STYLE_BITS;

      if ((style & COLOURTYPE) == COLOUR_ANSI)
        {
        if (style & HILITE)
          print_font (pcb, pDoc->m_nBoldPrintStyle [pStyle->iForeColour]);
        else
          print_font (pcb, pDoc->m_nNormalPrintStyle [pStyle->iForeColour]);
        }
      else
        {
        if (style & HILITE)
          print_font (pcb, pDoc->m_nBoldPrintStyle [WHITE]);
        else
          print_font (pcb, pDoc->m_nNormalPrintStyle [WHITE]);
        }

      if (print_printline (pcb, 0, "%.*s", thislen, &pLine->text [thiscol]))
        break;
           
      thiscol += thislen;
      cols_to_go -= thislen;  // this many fewer to go
 
      if (!stylepos)
        break;  // no more styles

      // get next style in sequence
      pStyle = pLine->styleList.GetNext (stylepos);
      thislen = pStyle->iLength;

      } // end of this group of the same colour

    print_font (pcb, FONT_NORMAL);

    if (print_printline (pcb, 1, ""))   // move onto next line
      break;

    current_line++;
    line++;

// if printing selection, stop when past end of it

    if ((pcb.pd->m_pd.Flags & PD_SELECTION) && 
      ((line > m_selend_line && m_selend_col > 0) ||    // overshoot if we have stuff on last line
      (line >= m_selend_line && m_selend_col == 0))
      )
      break;

    }

// do final page footer

  if (!pcb.cancelled)
    {
    while (current_line <= (App.m_nPrinterLinesPerPage - 2))
      {
      if (print_printline (pcb, 1, ""))     // first do a blank line
        break;
      current_line++;
      }
      
    if (!pcb.cancelled)
      {
      print_font (pcb, FONT_BOLD);

      print_printline (pcb, 1, "Page %ld", pcb.current_page); 

      print_font (pcb, FONT_NORMAL);
      } 

    }   // end of doing page footer

// terminate the report by sending out the last page

  print_end_page (pcb);
  
  print_end_document (pcb);

  if (pcb.cancelled)
    ::TMessageBox ("Printing cancelled");

  pDoc->ShowStatusLine ();
  

  } // end of CMUSHView::PrintWorld

void CMUSHView::OnFilePrintWorld() 
{
PrintWorld ();
}

void CMUSHView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
  SCROLLINFO ScrollInfo;

  GetScrollInfo (SB_VERT, &ScrollInfo, SIF_ALL);

  RemoveToolTip ();

  switch (nSBCode)
    {
    case SB_THUMBTRACK:   // Drag scroll box to specified position. The current position is provided in nPos.
    case SB_THUMBPOSITION:// Scroll to the absolute position. The current position is provided in nPos.
          {
          ScrollToPosition (CPoint (0, ScrollInfo.nTrackPos), false);
          break;
          }
    case SB_BOTTOM:       //  Scroll to bottom.
          OnTestEnd ();
          break;
    case SB_ENDSCROLL:    //  End scroll.
          // what does this do? nothing?
          break;
    case SB_LINEDOWN:     //  Scroll one line down.
          OnTestLinedown ();
          break;
    case SB_LINEUP:       //  Scroll one line up.
          OnTestLineup ();
          break;
    case SB_PAGEDOWN:     //  Scroll one page down.
          OnTestPagedown();
          break;
    case SB_PAGEUP:       //  Scroll one page up.
          OnTestPageup();
          break;
    case SB_TOP:          //  Scroll to top.
          OnTestStart ();
          break;

    } // end of switch

}

void CMUSHView::OnFileSaveselection() 
{
    CFileDialog dlg(FALSE,            // FALSE for FileSave
            "txt",            // default extension
            "selection.txt",
            OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST,
            "Text files (*.txt)|*.txt||",
            this);
    dlg.m_ofn.lpstrTitle = "Saved selection";

    ChangeToFileBrowsingDirectory ();
    int nResult = dlg.DoModal();
    ChangeToStartupDirectory ();

    if (nResult != IDOK)
      return;

    try
      {
      CFile f (dlg.GetPathName(), CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive);

      if (GetSelection (f))
        return;     // error in producing selection
    
      }
    catch (CException* e)
    {
      e->ReportError();
      e->Delete();
    }
}    // end of CMUSHView::OnFileSaveselection

// gets the current selection and writes it to a file - returns true if failed

bool CMUSHView::GetSelection(CFile & f) 
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

long startcol,
     endcol;

int i;

// return if no valid selection

  CLine * pStartLine,
        * pEndLine;

  pStartLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (m_selstart_line));
  pEndLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (m_selend_line));
  
  startcol  = MIN (m_selstart_col, pStartLine->len);
  endcol    = MIN (m_selend_col, pEndLine->len);

  if (!(m_selend_line > m_selstart_line || 
              (m_selend_line == m_selstart_line && 
               endcol > startcol)))
    return true;

  try
    {

    CArchive ar(&f, CArchive::store);

  // starting and ending on the same line is a special case

    if (m_selend_line == m_selstart_line)
      ar.Write (&pStartLine->text [startcol], endcol - startcol);
    else
      {

  // do first line

      ar.Write (&pStartLine->text [startcol], pStartLine->len - startcol);

      if (pStartLine->hard_return)
        ar.WriteString (ENDLINE);

  // do middle lines

      POSITION pos = pDoc->GetLinePosition (m_selstart_line + 1);

      for (i = m_selstart_line + 1; i < m_selend_line && pos; i++)
        {
        CLine * pLine = pDoc->m_LineList.GetNext (pos);

        ar.Write (pLine->text, pLine->len);

        if (pLine->hard_return)
          ar.WriteString (ENDLINE);

        } // end of doing middle lines

  // do last line

      ar.Write (pEndLine->text, endcol);
    
      }

    ar.Close();
    }   // end of try block

  catch (CException * e)
    {
    e->ReportError ();
    e->Delete ();
    return true;    // bad return
    }

  return false;   // good return

  }  // end of CMUSHView::GetSelection

// get selection into a string
bool CMUSHView::GetSelection(CString & s) 
  {
char * p = NULL;

  try
    {
    CMemFile f;      // open memory file for writing

    if (GetSelection (f))
      return true;     // error in producing selection

    int nLength = f.GetLength ();
    p = (char *) f.Detach ();

    s = CString (p, nLength);

    free (p);   // remove memory allocated in CMemFile

    }   // end of try block

  catch (CException* e)
    {
    if (p)
      free (p);   // remove memory allocated in CMemFile
    e->ReportError();
    e->Delete();
    return true;
    }

  return false; // OK

  }

BOOL CMUSHView::PreTranslateMessage(MSG* pMsg) 
{
  if (::IsWindow(m_ToolTip.m_hWnd) && pMsg->hwnd == m_hWnd)
  {
    switch(pMsg->message)
    {
    case WM_LBUTTONDOWN:  
    case WM_MOUSEMOVE:
    case WM_LBUTTONUP:  
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:  
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
      m_ToolTip.RelayEvent(pMsg);
      break;
    }
  }
  
  // F1 becomes a macro if wanted
 if ( pMsg->message == WM_KEYDOWN && App.m_bF1macro ) 
   { 
   map<long, WORD>::const_iterator it;
   CMUSHclientDoc* pDoc = GetDocument();
   ASSERT_VALID(pDoc);

   if ( pMsg->wParam == VK_F1 ) 
     { 
     if (GetKeyState (VK_SHIFT) < 0 &&  // shift
        GetKeyState (VK_CONTROL) >= 0)  // but not control
       {
       it = pDoc->m_AcceleratorToCommandMap.find (
         ((FVIRTKEY | FNOINVERT | FSHIFT) << 16) | VK_F1);

       if (it != pDoc->m_AcceleratorToCommandMap.end ())
         OnAcceleratorCommand (it->second);
       else
         OnMacroCommand (ID_MACRO_SHIFT_F1); 
       return TRUE; // No more processing 
       }
     if (GetKeyState (VK_CONTROL) < 0 &&  // control
        GetKeyState (VK_SHIFT) >= 0)  // but not shift
       {
       it = pDoc->m_AcceleratorToCommandMap.find (
         ((FVIRTKEY | FNOINVERT | FCONTROL) << 16) | VK_F1);

       if (it != pDoc->m_AcceleratorToCommandMap.end ())
         OnAcceleratorCommand (it->second);
       else
         OnMacroCommand (ID_MACRO_CTRL_F1); 
       return TRUE; // No more processing 
       }
     if (GetKeyState (VK_CONTROL) >= 0 &&  // not control
        GetKeyState (VK_SHIFT) >= 0)  // and not shift
       {
       it = pDoc->m_AcceleratorToCommandMap.find (
         ((FVIRTKEY | FNOINVERT) << 16) | VK_F1);
       if (it != pDoc->m_AcceleratorToCommandMap.end ())
         OnAcceleratorCommand (it->second);
       else
         OnMacroCommand (ID_MACRO_F1);
       return TRUE; // No more processing 
       }
     } 
   }    // end of WM_KEYDOWN &&  m_bF1macro

  return CView::PreTranslateMessage(pMsg);
}

BOOL CMUSHView::OnToolTipNeedText(UINT id, NMHDR * pNMHDR, LRESULT * pResult)
{
  BOOL bHandledNotify = FALSE;

  CPoint CursorPos;
  VERIFY(::GetCursorPos(&CursorPos));
  ScreenToClient(&CursorPos);

  CRect ClientRect;
  GetClientRect(ClientRect);

  // Make certain that the cursor is in the client rect, because the
  // mainframe also wants these messages to provide tooltips for the
  // toolbar.
  if (ClientRect.PtInRect(CursorPos))
  {
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;

    CMUSHclientDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);


    int line,
        col;

    if (!pDoc->m_FontHeight)
      return FALSE;

    if (Mouse_Tooltip_MiniWindow (pDoc, CursorPos, pNMHDR))
      return TRUE;

    // work out where the cursor is in the scroll view, and find the line it is on
    CClientDC dc(this);
    OnPrepareDC(&dc);
    dc.DPtoLP(&CursorPos);


    // another check for the rectangle, now we are past the miniwindow stuff
    ClientRect = GetTextRectangle ();

    if (!ClientRect.PtInRect(CursorPos))
      return FALSE;

    dc.SelectObject(pDoc->m_font [0]);
    if (calculate_line_and_column (CursorPos, dc, line, col, false))
       pTTT->szText [0] = 0;    // cursor is outside a line
    else
      {
      CLine * pLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (line));
      CString strText;

      // see if it is a hyperlink, and if so display the text of what the link will do
      unsigned int iStyle;
      CStyle * pStyle;
      POSITION foundpos;
      int iCol = 0;

    // don't do it *past* end of last word
    long pixel = calculate_width (line, pLine->len, pDoc, dc) + pDoc->m_iPixelOffset + pDoc->m_TextRectangle.left;

//    m_ToolTip.SendMessage (TTM_SETTITLE, TTI_INFO, (LPARAM) Translate ("Line information") );

    if (CursorPos.x < pixel && 
        pDoc->FindStyle (pLine, col, iCol, pStyle, foundpos))
        {
        iStyle = pStyle->iFlags;
        if (pStyle->pAction &&
            !pStyle->pAction->m_strAction.IsEmpty () &&
            pStyle->pAction->m_strAction.Find ("&text;") == -1)
          if ((iStyle & ACTIONTYPE) == ACTION_SEND ||
            (iStyle & ACTIONTYPE) == ACTION_PROMPT)

            {

            CString strActions = pStyle->pAction->m_strAction;   // action
            CString strHints = pStyle->pAction->m_strHint;   // hints, if any

            CStringList actionsList,
                        hintsList;

            StringToList (strActions, POPUP_DELIMITER, actionsList);
            StringToList (strHints,   POPUP_DELIMITER, hintsList);

            // hint defaults to the default action
            strText = actionsList.GetHead ();

            // if we have any hints, that can be the flyover text
            // it will either be the special hint (extra first item)
            // or the hint for the first action item

            if (!hintsList.IsEmpty ())
              {
               strText = hintsList.GetHead ();
              // don't say "Line information" for MXP hints
              m_ToolTip.SendMessage (TTM_SETTITLE, TTI_NONE, (LPARAM) "" );
              }

            } // end of ACTION_SEND  or ACTION_PROMPT
          else
          if ((iStyle & ACTIONTYPE) == ACTION_HYPERLINK)
            {
            if (pStyle->pAction->m_strHint.IsEmpty ())
              strText = pStyle->pAction->m_strAction;
            else
              strText = pStyle->pAction->m_strHint;

            // don't say "Line information" for MXP hints
            m_ToolTip.SendMessage (TTM_SETTITLE, TTI_NONE, (LPARAM) "" );
            } // end of ACTION_HYPERLINK


        } // end of column found

      // here for just the line information
      if (strText.IsEmpty ())
        {
        
        // if not wanted, don't do it
        if (!pDoc->m_bLineInformation)
          return FALSE;

        char * p = "";

        // Adjust the text by filling in TOOLTIPTEXT
        CString strDate;
        strDate = pLine->m_theTime.Format (TranslateTime ("%A, %B %d, %#I:%M:%S %p"));

        if (pLine->flags & COMMENT)
          p = ", (note)";
        else
          if (pLine->flags & USER_INPUT)
            p = ", (input)";

        strText = TFormat ("Line %ld, %s%s", 
                        pLine->m_nLineNumber,
                        (LPCTSTR) strDate,
                        p);
        }   // end of not being a hyperlink

//      ::strcpy(pTTT->szText, strText.Left (sizeof(pTTT->szText) - 1));

      // allow more text
      static char buf [1000];
      strcpy (buf, strText.Left (sizeof(buf) - 1));
      pTTT->lpszText = buf;


      return TRUE;
      }   // end of line being in range (ie. cursor over actual line)

    bHandledNotify = TRUE;
  }
  return bHandledNotify;
}

void CMUSHView::RemoveToolTip (void)
  {
  // get rid of tooltip, it is incorrect if we scroll
  if (::IsWindow(m_ToolTip.m_hWnd))
    {
    m_ToolTip.Activate(FALSE);    
    m_nLastToolTipLine = 0;
    m_nLastToolTipColumn = 0;
    }
  } // end of CMUSHView::RemoveToolTip

void CMUSHView::OnFindAgainBackwards() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

pDoc->m_DisplayFindInfo.m_bForwards = false;
m_bottomview->DoFind(true);
}

void CMUSHView::OnUpdateFindAgainBackwards(CCmdUI* pCmdUI) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);
  
pCmdUI->Enable (!pDoc->m_DisplayFindInfo.m_strFindStringList.IsEmpty ());
}

void CMUSHView::OnFindAgainForwards() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

pDoc->m_DisplayFindInfo.m_bForwards = true;
m_bottomview->DoFind(true);
  
}

void CMUSHView::OnUpdateFindAgainForwards(CCmdUI* pCmdUI) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

pCmdUI->Enable (!pDoc->m_DisplayFindInfo.m_strFindStringList.IsEmpty ());
  
}

void CMUSHView::OnDisplayGotourl() 
{
CString strSelection;

  if (GetSelection (strSelection))
    return;     // error in producing selection

  CString strSelectionUpper;

  strSelection.TrimLeft ();
  strSelection.TrimRight ();

  strSelectionUpper = strSelection;
  strSelectionUpper.MakeUpper ();

  if (strSelection.IsEmpty ())
    ::TMessageBox ("No URL selected", MB_ICONEXCLAMATION);
  else if (strSelection.GetLength () > 512)
    ::TMessageBox ("URL too long", MB_ICONEXCLAMATION);
  else
    {
    if (strSelectionUpper.Left (15).Find ("://") == -1)
      strSelection = "http://" + strSelection;
    
    // fix problem with imbedded quotes
    strSelection.Replace ("\"", "\\\"");

    if ((long) ShellExecute (Frame, _T("open"), strSelection, NULL, NULL, SW_SHOWNORMAL) <= 32)
      ::UMessageBox(TFormat ("Unable to open the URL \"%s\"", (const char *) strSelection), 
                      MB_ICONEXCLAMATION);
    }   // end of non-empty selection


// put the focus back in the send window

  OnKeysActivatecommandview ();
  
}


void CMUSHView::OnDisplaySendmailto() 
{
CString strSelection;


  if (GetSelection (strSelection))
    return;     // error in producing selection

  CString strSelectionUpper;


  strSelection.TrimLeft ();
  strSelection.TrimRight ();

  strSelectionUpper = strSelection;
  strSelectionUpper.MakeUpper ();

  if (strSelection.IsEmpty ())
    ::TMessageBox ("No email address selected", MB_ICONEXCLAMATION);
  else if (strSelection.GetLength () > 128)
    ::TMessageBox ("Email address too long", MB_ICONEXCLAMATION);
  else if (strSelection.Find ('@') == -1)
    ::UMessageBox (TFormat ("Email address \"%s\" invalid - does not contain a \"@\"",
      (const char *) strSelection), MB_ICONEXCLAMATION);
  else
    {
    if (strSelectionUpper.Left (7) != "MAILTO:")
      strSelection = "mailto:" + strSelection;

    if ((long) ShellExecute (Frame, _T("open"), strSelection, NULL, NULL, SW_SHOWNORMAL) <= 32)
      ::UMessageBox(TFormat ("Unable to send mail to \"%s\"", (const char *) strSelection), 
                      MB_ICONEXCLAMATION);
    }   // end of non-empty selection


// put the focus back in the send window

  OnKeysActivatecommandview ();
  
}

void CMUSHView::OnEditSelectAll() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

  m_selstart_line = 0;
  m_selstart_col = 0;
  m_selend_line = pDoc->m_LineList.GetCount ();
  m_selend_col = pDoc->m_nWrapColumn;

  SelectionChanged ();

  Invalidate ();

#if REDRAW_DEBUG
    RECT r;
    GetClientRect (&r);
    ShowInvalidatedRect (this, r);
#endif
}


BOOL CMUSHView::PreCreateWindow(CREATESTRUCT& cs) 
{

  cs.style &= ~FWS_ADDTOTITLE;
  cs.style  |= WS_VSCROLL;

  return CView::PreCreateWindow(cs);
}


void CMUSHView::ScrollToPosition ( POINT pt, const bool bSmooth)
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

SCROLLINFO ScrollInfo;
RECT r;
int iDeltaY = m_scroll_position.y - pt.y;

  RemoveToolTip ();
  GetClientRect (&r);
  // if we can do a smooth scroll, well let's do it!
  if (abs (iDeltaY) < GetOutputWindowHeight ())
    {
// very smooth scrolling

    int iSmoothDelta = iDeltaY < 0 ? -1 : 1;

    if (App.m_bSmoothScrolling)
      {
      for (int i = 0; i < abs (iDeltaY); i++)
        {
        m_scroll_position.y -= iSmoothDelta;
        // update scroll bar
        GetScrollInfo (SB_VERT, &ScrollInfo, SIF_POS);
        ScrollInfo.nPos = m_scroll_position.y;
        SetScrollInfo (SB_VERT, &ScrollInfo, pDoc->m_bScrollBarWanted);
        ScrollWindow (0, iSmoothDelta);
        UpdateWindow ();
        }
      } // end of smooth scrolling
    else
      {
      Invalidate ();
      m_scroll_position = pt;
      if (App.m_bSmootherScrolling)
        UpdateWindow ();    // redraw immediately if wanted
      } // end of not smooth scrolling
    }
  else
    // more than a screenfull away? just redraw the whole lot
    {
    Invalidate ();
#if REDRAW_DEBUG
    RECT r;
    GetClientRect (&r);
    ShowInvalidatedRect (this, r);
#endif
    }

#if REDRAW_DEBUG
    CRgn rgn;
    GetUpdateRgn(&rgn);
    ShowInvalidatedRgn (this, rgn);
#endif

  m_scroll_position = pt;

  // update scroll bar
  GetScrollInfo (SB_VERT, &ScrollInfo, SIF_POS);
  ScrollInfo.nPos = pt.y;
  SetScrollInfo (SB_VERT, &ScrollInfo, pDoc->m_bScrollBarWanted);

//  GetScrollInfo (SB_HORZ, &ScrollInfo, SIF_POS);
//  ScrollInfo.nPos = pt.x;
//  SetScrollInfo (SB_HORZ, &ScrollInfo, pDoc->m_bScrollBarWanted);
  
  if (pDoc->m_bAutoFreeze)
    m_freeze = GetScrollPos (SB_VERT) < GetScrollLimit(SB_VERT);

  } // end of CMUSHView::ScrollToPosition

void CMUSHView::SetScrollSizes (SIZE sizeTotal, 
                     const SIZE& sizePage, 
                     const SIZE& sizeLine)
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);
SCROLLINFO ScrollInfo;

  m_scroll_limit = sizeTotal;

  GetScrollInfo (SB_VERT, &ScrollInfo, SIF_ALL);
  ScrollInfo.nMin = 0;
  ScrollInfo.nMax = sizeTotal.cy - 1;
  ScrollInfo.nPage = sizePage.cy;
  SetScrollInfo (SB_VERT, &ScrollInfo, pDoc->m_bScrollBarWanted);

  /*
  GetScrollInfo (SB_HORZ, &ScrollInfo, SIF_ALL);
  ScrollInfo.nMin = 0;
  ScrollInfo.nMax = sizeTotal.cx - 1;
  ScrollInfo.nPage = sizePage.cx;
  SetScrollInfo (SB_HORZ, &ScrollInfo, pDoc->m_bScrollBarWanted);
*/

  } // end of CMUSHView::SetScrollSizes

void CMUSHView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);


int line,
    col;

  if (!pDoc->m_FontHeight)
    return;

  // CView changes the viewport origin and mapping mode.
  // It's necessary to convert the point from device coordinates
  // to logical coordinates, such as are stored in the document.
  CClientDC dc(this);
  OnPrepareDC(&dc);
  dc.DPtoLP(&point);

  // if click in mini-window, don't continue
  if (Mouse_Down_MiniWindow (pDoc, point, 0x50))  // LH mouse - double click
    return;


  // check for the rectangle, now we are past the miniwindow stuff
  CRect ClientRect = GetTextRectangle (true);

  if (!ClientRect.PtInRect(point))
    return;

  dc.SelectObject(pDoc->m_font [0]);

  m_last_mouse_position = point;

  CRgn oldrgn;
  CRgn newrgn;

// Get old selection region

  get_selection (oldrgn);
  
// find which line and column the mouse position is at
              
  calculate_line_and_column (point, dc, line, col, false);

  POSITION pos = pDoc->GetLinePosition (line);

  CString strCurrentLine;

  m_selstart_col = m_selend_col = col;
  m_selstart_line = m_selend_line = line;

  // SHIFT-double-click to select a paragraph
  if (nFlags & MK_SHIFT)
    {
    POSITION prevpos = NULL;
    CLine * pLine;

     pLine = pDoc->m_LineList.GetPrev (pos);    // must go back at least one line
     while (pos)
       {
       prevpos = pos;   // remember line which did have a hard return
       pLine = pDoc->m_LineList.GetPrev (pos);
       if (pLine->hard_return)
         break;
       m_selstart_line--;
       }

     // if prevpos is non-null it is now the position of the last line with a hard return
     // so, get the next one, that is the one which starts *our* sequence

     if (prevpos)
        pDoc->m_LineList.GetNext (prevpos);
     else       // must be the only line in the buffer
       {
        prevpos = pDoc->m_LineList.GetHeadPosition ();
        m_selstart_line = 0;
       }

     m_selend_line = m_selstart_line;
     // prevpos now points to the first line from the previous batch of lines
     pos = prevpos;
     do
       {
       pLine = pDoc->m_LineList.GetNext (pos);
       strCurrentLine += CString (pLine->text, pLine->len);
       if (!pLine->hard_return)
         m_selend_line++;
       }  while (!pLine->hard_return && pos);

     m_selstart_col = 0;
     m_selend_col = pLine->len;

    }
  else
    {
    CLine * pLine = pDoc->m_LineList.GetAt (pos);
    while (m_selstart_col >= 0 && 
          !isspace ((unsigned char) pLine->text [m_selstart_col]) &&
          strchr (App.m_strWordDelimitersDblClick, pLine->text [m_selstart_col]) == NULL)
      m_selstart_col--;
    m_selstart_col++;   // now onto the start of that word

    m_pin_col = m_selstart_col;

    point.x = calculate_width (line, col, pDoc, dc);

    // a word will end on a space, or whatever
    while (m_selend_col < pLine->len && 
          !isspace ((unsigned char) pLine->text [m_selend_col]) &&
          strchr (App.m_strWordDelimitersDblClick, pLine->text [m_selend_col]) == NULL)
      m_selend_col++;

    if (m_selend_col > m_selstart_col)
      strCurrentLine = CString (&pLine->text [m_selstart_col], 
                                m_selend_col - m_selstart_col);
    }

// Get new selection region

  if (!get_selection (newrgn) && pDoc->m_bAutoFreeze)
     m_freeze = true;   // freeze output so they can copy or print it

  // Invalidate new stuff
  Invalidate ();

#if REDRAW_DEBUG
  ShowInvalidatedRgn (this, oldrgn);
#endif

  // Finished with regions
  newrgn.DeleteObject ();
  oldrgn.DeleteObject ();

  SelectionChanged ();

  // if they want, send word to MUD
  if (!strCurrentLine.IsEmpty ())
    if (pDoc->m_bDoubleClickSends)
      {
      if (pDoc->CheckConnected ())
        return;

      m_bottomview->AddToCommandHistory (strCurrentLine);
      pDoc->SendMsg (strCurrentLine, pDoc->m_display_my_input, false, pDoc->LoggingInput ());
      }   // end of sending word to MUD
    else if (pDoc->m_bDoubleClickInserts)
      {
      m_bottomview->GetEditCtrl().ReplaceSel (strCurrentLine);
      }   // end of copying word into command buffer

  // make sure selection is meaningful
  if (m_selstart_line == m_selend_line && 
      m_selend_col <= m_selstart_col)
    {
    m_selstart_col = 0;
    m_selend_col = 0;
    }

  // auto-copy selection to clipboard
  if (pDoc->m_bCopySelectionToClipboard)
    if (pDoc->m_bAutoCopyInHTML)
      OnEditCopyashtml ();
    else
      OnEditCopy ();

}

void CMUSHView::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);


  // CView changes the viewport origin and mapping mode.
  // It's necessary to convert the point from device coordinates
  // to logical coordinates, such as are stored in the document.
  CPoint mwpoint = point;
  CClientDC dc(this);
  OnPrepareDC(&dc);
  dc.DPtoLP(&mwpoint);

  // if click in mini-window, don't continue
  if (Mouse_Down_MiniWindow (pDoc, mwpoint, 0x60))  // RH mouse - double click
    return;
  
  CView::OnRButtonDblClk(nFlags, point);
}


void CMUSHView::OnDisplayGotobookmark() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

POSITION startpos, pos;
CLine * pLine = NULL,
      * pInitialLine;
int nNewLine = m_selstart_line,
    nInitialLine;

  // empty buffer? can't do it
  if (pDoc->m_LineList.IsEmpty ())
    return;

  // first find the position of the current selection, or start of buffer if none
  if (m_selstart_line <= 0 || m_selstart_line >= pDoc->m_LineList.GetCount ())
    {
    nNewLine = 0;
    pos = pDoc->m_LineList.GetHeadPosition ();
    }
  else
    pos = pDoc->GetLinePosition (m_selstart_line);

  startpos = pos;
  pInitialLine = pDoc->m_LineList.GetNext (pos);   // skip current position
  nInitialLine = nNewLine;

  while (pos)
    {
    nNewLine++;
    pLine = pDoc->m_LineList.GetNext (pos);
    if (pLine->flags & BOOKMARK)
      break;
    else
      pLine = NULL;

    // if we come to the end, loop back to the start
    if (!pos)
      {
      pos = pDoc->m_LineList.GetHeadPosition ();
      nNewLine = -1;
      }

    if (startpos == pos)
      break;  // reached start again? don't loop forever
    }   // end of loop

  // if we didn't find one *but* we were on one anyway, re-go to it

  if (!pLine && pInitialLine->flags & BOOKMARK)
    {
    pLine = pInitialLine;
    nNewLine = nInitialLine;
    }

  // if we found one, select it
  if (pLine)
    SelectLine (nNewLine);
  
} // end of MUSHView::OnDisplayGotobookmark

void CMUSHView::OnDisplayBookmarkselection() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

  // empty buffer? can't do it
  if (pDoc->m_LineList.IsEmpty ())
    return;

  CRgn oldrgn;
  CRgn newrgn;

// Get old selection region

  get_selection (oldrgn);

  // if no selection (or out of range) select last line in display
  if (m_selstart_line <= 0 || m_selstart_line >= pDoc->m_LineList.GetCount ())
    m_selstart_line = pDoc->GetLastLine () - 1;

  CLine * pLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (m_selstart_line));

  pLine->flags ^= BOOKMARK;   // toggle bookmark

  m_selstart_col = 0;
  m_selend_col = 0;
  m_selend_line = m_selstart_line;    

  if (pLine->flags & BOOKMARK)
    m_selend_col = pLine->len;

  SelectionChanged ();

// Get new selection region

  get_selection (newrgn);

  // Combine previously selected with new selection, excluding union
  oldrgn.CombineRgn (&oldrgn, &newrgn, RGN_XOR);

  // Invalidate new stuff
  InvalidateRgn (&oldrgn);

#if REDRAW_DEBUG
  ShowInvalidatedRgn (this, oldrgn);
#endif

  // Finished with regions
  newrgn.DeleteObject ();
  oldrgn.DeleteObject ();

  
}   // end of CMUSHView::OnDisplayBookmarkselection


void CMUSHView::doBookmark (void)
  {
  OnDisplayBookmarkselection ();
  } // end of CMUSHView::doBookmark

void CMUSHView::doGotoBookmark (void)
  {
  OnDisplayGotobookmark ();
  } // end of CMUSHView::doGotoBookmark


void CMUSHView::EnsureSelectionVisible (void)
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

  if (!pDoc->m_FontHeight)
    return;

  CPoint pt = GetScrollPosition ();
   
RECT r;

GetClientRect (&r);

  int startline = (pt.y - pDoc->m_iPixelOffset) / pDoc->m_FontHeight + 1;
  int endline =  ((pt.y + GetOutputWindowHeight ()) / pDoc->m_FontHeight) - 1;

  if (startline < 0)
    startline = 0;

// include partial last line if necessary

  int lastline = pDoc->GetLastLine ();

  if (endline > lastline)
    endline = lastline;

  if (m_selend_line < startline || m_selstart_line > endline)
    {
    // scroll so bookmark is halfway down page
    CPoint pt (0, m_selstart_line * pDoc->m_FontHeight -
                  (GetOutputWindowHeight ()) / 2);

    if (pt.y < 0)
      pt.y = 0;

    CPoint lastline_pt;

    // include partial last line if necessary

    lastline_pt.y =  (lastline * pDoc->m_FontHeight) - GetOutputWindowHeight ();
    lastline_pt.x = 0;

    if (lastline_pt.y < 0)
      lastline_pt.y = 0;

    // make sure we don't scroll so that the last line is half-way up the screen
    if (pt.y > lastline_pt.y)
      pt.y = lastline_pt.y;
    else
      m_freeze = true;


    ScrollToPosition (pt, App.m_bSmoothScrolling);
    } // end of selection not visible

  } // end of CMUSHView::EnsureSelectionVisible

BOOL CMUSHView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);


CRect rect;
  
  m_bottomview->GetClientRect (&rect);
  m_bottomview->ClientToScreen(&rect);

  // if mouse is over lower window (command window) then let it scroll instead
  if (rect.PtInRect (pt))
    return m_bottomview->OnMouseWheel (nFlags, zDelta, pt);


unsigned int iScrollLines;


  SystemParametersInfo (SPI_GETWHEELSCROLLLINES, 0, &iScrollLines, 0);

  if (iScrollLines == 0)
    return 0;   // scrolling not enabled

  CPoint point (pt);
  CClientDC dc(this);
  // CView changes the viewport origin and mapping mode.
  // It's necessary to convert the point from device coordinates
  // to logical coordinates, such as are stored in the document.
  OnPrepareDC(&dc);

  dc.DPtoLP(&point);

  // if over miniwindow, don't keep going

  if (Mouse_Wheel_MiniWindow (pDoc, point, zDelta < 0 ? 0x100 : 0))
    return 1;

    /*
     * Don't handle zoom and datazoom.
     */
    
  if (nFlags & MK_CONTROL || nFlags & MK_SHIFT) 
    return CView::OnMouseWheel(nFlags, zDelta, pt);

  if (iScrollLines == WHEEL_PAGESCROLL)
    {
    if (zDelta < 0)   // page down
      doPagedown ();
    else
      doPageup ();

    return 1;
    }

  // scroll lines not pages

bool bSmooth_saved = App.m_bSmoothScrolling;

  App.m_bSmoothScrolling = false;

  if (zDelta < 0) // line down
    {
    zDelta = 0 - zDelta;  // make positive
    zDelta /= WHEEL_DELTA;
    zDelta *= iScrollLines;

/*   old way
    for (int i = 0; i < zDelta; i++)
      doLinedown ();

*/

// -- new way

    RECT r;
    int lastline;


    CPoint ScrollPt = GetScrollPosition ();

    GetClientRect (&r);

    ScrollPt.y += zDelta * pDoc->m_FontHeight;

    // include partial last line if necessary

    lastline = pDoc->GetLastLine ();

    // limit to end of screen
    if (ScrollPt.y > ((lastline * pDoc->m_FontHeight) - GetOutputWindowHeight ()))
      ScrollPt.y =  (lastline * pDoc->m_FontHeight) - GetOutputWindowHeight ();

    ScrollToPosition (ScrollPt, false);

// -- end new way

    App.m_bSmoothScrolling = bSmooth_saved;
    return 1;
    }

  // line up

  zDelta /= WHEEL_DELTA;
  zDelta *= iScrollLines;


/*   old way

  for (int i = 0; i < zDelta; i++)
    doLineup ();

*/

// -- new way

  RECT r;

  CPoint ScrollPt = GetScrollPosition ();

  GetClientRect (&r);

  ScrollPt.y -= zDelta * pDoc->m_FontHeight;

  if (ScrollPt.y < 0)
    ScrollPt.y = 0;

  ScrollToPosition (ScrollPt, false);

// -- end new way

  App.m_bSmoothScrolling = bSmooth_saved;

  return 1;
}

void CMUSHView::OnAltUparrow() 
{
// alt+up+arrow is received in the top view, activate bottom view and pass command on

  OnKeysActivatecommandview ();  
  m_bottomview->SendMessage(WM_COMMAND, ID_ALT_UPARROW, 0);
  
}

void CMUSHView::OnAltDownarrow() 
{
// alt+down+arrow is received in the top view, activate bottom view and pass command on

  OnKeysActivatecommandview ();  
  m_bottomview->SendMessage(WM_COMMAND, ID_ALT_DOWNARROW, 0);
  
}

void CMUSHView::OnDisplayTextattributes() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

CString str;

long startcol,
     endcol;

// return if no valid selection

  CLine * pStartLine,
        * pEndLine;

  pStartLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (m_selstart_line));
  pEndLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (m_selend_line));
  
  startcol  = MIN (m_selstart_col, pStartLine->len);
  endcol    = MIN (m_selend_col, pEndLine->len);

  if (!(m_selend_line > m_selstart_line || 
              (m_selend_line == m_selstart_line && 
               endcol > startcol)))
    return;

unsigned int iStyle;
CStyle * pStyle;
int iCol;
POSITION foundpos;

if (!pDoc->FindStyle (pStartLine, startcol + 1, iCol, pStyle, foundpos))
  {
  ::TMessageBox ("Cannot find style of this character");
  return;
  }

iStyle = pStyle->iFlags;

char c = pStartLine->text [startcol];

char * sColours [8] = 
  {
  "Black",
  "Red",
  "Green",
  "Yellow",
  "Blue",
  "Magenta",
  "Cyan",
  "White"
   };

CTextAttributesDlg dlg;

  dlg.m_pDoc = pDoc;
  dlg.m_iLine = m_selstart_line + 1;    // make 1-relative
  dlg.m_pLine = pStartLine; 

  dlg.m_strLetter = c;
  if ((iStyle & COLOURTYPE) == COLOUR_CUSTOM)
    {
    ASSERT (pStyle->iForeColour >= 0 && pStyle->iForeColour < MAX_CUSTOM);
    dlg.m_strTextColour = "Custom";
    dlg.m_strBackColour = "Custom";
    dlg.m_strCustomColour = pDoc->m_strCustomColourName [pStyle->iForeColour];
    }
  else
  if ((iStyle & COLOURTYPE) == COLOUR_RGB)
    {
    dlg.m_strTextColour = CFormat ("R=%i, G=%i, B=%i", 
                                   GetRValue (pStyle->iForeColour),
                                   GetGValue (pStyle->iForeColour),
                                   GetBValue (pStyle->iForeColour));
    dlg.m_strBackColour = CFormat ("R=%i, G=%i, B=%i", 
                                   GetRValue (pStyle->iBackColour),
                                   GetGValue (pStyle->iBackColour),
                                   GetBValue (pStyle->iBackColour));                                  
    dlg.m_strCustomColour = "RGB";
    }
  else
    {
    ASSERT (pStyle->iForeColour >= 0 && pStyle->iForeColour < 8);
    ASSERT (pStyle->iBackColour >= 0 && pStyle->iBackColour < 8);
    dlg.m_strTextColour = sColours [pStyle->iForeColour & 7];
    dlg.m_strBackColour = sColours [pStyle->iBackColour & 7];
    dlg.m_strCustomColour = "n/a";
    }

  dlg.m_bBold    = (iStyle & HILITE) != 0;
  dlg.m_bItalic  = (iStyle & BLINK) != 0;
  dlg.m_bInverse = (iStyle & INVERSE) != 0;

  if ((iStyle & CHANGED) == 0)
    dlg.m_strModified.Empty ();
  else
    dlg.m_strModified = "The colour or style HAS been modified by a trigger.";

  COLORREF colour1, colour2;

  pDoc->GetStyleRGB (pStyle, colour1, colour2); // find what colour this style is

  dlg.m_strTextColourRGB = CFormat ("#%02X%02X%02X",
                            GetRValue (colour1),
                            GetGValue (colour1),
                            GetBValue (colour1)
                            );

  dlg.m_strBackgroundColourRGB = CFormat ("#%02X%02X%02X",
                            GetRValue (colour2),
                            GetGValue (colour2),
                            GetBValue (colour2)
                            );

  dlg.m_ctlTextSwatch.m_colour = colour1;
  dlg.m_ctlBackgroundSwatch.m_colour = colour2;

  dlg.DoModal ();

}

void CMUSHView::OnEditNotesworkarea() 
{
  CMUSHclientDoc* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  // see if they want an existing one
  if (pDoc->SwitchToNotepad ())
    return;

CString strSelection;

  GetSelection (strSelection);    // get selection, if any


  // edit current input window
  CreateTextWindow (strSelection,     // contents
                    TFormat ("Notepad: %s", (LPCTSTR) pDoc->m_mush_name),     // title
                    pDoc,   // document
                    pDoc->m_iUniqueDocumentNumber,      // document number
                    pDoc->m_input_font_name,
                    pDoc->m_input_font_height,
                    pDoc->m_input_font_weight,
                    pDoc->m_input_font_charset,
                    pDoc->m_input_text_colour,
                    pDoc->m_input_background_colour,
                    "",       // search string
                    "",       // line preamble
                    false,
                    false,
                    false,
                    false,  
                    false,
                    false,
                    eNotepadNormal
                    );
  
  
}

void CMUSHView::OnDisplayRecalltext() 
{
  CMUSHclientDoc* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

CString strSelection;

  GetSelection (strSelection);

  // if no selection, or more than one paragraph, ask what to recall
  if (strSelection.IsEmpty () || strSelection.Find (ENDLINE) != -1)
    {
    pDoc->DoRecallText ();
    return;
    }


CString strMessage;

  strMessage = pDoc->RecallText (
                                strSelection,   // what to search for
                                false,    // match case
                                false,    // regexp
                                true,     // search commands
                                true,     // search output
                                true,     // search notes
                                0,         // all lines
                                pDoc->m_strRecallLinePreamble);  // default preamble    

  CreateTextWindow (strMessage, 
                    TFormat ("Recall: %s",
                       (LPCTSTR) strSelection),
                    pDoc,
                    pDoc->m_iUniqueDocumentNumber,
                    pDoc->m_font_name,
                    pDoc->m_font_height,
                    pDoc->m_font_weight,
                    pDoc->m_font_charset,
                    pDoc->m_normalcolour [WHITE],
                    pDoc->m_normalcolour [BLACK],
                    strSelection,
                    pDoc->m_strRecallLinePreamble,
                    false,    // match case
                    false,    // regexp
                    true,     // search commands
                    true,     // search output
                    true,     // search notes
                    0,       // all lines
                    eNotepadRecall);
  
}

/* If AllTypingToCommandWindow is on, we redirect all keyboard messages to the bottom view. */

void CMUSHView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  if (App.m_bAllTypingToCommandWindow) {
    OnKeysActivatecommandview ();  
    m_bottomview->SendMessage(WM_KEYDOWN, nChar, nRepCnt | (nFlags << 16));
  } else
    CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CMUSHView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  if (App.m_bAllTypingToCommandWindow) {
    OnKeysActivatecommandview();
    m_bottomview->SendMessage(WM_SYSKEYDOWN, nChar, nRepCnt | (nFlags << 16));
  } else
    CView::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

void CMUSHView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  if (App.m_bAllTypingToCommandWindow) {
    OnKeysActivatecommandview();
    m_bottomview->SendMessage(WM_KEYUP, nChar, nRepCnt | (nFlags << 16));
  } else
    CView::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CMUSHView::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  if (App.m_bAllTypingToCommandWindow) {
    OnKeysActivatecommandview();
    m_bottomview->SendMessage(WM_SYSKEYUP, nChar, nRepCnt | (nFlags << 16));
  } else
    CView::OnSysKeyUp(nChar, nRepCnt, nFlags);
}

BOOL CMUSHView::OnKeypadCommand(UINT nIDC) 
  {
// if numeric keypad commands are received in the top view, activate bottom view and pass command on

  OnKeysActivatecommandview ();  
  m_bottomview->SendMessage(WM_COMMAND, nIDC, 0);
  return TRUE;

  }

BOOL CMUSHView::OnMacroCommand(UINT nIDC) 
  {
// if macro commands are received in the top view, activate bottom view and pass command on

  OnKeysActivatecommandview ();  
  m_bottomview->SendMessage(WM_COMMAND, nIDC, 0);
  return TRUE;

  }

void CMUSHView::OnAcceleratorCommand (UINT nID)
  {
// if accelerator commands are received in the top view, activate bottom view and pass command on

  OnKeysActivatecommandview ();  
  m_bottomview->OnAcceleratorCommand (nID);
  }


void CMUSHView::OnMXPMenu (UINT nID)
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

  // send the appropriate menu item

  switch (iAction)
    {
    case ACTION_SEND:
      {
      if (pDoc->CheckConnected ())
        return;
      CString strAction = strMXP_menu_item [nID - MXP_FIRST_MENU];
      if (pDoc->m_bHyperlinkAddsToCommandHistory)
        m_bottomview->AddToCommandHistory (strAction);
      pDoc->m_tLastPlayerInput = CTime::GetCurrentTime();   // for <afk> tests
      pDoc->SendMsg (strAction, pDoc->m_bEchoHyperlinkInOutputWindow,
                    false, pDoc->LoggingInput ());
      }
      break;  // end of ACTION_SEND

    case ACTION_PROMPT:
      if (m_bottomview->CheckTyping (pDoc, strMXP_menu_item [nID - MXP_FIRST_MENU]))
        return;             
      m_bottomview->SetCommand (strMXP_menu_item [nID - MXP_FIRST_MENU]);
      break; // end of ACTION_PROMPT

    case ACTION_HYPERLINK:
      {
      CString strAction = strMXP_menu_item [nID - MXP_FIRST_MENU];

      // don't let them slip in arbitrary OS commands
      if (strAction.Left (7).CompareNoCase ("http://") != 0 &&
          strAction.Left (7).CompareNoCase ("mailto:") != 0)
        ::UMessageBox(TFormat ("Hyperlink action \"%s\" - permission denied.", 
                        (const char *) strAction), 
                        MB_ICONEXCLAMATION);
      else
        if ((long) ShellExecute (Frame, _T("open"), strAction, NULL, NULL, SW_SHOWNORMAL) <= 32)
          ::UMessageBox(TFormat ("Unable to open the hyperlink \"%s\"", 
                          (const char *) strAction), 
                          MB_ICONEXCLAMATION);


      }   
      break; // end of ACTION_HYPERLINK


    case ACTION_ALIAS:
      {
      CAlias * pAlias;
      CString strLabel;
      
      // strip off plugin id
      CStringList strList;
      StringToList (strMXP_menu_item [nID - MXP_FIRST_MENU], ":", strList);
      strLabel = strList.GetHead ();

      // alias might be in a plugin
      switch (strList.GetCount ())
        {
        case 1:   // main world
            if (!pDoc->m_AliasMap.Lookup (strLabel, pAlias))
              return;   // not there? strange
        break;

        case 2:   // plugin
          {
            CString strPluginID;
            strPluginID = strList.GetTail ();
            CPlugin * pPlugin = pDoc->GetPlugin (strPluginID);
            if (!pPlugin)
              return;   // plugin does not exist? strange
            if (!pPlugin->m_AliasMap.Lookup (strLabel, pAlias))
              return;   // not there? strange
          pDoc->m_CurrentPlugin = pPlugin;     // remember plugin so alias executes in correct space
          }

        break;
            default:   return;   // not 1 or 2 items? very strange
        } // end of switch


      CString strAction;

//---

        // copy contents to strAction area, substituting variables

        const char * p,
                   * start;
        long length;
        int subscript;

        CString strFixedContents = ::FixupEscapeSequences (pAlias->contents);
        p = start = strFixedContents;

        while (*p)
          {
          if (p [0] == '%' && isdigit (p [1]))
            {

        // copy up to the percent sign

            length = p - start;
    
            if (length > 0)
              strAction += CString (start, length);
                
            subscript = *++p - '0';

          // get ready for next batch from beyond the digit

            start = ++p;

            } // end of finding a percent sign, followed by a number
          else if (p [0] == '%' && toupper (p [1]) == 'C')
            {
            CString strClipboard;

            if (!GetClipboardContents (strClipboard, pDoc->m_bUTF_8))
              {
              pDoc->m_CurrentPlugin = NULL;
              return;
              }

        // copy up to the percent sign

            length = p - start;
          
            if (length > 0)
              strAction += CString (start, length);

            strAction += strClipboard;

  // get ready for next batch from beyond the 'c'

            p += 2;
            start = p;

            }   // end of finding %c
        else if (p [0] == '%' && toupper (p [1]) == '%')
          {
// copy up to the percent sign

         length = p - start;
      
          if (length > 0)
           strAction += CString (start, length);

          // get ready for next batch from beyond the %

          start = ++p;
          p++;    // don't reprocess the %

          }   // end of %%
         else if (pAlias->bExpandVariables && p [0] == '@')
           {
// copy up to the @ sign

         length = p - start;
      
          if (length > 0)
           strAction += CString (start, length);
            
          p++;    // skip the @

          // @@ becomes @
          if (*p == '@')
            {
            start = ++p;
            strAction += "@";
            continue;
            }

          const char * pName = p;

          // find end of variable name
          while (*p)
            if (*p == '_' || isalnum (*p))
              p++;
            else
              break;
          
          CString strName (pName, p - pName);

          if (strName.IsEmpty ())
            {
            ::TMessageBox("@ must be followed by a variable name");
            pDoc->m_CurrentPlugin = NULL;
            return;
            }

          CVariable * variable_item;

          strName.MakeLower ();
          if (!pDoc->m_VariableMap.Lookup (strName, variable_item))
            {
            ::UMessageBox(TFormat ("Variable '%s' is not defined.", (LPCTSTR) strName));
            pDoc->m_CurrentPlugin = NULL;
            return;
            }

          strAction += variable_item->strContents;

          // get ready for next batch from beyond the variable

          start = p;
           }
         else
            p++;
          } // end of processing each character in the replacement text

  // copy last group

        strAction += start;

//--- 


      if (pDoc->m_ScriptEngine)
        if (pAlias->dispid != DISPID_UNKNOWN)        // if we have a dispatch id
          {


          CString strType = "alias";
          CString strReason =  TFormat ("processing alias \"%s\"", 
                                        (LPCTSTR) pAlias->strLabel);

      if (pDoc->GetScriptEngine () && pDoc->GetScriptEngine ()->IsLua ())
            {
            list<double> nparams;
            list<string> sparams;
            sparams.push_back ((LPCTSTR) strMXP_menu_item [nID - MXP_FIRST_MENU]);
            sparams.push_back ((LPCTSTR) pAlias->name);
            pDoc->GetScriptEngine ()->ExecuteLua (pAlias->dispid, 
                                           pAlias->strProcedure, 
                                           eUserMenuAction,
                                           strType, 
                                           strReason, 
                                           nparams,
                                           sparams, 
                                           pAlias->nInvocationCount); 
            }   // end of Lua
          else
            {

            // execute any scripts associated with alias we found
            enum
              {
              eWildcards,
              eInputLine,
              eAliasName,
              eArgCount,     // this MUST be last
              };    

            COleSafeArray sa;   // for wildcard list
            COleVariant args [eArgCount];
            DISPPARAMS params = { args, NULL, eArgCount, 0 };

            args [eAliasName] = strMXP_menu_item [nID - MXP_FIRST_MENU];
            args [eInputLine] = pAlias->name;

            // --------------- set up wildcards array ---------------------------
            sa.Clear ();
            // nb - to be consistent with %1, %2 etc. we will make array 1-relative
            sa.CreateOneDim (VT_VARIANT, MAX_WILDCARDS, NULL, 1);
            for (long i = 1; i <= MAX_WILDCARDS; i++)
              {
              CString s ("");
              COleVariant v (s); // no wildcards are relevant
              sa.PutElement (&i, &v);
              }
            args [eWildcards] = sa;

            pDoc->ExecuteScript (pAlias->dispid,  
                                 pAlias->strProcedure,
                                 eUserMenuAction,
                                 strType, 
                                 strReason,
                                 params, 
                                 pAlias->nInvocationCount); 
            } // end of not Lua

          }     // end of having a dispatch ID

    // send the message

      CString strExtraOutput;

      pDoc->m_iCurrentActionSource = eUserMenuAction;

      pDoc->SendTo (pAlias->iSendTo, 
              strAction, 
              pAlias->bOmitFromOutput,
              pAlias->bOmitFromLog,
              TFormat ("Alias: %s", (LPCTSTR) pAlias->strLabel),
              pAlias->strVariable,
              strExtraOutput
              );

      pDoc->m_iCurrentActionSource = eUnknownActionSource;

      if (!strExtraOutput.IsEmpty ())
         pDoc->DisplayMsg (strExtraOutput, strExtraOutput.GetLength (), COMMENT);

      pDoc->m_tLastPlayerInput = CTime::GetCurrentTime();   // for <afk> tests

      pAlias->nMatched++;   // count alias matches

      }
      pDoc->m_CurrentPlugin = NULL;     // no plugin active right now
      break;  // end of ACTION_ALIAS
    } // end of switch on iAction

  // unpause the output window if wanted
  if (pDoc->m_bUnpauseOnSend && m_freeze)
    {
    m_freeze = false;
    addedstuff ();   
    }

  // put the focus back in the send window

  OnKeysActivatecommandview ();

  } // end of CMUSHView::OnMXPMenu


void CMUSHView::OnDisplayGotoline() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

CGoToLineDlg dlg;

  dlg.m_iMaxLine = pDoc->m_LineList.GetCount ();
  dlg.m_iLineNumber = pDoc->m_lastGoTo;
  
  if (dlg.DoModal () != IDOK)
    return;

  SelectLine (dlg.m_iLineNumber - 1); // line is zero-relative

  pDoc->m_lastGoTo = dlg.m_iLineNumber; // in case we want to go there again

}


void CMUSHView::doGotoLine (void)
  {
  OnDisplayGotoline ();
  } // end of CMUSHView::doGotoLine


void CMUSHView::SelectLine (const int iLine)
  {

  // select the found text, so it is highlighted

  m_selstart_line = iLine;
  m_selend_line = iLine + 1;
  m_selstart_col =  0;
  m_selend_col = 0;

  SelectionChanged ();


#if REDRAW_DEBUG
  ShowInvalidatedRgn (this, oldrgn);
#endif

  Invalidate ();

// freeze output, so they can study the found text at leisure

  EnsureSelectionVisible ();

  } // end of CMUSHView::SelectLine 


void CMUSHView::OnGameCommandhistory() 
{
m_bottomview->DoCommandHistory();
}

void CMUSHView::OnFileCtrlN() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

  if (pDoc->m_bCtrlNGoesToNextCommand)
    m_bottomview->DoNextCommand ();
  else
    App.OnFileNew (); 
  
}

void CMUSHView::OnFileCtrlP() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

  if (pDoc->m_bCtrlPGoesToPreviousCommand)
    m_bottomview->DoPreviousCommand ();
  else
    PrintWorld ();  
}

void CMUSHView::OnEditCtrlZ() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

  if (pDoc->m_bCtrlZGoesToEndOfBuffer)
     OnTestEnd ();
  
}

void OutputOneLine (CMUSHclientDoc* pDoc,
                    CArchive & ar, 
                    CLine * pLine, 
                    const int iStartCol, 
                    const int iEndCol,
                    const COLORREF defaultbackcolour,
                    COLORREF & lastforecolour,
                    COLORREF & lastbackcolour,
                    bool & bInSpan)
  {
  int iCol = 0;

  CString strText = CString (pLine->text, pLine->len);

  // can't do if no styles
  if (pLine->styleList.IsEmpty ())
     return;

  if (pLine->len <= 0)
     return;

  for (POSITION pos = pLine->styleList.GetHeadPosition(); pos; )
    {
    COLORREF colour1,
             colour2;

    CStyle * pStyle = pLine->styleList.GetNext (pos);

    int iLength = pStyle->iLength;

    // ignore zero length styles
    if (iLength <= 0)
       continue;

    // skip if before starting column
    if ((iCol + iLength) >= iStartCol)
      {

      // if it starts in this style run - adjust length down and column up
      if (iStartCol > iCol)
        {
        iLength -= iStartCol - iCol;
        iCol += iStartCol - iCol;
        }

      // if it ends in this style run - adjust length down

      if ((iCol + iLength) > iEndCol)
        iLength = iEndCol - iCol;

      // ignore zero length styles
      if (iLength <= 0)
         continue;

      pDoc->GetStyleRGB (pStyle, colour1, colour2); // find what colour this style is

      if (colour1 != lastforecolour || colour2 != lastbackcolour)
        {
        // cancel earlier span
        if (bInSpan)
          {
           ar.WriteString ("</span>");
           bInSpan = false;
          }

        ar.WriteString (CFormat ("</font><font color=\"#%02X%02X%02X\">",
                            GetRValue (colour1),
                            GetGValue (colour1),
                            GetBValue (colour1)
                            ));

        // for efficiency we will only use <span> if we need to change the
        // background colour
        if (colour2 != defaultbackcolour)
          {
          ar.WriteString (CFormat ("<span style=\"color: #%02X%02X%02X; "
                                   "background: #%02X%02X%02X\">",
                                      GetRValue (colour1),
                                      GetGValue (colour1),
                                      GetBValue (colour1),
                                      GetRValue (colour2),
                                      GetGValue (colour2),
                                      GetBValue (colour2)
                                      ));
          bInSpan = true;
          }
        lastforecolour = colour1;
        lastbackcolour = colour2;
        }

      if (pStyle->iFlags & UNDERLINE)
        ar.WriteString ("<u>");

  // bold actually looks a bit silly :)
  //    if (pStyle->iFlags & HILITE)
  //      ar.WriteString ("<b>");

      ar.WriteString (FixHTMLString (strText.Mid (iCol, iLength)));

  //    if (pStyle->iFlags & HILITE)
  //      ar.WriteString ("</b>");
      if (pStyle->iFlags & UNDERLINE)
        ar.WriteString ("</u>");
      }   // end of being before starting column

    iCol += iLength; // new column

    // past end column - stop

    if (iCol >= iEndCol)
      break;

    }   // end of doing each style

  } // end of OutputOneLine

void CMUSHView::OnEditCopyashtml() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

char * p = NULL;

  try
    {
    CMemFile f;      // open memory file for writing

    long startcol,
         endcol;

    int i;

  // return if no valid selection

    CLine * pStartLine,
          * pEndLine;

    pStartLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (m_selstart_line));
    pEndLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (m_selend_line));

    startcol  = MIN (m_selstart_col, pStartLine->len);
    endcol    = MIN (m_selend_col, pEndLine->len);

    if (!(m_selend_line > m_selstart_line || 
                (m_selend_line == m_selstart_line && 
                 endcol > startcol)))
      return;

    CArchive ar(&f, CArchive::store);


    COLORREF colour1 = 0,
             colour2 = 0,
             lastforecolour = 0,
             lastbackcolour = 0,
             defaultbackcolour;
    bool bInSpan = false;

    if (!pStartLine->styleList.IsEmpty ())
      {
      CStyle * pStartStyle = pStartLine->styleList.GetHead ();
      pDoc->GetStyleRGB (pStartStyle, colour1, colour2); 
      }

    defaultbackcolour = colour2;

    ar.WriteString (CFormat ("<!-- Produced by MUSHclient v %s - www.mushclient.com -->",
                             MUSHCLIENT_VERSION));
    ar.WriteString (ENDLINE);
    ar.WriteString (CFormat ("<table border=0 cellpadding=5 bgcolor=\"#%02X%02X%02X\">",
                        GetRValue (colour2),
                        GetGValue (colour2),
                        GetBValue (colour2)));
    ar.WriteString (ENDLINE);

    ar.WriteString ("<tr><td>");
    ar.WriteString (ENDLINE);

    CString strFont = "<pre><code>"
                    "<font size=2 face=\"$FONT$FixedSys, Lucida Console, Courier New, Courier\">"
                    "<font color=\"#0\">";

    if (pDoc->m_font_name != "FixedSys")
      {
      CString s = pDoc->m_font_name;
      s += ", ";
      strFont = ::Replace (strFont, "$FONT$", s);
      }
    else
       strFont = ::Replace (strFont, "$FONT$", "");  // font already there

    ar.WriteString (strFont);
    ar.WriteString (ENDLINE);


  // starting and ending on the same line is a special case

    if (m_selend_line == m_selstart_line)
      OutputOneLine (pDoc, ar, pStartLine, startcol, endcol, 
                     defaultbackcolour, lastforecolour, lastbackcolour, bInSpan);
    else
      {

  // do first line

      OutputOneLine (pDoc, ar, pStartLine, startcol, pStartLine->len, 
                    defaultbackcolour, lastforecolour, lastbackcolour, bInSpan);
      ar.WriteString (ENDLINE);

  // do middle lines

      POSITION pos = pDoc->GetLinePosition (m_selstart_line + 1);

      for (i = m_selstart_line + 1; i < m_selend_line && pos; i++)
        {
        CLine * pLine = pDoc->m_LineList.GetNext (pos);

        OutputOneLine (pDoc, ar, pLine, 0, pLine->len, 
                       defaultbackcolour, lastforecolour, lastbackcolour, bInSpan);
        ar.WriteString (ENDLINE);

        } // end of doing middle lines

  // do last line

      OutputOneLine (pDoc, ar, pEndLine, 0, endcol, 
                      defaultbackcolour, lastforecolour, lastbackcolour, bInSpan);
      ar.WriteString (ENDLINE);
    
      }

    if (bInSpan)
       ar.WriteString ("</span>");
    ar.WriteString ("</font></font></code></pre>");
    ar.WriteString (ENDLINE);
    ar.WriteString ("</td></tr></table>");
    ar.WriteString (ENDLINE);

    ar.Close();

    int nLength = f.GetLength ();
    p = (char *) f.Detach ();

    CString strSelection (p, nLength);

    free (p);   // remove memory allocated in CMemFile
    p = NULL;
    putontoclipboard (strSelection, pDoc->m_bUTF_8);

    }   // end of try block

  catch (CException* e)
    {
    if (p)
      free (p);   // remove memory allocated in CMemFile
    e->ReportError();
    e->Delete();
    }

// put the focus back in the send window

  OnKeysActivatecommandview ();

  
}


void CMUSHView::OnDisplayHighlightphrase() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

CHighlightPhraseDlg dlg;

  dlg.m_iOtherForeground = pDoc->m_normalcolour [WHITE]; // their normal white
  dlg.m_iOtherBackground = pDoc->m_normalcolour [BLACK]; // their normal black

CString str;

long startcol,
     endcol;

// return if no valid selection

  CLine * pStartLine,
        * pEndLine;

  pStartLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (m_selstart_line));
  pEndLine = pDoc->m_LineList.GetAt (pDoc->GetLinePosition (m_selend_line));
  
  startcol  = MIN (m_selstart_col, pStartLine->len);
  endcol    = MIN (m_selend_col, pEndLine->len);

  if ((m_selend_line > m_selstart_line || 
              (m_selend_line == m_selstart_line && 
               endcol > startcol)))
    {
    CStyle * pStyle;
    int iCol;
    POSITION foundpos;

    if (pDoc->FindStyle (pStartLine, startcol, iCol, pStyle, foundpos))
      pDoc->GetStyleRGB (pStyle, dlg.m_iOtherForeground, dlg.m_iOtherBackground); // find what colour this style is

    CString strSelection;
    int i;

    GetSelection (strSelection);

    // stop at first linefeed
    i = strSelection.Find ("\n");

    if (i != -1)
      strSelection = strSelection.Left (i - 1);

    if (strSelection.GetLength () > 80)
      strSelection = strSelection.Left (80);

    dlg.m_strText = strSelection;

    } // end of selected text

  memcpy (dlg.m_customtext, pDoc->m_customtext, sizeof (dlg.m_customtext));
  memcpy (dlg.m_customback, pDoc->m_customback, sizeof (dlg.m_customback));
  for (int i = 0; i < MAX_CUSTOM; i++)
     dlg.m_strCustomName [i] = pDoc->m_strCustomColourName [i];
  
  dlg.m_iColour = OTHER_CUSTOM + 1;    // other custom colour

  // assume they want to match whole words
  dlg.m_bWord = true;

  if (dlg.DoModal () != IDOK)
    return;

  CString strText;
 
  if (dlg.m_bWord)
     strText += "\\b";    // word boundary

  strText += ConvertToRegularExpression (dlg.m_strText, false, false);

  if (dlg.m_bWord)
     strText += "\\b";    // word boundary

  t_regexp * regexp = NULL;

  // compile regular expression
  try 
    {
    regexp = regcomp (strText, (dlg.m_bMatchCase ?  0 : PCRE_CASELESS) | (pDoc->m_bUTF_8 ? PCRE_UTF8 : 0) );
    }   // end of try
  catch(CException* e)
    {
    e->Delete ();
    ::TMessageBox ("Cannot compile regular expression");
    return;
    } // end of catch

  CString strTriggerName;

  // invent name for it
  strTriggerName.Format ("*trigger%s", (LPCTSTR) App.GetUniqueString ());

  CTrigger * trigger_item;

  // create new trigger item and insert in trigger map
  pDoc->GetTriggerMap ().SetAt (strTriggerName, trigger_item = new CTrigger);

  trigger_item->nUpdateNumber    = App.GetUniqueNumber ();   // for concurrency checks
  trigger_item->strInternalName  = strTriggerName;    // for deleting one-shot triggers

  trigger_item->trigger          = strText;
  trigger_item->colour           = dlg.m_iColour - 1;
  trigger_item->bKeepEvaluating  = true;      // keep evaluating wanted
  trigger_item->ignore_case      = !dlg.m_bMatchCase;
  
  trigger_item->bRegexp          = true;      // yes, it's a regexp
  trigger_item->bRepeat          = true;      // repeat on same line
  trigger_item->regexp           = regexp;
  trigger_item->iSendTo          = eSendToWorld;
  trigger_item->iSequence        = 90;
  trigger_item->strGroup         = "Highlighted Words";

  trigger_item->iOtherForeground = dlg.m_iOtherForeground;
  trigger_item->iOtherBackground = dlg.m_iOtherBackground;

  pDoc->SortTriggers ();

  // we need to know to save it
  pDoc->SetModifiedFlag (TRUE);
  
} // end of CMUSHView::OnDisplayHighlightphrase

void CMUSHView::OnDisplayMultilinetrigger() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

CMultiLineTriggerDlg dlg;

CString strSelection;

  GetSelection (strSelection);

  strSelection = strSelection.Left (10000); // just make sure not really big
  strSelection.Replace ("\r", "");  // don't want \r in the string

  dlg.m_strTriggerText = ConvertToRegularExpression (strSelection, false, false);
  dlg.m_strTriggerText.Replace ("\\n", "\r\n"); // put linebreaks back
 
  dlg.m_bMatchCase = true;

  if (dlg.DoModal () != IDOK)
    return;

  CString strText =  dlg.m_strTriggerText;

  int iCount = 1;          // the first line is always there
  // count lines in trigger
  const char * p;

  for (p = strText; *p; p++)
    if (*p == '\n')
      iCount++;

  strText.Replace (ENDLINE, "\\n"); // replace linebreaks with \n
  strText += "\\Z";    // subject boundary

  t_regexp * regexp = NULL;

  // compile regular expression
  try 
    {
    regexp = regcomp (strText, (dlg.m_bMatchCase ?  0 : PCRE_CASELESS) | (pDoc->m_bUTF_8 ? PCRE_UTF8 : 0) );
    }   // end of try
  catch(CException* e)
    {
    e->Delete ();
    ::TMessageBox ("Cannot compile regular expression");
    return;
    } // end of catch

  CString strTriggerName;

  // invent name for it
  strTriggerName.Format ("*trigger%s", (LPCTSTR) App.GetUniqueString ());

  CTrigger * trigger_item;

  // create new trigger item and insert in trigger map
  pDoc->GetTriggerMap ().SetAt (strTriggerName, trigger_item = new CTrigger);

  trigger_item->nUpdateNumber    = App.GetUniqueNumber ();   // for concurrency checks
  trigger_item->strInternalName  = strTriggerName;    // for deleting one-shot triggers

  trigger_item->trigger          = strText;
  trigger_item->colour           = SAMECOLOUR;
  trigger_item->ignore_case      = !dlg.m_bMatchCase;
  
  trigger_item->contents         = "%0";      // display all text
  trigger_item->bRegexp          = true;      // yes, it's a regexp
  trigger_item->bMultiLine        = true;      // multiline
  trigger_item->bKeepEvaluating  = true;      // keep evaluating wanted
  trigger_item->regexp           = regexp;
  trigger_item->iSendTo          = eSendToOutput;
  trigger_item->strGroup         = "Multi Line";

  trigger_item->iLinesToMatch = iCount;

  pDoc->SortTriggers ();

  // we need to know to save it
  pDoc->SetModifiedFlag (TRUE);

} // end of CMUSHView::OnDisplayMultilinetrigger

void CMUSHView::OnWindowMaximize() 
{
    GetParentFrame ()->ShowWindow(SW_SHOWMAXIMIZED);
  
}

void CMUSHView::OnWindowRestore() 
{
    GetParentFrame ()->ShowWindow(SW_RESTORE);
  
}

void CMUSHView::OnSysCommand(UINT nID, LPARAM lParam) 
{
  if (nID == SC_KEYMENU && App.m_bDisableKeyboardMenuActivation) {
    /* Discard. */
  } else
    CView::OnSysCommand(nID, lParam);
}


// a mouse-over a window hotspot will come here
void CMUSHView::Send_Mouse_Event_To_Plugin (DISPID iDispatchID,
                                            const string m_sPluginID, 
                                            const string sRoutineName, 
                                            const string HotspotId,
                                            long Flags,
                                            bool dont_modify_flags)
  {

  // only if they have a routine
  if (sRoutineName.empty ())
    return;

CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

// also Flags might be (from caller):

// 0x10  - LH mouse
// 0x20  - RH mouse
// 0x40  - double-click
// 0x80  - mouse-over not first time 
// 0x100 - scroll wheel backwards

if (!dont_modify_flags)
  {
  if (GetKeyState (VK_SHIFT) < 0)     // shift+click
     Flags |= 1;

  if (GetKeyState (VK_CONTROL) < 0)   // ctrl+click
     Flags |= 2;

  if (GetKeyState (VK_MENU) < 0)      // alt+click
     Flags |= 4;
  }  // can modify flags


if (m_sPluginID.empty ())
  {
  if (iDispatchID == DISPID_UNKNOWN)
    {
    pDoc->Trace ("Mouse event (%s): Function %s not found.", HotspotId.c_str (), sRoutineName.c_str ());
    return;                       
    }


  pDoc->ExecuteHotspotScript (iDispatchID,  // dispatch ID, will be set to DISPID_UNKNOWN on an error
                              sRoutineName.c_str (),      // what name was in the hotspot callback
                              Flags,              // flags: ctrl, shift, whatever
                              HotspotId.c_str ()       // which hotspot
                              );

  return;


  } // end of no plugin

CPlugin * pPlugin = pDoc->GetPlugin (m_sPluginID.c_str ()); 

  if (!pPlugin) 
    {
    pDoc->Trace ("Mouse event (%s): No plugin ID: %s", HotspotId.c_str (), m_sPluginID.c_str ());
    return;                       
    }

  if (!pPlugin->m_bEnabled)
    {
    pDoc->Trace ("Mouse event (%s): Plugin: %s disabled.", HotspotId.c_str (), m_sPluginID.c_str ());
    return;                       
    }

DISPID iDispid = pPlugin->m_ScriptEngine->GetDispid (sRoutineName.c_str ());

  if (iDispid == DISPID_UNKNOWN)
    {
    pDoc->Trace ("Mouse event (%s): Function %s not found.", HotspotId.c_str (), sRoutineName.c_str ());
    return;                       
    }

  CPlugin * pSavedPlugin = pDoc->m_CurrentPlugin;

  pDoc->m_CurrentPlugin = pPlugin;   
   

  pPlugin->ExecutePluginScript (sRoutineName.c_str (), 
                          iDispid, 
                          Flags,              // keyboard flags
                          HotspotId.c_str ()  // which hotspot
                          );
  pDoc->m_CurrentPlugin = pSavedPlugin;


  }  // end of CMUSHView::Send_Mouse_Event_To_Plugin



CMiniWindow * CMUSHView::Mouse_Over_Miniwindow (CMUSHclientDoc* pDoc, 
                                                CPoint point, 
                                                string & sHotspotId, 
                                                CHotspot * &pHotspot,
                                                string & sMiniwindowId
                                                )
  {
  pHotspot = NULL;

  map<string, CMiniWindow *>::reverse_iterator mwit;

   // mini windows - check backwards so one on top is found first
  for (mwit = pDoc->m_MiniWindows.rbegin (); 
       mwit != pDoc->m_MiniWindows.rend ();
       mwit++)  // NB you still go "up" to go backwards
         {
         CMiniWindow * mw = mwit->second;
         // not if not being shown
         if (!mw->GetShow ())
           continue;

         // underneath ones can't be clicked on - or if set to ignore the mouse
         if (mw->GetFlags () & (MINIWINDOW_DRAW_UNDERNEATH | MINIWINDOW_IGNORE_MOUSE))
           continue;

         if (!mw->m_rect.PtInRect(point))
           continue;  // not in window

         CPoint mw_relative_mouse (point);
         mw_relative_mouse = mw_relative_mouse - mw->m_rect.TopLeft ();
         for (HotspotMapIterator hit = mw->m_Hotspots.begin (); 
              hit != mw->m_Hotspots.end ();
              hit++)
                {
                pHotspot = hit->second;

                if (!pHotspot->m_rect.PtInRect(mw_relative_mouse))
                  continue;  // not in hotspot

                sHotspotId = hit->first;
                sMiniwindowId = mwit->first;
                return mw;
                } // for each hotspot
         

         pHotspot = NULL;
         sMiniwindowId = mwit->first;
         return mw;  
         }  // end for each window



  sMiniwindowId.erase ();
  return NULL;
  } // end of  CMUSHView::Mouse_Over_Miniwindow


bool CMUSHView::Mouse_Move_MiniWindow (CMUSHclientDoc* pDoc, CPoint point)
  {
  // see if we moused over one of our miniwindows

  CMiniWindow * mw = NULL;
  CHotspot * pHotspot = NULL;
  string sHotspotId;
  string sMiniWindowId;

  mw = Mouse_Over_Miniwindow (pDoc, point, sHotspotId, pHotspot, sMiniWindowId);

  if (mw)
    {
    mw->m_last_mouseposition = CPoint (point.x - mw->m_rect.left, point.y - mw->m_rect.top);
    mw->m_last_mouse_update++;
    }


  // report mouse movements: version 4.45
  pDoc->m_lastMousePosition = point;

  CPlugin * pSavedPlugin = pDoc->m_CurrentPlugin;
  pDoc->m_CurrentPlugin = NULL;

  // tell each plugin about the mouse movement
  for (POSITION pos = pDoc->m_PluginList.GetHeadPosition(); pos; )
    {
    CPlugin * pPlugin = pDoc->m_PluginList.GetNext (pos);

    if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
      continue;

    pPlugin->ExecutePluginScript (ON_PLUGIN_MOUSE_MOVED, 
                                  pPlugin->m_dispid_plugin_mouse_moved, point.x, point.y, sMiniWindowId.c_str ());
    }   // end of doing each plugin

  pDoc->m_CurrentPlugin = pSavedPlugin;

  // drag-and-drop stuff

  // see if mouse was already down and captured

  if (!m_sPreviousMiniWindow.empty ())
    {

    // here if *something* is up
    MiniWindowMapIterator it = pDoc->m_MiniWindows.find (m_sPreviousMiniWindow);

    // find the invoking miniwindow
    if (it != pDoc->m_MiniWindows.end ())
      {

      CMiniWindow * prev_mw = it->second;

      prev_mw->m_client_mouseposition = point;

      // see if activity was mouse-down
      if (!prev_mw->m_sMouseDownHotspot.empty ())   // mouse was clicked
        {
        // lookup that HotspotId
        HotspotMapIterator it = prev_mw->m_Hotspots.find (prev_mw->m_sMouseDownHotspot);

        // call MoveCallback for that hotspot, if it exists
        if (it != prev_mw->m_Hotspots.end ())
          Send_Mouse_Event_To_Plugin (it->second->m_dispid_MoveCallback,
                                      prev_mw->m_sCallbackPlugin,
                                      it->second->m_sMoveCallback, 
                                      prev_mw->m_sMouseDownHotspot);
        return true;  // that's all

        }   // we had previous hotspot


      } // previous window still exists


    } // end we have been up to something in a miniwindow


  // end drag-and-drop


  // next see if they have moved away from our previous miniwindow, if any
  if (sMiniWindowId != m_sPreviousMiniWindow  && !m_sPreviousMiniWindow.empty ())
    {

    MiniWindowMapIterator it = pDoc->m_MiniWindows.find (m_sPreviousMiniWindow);
  
    if (it != pDoc->m_MiniWindows.end ())
      {

      CMiniWindow * old_mw = it->second;

      old_mw->m_last_mouseposition = CPoint (point.x - old_mw->m_rect.left, point.y - old_mw->m_rect.top);
      old_mw->m_last_mouse_update++;

      // cancel previous move-over hotspot
      if (!old_mw->m_sMouseOverHotspot.empty ())   // HotspotId was used
        {
        // lookup that HotspotId
        HotspotMapIterator it = old_mw->m_Hotspots.find (old_mw->m_sMouseOverHotspot);

        // call CancelMouseOver for that hotspot, if it exists
        if (it != old_mw->m_Hotspots.end ())
          {
          RemoveToolTip ();
          Send_Mouse_Event_To_Plugin (it->second->m_dispid_CancelMouseOver,
                                      old_mw->m_sCallbackPlugin,
                                      it->second->m_sCancelMouseOver, 
                                      old_mw->m_sMouseOverHotspot);
          }
        old_mw->m_sMouseOverHotspot.erase ();  // no mouse-over right now
        m_sPreviousMiniWindow.erase ();  // no longer have a previous mouse-over
        }   // we had previous hotspot


      } // previous window still exists

    } // moved to different window


  // here if currently over a miniwindow
  if (mw && GetCapture () != this)
    {


    // cancel previous move-over hotspot (in this miniwindow)
    if ((pHotspot == NULL ||                      // not on any hotspot
         mw->m_sMouseOverHotspot != sHotspotId)   // or on different hotspot         
         && !mw->m_sMouseOverHotspot.empty ())    // and we previously were over one in this miniwindow
      {
      HotspotMapIterator it = mw->m_Hotspots.find (mw->m_sMouseOverHotspot);

      if (it != mw->m_Hotspots.end ())
        {
        RemoveToolTip ();
        Send_Mouse_Event_To_Plugin (it->second->m_dispid_CancelMouseOver,
                                    mw->m_sCallbackPlugin,
                                    it->second->m_sCancelMouseOver, 
                                    mw->m_sMouseOverHotspot);
        }

      mw->m_sMouseOverHotspot.erase ();  // no mouse-over right now
      } // previous one which isn't this one, or we are no longer on one

    // now, are we now over a hotspot?
    if (pHotspot)
      {

      // if they have moused-down, ignore mouse moves
      if (!mw->m_sMouseDownHotspot.empty ())
        return true;
  
      m_sPreviousMiniWindow = sMiniWindowId;   // remember in case they move outside window

      // if different hotspot from before
      if (sHotspotId != mw->m_sMouseOverHotspot)
        {
        // this is our new one
        mw->m_sMouseOverHotspot = sHotspotId;
        Send_Mouse_Event_To_Plugin (pHotspot->m_dispid_MouseOver,
                                    mw->m_sCallbackPlugin, 
                                    pHotspot->m_sMouseOver, 
                                    sHotspotId);
        // activate tooltip if possible
        if (::IsWindow(m_ToolTip.m_hWnd))
          {
          // now activate for *this* hotspot
          m_ToolTip.Activate(TRUE);
          m_nLastToolTipLine = 0;
          m_nLastToolTipColumn = 0;
          }   // end of having tooltip window


        // capture mouse movements out of the miniwindow (version 4.46)
        // see: http://www.gammon.com.au/forum/?id=9980

        if (!m_mousedover && !bWin95 && !pHotspot->m_sCancelMouseOver.empty () )
          {
          TRACKMOUSEEVENT tme;
          ZeroMemory (&tme, sizeof tme);
          tme.cbSize = sizeof(TRACKMOUSEEVENT);
          tme.dwFlags = TME_LEAVE;
          tme.hwndTrack = m_hWnd;
          _TrackMouseEvent(&tme);

          m_mousedover = true;
         }

        }
      else
        {  // same hotspot (new in version 4.45)
        // see lengthy forum discussion: http://www.gammon.com.au/forum/?id=9942
        if (pHotspot->m_Flags & 1)
          Send_Mouse_Event_To_Plugin (pHotspot->m_dispid_MouseOver,
                                      mw->m_sCallbackPlugin, 
                                      pHotspot->m_sMouseOver, 
                                      sHotspotId, 0x80);
        }
      }

    return true;    // we are over mini-window - don't check for underlying text
    }

  return false;     // keep going

  } // end of CMUSHView::Mouse_Move_MiniWindow 

bool CMUSHView::Mouse_Down_MiniWindow (CMUSHclientDoc* pDoc, CPoint point, long flags)
  {
  // see if we clicked over one of our miniwindows

  CMiniWindow * mw = NULL;
  CHotspot * pHotspot = NULL;
  string sHotspotId;
  string sMiniWindowId;

  mw = Mouse_Over_Miniwindow (pDoc, point, sHotspotId, pHotspot, sMiniWindowId);

  // ANY mouse-down cancels a previous mouse-over
  if (!m_sPreviousMiniWindow.empty ())
    {

    MiniWindowMapIterator it = pDoc->m_MiniWindows.find (m_sPreviousMiniWindow);
  
    if (it != pDoc->m_MiniWindows.end ())
      {

      CMiniWindow * old_mw = it->second;

      // cancel previous move-over hotspot
      if (!old_mw->m_sMouseOverHotspot.empty ())   // HotspotId was used
        {
        // lookup that HotspotId
        HotspotMapIterator it = old_mw->m_Hotspots.find (old_mw->m_sMouseOverHotspot);

        // call CancelMouseOver for that hotspot, if it exists
        if (it != old_mw->m_Hotspots.end ())
          Send_Mouse_Event_To_Plugin (it->second->m_dispid_CancelMouseOver, 
                                      old_mw->m_sCallbackPlugin,
                                      it->second->m_sCancelMouseOver, 
                                      old_mw->m_sMouseOverHotspot);    
        old_mw->m_sMouseOverHotspot.erase ();  // no mouse-over right now
        m_sPreviousMiniWindow.erase ();  // no longer have a previous mouse-over
        }   // we had previous hotspot

      } // previous window still exists

    } // moved to different window

  // here if currently over a miniwindow
  if (mw)
    {

    mw->m_last_mouseposition = CPoint (point.x - mw->m_rect.left, point.y - mw->m_rect.top);
    mw->m_last_mouse_update++;

    // now, are we now over a hotspot?
    if (pHotspot)
      {
      mw->m_sMouseDownHotspot = sHotspotId;  // remember this is mousedown hotspot

      Send_Mouse_Event_To_Plugin (pHotspot->m_dispid_MouseDown, 
                                  mw->m_sCallbackPlugin, 
                                  pHotspot->m_sMouseDown, 
                                  sHotspotId,
                                  flags);   // LH / RH mouse?
      }

    m_sPreviousMiniWindow = sMiniWindowId;   // remember in case they move outside window
    // have to capture anyway or we get mouse-up without previous mouse-down
    SetCapture();       // Capture the mouse until button up.
    return true;    // we are over mini-window - don't check for underlying text
    }

  return false;     // keep going

  } // end of CMUSHView::Mouse_Down_MiniWindow 

bool CMUSHView::Mouse_Up_MiniWindow (CMUSHclientDoc* pDoc, CPoint point, long flags)
  {
  // see if we clicked over one of our miniwindows

  CMiniWindow * mw = NULL;
  CHotspot * pHotspot = NULL;
  string sHotspotId;
  string sMiniWindowId;
  bool bPreviouslyInMiniwindow = false;


  // drag-and-drop stuff

  // see if mouse was already down and captured

  if (!m_sPreviousMiniWindow.empty ())
    {

    // here if *something* is up
    MiniWindowMapIterator it = pDoc->m_MiniWindows.find (m_sPreviousMiniWindow);

    // find the invoking miniwindow
    if (it != pDoc->m_MiniWindows.end ())
      {

      CMiniWindow * prev_mw = it->second;

      prev_mw->m_client_mouseposition = point;

      // see if activity was mouse-down
      if (!prev_mw->m_sMouseDownHotspot.empty ())   // mouse was clicked
        {
        // lookup that HotspotId
        HotspotMapIterator it = prev_mw->m_Hotspots.find (prev_mw->m_sMouseDownHotspot);

        // call ReleaseCallback for that hotspot, if it exists
        if (it != prev_mw->m_Hotspots.end ())
          Send_Mouse_Event_To_Plugin (it->second->m_dispid_ReleaseCallback, 
                                      prev_mw->m_sCallbackPlugin,
                                      it->second->m_sReleaseCallback, 
                                      prev_mw->m_sMouseDownHotspot);

        }   // we had previous hotspot


      } // previous window still exists


    } // end we have been up to something in a miniwindow


  // end drag-and-drop



  mw = Mouse_Over_Miniwindow (pDoc, point, sHotspotId, pHotspot, sMiniWindowId);

  // A mouse-up not in this miniwindow cancels a previous mouse-down
  if (sMiniWindowId != m_sPreviousMiniWindow && !m_sPreviousMiniWindow.empty ())
    {
    ReleaseCapture();   // Release the mouse capture established at
                        // the beginning of the mouse click.

    MiniWindowMapIterator it = pDoc->m_MiniWindows.find (m_sPreviousMiniWindow);
  
    if (it != pDoc->m_MiniWindows.end ())
      {

      CMiniWindow * old_mw = it->second;

      old_mw->m_last_mouseposition = CPoint (point.x - old_mw->m_rect.left, point.y - old_mw->m_rect.top);
      old_mw->m_last_mouse_update++;

      // cancel previous move-down hotspot
      if (!old_mw->m_sMouseDownHotspot.empty ())   // HotspotId was used
        {
        // lookup that HotspotId
        HotspotMapIterator it = old_mw->m_Hotspots.find (old_mw->m_sMouseDownHotspot);

        // call CancelMouseDown for that hotspot, if it exists
        if (it != old_mw->m_Hotspots.end ())
          Send_Mouse_Event_To_Plugin (it->second->m_dispid_CancelMouseDown, 
                                      old_mw->m_sCallbackPlugin,
                                      it->second->m_sCancelMouseDown, 
                                      old_mw->m_sMouseDownHotspot);
        old_mw->m_sMouseDownHotspot.erase ();  // no mouse-down right now
        }   // we had previous hotspot

      } // previous window still exists

    m_sPreviousMiniWindow.erase ();  // no longer have a previous window

    bPreviouslyInMiniwindow = true;

    } // released mouse in different window

  // here if currently over a miniwindow
  if (mw)
    {

    mw->m_last_mouseposition = CPoint (point.x - mw->m_rect.left, point.y - mw->m_rect.top);
    mw->m_last_mouse_update++;
    ReleaseCapture();   // Release the mouse capture established at
                        // the beginning of the mouse click.

    // if mouse-up outside the current hotspot just cancel previous one
    // cancel previous mouse-down hotspot (in this miniwindow)
    if ((pHotspot == NULL ||                      // not on any hotspot
         mw->m_sMouseDownHotspot != sHotspotId)   // or on different hotspot         
         && !mw->m_sMouseDownHotspot.empty ())    // and we previously were down in this miniwindow
      {
      HotspotMapIterator it = mw->m_Hotspots.find (mw->m_sMouseDownHotspot);

      if (it != mw->m_Hotspots.end ())
        Send_Mouse_Event_To_Plugin (it->second->m_dispid_CancelMouseDown, 
                                    mw->m_sCallbackPlugin,
                                    it->second->m_sCancelMouseDown, 
                                    mw->m_sMouseDownHotspot);

      mw->m_sMouseDownHotspot.erase ();  // no mouse-down right now
      } // previous one which isn't this one, or we are no longer on one

    // now, did we release mouse over the hotspot it went down in?
    if (pHotspot && mw->m_sMouseDownHotspot == sHotspotId)
      {
      Send_Mouse_Event_To_Plugin (pHotspot->m_dispid_MouseUp, 
                                  mw->m_sCallbackPlugin, 
                                  pHotspot->m_sMouseUp, 
                                  sHotspotId, 
                                  flags);  // LH / RH mouse?

      mw->m_sMouseDownHotspot.erase ();  // no mouse-down right now
      }

    m_sPreviousMiniWindow.erase ();  // no longer have a previous mouse-over

    return true;    // we are over mini-window - don't check for underlying text
    }


  return bPreviouslyInMiniwindow;     // keep going unless we started in another miniwindow
  } // end of CMUSHView::Mouse_Up_MiniWindow 


// version 4.41 onwards - you can have title \t body (title and body separated by a tab)
bool CMUSHView::Mouse_Tooltip_MiniWindow (CMUSHclientDoc* pDoc, CPoint point, NMHDR * pNMHDR)
  {
  // see if we clicked over one of our miniwindows

  CMiniWindow * mw = NULL;
  CHotspot * pHotspot = NULL;
  string sHotspotId;
  string sMiniWindowId;

  mw = Mouse_Over_Miniwindow (pDoc, point, sHotspotId, pHotspot, sMiniWindowId);

  if (!mw)
    return false;

  TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;

  static char buf [1000];

  if (pHotspot && !pHotspot->m_sTooltipText.empty ())
    {
    string s = pHotspot->m_sTooltipText;
    string delim = "\t";

    // find delimiter  
    string::size_type i (s.find (delim));

    // split into before and after delimiter
    string body (s.substr (0, i));

    // if no delimiter, no title
    if (i == string::npos)
      m_ToolTip.SendMessage (TTM_SETTITLE, TTI_INFO, (LPARAM) "" );
    else
      {
      string title = body;
      body = s.substr (i + delim.size (), string::npos); 
      m_ToolTip.SendMessage (TTM_SETTITLE, TTI_INFO, (LPARAM) title.c_str ());
      }
    
    // allow more text
    strcpy (buf, body.substr (0, (sizeof buf) - 1).c_str ());
    pTTT->lpszText = buf;

    }
  else
    {
    buf [0] = 0;    // cursor is not over a hotspot
    pTTT->lpszText = buf;
    }



  return true;

  }  // end of CMUSHView::Mouse_Tooltip_MiniWindow


// we will work out rectangles for each miniwindow, for this drawing moment
void CMUSHView::Calculate_MiniWindow_Rectangles (CMUSHclientDoc* pDoc, const bool bUnderneath)
  {

  CChildFrame * pFrame = m_owner_frame;

  RECT clientrect, 
       ownerrect;

  pFrame->GetClientRect (&ownerrect);
  GetClientRect (&clientrect);

  MiniWindowMapIterator mwit;

  // record here how far out each corner extends
  CPoint pt_topleft (0, 0),
         pt_topright (clientrect.right, 0),
         pt_bottomleft (0,clientrect.bottom),
         pt_bottomright (clientrect.right, clientrect.bottom);
     

  // total amount taken by the centered items along each wall
  long  top_widths = 0,
        right_widths = 0,
        bottom_widths = 0,
        left_widths = 0;

  // remember each one in case there are too many
  list<CMiniWindow *> top_ones,
        right_ones,
        bottom_ones,
        left_ones;

  // do the corners, and fixed positions, and remember how much the centered ones wanted
  for (mwit = pDoc->m_MiniWindows.begin (); 
       mwit != pDoc->m_MiniWindows.end ();
       mwit++)
   {
   CMiniWindow * mw = mwit->second;

   mw->m_bTemporarilyHide = false;


   // not if not to be shown
   if (!mw->GetShow ())
     continue;

   // not if wrong time
   if (((mw->GetFlags () & MINIWINDOW_DRAW_UNDERNEATH) != 0) != bUnderneath)
     continue;

   long iWidth = mw->GetWidth (),
         iHeight = mw->GetHeight ();

   // if absolute location wanted, just take that and press on
   if (mw->GetFlags () & MINIWINDOW_ABSOLUTE_LOCATION)
      {
      CPoint loc          = mw->GetLocation ();
      mw->m_rect.left     = loc.x;
      mw->m_rect.top      = loc.y;
      mw->m_rect.right    = mw->m_rect.left + iWidth;
      mw->m_rect.bottom   = mw->m_rect.top + iHeight;
      continue;
      }

    switch (mw->GetPosition ())
      {
      case 0 :  // stretch to output window
         mw->m_rect = CRect (0, 0, clientrect.right, clientrect.bottom);
         continue;

      case 1:  // stretch with aspect ratio
        {
        double ratio = ((double) iWidth) / ((double) iHeight);
        mw->m_rect = CRect (0, 0, clientrect.bottom * ratio, clientrect.bottom);
        continue;
        }

      case 2 :  // stretch to frame
         mw->m_rect = CRect (0, 0, ownerrect.right, ownerrect.bottom);
         continue;

      case 3:    // stretch to frame with aspect ratio
        {
        double ratio = ((double) iWidth) / ((double) iHeight);
        mw->m_rect = CRect (0, 0, ownerrect.bottom * ratio, ownerrect.bottom);
        continue;
        }

      case 4: // top left
         mw->m_rect = CRect (0, 0, iWidth, iHeight);
         pt_topleft.x = max (pt_topleft.x, mw->m_rect.right);     // shrinks available
         pt_topleft.y = max (pt_topleft.y, mw->m_rect.bottom);
         break;

      case 5 :  // center left-right at top
        top_widths += iWidth; 
        top_ones.push_back (mw);
        break;

      case 6 : // top right
         mw->m_rect = CRect (clientrect.right - iWidth, 0, iWidth, iHeight);
         pt_topright.x = min (pt_topright.x, mw->m_rect.left);     // shrinks available
         pt_topright.y = max (pt_topright.y, mw->m_rect.bottom);
         break;

      case 7 :  // on right, center top-bottom
         right_widths += iHeight;
         right_ones.push_back (mw);
         break;

      case 8 :  // on right at bottom
         mw->m_rect = CRect (clientrect.right - iWidth, clientrect.bottom - iHeight, iWidth, iHeight);
         pt_bottomright.x = min (pt_bottomright.x, mw->m_rect.left);     // shrinks available
         pt_bottomright.y = min (pt_bottomright.y, mw->m_rect.top);
         break;

      case 9 :  // center left-right at bottom
          bottom_widths += iWidth;
          bottom_ones.push_back (mw);
          break;

      case 10 :  // on left, at bottom
         mw->m_rect = CRect (0, clientrect.bottom - iHeight, iWidth, iHeight);
         pt_bottomleft.x = max (pt_bottomleft.x, mw->m_rect.right);     // shrinks available
         pt_bottomleft.y = min (pt_bottomleft.y, mw->m_rect.top);
         break;

      case 11 :  // on left, center top-bottom
          left_widths += iHeight;
          left_ones.push_back (mw);
          break;

      case 12:  // center all
         mw->m_rect = CRect ((clientrect.right - iWidth) / 2, (clientrect.bottom - iHeight) / 2, iWidth, iHeight);
         continue;


      case 13:
          continue;  // tile - ignore

      } // end switch


   }  // end for each window


  // see the size of the gaps on each side
  long top_room = pt_topright.x - pt_topleft.x;
  long right_room = pt_bottomright.y - pt_topright.y;
  long bottom_room = pt_bottomright.x - pt_bottomleft.x;
  long left_room = pt_bottomleft.y - pt_topleft.y;


  // throw away ones that won't fit (may be left with nothing)

  // top
  while (!top_ones.empty () && (top_widths > top_room))
    {
    CMiniWindow * mw = top_ones.back ();
    top_ones.pop_back ();
    top_widths -= mw->GetWidth ();
    mw->m_bTemporarilyHide = true;
    }

  // right
  while (!right_ones.empty () && (right_widths > right_room))
    {
    CMiniWindow * mw = right_ones.back ();
    right_ones.pop_back ();
    right_widths -= mw->GetHeight ();
    mw->m_bTemporarilyHide = true;
    }

  // bottom
  while (!bottom_ones.empty () && (bottom_widths > bottom_room))
    {
    CMiniWindow * mw = bottom_ones.back ();
    bottom_ones.pop_back ();
    bottom_widths -= mw->GetWidth ();
    mw->m_bTemporarilyHide = true;
    }

  // left
  while (!left_ones.empty () && (left_widths > left_room))
    {
    CMiniWindow * mw = left_ones.back ();
    left_ones.pop_back ();
    left_widths -= mw->GetHeight ();
    mw->m_bTemporarilyHide = true;
    }


  // allocate positions
  // in each case the number of gaps will be the number of items + 1
  // (eg. for a single centered item, there is a gap at each side)
  list<CMiniWindow *>::iterator lit;
  long gap;
  long start;


  // along top wall
  gap = (top_room - top_widths) / (top_ones.size () + 1);
  start = pt_topleft.x + gap;

  for (lit = top_ones.begin (); lit != top_ones.end (); lit++)
    {
    CMiniWindow * mw = *lit;
    mw->m_rect = CRect (start, 0, mw->GetWidth (), mw->GetHeight ());
    start += mw->GetWidth () + gap;
    }

  // along right wall
  gap = (right_room - right_widths) / (right_ones.size () + 1);
  start = pt_topright.y + gap;

  for (lit = right_ones.begin (); lit != right_ones.end (); lit++)
    {
    CMiniWindow * mw = *lit;
    mw->m_rect = CRect (clientrect.right - mw->GetWidth (), start, mw->GetWidth (), mw->GetHeight ());
    start += mw->GetHeight () + gap;
    }

  // along bottom wall
  gap = (bottom_room - bottom_widths) / (bottom_ones.size () + 1);
  start = pt_bottomleft.x + gap;

  for (lit = bottom_ones.begin (); lit != bottom_ones.end (); lit++)
    {
    CMiniWindow * mw = *lit;
    mw->m_rect = CRect (start, clientrect.bottom - mw->GetHeight (), mw->GetWidth (), mw->GetHeight ());
    start += mw->GetWidth () + gap;
    }

  // along left wall
  gap = (left_room - left_widths) / (left_ones.size () + 1);
  start = pt_topleft.y + gap;

  for (lit = left_ones.begin (); lit != left_ones.end (); lit++)
    {
    CMiniWindow * mw = *lit;
    mw->m_rect = CRect (0, start, mw->GetWidth (), mw->GetHeight ());
    start += mw->GetHeight () + gap;
    }

  } // end of CMUSHView::Calculate_MiniWindow_Rectangles


long CMUSHView::GetOutputWindowHeight () 
  {

  RECT r = GetTextRectangle ();
 
  return r.bottom - r.top;

  }  // end of CMUSHView::GetOutputWindowHeight 

long CMUSHView::GetOutputWindowWidth () 
  {

  RECT r = GetTextRectangle ();
 
  return r.right - r.left;

  }  // end of CMUSHView::GetOutputWindowHeight 


// returns the rectangle we can draw text in
void CMUSHView::GetTextRect (RECT * rect)
  {
  *rect = GetTextRectangle ();
  } // end of CMUSHView::GetTextRect

// returns the text rectangle, if any, normalized
RECT CMUSHView::GetTextRectangle (const bool bIncludeBorder)
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

  CRect TextRectangle (pDoc->m_TextRectangle);

RECT r;

  GetClientRect (&r);

  if (pDoc->m_TextRectangle != CRect (0, 0, 0, 0))
    {
    // they can use -n on right/bottom to indicate offset from side

    if (TextRectangle.right <= 0)
      {
      TextRectangle.right += r.right;
      // ensure not negative
      TextRectangle.right = max (TextRectangle.right, TextRectangle.left + 20);
      }


    if (TextRectangle.bottom <= 0)
      {
      TextRectangle.bottom += r.bottom;
      // ensure not negative
      TextRectangle.bottom = max (TextRectangle.bottom, TextRectangle.top + 20);
      }

    if (bIncludeBorder)
      TextRectangle.InflateRect (pDoc->m_TextRectangleBorderOffset, pDoc->m_TextRectangleBorderOffset);

    return TextRectangle;
    }

  return r;

  } // end of CMUSHView::GetTextRectangle


bool CMUSHView::HaveTextRectangle ()
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

  return pDoc->m_TextRectangle != CRect (0, 0, 0, 0);

  } // end of  CMUSHView::HaveTextRectangle

void CMUSHView::SelectionChanged (void)
  {

  // no selection? do nothing
  if (!(m_selend_line > m_selstart_line || 
              (m_selend_line == m_selstart_line && 
               m_selend_col > m_selstart_col)))
    return;

  // something selected? cancel command window selection
  m_bottomview->CancelSelection ();
 
  } // end of CMUSHView::::SelectionChanged


LRESULT CMUSHView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
  switch (message)
   {
    case WM_MOUSELEAVE:
    {
      CMUSHclientDoc* pDoc = GetDocument();
      ASSERT_VALID(pDoc);
      CPoint point (-1, -1);

      m_mousedover = false;

      Mouse_Move_MiniWindow (pDoc, point);

      return 0;
    }  // end of WM_MOUSELEAVE

    default: 
      return CView::WindowProc(message, wParam, lParam);

   }  // end of switch
  
 
  // cannot get here

}


bool CMUSHView::Mouse_Wheel_MiniWindow (CMUSHclientDoc* pDoc, CPoint point, long delta)
  {
  // see if we moused over one of our miniwindows

  CMiniWindow * mw = NULL;
  CHotspot * pHotspot = NULL;
  string sHotspotId;
  string sMiniWindowId;

  ScreenToClient(&point);

  mw = Mouse_Over_Miniwindow (pDoc, point, sHotspotId, pHotspot, sMiniWindowId);

  if (!mw)
    return false;

  // now, are we now over a hotspot?
  if (pHotspot)
    Send_Mouse_Event_To_Plugin (pHotspot->m_dispid_ScrollwheelCallback,
                                mw->m_sCallbackPlugin, 
                                pHotspot->m_sScrollwheelCallback, 
                                sHotspotId,
                                delta);

  return true;    // we are over mini-window - don't scroll underlying text

  } // end of CMUSHView::Mouse_Wheel_MiniWindow 

