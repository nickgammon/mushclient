// TextView.cpp : implementation file
//

/*
Copyright (C) 2000 Nick Gammon.

*/

#include "stdafx.h"
#include "MUSHclient.h"
#include "TextDocument.h"
#include "TextView.h"
#include "childfrm.h"
#include "sendvw.h"
#include "mainfrm.h"
#include "doc.h"

#include "dialogs\GoToLineDlg.h"
#include "dialogs\LuaGsubDlg.h"

#pragma warning( disable : 4100)  // unreferenced formal parameter

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C"
  {
  LUALIB_API int luaopen_rex(lua_State *L);
  LUALIB_API int luaopen_bits(lua_State *L);
  LUALIB_API int luaopen_compress(lua_State *L);
  LUALIB_API int luaopen_bc(lua_State *L);
  LUALIB_API int luaopen_lsqlite3(lua_State *L);
  LUALIB_API int luaopen_lpeg (lua_State *L);
  }

LUALIB_API int luaopen_progress_dialog(lua_State *L);

/////////////////////////////////////////////////////////////////////////////
// CTextView

IMPLEMENT_DYNCREATE(CTextView, CEditView)

CTextView::CTextView()
{
  m_font = NULL;
  m_bInsertMode = true;
  m_backbr = NULL;
}

CTextView::~CTextView()
{
delete m_font;
delete m_backbr;
}


BEGIN_MESSAGE_MAP(CTextView, CEditView)
	//{{AFX_MSG_MAP(CTextView)
	ON_COMMAND(ID_EDIT_GOTO, OnEditGoto)
	ON_COMMAND(ID_KEYS_ESCAPE, OnEscape)
	ON_COMMAND(ID_VIEW_MAXIMIZE, OnViewMaximize)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MAXIMIZE, OnUpdateViewMaximize)
	ON_COMMAND(ID_CONVERT_UNIXTODOS, OnConvertUnixtodos)
	ON_COMMAND(ID_CONVERT_DOSTOUNIX, OnConvertDostounix)
	ON_COMMAND(ID_CONVERT_MACTODOS, OnConvertMactodos)
	ON_COMMAND(ID_CONVERT_DOSTOMAC, OnConvertDostomac)
	ON_COMMAND(ID_CONVERT_REMOVEENDOFLINES, OnConvertRemoveendoflines)
	ON_COMMAND(ID_CONVERT_QUOTELINES, OnConvertQuotelines)
	ON_COMMAND(ID_EDIT_INSERTDATETIME, OnEditInsertdatetime)
	ON_COMMAND(ID_EDIT_WORDCOUNT, OnEditWordcount)
	ON_COMMAND(ID_CONVERT_UPPERCASE, OnConvertUppercase)
	ON_COMMAND(ID_CONVERT_LOWERCASE, OnConvertLowercase)
	ON_COMMAND(ID_CONVERT_REMOVEEXTRABLANKS, OnConvertRemoveextrablanks)
	ON_COMMAND(ID_EDIT_SPELLCHECK, OnEditSpellcheck)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SPELLCHECK, OnUpdateEditSpellcheck)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR_REFLECT()
	ON_COMMAND(ID_EDIT_GOTOMATCHINGBRACE, OnEditGotomatchingbrace)
	ON_COMMAND(ID_EDIT_SELECTTOMATCHINGBRACE, OnEditSelecttomatchingbrace)
	ON_COMMAND(ID_EDIT_SENDTOWORLD, OnEditSendtoworld)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SENDTOWORLD, OnUpdateEditSendtoworld)
	ON_UPDATE_COMMAND_UI(ID_STATUSLINE_TIME, OnUpdateStatuslineTime)
	ON_UPDATE_COMMAND_UI(ID_STATUSLINE_LINES, OnUpdateStatuslineLines)
	ON_UPDATE_COMMAND_UI(ID_STATUSLINE_FREEZE, OnUpdateStatuslineFreeze)
	ON_COMMAND(ID_EDIT_REFRESHRECALLEDDATA, OnEditRefreshrecalleddata)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REFRESHRECALLEDDATA, OnUpdateEditRefreshrecalleddata)
	ON_COMMAND(ID_EDIT_SENDTOCOMMANDWINDOW, OnEditSendtocommandwindow)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SENDTOCOMMANDWINDOW, OnUpdateEditSendtocommandwindow)
	ON_COMMAND(ID_EDIT_FLIPTONOTEPAD, OnEditFliptonotepad)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FLIPTONOTEPAD, OnUpdateEditFliptonotepad)
	ON_UPDATE_COMMAND_UI(ID_STATUSLINE_LOG, OnUpdateStatuslineLog)
	ON_COMMAND(ID_EDIT_INSERT, OnEditInsert)
	ON_WM_CHAR()
	ON_COMMAND(ID_EDIT_ASCIIART, OnEditAsciiart)
	ON_COMMAND(ID_CONVERT_QUOTEFORUMCODES, OnConvertQuoteforumcodes)
	ON_COMMAND(ID_CONVERT_BASE64ENCODE, OnConvertBase64encode)
	ON_COMMAND(ID_CONVERT_BASE64DECODE, OnConvertBase64decode)
	ON_COMMAND(ID_CONVERT_CONVERTHTMLSPECIAL, OnConvertConverthtmlspecial)
	ON_COMMAND(ID_UNCONVERT_CONVERTHTMLSPECIAL, OnUnconvertConverthtmlspecial)
	ON_COMMAND(ID_EDIT_SENDTOIMMEDIATE, OnEditSendtoimmediate)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SENDTOIMMEDIATE, OnUpdateEditSendtoimmediate)
	ON_COMMAND(ID_GAME_FUNCTIONSLIST, OnGameFunctionslist)
	ON_COMMAND(ID_SEARCH_GLOBALREPLACE, OnSearchGlobalreplace)
	ON_COMMAND(ID_COMPLETE_FUNCTION, OnCompleteFunction)
	ON_UPDATE_COMMAND_UI(ID_COMPLETE_FUNCTION, OnUpdateCompleteFunction)
	//}}AFX_MSG_MAP
  // Standard find/replace commands
	ON_COMMAND(ID_SEARCH_FIND, CEditView::OnEditFind)
	ON_COMMAND(ID_SEARCH_FINDNEXT, CEditView::OnEditRepeat)
	ON_COMMAND(ID_SEARCH_REPLACE, CEditView::OnEditReplace)
  ON_UPDATE_COMMAND_UI(ID_SEARCH_FINDNEXT, CEditView::OnUpdateNeedFind)
  // Select all
	ON_COMMAND(ID_EDIT_SELECT_ALL, CEditView::OnEditSelectAll)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CEditView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextView drawing

void CTextView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
}

/////////////////////////////////////////////////////////////////////////////
// CTextView diagnostics

#ifdef _DEBUG
void CTextView::AssertValid() const
{
	CEditView::AssertValid();
}

void CTextView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTextView message handlers

BOOL CTextView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style &= ~FWS_ADDTOTITLE;  // do not add document name to window title

	BOOL bPreCreated = CEditView::PreCreateWindow(cs);

  if (App.m_bNotepadWordWrap)    // use global pref here
	  cs.style &= ~(ES_AUTOHSCROLL|WS_HSCROLL);	// Enable word-wrapping
  else
	  cs.style |= (ES_AUTOHSCROLL|WS_HSCROLL);	// Disable word-wrapping

	return bPreCreated;
}


BOOL CTextView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default CEditView preparation
	return CEditView::OnPreparePrinting(pInfo);
}

void CTextView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Default CEditView begin printing.
	CEditView::OnBeginPrinting(pDC, pInfo);
}

void CTextView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Default CEditView end printing
	CEditView::OnEndPrinting(pDC, pInfo);
}

void CTextView::OnInitialUpdate() 
{
	CTextDocument* pDoc = (CTextDocument*) GetDocument();
	ASSERT_VALID(pDoc);

	CEditView::OnInitialUpdate();
	
  SetTheFont ();
	
  m_backbr = new CBrush (pDoc->m_backColour);
  m_backcolour = pDoc->m_backColour;

}


void CTextView::OnEditGoto() 
{
CGoToLineDlg dlg;

  dlg.m_iMaxLine = 	GetEditCtrl().GetLineCount ();

  // default to the current line number
  dlg.m_iLineNumber = GetEditCtrl().LineFromChar () + 1;

  if (dlg.DoModal () != IDOK)
    return;

  // find position in file - this needs to be zero-relative
  int iIndex = GetEditCtrl().LineIndex (dlg.m_iLineNumber - 1);

  if (iIndex == -1)
    return;   // failed to find position

  // go to the start of that line
  GetEditCtrl().SetSel(iIndex, iIndex);
	
}


void CTextView::OnEscape() 
{
  int nStartChar,
      nEndChar;

  GetEditCtrl().GetSel(nStartChar, nEndChar);

  if (nEndChar > nStartChar &&
                nStartChar != -1 &&
                nEndChar != -1)
    GetEditCtrl().SetSel(nEndChar, nEndChar);
	
}


void CTextView::SerializeRaw(CArchive& ar)
	// Read/Write object as stand-alone file.
{
	ASSERT_VALID(this);

	if (ar.IsStoring())
	{
		WriteToArchive(ar);
	}
	else
	{
		CFile* pFile = ar.GetFile();
		ASSERT(pFile->GetPosition() == 0);
		DWORD nFileSize = pFile->GetLength(); 
		// ReadFromArchive takes the number of characters as argument
		ReadFromArchive(ar, (UINT)nFileSize/sizeof(TCHAR));
	}
	ASSERT_VALID(this);
} /* end of CTextView::SerializeRaw */


void CTextView::OnViewMaximize() 
{
WINDOWPLACEMENT wp;
  wp.length = sizeof (wp);
  GetParent ()->GetWindowPlacement (&wp);

  // if maximized, show normally otherwise maximize it
  if (wp.showCmd == SW_MAXIMIZE)
    GetParent ()->ShowWindow (SW_SHOWNORMAL);	
  else
    GetParent ()->ShowWindow (SW_SHOWMAXIMIZED);	
	
}

void CTextView::OnUpdateViewMaximize(CCmdUI* pCmdUI) 
{
WINDOWPLACEMENT wp;
  wp.length = sizeof (wp);
  GetParent ()->GetWindowPlacement (&wp);
  pCmdUI->SetCheck (wp.showCmd == SW_MAXIMIZE);
  pCmdUI->Enable ();
	
}

bool CTextView::GetSelection (CString & strSelection)
  {
int nStartChar,
    nEndChar;

  // find the selection range
  GetEditCtrl().GetSel(nStartChar, nEndChar);
  // get window text
  GetEditCtrl().GetWindowText (strSelection);

  // get selection unless no selection in which case take all of it
  if (nEndChar > nStartChar)
    {
    strSelection = strSelection.Mid (nStartChar, nEndChar - nStartChar);
    return false;
    }

  return true;    // all was selected

  } // end of GetSelection

void CTextView::OnConvertUnixtodos() 
{
CString strText;
CString strNewText;
const char * pOld;
char * pNew;
int iNewLines = 0;
bool bAll;

  // get contents of selection
  bAll = GetSelection (strText);
  pOld = strText;   // where we are copying from

  // count how many newlines we see
  for (pOld = strText; *pOld; pOld++)
    if (*pOld == '\n')
      iNewLines++;

  // allocate memory for new buffer
  pNew = strNewText.GetBuffer (strText.GetLength () + iNewLines + 1);

  for (pOld = strText; *pOld; pOld++)
    {
    // ignore any existing carriage-returns
    if (*pOld == '\r')
      continue;
    // when we see a linefeed, add a carriage return first
    if (*pOld == '\n')
      *pNew++ = '\r';
    // now copy over whatever it was
    *pNew++ = *pOld;
    }
  
  *pNew++ = 0;    // terminator at end
  strNewText.ReleaseBuffer ();

  // put selection back    
  ReplaceAndReselect (bAll, strNewText);

}

void CTextView::OnConvertDostounix() 
{
CString strText;
const char * pOld;
char * pNew;
bool bAll;

  // get contents of selection
  bAll = GetSelection (strText);
  pOld = strText;   // where we are copying from
  pNew = strText.GetBuffer (strText.GetLength () + 1);

  for (pOld = strText; *pOld; pOld++)
    {
    // skip carriage returns
    if (*pOld == '\r')
      continue;
    // copy everything else
    *pNew++ = *pOld;
    }
	
  *pNew++ = 0;    // terminator at end
  strText.ReleaseBuffer ();

  // put selection back
  ReplaceAndReselect (bAll, strText);

}

void CTextView::OnConvertMactodos() 
{
CString strText;
CString strNewText;
const char * pOld;
char * pNew;
int iNewLines = 0;
bool bAll;

  // get contents of selection
  bAll = GetSelection (strText);
  pOld = strText;   // where we are copying from

  // count how many carriage returns we see
  for (pOld = strText; *pOld; pOld++)
    if (*pOld == '\r')
      iNewLines++;

  // allocate memory for new buffer
  pNew = strNewText.GetBuffer (strText.GetLength () + iNewLines + 1);

  for (pOld = strText; *pOld; pOld++)
    {
    // ignore any existing newlines
    if (*pOld == '\n')
      continue;
    // now copy over whatever it was
    *pNew++ = *pOld;
    // after we see a carriage return, add a newline afterwards
    if (*pOld == '\r')
      *pNew++ = '\n';
    }
  
  *pNew++ = 0;    // terminator at end
  strNewText.ReleaseBuffer ();
  
  // put selection back
  ReplaceAndReselect (bAll, strNewText);
	
}

void CTextView::OnConvertDostomac() 
{
CString strText;
const char * pOld;
char * pNew;
bool bAll;

  // get contents of selection
  bAll = GetSelection (strText);
  pOld = strText;   // where we are copying from
  pNew = strText.GetBuffer (strText.GetLength () + 1);

  for (pOld = strText; *pOld; pOld++)
    {
    // skip newlines
    if (*pOld == '\n')
      continue;
    // copy everything else
    *pNew++ = *pOld;
    }
	
  *pNew++ = 0;    // terminator at end
  strText.ReleaseBuffer ();
  
  // put selection back
  ReplaceAndReselect (bAll, strText);

	
}

void CTextView::OnConvertRemoveendoflines() 
{
CString strText;
const char * pOld;
char * pNew;
bool bAll;
char cLastChar = ' ';

  // get contents of selection
  bAll = GetSelection (strText);
  pOld = strText;   // where we are copying from
  pNew = strText.GetBuffer (strText.GetLength () + 1);

  // the buffer should not get larger, after all we are, if anything, replacing
  // \r\n with either nothing or a space.

  for (pOld = strText; *pOld; )
    {
    // end-of-line will start with a carriage-return
    if (*pOld == '\r')
      {
      // advance past carriage-return, if we have a newline, skip that as well
      if (*++pOld == '\n')
        pOld++;
      // if we have another \r\n after this one, with only spaces or tabs inbetween
      // leave it in - it will be a new paragraph
      const char * p = pOld;
      while (*p == ' ' || *p == '\t')
        *p++;
      if (p [0] == '\r' && p [1] == '\n')
        {
        pOld = p + 2;   // bypass carriage-return/linefeed
        *pNew++ = '\r'; // however make sure they end up in the output
        *pNew++ = '\n';
        }
      else
        // if last thing we copied was not a space, better put one in
        if (cLastChar != ' ')
          *pNew++ = ' ';
      }
    else
      // copy everything else
      cLastChar = *pNew++ = *pOld++;
    }
	
  *pNew++ = 0;    // terminator at end
  strText.ReleaseBuffer ();
  
  // put selection back
  ReplaceAndReselect (bAll, strText);
	
}

void CTextView::OnConvertQuotelines() 
{
CString strText;
CString strNewText;
const char * pOld;
char * pNew;
int iNewLines = 1;    // allow for doing it to the first line
bool bAll;
int iQuoteLength = App.m_strNotepadQuoteString.GetLength ();
const char * p;

  // get contents of selection
  bAll = GetSelection (strText);
  pOld = strText;   // where we are copying from

  // count how many newlines we see
  for (pOld = strText; *pOld; pOld++)
    if (*pOld == '\n')
      iNewLines++;

  // allocate memory for new buffer
  pNew = strNewText.GetBuffer (strText.GetLength () + (iNewLines * iQuoteLength) + 1);

  // quote first line
  for (p = App.m_strNotepadQuoteString; *p; p++)
    *pNew++ = *p;

  for (pOld = strText; *pOld; pOld++)
    {
    // now copy over whatever it was
    *pNew++ = *pOld;
    // when we see a linefeed, add the quote string to after it
    if (*pOld == '\n')
      {
      for (p = App.m_strNotepadQuoteString; *p; p++)
        *pNew++ = *p;
      }
    }
  
  *pNew++ = 0;    // terminator at end
  strNewText.ReleaseBuffer ();
  
  // put selection back
  ReplaceAndReselect (bAll, strNewText);

	
}

void CTextView::OnEditInsertdatetime() 
{
  CTime theTime = CTime::GetCurrentTime();

  CString strTime = theTime.Format (TranslateTime ("%A, %#d %B %Y, %#I:%M %p"));

  // insert the time
  GetEditCtrl ().ReplaceSel (strTime, true);
}

void CTextView::OnEditWordcount() 
{
CString strText;
bool bAll;
const char * p;
int iNewLines = 0;    
int iWords = 0;
  
  // get contents of selection
  bAll = GetSelection (strText);

  
  // count how many newlines we see
  for (p = strText; *p; p++)
    {
    // first count lines
    if (*p == '\n')
      iNewLines++;
    // we will define a word as something that is not a space preceded by a space
    if (isspace (p[0]) && !isspace (p[1]) && p[1])
      iWords++;
    }


  if (!strText.IsEmpty ())
    {
    // unless zero length, must have one line in it
    iNewLines++;
    // if first character is not a space, that counts as our first word
    if (!isspace (strText [0]))
      iWords++;
    }

  CString strSelection;
  if (bAll)
    strSelection = "document";
  else
    strSelection = "selection";

  CString strDisplay;

  strDisplay = TFormat ("The %s contains %i line%s, %i word%s, %i character%s",
       (LPCTSTR) strSelection,
       PLURAL (iNewLines),
       PLURAL (iWords),
       PLURAL (strText.GetLength ()));

  UMessageBox (strDisplay, MB_ICONINFORMATION);

}

void CTextView::OnConvertUppercase() 
{
CString strText;
bool bAll;

  // get contents of selection
  bAll = GetSelection (strText);

  strText.MakeUpper ();
  
  // put selection back
  ReplaceAndReselect (bAll, strText);
	
}

void CTextView::OnConvertLowercase() 
{
CString strText;
bool bAll;

  // get contents of selection
  bAll = GetSelection (strText);

  strText.MakeLower ();
  
  // put selection back
  ReplaceAndReselect (bAll, strText);
	
}

void CTextView::OnConvertRemoveextrablanks() 
{
CString strText;
const char * pOld;
char * pNew;
bool bAll;

  // get contents of selection
  bAll = GetSelection (strText);
  pOld = strText;   // where we are copying from
  pNew = strText.GetBuffer (strText.GetLength () + 1);

  for (pOld = strText; *pOld; pOld++)
    {
    if (*pOld == ' ' || *pOld == '\t')
      {
      // we have whitespace, if next character not whitespace
      // copy a space across (ie. converting tabs to spaces), otherwise
      // drop it
      if (pOld [1] != ' ' && pOld [1] != '\t')
        *pNew++ = ' ';
      continue;
      }
    // copy everything else
    *pNew++ = *pOld;
    }
	
  *pNew++ = 0;    // terminator at end
  strText.ReleaseBuffer ();
  
  // put selection back
  ReplaceAndReselect (bAll, strText);
	
}

void CTextView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CTextDocument * pDoc = (CTextDocument *) GetDocument();
	ASSERT_VALID(pDoc);

  CString strOldTitle;
  CString strNewTitle = "Untitled";

  if (!pDoc->m_strTitle.IsEmpty ())
     strNewTitle = pDoc->m_strTitle;

  // Show full pathname to document in title
  if (!pDoc->GetPathName ().IsEmpty ())
    strNewTitle = pDoc->GetPathName ();

  GetParentFrame ()->GetWindowText (strOldTitle);

  // amend title if necessary (avoid flicker)
  if (strOldTitle != strNewTitle)
    GetParentFrame ()->SetWindowText (strNewTitle);
	
	
}


void CTextView::OnEditSpellcheck() 
{
    Frame.SetStatusMessageNow (Translate ("Spell check ..."));
    App.SpellCheck (this, &GetEditCtrl());
    Frame.SetStatusNormal (); 
}

void CTextView::OnUpdateEditSpellcheck(CCmdUI* pCmdUI) 
{
CString strCurrent;

  GetEditCtrl().GetWindowText (strCurrent);
	
  // they can do a spell check if not empty
  pCmdUI->Enable (!strCurrent.IsEmpty () && App.m_bSpellCheckOK);
	
}


bool CTextView::SetText(const char * sText) 
  {
  try
    {
    int nLen = strlen (sText);

	  LPVOID hText = LocalAlloc(LMEM_MOVEABLE, (nLen+1)*sizeof(TCHAR));
	  if (hText == NULL)
		  AfxThrowMemoryException();

	  LPTSTR lpszText = (LPTSTR)LocalLock(hText);
	  ASSERT(lpszText != NULL);

    strcpy (lpszText, sText);

#ifndef _UNICODE
	  if (bWin95 || bWin98)
	  {
		  // set the text with SetWindowText, then free
		  BOOL bResult = ::SetWindowText(m_hWnd, lpszText);
		  LocalUnlock(hText);
		  LocalFree(hText);

		  // make sure that SetWindowText was successful
		  if (!bResult || ::GetWindowTextLength(m_hWnd) < nLen)
			  AfxThrowMemoryException();

		  // remove old shadow buffer
		  delete[] m_pShadowBuffer;
		  m_pShadowBuffer = NULL;
		  m_nShadowSize = 0;

		  ASSERT_VALID(this);
	  }    // end of Win95
    else
#endif

      {   // not Win95
	    LocalUnlock(hText);
	    HLOCAL hOldText = GetEditCtrl().GetHandle();
	    ASSERT(hOldText != NULL);
	    LocalFree(hOldText);
	    GetEditCtrl().SetHandle((HLOCAL)(UINT)(DWORD)hText);
	    Invalidate();
	    ASSERT_VALID(this);
      }   // end of not Win95

    } // end of try

  catch(CException* e)
    {
    e->ReportError ();
    e->Delete ();
    return false;
    }     // end of catch

  return true;
  }

BOOL CTextView::OnEraseBkgnd(CDC* pDC) 
{
	CTextDocument * pDoc = (CTextDocument*) GetDocument();
	ASSERT_VALID(pDoc);

CRect rect;

  // recreate background colour if necessary  
  if (m_backcolour != pDoc->m_backColour)
    {
    delete m_backbr;
    m_backbr = new CBrush (pDoc->m_backColour);
    m_backcolour = pDoc->m_backColour;
    }
 
  GetClientRect (&rect);

  pDC->SetBkMode (OPAQUE);
  pDC->FillRect (&rect, m_backbr);

  return TRUE;
}

HBRUSH CTextView::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	CTextDocument * pDoc = (CTextDocument*) GetDocument();
	ASSERT_VALID(pDoc);
	
pDC->SetTextColor (pDoc->m_textColour); 
pDC->SetBkColor  (pDoc->m_backColour);
pDC->SetBkMode (OPAQUE);

if (m_backbr)
  return *m_backbr;
else
  return (HBRUSH) GetStockObject (BLACK_BRUSH);
}


void CTextView::SetTheFont (void)
  {
	CTextDocument * pDoc = (CTextDocument*) GetDocument();
	ASSERT_VALID(pDoc);

  delete m_font;

  m_font = new CFont;

  if (!m_font)
    return;

  CDC dc;

  dc.CreateCompatibleDC (NULL);

   int lfHeight = -MulDiv(pDoc->m_iFontSize, dc.GetDeviceCaps(LOGPIXELSY), 72);

   DWORD iCharSet = DEFAULT_CHARSET;
   CString strFont = pDoc->m_strFontName;
   if (strFont.IsEmpty ())
     {
     strFont = App.m_strFixedPitchFont;
     iCharSet = pDoc->m_iFontCharset;
     lfHeight = -MulDiv(App.m_iFixedPitchFontSize, dc.GetDeviceCaps(LOGPIXELSY), 72);
     }

   m_font->CreateFont(lfHeight, // int nHeight, 
				  0, // int nWidth, 
				  0, // int nEscapement, 
				  0, // int nOrientation, 
				  pDoc->m_iFontWeight, // int nWeight, 
				  pDoc->m_bFontItalic,        // BYTE bItalic, 
				  pDoc->m_bFontUnderline,     // BYTE bUnderline, 
          pDoc->m_bFontStrikeout,     // BYTE cStrikeOut, 
          iCharSet, // BYTE nCharSet, 
          0, // BYTE nOutPrecision, 
          0, // BYTE nClipPrecision, 
          0, // BYTE nQuality, 
          MUSHCLIENT_FONT_FAMILY, // BYTE nPitchAndFamily,  
          strFont);// LPCTSTR lpszFacename );

    // Get the metrics of the font.

    dc.SelectObject(m_font);

    GetEditCtrl().SendMessage (WM_SETFONT,
                              (WPARAM) m_font->m_hObject,
                               MAKELPARAM (TRUE, 0));

    SetTabStops (16);

    OnUpdate (NULL, 0, NULL); // update title bar
  }

void CTextView::OnEditGotomatchingbrace() 
  {
  FindMatchingBrace (GetEditCtrl(), false);  // find without selecting range
  }

void CTextView::OnEditSelecttomatchingbrace() 
{
  FindMatchingBrace (GetEditCtrl(), true);  // find with selecting range
}


void CTextView::OnEditSendtoworld() 
{
	CTextDocument * pDoc = (CTextDocument*) GetDocument();
	ASSERT_VALID(pDoc);

CMUSHclientDoc * pWorld = pDoc->FindWorld ();

  // give up if no related world

  if (!pWorld)
    return;

CString strText;

bool bAll = GetSelection (strText);

  if (strText.IsEmpty ())
    return;

  CMemFile f ((unsigned char *) (const char *) strText, strText.GetLength ());

bool bOK =  pWorld->SendToMushHelper (&f, 
                   pWorld->m_paste_preamble,
                   pWorld->m_pasteline_preamble,
                   pWorld->m_pasteline_postamble,
                   pWorld->m_paste_postamble,
                   pWorld->m_bPasteCommentedSoftcode,
                   pWorld->m_nPasteDelay,
                   pWorld->m_nPasteDelayPerLines,
                   pWorld->m_bConfirmOnPaste,
                   pWorld->m_bPasteEcho);
	
// they sent all of it, we can claim we have saved it
  if (bOK && bAll)
    pDoc->SetModifiedFlag  (FALSE);
}

void CTextView::OnUpdateEditSendtoworld(CCmdUI* pCmdUI) 
{
	CTextDocument * pDoc = (CTextDocument*) GetDocument();
	ASSERT_VALID(pDoc);

  CMUSHclientDoc * pWorld = pDoc->FindWorld ();

  if (pWorld)
    {
    CString strText;
    GetSelection (strText);

    pCmdUI->Enable(!strText.IsEmpty ());
    pCmdUI->SetText(TFormat ("&Send To %s\tShift+Ctrl+S",
                    (LPCTSTR) pWorld->m_mush_name));	
	  }
  else
    {
    pCmdUI->Enable(FALSE);
    pCmdUI->SetText(Translate ("&Send To World\tShift+Ctrl+S"));	
    }
}

void CTextView::OnUpdateStatuslineTime(CCmdUI* pCmdUI) 
{
CString strText;
  strText.Format ("Line %i / %i", 
              GetEditCtrl().LineFromChar () + 1, 
              GetEditCtrl().GetLineCount ());
  pCmdUI->SetText (strText);
	
}


void CTextView::OnUpdateStatuslineLines(CCmdUI* pCmdUI) 
{
CString strText;

double fPercent = (double) (GetEditCtrl().LineFromChar () + 1) /
                  (double) GetEditCtrl().GetLineCount () * 100.0;

  strText.Format ("%3.0f %%", fPercent);
  pCmdUI->SetText (strText);
	
}


void CTextView::OnUpdateStatuslineFreeze(CCmdUI* pCmdUI) 
{
	CTextDocument * pDoc = (CTextDocument*) GetDocument();
	ASSERT_VALID(pDoc);

  pCmdUI->Enable (TRUE);

  if (pDoc->IsModified ())
  	pCmdUI->SetText ("Modified");
  else
  	pCmdUI->SetText (NULL);
	
}

void CTextView::OnEditRefreshrecalleddata() 
{
	CTextDocument * pDoc = (CTextDocument*) GetDocument();
	ASSERT_VALID(pDoc);

  CMUSHclientDoc * pWorld = pDoc->FindWorld ();

  if (!pWorld)
    return;

  if (pDoc->IsModified ())
    if (::UMessageBox (TFormat ("Replace entire window contents with \'recall\' from %s?",
        (LPCTSTR) pWorld->m_mush_name),
        MB_YESNO | MB_ICONQUESTION ) != IDYES)
        return;

  CString strMessage = pWorld->RecallText (
              pDoc->m_strSearchString,   // what to search for
              pDoc->m_bMatchCase ,
              pDoc->m_bRegexp,          // and other params
              pDoc->m_bCommands,
              pDoc->m_bOutput,
              pDoc->m_bNotes,
              pDoc->m_iLines,
              pDoc->m_strRecallLinePreamble);
	SetText (strMessage);

}

void CTextView::OnUpdateEditRefreshrecalleddata(CCmdUI* pCmdUI) 
{
	CTextDocument * pDoc = (CTextDocument*) GetDocument();
	ASSERT_VALID(pDoc);

  CMUSHclientDoc * pWorld = pDoc->FindWorld ();

  if (!pWorld)
    {
    pCmdUI->Enable (FALSE);
    return;
    }

  // they can refresh if we know what to do
  pCmdUI->Enable (!pDoc->m_strSearchString.IsEmpty ());	
}


void CTextView::ReplaceAndReselect (const bool bAll, const CString & strText)
  {
  int nStartChar,
      nEndChar;

  GetEditCtrl().GetSel(nStartChar, nEndChar);

  // if no selection (ie. doing whole window, select all of it)
  if (bAll)
    GetEditCtrl ().SetSel (0, -1, TRUE);

  GetEditCtrl ().ReplaceSel (strText, true);

  if (!bAll)
    {
    nEndChar = nStartChar + strText.GetLength ();
    GetEditCtrl().SetSel(nStartChar, nEndChar);
    }
  }

void CTextView::OnEditSendtocommandwindow() 
{
	CTextDocument * pDoc = (CTextDocument*) GetDocument();
	ASSERT_VALID(pDoc);

  CMUSHclientDoc * pWorld = pDoc->FindWorld ();

  if (!pWorld)
    return;

  CString strText;
  GetSelection (strText);

  if (strText.IsEmpty ())
    return;

  // put selection into command buffer
  for(POSITION pos = pWorld->GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = pWorld->GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	  {
		  CSendView* pmyView = (CSendView*)pView;

      // check they want to replace it
      if (pmyView->CheckTyping (pWorld, strText))
        return;
      pmyView->SetCommand (strText);

      // now activate the view
      if (pmyView->GetParentFrame ()->IsIconic ())
        pmyView->GetParentFrame ()->ShowWindow (SW_RESTORE);

      pmyView->GetParentFrame ()->ActivateFrame ();
      pmyView->m_owner_frame->SetActiveView(pmyView);

      break;    // just do first view that we can use
	    }	  // end of being a CMUSHView
    }   // end of loop through views
   

	
}

void CTextView::OnUpdateEditSendtocommandwindow(CCmdUI* pCmdUI) 
{
	CTextDocument * pDoc = (CTextDocument*) GetDocument();
	ASSERT_VALID(pDoc);

  CMUSHclientDoc * pWorld = pDoc->FindWorld ();

  if (pWorld)
    {
    CString strText;
    GetSelection (strText);
    pCmdUI->Enable(!strText.IsEmpty ());
	  }
  else
    pCmdUI->Enable(FALSE);
	
}

void CTextView::OnEditFliptonotepad() 
{
	CTextDocument * pDoc = (CTextDocument*) GetDocument();
	ASSERT_VALID(pDoc);

  CMUSHclientDoc * pWorld = pDoc->FindWorld ();

  if (!pWorld)
    return;

  for(POSITION pos = pWorld->GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = pWorld->GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	  {
		  CSendView* pmyView = (CSendView*)pView;

      // now activate the view
      if (pmyView->GetParentFrame ()->IsIconic ())
        pmyView->GetParentFrame ()->ShowWindow (SW_RESTORE);

      pmyView->GetParentFrame ()->ActivateFrame ();
      pmyView->m_owner_frame->SetActiveView(pmyView);

      break;    // just do first view that we can use
	    }	  // end of being a CMUSHView
    }   // end of loop through views
   
	
}

void CTextView::OnUpdateEditFliptonotepad(CCmdUI* pCmdUI) 
{
	CTextDocument * pDoc = (CTextDocument*) GetDocument();
	ASSERT_VALID(pDoc);

  CMUSHclientDoc * pWorld = pDoc->FindWorld ();

  if (pWorld)
    {
    pCmdUI->Enable(TRUE);
    pCmdUI->SetText(TFormat ("&Flip To %s\tCtrl+Alt+Space",
                    (LPCTSTR) pWorld->m_mush_name));	
	  }
  else
    {
    pCmdUI->Enable(FALSE);
    pCmdUI->SetText(Translate ("&Flip To World\tCtrl+Alt+Space"));	
    }
	
}

void CTextView::OnUpdateStatuslineLog(CCmdUI* pCmdUI) 
{
  if (m_bInsertMode)
    pCmdUI->SetText ("INS");
  else
    pCmdUI->SetText ("OVR");
  pCmdUI->Enable ();
}

void CTextView::OnEditInsert() 
{
  m_bInsertMode = !m_bInsertMode;	
}

void CTextView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  int nStartChar,
      nEndChar;

  GetEditCtrl ().GetSel (nStartChar, nEndChar);


  // for overwrite mode we will select (and therefore replace)
  // non-control characters providing there isn't a selection

  if (nChar >= ' ' && 
      !m_bInsertMode	&&
       nStartChar == nEndChar)
    {
    CString strText;
    GetEditCtrl().GetWindowText (strText);

    // don't overwrite the end of a line
    if (nEndChar < (strText.GetLength () - 2))
      if (strText.Mid (nStartChar, 2) == ENDLINE)
        {
      	CEditView::OnChar(nChar, nRepCnt, nFlags);
        return;
        }

    GetEditCtrl ().SetSel (nStartChar, nStartChar+1);
    GetEditCtrl ().ReplaceSel (CString ((TCHAR)nChar), true);
    }
  else
    	CEditView::OnChar(nChar, nRepCnt, nFlags);
}


// for forum posts, where I am quoting code, I want to fix up
// [ ] and \ to have a \ in front of them

void CTextView::OnConvertQuoteforumcodes() 
{
CString strText;
bool bAll;

  // get contents of selection
  bAll = GetSelection (strText);

  // put selection back
  ReplaceAndReselect (bAll, QuoteForumCodes (strText));
	
}

void CTextView::OnConvertBase64encode() 
{
CString strText;
bool bAll = GetSelection (strText);
ReplaceAndReselect (bAll, encodeBase64 (strText, true));
}

void CTextView::OnConvertBase64decode() 
{
CString strText;
bool bAll;

  try
    {
    bAll = GetSelection (strText);
    strText =  decodeBase64 (strText);
    }

  catch(CException* e)
    {
    e->ReportError ();
    e->Delete ();
    return;
    }     // end of catch

ReplaceAndReselect (bAll, strText);
}

void CTextView::OnConvertConverthtmlspecial() 
{
CString strText;
CString strNewText;
const char * pOld;
char * pNew;
int iExtra = 0; 
bool bAll;

  // get contents of selection
  bAll = GetSelection (strText);
  pOld = strText;   // where we are copying from

  // count how many characters we see that need converting
  for (pOld = strText; *pOld; pOld++)
    switch (*pOld)
      {
      case '<':   iExtra += 3; break;  // < -> &lt;
      case '>':   iExtra += 3; break;  // > -> &gt;
      case '&':   iExtra += 4; break;  // & -> &amp;
      case '\"':  iExtra += 5; break;  // ' -> &quot;
      }   // end of switch


  // allocate memory for new buffer
  pNew = strNewText.GetBuffer (strText.GetLength () + iExtra + 1);

  for (pOld = strText; *pOld; pOld++)
    {
    const char * p;

    switch (*pOld)
      {
      case '<': 
          for (p = "&lt;"; *p; p++)
            *pNew++ = *p;
          break;

      case '>': 
          for (p = "&gt;"; *p; p++)
            *pNew++ = *p;
          break;

      case '&': 
          for (p = "&amp;"; *p; p++)
            *pNew++ = *p;
          break;

      case '\"': 
          for (p = "&quot;"; *p; p++)
            *pNew++ = *p;
          break;

      default:  
          *pNew++ = *pOld; 
          break;
      }   // end of switch
   
    }   // end of loop
  
  *pNew++ = 0;    // terminator at end
  strNewText.ReleaseBuffer ();
  
  // put selection back
  ReplaceAndReselect (bAll, strNewText);

     

}

extern char * character_entities [1];


void CTextView::OnUnconvertConverthtmlspecial() 
{
CString strText;
CString strNewText;
const char * pOld;
char * pNew;
bool bAll;
CString strEntity;
CString strEntityContents;

  // get contents of selection
  bAll = GetSelection (strText);
  pOld = strText;   // where we are copying from

  // allocate memory for new buffer
  // The buffer won't get larger, eg. &lt; becomes <, and in the worst case
  // will stay the same size.

  pNew = strNewText.GetBuffer (strText.GetLength () + 1);

  for (pOld = strText; *pOld; pOld++)
    {
    switch (*pOld)
      {
      // element - discard it
      case '<':
        pOld++; // skip < symbol
        while (*pOld && *pOld != '>')
          pOld++;

        if (*pOld != '>') 
          {
          ::TMessageBox ("Unterminated element (\"<\" not followed by \">\")", 
                            MB_ICONSTOP);
          return;    
          }

        break;

      // entity - convert it
      case '&':
        pOld++; // skip & symbol
        strEntity.Empty ();
        while (*pOld && *pOld != ';')
          strEntity += *pOld++;

        if (*pOld != ';') 
          {
          ::TMessageBox ("Unterminated entity (\"&\" not followed by \";\")", 
                            MB_ICONSTOP);
          return;    
          }
        
        // look for &#nnn; 

        if (strEntity [0] == '#')
          {
          int iResult = 0;

          // validate and work out number
          if (strEntity [1] == 'x')
            {
            for (int i = 2; i < strEntity.GetLength (); i++)
              {
              if (!isxdigit (strEntity [i]))
                {
                ::UMessageBox  (TFormat ("Invalid hex number in entity: &%s;" ,
                                    (LPCTSTR) strEntity), MB_ICONSTOP);
                return;
                }

              int iNewDigit = toupper (strEntity [i]);
              if (iNewDigit >= 'A')
                iNewDigit -= 7;
              if (iResult & 0xF0)
                ::UMessageBox (TFormat ("Invalid hex number in entity: &%s; "
                                     "- maximum of 2 hex digits",
                                    (LPCTSTR) strEntity), MB_ICONSTOP);
                iResult = (iResult << 4) + iNewDigit - '0';
              }
            } // end of hex entity
          else
            {
            for (int i = 1; i < strEntity.GetLength (); i++)
              {
              if (!isdigit (strEntity [i]))
                {
                ::UMessageBox (TFormat ("Invalid number in entity: &%s;" ,
                                    (LPCTSTR) strEntity), MB_ICONSTOP);
                return;
                }
              iResult *= 10;
              iResult += strEntity [i] - '0';
              }
            } // end of decimal entity

          if (iResult != 9)       // we will accept tabs ;)
            if (iResult < 32 ||   // don't allow nonprintable characters
                iResult > 255)   // don't allow characters more than 1 byte
                {
                ::UMessageBox (TFormat ("Disallowed number in entity: &%s;" ,
                                    (LPCTSTR) strEntity), MB_ICONSTOP);
                return;
                }
          *pNew++ = (unsigned char) iResult;
          break;
          } // end of entity starting with #

        // look up global entities
        if (App.m_EntityMap.Lookup (strEntity, strEntityContents))
          *pNew++ = strEntityContents [0]; 
        else
          {
          ::UMessageBox (TFormat ("Unknown entity: &%s;" ,
                              (LPCTSTR) strEntity), MB_ICONSTOP);
          return;
          }
        
        break;

      default:  
          *pNew++ = *pOld; 
          break;      

      } // end of switch on current character
    
    }   // end of loop

  *pNew++ = 0;    // terminator at end
  strNewText.ReleaseBuffer ();

  // put selection back
  ReplaceAndReselect (bAll, strNewText);
	
}


void CTextView::OnEditSendtoimmediate() 
{
	CTextDocument * pDoc = (CTextDocument*) GetDocument();
	ASSERT_VALID(pDoc);

  CMUSHclientDoc * pWorldDoc = pDoc->FindWorld ();
  
  // only if we can ;)
  if (!pWorldDoc || !pWorldDoc->m_ScriptEngine)
    return;

  CString strText;

  GetSelection (strText);

  Frame.SetStatusMessageNow (Translate ("Executing immediate script"));

  pWorldDoc->m_ScriptEngine->Parse (strText, "Immediate");

  Frame.SetStatusNormal (); 

}

void CTextView::OnUpdateEditSendtoimmediate(CCmdUI* pCmdUI) 
{

	CTextDocument * pDoc = (CTextDocument*) GetDocument();
	ASSERT_VALID(pDoc);

  CMUSHclientDoc * pWorldDoc = pDoc->FindWorld ();

  if (pWorldDoc && pWorldDoc->m_ScriptEngine)
    pCmdUI->Enable (TRUE);
  else
    pCmdUI->Enable (FALSE);
	
}

void CTextView::OnGameFunctionslist() 
{
int nStartChar,
    nEndChar;
CString strSelection;

  // find the selection range
  GetEditCtrl().GetSel(nStartChar, nEndChar);
  // get window text
  GetEditCtrl().GetWindowText (strSelection);

  ShowFunctionslist (strSelection, nStartChar, nEndChar, true);

}

extern const struct luaL_reg *ptr_xmllib;

void CTextView::OpenLuaDelayed ()
  {
  L = MakeLuaState();   /* opens Lua */
  if (!L)
    return;         // can't open Lua

  luaL_openlibs (L);           // open all standard Lua libraries

  luaopen_rex (L);             // regular expression library
  luaopen_bits (L);            // bit manipulation library
  luaopen_compress (L);  // compression (utils) library
  luaopen_progress_dialog (L); // progress dialog
  luaopen_bc (L);   // open bc library   
  luaopen_lsqlite3 (L);   // open sqlite library
  lua_pushcfunction(L, luaopen_lpeg);   // open lpeg library
  lua_call(L, 0, 0);

  // add xml reader to utils lib
  luaL_register (L, "utils", ptr_xmllib);

  lua_settop(L, 0);   // clear stack

  // unless they explicitly enable it, remove ability to load DLLs
  DisableDLLs (L);

  } // end of OpenLuaDelayed


void CTextView::OpenLua ()
  {

   OpenLuaDelayed ();


  }   // end of CScriptEngine::OpenLua

bool CTextView::ReplaceBlock (const char * source,
                              const int sourcelen,
                              CString & result,
                              int & count)
  {

  // we have pre-pushed the various things needed to do a string.gsub, just make a copy of them

  lua_pushvalue(L, 2);        // string.gsub function
  lua_pushlstring (L, source, sourcelen);     // text to search (varies each time of course)
  lua_pushvalue(L, 3);        // what to find
  lua_pushvalue(L, 4);        // replacement text or function

  if (lua_pcall (L, 3, 2, 0))   // call with 3 args and 2 results
    {
    LuaError (L);
    lua_close (L);
    return true; // bad result
    }

  // count is 2nd result (therefore top of stack)
  count += lua_tonumber (L, -1);   // how many replaced

  // get results of find (1st result, therefore 2 from top)
  const char * p = lua_tostring (L, -2);  // result string

  if (p == NULL)
    {
    lua_close (L);
    return true;    // no string result? bad! very bad.
    }

  result = p;  // new string

  lua_pop (L, 2);   // pop result and count

  return false;  // ok result

  } // end of CTextView::ReplaceBlock 

void CTextView::OnSearchGlobalreplace() 
{
CLuaGsubDlg dlg;
  
  OpenLua ();    // get private Lua script space

  if (!L)
    return;   // doh!

  lua_settop(L, 0);   // clear stack, just in case

int nStartChar,
    nEndChar;

  // find the selection range
  GetEditCtrl().GetSel(nStartChar, nEndChar);

CString strText;
bool bAll;

  // get contents of selection
  bAll = GetSelection (strText);

  strText.Replace ("\r", "");  // get rid of carriage-returns

  // show selection size
  if (nEndChar > nStartChar)
    dlg.m_strSelectionSize = TFormat ("%i character%s selected.", PLURAL (strText.GetLength ()));
  else
    dlg.m_strSelectionSize = TFormat ("All text selected: %i character%s", PLURAL (strText.GetLength ()));


  int iLines = 0; 

  // count lines in selection
  const char * p = strText;
  while (*p)
    if (*p++ == '\n')
      iLines++;

  dlg.m_strSelectionSize += TFormat (" (%i line break%s)", PLURAL (iLines));

  // restore from last time              
  dlg.m_strFindPattern    = App.m_strFind;
  dlg.m_strReplacement    = App.m_strReplace;
  dlg.m_bEachLine         = App.m_bEachLine;
  dlg.m_bEscapeSequences  = App.m_bEscapeSequences;
  dlg.m_strFunctionText   = App.m_strFunctionText;
  dlg.m_bCallFunction     = App.m_bCallFunction;
  dlg.L                   = L;  // for parsing function

  if (dlg.DoModal () != IDOK)
    return;  // ach! they decided against it

  // save for next time
  App.m_strFind             =   dlg.m_strFindPattern;  
  App.m_strReplace          =   dlg.m_strReplacement;
  App.m_bEachLine           =   dlg.m_bEachLine;
  App.m_bEscapeSequences    =   dlg.m_bEscapeSequences;
  App.m_strFunctionText     =   dlg.m_strFunctionText;
  App.m_bCallFunction       =   dlg.m_bCallFunction;

  // escape sequences mean they can type things like \n in the find or replace box

  if (dlg.m_bEscapeSequences)
    {
    dlg.m_strFindPattern = FixupEscapeSequences (dlg.m_strFindPattern);
    if (!dlg.m_bCallFunction)
      dlg.m_strReplacement = FixupEscapeSequences (dlg.m_strReplacement);
    }

  const char * source       = strText;
  const char * find         = dlg.m_strFindPattern;
  const char * replace      = dlg.m_strReplacement;
  const bool bCallFunction  = dlg.m_bCallFunction;
  
  //--------------------------------------------------------------------------------
  // To save time, we will push all required functions and strings onto the stack now
  //  and simply duplicate them in ReplaceBlock. This saves some lookups later on.
  //--------------------------------------------------------------------------------

  lua_settop(L, 0);               // clear stack
  lua_getglobal (L, "string");    // STACK 1: string functions table - keep on stack for each iteration
  lua_pushliteral (L, "gsub");    // gsub function 
  lua_gettable (L, -2);           // STACK 2: get (string.gsub) function 
  lua_pushstring (L, find);       // STACK 3: what to find 

  // STACK 4: final argument might be either a substitution string, or a function itself
  if (bCallFunction)
    {
    if (!GetNestedFunction (L, replace, true))
      return;  // function not found
    }
  else
    lua_pushstring (L, replace);  // STACK 4: otherwise get replacement text

  CString result;
  CString strLine;
  int count = 0;

  // if each line, look for newlines, and send text up to newline to the string.gsub
  if (dlg.m_bEachLine)
    {
    const char * end;

    /* repeat for each newline */
    while ((end = strchr (source, '\n')) != NULL) 
      {
      if (ReplaceBlock (source, end - source, strLine, count))
        return;   // error in string.gsub

      result += strLine + '\n';  // add to result, with newline separator
      source = end + 1;          // skip separator
      }
    }   // end of do each line

  // do final string (or whole string, if not doing each line)
  if (ReplaceBlock (source, strlen (source), strLine, count))
    return;      // error in string.gsub

  result += strLine;

  // back to \r\n to keep edit control happy
  result.Replace ("\n", ENDLINE);  

  // put selection back
  ReplaceAndReselect (bAll, result);

  lua_close (L);      // done with Lua script space

  // show count of replacements in status bar
  Frame.SetStatusMessageNow (TFormat ("%i replaced.", count));
  
  }

void CTextView::OnCompleteFunction() 
{
FunctionMenu (GetEditCtrl(), true);
}

void CTextView::OnUpdateCompleteFunction(CCmdUI* pCmdUI) 
{
pCmdUI->Enable ();
}
