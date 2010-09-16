// sendvw.cpp : implementation file
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1994 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "MUSHclient.h"

#include "doc.h"
#include "MUSHview.h"
#include "sendvw.h"
#include "mainfrm.h"
#include "childfrm.h"
#include "dialogs\GlobalChangeDlg.h"
#include "dialogs\GeneratedNameDlg.h"
#include "ActivityView.h"

#include "dialogs\cmdhist.h"

#include "ActivityDoc.h"

#include "winplace.h"

#include "scripting\errors.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static CString RandomString();

/////////////////////////////////////////////////////////////////////////////
// CSendView

IMPLEMENT_DYNCREATE(CSendView, CEditView)

BEGIN_MESSAGE_MAP(CSendView, CEditView)
	ON_WM_CONTEXTMENU()
	//{{AFX_MSG_MAP(CSendView)
	ON_WM_CHAR()
	ON_COMMAND(ID_KEYS_NEXTCOMMAND, OnKeysNextcommand)
	ON_COMMAND(ID_KEYS_PREVCOMMAND, OnKeysPrevcommand)
	ON_COMMAND(ID_TEST_END, OnTestEnd)
	ON_COMMAND(ID_TEST_PAGEDOWN, OnTestPagedown)
	ON_COMMAND(ID_TEST_PAGEUP, OnTestPageup)
	ON_COMMAND(ID_TEST_START, OnTestStart)
	ON_COMMAND(ID_TEST_LINEDOWN, OnTestLinedown)
	ON_COMMAND(ID_TEST_LINEUP, OnTestLineup)
	ON_COMMAND(ID_GAME_DOWN, OnGameDown)
	ON_COMMAND(ID_GAME_EAST, OnGameEast)
	ON_COMMAND(ID_GAME_EXAMINE, OnGameExamine)
	ON_COMMAND(ID_GAME_LOOK, OnGameLook)
	ON_COMMAND(ID_GAME_NORTH, OnGameNorth)
	ON_COMMAND(ID_GAME_SOUTH, OnGameSouth)
	ON_COMMAND(ID_GAME_UP, OnGameUp)
	ON_COMMAND(ID_GAME_WEST, OnGameWest)
	ON_COMMAND(ID_GAME_SAY, OnGameSay)
	ON_COMMAND(ID_GAME_WHISPER, OnGameWhisper)
	ON_COMMAND(ID_GAME_PAGESOMEONE, OnGamePagesomeone)
	ON_COMMAND(ID_GAME_COMMANDHISTORY, OnGameCommandhistory)
	ON_COMMAND(ID_GAME_DOING, OnGameDoing)
	ON_COMMAND(ID_GAME_WHO, OnGameWho)
	ON_COMMAND(ID_GAME_DROP, OnGameDrop)
	ON_COMMAND(ID_GAME_TAKE, OnGameTake)
	ON_COMMAND(ID_DISPLAY_FREEZEOUTPUT, OnDisplayFreezeoutput)
	ON_UPDATE_COMMAND_UI(ID_DISPLAY_FREEZEOUTPUT, OnUpdateDisplayFreezeoutput)
	ON_UPDATE_COMMAND_UI(ID_STATUSLINE_FREEZE, OnUpdateStatuslineFreeze)
	ON_COMMAND(ID_ACTIONS_LOGOUT, OnActionsLogout)
	ON_COMMAND(ID_ACTIONS_QUIT, OnActionsQuit)
  ON_COMMAND_EX(ID_KEYPAD_0, OnKeypadCommand)
	ON_COMMAND_EX(ID_MACRO_CTRL_F10, OnMacroCommand)
  ON_COMMAND(ID_WINDOW_MINIMIZE, OnWindowMinimize)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_MINIMIZE, OnUpdateWindowMinimize)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR_REFLECT()
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	ON_UPDATE_COMMAND_UI(ID_GAME_WEST, OnUpdateSessionOpen)
	ON_COMMAND(ID_DISPLAY_FIND, OnDisplayFind)
	ON_COMMAND(ID_DISPLAY_FINDAGAIN, OnDisplayFindagain)
	ON_COMMAND(ID_FILE_PRINT_WORLD, OnFilePrintWorld)
	ON_WM_DESTROY()
	ON_COMMAND(ID_FIND_AGAIN_BACKWARDS, OnFindAgainBackwards)
	ON_UPDATE_COMMAND_UI(ID_FIND_AGAIN_BACKWARDS, OnUpdateFindAgainBackwards)
	ON_COMMAND(ID_FIND_AGAIN_FORWARDS, OnFindAgainForwards)
	ON_UPDATE_COMMAND_UI(ID_FIND_AGAIN_FORWARDS, OnUpdateFindAgainForwards)
	ON_COMMAND(ID_REPEAT_LAST_COMMAND, OnRepeatLastCommand)
	ON_UPDATE_COMMAND_UI(ID_REPEAT_LAST_COMMAND, OnUpdateRepeatLastCommand)
	ON_COMMAND(ID_ALT_DOWNARROW, OnAltDownarrow)
	ON_COMMAND(ID_ALT_UPARROW, OnAltUparrow)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_KEYS_TAB, OnKeysTab)
	ON_COMMAND(ID_DISPLAY_CLEAR_COMMAND_HISTORY, OnDisplayClearCommandHistory)
	ON_UPDATE_COMMAND_UI(ID_DISPLAY_CLEAR_COMMAND_HISTORY, OnUpdateDisplayClearCommandHistory)
	ON_COMMAND(ID_KEYS_ESCAPE, OnProcessEscape)
	ON_COMMAND(ID_DISPLAY_GOTOBOOKMARK, OnDisplayGotobookmark)
	ON_COMMAND(ID_DISPLAY_BOOKMARKSELECTION, OnDisplayBookmarkselection)
	ON_COMMAND(ID_INPUT_GLOBALCHANGE, OnInputGlobalchange)
	ON_UPDATE_COMMAND_UI(ID_INPUT_GLOBALCHANGE, OnUpdateInputGlobalchange)
	ON_COMMAND(ID_EDIT_SPELLCHECK, OnEditSpellcheck)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SPELLCHECK, OnUpdateEditSpellcheck)
	ON_COMMAND(ID_EDIT_GENERATECHARACTERNAME, OnEditGeneratecharactername)
	ON_COMMAND(ID_EDIT_NOTESWORKAREA, OnEditNotesworkarea)
	ON_COMMAND(ID_DISPLAY_GOTOLINE, OnDisplayGotoline)
	ON_COMMAND(ID_FILE_CTRL_N, OnFileCtrlN)
	ON_COMMAND(ID_FILE_CTRL_P, OnFileCtrlP)
	ON_COMMAND(ID_EDIT_CTRL_Z, OnEditCtrlZ)
	ON_COMMAND(ID_REPEAT_LAST_WORD, OnRepeatLastWord)
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_COMMAND_HOME, OnCommandHome)
	ON_COMMAND(ID_COMMAND_END, OnCommandEnd)
	ON_COMMAND(ID_EDIT_GOTOMATCHINGBRACE, OnEditGotomatchingbrace)
	ON_COMMAND(ID_EDIT_SELECTTOMATCHINGBRACE, OnEditSelecttomatchingbrace)
	ON_COMMAND(ID_DISPLAY_HIGHLIGHTPHRASE, OnDisplayHighlightphrase)
	ON_COMMAND(ID_DISPLAY_MULTILINETRIGGER, OnDisplayMultilinetrigger)
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_GAME_FUNCTIONSLIST, OnGameFunctionslist)
	ON_COMMAND(ID_COMPLETE_FUNCTION, OnCompleteFunction)
	ON_UPDATE_COMMAND_UI(ID_COMPLETE_FUNCTION, OnUpdateCompleteFunction)
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
	ON_COMMAND_EX(ID_MACRO_SHIFT_F6, OnMacroCommand)
  ON_UPDATE_COMMAND_UI(ID_GAME_WHO, OnUpdateSessionOpen)
	ON_UPDATE_COMMAND_UI(ID_GAME_WHISPER, OnUpdateSessionOpen)
	ON_UPDATE_COMMAND_UI(ID_GAME_UP, OnUpdateSessionOpen)
	ON_UPDATE_COMMAND_UI(ID_GAME_TAKE, OnUpdateSessionOpen)
	ON_UPDATE_COMMAND_UI(ID_GAME_SOUTH, OnUpdateSessionOpen)
	ON_UPDATE_COMMAND_UI(ID_GAME_SAY, OnUpdateSessionOpen)
	ON_UPDATE_COMMAND_UI(ID_GAME_PAGESOMEONE, OnUpdateSessionOpen)
	ON_UPDATE_COMMAND_UI(ID_GAME_NORTH, OnUpdateSessionOpen)
	ON_UPDATE_COMMAND_UI(ID_GAME_LOOK, OnUpdateSessionOpen)
	ON_UPDATE_COMMAND_UI(ID_GAME_EXAMINE, OnUpdateSessionOpen)
	ON_UPDATE_COMMAND_UI(ID_GAME_EAST, OnUpdateSessionOpen)
	ON_UPDATE_COMMAND_UI(ID_GAME_DROP, OnUpdateSessionOpen)
	ON_UPDATE_COMMAND_UI(ID_GAME_DOWN, OnUpdateSessionOpen)
	ON_UPDATE_COMMAND_UI(ID_GAME_DOING, OnUpdateSessionOpen)
	ON_UPDATE_COMMAND_UI(ID_ACTIONS_LOGOUT, OnUpdateSessionOpen)
	ON_UPDATE_COMMAND_UI(ID_ACTIONS_QUIT, OnUpdateSessionOpen)
	ON_COMMAND(ID_WINDOW_MAXIMIZE, OnWindowMaximize)
	ON_COMMAND(ID_WINDOW_RESTORE, OnWindowRestore)
	//}}AFX_MSG_MAP

  ON_WM_INITMENU( )
  ON_WM_INITMENUPOPUP( )

  // MXP pop-up menu
  ON_COMMAND_RANGE(MXP_FIRST_MENU, 
                    MXP_FIRST_MENU + MXP_MENU_COUNT - 1, 
                    OnMXPMenu)

  // Accelerator commands
  ON_COMMAND_RANGE(ACCELERATOR_FIRST_COMMAND, 
                   ACCELERATOR_FIRST_COMMAND + ACCELERATOR_COUNT - 1, 
                   OnAcceleratorCommand)

	/* Handler to suppress default ALT key behaviour. */
	ON_WM_SYSCOMMAND()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateNeedSel)

	ON_COMMAND(ID_EDIT_COPYASHTML, OnEditCopyashtml)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPYASHTML, OnUpdateNeedOutputSel)

	ON_COMMAND(ID_DISPLAY_TEXTATTRIBUTES, OnDisplayTextattributes)
	ON_UPDATE_COMMAND_UI(ID_DISPLAY_TEXTATTRIBUTES, OnUpdateNeedOutputSel)

END_MESSAGE_MAP()

//#define new DEBUG_NEW 

/////////////////////////////////////////////////////////////////////////////
// CSendView construction/destruction

CSendView::CSendView()
{
  m_HistoryPosition = NULL;
  m_inputcount = 0;
  m_HistoryFindInfo.m_strTitle = "Find in command history...";
  m_iHistoryStatus = eAtBottom;
  m_backbr = NULL;
}

CSendView::~CSendView()
{
delete m_backbr;
}


/////////////////////////////////////////////////////////////////////////////
// CSendView drawing

void CSendView::OnDraw(CDC* pDC)
{
}

/////////////////////////////////////////////////////////////////////////////
// CSendView diagnostics

#ifdef _DEBUG
void CSendView::AssertValid() const
{
	CEditView::AssertValid();
}

void CSendView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CMUSHclientDoc* CSendView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMUSHclientDoc)));
	return (CMUSHclientDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSendView message handlers

BOOL CSendView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style &= ~FWS_ADDTOTITLE;
	BOOL ret = CEditView::PreCreateWindow(cs);
	cs.style = AFX_WS_DEFAULT_VIEW | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | 
	            ES_NOHIDESEL;
	return ret;
}

void CSendView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

	if ((nChar == VK_RETURN) && (nRepCnt == 1))
  	{

    pDoc->m_iCurrentActionSource = eUserTyping;

		CString strText;
		GetEditCtrl().GetWindowText(strText);

    // spell check on send?
    if (pDoc->m_bSpellCheckOnSend)
      {
            // look for scripting prefix
      if (pDoc->m_bEnableScripts &&            // providing scripting active
          !pDoc->m_strScriptPrefix.IsEmpty () &&    // and we *have* a script prefix
          strText.Left (pDoc->m_strScriptPrefix.GetLength ()) == pDoc->m_strScriptPrefix  // and it matches
          )
        { } // do nothing  (don't spell check script commands)
      else
        {
        Frame.SetStatusMessageNow (Translate ("Spell check ..."));
        bool bOK = App.SpellCheck (this, &GetEditCtrl());
        pDoc->ShowStatusLine ();
        if (!bOK)
          return;   // spell check was cancelled
        // get the text again, as the spell check may have changed it
		    GetEditCtrl().GetWindowText(strText);
        }

      }   // end of spell check wanted

    CPlugin * pSavedPlugin = pDoc->m_CurrentPlugin;

    // tell each plugin what we have received
    // the plugin callback OnPluginCommandEntered gets a chance to attack the entire command
    for (POSITION pluginpos = pDoc->m_PluginList.GetHeadPosition(); pluginpos; )
      {
      CPlugin * pPlugin = pDoc->m_PluginList.GetNext (pluginpos);


      if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
        continue;

      // see what the plugin makes of this,
      pPlugin->ExecutePluginScript (ON_PLUGIN_COMMAND_ENTERED,
                                    strText,  // input and output command
                                    pPlugin->m_dispid_plugin_command_entered); 
      }   // end of doing each plugin
    pDoc->m_CurrentPlugin = pSavedPlugin;

    // special string to indicate command should be discarded
    if (strText == "\t")
      {
      if (!pDoc->m_bAutoRepeat)
        SetCommand ("");
      return;
      }

    // special string to indicate command should be ignored
    if (strText == "\r")
      return;

    SendCommand (strText, FALSE);

// cancel any previous message on the status line
    pDoc->ShowStatusLine ();

    pDoc->m_iCurrentActionSource = eUnknownActionSource;
    return;

  	} // end of return key

	
  CEditView::OnChar(nChar, nRepCnt, nFlags);

}


void CSendView::OnKeysNextcommand() 
{
	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

  // no history? no next command
  if (m_msgList.IsEmpty ())
    return;

  CString strText;
  CString strCommand = GetText (GetEditCtrl());
  
  // if they want down-arrow to recall a partial command, do it
  if (pDoc->m_bArrowRecallsPartial && 
       ((!strCommand.IsEmpty () && m_bChanged) ||  // starting a new one
       !m_strPartialCommand.IsEmpty ())  // or have an existing one
     )
    {
    OnAltDownarrow ();
    return;
    }

// check they want to wipe out their typing

  if (CheckTyping (pDoc, strText))
    return;

  // if they are at the bottom, and they don't want to wrap, give up
  if (m_iHistoryStatus == eAtBottom &&
     !pDoc->m_bArrowKeysWrap)
    {   

    // check they want to wipe out their typing

    if (CheckTyping (pDoc, ""))
      return;

    SetCommand ("");
    return;
    }

  // if they typed something, we are back at the top
  // or, we are out of history, and they want to wrap
  // or, we are out of history, and we are at the top
  if (// m_bChanged ||
      (!m_HistoryPosition &&  // out of history
         (pDoc->m_bArrowKeysWrap || m_iHistoryStatus == eAtTop)) // and they want to wrap
      )
    m_HistoryPosition = m_msgList.GetHeadPosition (); // back to top
  else
    if (m_HistoryPosition)
      m_msgList.GetNext (m_HistoryPosition);

  if (m_HistoryPosition)
    strText = m_msgList.GetAt (m_HistoryPosition);  // get this one
  else
    {   // otherwise, end of list old chap
    // check they want to wipe out their typing

    if (CheckTyping (pDoc, ""))
      return;

    SetCommand ("");
    m_iHistoryStatus = eAtBottom;
    return;
    }

  m_iHistoryStatus = eInMiddle;

// check they want to wipe out their typing

  if (CheckTyping (pDoc, strText))
    return;
  
  SetCommand (strText);

}

void CSendView::OnKeysPrevcommand() 
{
	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

  TRACE ("OnKeysPrevcommand\n");

  // no history? no previous command
  if (m_msgList.IsEmpty ())
    return;

  CString strText;
  CString strCommand = GetText (GetEditCtrl());

  // if they want up-arrow to recall a partial command, do it
  if (pDoc->m_bArrowRecallsPartial && 
       ((!strCommand.IsEmpty () && m_bChanged) ||  // starting a new one
       !m_strPartialCommand.IsEmpty ())  // or have an existing one
     )
    {
    OnAltUparrow ();
    return;
    }


  // if they are at the top, and they don't want to wrap, give up
  if (m_iHistoryStatus == eAtTop &&
     !pDoc->m_bArrowKeysWrap)
    {   
    // check they want to wipe out their typing

    if (CheckTyping (pDoc, ""))
      return;

    SetCommand ("");
    return;
    }

  // if we are at the bottom, and no history position, take the last one.

  // if they typed something, we are back at the bottom
  // or, we are out of history, and they want to wrap
  // or, we are out of history, and we are at the bottom
  if (// m_bChanged ||
      (!m_HistoryPosition &&  // out of history
         (pDoc->m_bArrowKeysWrap || m_iHistoryStatus == eAtBottom)) // and they want to wrap
      )
    m_HistoryPosition = m_msgList.GetTailPosition (); // back to bottom
  else
    if (m_HistoryPosition)
      m_msgList.GetPrev (m_HistoryPosition);

  // if we have auto repeat on, skip very bottom one - they already have that
  if (m_HistoryPosition &&   
      !strCommand.IsEmpty () &&
      pDoc->m_bAutoRepeat && 
      m_iHistoryStatus == eAtBottom)
    m_msgList.GetPrev (m_HistoryPosition);

  if (m_HistoryPosition)
    strText = m_msgList.GetAt (m_HistoryPosition);  // get this one
  else
    {   // otherwise, end of list old chap
    // check they want to wipe out their typing

    if (CheckTyping (pDoc, ""))
      return;

    SetCommand ("");
    m_iHistoryStatus = eAtTop;
    return;
    }

  m_iHistoryStatus = eInMiddle;

  // check they want to wipe out their typing

  if (CheckTyping (pDoc, strText))
    return;
  
  SetCommand (strText);
  
}

void CSendView::OnTestEnd() 
{
  m_topview->doEnd ();
}

void CSendView::OnTestPagedown() 
{
  m_topview->doPagedown ();
}

void CSendView::OnTestPageup() 
{
	m_topview->doPageup ();
}

void CSendView::OnTestStart() 
{
  m_topview->doStart ();
}

void CSendView::OnTestLinedown() 
{
	m_topview->doLinedown ();
}

void CSendView::OnTestLineup() 
{
	m_topview->doLineup ();
}

void CSendView::SetCommand (LPCTSTR str)
  {

//  TRACE1 ("SetCommand = %s\n", str);
	GetEditCtrl().SetWindowText (str);
  int len = GetWindowTextLength ();
  GetEditCtrl().SetSel (len, len);
  OnChange ();
  m_bChanged = FALSE;


  }

void CSendView::SendCommand (const CString strOriginalCommand, 
                             const BOOL bSavePrevious, 
                             const BOOL bKeepInHistory)
  {

  CString strFullCommand = strOriginalCommand;

	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

  pDoc->m_bOmitFromCommandHistory = false;    // don't omit it yet

  // auto-say only applies when you actually type something, so I removed it 
  // from the execute function

  // handle auto-say
  
  BOOL bAutoSay = pDoc->m_bEnableAutoSay;

  // if auto-say enabled, see if we can exclude this line, because of override prefix
  if (bAutoSay && !pDoc->m_strOverridePrefix.IsEmpty ())    // only if we have one
    if (strFullCommand.Left (pDoc->m_strOverridePrefix.GetLength ()) == pDoc->m_strOverridePrefix)
      {
      bAutoSay = false;
      strFullCommand = strFullCommand.Mid (pDoc->m_strOverridePrefix.GetLength ());   // exclude prefix
      }

  // exclude special characters, if wanted

  if (!strFullCommand.IsEmpty ())
    if (bAutoSay && pDoc->m_bExcludeNonAlpha && 
        !(isalpha (strFullCommand [0]) || isdigit (strFullCommand [0])))
        bAutoSay = false;

  // exclude text of macros, if wanted

  if (bAutoSay && pDoc->m_bExcludeMacros)
    {
    for (int i = 0; bAutoSay && i < NUMITEMS (pDoc->m_macro_type); i++)
      if (pDoc->m_macro_type [i] == REPLACE_COMMAND || 
          pDoc->m_macro_type [i] == SEND_NOW)
        if ( !pDoc->m_macros [i].IsEmpty ())    // ignore blank macros
          if (strFullCommand.Left (pDoc->m_macros [i].GetLength ()) == pDoc->m_macros [i])
            bAutoSay = false;
    } // end of checking for macros

  // exclude auto-say sequence itself. ie. don't have "page nick =page nick ="

  if (bAutoSay && strFullCommand.Left (pDoc->m_strAutoSayString.GetLength ()) ==
      pDoc->m_strAutoSayString)
      bAutoSay = false;


  if (bAutoSay)
    {

    // let them know if they are foolishly trying to send to a closed connection

    if (!pDoc->m_bReEvaluateAutoSay && pDoc->CheckConnected ())
      return;

    // break up auto-say string into a list, terminated by newlines
    CStringList strList;
    StringToList (strFullCommand, ENDLINE, strList);
    pDoc->m_bEnableAutoSay = false; // disable to prevent loop

    // disable command stacking
    unsigned short bSaveCommandStack = pDoc->m_enable_command_stack;
    pDoc->m_enable_command_stack = false;

    for (POSITION command_pos = strList.GetHeadPosition (); command_pos; )
      {
      CString str = strList.GetNext (command_pos);
      
      if (pDoc->m_bReEvaluateAutoSay)
        {
        // evaluate aliases, speed walking, command stacking etc.

        pDoc->m_iExecutionDepth = 0;    // hand-typed command, assume depth zero

        // execution is now done separately :)

        str = pDoc->m_strAutoSayString + str;   // prepend auto-say string

        if (pDoc->Execute (
                pDoc->m_bTranslateBackslashSequences ? 
                  ::FixupEscapeSequences (str) : str
            ) != eOK)
          break;    // some obscure error? give up

        }
      else
        {  // just send it
        pDoc->SendMsg (pDoc->m_strAutoSayString + str, 
                      pDoc->m_display_my_input, 
                      false,          // don't queue
                      pDoc->LoggingInput ()); 
        }
      }

    pDoc->m_bEnableAutoSay = true; // re-enable it
    pDoc->m_enable_command_stack = bSaveCommandStack; // re-enable it

    } // end of auto say
  else
    {  // not auto-say

    // evaluate aliases, speed walking, command stacking etc.

    pDoc->m_iExecutionDepth = 0;    // hand-typed command, assume depth zero

    // execution is now done separately :)

    if (pDoc->Execute (
            pDoc->m_bTranslateBackslashSequences ? 
              ::FixupEscapeSequences (strFullCommand) : strFullCommand
        ) != eOK)
      return;
    }  // end not auto-say

  // put original command in command history (we might have stripped auto-say prefix)
  if (bKeepInHistory && !pDoc->m_bOmitFromCommandHistory)
    AddToCommandHistory (strOriginalCommand);

  // history starts at bottom of list again
  m_HistoryPosition = NULL;
  m_iHistoryStatus = eAtBottom;
  // alt+arrow now rematches on what you type
  m_strPartialCommand.Empty ();

  // remove this command, unless we are supposed to keep it
  if (!bSavePrevious)
    if (pDoc->m_bAutoRepeat && !pDoc->m_bNoEcho)    // auto repeat re-enters the last command
      {
      GetEditCtrl().SetSel (0, -1);   // select all
      m_bChanged = FALSE;   // no change yet
      }
    else
      SetCommand ("");

  // unpause the output window if wanted
  if (pDoc->m_bUnpauseOnSend && m_topview->m_freeze)
    {
    m_topview->m_freeze = false;
    m_topview->addedstuff ();   
    }

  }  // end of CSendView::SendCommand 

void CSendView::SendMacro (int whichone)
  {
	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

  // ignore empty macros
  if (pDoc->m_macros [whichone].IsEmpty ())
    return;

// turn auto-say off, they obviously don't want to say west, QUIT, etc.

  BOOL bSavedAutoSay = pDoc->m_bEnableAutoSay;
  pDoc->m_bEnableAutoSay = FALSE;

// send the command in the appropriate way

  switch (pDoc->m_macro_type [whichone])
  {
  case REPLACE_COMMAND: 

// check they want to wipe out their typing

        if (CheckTyping (pDoc, pDoc->m_macros [whichone]))
          break;

// it's OK, do it

        SetCommand (pDoc->m_macros [whichone]);
        break;

  case SEND_NOW:        

        pDoc->m_iCurrentActionSource = eUserMacro;
        SendCommand (pDoc->m_macros [whichone], TRUE, ! pDoc->m_bDoNotAddMacrosToCommandHistory);
        pDoc->m_iCurrentActionSource = eUnknownActionSource;
        break;

  case ADD_TO_COMMAND:  
        
        GetEditCtrl().ReplaceSel (pDoc->m_macros [whichone], TRUE);
        break;

  default:              
        break;  // do nothing
  } // end of switch

// restore auto-say

  pDoc->m_bEnableAutoSay = bSavedAutoSay;

  } // end of SendMacro

void CSendView::OnGameDown() 
{
  SendMacro (MAC_DOWN);
}

void CSendView::OnGameEast() 
{
  SendMacro (MAC_EAST);
}

void CSendView::OnGameExamine() 
{
  SendMacro (MAC_EXAMINE);
}

void CSendView::OnGameLook() 
{
  SendMacro (MAC_LOOK);
}

void CSendView::OnGameNorth() 
{
  SendMacro (MAC_NORTH);
}

void CSendView::OnGameSouth() 
{
  SendMacro (MAC_SOUTH);
}

void CSendView::OnGameUp() 
{
  SendMacro (MAC_UP);
}

void CSendView::OnGameWest() 
{
  SendMacro (MAC_WEST);
}

void CSendView::OnGameSay() 
{
  SendMacro (MAC_SAY);
}

void CSendView::OnGameWhisper() 
{
  SendMacro (MAC_WHISPER);
}

void CSendView::OnGamePagesomeone() 
{
  SendMacro (MAC_PAGE);
}

void CSendView::OnGameCommandhistory() 
{
	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);


CCmdHistory dlg;

  dlg.m_msgList = &m_msgList;
  dlg.m_sendview = this;
  dlg.m_pHistoryFindInfo = &m_HistoryFindInfo;    // for finding
  dlg.m_pDoc = pDoc;            // for confirming replacement of typing

  dlg.DoModal ();

}

void CSendView::OnGameDoing() 
{
  SendMacro (MAC_DOING);
}

void CSendView::OnGameWho() 
{
  SendMacro (MAC_WHO);
}

void CSendView::OnGameDrop() 
{
  SendMacro (MAC_DROP);
}

void CSendView::OnGameTake() 
{
  SendMacro (MAC_TAKE);
}


void CSendView::OnDisplayFreezeoutput() 
{
  m_topview->m_freeze = !m_topview->m_freeze;

  if (!m_topview->m_freeze)
    m_topview->addedstuff ();   
}

void CSendView::OnUpdateDisplayFreezeoutput(CCmdUI* pCmdUI) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

  pCmdUI->SetCheck (m_topview->m_freeze);
  pCmdUI->Enable (pDoc->m_iConnectPhase == eConnectConnectedToMud);
//  m_topview->OnUpdateStatuslineFreeze_helper (pCmdUI);

}

void CSendView::OnUpdateStatuslineFreeze(CCmdUI* pCmdUI) 
{

m_topview->OnUpdateStatuslineFreeze_helper (pCmdUI);

}

void CSendView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);
	
  m_topview->m_iPauseStatus = m_topview->ePauseUninitialised;

	if (bActivate)
	  {

    pDoc->m_pActiveCommandView = this;
    Frame.FixUpTitleBar ();
    if (pDoc->m_new_lines && App.m_pActivityDoc)
      {
      pDoc->m_new_lines = 0;
      App.m_bUpdateActivity = TRUE;
      }
    else
      pDoc->m_new_lines = 0;	  

    // execute "get focus" script
    if (pDeactiveView != m_topview) // don't worry about swapping with top view
      {
      if (pDoc->m_ScriptEngine)
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
        }  // end of not swapping

    // make sure status line is updated
    pDoc->ShowStatusLine ();
	  }
	else
    {
    pDoc->m_pActiveCommandView = NULL;

    Frame.FixUpTitleBar ();

    // execute "Lose focus" script
    if (pActivateView != m_topview) // don't worry about swapping with top view
      {
      if (pDoc->m_ScriptEngine)
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

      }
    // make sure status line is updated
    Frame.SetStatusNormal (); 

    }
	
	CEditView::OnActivateView(bActivate, pActivateView, pDeactiveView);

  // update which world has the tick
  if (App.m_pActivityView && App.m_pActivityDoc)
      App.m_pActivityDoc->UpdateAllViews (NULL);

}

void CSendView::OnActionsLogout() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

CString str;

  str = TFormat ("Logout from this character on %s?", (LPCTSTR) pDoc->m_mush_name);

  if (::UMessageBox (str, MB_YESNO | MB_ICONQUESTION)
      == IDYES) 
    SendMacro (MAC_LOGOUT);
}

void CSendView::OnActionsQuit() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

CString str;
  
  str = TFormat ("Quit from %s?", (LPCTSTR) pDoc->m_mush_name);

  if (::UMessageBox (str, MB_YESNO | MB_ICONQUESTION)
      == IDYES) 
    {
    pDoc->m_bDisconnectOK = true;     // don't want reconnect on quit
    SendMacro (MAC_QUIT);
    }

}


BOOL CSendView::OnKeypadCommand(UINT nIDC) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

// turn auto-say off, they obviously don't want to say west, examine, etc.

  BOOL bSavedAutoSay = pDoc->m_bEnableAutoSay;
  pDoc->m_bEnableAutoSay = FALSE;

const char * sValues [eKeypad_Max_Items] =
  {
  // normal
  "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ".", "/", "*", "-", "+",
  // ctrl
  "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ".", "/", "*", "-", "+"
  };

int iIndex = -1;

  switch (nIDC)
    {
    // normal keys
    case ID_KEYPAD_0          : iIndex = eKeypad_0    ; break;
    case ID_KEYPAD_1          : iIndex = eKeypad_1    ; break;
    case ID_KEYPAD_2          : iIndex = eKeypad_2    ; break;
    case ID_KEYPAD_3          : iIndex = eKeypad_3    ; break;
    case ID_KEYPAD_4          : iIndex = eKeypad_4    ; break;
    case ID_KEYPAD_5          : iIndex = eKeypad_5    ; break;
    case ID_KEYPAD_6          : iIndex = eKeypad_6    ; break;
    case ID_KEYPAD_7          : iIndex = eKeypad_7    ; break;
    case ID_KEYPAD_8          : iIndex = eKeypad_8    ; break;
    case ID_KEYPAD_9          : iIndex = eKeypad_9    ; break;
    case ID_KEYPAD_DOT        : iIndex = eKeypad_Dot  ; break;
    case ID_KEYPAD_SLASH      : iIndex = eKeypad_Slash; break;
    case ID_KEYPAD_STAR       : iIndex = eKeypad_Star ; break;
    case ID_KEYPAD_DASH       : iIndex = eKeypad_Dash ; break;
    case ID_KEYPAD_PLUS       : iIndex = eKeypad_Plus ; break;

    // control keys
    case  ID_CTRL_KEYPAD_0    : iIndex = eCtrl_Keypad_0    ; break;
    case  ID_CTRL_KEYPAD_1    : iIndex = eCtrl_Keypad_1    ; break;
    case  ID_CTRL_KEYPAD_2    : iIndex = eCtrl_Keypad_2    ; break;
    case  ID_CTRL_KEYPAD_3    : iIndex = eCtrl_Keypad_3    ; break;
    case  ID_CTRL_KEYPAD_4    : iIndex = eCtrl_Keypad_4    ; break;
    case  ID_CTRL_KEYPAD_5    : iIndex = eCtrl_Keypad_5    ; break;
    case  ID_CTRL_KEYPAD_6    : iIndex = eCtrl_Keypad_6    ; break;
    case  ID_CTRL_KEYPAD_7    : iIndex = eCtrl_Keypad_7    ; break;
    case  ID_CTRL_KEYPAD_8    : iIndex = eCtrl_Keypad_8    ; break;
    case  ID_CTRL_KEYPAD_9    : iIndex = eCtrl_Keypad_9    ; break;
    case  ID_CTRL_KEYPAD_DOT  : iIndex = eCtrl_Keypad_Dot  ; break;
    case  ID_CTRL_KEYPAD_SLASH: iIndex = eCtrl_Keypad_Slash; break;
    case  ID_CTRL_KEYPAD_STAR : iIndex = eCtrl_Keypad_Star ; break;
    case  ID_CTRL_KEYPAD_DASH : iIndex = eCtrl_Keypad_Dash ; break;
    case  ID_CTRL_KEYPAD_PLUS : iIndex = eCtrl_Keypad_Plus ; break;

    } // end of switch
         
  // if command found
  if (iIndex != -1)
    {
    if (pDoc->m_keypad_enable)
      {
      pDoc->m_iCurrentActionSource = eUserKeypad;
      SendCommand (pDoc->m_keypad [iIndex], TRUE, FALSE);    // do not keep in history window
      pDoc->m_iCurrentActionSource = eUnknownActionSource;
      }
    else
      GetEditCtrl().ReplaceSel (sValues [iIndex], TRUE);
    }

// restore auto-say

  pDoc->m_bEnableAutoSay = bSavedAutoSay;

  return TRUE;

}

void CSendView::OnWindowMinimize() 
{

	if (GetParentFrame ()->IsIconic ())
	  GetParentFrame ()->ShowWindow(SW_RESTORE);
  else
	  GetParentFrame ()->ShowWindow(SW_MINIMIZE);
}

void CSendView::OnUpdateWindowMinimize(CCmdUI* pCmdUI) 
{
	if (GetParentFrame ()->IsIconic ())
    pCmdUI->SetCheck (TRUE);
  else
    pCmdUI->SetCheck (FALSE);
}


BOOL CSendView::OnEraseBkgnd(CDC* pDC) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

CRect rect;


  // recreate background colour if necessary  
  if (m_backcolour != pDoc->m_input_background_colour)
    {
    delete m_backbr;
    m_backbr = new CBrush (pDoc->m_input_background_colour);
    m_backcolour = pDoc->m_input_background_colour;
    }

  GetClientRect (&rect);

  pDC->SetBkMode (OPAQUE);
  pDC->FillRect (&rect, m_backbr);

  return TRUE;
}

HBRUSH CSendView::CtlColor(CDC* pDC, UINT nCtlColor) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);
	
pDC->SetTextColor (pDoc->m_input_text_colour); 
pDC->SetBkColor  (pDoc->m_input_background_colour);
pDC->SetBkMode (OPAQUE);

if (m_backbr)
  return *m_backbr;
else
  return (HBRUSH) GetStockObject (BLACK_BRUSH);

}


// Need to have this function to stop editing in the "send" window
// setting the document "dirty" flag

void CSendView::OnChange() 
{

	m_bChanged = true;
  AdjustCommandWindowSize ();

  NotifyPluginCommandChanged ();

}


void CSendView::NotifyPluginCommandChanged ()
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

    static bool doing_change = false;

    // tell each plugin the edit window has changed. Hello, Worstje!

  if (!doing_change)      // don't recurse
    {
    doing_change = true;
    for (POSITION pluginpos = pDoc->m_PluginList.GetHeadPosition(); pluginpos; )
      {
      CPlugin * pPlugin = pDoc->m_PluginList.GetNext (pluginpos);


      if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
        continue;

      // see what the plugin makes of this,
      pPlugin->ExecutePluginScript (ON_PLUGIN_COMMAND_CHANGED,
                                    pPlugin->m_dispid_plugin_on_command_changed); 

      }   // end of doing each plugin

    doing_change = false;
    }

  }  // end of CSendView::NotifyPluginCommandChanged


void CSendView::OnInitialUpdate() 
{

  CEditView::OnInitialUpdate();

  CMUSHclientDoc* pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  
  CWindowPlacement wp;
  if (pDoc->m_bLoaded)
    wp.Restore (CString (pDoc->m_mush_name + " World Position"), m_owner_frame, false);

  if (App.m_bOpenWorldsMaximised)
     m_owner_frame->ShowWindow (SW_SHOWMAXIMIZED);

  m_owner_frame->FixUpSplitterBar ();
  
  m_backbr = new CBrush (pDoc->m_input_background_colour);
  m_backcolour = pDoc->m_input_background_colour;

  // if they want auto-command size, put back to 1
  AdjustCommandWindowSize ();
}

void CSendView::OnContextMenu(CWnd*, CPoint point)
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
		VERIFY(menu.LoadMenu(CG_IDR_POPUP_SEND_VIEW));

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
}

// Command handlers for commands which need a session open

void CSendView::OnUpdateSessionOpen(CCmdUI* pCmdUI) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

  pCmdUI->Enable (pDoc->m_iConnectPhase == eConnectConnectedToMud);    // not if session closed
}

// returns true if they don't want their typing replaced

bool CSendView::CheckTyping (CMUSHclientDoc* pDoc, CString strReplacement)
  {
const int iLimit = 200;

  if (!pDoc->m_bConfirmBeforeReplacingTyping)
    return false;

CString strCurrent;

  GetEditCtrl().GetWindowText (strCurrent);

  if (strCurrent.GetLength () != 0 && m_bChanged)
    {

// don't echo ridiculous amounts of text

    if (strCurrent.GetLength () > iLimit)
      {
      strCurrent = strCurrent.Left (iLimit);
      strCurrent += " ...";
      }

// don't echo ridiculous amounts of text

    if (strReplacement.GetLength () > iLimit)
      {
      strReplacement = strReplacement.Left (iLimit);
      strReplacement += " ...";
      }

    CString strMsg;
    strMsg = TFormat ("Replace your typing of\n\n\"%s\"\n\nwith\n\n\"%s\"?",
                   (LPCTSTR) strCurrent, (LPCTSTR) strReplacement);
    if (::UMessageBox (strMsg, MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2)
        == IDCANCEL)
      {
        m_iHistoryStatus = eAtBottom;   // we are still at bottom therefore
        m_HistoryPosition = NULL;
        return true;
      }

    // preserve what we are about to delete, if requested to do so

    if (pDoc->m_bSaveDeletedCommand)
      {
      CString str = GetText (GetEditCtrl());

    // do not record null commands, or ones identical to the previous one

      if (!str.IsEmpty () && str != m_last_command)
        {
        if (m_inputcount >= pDoc->m_nHistoryLines)
          {
          m_msgList.RemoveHead ();   // keep max of "m_nHistoryLines" previous commands
          m_HistoryFindInfo.m_nCurrentLine--;     // adjust for a "find again"
          if (m_HistoryFindInfo.m_nCurrentLine < 0)
            m_HistoryFindInfo.m_nCurrentLine = 0;
          }
        else
          m_inputcount++;
        m_msgList.AddTail (str);
        m_last_command = str;
        }
      }

    }

  return false;
  }

void CSendView::DoFind (bool bAgain)
  {

CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

pDoc->m_DisplayFindInfo.m_bAgain = bAgain;

bool found = FindRoutine (pDoc,                    // passed back to callback routines
                          pDoc->m_DisplayFindInfo, // finding structure
                          InitiateSearch,          // how to re-initiate a find
                          GetNextLine);            // get the next line
	

if (found)
  {

// because find now finds batches of lines, we must work out which line it is 
// really on, so we highlight the correct columns

  POSITION pos = pDoc->GetLinePosition (pDoc->m_DisplayFindInfo.m_nCurrentLine);
  POSITION prevpos = NULL;
  CLine * pLine;

// select the found text, so it is highlighted

  m_topview->m_selstart_line = pDoc->m_DisplayFindInfo.m_nCurrentLine;
  m_topview->m_selend_line = pDoc->m_DisplayFindInfo.m_nCurrentLine;
  m_topview->m_selstart_col =  pDoc->m_DisplayFindInfo.m_iStartColumn;
  m_topview->m_selend_col = pDoc->m_DisplayFindInfo.m_iEndColumn;

  pLine = pDoc->m_LineList.GetPrev (pos);
  while (pos)
   {
   prevpos = pos;   // remember line which did have a hard return
   pLine = pDoc->m_LineList.GetPrev (pos);
   if (pLine->hard_return)
     break;
   m_topview->m_selstart_line--;
   m_topview->m_selend_line--;
   }

 // if prevpos is non-null it is now the position of the last line with a hard return
 // so, get the next one, that is the one which starts *our* sequence


   if (prevpos)
      pDoc->m_LineList.GetNext (prevpos);
   else       // must be the only line in the buffer
      prevpos = pDoc->m_LineList.GetHeadPosition ();

  pos = prevpos;
  while (pos)
   {
   pLine = pDoc->m_LineList.GetNext (pos);
   if (m_topview->m_selstart_col < pLine->len)
     break;
   m_topview->m_selstart_col -= pLine->len;
   m_topview->m_selend_col -= pLine->len;
   m_topview->m_selstart_line ++;
   m_topview->m_selend_line ++;
   }

  // if selendcol is > line length, selection must extend over multiple lines

  if (m_topview->m_selend_col > pLine->len)
    {       
    while (pos)
     {
     // first subtract out the line we were on (and subsequent ones)
     m_topview->m_selend_col -= pLine->len;
     m_topview->m_selend_line ++;

     // now check out the next ones
     pLine = pDoc->m_LineList.GetNext (pos);
     if (m_topview->m_selend_col < pLine->len)
       break;
     }
    }

  // make sure selection visible

  m_topview->EnsureSelectionVisible ();

  }   // end of being found

  // Invalidate new selection rectangle
  m_topview->Invalidate ();

  } // end of CSendView::DoFind


void CSendView::InitiateSearch (const CObject * pObject,
                                CFindInfo & FindInfo)
  {
CMUSHclientDoc* pDoc = (CMUSHclientDoc*) pObject;

  FindInfo.m_nTotalLines = pDoc->m_LineList.GetCount ();

  if (FindInfo.m_bAgain)
    FindInfo.m_pFindPosition = pDoc->GetLinePosition (FindInfo.m_nCurrentLine);
  else
    if (FindInfo.m_bForwards)
      FindInfo.m_pFindPosition = pDoc->m_LineList.GetHeadPosition ();
    else
      FindInfo.m_pFindPosition = pDoc->m_LineList.GetTailPosition ();

  } // end of CSendView::InitiateSearch

bool CSendView::GetNextLine (const CObject * pObject,
                             CFindInfo & FindInfo, 
                             CString & strLine)
  {
CMUSHclientDoc* pDoc = (CMUSHclientDoc*) pObject;

CLine * pLine;
POSITION prevpos = NULL;

  if (FindInfo.m_pFindPosition == NULL)
    return true;

  // if doing backwards, we must go back a whole *line* (ie. the one after a hard return)
  if (!FindInfo.m_bForwards)
    {
    pLine = pDoc->m_LineList.GetPrev (FindInfo.m_pFindPosition);
    while (FindInfo.m_pFindPosition)
     {
     prevpos = FindInfo.m_pFindPosition;   // remember line which did have a hard return
     pLine = pDoc->m_LineList.GetPrev (FindInfo.m_pFindPosition);
     if (pLine->hard_return)
       break;
     }

   // if prevpos is non-null it is now the position of the last line with a hard return
   // so, get the next one, that is the one which starts *our* sequence

     if (prevpos)
        pDoc->m_LineList.GetNext (prevpos);
     else       // must be the only line in the buffer
        prevpos = pDoc->m_LineList.GetHeadPosition ();

// sequence is now at the start of the batch of lines

     FindInfo.m_pFindPosition = prevpos;
    }

  strLine.Empty ();

  // get lines until a hard return

  while (FindInfo.m_pFindPosition)
    {
    pLine = pDoc->m_LineList.GetNext (FindInfo.m_pFindPosition);   // get next line
    strLine += CString (pLine->text, pLine->len);
    if (FindInfo.m_bForwards)
      FindInfo.m_nCurrentLine++;
    else
      FindInfo.m_nCurrentLine--;
    if (pLine->hard_return)
      break;
    }

  // adjust current line (main find loop adds/subtracts one anyway)
  if (FindInfo.m_bForwards)
    FindInfo.m_nCurrentLine--;
  else
    {
    FindInfo.m_pFindPosition = prevpos;
    pDoc->m_LineList.GetPrev (FindInfo.m_pFindPosition);  // go back to line prior to this batch
    FindInfo.m_nCurrentLine++;
    }

  return false;
  } // end of CSendView::GetNextLine

void CSendView::OnDisplayFind() 
{
  DoFind (false);
}     // end of CSendView::OnDisplayFind

void CSendView::OnDisplayFindagain() 
{
  DoFind (true);
}

void CSendView::OnFilePrintWorld() 
{
  m_topview->PrintWorld ();

}

void CSendView::OnDestroy() 
{

	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
		
  CWindowPlacement wp;
  wp.Save (CString (pDoc->m_mush_name + " World Position"), m_owner_frame);

  int cyCur,
      cyMin;
   
  m_owner_frame->m_wndSplitter.GetRowInfo (OUTPUT_PANE, cyCur, cyMin);
  App.db_write_int ("worlds", (LPCTSTR) CFormat ("%s:%s", (LPCTSTR) pDoc->m_mush_name, "Top Height"), cyCur);

  m_owner_frame->m_wndSplitter.GetRowInfo (COMMAND_PANE, cyCur, cyMin);
  App.db_write_int ("worlds", (LPCTSTR) CFormat ("%s:%s", (LPCTSTR) pDoc->m_mush_name, "Bottom Height"), cyCur);
//  TRACE1 ("Bottom height (saved) = %i\n", cyCur);

	CEditView::OnDestroy();
	
}

void CSendView::OnFindAgainBackwards() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

pDoc->m_DisplayFindInfo.m_bForwards = false;
DoFind(true);
}

void CSendView::OnUpdateFindAgainBackwards(CCmdUI* pCmdUI) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);
	
pCmdUI->Enable (!pDoc->m_DisplayFindInfo.m_strFindStringList.IsEmpty ());
}

void CSendView::OnFindAgainForwards() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

pDoc->m_DisplayFindInfo.m_bForwards = true;
DoFind(true);
}

void CSendView::OnUpdateFindAgainForwards(CCmdUI* pCmdUI) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

pCmdUI->Enable (!pDoc->m_DisplayFindInfo.m_strFindStringList.IsEmpty ());
}

void CSendView::OnProcessEscape() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);
	
  if (pDoc->m_bEscapeDeletesInput)
    {

    // preserve what we are about to delete, if requested to do so

    if (pDoc->m_bSaveDeletedCommand)
      {
      CString str = GetText (GetEditCtrl());

    // do not record null commands, or ones identical to the previous one

      if (!str.IsEmpty () && str != m_last_command)
        {
        if (m_inputcount >= pDoc->m_nHistoryLines)
          {
          m_msgList.RemoveHead ();   // keep max of "m_nHistoryLines" previous commands
          m_HistoryFindInfo.m_nCurrentLine--;     // adjust for a "find again"
          if (m_HistoryFindInfo.m_nCurrentLine < 0)
            m_HistoryFindInfo.m_nCurrentLine = 0;
          }
        else
          m_inputcount++;
        m_msgList.AddTail (str);
        m_last_command = str;
        }
      }

  	GetEditCtrl().SetWindowText ("");
    OnChange ();
    }

}


BOOL CSendView::PreTranslateMessage(MSG* pMsg) 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

    if (pMsg->message == WM_KEYDOWN &&  // keydown message
        pMsg->wParam == VK_UP &&        // up-arrow key
        pDoc->m_bArrowsChangeHistory && // and they want to traverse through history
        GetKeyState (VK_CONTROL) >= 0)      // and control key not pressed
      {
      int nStartChar;
      int nEndChar;

		  CString strText;
		  GetEditCtrl().GetWindowText(strText);

      TRACE ("PreTranslateMessage - OnKeysPrevcommand\n");

      // only traverse commands if we are at the *start* or *end* of the command
      GetEditCtrl().GetSel(nStartChar, nEndChar);	
      if ((nStartChar == 0 && nEndChar == 0) ||
          (nStartChar == strText.GetLength () && nEndChar == strText.GetLength ()) )
        {
        if (GetKeyState (VK_MENU) < 0)  // alt+up-arrow
          OnAltUparrow ();
        else
          OnKeysPrevcommand ();         // just up-arrow
        return true;
        }
      }   // end of VK_UP

    if (pMsg->message == WM_KEYDOWN &&  // keydown message
        pMsg->wParam == VK_DOWN &&      // down-arrow key
        pDoc->m_bArrowsChangeHistory && // and they want to traverse through history
        GetKeyState (VK_CONTROL) >= 0)      // and control key not pressed
      {
      int nStartChar;
      int nEndChar;

		  CString strText;
		  GetEditCtrl().GetWindowText(strText);

      TRACE ("PreTranslateMessage - OnKeysNextcommand\n");

      // only traverse commands if we are at the *start* or *end* of the command
      GetEditCtrl().GetSel(nStartChar, nEndChar);	
      if ((nStartChar == 0 && nEndChar == 0) ||
          (nStartChar == strText.GetLength () && nEndChar == strText.GetLength ()) )
        {
        if (GetKeyState (VK_MENU) < 0)  // alt+down-arrow
          OnAltDownarrow ();
        else
          OnKeysNextcommand ();   // just down-arrow
        return true;
        }
      } // end of VK_DOWN


//    TRACE2 ("Message = %i, param = %i\n", pMsg->message, pMsg->wParam);

  // F1 becomes a macro if wanted
 if ( pMsg->message == WM_KEYDOWN && App.m_bF1macro ) 
   { 
   map<long, WORD>::const_iterator it;

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
     }  // end of VK_F1

   }    // end of WM_KEYDOWN &&  m_bF1macro

  return CEditView::PreTranslateMessage(pMsg);

}

void CSendView::OnRepeatLastCommand() 
{
	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

  POSITION pos = m_msgList.GetTailPosition ();   // if at bottom, get it
  CString strText;

  if (!pos)    
    return;

   strText = m_msgList.GetAt (pos);    // if found, get its text

  SendCommand (strText, TRUE);
	
}

void CSendView::OnUpdateRepeatLastCommand(CCmdUI* pCmdUI) 
{
pCmdUI->Enable (!m_msgList.IsEmpty ());
}

void CSendView::OnAltDownarrow() 
{
	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

  TRACE ("OnAltDownarrow\n");

  // no history? no next command
  if (m_msgList.IsEmpty ())
    return;

  CString strText;    // history text
  CString strCommand = GetText (GetEditCtrl());

  // m_strPartialCommand is the command we are matching on for
  // alt+arrow matching
  // it only is set if they have *typed something*
     
  bool bAtTop = false;

  if (m_bChanged)
    {
    m_strPartialCommand = strCommand;
    bAtTop = true;
    TRACE1 ("NEW m_strPartialCommand = '%s'\n", (LPCTSTR) m_strPartialCommand);
    }

  
  TRACE1 ("m_strPartialCommand = '%s'\n", (LPCTSTR) m_strPartialCommand);

// if they want to do thing the old-fashioned way (ie. just use alt-arrow to 
//  traverse the history window), do it for them

  if ((!pDoc->m_bAltArrowRecallsPartial &&
       !pDoc->m_bArrowRecallsPartial) || 
      m_strPartialCommand.IsEmpty ())
    {
    OnKeysNextcommand ();
    return;
    }

// traverse history, keeping going until the history matches the entered text

  do
    {
   
    if (bAtTop)
      {
      m_HistoryPosition = m_msgList.GetHeadPosition (); // back to top
      bAtTop = false;
      }
    else
      if (m_HistoryPosition)
        m_msgList.GetNext (m_HistoryPosition);

    // if position is NULL, we have reached the bottom - echo a blank command
    // and discard the partial command so an ordinary arrow will work now
    if (!m_HistoryPosition) 
      {
      strText.Empty ();
      m_strPartialCommand.Empty ();
      break;
      }

    // get next command
    strText = m_msgList.GetAt (m_HistoryPosition);    

    // don't accept an exact match (see forum subject 2313
    // they won't want to recall exactly what they typed :)
    } while (m_strPartialCommand.CompareNoCase 
             (strText.Left (m_strPartialCommand.GetLength ())) != 0 ||
             m_strPartialCommand.CompareNoCase (strText) == 0
             );

  SetCommand (strText);

    
}   // end of OnAltDownarrow

void CSendView::OnAltUparrow() 
{
	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

  TRACE ("OnAltUparrow\n");

  // no history? no previous command
  if (m_msgList.IsEmpty ())
    return;

  CString strText;    // history text
  CString strCommand = GetText (GetEditCtrl());

  // m_strPartialCommand is the command we are matching on for
  // alt+arrow matching
  // it only is set if they have *typed something*

  bool bAtBottom = false;

  if (m_bChanged)
    {
    m_strPartialCommand = strCommand;
    bAtBottom = true;
    TRACE1 ("NEW m_strPartialCommand = '%s'\n", (LPCTSTR) m_strPartialCommand);
    }

  
  TRACE1 ("m_strPartialCommand = '%s'\n", (LPCTSTR) m_strPartialCommand);

// if they want to do thing the old-fashioned way (ie. just use alt-arrow to 
//  traverse the history window), do it for them

  if ((!pDoc->m_bAltArrowRecallsPartial &&
       !pDoc->m_bArrowRecallsPartial) || 
      m_strPartialCommand.IsEmpty ())
    {
    OnKeysPrevcommand ();
    return;
    }

// traverse history, keeping going until the history matches the entered text

  do
    {
   
    if (bAtBottom)
      {
      m_HistoryPosition = m_msgList.GetTailPosition (); // back to bottom
      bAtBottom = false;
      }
    else
      if (m_HistoryPosition)
        m_msgList.GetPrev (m_HistoryPosition);

    // if position is NULL, we have reached the top - echo a blank command
    // and discard the partial command so an ordinary arrow will work now
    if (!m_HistoryPosition) 
      {
      strText.Empty ();
      m_strPartialCommand.Empty ();
      break;
      }

    // get previous command
    strText = m_msgList.GetAt (m_HistoryPosition);    

    // don't accept an exact match (see forum subject 2313
    // they won't want to recall exactly what they typed :)
    } while (m_strPartialCommand.CompareNoCase 
             (strText.Left (m_strPartialCommand.GetLength ())) != 0 ||
             m_strPartialCommand.CompareNoCase (strText) == 0
             );

  SetCommand (strText);
 
  }

void CSendView::OnEditSelectAll() 
{
  GetEditCtrl().SetSel (0, -1);   // select all
}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 */
bool str_prefix (const char *astr, const char *bstr)
{
    if ( !astr )
	    return true;

    if ( !bstr )
	    return true;

    if (*astr == 0)
      return true;

    for ( ; *astr; astr++, bstr++ )
    {
	  if ( tolower(*astr) != tolower(*bstr) )
	      return true;
    }

    return false;
}

bool CSendView::TabCompleteOneLine (CMUSHclientDoc* pDoc,
                                    const int nStartChar,
                                    const int nEndChar,
                                    const CString & strWord, 
                                    const CString & strLine)
  {
const char * p = strLine;

while (*p)
  {
// skip leading non-alpha/numeric
  while (*p && !isalnum (*p))
    p++;

  if (*p == 0)
    break;

  if (str_prefix (strWord, p))
    {
    // no match - skip until the end of the word
    while (*p && isalnum (*p))
      p++;

    } // end of no match
  else
    { // match on desired word
    const char * p1 = p;
    int len = 0;
    // find end of word, assuming word consists of letters or numbers

    while (!isspace ((unsigned char) *p1) && strchr (App.m_strWordDelimiters, *p1) == NULL)
      p1++, len++;

    // don't match on same length word
    if (len > strWord.GetLength () )
      {
      // build up replacement word
      CString sReplacement = CString (p, len);
      // make lower case if wanted
      if (pDoc->m_bLowerCaseTabCompletion)
        sReplacement.MakeLower ();

      CPlugin * pSavedPlugin = pDoc->m_CurrentPlugin;

      // tell each plugin what we are doing
      for (POSITION pluginpos = pDoc->m_PluginList.GetHeadPosition(); pluginpos; )
        {
        CPlugin * pPlugin = pDoc->m_PluginList.GetNext (pluginpos);


        if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
          continue;

        // see what the plugin makes of this,
        pPlugin->ExecutePluginScript (ON_PLUGIN_TABCOMPLETE,
                                      sReplacement,  // input and output line
                                      pPlugin->m_dispid_plugin_tabcomplete); 
        }   // end of doing each plugin

      pDoc->m_CurrentPlugin = pSavedPlugin;

      // stop flicker
      LockWindowUpdate ();
      // select the characters already typed
      GetEditCtrl().SetSel (nStartChar, nEndChar);
      UnlockWindowUpdate ();
      // replace it, allow undos
      if (pDoc->m_bTabCompletionSpace)
        sReplacement = sReplacement + " ";

      GetEditCtrl().ReplaceSel (sReplacement, TRUE);
      // done
      return true;    // done it!
      }
    else
      p = p1;   // skip this word
    }   // end of match

  }   // end of scanning line for words

  return false;   // no match

  } // end of CSendView::TabCompleteOneLine 

void CSendView::OnKeysTab() 
{
	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

int nStartChar;
int nEndChar;
CString strCurrent;

//save old config
bool old_bTabCompletionSpace = pDoc->m_bTabCompletionSpace;

  // find where cursor is
  
  GetEditCtrl().GetSel(nStartChar, nEndChar);	

  // don't bother if something selected

  if (nStartChar != nEndChar)
    return;

  // get current text

  GetEditCtrl().GetWindowText (strCurrent);

  // don't bother if nothing typed

  if (strCurrent.IsEmpty ())
    return;

  // ignore if not at end of line or on a delimiter

  if (nEndChar < strCurrent.GetLength ())
    {
    unsigned char c = strCurrent [nEndChar]; // what is the next character?
    if (!isspace (c) && strchr (App.m_strWordDelimiters, c) == NULL)
      pDoc->m_bTabCompletionSpace = true;
    }
  
  // ignore if at start of line

  if (nEndChar <= 0)
    {
    pDoc->m_bTabCompletionSpace = old_bTabCompletionSpace;
    return;
    }

  // search backwards for another delimiter
  for (nStartChar = nEndChar - 1; nStartChar >= 0; nStartChar--)
    {
    unsigned char c = strCurrent [nStartChar]; // what is the next character?
    if (isspace (c) || strchr (App.m_strWordDelimiters, c) != NULL)
      break;
    }

  // move forwards past delimiter

  nStartChar++;

  // ignore if left of cursor is a delimiter too

  if (nStartChar == nEndChar)
    {
    pDoc->m_bTabCompletionSpace = old_bTabCompletionSpace;
    return;
    }

  CString sWord = strCurrent.Mid (nStartChar, nEndChar - nStartChar);

  sWord.MakeLower ();

  // we are getting somewhere now ...
  
  // search for that word in the default tab completion list

  if (TabCompleteOneLine (pDoc, nStartChar, nEndChar, sWord, 
                          pDoc->m_strTabCompletionDefaults))
    {
    pDoc->m_bTabCompletionSpace = old_bTabCompletionSpace;
    return;   // found it - ignore output buffer
    }


  // search for that word in the output buffer

  int iCount = 0;

  for (POSITION pos = pDoc->m_LineList.GetTailPosition (); pos; )
    {
    CLine * pLine = pDoc->m_LineList.GetPrev (pos);
    CString strLine = CString (pLine->text, pLine->len);

    if (++iCount > pDoc->m_iTabCompletionLines)
      break;    // hit line limit
    if (TabCompleteOneLine (pDoc, nStartChar, nEndChar, sWord, strLine))
      {
      pDoc->m_bTabCompletionSpace = old_bTabCompletionSpace;
      return;  // stop when we succeed
      }
    }   // end of scanning each line

  pDoc->m_bTabCompletionSpace = old_bTabCompletionSpace;
  return;

}

void CSendView::OnDisplayClearCommandHistory() 
{
  // check they really want to
  if (::UMessageBox 
      (TFormat ("Are you SURE you want to clear all %i commands you have typed?",
                   m_msgList.GetCount ()),
                    MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES)
                    return;

  // OK, do it ...
	m_msgList.RemoveAll ();
  m_HistoryPosition = NULL;
  m_inputcount = 0;
  m_HistoryFindInfo.m_pFindPosition = NULL;
  m_HistoryFindInfo.m_nCurrentLine = 0;
  m_HistoryFindInfo.m_bAgain = FALSE;  
  m_strPartialCommand.Empty ();
  m_last_command.Empty ();
  
  }

void CSendView::OnUpdateDisplayClearCommandHistory(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable (!m_msgList.IsEmpty ());	
}


void CSendView::OnDisplayGotobookmark() 
{
  m_topview->doGotoBookmark ();	
}

void CSendView::OnDisplayBookmarkselection() 
{
  m_topview->doBookmark ();	
}

void CSendView::OnInputGlobalchange() 
{
int nStartChar;
int nEndChar;
CString strCurrent;
  // get current text

  GetEditCtrl().GetWindowText (strCurrent);
  GetEditCtrl().GetSel(nStartChar, nEndChar);	
	
  CGlobalChangeDlg dlg;

  // use last entry they entered
  dlg.m_strChangeFrom = m_strChangeFrom;
  dlg.m_strChangeTo = m_strChangeTo;

  if (dlg.DoModal () != IDOK)
    return;

  // save for next time
  m_strChangeFrom = dlg.m_strChangeFrom;
  m_strChangeTo =   dlg.m_strChangeTo;


  // if they are replacing a selection - just get that bit
  if (nEndChar > nStartChar)
    strCurrent = strCurrent.Mid (nStartChar, nEndChar - nStartChar);

  // fix up special characters

  // version 3.65 - better handling of backslash
  // without this something like \\n becomes treated as \ \n

  strCurrent.Replace ("\r", "");  // get rid of carriage-returns

  // from
  dlg.m_strChangeFrom.Replace ("\\\\", "\x01"); // temporarily make \\ something else
  dlg.m_strChangeFrom.Replace ("\\n", "\n");
  dlg.m_strChangeFrom.Replace ("\\t", "\t");
  dlg.m_strChangeFrom.Replace ("\x01", "\\");   // put backslashes back

  //to
  dlg.m_strChangeTo.Replace ("\\\\", "\x01");   // temporarily make \\ something else
  dlg.m_strChangeTo.Replace ("\\n", "\n");
  dlg.m_strChangeTo.Replace ("\\t", "\t");
  dlg.m_strChangeTo.Replace ("\x01", "\\");     // put backslashes back

  CString strNew = Replace (strCurrent, 
                            dlg.m_strChangeFrom, 
                            dlg.m_strChangeTo);  // replace all

  if (strNew == strCurrent)
    {
    ::UMessageBox (TFormat ("No replacements made for \"%s\".", (LPCTSTR) m_strChangeFrom), 
                    MB_ICONINFORMATION);
    return;
    }

  strNew.Replace ("\n", ENDLINE); // put carriage-returns back

  // if nothing selected, select all
  if (nEndChar <= nStartChar)
    GetEditCtrl().SetSel (0, -1);   // select all

  // replace existing - can undo
  GetEditCtrl().ReplaceSel (strNew, TRUE);

}

void CSendView::OnUpdateInputGlobalchange(CCmdUI* pCmdUI) 
{
	
CString strCurrent;

  GetEditCtrl().GetWindowText (strCurrent);
	
  // they can do a global change if not empty
  pCmdUI->Enable (!strCurrent.IsEmpty ());
	
}

void CSendView::OnEditSpellcheck() 
{
	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

    Frame.SetStatusMessageNow (Translate ("Spell check ..."));
    App.SpellCheck (this, &GetEditCtrl());
    pDoc->ShowStatusLine ();
}

void CSendView::OnUpdateEditSpellcheck(CCmdUI* pCmdUI) 
{
CString strCurrent;

  GetEditCtrl().GetWindowText (strCurrent);
	
  // they can do a spell check if not empty
  pCmdUI->Enable (!strCurrent.IsEmpty () && App.m_bSpellCheckOK);
	
}

void CSendView::OnEditGeneratecharactername() 
{
	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

  CString strName = GenerateName ();

  if (strName.IsEmpty ())
    return;   // no names.txt file?

CGeneratedNameDlg dlg;

  dlg.m_strName = strName;
  dlg.m_pDoc = pDoc;
  dlg.m_strFileName = App.db_get_string  
    ("prefs", "DefaultNameGenerationFile", "names.txt");
  
  dlg.DoModal ();
	
}

void CSendView::OnEditNotesworkarea() 
{
	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

  // see if they want an existing one
  if (pDoc->SwitchToNotepad ())
    return;

int nStartChar;
int nEndChar;
CString strCurrent;
  // get current text

  GetEditCtrl().GetWindowText (strCurrent);
  GetEditCtrl().GetSel(nStartChar, nEndChar);	

  // if they are replacing a selection - just get that bit
  if (nEndChar > nStartChar)
    strCurrent = strCurrent.Mid (nStartChar, nEndChar - nStartChar);

  // edit current input window
  CreateTextWindow (strCurrent,     // selection
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



void CSendView::OnDisplayGotoline() 
{
  m_topview->doGotoLine ();	
}   // end of CSendView::OnDisplayGotoline

void CSendView::DoCommandHistory() 
{
OnGameCommandhistory ();
}   // end of CSendView::DoCommandHistory


void CSendView::DoPreviousCommand ()
  {
  OnKeysPrevcommand ();
  }  // end of CSendView::DoPreviousCommand

void CSendView::DoNextCommand ()
  {
  OnKeysNextcommand ();
  }  // end of CSendView::DoNextCommand

void CSendView::OnFileCtrlN() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

  if (pDoc->m_bCtrlNGoesToNextCommand)
    DoNextCommand ();
  else
    App.OnFileNew ();	
	
}

void CSendView::OnFileCtrlP() 
{
	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

  if (pDoc->m_bCtrlPGoesToPreviousCommand)
    OnKeysPrevcommand ();
  else
    m_topview->PrintWorld ();
	
}  // end of CSendView::OnFileCtrlP

void CSendView::OnEditCtrlZ() 
{
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

  if (pDoc->m_bCtrlZGoesToEndOfBuffer)
     OnTestEnd ();
  else
	   GetEditCtrl().Undo ();
}   // end of CSendView::OnEditCtrlZ


void CSendView::AddToCommandHistory (const CString & strCommand)
  {
CMUSHclientDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);

// do not record null commands, or ones identical to the previous one
// also do not record if the server has suppressed echoes

  if (!strCommand.IsEmpty () && 
      strCommand != m_last_command &&
      !(pDoc->m_bNoEcho && !pDoc->m_bAlwaysRecordCommandHistory)) 
    {
    if (m_inputcount >= pDoc->m_nHistoryLines)
      {
      m_msgList.RemoveHead ();   // keep max of "m_nHistoryLines" previous commands
      m_HistoryFindInfo.m_nCurrentLine--;     // adjust for a "find again"
      if (m_HistoryFindInfo.m_nCurrentLine < 0)
        m_HistoryFindInfo.m_nCurrentLine = 0;
      }
    else
      m_inputcount++;
    m_msgList.AddTail (strCommand);
    m_last_command = strCommand;
    }

  // history starts at bottom of list again - especially as we may have discarded lines
  m_HistoryPosition = NULL;
  m_iHistoryStatus = eAtBottom;

  } // end of  CSendView::AddToCommandHistory 

void CSendView::OnRepeatLastWord() 
{
// can't, if no previous command
if (m_msgList.IsEmpty ())
  return;

CString strLine = m_msgList.GetTail ();

  strLine.TrimRight ();

int iPos = strLine.ReverseFind (' ');

CString strWord = strLine.Mid (iPos + 1);

GetEditCtrl().ReplaceSel (strWord, TRUE);

}

void CSendView::OnLButtonDown(UINT nFlags, CPoint point) 
{

  if (GetKeyState (VK_CONTROL) < 0)  // ctrl+click
    {
  	ClientToScreen(&point);
    m_topview->ScreenToClient (&point);
    m_topview->AliasMenu (point);
	  return;
    } // end of ctrl+click
	
	CEditView::OnLButtonDown(nFlags, point);
}

void CSendView::OnCommandHome() 
{
  GetEditCtrl().SetSel (0, 0);
	
}

void CSendView::OnCommandEnd() 
{
  int len = GetWindowTextLength ();
  GetEditCtrl().SetSel (len, len);
	
}

void CSendView::OnMXPMenu (UINT nID)
  {
  m_topview->OnMXPMenu (nID);
  }


// see Josuttis: page 211
template <class K, class V>
class value_equals {
    private: 
       V value;
    public:
        // constructor
      value_equals (const  V& v) : value (v) {}
      // comparison
      bool operator () (pair<const K, V> elem)
      {
        return elem.second == value;
      }
};

void CSendView::OnAcceleratorCommand (UINT nID)
  {
	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

  string sCommand = pDoc->m_CommandToMacroMap [nID];

  // ignore empty macros
  if (sCommand.empty ())
    return;

// turn auto-say off, they obviously don't want to say west, QUIT, etc.

  BOOL bSavedAutoSay = pDoc->m_bEnableAutoSay;
  pDoc->m_bEnableAutoSay = FALSE;


  pDoc->m_iCurrentActionSource = eUserAccelerator;

  // for backwards compatability, call the same thing as before
  if (pDoc->m_CommandToSendToMap [nID] == eSendToExecute)
    SendCommand (sCommand.c_str (), TRUE, FALSE); // save previous, don't keep in history
  else
  {

    // new in 4.27 - allow sending to other places

    // I want to find the accelerator key name - need a linear search unfortunately
    // need to iterate over the command map, looking for a match on the ID we got
    map<long, WORD>::const_iterator it;
    it = find_if (pDoc->m_AcceleratorToCommandMap.begin (), 
                  pDoc->m_AcceleratorToCommandMap.end (),
                  value_equals<long, WORD>(nID));

    CString key = "unknown key";  // in case not found
    if (it != pDoc->m_AcceleratorToCommandMap.end ())
      key = KeyCodeToString (it->first >> 16, it->first);

    CString strExtraOutput;
    pDoc->m_iCurrentActionSource = eUserAccelerator;  

    // ok let's do it now
    pDoc->SendTo (pDoc->m_CommandToSendToMap [nID], 
            sCommand.c_str (), 
            TRUE,     // omit from output
            TRUE,     // omit from log
            TFormat ("Accelerator: %s", (LPCTSTR) key),
            "",       // Variable name
            strExtraOutput  // if we sent to output, it goes here
            );

    // display any stuff sent to output window

    if (!strExtraOutput.IsEmpty ())
       pDoc->DisplayMsg (strExtraOutput, strExtraOutput.GetLength (), COMMENT);

  }

  pDoc->m_iCurrentActionSource = eUnknownActionSource;

// restore auto-say

  pDoc->m_bEnableAutoSay = bSavedAutoSay;

  }


BOOL CSendView::OnMacroCommand(UINT nIDC) 
  {

int iIndex = -1;

  switch (nIDC)
    {
    case ID_MACRO_F2          : iIndex = MAC_F2    ; break;
    case ID_MACRO_F3          : iIndex = MAC_F3    ; break;
    case ID_MACRO_F4          : iIndex = MAC_F4    ; break;
    case ID_MACRO_F5          : iIndex = MAC_F5    ; break;
    case ID_MACRO_F7          : iIndex = MAC_F7    ; break;
    case ID_MACRO_F8          : iIndex = MAC_F8    ; break;
    case ID_MACRO_F9          : iIndex = MAC_F9    ; break;
    case ID_MACRO_F10         : iIndex = MAC_F10   ; break;
    case ID_MACRO_F11         : iIndex = MAC_F11   ; break;
    case ID_MACRO_F12         : iIndex = MAC_F12   ; break;

    case ID_MACRO_CTRL_F2          : iIndex = MAC_CTRL_F2    ; break;
    case ID_MACRO_CTRL_F3          : iIndex = MAC_CTRL_F3    ; break;
//    case ID_MACRO_CTRL_F4          : iIndex = MAC_CTRL_F4    ; break;
    case ID_MACRO_CTRL_F5          : iIndex = MAC_CTRL_F5    ; break;
    case ID_MACRO_CTRL_F7          : iIndex = MAC_CTRL_F7    ; break;
    case ID_MACRO_CTRL_F8          : iIndex = MAC_CTRL_F8    ; break;
    case ID_MACRO_CTRL_F9          : iIndex = MAC_CTRL_F9    ; break;
    case ID_MACRO_CTRL_F10         : iIndex = MAC_CTRL_F10   ; break;
    case ID_MACRO_CTRL_F11         : iIndex = MAC_CTRL_F11   ; break;
    case ID_MACRO_CTRL_F12         : iIndex = MAC_CTRL_F12   ; break;

    case ID_MACRO_SHIFT_F2          : iIndex = MAC_SHIFT_F2    ; break;
    case ID_MACRO_SHIFT_F3          : iIndex = MAC_SHIFT_F3    ; break;
    case ID_MACRO_SHIFT_F4          : iIndex = MAC_SHIFT_F4    ; break;
    case ID_MACRO_SHIFT_F5          : iIndex = MAC_SHIFT_F5    ; break;
    case ID_MACRO_SHIFT_F7          : iIndex = MAC_SHIFT_F7    ; break;
    case ID_MACRO_SHIFT_F8          : iIndex = MAC_SHIFT_F8    ; break;
    case ID_MACRO_SHIFT_F9          : iIndex = MAC_SHIFT_F9    ; break;
    case ID_MACRO_SHIFT_F10         : iIndex = MAC_SHIFT_F10   ; break;
    case ID_MACRO_SHIFT_F11         : iIndex = MAC_SHIFT_F11   ; break;
    case ID_MACRO_SHIFT_F12         : iIndex = MAC_SHIFT_F12   ; break;

    // new ones in 3.42

    case ID_ALT_A                   : iIndex = MAC_ALT_A    ; break;
    case ID_ALT_B                   : iIndex = MAC_ALT_B    ; break;
    case ID_ALT_J                   : iIndex = MAC_ALT_J    ; break;
    case ID_ALT_K                   : iIndex = MAC_ALT_K    ; break;
    case ID_ALT_L                   : iIndex = MAC_ALT_L    ; break;
    case ID_ALT_M                   : iIndex = MAC_ALT_M    ; break;
    case ID_ALT_N                   : iIndex = MAC_ALT_N    ; break;
    case ID_ALT_O                   : iIndex = MAC_ALT_O    ; break;
    case ID_ALT_P                   : iIndex = MAC_ALT_P    ; break;
    case ID_ALT_Q                   : iIndex = MAC_ALT_Q    ; break;
    case ID_ALT_R                   : iIndex = MAC_ALT_R    ; break;
    case ID_ALT_S                   : iIndex = MAC_ALT_S    ; break;
    case ID_ALT_T                   : iIndex = MAC_ALT_T    ; break;
    case ID_ALT_U                   : iIndex = MAC_ALT_U    ; break;
    case ID_ALT_X                   : iIndex = MAC_ALT_X    ; break;
    case ID_ALT_Y                   : iIndex = MAC_ALT_Y    ; break;
    case ID_ALT_Z                   : iIndex = MAC_ALT_Z    ; break;
                 
      
    // new ones in 3.42

    case ID_MACRO_F1                : iIndex = MAC_F1       ; break;
    case ID_MACRO_CTRL_F1           : iIndex = MAC_CTRL_F1  ; break;
    case ID_MACRO_SHIFT_F1          : iIndex = MAC_SHIFT_F1 ; break;
    case ID_MACRO_F6                : iIndex = MAC_F6       ; break;
    case ID_MACRO_CTRL_F6           : iIndex = MAC_CTRL_F6  ; break;
    case ID_MACRO_SHIFT_F6          : iIndex = MAC_SHIFT_F6 ; break;


    } // end of switch
         
  // if command found
  if (iIndex != -1)
    SendMacro (iIndex);

  return TRUE;

  } // end of CSendView::OnMacroCommand

void CSendView::OnEditGotomatchingbrace() 
{
  FindMatchingBrace (GetEditCtrl(), false);  // find without selecting range
	
}

void CSendView::OnEditSelecttomatchingbrace() 
{
  FindMatchingBrace (GetEditCtrl(), true);  // find with selecting range
	
}

void CSendView::OnInitMenu(CMenu* pMenu)
  {
  }

void CSendView::OnInitMenuPopup(CMenu* pMenu, UINT nIndex, BOOL bSysMenu)
  {
  }

void CSendView::OnDisplayHighlightphrase() 
{
  m_topview->OnDisplayHighlightphrase ();

}

void CSendView::OnDisplayMultilinetrigger() 
{
  m_topview->OnDisplayMultilinetrigger ();
	
}

BOOL CSendView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
CRect rect;
	
  m_topview->GetClientRect (&rect);
  m_topview->ClientToScreen(&rect);

  // if mouse is over upper window (output window) then let it scroll instead
  if (rect.PtInRect (pt))
    return m_topview->OnMouseWheel (nFlags, zDelta, pt);
    
	return CEditView::OnMouseWheel(nFlags, zDelta, pt);
}

void CSendView::OnGameFunctionslist() 
{
	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

int nStartChar,
    nEndChar;
CString strSelection;

  // find the selection range
  GetEditCtrl().GetSel(nStartChar, nEndChar);
  // get window text
  GetEditCtrl().GetWindowText (strSelection);

  bool bLua = false;
  
  if (pDoc->GetScriptEngine () && pDoc->GetScriptEngine ()->L)
    bLua = true;

  ShowFunctionslist (strSelection, nStartChar, nEndChar, bLua);
	
}

void CSendView::OnCompleteFunction() 
{
	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

  bool bLua = false;
  
  if (pDoc->GetScriptEngine () && pDoc->GetScriptEngine ()->L)
    bLua = true;

  FunctionMenu (GetEditCtrl(), bLua, &pDoc->m_ExtraShiftTabCompleteItems, pDoc->m_bTabCompleteFunctions);
}

void CSendView::OnUpdateCompleteFunction(CCmdUI* pCmdUI) 
{
pCmdUI->Enable ();
}



void CSendView::OnWindowMaximize() 
{
	  GetParentFrame ()->ShowWindow(SW_SHOWMAXIMIZED);
	
}

void CSendView::OnWindowRestore() 
{
	  GetParentFrame ()->ShowWindow(SW_RESTORE);
	
}

void CSendView::OnSysCommand(UINT nID, LPARAM lParam)
{
	/* DefWindowProc sends this message to the focused window when it processes
	 * a WM_SYSCHAR. In other words we end up here when the user presses ALT+f,
	 * etc. If we pass this message to DefWindowProc, the corresponding popup menu
	 * will be opened. We supress this behaviour if DisableKeyboardMenuActivation 
	 * is enabled. See also the analogous handler in CMainFrame which stops the 
	 * menu bar being focused when ALT and F10 are released. */
	if (nID == SC_KEYMENU && App.m_bDisableKeyboardMenuActivation) {
		/* Discard. */
	} else
		CEditView::OnSysCommand(nID, lParam);
}


// auto resizing of command window based on amount of content
void CSendView::AdjustCommandWindowSize (void)
  {
	CMUSHclientDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

  // do nothing if not wanted
  if (!pDoc->m_bAutoResizeCommandWindow)
    return;

  // find how many lines in command window
  int iLines = GetEditCtrl().GetLineCount ();

  // if too small, make the minimum
  if (iLines < pDoc->m_iAutoResizeMinimumLines)
    iLines = pDoc->m_iAutoResizeMinimumLines;

  CRect rect;
  GetClientRect (&rect);

  if (iLines > pDoc->m_iAutoResizeMaximumLines)
    {
    // if very large, let them manage it (that is, if they have already resized it to be larger than the maximum)
    if ((rect.bottom - rect.top) > (pDoc->m_InputFontHeight * pDoc->m_iAutoResizeMaximumLines + 4))
      return;

    // otherwise, take the maximum
    iLines = pDoc->m_iAutoResizeMaximumLines;
    }

  // resize - seem to need an extra 4 pixels or things go a bit strange
  pDoc->SetCommandWindowHeight (pDoc->m_InputFontHeight * iLines + 4);

  }

// if we have a selection here, drop the selection in the output window
void CSendView::CheckForSelectionChange (void)
  {
  int nStartChar;
  int nEndChar;

  GetEditCtrl().GetSel(nStartChar, nEndChar);	

  // no selection? leave alone
  if (nStartChar == nEndChar)
    return;

  // no selection already? don't do anything else
  if (!(m_topview->m_selend_line > m_topview->m_selstart_line || 
              (m_topview->m_selend_line == m_topview->m_selstart_line && 
               m_topview->m_selend_col > m_topview->m_selstart_col)))
    return;

  // we have a selection, so cancel top view's selection
  m_topview->m_selstart_line = 0;  
  m_topview->m_selstart_col = 0;
  m_topview->m_selend_line = 0;
  m_topview->m_selend_col = 0;

  m_topview->Invalidate ();

  }   // end of  CSendView::CheckForSelectionChange

void CSendView::CancelSelection (void)
  {

  int nStartChar;
  int nEndChar;

  GetEditCtrl().GetSel(nStartChar, nEndChar);	

  // no selection? leave alone
  if (nStartChar == nEndChar)
    return;

  // put cursor at end of previous selection
  GetEditCtrl().SetSel (nEndChar, nEndChar);

  } // end of CSendView::CancelSelection


void CSendView::OnEditCopy()
{

  // if top view does NOT has a selection, copy from here
  if (!(m_topview->m_selend_line > m_topview->m_selstart_line || 
              (m_topview->m_selend_line == m_topview->m_selstart_line && 
               m_topview->m_selend_col > m_topview->m_selstart_col)))
    CEditView::OnEditCopy ();
  else
    m_topview->OnEditCopy ();    // otherwise copy from there

}

void CSendView::OnUpdateNeedSel(CCmdUI* pCmdUI)
{

  // if top view does NOT has a selection, enable copy in the usual way
  if (!(m_topview->m_selend_line > m_topview->m_selstart_line || 
              (m_topview->m_selend_line == m_topview->m_selstart_line && 
               m_topview->m_selend_col > m_topview->m_selstart_col)))
    CEditView::OnUpdateNeedSel (pCmdUI);
  else
	  pCmdUI->Enable(TRUE);   // we have a selection in the top view, enable copying

}

void CSendView::OnUpdateNeedOutputSel(CCmdUI* pCmdUI)
{

  pCmdUI->Enable(m_topview->m_selend_line > m_topview->m_selstart_line || 
              (m_topview->m_selend_line == m_topview->m_selstart_line && 
               m_topview->m_selend_col > m_topview->m_selstart_col));

}


void CSendView::OnDisplayTextattributes() 
  {
  m_topview->OnDisplayTextattributes ();
  } // end of CSendView::OnDisplayTextattributes() 

void CSendView::OnEditCopyashtml() 
  {
  m_topview->OnEditCopyashtml ();
  } // end of CSendView::OnEditCopyashtml() 

