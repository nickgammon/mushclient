#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "TreePropertySheet.h"
#include "MissingEntryPoints.h"
#include "..\..\mainfrm.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/*
  

  Configuration page numbers:
    
 1 - general
 2 - sounds   // NOT USED NOW
 3 - custom colours
 4 - logging
 5 - ANSI colours
 6 - macros
 7 - aliases
 8 - triggers
 9 - commands
10 - send
11 - notes
12 - keypad
13 - paste
14 - output
15 - info
16 - timers
17 - scripts
18 - variables
19 - auto - say
20 - printing
21 - connecting (name and password)
22 - MXP
23 - chat

  */


#include "..\..\doc.h"

#include "PrefsPropertySheet.h"

#include "..\..\genprint.h"

// for debug messages
static const char * sConfigPhase [] = {
    "undefined",        // 0 - error if this
    "general",  //             1 
    "sounds   ",  //           2 
    "custom colours",  //      3 
    "logging",  //             4 
    "ANSI colours",  //        5 
    "macros",  //              6 
    "aliases",  //             7 
    "triggers",  //            8 
    "commands",  //            9 
    "send",  //               10 
    "notes",  //              11 
    "keypad",  //             12 
    "paste",  //              13 
    "output",  //             14 
    "info",  //               15 
    "timers",  //             16 
    "scripts",  //            17 
    "variables",  //          18 
    "autosay",  //            19 
    "printing",  //           20 
    "connecting",  //         21 
    "MXP",  //                22 
    "chat",  //               23 
  };

static void DelayDebugMsg (const char * sPhase, int iWhich)
  {
  CString msg;
  if (iWhich < 1 || iWhich > NUMITEMS (sConfigPhase))
    iWhich = 0;
  msg.Format ("%s %s config", sPhase, sConfigPhase [iWhich]);
  Frame.DelayDebugStatus (msg);
  }

CString CMUSHclientDoc::AppendMUDName (const CString strName)
  {

  if (m_mush_name.IsEmpty ())
    return strName;
  else
    return strName + " - " + m_mush_name;

  } // end of CMUSHclientDoc::AppendMUDName

/* *******************************************************************
   load from document into property page
   *******************************************************************/

void CMUSHclientDoc::LoadPrefsP1  (CPrefsP1  &page1)
  {
  DelayDebugMsg ("Loading", 1);
  page1.m_doc = this;
  page1.m_port          = m_port;       
  page1.m_mush_name     = m_mush_name;  
  page1.m_server        = m_server;     
  page1.m_bSaveWorldAutomatically = m_bSaveWorldAutomatically;
  page1.m_strProxyServerName = m_strProxyServerName;
  page1.m_iProxyServerPort = m_iProxyServerPort;
  page1.m_iSocksProcessing = m_iSocksProcessing;
  page1.m_strProxyUserName = m_strProxyUserName;
  page1.m_strProxyPassword = m_strProxyPassword;
  } // end of CMUSHclientDoc::LoadPrefsP1

void CMUSHclientDoc:: LoadPrefsP2  (CPrefsP2  &page2)  
  {  
  DelayDebugMsg ("Loading", 2);
  page2.m_doc = this;

  }   // end of CMUSHclientDoc::LoadPrefsP2

void CMUSHclientDoc:: LoadPrefsP3  (CPrefsP3  &page3)  
  {  
  DelayDebugMsg ("Loading", 3);
  page3.m_doc = this;
  memcpy (page3.m_customtext, m_customtext, sizeof (page3.m_customtext));
  memcpy (page3.m_customback, m_customback, sizeof (page3.m_customback));
  for (int i = 0; i < MAX_CUSTOM; i++)
     page3.m_strCustomName [i] = m_strCustomColourName [i];

  }   // end of CMUSHclientDoc::LoadPrefsP3

void CMUSHclientDoc:: LoadPrefsP4  (CPrefsP4  &page4)  
  {  
  DelayDebugMsg ("Loading", 4);
//  Sleep (1000);
  page4.m_doc = this;
  page4.m_bLogOutput       = m_bLogOutput;
  page4.m_log_input        = m_log_input; 
  page4.m_bLogNotes        = m_bLogNotes;
  page4.m_bLogHTML         = m_bLogHTML;
  page4.m_strLogFilePreamble = m_strLogFilePreamble;
  page4.m_strLogFilePostamble = m_strLogFilePostamble;
  page4.m_strAutoLogFileName = m_strAutoLogFileName;
  page4.m_bWriteWorldNameToLog = m_bWriteWorldNameToLog;
  page4.m_strLogLinePreambleOutput = m_strLogLinePreambleOutput;
  page4.m_strLogLinePreambleInput = m_strLogLinePreambleInput;
  page4.m_strLogLinePreambleNotes = m_strLogLinePreambleNotes;
  page4.m_strLogLinePostambleOutput = m_strLogLinePostambleOutput;
  page4.m_strLogLinePostambleInput = m_strLogLinePostambleInput;
  page4.m_strLogLinePostambleNotes = m_strLogLinePostambleNotes;
  page4.m_bLogInColour = m_bLogInColour;
  page4.m_bLogRaw = m_bLogRaw;
  }   // end of CMUSHclientDoc::LoadPrefsP4

void CMUSHclientDoc:: LoadPrefsP5  (CPrefsP5  &page5)  
  {  
  DelayDebugMsg ("Loading", 5);
  page5.m_doc = this;
  memcpy (page5.m_normalcolour, m_normalcolour, sizeof (page5.m_normalcolour));
  memcpy (page5.m_boldcolour, m_boldcolour, sizeof (page5.m_boldcolour));
  page5.m_bUseDefaultColours = m_bUseDefaultColours;
  page5.m_bCustom16isDefaultColour = m_bCustom16isDefaultColour;
  }   // end of CMUSHclientDoc::LoadPrefsP5

void CMUSHclientDoc:: LoadPrefsP6  (CPrefsP6  &page6)  
  {  
  DelayDebugMsg ("Loading", 6);
int i;
  page6.m_doc = this;
  for (i = 0; i < NUMITEMS (m_macros); i++)
    {
    page6.m_macros [i] = m_macros [i];
    page6.m_macro_type [i] = m_macro_type [i];
    switch (m_macro_type [i])
      {
      case REPLACE_COMMAND: page6.m_macro_type_alpha [i] = "Replace";
                            break;
      case SEND_NOW:        page6.m_macro_type_alpha [i] = "Send now";
                            break;
      case ADD_TO_COMMAND:  page6.m_macro_type_alpha [i] = "Insert";
                            break;
      default:              page6.m_macro_type_alpha [i] = "Unknown";
                            break;
      } // end of switch
    }

  page6.m_pMacrosFindInfo = &m_MacrosFindInfo;    // for finding
  page6.m_bUseDefaultMacros = m_bUseDefaultMacros;

  }   // end of CMUSHclientDoc::LoadPrefsP6

void CMUSHclientDoc:: LoadPrefsP7  (CPrefsP7  &page7)  
  {  
  DelayDebugMsg ("Loading", 7);
  page7.m_doc = this;
	page7.m_enable_aliases = m_enable_aliases;
  page7.m_bUseDefaultAliases = m_bUseDefaultAliases;

  page7.SetUpPage ("alias",
                  (CObjectMap *) &m_AliasMap,
                  &page7.m_ctlAliasList,
                  &page7.m_ctlSummary,
                  page7.CompareObjects,
                  &m_AliasesFindInfo,
                  XML_ALIASES);

  page7.m_iColumnCount      = CPrefsP7::eColumnCount;
  page7.m_iColWidth         = new int [CPrefsP7::eColumnCount];
  page7.m_strColumnHeadings = new CString [CPrefsP7::eColumnCount];
  page7.m_iColJust         = new int [CPrefsP7::eColumnCount];

  page7.m_iColWidth [CPrefsP7::eColumnAlias] = 160;
  page7.m_iColWidth [CPrefsP7::eColumnSequence] = 40;
  page7.m_iColWidth [CPrefsP7::eColumnContents] = 150;    // will be modified in WM_INITDIALOG
  page7.m_iColWidth [CPrefsP7::eColumnLabel] = 50;    // will be modified in WM_INITDIALOG
  page7.m_iColWidth [CPrefsP7::eColumnGroup] = 50;    // will be modified in WM_INITDIALOG

  page7.m_iColJust [CPrefsP7::eColumnAlias] = LVCFMT_LEFT;
  page7.m_iColJust [CPrefsP8::eColumnSequence] = LVCFMT_RIGHT;
  page7.m_iColJust [CPrefsP7::eColumnContents] = LVCFMT_LEFT;  
  page7.m_iColJust [CPrefsP7::eColumnLabel] = LVCFMT_LEFT;  
  page7.m_iColJust [CPrefsP7::eColumnGroup] = LVCFMT_LEFT;  
  
  page7.m_strColumnHeadings [CPrefsP7::eColumnAlias] = TranslateHeading ("Alias");
  page7.m_strColumnHeadings [CPrefsP8::eColumnSequence] = TranslateHeading ("Seq");
  page7.m_strColumnHeadings [CPrefsP7::eColumnContents] = TranslateHeading ("Send");
  page7.m_strColumnHeadings [CPrefsP7::eColumnLabel] = TranslateHeading ("Label");
  page7.m_strColumnHeadings [CPrefsP7::eColumnGroup] = TranslateHeading ("Group");
  }   // end of CMUSHclientDoc::LoadPrefsP7

void CMUSHclientDoc:: LoadPrefsP8  (CPrefsP8  &page8)  
  {  
  DelayDebugMsg ("Loading", 8);
  page8.m_doc = this;
  page8.m_prefsp3 = NULL;   // so we can look up colours (in the document)
	page8.m_enable_triggers = m_enable_triggers;
	page8.m_enable_trigger_sounds = m_enable_trigger_sounds;
  page8.m_bUseDefaultTriggers = m_bUseDefaultTriggers;

  page8.SetUpPage ("trigger",
                  (CObjectMap *) &m_TriggerMap,
                  &page8.m_ctlTriggerList,
                  &page8.m_ctlSummary,
                  page8.CompareObjects,
                  &m_TriggersFindInfo,
                  XML_TRIGGERS);

  page8.m_iColumnCount      = CPrefsP8::eColumnCount;
  page8.m_iColWidth         = new int [CPrefsP8::eColumnCount];
  page8.m_strColumnHeadings = new CString [CPrefsP8::eColumnCount];
  page8.m_iColJust         = new int [CPrefsP8::eColumnCount];

  page8.m_iColWidth [CPrefsP8::eColumnTrigger] = 100;
  page8.m_iColWidth [CPrefsP8::eColumnSequence] = 40;
  page8.m_iColWidth [CPrefsP8::eColumnContents] = 240;
  page8.m_iColWidth [CPrefsP8::eColumnLabel] = 50;
  page8.m_iColWidth [CPrefsP8::eColumnGroup] = 50;

  page8.m_iColJust [CPrefsP8::eColumnTrigger] = LVCFMT_LEFT;
  page8.m_iColJust [CPrefsP8::eColumnSequence] = LVCFMT_RIGHT;
  page8.m_iColJust [CPrefsP8::eColumnContents] = LVCFMT_LEFT;
  page8.m_iColJust [CPrefsP8::eColumnLabel] = LVCFMT_LEFT;
  page8.m_iColJust [CPrefsP8::eColumnGroup] = LVCFMT_LEFT;

  page8.m_strColumnHeadings [CPrefsP8::eColumnTrigger] = TranslateHeading ("Trigger");
  page8.m_strColumnHeadings [CPrefsP8::eColumnSequence] = TranslateHeading ("Seq");
  page8.m_strColumnHeadings [CPrefsP8::eColumnContents] = TranslateHeading ("Send");
  page8.m_strColumnHeadings [CPrefsP8::eColumnLabel] = TranslateHeading ("Label");
  page8.m_strColumnHeadings [CPrefsP8::eColumnGroup] = TranslateHeading ("Group");
  }   // end of CMUSHclientDoc::LoadPrefsP8

void CMUSHclientDoc:: LoadPrefsP9  (CPrefsP9  &page9)  
  {  
  DelayDebugMsg ("Loading", 9);
  page9.m_doc = this;
  page9.m_prefsp3 = NULL;   // so we can look up colours (in the document)
	page9.m_command_stack_character = m_strCommandStackCharacter;
	page9.m_enable_command_stacking = m_enable_command_stack;
	page9.m_enable_speed_walk = m_enable_speed_walk;
	page9.m_speed_walk_prefix = m_speed_walk_prefix;
	page9.m_echo_colour = m_echo_colour == SAMECOLOUR ? 0 : m_echo_colour + 1;
	page9.m_echo_input = m_display_my_input;
	page9.m_nHistoryLines = m_nHistoryLines;
  page9.m_input_text_colour = m_input_text_colour;
  page9.m_input_background_colour = m_input_background_colour;
  page9.m_input_font_height = m_input_font_height;  
  page9.m_input_font_name =   m_input_font_name;    
  page9.m_input_font_italic = m_input_font_italic;  
  page9.m_input_font_weight = m_input_font_weight;
  page9.m_input_font_charset = m_input_font_charset;
  page9.m_strSpeedWalkFiller = m_strSpeedWalkFiller;
  page9.m_bAutoRepeat = m_bAutoRepeat;
  page9.m_bLowerCaseTabCompletion = m_bLowerCaseTabCompletion;
  page9.m_bTranslateGerman  = m_bTranslateGerman;
  page9.m_bTranslateBackslashSequences = m_bTranslateBackslashSequences;
  page9.m_strTabCompletionDefaults = m_strTabCompletionDefaults;
  page9.m_iTabCompletionLines = m_iTabCompletionLines;
  page9.m_bTabCompletionSpace = m_bTabCompletionSpace;
  page9.m_bSpellCheckOnSend = m_bSpellCheckOnSend; 
  page9.m_bUseDefaultInputFont = m_bUseDefaultInputFont;
  page9.m_bKeepCommandsOnSameLine = m_bKeepCommandsOnSameLine;
  page9.m_iSpeedWalkDelay = m_iSpeedWalkDelay;
  page9.m_bNoEchoOff = m_bNoEchoOff;
  page9.m_bEnableSpamPrevention  = m_bEnableSpamPrevention;
  page9.m_iSpamLineCount  = m_iSpamLineCount;
  page9.m_strSpamMessage  = m_strSpamMessage;

// for sub-dialog

  page9.m_bConfirmBeforeReplacingTyping = m_bConfirmBeforeReplacingTyping;
  page9.m_bEscapeDeletesInput           = m_bEscapeDeletesInput;  
  page9.m_bArrowsChangeHistory          = m_bArrowsChangeHistory; 
  page9.m_bAltArrowRecallsPartial       = m_bAltArrowRecallsPartial;
  page9.m_bArrowRecallsPartial          = m_bArrowRecallsPartial;
  page9.m_bDoubleClickInserts           = m_bDoubleClickInserts;
  page9.m_bDoubleClickSends             = m_bDoubleClickSends;
  page9.m_bSaveDeletedCommand           = m_bSaveDeletedCommand;
  page9.m_bArrowKeysWrap                = m_bArrowKeysWrap;
  page9.m_bCtrlZGoesToEndOfBuffer       = m_bCtrlZGoesToEndOfBuffer;
  page9.m_bCtrlPGoesToPreviousCommand   = m_bCtrlPGoesToPreviousCommand;
  page9.m_bCtrlNGoesToNextCommand       = m_bCtrlNGoesToNextCommand;

  CString strStyle = CFormat ("%i pt.", m_input_font_height);
  if (m_input_font_weight == FW_BOLD)
    strStyle += " bold";
  if (m_input_font_italic)
    strStyle += " italic";
                    
  page9.m_strInputStyle = strStyle;

  }   // end of CMUSHclientDoc::LoadPrefsP9

void CMUSHclientDoc:: LoadPrefsP10 (CPrefsP10 &page10)  
  {  
  DelayDebugMsg ("Loading", 10);
  page10.m_doc = this;
	page10.m_file_postamble  = m_file_postamble;
	page10.m_file_preamble   = m_file_preamble; 
	page10.m_line_postamble  = m_line_postamble;
	page10.m_line_preamble   = m_line_preamble; 
  page10.m_bConfirmOnSend      = m_bConfirmOnSend;
  page10.m_bCommentedSoftcode = m_bFileCommentedSoftcode;
  page10.m_iLineDelay = m_nFileDelay;
  page10.m_bEcho = m_bSendEcho;
  page10.m_nFileDelayPerLines = m_nFileDelayPerLines;
  }   // end of CMUSHclientDoc::LoadPrefsP10

void CMUSHclientDoc:: LoadPrefsP11 (CPrefsP11 &page11)  
  {  
  DelayDebugMsg ("Loading", 11);
  page11.m_doc = this;
	page11.m_notes   = m_notes; 
  page11.m_pNotesFindInfo = &m_NotesFindInfo;    // for finding
  }   // end of CMUSHclientDoc::LoadPrefsP11

void CMUSHclientDoc:: LoadPrefsP12 (CPrefsP12 &page12)  
  {  
  DelayDebugMsg ("Loading", 12);
  page12.m_doc = this;

  for (int i = 0; i < eKeypad_Max_Items; i++)
    page12.m_keypad [i] = m_keypad [i];

  page12.m_keypad_enable = m_keypad_enable;
  }   // end of CMUSHclientDoc::LoadPrefsP12

void CMUSHclientDoc:: LoadPrefsP13 (CPrefsP13 &page13)  
  {  
  DelayDebugMsg ("Loading", 13);
  page13.m_doc = this;
  page13.m_paste_postamble      = m_paste_postamble;
	page13.m_paste_preamble       = m_paste_preamble; 
	page13.m_pasteline_postamble  = m_pasteline_postamble;
	page13.m_pasteline_preamble   = m_pasteline_preamble; 
  page13.m_bConfirmOnPaste      = m_bConfirmOnPaste;
  page13.m_bCommentedSoftcode = m_bPasteCommentedSoftcode;
  page13.m_iLineDelay = m_nPasteDelay;
  page13.m_bEcho = m_bPasteEcho;
  page13.m_nPasteDelayPerLines = m_nPasteDelayPerLines;
  }   // end of CMUSHclientDoc::LoadPrefsP13

void CMUSHclientDoc:: LoadPrefsP14 (CPrefsP14 &page14)  
  {  
  DelayDebugMsg ("Loading", 14);
  page14.m_doc = this;
  page14.m_font_height = m_font_height;  
  page14.m_font_name =   m_font_name;    
  page14.m_font_weight = m_font_weight;
  page14.m_font_charset = m_font_charset;
  page14.m_wrap_output = m_wrap;
  page14.m_indent_paras = m_indent_paras;
  page14.m_enable_beeps = m_enable_beeps;
  page14.m_bUTF_8 = m_bUTF_8;
  page14.m_nLines = m_maxlines;
  page14.m_nWrapColumn = m_nWrapColumn;
  page14.m_bLineInformation = m_bLineInformation;
  page14.m_bStartPaused = m_bStartPaused;
  page14.m_bAutoFreeze = m_bAutoFreeze;
  page14.m_bShowBold = m_bShowBold;
  page14.m_bShowItalic = m_bShowItalic;
  page14.m_bShowUnderline = m_bShowUnderline;
  page14.m_iPixelOffset = m_iPixelOffset;
  page14.m_bDisableCompression = m_bDisableCompression;
  page14.m_bFlashIcon = m_bFlashIcon;
  page14.m_bUseDefaultOutputFont = m_bUseDefaultOutputFont;
  page14.m_bUnpauseOnSend = m_bUnpauseOnSend;
  page14.m_bAlternativeInverse = m_bAlternativeInverse;
  page14.m_bAutoWrapWindowWidth = m_bAutoWrapWindowWidth;
  page14.m_bNAWS = m_bNAWS;
  page14.m_bCarriageReturnClearsLine = m_bCarriageReturnClearsLine;
  page14.m_bConvertGAtoNewline = m_bConvertGAtoNewline;
  page14.m_strBeepSound = m_strBeepSound;
  page14.m_strTerminalIdentification = m_strTerminalIdentification;
  page14.m_bShowConnectDisconnect = m_bShowConnectDisconnect;
  page14.m_bCopySelectionToClipboard = m_bCopySelectionToClipboard;
  page14.m_bAutoCopyInHTML = m_bAutoCopyInHTML;
  page14.m_iLineSpacing = m_iLineSpacing;

  page14.m_nOldLines = m_maxlines;    // to see if they have changed them

  page14.m_strOutputStyle = CFormat ("%i pt.",m_font_height);

  page14.m_sound_pathname = m_new_activity_sound;


  }   // end of CMUSHclientDoc::LoadPrefsP14

void CMUSHclientDoc:: LoadPrefsP15 (CPrefsP15 &page15)  
  {  
  DelayDebugMsg ("Loading", 15);

  __int64 nTotalTriggers = 0,
          nTotalAliases = 0,
          nTotalTimers = 0;
  page15.m_doc = this;

  CTrigger * pTrigger;
  CAlias * pAlias;
  CTimer * pTimer;
  CString strName;
  POSITION pos;
  LONGLONG iTimeTaken = 0;
  CString strMessage;
  double   elapsed_time;

  // count number of triggers matched
  for (pos = m_TriggerMap.GetStartPosition(); pos; )
    {
    m_TriggerMap.GetNextAssoc (pos, strName, pTrigger);
    nTotalTriggers += pTrigger->nMatched;
    // calculate time taken to execute triggers
    if (pTrigger->regexp)
      iTimeTaken += pTrigger->regexp->iTimeTaken;
    }

  // count number of aliases matched
  for (pos = m_AliasMap.GetStartPosition(); pos; )
    {
    m_AliasMap.GetNextAssoc (pos, strName, pAlias);
    nTotalAliases += pAlias->nMatched;
    }

  // count number of timers fired
  for (pos = m_TimerMap.GetStartPosition(); pos; )
    {
    m_TimerMap.GetNextAssoc (pos, strName, pTimer);
    nTotalTimers += pTimer->nMatched;
    }

  page15.m_strBufferLines.Format ("%i / %ld", 
                                  m_LineList.GetCount (),
                                  m_maxlines);
  page15.m_strTriggers.Format ("%i   (%I64d matched)", 
                              m_TriggerMap.GetCount (),
                              nTotalTriggers);

  // time taken to execute triggers
  if (App.m_iCounterFrequency > 0)
    {
    elapsed_time = ((double) iTimeTaken) / 
                    ((double) App.m_iCounterFrequency);
    page15.m_strTriggerTimeTaken.Format ("%12.6f seconds.", elapsed_time);
    elapsed_time = ((double) m_iCompressionTimeTaken) / 
                    ((double) App.m_iCounterFrequency);
    if (m_iCompressionTimeTaken)
      page15.m_strTimeTakenCompressing.Format ("%12.6f seconds ", elapsed_time);
    }

  if (m_bCompress)
     page15.m_strTimeTakenCompressing += "(MCCP active)";
  else
     page15.m_strTimeTakenCompressing += "(MCCP not active)";

  page15.m_strAliases.Format ("%i   (%I64d used)", 
                              m_AliasMap.GetCount (),
                              nTotalAliases);
  page15.m_strTimers.Format ("%i   (%I64d fired)", 
                              m_TimerMap.GetCount (),
                              nTotalTimers);

  if (m_sockAddr.sin_addr.s_addr == INADDR_NONE)
    page15.m_strIpAddress = "(unknown)";
  else
    page15.m_strIpAddress = inet_ntoa (m_sockAddr.sin_addr);

  if (m_iConnectPhase != eConnectConnectedToMud)
    page15.m_strConnectionTime = "n/a";
  else
    page15.m_strConnectionTime = m_tConnectTime.Format (TranslateTime ("%A, %B %d, %Y, %#I:%M %p"));


  // first time spent in previous connections
  CTimeSpan ts = m_tsConnectDuration;
  
  // now time spent connected in this session, if we are connected
  if (m_iConnectPhase == eConnectConnectedToMud)
    ts += CTime::GetCurrentTime() - m_tConnectTime;

  page15.m_strConnectionDuration = ts.Format ("%Dd %Hh %Mm %Ss");

  if (m_nTotalUncompressed)
    {
    double fRatio = (double) m_nTotalCompressed / (double) m_nTotalUncompressed * 100.0;
    page15.m_strCompressionRatio.Format ("%6.1f%% (lower is better)", fRatio);
    if (m_iMCCP_type == 1)
       page15.m_strCompressionRatio += " MCCP v 1";
    else if (m_iMCCP_type == 2)
       page15.m_strCompressionRatio += " MCCP v 2";
    page15.m_strCompressedIn = CFormat ("%I64d", m_nTotalCompressed);
    page15.m_strCompressedOut = CFormat ("%I64d", m_nTotalUncompressed);
    }
  else
    {
    page15.m_strCompressedIn = "n/a";
    page15.m_strCompressedOut = "n/a";
    page15.m_strCompressionRatio = "(world is not using compression)";
    }

  __int64 nInK = m_nBytesIn / (__int64) 1024;
  __int64 nOutK = m_nBytesOut / (__int64) 1024;
  page15.m_strBytesReceived = CFormat ("%I64d bytes (%I64d Kb)", m_nBytesIn, nInK);
  page15.m_strBytesSent = CFormat ("%I64d bytes (%I64d Kb)", m_nBytesOut, nOutK);

  // MXP stuff

  int iRefCount = 0;

  for (pos = m_ActionList.GetHeadPosition (); pos; )
    iRefCount += m_ActionList.GetNext (pos)->GetReferenceCount ();

  page15.m_strMXPinbuiltElements  = CFormat ("%i", App.m_ElementMap.GetCount ());     
  page15.m_strMXPinbuiltEntities  = CFormat ("%i", App.m_EntityMap.GetCount ());     
  page15.m_strMXPmudElements      = CFormat ("%i", m_CustomElementMap.GetCount ());         
  page15.m_strMXPmudEntities      = CFormat ("%i", m_CustomEntityMap.GetCount ());         
  page15.m_strMXPunclosedTags     = CFormat ("%i", m_ActiveTagList.GetCount ());        
  page15.m_strMXPactionsCached    = CFormat ("%i", m_ActionList.GetCount ());        
  page15.m_strMXPreferenceCount   = CFormat ("%i", iRefCount);        

  // count of how many we got
  page15.m_strMXPtagsReceived = CFormat ("%I64d", m_iMXPtags);        
  page15.m_strMXPentitiesReceived = CFormat ("%I64d", m_iMXPentities);    
  page15.m_strMXPerrors = CFormat ("%I64d", m_iMXPerrors);              



  }   // end of CMUSHclientDoc::LoadPrefsP15

void CMUSHclientDoc:: LoadPrefsP16 (CPrefsP16 &page16)  
  {  
  DelayDebugMsg ("Loading", 16);
  page16.m_doc = this;
	page16.m_bEnableTimers = m_bEnableTimers;
	m_bConfigEnableTimers = m_bEnableTimers;
  page16.m_bUseDefaultTimers = m_bUseDefaultTimers;

  page16.SetUpPage ("timer",
                  (CObjectMap *) &m_TimerMap,
                  &page16.m_ctlTimerList,
                  &page16.m_ctlSummary,
                  page16.CompareObjects,
                  &m_TimersFindInfo,
                  XML_TIMERS);

  page16.m_iColumnCount      = CPrefsP16::eColumnCount;
  page16.m_iColWidth         = new int [CPrefsP16::eColumnCount];
  page16.m_strColumnHeadings = new CString [CPrefsP16::eColumnCount];
  page16.m_iColJust          = new int [CPrefsP16::eColumnCount];

  page16.m_iColWidth [CPrefsP16::eColumnType] = 70;
  page16.m_iColWidth [CPrefsP16::eColumnWhen] = 40;
  page16.m_iColWidth [CPrefsP16::eColumnContents] = 100;
  page16.m_iColWidth [CPrefsP16::eColumnLabel] = 50;
  page16.m_iColWidth [CPrefsP16::eColumnGroup] = 50;
  page16.m_iColWidth [CPrefsP16::eColumnNext] = 80;

  page16.m_iColJust [CPrefsP16::eColumnType] = LVCFMT_LEFT;
  page16.m_iColJust [CPrefsP16::eColumnWhen] = LVCFMT_LEFT;
  page16.m_iColJust [CPrefsP16::eColumnContents] = LVCFMT_LEFT;
  page16.m_iColJust [CPrefsP16::eColumnLabel] = LVCFMT_LEFT;
  page16.m_iColJust [CPrefsP16::eColumnGroup] = LVCFMT_LEFT;
  page16.m_iColJust [CPrefsP16::eColumnNext] = LVCFMT_LEFT;

  page16.m_strColumnHeadings [CPrefsP16::eColumnType] = TranslateHeading ("Type");
  page16.m_strColumnHeadings [CPrefsP16::eColumnWhen] = TranslateHeading ("When");
  page16.m_strColumnHeadings [CPrefsP16::eColumnContents] = TranslateHeading ("Send");
  page16.m_strColumnHeadings [CPrefsP16::eColumnLabel] = TranslateHeading ("Label");
  page16.m_strColumnHeadings [CPrefsP16::eColumnGroup] = TranslateHeading ("Group");
  page16.m_strColumnHeadings [CPrefsP16::eColumnNext] = TranslateHeading ("Next");
  }   // end of CMUSHclientDoc::LoadPrefsP16

void CMUSHclientDoc:: LoadPrefsP17 (CPrefsP17 &page17)  
  {  
  DelayDebugMsg ("Loading", 17);
  page17.m_doc = this;
 	page17.m_strLanguage          = m_strLanguage;            
	page17.m_bEnableScripts       = m_bEnableScripts;  
  if (GetScriptEngine ())
    page17.m_strIsActive = "(active)";
  else
    page17.m_strIsActive = "(not active)";

	page17.m_strWorldOpen         = m_strWorldOpen;     
	page17.m_strWorldClose        = m_strWorldClose;      
	page17.m_strWorldSave         = m_strWorldSave;      
	page17.m_strWorldConnect      = m_strWorldConnect;    
	page17.m_strWorldDisconnect   = m_strWorldDisconnect; 
	page17.m_strWorldGetFocus     = m_strWorldGetFocus;    
	page17.m_strWorldLoseFocus    = m_strWorldLoseFocus; 
	page17.m_strScriptFilename    = m_strScriptFilename;  
	page17.m_strScriptPrefix      = m_strScriptPrefix;
  page17.m_strScriptEditor      = m_strScriptEditor;
	page17.m_iNoteTextColour      = m_iNoteTextColour == SAMECOLOUR ? 0 : m_iNoteTextColour + 1;
  page17.m_iReloadOption = m_nReloadOption;
  page17.m_bEditScriptWithNotepad = m_bEditScriptWithNotepad;
  page17.m_bWarnIfScriptingInactive = m_bWarnIfScriptingInactive;
  page17.m_bScriptErrorsToOutputWindow = m_bScriptErrorsToOutputWindow;

  page17.m_strOnMXP_Start =        m_strOnMXP_Start;       
  page17.m_strOnMXP_Stop =         m_strOnMXP_Stop;        
  page17.m_strOnMXP_OpenTag =      m_strOnMXP_OpenTag;     
  page17.m_strOnMXP_CloseTag =     m_strOnMXP_CloseTag;    
  page17.m_strOnMXP_SetVariable =  m_strOnMXP_SetVariable; 
  page17.m_strOnMXP_Error =        m_strOnMXP_Error;       

  CString strExecutionTime;

  // time taken to execute scripts
  if (App.m_iCounterFrequency > 0)
    {
    double   elapsed_time;
    elapsed_time = ((double) m_iScriptTimeTaken) / 
                    ((double) App.m_iCounterFrequency);
    page17.m_strExecutionTime.Format ("Time spent: %16.6f seconds.", elapsed_time);
    }
  else
    page17.m_strExecutionTime.Empty ();

  }   // end of CMUSHclientDoc::LoadPrefsP17

void CMUSHclientDoc:: LoadPrefsP18 (CPrefsP18 &page18)  
  {  
  DelayDebugMsg ("Loading", 18);
  page18.m_doc = this;
// variables are used directly from the document

  page18.SetUpPage ("variable",
                  (CObjectMap *) &m_VariableMap,
                  &page18.m_ctlVariableList,
                  &page18.m_ctlSummary,
                  page18.CompareObjects,
                  &m_VariablesFindInfo,
                  XML_VARIABLES);

  page18.m_iColumnCount      = CPrefsP18::eColumnCount;
  page18.m_iColWidth         = new int [CPrefsP18::eColumnCount];
  page18.m_strColumnHeadings = new CString [CPrefsP18::eColumnCount];
  page18.m_iColJust        = new int [CPrefsP18::eColumnCount];

  page18.m_iColWidth [CPrefsP18::eColumnName] = 150;
  page18.m_iColWidth [CPrefsP18::eColumnContents] = 200;

  page18.m_iColJust [CPrefsP18::eColumnName] = LVCFMT_LEFT;
  page18.m_iColJust [CPrefsP18::eColumnContents] = LVCFMT_LEFT;

  page18.m_strColumnHeadings [CPrefsP18::eColumnName] = TranslateHeading ("Name");
  page18.m_strColumnHeadings [CPrefsP18::eColumnContents] = TranslateHeading ("Contents");
  }   // end of CMUSHclientDoc::LoadPrefsP18

void CMUSHclientDoc:: LoadPrefsP19 (CPrefsP19 &page19)  
  {  
  DelayDebugMsg ("Loading", 19);
  page19.m_doc = this;
  page19.m_strAutoSayString   = m_strAutoSayString;
  page19.m_bEnableAutoSay     = m_bEnableAutoSay;        
  page19.m_bExcludeMacros     = m_bExcludeMacros;        
  page19.m_bExcludeNonAlpha   = m_bExcludeNonAlpha;      
  page19.m_strOverridePrefix  = m_strOverridePrefix;     
  page19.m_bReEvaluateAutoSay = m_bReEvaluateAutoSay;     
  }   // end of CMUSHclientDoc::LoadPrefsP19

void CMUSHclientDoc:: LoadPrefsP20 (CPrefsP20 &page20)  
  {  
  DelayDebugMsg ("Loading", 20);
  page20.m_doc = this;
  page20.m_bBold0  = (m_nNormalPrintStyle [0] & FONT_BOLD) != 0;
  page20.m_bBold1  = (m_nNormalPrintStyle [1] & FONT_BOLD) != 0;
  page20.m_bBold2  = (m_nNormalPrintStyle [2] & FONT_BOLD) != 0;
  page20.m_bBold3  = (m_nNormalPrintStyle [3] & FONT_BOLD) != 0;
  page20.m_bBold4  = (m_nNormalPrintStyle [4] & FONT_BOLD) != 0;
  page20.m_bBold5  = (m_nNormalPrintStyle [5] & FONT_BOLD) != 0;
  page20.m_bBold6  = (m_nNormalPrintStyle [6] & FONT_BOLD) != 0;
  page20.m_bBold7  = (m_nNormalPrintStyle [7] & FONT_BOLD) != 0;

  page20.m_bBold8  = (m_nBoldPrintStyle [0] & FONT_BOLD) != 0;
  page20.m_bBold9  = (m_nBoldPrintStyle [1] & FONT_BOLD) != 0;
  page20.m_bBold10 = (m_nBoldPrintStyle [2] & FONT_BOLD) != 0;
  page20.m_bBold11 = (m_nBoldPrintStyle [3] & FONT_BOLD) != 0;
  page20.m_bBold12 = (m_nBoldPrintStyle [4] & FONT_BOLD) != 0;
  page20.m_bBold13 = (m_nBoldPrintStyle [5] & FONT_BOLD) != 0;
  page20.m_bBold14 = (m_nBoldPrintStyle [6] & FONT_BOLD) != 0;
  page20.m_bBold15 = (m_nBoldPrintStyle [7] & FONT_BOLD) != 0;

  page20.m_bUnderline0  = (m_nNormalPrintStyle [0] & FONT_UNDERLINE) != 0;
  page20.m_bUnderline1  = (m_nNormalPrintStyle [1] & FONT_UNDERLINE) != 0;
  page20.m_bUnderline2  = (m_nNormalPrintStyle [2] & FONT_UNDERLINE) != 0;
  page20.m_bUnderline3  = (m_nNormalPrintStyle [3] & FONT_UNDERLINE) != 0;
  page20.m_bUnderline4  = (m_nNormalPrintStyle [4] & FONT_UNDERLINE) != 0;
  page20.m_bUnderline5  = (m_nNormalPrintStyle [5] & FONT_UNDERLINE) != 0;
  page20.m_bUnderline6  = (m_nNormalPrintStyle [6] & FONT_UNDERLINE) != 0;
  page20.m_bUnderline7  = (m_nNormalPrintStyle [7] & FONT_UNDERLINE) != 0;

  page20.m_bUnderline8  = (m_nBoldPrintStyle [0] & FONT_UNDERLINE) != 0;
  page20.m_bUnderline9  = (m_nBoldPrintStyle [1] & FONT_UNDERLINE) != 0;
  page20.m_bUnderline10 = (m_nBoldPrintStyle [2] & FONT_UNDERLINE) != 0;
  page20.m_bUnderline11 = (m_nBoldPrintStyle [3] & FONT_UNDERLINE) != 0;
  page20.m_bUnderline12 = (m_nBoldPrintStyle [4] & FONT_UNDERLINE) != 0;
  page20.m_bUnderline13 = (m_nBoldPrintStyle [5] & FONT_UNDERLINE) != 0;
  page20.m_bUnderline14 = (m_nBoldPrintStyle [6] & FONT_UNDERLINE) != 0;
  page20.m_bUnderline15 = (m_nBoldPrintStyle [7] & FONT_UNDERLINE) != 0;

  page20.m_bItalic0  = (m_nNormalPrintStyle [0] & FONT_ITALIC) != 0;
  page20.m_bItalic1  = (m_nNormalPrintStyle [1] & FONT_ITALIC) != 0;
  page20.m_bItalic2  = (m_nNormalPrintStyle [2] & FONT_ITALIC) != 0;
  page20.m_bItalic3  = (m_nNormalPrintStyle [3] & FONT_ITALIC) != 0;
  page20.m_bItalic4  = (m_nNormalPrintStyle [4] & FONT_ITALIC) != 0;
  page20.m_bItalic5  = (m_nNormalPrintStyle [5] & FONT_ITALIC) != 0;
  page20.m_bItalic6  = (m_nNormalPrintStyle [6] & FONT_ITALIC) != 0;
  page20.m_bItalic7  = (m_nNormalPrintStyle [7] & FONT_ITALIC) != 0;

  page20.m_bItalic8  = (m_nBoldPrintStyle [0] & FONT_ITALIC) != 0;
  page20.m_bItalic9  = (m_nBoldPrintStyle [1] & FONT_ITALIC) != 0;
  page20.m_bItalic10 = (m_nBoldPrintStyle [2] & FONT_ITALIC) != 0;
  page20.m_bItalic11 = (m_nBoldPrintStyle [3] & FONT_ITALIC) != 0;
  page20.m_bItalic12 = (m_nBoldPrintStyle [4] & FONT_ITALIC) != 0;
  page20.m_bItalic13 = (m_nBoldPrintStyle [5] & FONT_ITALIC) != 0;
  page20.m_bItalic14 = (m_nBoldPrintStyle [6] & FONT_ITALIC) != 0;
  page20.m_bItalic15 = (m_nBoldPrintStyle [7] & FONT_ITALIC) != 0;
  }   // end of CMUSHclientDoc::LoadPrefsP20

void CMUSHclientDoc:: LoadPrefsP21 (CPrefsP21 &page21)  
  {  
  DelayDebugMsg ("Loading", 21);
  page21.m_doc = this;
  page21.m_name          = m_name;       
  page21.m_password      = m_password;   
  page21.m_connect_now   = m_connect_now;
  page21.m_connect_text  = m_connect_text;     
  }   // end of CMUSHclientDoc::LoadPrefsP21

void CMUSHclientDoc:: LoadPrefsP22 (CPrefsP22 &page22)  
  {  
  DelayDebugMsg ("Loading", 22);
  page22.m_doc = this;

  page22.m_iHyperlinkColour = m_iHyperlinkColour;
  page22.m_iMXPdebugLevel   = m_iMXPdebugLevel;
  page22.m_bPueblo          = m_bPueblo;
  page22.m_iUseMXP          = m_iUseMXP;
  page22.m_bHyperlinkAddsToCommandHistory = m_bHyperlinkAddsToCommandHistory;
  page22.m_bEchoHyperlinkInOutputWindow = m_bEchoHyperlinkInOutputWindow;
  page22.m_bUseCustomLinkColour       =  m_bUseCustomLinkColour       ;
  page22.m_bMudCanChangeLinkColour    =  m_bMudCanChangeLinkColour    ;
  page22.m_bUnderlineHyperlinks       =  m_bUnderlineHyperlinks       ;
  page22.m_bMudCanRemoveUnderline     =  m_bMudCanRemoveUnderline     ;
  page22.m_bIgnoreMXPcolourChanges    =  m_bIgnoreMXPcolourChanges  ;
  page22.m_bSendMXP_AFK_Response      =  m_bSendMXP_AFK_Response  ;
  page22.m_bMudCanChangeOptions       =  m_bMudCanChangeOptions  ;

  if (m_bPuebloActive)
    page22.m_strMXPactive = "Pueblo active";
  else if (m_bMXP)
    page22.m_strMXPactive = "MXP active";
  else
    page22.m_strMXPactive = "MXP/Pueblo inactive";

  }   // end of CMUSHclientDoc::LoadPrefsP22

void CMUSHclientDoc:: LoadPrefsP23 (CPrefsP23 &page23)  
  {  
  DelayDebugMsg ("Loading", 23);
  page23.m_doc = this;

  // take reasonable defaults

  if (m_strOurChatName.IsEmpty ())
    m_strOurChatName = m_name;      // try login character name

  if (m_strOurChatName.IsEmpty ())
    m_strOurChatName = DEFAULT_CHAT_NAME;  // still nothing? blech!

  if (m_IncomingChatPort == 0)
    m_IncomingChatPort = DEFAULT_CHAT_PORT;  // won't use port 0

  if (m_cChatForegroundColour == m_cChatBackgroundColour) 
    {
    // colour X on X will be hard to see ;)
    m_cChatForegroundColour = RGB (255, 0, 0);  // Red
    m_cChatBackgroundColour = RGB (0, 0, 0);    // on Black
    }

  // show whether we really are accepting calls
  m_bAcceptIncomingChatConnections = m_pChatListenSocket != NULL;

  page23.m_strOurChatName                   = m_strOurChatName                   ;
  page23.m_bAutoAllowSnooping               = m_bAutoAllowSnooping               ;
  page23.m_bAcceptIncomingChatConnections   = m_bAcceptIncomingChatConnections   ;
  page23.m_IncomingChatPort                 = m_IncomingChatPort                 ;
  page23.m_bValidateIncomingCalls           = m_bValidateIncomingCalls           ;
  page23.m_bIgnoreChatColours               = m_bIgnoreChatColours               ;
  page23.m_strChatMessagePrefix             = m_strChatMessagePrefix             ;
  page23.m_iMaxChatLinesPerMessage          = m_iMaxChatLinesPerMessage          ;
  page23.m_iMaxChatBytesPerMessage          = m_iMaxChatBytesPerMessage          ;
  page23.m_bAutoAllowFiles                  = m_bAutoAllowFiles                  ;
  page23.m_strChatSaveDirectory             = m_strChatSaveDirectory             ;
  page23.m_iTextColour                      = m_cChatForegroundColour            ;
  page23.m_iBackColour                      = m_cChatBackgroundColour            ; 

  }   // end of CMUSHclientDoc::LoadPrefsP23

/* *******************************************************************
   check if document has changed by comparing to property page
   *******************************************************************/

bool CMUSHclientDoc::ChangedPrefsP1  (CPrefsP1  &page1)
  {
  DelayDebugMsg ("Checking", 1);
  page1.m_mush_name.TrimLeft (); 
  page1.m_mush_name.TrimRight ();

  page1.m_server.TrimLeft ();
  page1.m_server.TrimRight ();

  return
    m_port          != page1.m_port ||       
    m_mush_name     != page1.m_mush_name ||  
    m_server        != page1.m_server ||     
    m_bSaveWorldAutomatically != page1.m_bSaveWorldAutomatically ||
    m_strProxyServerName  !=     page1.m_strProxyServerName ||
    m_iProxyServerPort    !=     page1.m_iProxyServerPort ||
    m_iSocksProcessing    !=     page1.m_iSocksProcessing ||
    m_strProxyUserName    !=     page1.m_strProxyUserName ||
    m_strProxyPassword    !=     page1.m_strProxyPassword;


  } // end of CMUSHclientDoc::ChangedPrefsP1

bool CMUSHclientDoc:: ChangedPrefsP2  (CPrefsP2  &page2)  
  {  
  DelayDebugMsg ("Checking", 2);
  return  m_new_activity_sound != page2.m_sound_pathname;
  }   // end of CMUSHclientDoc::ChangedPrefsP2

bool CMUSHclientDoc:: ChangedPrefsP3  (CPrefsP3  &page3)  
  {  
  DelayDebugMsg ("Checking", 3);
  bool bChanged = false;
  for (int i = 0; i < MAX_CUSTOM; i++)
     if (page3.m_strCustomName [i] != m_strCustomColourName [i])
       bChanged = true;

  return
    memcmp (m_customtext, page3.m_customtext, 
            sizeof (m_customtext)) != 0 ||
    memcmp (m_customback, page3.m_customback, 
            sizeof (m_customback)) != 0 ||
            bChanged;
  }   // end of CMUSHclientDoc::ChangedPrefsP3

bool CMUSHclientDoc:: ChangedPrefsP4  (CPrefsP4  &page4)  
  {  
  DelayDebugMsg ("Checking", 4);
  return
    m_bLogOutput       != page4.m_bLogOutput ||
    m_log_input        != page4.m_log_input || 
    m_bLogNotes        != page4.m_bLogNotes ||
    m_bLogHTML         != page4.m_bLogHTML ||
    m_strLogFilePreamble != page4.m_strLogFilePreamble ||
    m_strLogFilePostamble != page4.m_strLogFilePostamble ||
    m_strAutoLogFileName != page4.m_strAutoLogFileName ||
    m_bWriteWorldNameToLog != page4.m_bWriteWorldNameToLog ||
    m_strLogLinePreambleOutput != page4.m_strLogLinePreambleOutput ||
    m_strLogLinePreambleInput  != page4.m_strLogLinePreambleInput ||
    m_strLogLinePreambleNotes  != page4.m_strLogLinePreambleNotes ||
    m_strLogLinePostambleOutput != page4.m_strLogLinePostambleOutput ||
    m_strLogLinePostambleInput  != page4.m_strLogLinePostambleInput ||
    m_strLogLinePostambleNotes  != page4.m_strLogLinePostambleNotes ||
    m_bLogInColour  != page4.m_bLogInColour ||
    m_bLogRaw  != page4.m_bLogRaw;


  }   // end of CMUSHclientDoc::ChangedPrefsP4

bool CMUSHclientDoc:: ChangedPrefsP5  (CPrefsP5  &page5)  
  {  
  DelayDebugMsg ("Checking", 5);
  return
    memcmp (m_normalcolour, page5.m_normalcolour, 
            sizeof (m_normalcolour)) != 0 ||
    memcmp (m_boldcolour, page5.m_boldcolour, 
            sizeof (m_boldcolour)) != 0 ||
    m_bUseDefaultColours != page5.m_bUseDefaultColours ||
    m_bCustom16isDefaultColour != page5.m_bCustom16isDefaultColour;

  }   // end of CMUSHclientDoc::ChangedPrefsP5

bool CMUSHclientDoc:: ChangedPrefsP6  (CPrefsP6  &page6)  
  {  
  DelayDebugMsg ("Checking", 6);
int i;
bool array_changed = false;

  for (i = 0; i < NUMITEMS (m_macros); i++)
    {
    if (m_macros [i] != page6.m_macros [i])
      array_changed = true;
    if (m_macro_type [i] != page6.m_macro_type [i])
      array_changed = true;
    }

  return array_changed ||
    m_bUseDefaultMacros != page6.m_bUseDefaultMacros;

  }   // end of CMUSHclientDoc::ChangedPrefsP6

bool CMUSHclientDoc:: ChangedPrefsP7  (CPrefsP7  &page7)  
  {  
  DelayDebugMsg ("Checking", 7);
  return
    m_enable_aliases  != page7.m_enable_aliases ||
    m_bUseDefaultAliases != page7.m_bUseDefaultAliases;

  }   // end of CMUSHclientDoc::ChangedPrefsP7

bool CMUSHclientDoc:: ChangedPrefsP8  (CPrefsP8  &page8)  
  {  
  DelayDebugMsg ("Checking", 8);
  return
    m_enable_triggers != page8.m_enable_triggers ||
    m_enable_trigger_sounds != page8.m_enable_trigger_sounds ||
    m_bUseDefaultTriggers != page8.m_bUseDefaultTriggers;
  }   // end of CMUSHclientDoc::ChangedPrefsP8

bool CMUSHclientDoc:: ChangedPrefsP9  (CPrefsP9  &page9)  
  {  
  DelayDebugMsg ("Checking", 9);

  return
    m_strCommandStackCharacter != page9.m_command_stack_character [0] ||
    m_enable_command_stack != page9.m_enable_command_stacking ||
    m_enable_speed_walk != page9.m_enable_speed_walk ||
    m_speed_walk_prefix != page9.m_speed_walk_prefix ||
    m_echo_colour != (page9.m_echo_colour - 1) ||
    m_nHistoryLines != page9.m_nHistoryLines ||
    m_display_my_input != page9.m_echo_input ||
    m_input_text_colour       != page9.m_input_text_colour ||
    m_input_background_colour != page9.m_input_background_colour ||
    m_input_font_height     != page9.m_input_font_height ||
    m_input_font_name       != page9.m_input_font_name ||
    m_input_font_italic     != page9.m_input_font_italic ||
    m_input_font_weight     != page9.m_input_font_weight ||
    m_input_font_charset     != page9.m_input_font_charset ||
    m_bConfirmBeforeReplacingTyping != page9.m_bConfirmBeforeReplacingTyping ||
    m_bEscapeDeletesInput    !=  page9.m_bEscapeDeletesInput ||  
    m_bArrowsChangeHistory   !=  page9.m_bArrowsChangeHistory ||
    m_bAltArrowRecallsPartial != page9.m_bAltArrowRecallsPartial ||
    m_bArrowRecallsPartial != page9.m_bArrowRecallsPartial ||
    m_strSpeedWalkFiller != page9.m_strSpeedWalkFiller ||
    m_bAutoRepeat != page9.m_bAutoRepeat ||
    m_bLowerCaseTabCompletion != page9.m_bLowerCaseTabCompletion ||
    m_bDoubleClickInserts != page9.m_bDoubleClickInserts ||
    m_bDoubleClickSends != page9.m_bDoubleClickSends ||
    m_bTranslateGerman != page9.m_bTranslateGerman ||
    m_bSaveDeletedCommand != page9.m_bSaveDeletedCommand ||
    m_bTranslateBackslashSequences != page9.m_bTranslateBackslashSequences ||
    m_bSpellCheckOnSend != page9.m_bSpellCheckOnSend ||
    m_strTabCompletionDefaults != page9.m_strTabCompletionDefaults ||
    m_iTabCompletionLines != page9.m_iTabCompletionLines ||
    m_bTabCompletionSpace != page9.m_bTabCompletionSpace ||
    m_bArrowKeysWrap != page9.m_bArrowKeysWrap ||
    m_bUseDefaultInputFont != page9.m_bUseDefaultInputFont ||
    m_bKeepCommandsOnSameLine != page9.m_bKeepCommandsOnSameLine ||
    m_iSpeedWalkDelay != page9.m_iSpeedWalkDelay ||
    m_bCtrlZGoesToEndOfBuffer       != page9.m_bCtrlZGoesToEndOfBuffer ||
    m_bCtrlPGoesToPreviousCommand   != page9.m_bCtrlPGoesToPreviousCommand ||
    m_bCtrlNGoesToNextCommand       != page9.m_bCtrlNGoesToNextCommand ||
    m_bNoEchoOff                    != page9.m_bNoEchoOff ||
    m_bEnableSpamPrevention         != page9.m_bEnableSpamPrevention ||
    m_iSpamLineCount                != page9.m_iSpamLineCount ||
    m_strSpamMessage                != page9.m_strSpamMessage;




}   // end of CMUSHclientDoc::ChangedPrefsP9

bool CMUSHclientDoc:: ChangedPrefsP10 (CPrefsP10 &page10)  
  {  
  DelayDebugMsg ("Checking", 10);
  return
    m_file_postamble  !=  page10.m_file_postamble ||
    m_file_preamble   !=  page10.m_file_preamble ||
    m_line_postamble  !=  page10.m_line_postamble ||
    m_line_preamble   !=  page10.m_line_preamble ||
    m_bConfirmOnSend != page10.m_bConfirmOnSend ||
    m_bFileCommentedSoftcode != page10.m_bCommentedSoftcode ||
    m_nFileDelay != page10.m_iLineDelay ||
    m_bSendEcho  != page10.m_bEcho ||
    m_nFileDelayPerLines != page10.m_nFileDelayPerLines;


  }   // end of CMUSHclientDoc::ChangedPrefsP10

bool CMUSHclientDoc:: ChangedPrefsP11 (CPrefsP11 &page11)  
  {  
  DelayDebugMsg ("Checking", 11);
  return     	
    m_notes   !=  page11.m_notes;
  }   // end of CMUSHclientDoc::ChangedPrefsP11

bool CMUSHclientDoc:: ChangedPrefsP12 (CPrefsP12 &page12)  
  {  
  DelayDebugMsg ("Checking", 12);

  bool bChanged = false;

  for (int i = 0; i < eKeypad_Max_Items; i++)
    if (m_keypad [i] != page12.m_keypad [i])
      bChanged = true;

  return
    bChanged  ||
    m_keypad_enable != page12.m_keypad_enable;

}   // end of CMUSHclientDoc::ChangedPrefsP12

bool CMUSHclientDoc:: ChangedPrefsP13 (CPrefsP13 &page13)  
  {  
  DelayDebugMsg ("Checking", 13);
  return
    m_paste_postamble  !=  page13.m_paste_postamble ||
    m_paste_preamble   !=  page13.m_paste_preamble ||
    m_pasteline_postamble  !=  page13.m_pasteline_postamble ||
    m_pasteline_preamble   !=  page13.m_pasteline_preamble ||
    m_bConfirmOnPaste != page13.m_bConfirmOnPaste ||
    m_bPasteCommentedSoftcode != page13.m_bCommentedSoftcode ||
    m_nPasteDelay != page13.m_iLineDelay ||
    m_bPasteEcho != page13.m_bEcho ||
    m_nPasteDelayPerLines != page13.m_nPasteDelayPerLines;
  
  }   // end of CMUSHclientDoc::ChangedPrefsP13

bool CMUSHclientDoc:: ChangedPrefsP14 (CPrefsP14 &page14)  
  {  
  DelayDebugMsg ("Checking", 14);

  return
    m_font_height     != page14.m_font_height ||  
    m_font_name       != page14.m_font_name ||    
    m_font_weight     != page14.m_font_weight ||
    m_font_charset    != page14.m_font_charset ||
    m_wrap            != page14.m_wrap_output ||
    m_indent_paras    != page14.m_indent_paras ||
    m_enable_beeps    != page14.m_enable_beeps ||
    m_bUTF_8          != page14.m_bUTF_8 ||
    m_maxlines        != page14.m_nLines ||
    m_nWrapColumn     != page14.m_nWrapColumn ||
    m_bLineInformation!= page14.m_bLineInformation ||
    m_bStartPaused    != page14.m_bStartPaused ||
    m_bAutoFreeze     != page14.m_bAutoFreeze ||
    m_bShowBold       != page14.m_bShowBold ||
    m_bShowItalic     != page14.m_bShowItalic ||
    m_bShowUnderline  != page14.m_bShowUnderline ||
    m_iPixelOffset    != page14.m_iPixelOffset ||
    m_bDisableCompression    != page14.m_bDisableCompression ||
    m_bFlashIcon != page14.m_bFlashIcon ||
    m_bUseDefaultOutputFont != page14.m_bUseDefaultOutputFont ||
    m_bUnpauseOnSend != page14.m_bUnpauseOnSend ||
    m_bAlternativeInverse != page14.m_bAlternativeInverse ||
    m_bAutoWrapWindowWidth  != page14.m_bAutoWrapWindowWidth ||
    m_bNAWS  != page14.m_bNAWS ||
    m_bCarriageReturnClearsLine  != page14.m_bCarriageReturnClearsLine ||
    m_bConvertGAtoNewline  != page14.m_bConvertGAtoNewline ||
    m_strTerminalIdentification  != page14.m_strTerminalIdentification ||
    m_strBeepSound  != page14.m_strBeepSound ||
    m_bShowConnectDisconnect  != page14.m_bShowConnectDisconnect ||
    m_bCopySelectionToClipboard  != page14.m_bCopySelectionToClipboard ||
    m_bAutoCopyInHTML  != page14.m_bAutoCopyInHTML ||
    m_iLineSpacing  != page14.m_iLineSpacing ||
    m_new_activity_sound != page14.m_sound_pathname; 


  }   // end of CMUSHclientDoc::ChangedPrefsP14

bool CMUSHclientDoc:: ChangedPrefsP15 (CPrefsP15 &page15)  
  {  
  DelayDebugMsg ("Checking", 15);
  return false;
  }   // end of CMUSHclientDoc::ChangedPrefsP15

bool CMUSHclientDoc:: ChangedPrefsP16 (CPrefsP16 &page16)  
  {  
  DelayDebugMsg ("Checking", 16);
  return 
  	m_bEnableTimers != page16.m_bEnableTimers ||
    m_bUseDefaultTimers != page16.m_bUseDefaultTimers;
}   // end of CMUSHclientDoc::ChangedPrefsP16

bool CMUSHclientDoc:: ChangedPrefsP17 (CPrefsP17 &page17)  
  {  
  DelayDebugMsg ("Checking", 17);
  return
 	  m_strLanguage          != page17.m_strLanguage ||            
	  m_bEnableScripts       != page17.m_bEnableScripts ||     
	  m_strWorldOpen         != page17.m_strWorldOpen ||     
	  m_strWorldClose        != page17.m_strWorldClose ||      
	  m_strWorldSave         != page17.m_strWorldSave ||      
	  m_strWorldConnect      != page17.m_strWorldConnect ||    
	  m_strWorldDisconnect   != page17.m_strWorldDisconnect || 
	  m_strWorldGetFocus     != page17.m_strWorldGetFocus ||
	  m_strWorldLoseFocus    != page17.m_strWorldLoseFocus ||
	  m_strScriptFilename    != page17.m_strScriptFilename ||  
    m_strScriptPrefix      != page17.m_strScriptPrefix ||
    m_strScriptEditor      != page17.m_strScriptEditor ||
    m_iNoteTextColour      != (page17.m_iNoteTextColour - 1) ||
    m_nReloadOption        != page17.m_iReloadOption || 
    m_bEditScriptWithNotepad != page17.m_bEditScriptWithNotepad ||
    m_bWarnIfScriptingInactive != page17.m_bWarnIfScriptingInactive ||
    m_strOnMXP_Start !=        page17.m_strOnMXP_Start ||       
    m_strOnMXP_Stop !=         page17.m_strOnMXP_Stop ||        
    m_strOnMXP_OpenTag !=      page17.m_strOnMXP_OpenTag ||     
    m_strOnMXP_CloseTag !=     page17.m_strOnMXP_CloseTag ||    
    m_strOnMXP_SetVariable !=  page17.m_strOnMXP_SetVariable || 
    m_strOnMXP_Error !=        page17.m_strOnMXP_Error ||
    m_bScriptErrorsToOutputWindow != page17.m_bScriptErrorsToOutputWindow;



  }   // end of CMUSHclientDoc::ChangedPrefsP17

bool CMUSHclientDoc:: ChangedPrefsP18 (CPrefsP18 &page18)  
  {  
  DelayDebugMsg ("Checking", 18);
// page 18 - nothing to check
  return false;
  }   // end of CMUSHclientDoc::ChangedPrefsP18

bool CMUSHclientDoc:: ChangedPrefsP19 (CPrefsP19 &page19)  
  {  
  DelayDebugMsg ("Checking", 19);
  return
    m_strAutoSayString   !=  page19.m_strAutoSayString ||
    m_bEnableAutoSay     !=  page19.m_bEnableAutoSay ||        
    m_bExcludeMacros     !=  page19.m_bExcludeMacros ||        
    m_bExcludeNonAlpha   !=  page19.m_bExcludeNonAlpha ||      
    m_strOverridePrefix  !=  page19.m_strOverridePrefix ||
    m_bReEvaluateAutoSay !=  page19.m_bReEvaluateAutoSay;

  }   // end of CMUSHclientDoc::ChangedPrefsP19

bool CMUSHclientDoc:: ChangedPrefsP20 (CPrefsP20 &page20)  
  {  
  DelayDebugMsg ("Checking", 20);
// fix up printer styles

  int i,
      nBold [16],
      nItalic [16],
      nUnderline [16];

  nBold [0]  = page20.m_bBold0;
  nBold [1]  = page20.m_bBold1;
  nBold [2]  = page20.m_bBold2;
  nBold [3]  = page20.m_bBold3;
  nBold [4]  = page20.m_bBold4;
  nBold [5]  = page20.m_bBold5;
  nBold [6]  = page20.m_bBold6;
  nBold [7]  = page20.m_bBold7;
  nBold [8]  = page20.m_bBold8;
  nBold [9]  = page20.m_bBold9;
  nBold [10] = page20.m_bBold10;
  nBold [11] = page20.m_bBold11;
  nBold [12] = page20.m_bBold12;
  nBold [13] = page20.m_bBold13;
  nBold [14] = page20.m_bBold14;
  nBold [15] = page20.m_bBold15;

  nItalic [0]  = page20.m_bItalic0;
  nItalic [1]  = page20.m_bItalic1;
  nItalic [2]  = page20.m_bItalic2;
  nItalic [3]  = page20.m_bItalic3;
  nItalic [4]  = page20.m_bItalic4;
  nItalic [5]  = page20.m_bItalic5;
  nItalic [6]  = page20.m_bItalic6;
  nItalic [7]  = page20.m_bItalic7;
  nItalic [8]  = page20.m_bItalic8;
  nItalic [9]  = page20.m_bItalic9;
  nItalic [10] = page20.m_bItalic10;
  nItalic [11] = page20.m_bItalic11;
  nItalic [12] = page20.m_bItalic12;
  nItalic [13] = page20.m_bItalic13;
  nItalic [14] = page20.m_bItalic14;
  nItalic [15] = page20.m_bItalic15;

  nUnderline [0]  = page20.m_bUnderline0;
  nUnderline [1]  = page20.m_bUnderline1;
  nUnderline [2]  = page20.m_bUnderline2;
  nUnderline [3]  = page20.m_bUnderline3;
  nUnderline [4]  = page20.m_bUnderline4;
  nUnderline [5]  = page20.m_bUnderline5;
  nUnderline [6]  = page20.m_bUnderline6;
  nUnderline [7]  = page20.m_bUnderline7;
  nUnderline [8]  = page20.m_bUnderline8;
  nUnderline [9]  = page20.m_bUnderline9;
  nUnderline [10] = page20.m_bUnderline10;
  nUnderline [11] = page20.m_bUnderline11;
  nUnderline [12] = page20.m_bUnderline12;
  nUnderline [13] = page20.m_bUnderline13;
  nUnderline [14] = page20.m_bUnderline14;
  nUnderline [15] = page20.m_bUnderline15;

  int nNormalPrintStyle [8],
      nBoldPrintStyle [8];

  bool bStylesChanged = false;

  for (i = 0; i < 8; i++)
    {
    nNormalPrintStyle [i] = 0;
    nBoldPrintStyle [i] = 0;

// bold styles

    if (nBold [i])
      nNormalPrintStyle [i] |= FONT_BOLD;
    if (nBold [i + 8])
      nBoldPrintStyle [i] |= FONT_BOLD;

// underline styles

    if (nUnderline [i])
      nNormalPrintStyle [i] |= FONT_UNDERLINE;
    if (nUnderline [i + 8])
      nBoldPrintStyle [i] |= FONT_UNDERLINE;

// italic styles

    if (nItalic [i])
      nNormalPrintStyle [i] |= FONT_ITALIC;
    if (nItalic [i + 8])
      nBoldPrintStyle [i] |= FONT_ITALIC;

    if (nNormalPrintStyle [i] != m_nNormalPrintStyle [i])
      bStylesChanged = true;

    if (nBoldPrintStyle [i] != m_nBoldPrintStyle [i])
      bStylesChanged = true;

    }   // end of converting styles

  // save in document as well, after all that trouble :)

  for (i = 0; i < 8; i++)
    {
    m_nNormalPrintStyle [i] = nNormalPrintStyle [i];
    m_nBoldPrintStyle [i] = nBoldPrintStyle [i];
    }

  return bStylesChanged;

  }   // end of CMUSHclientDoc::ChangedPrefsP20

bool CMUSHclientDoc:: ChangedPrefsP21 (CPrefsP21 &page21)  
  {  
  DelayDebugMsg ("Checking", 21);
  page21.m_name.TrimLeft (); 
  page21.m_name.TrimRight ();

  return
    m_name          != page21.m_name ||       
    m_password      != page21.m_password ||   
    m_connect_now   != page21.m_connect_now ||
    m_connect_text  != page21.m_connect_text   ;

  }   // end of CMUSHclientDoc::ChangedPrefsP21


bool CMUSHclientDoc:: ChangedPrefsP22 (CPrefsP22 &page22)  
  {  
  DelayDebugMsg ("Checking", 22);

  return
    m_iMXPdebugLevel != page22.m_iMXPdebugLevel ||
    m_bPueblo  != page22.m_bPueblo ||
    m_iHyperlinkColour != page22.m_iHyperlinkColour ||
    m_iUseMXP != page22.m_iUseMXP ||
    m_bHyperlinkAddsToCommandHistory   != page22.m_bHyperlinkAddsToCommandHistory ||
    m_bEchoHyperlinkInOutputWindow     != page22.m_bEchoHyperlinkInOutputWindow ||
    m_bUseCustomLinkColour             !=  page22.m_bUseCustomLinkColour        ||
    m_bMudCanChangeLinkColour          !=  page22.m_bMudCanChangeLinkColour     ||
    m_bUnderlineHyperlinks             !=  page22.m_bUnderlineHyperlinks        ||
    m_bMudCanRemoveUnderline           !=  page22.m_bMudCanRemoveUnderline      ||
    m_bIgnoreMXPcolourChanges          !=  page22.m_bIgnoreMXPcolourChanges     ||
    m_bSendMXP_AFK_Response            !=  page22.m_bSendMXP_AFK_Response       ||
    m_bMudCanChangeOptions             !=  page22.m_bMudCanChangeOptions;
  }   // end of CMUSHclientDoc::ChangedPrefsP22

bool CMUSHclientDoc:: ChangedPrefsP23 (CPrefsP23 &page23)  
  {  
  DelayDebugMsg ("Checking", 23);

  return
    m_strOurChatName                   != page23.m_strOurChatName                   ||
    m_bAutoAllowSnooping               != page23.m_bAutoAllowSnooping               ||
    m_bAcceptIncomingChatConnections   != page23.m_bAcceptIncomingChatConnections   ||
    m_IncomingChatPort                 != page23.m_IncomingChatPort                 ||
    m_bValidateIncomingCalls           != page23.m_bValidateIncomingCalls           ||
    m_bIgnoreChatColours               != page23.m_bIgnoreChatColours               ||
    m_strChatMessagePrefix             != page23.m_strChatMessagePrefix             ||
    m_iMaxChatLinesPerMessage          != page23.m_iMaxChatLinesPerMessage          ||
    m_iMaxChatBytesPerMessage          != page23.m_iMaxChatBytesPerMessage          ||
    m_bAutoAllowFiles                  != page23.m_bAutoAllowFiles                  ||
    m_strChatSaveDirectory             != page23.m_strChatSaveDirectory             ||
    m_cChatForegroundColour            != page23.m_iTextColour                      || 
    m_cChatBackgroundColour            != page23.m_iBackColour; 


  }   // end of CMUSHclientDoc::ChangedPrefsP23

/* *******************************************************************
   unload from property page into document
   *******************************************************************/

void CMUSHclientDoc::SavePrefsP1  (CPrefsP1  &page1)
  {
  DelayDebugMsg ("Saving", 1);
  m_port          = (unsigned short) page1.m_port;       
  m_mush_name     = page1.m_mush_name;  
  m_server        = page1.m_server;     
  m_bSaveWorldAutomatically = page1.m_bSaveWorldAutomatically;
  m_strProxyServerName = page1.m_strProxyServerName;
  m_iProxyServerPort = (unsigned short) page1.m_iProxyServerPort;
  m_iSocksProcessing = page1.m_iSocksProcessing;
  m_strProxyUserName   =     page1.m_strProxyUserName;
  m_strProxyPassword   =     page1.m_strProxyPassword;

  } // end of CMUSHclientDoc::SavePrefsP1

void CMUSHclientDoc:: SavePrefsP2  (CPrefsP2  &page2)  
  {  
  DelayDebugMsg ("Saving", 2);
  }   // end of CMUSHclientDoc::SavePrefsP2

void CMUSHclientDoc:: SavePrefsP3  (CPrefsP3  &page3)  
  {  
  DelayDebugMsg ("Saving", 3);
  memcpy (m_customtext,   page3.m_customtext, sizeof (m_customtext));
  memcpy (m_customback,   page3.m_customback, sizeof (m_customback));
  for (int i = 0; i < MAX_CUSTOM; i++)
     m_strCustomColourName [i] = page3.m_strCustomName [i];

  }   // end of CMUSHclientDoc::SavePrefsP3

void CMUSHclientDoc:: SavePrefsP4  (CPrefsP4  &page4)  
  {  
  DelayDebugMsg ("Saving", 4);
  m_bLogOutput       = page4.m_bLogOutput;
  m_log_input        = page4.m_log_input; 
  m_bLogNotes        = page4.m_bLogNotes;
  m_bLogHTML         = page4.m_bLogHTML;
  m_strLogFilePreamble = page4.m_strLogFilePreamble;
  m_strLogFilePostamble = page4.m_strLogFilePostamble;
  m_strAutoLogFileName = page4.m_strAutoLogFileName;
  m_bWriteWorldNameToLog = page4.m_bWriteWorldNameToLog;
  m_strLogLinePreambleOutput = page4.m_strLogLinePreambleOutput;
  m_strLogLinePreambleInput =  page4.m_strLogLinePreambleInput;
  m_strLogLinePreambleNotes =  page4.m_strLogLinePreambleNotes;
  m_strLogLinePostambleOutput = page4.m_strLogLinePostambleOutput;
  m_strLogLinePostambleInput =  page4.m_strLogLinePostambleInput;
  m_strLogLinePostambleNotes =  page4.m_strLogLinePostambleNotes;
  m_bLogInColour =  page4.m_bLogInColour;
  m_bLogRaw =  page4.m_bLogRaw;
  }   // end of CMUSHclientDoc::SavePrefsP4

void CMUSHclientDoc:: SavePrefsP5  (CPrefsP5  &page5)  
  {  
  DelayDebugMsg ("Saving", 5);
  memcpy (m_normalcolour, page5.m_normalcolour, sizeof (m_normalcolour));
  memcpy (m_boldcolour,   page5.m_boldcolour, sizeof (m_boldcolour));
  m_bUseDefaultColours = page5.m_bUseDefaultColours;
  m_bCustom16isDefaultColour = page5.m_bCustom16isDefaultColour;
  }   // end of CMUSHclientDoc::SavePrefsP5

void CMUSHclientDoc:: SavePrefsP6  (CPrefsP6  &page6)  
  {  
  DelayDebugMsg ("Saving", 6);
int i;
  for (i = 0; i < NUMITEMS (m_macros); i++)
    {
    m_macros [i]      = page6.m_macros [i];
    m_macro_type [i]  = page6.m_macro_type [i];
    }
  m_bUseDefaultMacros = page6.m_bUseDefaultMacros;
  }   // end of CMUSHclientDoc::SavePrefsP6

void CMUSHclientDoc:: SavePrefsP7  (CPrefsP7  &page7)  
  {  
  DelayDebugMsg ("Saving", 7);
	m_enable_aliases  = page7.m_enable_aliases;
  m_bUseDefaultAliases = page7.m_bUseDefaultAliases;
  }   // end of CMUSHclientDoc::SavePrefsP7

void CMUSHclientDoc:: SavePrefsP8  (CPrefsP8  &page8)  
  {  
  DelayDebugMsg ("Saving", 8);
	m_enable_triggers = page8.m_enable_triggers;
	m_enable_trigger_sounds = page8.m_enable_trigger_sounds;
  m_bUseDefaultTriggers = page8.m_bUseDefaultTriggers;
  }   // end of CMUSHclientDoc::SavePrefsP8

void CMUSHclientDoc:: SavePrefsP9  (CPrefsP9  &page9)  
  {  
  DelayDebugMsg ("Saving", 9);
	m_strCommandStackCharacter = page9.m_command_stack_character [0];
	m_enable_command_stack = page9.m_enable_command_stacking;
	m_enable_speed_walk = page9.m_enable_speed_walk;
	m_speed_walk_prefix = page9.m_speed_walk_prefix;
	m_echo_colour = page9.m_echo_colour - 1;
  m_nHistoryLines = page9.m_nHistoryLines;
	m_display_my_input = page9.m_echo_input;
  m_input_text_colour       = page9.m_input_text_colour;
  m_input_background_colour = page9.m_input_background_colour;
  m_input_font_height     = page9.m_input_font_height;  
  m_input_font_name       = page9.m_input_font_name;    
  m_input_font_italic     = page9.m_input_font_italic;  
  m_input_font_weight     = page9.m_input_font_weight;
  m_input_font_charset     = page9.m_input_font_charset;
  m_bConfirmBeforeReplacingTyping = page9.m_bConfirmBeforeReplacingTyping;
  m_bEscapeDeletesInput    =  page9.m_bEscapeDeletesInput;
  m_bArrowsChangeHistory   =  page9.m_bArrowsChangeHistory;
  m_bAltArrowRecallsPartial = page9.m_bAltArrowRecallsPartial;
  m_bArrowRecallsPartial = page9.m_bArrowRecallsPartial;
  m_strSpeedWalkFiller = page9.m_strSpeedWalkFiller;
  m_bAutoRepeat = page9.m_bAutoRepeat;
  m_bLowerCaseTabCompletion = page9.m_bLowerCaseTabCompletion;
  m_bDoubleClickInserts = page9.m_bDoubleClickInserts;
  m_bDoubleClickSends = page9.m_bDoubleClickSends;
  m_bTranslateGerman = page9.m_bTranslateGerman;
  m_bSaveDeletedCommand = page9.m_bSaveDeletedCommand;
  m_bTranslateBackslashSequences = page9.m_bTranslateBackslashSequences;
  m_bSpellCheckOnSend = page9.m_bSpellCheckOnSend;
  m_strTabCompletionDefaults = page9.m_strTabCompletionDefaults;
  m_iTabCompletionLines = page9.m_iTabCompletionLines;
  m_bTabCompletionSpace = page9.m_bTabCompletionSpace;
  m_bArrowKeysWrap = page9.m_bArrowKeysWrap;
  m_bUseDefaultInputFont = page9.m_bUseDefaultInputFont;
  m_bKeepCommandsOnSameLine = page9.m_bKeepCommandsOnSameLine;
  m_bCtrlZGoesToEndOfBuffer       = page9.m_bCtrlZGoesToEndOfBuffer;
  m_bCtrlPGoesToPreviousCommand   = page9.m_bCtrlPGoesToPreviousCommand;
  m_bCtrlNGoesToNextCommand       = page9.m_bCtrlNGoesToNextCommand;
  m_bNoEchoOff                    = page9.m_bNoEchoOff;
  m_bEnableSpamPrevention         = page9.m_bEnableSpamPrevention;
  m_iSpamLineCount                = page9.m_iSpamLineCount;
  m_strSpamMessage                = page9.m_strSpamMessage;


  // tell all views about our new speedwalk delay rate if necessary (and update it)
  if (m_iSpeedWalkDelay != page9.m_iSpeedWalkDelay)
    SetSpeedWalkDelay (page9.m_iSpeedWalkDelay);

  }   // end of CMUSHclientDoc::SavePrefsP9

void CMUSHclientDoc:: SavePrefsP10 (CPrefsP10 &page10)  
  {  
  DelayDebugMsg ("Saving", 10);
	m_file_postamble  =  page10.m_file_postamble;
	m_file_preamble   =  page10.m_file_preamble; 
	m_line_postamble  =  page10.m_line_postamble;
	m_line_preamble   =  page10.m_line_preamble; 
  m_bConfirmOnSend = page10.m_bConfirmOnSend;
  m_bFileCommentedSoftcode = page10.m_bCommentedSoftcode;
  m_nFileDelay = page10.m_iLineDelay;
  m_bSendEcho = page10.m_bEcho;
  m_nFileDelayPerLines = page10.m_nFileDelayPerLines;
  }   // end of CMUSHclientDoc::SavePrefsP10

void CMUSHclientDoc:: SavePrefsP11 (CPrefsP11 &page11)  
  {  
  DelayDebugMsg ("Saving", 11);
	m_notes   =  page11.m_notes; 
  }   // end of CMUSHclientDoc::SavePrefsP11

void CMUSHclientDoc:: SavePrefsP12 (CPrefsP12 &page12)  
  {  
  DelayDebugMsg ("Saving", 12);
  for (int i = 0; i < eKeypad_Max_Items; i++)
    m_keypad [i] = page12.m_keypad [i];

  m_keypad_enable = page12.m_keypad_enable;
  }   // end of CMUSHclientDoc::SavePrefsP12

void CMUSHclientDoc:: SavePrefsP13 (CPrefsP13 &page13)  
  {  
  DelayDebugMsg ("Saving", 13);
	m_paste_postamble  =  page13.m_paste_postamble;
	m_paste_preamble   =  page13.m_paste_preamble; 
	m_pasteline_postamble  =  page13.m_pasteline_postamble;
	m_pasteline_preamble   =  page13.m_pasteline_preamble; 
  m_bConfirmOnPaste = page13.m_bConfirmOnPaste;
  m_bPasteCommentedSoftcode = page13.m_bCommentedSoftcode;
  m_nPasteDelay = page13.m_iLineDelay;
  m_bPasteEcho = page13.m_bEcho;
  m_nPasteDelayPerLines = page13.m_nPasteDelayPerLines;
  }   // end of CMUSHclientDoc::SavePrefsP13

void CMUSHclientDoc:: SavePrefsP14 (CPrefsP14 &page14)  
  {  
  DelayDebugMsg ("Saving", 14);
  m_font_height     = page14.m_font_height;  
  m_font_name       = page14.m_font_name;    
  m_font_weight     = page14.m_font_weight;
  m_font_charset    = page14.m_font_charset;
  m_wrap            = page14.m_wrap_output;  
  m_indent_paras    = page14.m_indent_paras;
  m_enable_beeps    = page14.m_enable_beeps;
  m_bUTF_8          = page14.m_bUTF_8;
  m_bLineInformation= page14.m_bLineInformation;
  m_bStartPaused    = page14.m_bStartPaused;
  m_bAutoFreeze     = page14.m_bAutoFreeze;
  m_bShowBold       = page14.m_bShowBold;
  m_bShowItalic     = page14.m_bShowItalic;
  m_bShowUnderline  = page14.m_bShowUnderline;
  m_iPixelOffset    = page14.m_iPixelOffset;
  m_bDisableCompression    = page14.m_bDisableCompression;
  m_bFlashIcon = page14.m_bFlashIcon;
  m_bUseDefaultOutputFont = page14.m_bUseDefaultOutputFont;
  m_bUnpauseOnSend = page14.m_bUnpauseOnSend;
  m_bAlternativeInverse = page14.m_bAlternativeInverse;
  m_bAutoWrapWindowWidth    = page14.m_bAutoWrapWindowWidth;
  m_bNAWS    = page14.m_bNAWS;
  m_bCarriageReturnClearsLine    = page14.m_bCarriageReturnClearsLine;
  m_bConvertGAtoNewline    = page14.m_bConvertGAtoNewline;
  m_strBeepSound    = page14.m_strBeepSound;
  m_strTerminalIdentification    = page14.m_strTerminalIdentification;
  m_bShowConnectDisconnect    = page14.m_bShowConnectDisconnect;
  m_bCopySelectionToClipboard    = page14.m_bCopySelectionToClipboard;
  m_bAutoCopyInHTML    = page14.m_bAutoCopyInHTML;
  m_iLineSpacing    = page14.m_iLineSpacing;
  m_new_activity_sound = page14.m_sound_pathname;

// we must adjust the output buffer to reflect the new number of lines in it

  if (m_maxlines != page14.m_nLines)
    if (FixUpOutputBuffer (page14.m_nLines))  // if we could adjust it
      m_maxlines        = page14.m_nLines;

// We must adjust the current line to allow for the new wrap size

  if (m_pCurrentLine)     // a new world might not have a line yet
    {
    // save current line text
    CString strLine = CString (m_pCurrentLine->text, m_pCurrentLine->len);

    if (m_bUTF_8)
      m_pCurrentLine->iMemoryAllocated = MAX ((UINT) m_pCurrentLine->len, page14.m_nWrapColumn) * 4;
    else
      m_pCurrentLine->iMemoryAllocated = MAX ((UINT) m_pCurrentLine->len, page14.m_nWrapColumn);

#ifdef USE_REALLOC

    m_pCurrentLine->text  = (char *) realloc (m_pCurrentLine->text, 
                                              m_pCurrentLine->iMemoryAllocated);  

#else
    delete [] m_pCurrentLine->text;
    m_pCurrentLine->text = new char [m_pCurrentLine->iMemoryAllocated];
#endif

    // check we got it
    ASSERT (m_pCurrentLine->text);

    // put text back
    memcpy (m_pCurrentLine->text, (LPCTSTR) strLine, m_pCurrentLine->len);
    }   // end of having a current line

  if (m_nWrapColumn != page14.m_nWrapColumn)
    SendWindowSizes (page14.m_nWrapColumn);

  m_nWrapColumn     = page14.m_nWrapColumn;

  }   // end of CMUSHclientDoc::SavePrefsP14

void CMUSHclientDoc:: SavePrefsP15 (CPrefsP15 &page15)  
  {  
  DelayDebugMsg ("Saving", 15);
  }   // end of CMUSHclientDoc::SavePrefsP15

void CMUSHclientDoc:: SavePrefsP16 (CPrefsP16 &page16)  
  {  
  DelayDebugMsg ("Saving", 16);
  m_bEnableTimers = page16.m_bEnableTimers;
  m_bUseDefaultTimers = page16.m_bUseDefaultTimers;
  }   // end of CMUSHclientDoc::SavePrefsP16

void CMUSHclientDoc:: SavePrefsP17 (CPrefsP17 &page17)  
  {  
  DelayDebugMsg ("Saving", 17);
// disable old scripting engine if:
  // a) The language has changed; or
  // b) The script file name has changed

  if (m_strScriptFilename != page17.m_strScriptFilename ||
      m_strLanguage != page17.m_strLanguage)
    DisableScripting ();

 	m_strLanguage          = page17.m_strLanguage;            
	m_bEnableScripts       = page17.m_bEnableScripts;     
	m_strWorldOpen         = page17.m_strWorldOpen;     
	m_strWorldClose        = page17.m_strWorldClose;      
	m_strWorldSave         = page17.m_strWorldSave;      
	m_strWorldConnect      = page17.m_strWorldConnect;    
	m_strWorldDisconnect   = page17.m_strWorldDisconnect; 
	m_strWorldGetFocus     = page17.m_strWorldGetFocus;    
	m_strWorldLoseFocus    = page17.m_strWorldLoseFocus; 
	m_strScriptFilename    = page17.m_strScriptFilename;  
  m_strScriptPrefix      = page17.m_strScriptPrefix;
  m_strScriptEditor      = page17.m_strScriptEditor;
  m_iNoteTextColour      = page17.m_iNoteTextColour - 1;
  m_nReloadOption        = page17.m_iReloadOption;
  m_bEditScriptWithNotepad = page17.m_bEditScriptWithNotepad;
  m_bWarnIfScriptingInactive = page17.m_bWarnIfScriptingInactive;
  m_bScriptErrorsToOutputWindow = page17.m_bScriptErrorsToOutputWindow;

  m_strOnMXP_Start =        page17.m_strOnMXP_Start;       
  m_strOnMXP_Stop =         page17.m_strOnMXP_Stop;        
  m_strOnMXP_OpenTag =      page17.m_strOnMXP_OpenTag;     
  m_strOnMXP_CloseTag =     page17.m_strOnMXP_CloseTag;    
  m_strOnMXP_SetVariable =  page17.m_strOnMXP_SetVariable; 
  m_strOnMXP_Error =        page17.m_strOnMXP_Error;       

  // disable scripting if necessary

  if (!m_bEnableScripts)
    DisableScripting ();

  // initialise (new) scripting engine if necessary
  
  if (m_bEnableScripts &&     // provided wanted
     !m_ScriptEngine)         // and not already going
    {
    // warn in case they wonder why VBscript doesn't work
    if (bWine && m_strLanguage.CompareNoCase ("lua") != 0)
      ::TMessageBox ("Only the Lua script language is available with the /wine option",
      MB_ICONINFORMATION);
    else
      CreateScriptEngine ();  // create scripting engine
    }

// in case they are using Windows 95, reload script file if necessary

  OnScriptFileChanged (true);

  CString strErrorMessage;

  // find entry point for world open, close, connect, disconnect etc.

  FindGlobalEntryPoints (strErrorMessage);

  // no errors, don't display an error dialog box

  if (!strErrorMessage.IsEmpty ())
    {
    // show the errors

    CMissingEntryPoints dlg;

    dlg.m_strErrorMessage = strErrorMessage;

    dlg.DoModal ();
    }   // end of having errors

  }   // end of CMUSHclientDoc::SavePrefsP17

void CMUSHclientDoc:: SavePrefsP18 (CPrefsP18 &page18)  
  {  
  DelayDebugMsg ("Saving", 18);
// page 18 - nothing to copy
  }   // end of CMUSHclientDoc::SavePrefsP18

void CMUSHclientDoc:: SavePrefsP19 (CPrefsP19 &page19)  
  {  
  DelayDebugMsg ("Saving", 19);
  m_strAutoSayString   =  page19.m_strAutoSayString;
  m_bEnableAutoSay     =  page19.m_bEnableAutoSay;        
  m_bExcludeMacros     =  page19.m_bExcludeMacros;        
  m_bExcludeNonAlpha   =  page19.m_bExcludeNonAlpha;      
  m_strOverridePrefix  =  page19.m_strOverridePrefix;     
  m_bReEvaluateAutoSay =  page19.m_bReEvaluateAutoSay;     
  }   // end of CMUSHclientDoc::SavePrefsP19

void CMUSHclientDoc:: SavePrefsP20 (CPrefsP20 &page20)  
  {  
  DelayDebugMsg ("Saving", 20);

  // do the saving in the "ischanged" routine - it does a lot of work for it
  }   // end of CMUSHclientDoc::SavePrefsP20

void CMUSHclientDoc:: SavePrefsP21 (CPrefsP21 &page21)  
  {  
  DelayDebugMsg ("Saving", 21);
  m_name          = page21.m_name;       
  m_password      = page21.m_password;   
  m_connect_now   = page21.m_connect_now;
  m_connect_text  = page21.m_connect_text;    
  }   // end of CMUSHclientDoc::SavePrefsP21

void CMUSHclientDoc:: SavePrefsP22 (CPrefsP22 &page22)  
  {  
  DelayDebugMsg ("Saving", 22);

  m_iHyperlinkColour = page22.m_iHyperlinkColour;
  m_iMXPdebugLevel   = page22.m_iMXPdebugLevel;
  m_bPueblo          = page22.m_bPueblo;
  m_iUseMXP          = page22.m_iUseMXP;
  m_bHyperlinkAddsToCommandHistory  = page22.m_bHyperlinkAddsToCommandHistory;
  m_bEchoHyperlinkInOutputWindow    = page22.m_bEchoHyperlinkInOutputWindow;
  m_bUseCustomLinkColour             =  page22.m_bUseCustomLinkColour       ;
  m_bMudCanChangeLinkColour          =  page22.m_bMudCanChangeLinkColour    ;
  m_bUnderlineHyperlinks             =  page22.m_bUnderlineHyperlinks       ;
  m_bMudCanRemoveUnderline           =  page22.m_bMudCanRemoveUnderline     ;
  m_bIgnoreMXPcolourChanges          =  page22.m_bIgnoreMXPcolourChanges  ;
  m_bSendMXP_AFK_Response            =  page22.m_bSendMXP_AFK_Response  ;
  m_bMudCanChangeOptions             =  page22.m_bMudCanChangeOptions  ;
                                    
  // turn MXP on or off if specifically requested
  if (m_iUseMXP == eNoMXP && m_bMXP)
    MXP_Off (true);
  else if (m_iUseMXP == eUseMXP && !m_bMXP)
    MXP_On (false, true); // not pueblo, manually on

  }   // end of CMUSHclientDoc::SavePrefsP22

void CMUSHclientDoc:: SavePrefsP23 (CPrefsP23 &page23)  
  {  
  DelayDebugMsg ("Saving", 23);
  unsigned short iOldPort = m_IncomingChatPort;

  m_bAutoAllowSnooping               = page23.m_bAutoAllowSnooping               ;
  m_bAcceptIncomingChatConnections   = page23.m_bAcceptIncomingChatConnections   ;
  m_IncomingChatPort                 = (unsigned short) page23.m_IncomingChatPort                 ;
  m_bValidateIncomingCalls           = page23.m_bValidateIncomingCalls           ;
  m_bIgnoreChatColours               = page23.m_bIgnoreChatColours               ;
  m_strChatMessagePrefix             = page23.m_strChatMessagePrefix             ;
  m_iMaxChatLinesPerMessage          = page23.m_iMaxChatLinesPerMessage          ;
  m_iMaxChatBytesPerMessage          = page23.m_iMaxChatBytesPerMessage          ;
  m_bAutoAllowFiles                  = page23.m_bAutoAllowFiles                  ;
  m_strChatSaveDirectory             = page23.m_strChatSaveDirectory             ;
  m_cChatForegroundColour            = page23.m_iTextColour                     ;
  m_cChatBackgroundColour            = page23.m_iBackColour                     ; 

  // start accepting if required
  if (m_bAcceptIncomingChatConnections)
    {
    // stop old connection
    if (m_pChatListenSocket && iOldPort != m_IncomingChatPort)
      ChatStopAcceptingCalls ();

    // start new one
    ChatAcceptCalls (0);
    } // end need to start accepting calls
  else
    {
    // stop accepting if required
     if (m_pChatListenSocket)
      ChatStopAcceptingCalls ();
    } // end need to stop accepting calls

  // change chat name if necessary

  if (page23.m_strOurChatName != m_strOurChatName)
     ChatNameChange (page23.m_strOurChatName);

  m_strOurChatName                   = page23.m_strOurChatName                   ;

  }   // end of CMUSHclientDoc::SavePrefsP23



void CMUSHclientDoc::OnGameConfigureMudaddress() 
{
GamePreferences (ePageGeneral);
}

void CMUSHclientDoc::OnGameConfigureNameAndPassword() 
{
GamePreferences (ePageConnecting);
}

void CMUSHclientDoc::OnGameConfigureLogging() 
{
GamePreferences (ePageLogging);
}

void CMUSHclientDoc::OnGameConfigureChat() 
{
GamePreferences (ePageChat);
}

void CMUSHclientDoc::OnGameConfigureInfo() 
{
GamePreferences (ePageInfo);
}

void CMUSHclientDoc::OnGameConfigureNotes() 
{
GamePreferences (ePageNotes);
}


void CMUSHclientDoc::OnGameConfigureOutput() 
{
GamePreferences (ePageOutput);
}

void CMUSHclientDoc::OnGameConfigureColours() 
{
GamePreferences (ePageANSIColours);
}


void CMUSHclientDoc::OnGameConfigureCustomColours() 
{
GamePreferences (ePageCustomColours);
}

void CMUSHclientDoc::OnGameConfigureHighlighting() 
{
//GamePreferences (ePageSounds);
}


void CMUSHclientDoc::OnGameConfigurePrinting() 
{
GamePreferences (ePagePrinting);
}


void CMUSHclientDoc::OnGameConfigureCommands() 
{
GamePreferences (ePageCommands);
}

void CMUSHclientDoc::OnGameConfigureKeypad() 
{
GamePreferences (ePageKeypad);
}

void CMUSHclientDoc::OnGameConfigureMacros() 
{
GamePreferences (ePageMacros);
}

void CMUSHclientDoc::OnGameConfigureAutosay() 
{
GamePreferences (ePageAutosay);
}


void CMUSHclientDoc::OnGameConfigurePastetoworld() 
{
GamePreferences (ePagePaste);
}

void CMUSHclientDoc::OnGameConfigureSendfile() 
{
GamePreferences (ePageSend);
}


void CMUSHclientDoc::OnGameConfigureScripting() 
{
GamePreferences (ePageScripts);
}

void CMUSHclientDoc::OnGameConfigureVariables() 
{
GamePreferences (ePageVariables);
}


void CMUSHclientDoc::OnGameConfigureTriggers() 
{
GamePreferences (ePageTriggers);
}

void CMUSHclientDoc::OnGameConfigureAliases() 
{
GamePreferences (ePageAliases);
}

void CMUSHclientDoc::OnGameConfigureTimers() 
{
GamePreferences (ePageTimers);
}


void CMUSHclientDoc::OnGameConfigureMxppueblo() 
{
GamePreferences (ePageMXP);
}


bool CMUSHclientDoc::GamePreferences (const int iPage) 
{
Frame.DelayDebugStatus ("Starting world config");

int iWantedPage = iPage;

  if (iWantedPage == -1)
    iWantedPage = m_last_prefs_page;

CString str = AppendMUDName ("Configuration");

CTreePropertySheet sheet (150, str, NULL, iWantedPage);

  // sheet setup

  sheet.m_clrCaptionLeft = RGB (140, 0, 0);       // red
  if (App.m_bColourGradient)
    sheet.m_clrCaptionRight = RGB (255, 255, 0);    // yellow
  else
    sheet.m_clrCaptionRight = sheet.m_clrCaptionLeft;   // no gradient
  sheet.m_clrCaptionTextLeft = RGB (255, 255, 255);     // white

Frame.DelayDebugStatus ("World config - constructing pages");

  // dividers

CPrefsP0 page0;       // name
CPrefsP0 page00;      // appearance
CPrefsP0 page000;     // input
//CPrefsP0 page0000;    // paste
CPrefsP0 page00000;   // scripting

// name

CPrefsP1 page1;
CPrefsP4 page4;
CPrefsP11 page11;
CPrefsP15 page15;
CPrefsP21 page21;
CPrefsP23 page23;

// appearance

// CPrefsP2 page2;  // sounds
CPrefsP3 page3;
CPrefsP5 page5;
CPrefsP14 page14;
CPrefsP20 page20;
CPrefsP22 page22;

// input

CPrefsP6 page6;
CPrefsP9 page9;
CPrefsP12 page12;
CPrefsP19 page19;

// triggers/aliases

CPrefsP7 page7;
CPrefsP8 page8;
CPrefsP16 page16;

// paste

CPrefsP10 page10;
CPrefsP13 page13;

// scripting

CPrefsP17 page17;
CPrefsP18 page18;

Frame.DelayDebugStatus ("World config - loading pages");

// name
	
  LoadPrefsP1   (page1);
  LoadPrefsP4   (page4);
  LoadPrefsP11  (page11);
  LoadPrefsP15  (page15);
  LoadPrefsP21  (page21);
  LoadPrefsP16  (page16); // timers
  LoadPrefsP23  (page23); // chat

// appearance

//  LoadPrefsP2   (page2);  // sounds
  LoadPrefsP3   (page3);
  LoadPrefsP5   (page5);
  LoadPrefsP14  (page14);
  LoadPrefsP20  (page20);
  LoadPrefsP22  (page22);
  LoadPrefsP8   (page8);    // triggers
  page3.m_prefsp5 = &page5;   // so we can show ANSI colours
  page5.m_prefsp3 = &page3;   // so we can look up colours
  page8.m_prefsp3 = &page3;   // so we can look up colours
  page8.m_prefsp14 = &page14;   // so we can look up whether bold/italic/underline is enabled

// input

  LoadPrefsP6   (page6);
  LoadPrefsP9   (page9);
  LoadPrefsP12  (page12);
  LoadPrefsP19  (page19);
  LoadPrefsP7   (page7);    // aliases
  page9.m_prefsp3 = &page3;   // so we can look up colours
  
// triggers/aliases


// paste

  LoadPrefsP10 (page10);
  LoadPrefsP13 (page13);

  // scripting

  LoadPrefsP17 (page17);
  page17.m_prefsp3 = &page3;   // so we can look up colours

  LoadPrefsP18 (page18);

  // separator pages
  page0.m_doc = this;
  page00.m_doc = this;
  page000.m_doc = this;
//  page0000.m_doc = this;
  page00000.m_doc = this;


  Frame.DelayDebugStatus ("World config - adding pages to property sheet");

// Add pages to property sheet. Their page numbers will depend on the order
// in which they are added.

  // NB pages must be added in the sequence in the enum in doc.h

  // name

  // do they want the nodes expanded?
  int iExpand = App.m_bAutoExpand ? tps_item_expanded : 0;

  sheet.AddPage (tps_item_branch | iExpand, &page0, "General");  
  page0.m_page_number  = ePageGroup0; // group  
  sheet.AddPage (tps_item_node, &page1, "IP address");  
  page1.m_page_number  = ePageGeneral; // general  
  sheet.AddPage (tps_item_node, &page21, "Connecting"); 
  page21.m_page_number = ePageConnecting; // connecting 
  sheet.AddPage (tps_item_node, &page4, "Logging");  
  page4.m_page_number  = ePageLogging; // logging 
  sheet.AddPage (tps_item_node, &page16, "Timers"); 
  page16.m_page_number = ePageTimers; // timers 
  sheet.AddPage (tps_item_node, &page23, "Chat"); 
  page23.m_page_number = ePageChat; // chat 
  sheet.AddPage (tps_item_node, &page15, "Info"); 
  page15.m_page_number = ePageInfo; // info 
  sheet.AddPage (tps_item_endbranch, &page11, "Notes"); 
  page11.m_page_number = ePageNotes; // notes 

  // appearance

  sheet.AddPage (tps_item_branch | iExpand, &page00, "Appearance");  
  page00.m_page_number  = ePageGroup00; // group 
  sheet.AddPage (tps_item_node, &page14, "Output"); 
  page14.m_page_number = ePageOutput; // output 
  sheet.AddPage (tps_item_node, &page22, "MXP / Pueblo");  
  page22.m_page_number = ePageMXP; // MXP 
  sheet.AddPage (tps_item_node, &page5, "ANSI Colour");  
  page5.m_page_number  = ePageANSIColours; // ANSI colours 
  sheet.AddPage (tps_item_node, &page3, "Custom Colour");  
  page3.m_page_number  = ePageCustomColours; // custom colours 
  sheet.AddPage (tps_item_node, &page8, "Triggers");  
  page8.m_page_number  = ePageTriggers; // triggers 
//  sheet.AddPage (tps_item_node, &page2, "Sounds");  
//  page2.m_page_number  = ePageSounds; // sounds 
  sheet.AddPage (tps_item_endbranch, &page20, "Printing"); 
  page20.m_page_number = ePagePrinting; // printing  

  // input

  sheet.AddPage (tps_item_branch | iExpand, &page000, "Input");  
  page000.m_page_number  = ePageGroup000; // group 
  sheet.AddPage (tps_item_node, &page9, "Commands");  
  page9.m_page_number  = ePageCommands; // commands 
  sheet.AddPage (tps_item_node, &page7, "Aliases");  
  page7.m_page_number  = ePageAliases; // aliases 
  sheet.AddPage (tps_item_node, &page12, "Keypad"); 
  page12.m_page_number = ePageKeypad; // keypad 
  sheet.AddPage (tps_item_node, &page6, "Macros");  
  page6.m_page_number  = ePageMacros; // macros 
  sheet.AddPage (tps_item_node, &page19, "Auto Say"); 
  page19.m_page_number = ePageAutosay; // auto say 


  // paste

//  sheet.AddPage (tps_item_branch | iExpand, &page0000, "Paste/Send");  
//  page0000.m_page_number  = ePageGroup0000; // group 
  sheet.AddPage (tps_item_node, &page13, "Paste"); 
  page13.m_page_number = ePagePaste; // paste 
  sheet.AddPage (tps_item_endbranch, &page10, "Send"); 
  page10.m_page_number = ePageSend; // send 

  // scripting

  sheet.AddPage (tps_item_branch | iExpand, &page00000, "Scripting");  
  page00000.m_page_number  = ePageGroup00000; // group 
//  if (!bWine)   // allow for Lua's benefit
    {
    sheet.AddPage (tps_item_node, &page17, "Scripts"); 
    page17.m_page_number = ePageScripts; // scripts 
    }
  sheet.AddPage (tps_item_endbranch, &page18, "Variables"); 
  page18.m_page_number = ePageVariables; // variables 
      
    // this is truly a wank, however this is the only way
  // I can get the Help button to work if they have disabled F1 = help
  bool bSavedF1Flag = App.m_bF1macro;
  App.m_bF1macro = false;

  Frame.DelayDebugStatus ("World config - showing dialog");

  // do preferences page
  int iResult = sheet.DoModal ();

  Frame.DelayDebugStatus ("World config - dialog done");
  
  // now put it back - bluurrrggghhh!!!
  App.m_bF1macro = bSavedF1Flag;

  if (iResult != IDOK)
    {
    Frame.SetStatusNormal ();
    return false;
    }

  Frame.DelayDebugStatus ("World config - dialog not cancelled");

// make sure we re-evaluate the IP address if the server address changes

  if (m_server != page1.m_server)
    	ZeroMemory(&m_sockAddr, sizeof m_sockAddr);

// ditto for proxy server address

  if (m_strProxyServerName != page1.m_strProxyServerName)
    	ZeroMemory(&m_ProxyAddr, sizeof m_ProxyAddr);

  Frame.DelayDebugStatus ("World config - fixing fonts");

// change input font if necessary

  if (m_input_font_height != page9.m_input_font_height ||  
      m_input_font_name   != page9.m_input_font_name ||    
      m_input_font_italic != page9.m_input_font_italic ||  
      m_input_font_weight != page9.m_input_font_weight ||
      m_input_font_charset != page9.m_input_font_charset ||
      m_bUseDefaultInputFont != page9.m_bUseDefaultInputFont)
        ChangeInputFont (page9.m_input_font_height, 
                    page9.m_input_font_name, 
                    page9.m_input_font_weight, 
                    page9.m_input_font_charset,
                    page9.m_input_font_italic);

// update font in all views, if we changed it

  if (m_font_height != page14.m_font_height ||  
      m_font_name   != page14.m_font_name ||    
      m_font_charset!= page14.m_font_charset ||    
      m_font_weight != page14.m_font_weight ||
      m_bShowBold   != page14.m_bShowBold ||
      m_bShowItalic   != page14.m_bShowItalic ||
      m_bShowUnderline   != page14.m_bShowUnderline ||
      m_iLineSpacing   != page14.m_iLineSpacing ||
      m_bUseDefaultOutputFont != page14.m_bUseDefaultOutputFont)
        ChangeFont (page14.m_font_height, 
                    page14.m_font_name, 
                    page14.m_font_weight, 
                    page14.m_font_charset,
                    page14.m_bShowBold,
                    page14.m_bShowItalic,
                    page14.m_bShowUnderline,
                    page14.m_iLineSpacing);

  Frame.DelayDebugStatus ("World config - checking if world changed");

  if (
      // name

      ChangedPrefsP1  (page1) ||
      ChangedPrefsP4  (page4) ||
      ChangedPrefsP11 (page11) ||
      ChangedPrefsP15 (page15) ||
      ChangedPrefsP21 (page21) ||
      ChangedPrefsP23 (page23) ||

      // appearance

//      ChangedPrefsP2 (page2) ||
      ChangedPrefsP3 (page3) ||
      ChangedPrefsP5 (page5) ||
      ChangedPrefsP14 (page14) ||
      ChangedPrefsP20 (page20) ||
      ChangedPrefsP22 (page22) ||

      // input

      ChangedPrefsP6 (page6) ||
      ChangedPrefsP9 (page9) ||
      ChangedPrefsP12 (page12) ||
      ChangedPrefsP19 (page19) ||

      // triggers/aliases

      ChangedPrefsP7 (page7) ||
      ChangedPrefsP8 (page8) ||
      ChangedPrefsP16 (page16) ||

      // paste

      ChangedPrefsP10 (page10) ||
      ChangedPrefsP13 (page13) ||

      // scripting

      ChangedPrefsP17 (page17)  ||
      ChangedPrefsP18 (page18)
      

      
      )  
        SetModifiedFlag (TRUE);


  // I want this for the side-effect so I can't afford for the short-circuit
  // boolean evaluation to bypass it

  if (ChangedPrefsP20 (page20))
      SetModifiedFlag (TRUE);
  
  Frame.DelayDebugStatus ("World config - updating views");

  // update all views in case font or colours changed

  UpdateAllViews (NULL);

// save new values in the document

  Frame.DelayDebugStatus ("World config - saving pages");

  // name

  SavePrefsP1   (page1);
  SavePrefsP4   (page4);
  SavePrefsP11  (page11);
  SavePrefsP15  (page15);
  SavePrefsP21  (page21);
  SavePrefsP23  (page23);
  
  // appearance

//  SavePrefsP2   (page2);
  SavePrefsP3   (page3);
  SavePrefsP5   (page5);
  SavePrefsP14  (page14);
  SavePrefsP20  (page20);
  SavePrefsP22  (page22);

  // input

  SavePrefsP6   (page6);
  SavePrefsP9   (page9);
  SavePrefsP12  (page12);
  SavePrefsP19  (page19);

  // triggers/aliases

  SavePrefsP7   (page7);
  SavePrefsP8   (page8);
  SavePrefsP16  (page16);

  // paste

  SavePrefsP10 (page10);
  SavePrefsP13 (page13);

  // scripting

  SavePrefsP17 (page17);
  SavePrefsP18 (page18);

  Frame.SetStatusNormal ();

  return true;    // did it OK
}


void CMUSHclientDoc::OnGamePreferences() 
  {
  GamePreferences (-1);  // use last page, whatever it was
  }
