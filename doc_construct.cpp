// doc_construct.cpp : CMUSHclientDoc constructor/destructor
//

#include "stdafx.h"
#include "MUSHclient.h"
#include "mainfrm.h"

#include "doc.h"
#include "genprint.h"
#include "dialogs\QuickConnectDlg.h"
#include "UDPsocket.h"

extern int gdoccount;

/////////////////////////////////////////////////////////////////////////////
// CMUSHclientDoc construction/destruction

CMUSHclientDoc::CMUSHclientDoc()
  :	m_eventScriptFileChanged(FALSE, TRUE)

{    // constructor

int i;

  // each document will have a unique number
  m_iUniqueDocumentNumber = App.GetUniqueNumber ();

  AllocateConfigurationArrays ();

  m_bTrace = false;   // not tracing yet

// note - these initialisations merely make the hash lookup more efficient.
// until v 3.21 they were the default of 20, so this should help somewhat
// The number should be a prime number, hence the strange figures.

  m_VariableMap.InitHashTable (997); // allow for 1000 variables (at least)
  m_AliasMap.InitHashTable (293);    // probably won't have many more than 300 aliases
  m_TriggerMap.InitHashTable (293);  // probably won't have many more than 300 triggers
  m_TimerMap.InitHashTable (293);    // probably won't have many more than 300 timers

  SetDefaults (false);        // set up numeric/boolean defaults
  SetAlphaDefaults (false);   // set up alpha defaults

  m_strWorldID = GetUniqueID ();      // default world ID

  m_CurrentPlugin = NULL;     // no plugin active right now

  m_iBackgroundMode = 0;
  m_iForegroundMode = 0;
  m_iBackgroundColour = NO_COLOUR;

  m_TextRectangle = CRect (0, 0, 0, 0);
  m_TextRectangleBorderOffset = 0;           
  m_TextRectangleBorderColour = 0;           
  m_TextRectangleBorderWidth = 0;            
  m_TextRectangleOutsideFillColour = 0;      
  m_TextRectangleOutsideFillStyle = 0;   

  m_iCurrentActionSource = eUnknownActionSource;
  m_nBytesIn = 0;
  m_nBytesOut = 0;
  m_bTabCompleteFunctions = true;
  m_hSpecialFontHandle = 0;

  m_iTriggersEvaluatedCount = 0; 
  m_iTriggersMatchedCount = 0;   
  m_iAliasesEvaluatedCount = 0;  
  m_iAliasesMatchedCount = 0;    
  m_iTimersFiredCount = 0;       
  m_iTriggersMatchedThisSessionCount = 0;   
  m_iAliasesMatchedThisSessionCount = 0;    
  m_iTimersFiredThisSessionCount = 0;       

  m_bLoaded = false;
  m_bMapping = false;
  m_bRemoveMapReverses = true;
  m_MapFailureRegexp = NULL;
  m_bPluginProcessingCommand = false;
  m_bPluginProcessingSend = false;
  m_bPluginProcessingSent = false;
  m_iLastCommandCount = 0;
  m_iExecutionDepth = 0;
  m_iNextChatID = 0;
  m_tLastMessageTime = 0;
  m_tLastGroupMessageTime = 0;
  m_bOmitFromCommandHistory = false;
  m_bUTF_8 = false;
  m_bWorldClosing = false;
  m_bInSendToScript = true;

  m_bInPlaySoundFilePlugin = false;
  m_bInCancelSoundFilePlugin = false;

  m_strStatusMessage = Translate ("Ready");
  m_bShowingMapperStatus = false;
  m_iMCCP_type = 0;
  m_bSupports_MCCP_2 = false;
  m_bNoEcho = false;
  m_bDebugIncomingPackets = false;
  m_iInputPacketCount = 0;
  m_iOutputPacketCount = 0;
  m_iUTF8ErrorCount = 0;
  m_lastGoTo = 1;
  m_bNAWS_wanted = false;
  m_bCHARSET_wanted = false;
  m_bSuppressNewline = false;
  m_echo_colour = 9;
  m_bAlwaysRecordCommandHistory = false;
  m_bDoNotShowOutstandingLines = false;
  m_bDoNotTranslateIACtoIACIAC = false;
  m_bAutoResizeCommandWindow = false;
  m_iAutoResizeMinimumLines = 1;   
  m_iAutoResizeMaximumLines = 20;   
  m_bDoNotAddMacrosToCommandHistory = false;
  m_bCopySelectionToClipboard = false;
  m_bCarriageReturnClearsLine = false;
  m_bConvertGAtoNewline = false;
  m_bSendMXP_AFK_Response = true;
  m_bMudCanChangeOptions = true;
//  m_bShowGridLinesInListViews = true;  // NB - app-wide :P
  m_bEnableSpamPrevention = false;
  m_strSpamMessage = "look";
  m_iSpamLineCount = 20;
  m_iLastOutgoingChatPort = DEFAULT_CHAT_PORT;

  m_iSocksProcessing = 0;   // no socks processing wanted
  m_iProxyServerPort = 1080; // default socks port

  // deprecated options  - set to zero for saving comparison (for XML testing)

  m_page_colour = 0;
  m_whisper_colour= 0;
  m_mail_colour = 0;
  m_game_colour = 0;
  m_chat_colour = 0;
  m_remove_channels1 = 0;
  m_remove_channels2 = 0;
  m_remove_pages= 0;
  m_remove_whispers = 0;
  m_remove_set= 0;
  m_remove_mail = 0;
  m_remove_game = 0;

  // things I found to be not initialised in version 3.43

  m_timestamps = 0;
  m_iFlags1 = 0;
  m_iFlags2 = 0;
  m_iListMode = 0;
  m_iListCount = 0;
  m_iMXP_previousMode = eMXP_open;
  m_code = 0;

  // sound buffers
  for (i = 0; i < MAX_SOUND_BUFFERS; i++)
    m_pDirectSoundSecondaryBuffer [i] = NULL;

  /*---------------------------------------------------------
  I want to be able to time speed walking delays (for pulling things
  out of the list, so I need a window that can take timer events, however
  a CDocument isn't a window. Thus, I'll make a hidden window for this
  specific purpose.

  I previously had speed walking tied to the CSendView window but this was
  pretty crappy, because you could have multiple send views, thus the timers
  would be all over the shop, and it was very fiddly when firing a "send" from
  a trigger, because a trigger is not related to a particular send view.

  --------------------------------------------------------- */


  // create window that we can use for firing timers
  m_pTimerWnd = new CTimerWnd (this);

	m_pTimerWnd->m_hWnd = NULL;
	if (!m_pTimerWnd->CreateEx(0, AfxRegisterWndClass(0),
		_T("Document timer window"),
		WS_OVERLAPPED, 0, 0, 0, 0, NULL, NULL))
	  {
		TRACE0("Warning: unable to create document timer window!\n");
		AfxThrowResourceException();
  	}
	ASSERT(m_pTimerWnd->m_hWnd != NULL);
	ASSERT(CWnd::FromHandlePermanent(m_pTimerWnd->m_hWnd) == m_pTimerWnd);


  for (i = 0; i < 8; i++)  
    m_font [i] = NULL;
  m_input_font = NULL;
  m_FontHeight = 0;
  m_FontWidth = 0;
  m_InputFontHeight = 0;
  m_InputFontWidth = 0;
  m_total_lines = 0;
  m_last_line_with_IAC_GA = 0;
  m_nTotalLinesSent = 0;
  m_nTotalLinesReceived = 0;
  m_phase = NONE;
  m_version = VERSION;

  m_bVariablesChanged = false;

  m_view_number = 0;
  m_LastFlushTime = CTime::GetCurrentTime();

  m_bCompress = false;    // no compression yet
  m_bCompressInitOK = false;
  m_CompressInput = NULL;
  m_CompressOutput = NULL;

  // initialise zlib decompression 

  m_zCompress.next_in = Z_NULL; 
  m_zCompress.avail_in = 0; 
  m_zCompress.zalloc = Z_NULL; 
  m_zCompress.zfree = Z_NULL; 
  m_zCompress.opaque = Z_NULL; 
  m_nTotalUncompressed = 0;
  m_nTotalCompressed = 0;
  m_iCompressionTimeTaken = 0;

  // we will defer initialising zlib until we really have to

	m_pSocket = NULL;
  m_pChatListenSocket = NULL;    // no listening socket
  m_logfile = NULL;
  m_match_width = 30;

  m_bSelected = true;

  m_bNotesInRGB = false;    // notes are in palette colour right now
  m_iNoteStyle = NORMAL;    // notes are not bold right now

// MXP

  m_bMXP = false;   // no MXP yet
  m_bPuebloActive = false; // no Pueblo yet either <evil grin>
  m_bInParagraph = false;
  m_bMXP_script = false;
  m_bPreMode = false;
  m_iMXP_defaultMode = eMXP_open;
  m_iMXP_mode = m_iMXP_defaultMode;
  m_cLastChar = 0;

  // initial styles

  m_iFlags = 0;
  m_iForeColour = WHITE;
  m_iBackColour = BLACK;

  if (m_bCustom16isDefaultColour)
    {
    m_iForeColour = 15;   // use custom colour 16
    m_iBackColour = 15;
    m_iFlags |= COLOUR_CUSTOM;
    }

//  m_normalcolour [BLACK]    = RGB (192, 192, 192) | PALETTE;    // black   
//  m_normalcolour [RED]      = RGB (128,   0,  64) | PALETTE;    // red     
//  m_normalcolour [GREEN]    = RGB (128, 255, 128) | PALETTE;    // green   
//  m_normalcolour [YELLOW]   = RGB (255, 255, 128) | PALETTE;    // yellow  
//  m_normalcolour [BLUE]     = RGB (  0, 255, 255) | PALETTE;    // blue    
//  m_normalcolour [MAGENTA]  = RGB (255,   0, 128) | PALETTE;    // magenta 
//  m_normalcolour [CYAN]     = RGB (128, 255, 255) | PALETTE;    // cyan    
//  m_normalcolour [WHITE]    = RGB (  0, 128,   0) | PALETTE;    // white   

//  m_boldcolour [BLACK]      = RGB (  0,   0,   0) |  PALETTE;    // black   
//  m_boldcolour [RED]        = RGB (255,   0,   0) |  PALETTE;    // red     
//  m_boldcolour [GREEN]      = RGB (  0, 255,   0) |  PALETTE;    // green   
//  m_boldcolour [YELLOW]     = RGB (255, 255,   0) |  PALETTE;    // yellow  
//  m_boldcolour [BLUE]       = RGB (  0,  64, 128) |  PALETTE;    // blue    
//  m_boldcolour [MAGENTA]    = RGB (255,   0, 255) |  PALETTE;    // magenta 
//  m_boldcolour [CYAN]       = RGB (  0, 255, 255) |  PALETTE;    // cyan    
//  m_boldcolour [WHITE]      = RGB (255, 255, 255) |  PALETTE;    // white   

  // better use ANSI colours as the default

  SetDefaultAnsiColours (m_normalcolour, m_boldcolour);

  // and some nice custom colours
  SetDefaultCustomColours (m_customtext, m_customback);

  // default custom colour names
  for (i = 0; i < NUMITEMS (m_strCustomColourName); i++)
    m_strCustomColourName [i].Format ("Custom%i", i + 1);

  for (i = 0; i < NUMITEMS (m_macro_type); i++)
    m_macro_type [i] = SEND_NOW;

// printer styles for printing from the screen

  for (i = 0; i < 8; i++)
    {
     m_nNormalPrintStyle [i] = 0;
     m_nBoldPrintStyle [i] = FONT_BOLD;
    }

  m_bUseDefaultOutputFont = !App.m_strDefaultOutputFont.IsEmpty (); 

  m_bUseDefaultColours   = !App.m_strDefaultColoursFile.IsEmpty ();   
  m_bUseDefaultTriggers  = !App.m_strDefaultTriggersFile.IsEmpty ();   
  m_bUseDefaultAliases   = !App.m_strDefaultAliasesFile.IsEmpty ();    
  m_bUseDefaultMacros    = !App.m_strDefaultMacrosFile.IsEmpty ();     
  m_bUseDefaultTimers    = !App.m_strDefaultTimersFile.IsEmpty ();     
  m_bUseDefaultInputFont = !App.m_strDefaultInputFont.IsEmpty ();  
  
  setupstrings ();

  m_tStatusTime = CTime::GetCurrentTime(); 
  m_tConnectTime = CTime::GetCurrentTime();
  m_tLastPlayerInput = CTime::GetCurrentTime();
  m_timeScriptFileMod = 0;

  m_tsConnectDuration = CTimeSpan (0, 0, 0, 0);

  m_iConnectPhase = eConnectNotConnected;
                                      
  gdoccount++;

  m_pActiveCommandView = NULL;
  m_pActiveOutputView = NULL;

  m_new_lines = 0;

  m_last_prefs_page = 1;

  App.m_bUpdateActivity = TRUE;

// for looking up host names

  ZeroMemory (&m_sockAddr, sizeof m_sockAddr);
  ZeroMemory (&m_ProxyAddr, sizeof m_ProxyAddr);
  m_hNameLookup = NULL;
  m_pGetHostStruct = NULL;
  m_pLinePositions = NULL;

  m_pCurrentLine = NULL;
  m_total_lines = 0;

  // set up the dialog box headings for the finding dialogs
  m_DisplayFindInfo.m_strTitle    = "Find in output buffer...";
  m_MacrosFindInfo.m_strTitle     = "Find macro...";
  m_AliasesFindInfo.m_strTitle    = "Find alias...";
  m_TriggersFindInfo.m_strTitle   = "Find trigger...";
  m_TimersFindInfo.m_strTitle     = "Find timer...";
  m_VariablesFindInfo.m_strTitle  = "Find variable...";
  m_NotesFindInfo.m_strTitle  = "Find in notes...";
  m_RecallFindInfo.m_strTitle    = "Recall...";
          
  m_bRecallCommands = true;
  m_bRecallOutput = true;
  m_bRecallNotes = true;

  // set up column counts for use in the loading and saving of the column orders etc.
  m_MacrosFindInfo.m_iControlColumns    = CPrefsP6::eColumnCount;
  m_AliasesFindInfo.m_iControlColumns   = CPrefsP7::eColumnCount;
  m_TriggersFindInfo.m_iControlColumns  = CPrefsP8::eColumnCount;
  m_TimersFindInfo.m_iControlColumns    = CPrefsP16::eColumnCount;
  m_VariablesFindInfo.m_iControlColumns = CPrefsP18::eColumnCount;

  m_bDisconnectOK = true;    // so we don't try to reconnect to prematurely
  
  m_iMXPerrors = 0;     
  m_iMXPtags = 0;       
  m_iMXPentities = 0;   

  // scripting support

	EnableAutomation();     // not needed?

  if (!bWine)
  	AfxOleLockApp();        // not needed?

  m_ScriptEngine = NULL;

  m_bInScriptFileChanged = false;
  m_pThread = NULL;
  m_bSyntaxErrorOnly = false;

  m_dispidWorldOpen = DISPID_UNKNOWN;
  m_dispidWorldClose = DISPID_UNKNOWN;
  m_dispidWorldSave = DISPID_UNKNOWN;
  m_dispidWorldConnect = DISPID_UNKNOWN;
  m_dispidWorldDisconnect = DISPID_UNKNOWN;
  m_dispidWorldGetFocus = DISPID_UNKNOWN;
  m_dispidWorldLoseFocus = DISPID_UNKNOWN;
  m_dispidOnMXP_Start = DISPID_UNKNOWN;        
  m_dispidOnMXP_Stop = DISPID_UNKNOWN;         
  m_dispidOnMXP_OpenTag = DISPID_UNKNOWN;      
  m_dispidOnMXP_CloseTag = DISPID_UNKNOWN;     
  m_dispidOnMXP_SetVariable = DISPID_UNKNOWN;  
  m_dispidOnMXP_Error = DISPID_UNKNOWN;    

  m_iScriptTimeTaken = 0;   // time taken on scripts

  m_bPluginProcessesOpenTag = false;    
  m_bPluginProcessesCloseTag = false;   
  m_bPluginProcessesSetVariable = false;
  m_bPluginProcessesSetEntity = false;
  m_bPluginProcessesError = false;      

  ZeroMemory (&m_bClient_IAC_DO, sizeof m_bClient_IAC_DO);
  ZeroMemory (&m_bClient_IAC_DONT, sizeof m_bClient_IAC_DONT);
  ZeroMemory (&m_bClient_IAC_WILL, sizeof m_bClient_IAC_WILL);
  ZeroMemory (&m_bClient_IAC_WONT, sizeof m_bClient_IAC_WONT);

  // set up some default triggers for MUSHes

  /*
  The parameter are:

  Trigger Name
  Match Text 
  Response Text 
  Flags 
  Colour 
  Wildcard to copy to clipboard
  Sound File Name 
  Script Name
  */

  /*  No, don't. It just confuses Diku MUD people

  AddTrigger ("Game_messages", "GAME:*", "", eEnabled, 0, 0, "", ""); 
  AddTrigger ("Page_poses", "From afar,*", "", eEnabled, 1, 0, "", ""); 
  AddTrigger ("Pages", "^\\w+ pages.*$", "", eEnabled | eTriggerRegularExpression, 1, 0, "", ""); 
  AddTrigger ("Whisper_poses", "You sense:*", "", eEnabled, 2, 0, "", ""); 
  AddTrigger ("Whispers", "^\\w+ whispers.*$", "", eEnabled | eTriggerRegularExpression, 2, 0, "", ""); 
  AddTrigger ("Mail_messages", "MAIL:*", "", eEnabled, 3, 0, "", ""); 
  AddTrigger ("Chat_channel_1", "^\\<.+?\\>.*$", "", eEnabled | eTriggerRegularExpression, 4, 0, "", ""); 
  AddTrigger ("Chat_channel_2", "^\\[.+?\\].*$", "", eEnabled | eTriggerRegularExpression, 4, 0, "", ""); 
  */

  m_nextAcceleratorCommand = ACCELERATOR_FIRST_COMMAND;
  m_accelerator = NULL;

}  // end of CMUSHclientDoc::CMUSHclientDoc()

#ifdef PANE
void closepane (pair<string, CPaneView *> item)
  {
  item.second->GetParentFrame ()->SendMessage (WM_CLOSE);
  }
#endif // PANE

CMUSHclientDoc::~CMUSHclientDoc()
{
int i;

  // stop sounds playing, release sound buffers
  for (i = 0; i < MAX_SOUND_BUFFERS; i++)
    if (m_pDirectSoundSecondaryBuffer [i])
      {
      DWORD iStatus;
      if (SUCCEEDED (m_pDirectSoundSecondaryBuffer [i]->GetStatus (&iStatus)) &&
          (iStatus & DSBSTATUS_PLAYING))
        m_pDirectSoundSecondaryBuffer [i]->Stop ();

      m_pDirectSoundSecondaryBuffer [i]->Release ();
      }

  if (m_pTimerWnd)
    {
	  m_pTimerWnd->DestroyWindow();
	  delete m_pTimerWnd;
    }

  for (i = 0; i < 8; i++)  
    delete m_font [i];
  delete m_input_font;

  if (m_hNameLookup)
    WSACancelAsyncRequest (m_hNameLookup);  // cancel host name lookup in progress

  delete [] m_pGetHostStruct;   // delete buffer used by host name lookup

  delete m_MapFailureRegexp;    // delete regexp structure for mapping failures

	if (m_pSocket)
	{

    ShutDownSocket (*m_pSocket);

    delete m_pSocket;
    m_pSocket = NULL;

    }

	if (m_pChatListenSocket)
	{

    ShutDownSocket (*m_pChatListenSocket);

    delete m_pChatListenSocket;
    m_pChatListenSocket = NULL;

    }

  // UDP listening sockets
  for (map<int, UDPsocket *>::iterator udpSocketIterator = m_UDPsocketMap.begin ();
       udpSocketIterator != m_UDPsocketMap.end ();
       udpSocketIterator++)
      delete udpSocketIterator->second;

// delete chat sessions

  DELETE_LIST (m_ChatList);

// delete plugins

  DELETE_LIST (m_PluginList);

  CloseLog ();    // this writes out the log file postamble as well

// delete triggers
       
  DELETE_MAP (m_TriggerMap, CTrigger); 

// delete aliass

  DELETE_MAP (m_AliasMap, CAlias); 

// delete lines list

  DELETE_LIST (m_LineList);

// delete timer map

  DELETE_MAP (m_TimerMap, CTimer); 
  
// delete variables map

  DELETE_MAP (m_VariableMap, CVariable); 

// delete Element map

  DELETE_MAP (m_CustomElementMap, CElement); 

// delete active tags list

  DELETE_LIST (m_ActiveTagList);

// delete actions list

  DELETE_LIST (m_ActionList);


// get rid of our positions array

  delete [] m_pLinePositions;

// one less document

  gdoccount--;

// update activity window

  App.m_bUpdateActivity = TRUE;

// ****************** release scripting stuff

  DisableScripting ();

  if (!bWine)
  	AfxOleUnlockApp();        // not needed?

  // free compression stuff

  if (m_CompressOutput)
    free (m_CompressOutput);
  if (m_CompressInput)
    free (m_CompressInput);

  // don't wrap up if not initialised
  if (m_bCompressInitOK)
    inflateEnd (&m_zCompress);

  // don't need to know what the configuration was any more
  DeleteConfigurationArrays ();

  // delete our arrays
  for (tStringMapOfMaps::iterator it = m_Arrays.begin (); 
       it != m_Arrays.end ();
       it++)
         {
         tStringToStringMap * m = it->second;
         m->clear ();
         delete m;
         }

  // destroy accelerator table, if we had one
  if (m_accelerator)
    DestroyAcceleratorTable (m_accelerator);

  // if they loaded a special font, get rid of it
  RemoveSpecialFont ();

#ifdef PANE

  // get rid of owned panes

  safe_for_each (m_PaneMap.begin (), m_PaneMap.end (), closepane);
#endif // PANE

  // delete MiniWindow map

  for (MiniWindowMapIterator mwit = m_MiniWindows.begin (); 
       mwit != m_MiniWindows.end ();
       mwit++)
         delete mwit->second;


  // delete databases
  for (tDatabaseMapIterator dbit = m_Databases.begin (); 
       dbit != m_Databases.end ();
       dbit++)
         {
         if (dbit->second->pStmt)        // finalize any outstanding statement
           sqlite3_finalize(dbit->second->pStmt);
         if (dbit->second->db)           // and close the database
           sqlite3_close(dbit->second->db);
         delete dbit->second;      // now delete memory used by it
         }

}


BOOL CMUSHclientDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

  SetDefaults (false);        // set up numeric/boolean defaults
  SetAlphaDefaults (false);   // set up alpha defaults

  if (App.m_TypeOfNewDocument == App.eNormalNewDocument) // only ask on normal new world
    {
    int i = ::TMessageBox ("Preload world defaults from an existing world?",
           MB_YESNOCANCEL | MB_ICONQUESTION);

    if (i == IDCANCEL)
      return FALSE;

    if (i == IDYES)
      while (TRUE)    // loop until successful read of file, or cancel
      {
	    // Get file
	    CString	strFilter(MAKEINTRESOURCE(IDS_OPEN_WORLD_FILTER));
	    CString	strTitle(MAKEINTRESOURCE(IDS_OPEN_WORLD_TITLE));
	    CFileDialog	dlg(TRUE,						// TRUE for FileOpen
					    NULL,						// default extension
					    NULL,						// no initial file name
					    OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
					    strFilter);
	    dlg.m_ofn.lpstrTitle = strTitle;

      // use default world file directory
      dlg.m_ofn.lpstrInitialDir = Make_Absolute_Path (App.m_strDefaultWorldFileDirectory);

      ChangeToFileBrowsingDirectory ();
      int nResult = dlg.DoModal();
      ChangeToStartupDirectory ();

      if (nResult != IDOK)
		    break;
	    CString	strFileName = dlg.GetPathName();

		  CWaitCursor	wait;

	    try
	      {

        // Open existing world file
		    CFile	fileWorld(strFileName, CFile::modeRead|CFile::shareDenyWrite);

        CArchive ar(&fileWorld, CArchive::load);

	      try
	        {
		      Serialize (ar);
	        }
	      catch(CException* e)
  	      {
          ::TMessageBox ("Unexpected file format - invalid world file", MB_ICONEXCLAMATION);
		      e->Delete();
          ar.Close();
          continue;
	        }

        ar.Close();

	      }
	    catch(CException* e)
  	    {
        ::TMessageBox ("Unable to open world file", MB_ICONEXCLAMATION);
		    e->Delete();
        continue;
	      }
    
  // if we got here, we successfully loaded the world!

      SetModifiedFlag ();
      m_strWorldID = GetUniqueID ();      // new world ID

      break;

      } // end of preloading defaults
  } // end of normal new world

   // if defaults are wanted, overwrite what we loaded with them :)
             
  if (m_bLoaded)
    OnFileReloaddefaults ();
  else
    {

    if (!App.m_strDefaultColoursFile.IsEmpty ())
      {
      m_bUseDefaultColours = true;
      Load_Set (COLOUR, App.m_strDefaultColoursFile, &Frame);
      }

    if (!App.m_strDefaultTriggersFile.IsEmpty ())
      {
      m_bUseDefaultTriggers = true;
      Load_Set (TRIGGER, App.m_strDefaultTriggersFile, &Frame);
      }

    if (!App.m_strDefaultAliasesFile.IsEmpty ())
      {
      m_bUseDefaultAliases = true;
      Load_Set (ALIAS, App.m_strDefaultAliasesFile, &Frame);
      }

    if (!App.m_strDefaultTimersFile.IsEmpty ())
      {
      m_bUseDefaultTimers = true;
      Load_Set (TIMER, App.m_strDefaultTimersFile, &Frame);
      }

    if (!App.m_strDefaultMacrosFile.IsEmpty ())
      {
      m_bUseDefaultMacros = true;
      Load_Set (MACRO, App.m_strDefaultMacrosFile, &Frame);
      }

    if (!App.m_strDefaultInputFont.IsEmpty ())
      {
      m_bUseDefaultInputFont = true;
      m_input_font_height = App.m_iDefaultInputFontHeight; 
      m_input_font_name = App.m_strDefaultInputFont;   
      m_input_font_italic = App.m_iDefaultInputFontItalic; 
      m_input_font_weight = App.m_iDefaultInputFontWeight;
      m_input_font_charset = App.m_iDefaultInputFontCharset;
      }   // end of input font override

    if (!App.m_strDefaultOutputFont.IsEmpty ())
      {
      m_bUseDefaultOutputFont = true;
      m_font_height = App.m_iDefaultOutputFontHeight; 
      m_font_name = App.m_strDefaultOutputFont;   
      m_font_weight = FW_NORMAL;
      m_font_charset = App.m_iDefaultOutputFontCharset;
    }   // end of output font override
    } // end of not loading an existing world

  m_bLoaded = false;    // not really loaded - effectively a new world

// get name and IP address, give up if cancelled

  if (App.m_TypeOfNewDocument == App.eQuickConnect) // pop up nice simple dialog box
    {
    CQuickConnectDlg dlg;
    dlg.m_iPort = 4000;
    dlg.m_strWorldName = "Untitled world";
    dlg.m_strAddress = "";
    if (dlg.DoModal () != IDOK)
      return FALSE;
    m_server = dlg.m_strAddress;
    m_port = dlg.m_iPort;
    m_mush_name = dlg.m_strWorldName;

    // save auto connect flag, then set to true, to make sure we connect
    unsigned int savebAutoConnectWorlds = App.m_bAutoConnectWorlds;
    App.m_bAutoConnectWorlds = TRUE;
    SetUpOutputWindow ();
    OpenSession ();
    App.m_bAutoConnectWorlds = savebAutoConnectWorlds;
    SetModifiedFlag ();
    return TRUE;
        
    } // end of quick connect

  if (App.m_TypeOfNewDocument == App.eTelnetFromNetscape) // just do it
    {
    // get rid of telnet stuff
    CString strCommandLine = ::Replace (App.m_lpCmdLine, "telnet://", "");
    int iSpace = strCommandLine.FindOneOf (" :");
    if (iSpace == -1)
      {
      m_server = strCommandLine;
      m_mush_name = strCommandLine;
      m_port = 23;
      }
    else
      {
      m_server = strCommandLine.Left (iSpace);
      m_mush_name = m_server;
      m_port = atoi (strCommandLine.Mid (iSpace + 1));
      }

    // save auto connect flag, then set to true, to make sure we connect
    unsigned int savebAutoConnectWorlds = App.m_bAutoConnectWorlds;
    App.m_bAutoConnectWorlds = TRUE;
    SetUpOutputWindow ();
    OpenSession ();
    App.m_bAutoConnectWorlds = savebAutoConnectWorlds;
    SetModifiedFlag ();
    return TRUE;
        
    } // end of telnet called from netscape navigator

  // we have to do this *before* getting the preferences
  SetUpOutputWindow ();

  if (!GamePreferences (ePageGeneral))
    return FALSE;

  if(m_mush_name.IsEmpty ())
    {
    ::TMessageBox("Your world name cannot be blank.", MB_ICONEXCLAMATION);
    return FALSE;
    }

  if(m_server.IsEmpty ())
    {
    ::TMessageBox("The world TCP/IP address cannot be blank.", MB_ICONEXCLAMATION);
    return FALSE;
    }


  OpenSession ();

  return TRUE;

  }
