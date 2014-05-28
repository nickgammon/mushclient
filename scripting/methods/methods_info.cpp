// methods_info.cpp

// General "information" methods

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\..\MUSHview.h"
#include "..\..\mainfrm.h"
#include "..\..\childfrm.h"
#include "..\..\sendvw.h"
#include "..\..\winplace.h"
#include "..\..\png\png.h"
#include "..\errors.h"
#include <direct.h>

// Implements:

//    ErrorDesc
//    GetConnectDuration
//    GetDeviceCaps
//    GetGlobalOption
//    GetGlobalOptionList
//    GetHostAddress
//    GetHostName
//    GetInfo
//    GetLineCount
//    GetLineInfo
//    GetLinesInBufferCount
//    GetMainWindowPosition
//    GetNotes
//    GetReceivedBytes
//    GetScriptTime
//    GetSelectionEndColumn
//    GetSelectionEndLine
//    GetSelectionStartColumn
//    GetSelectionStartLine
//    GetSentBytes
//    GetStyleInfo
//    GetSysColor
//    GetSystemMetrics
//    GetWorldID
//    GetWorldWindowPosition
//    IsConnected
//    Version
//    WorldAddress
//    WorldName
//    WorldPort

extern char working_dir [_MAX_PATH];
extern char file_browsing_dir [_MAX_PATH];

// world.GetLineCount - returns the count of lines received by this world

long CMUSHclientDoc::GetLineCount() 
{
	return m_total_lines;
}  // end of CMUSHclientDoc::GetLineCount


// world.IsConnected - returns true if the world is open

BOOL CMUSHclientDoc::IsConnected() 
{
	return m_iConnectPhase == eConnectConnectedToMud;
}  // end of CMUSHclientDoc::IsConnected


// world.worldname - returns the name of the current world

BSTR CMUSHclientDoc::WorldName() 
{
	CString strResult;

  strResult = m_mush_name;

	return strResult.AllocSysString();
}  // end of CMUSHclientDoc::WorldName


// world.version - returns MUSHclient version

BSTR CMUSHclientDoc::Version() 
{
	CString strResult = MUSHCLIENT_VERSION;

	return strResult.AllocSysString();
}   // end of CMUSHclientDoc::Version


tInfoTypeMapping InfoTypes [] =
 
   {

//   (strings - configuration)

  { 1, "Server name" },
  { 2, "World name" },
  { 3, "Character name" },
  { 4, "Send to world - file preamble" },
  { 5, "Send to world - file postamble" },
  { 6, "Send to world - line preamble" },
  { 7, "Send to world - line postamble" },
  { 8, "World notes" },
  { 9, "Sound on new activity" },
 { 10, "Scripts editor" },
 { 11, "Log file preamble" },
 { 12, "Log file postamble" },
 { 13, "Log file line preamble - player input" },
 { 14, "Log file line preamble - notes" },
 { 15, "Log file line preamble - MUD output" },
 { 16, "Log file line postamble - player input" },
 { 17, "Log file line postamble - notes" },
 { 18, "Log file line postamble - MUD output" },
 { 19, "Speed Walk Filler" },
 { 20, "Output window font name" },
 { 21, "Speed walk prefix" },
 { 22, "Text sent on connecting" },
 { 23, "Input font name" },
 { 24, "Paste to world - file preamble" },
 { 25, "Paste to world - file postamble" },
 { 26, "Paste to world - line preamble" },
 { 27, "Paste to world - line postamble" },
 { 28, "Scripting language" },
 { 29, "Script function for world open" },
 { 30, "Script function for world close" },
 { 31, "Script function for world connect" },
 { 32, "Script function for world disconnect" },
 { 31, "Script function for world get focus" },
 { 32, "Script function for world lose focus" },
 { 35, "Script file name" },
 { 36, "Scripting prefix" },
 { 37, "Auto-say string" },
 { 38, "Auto-say override" },
 { 39, "Tab-completion defaults" },
 { 40, "Auto-log file name" },
 { 41, "Recall window - line preamble" },
 { 42, "Terminal ID (telnet negotiation)" },
 { 43, "Mapping failure message" },
 { 44, "Script function for MXP starting up" },
 { 45, "Script function for MXP closing down" },
 { 46, "Script function for MXP error" },
 { 47, "Script function for MXP tag open" },
 { 48, "Script function for MXP tag close" },
 { 49, "Script function for MXP variable set" },
 { 50, "Sound to play for beeps" },

// (strings - calculated at runtime)
 
 { 51, "Current log file name" },
 { 52, "Last \"immediate\" script expression" },
 { 53, "Current status line message" },
 { 54, "World file pathname" },
 { 55, "World title" },
 { 56, "MUSHclient application path name" },
 { 57, "World files default path (directory)" },
 { 58, "Log files default path (directory)" },
 { 59, "Script files default path (directory)" },
 { 60, "Plugin files default path (directory)" },
 { 61, "World IP address" },
 { 62, "Proxy server IP address" },
 { 63, "Host name (name of this PC)" },
 { 64, "Current directory path" },
 { 65, "Script function for world save" },
 { 66, "MUSHclient application directory" },
 { 67, "World file directory" },
 { 68, "Startup directory" },
 { 69, "Translation file" },
 { 70, "Locale" },
 { 71, "Fixed Pitch Font" },
 { 72, "MUSHclient version" },
 { 73, "MUSHclient compilation date/time" },
 { 74, "Sounds directory" },
 { 75, "Last telnet subnegotiation string received" },
 { 76, "Special font pathname" },
 { 77, "OS Version - service pack" },
 { 78, "Foreground image filename" },
 { 79, "Background image filename" },
 { 80, "LibPNG version number" },
 { 81, "LibPNG header version string" },
 { 82, "Preferences database pathname" },
 { 83, "SQLite3 version" },
 { 84, "File browsing directory" },
 { 85, "State files path (directory)" },
 { 86, "Word under mouse on mouse menu click" },
 { 87, "Last command sent" },
 { 88, "Window title last set by SetTitle" },
 { 89, "Window title last set by SetMainTitle" },


 
// (booleans - calculated at runtime)
 
{ 101, "No Echo" },
{ 102, "Debug incoming packets" },
{ 103, "Decompressing" },
{ 104, "MXP active" },
{ 105, "Pueblo active" },
{ 106, "World close flag" },
{ 107, "Currently-connecting" },
{ 108, "OK-to-disconnect" },
{ 109, "Trace" },
{ 110, "Script file changed" },
{ 111, "World file is modified" },
{ 112, "Automapper active" },
{ 113, "World is active" },
{ 114, "Output window paused" },
{ 115, "Localization active" },
{ 118, "Variables have changed" },
{ 119, "Script engine active" },
{ 120, "Scroll bar visible" },
{ 121, "High-resolution timer available" },
{ 122, "SQLite3 is thread-safe" },



// (numbers (longs) - calculated at runtime)

{ 201, "Total lines received" },
{ 202, "Lines received but not yet seen (new lines)" },
{ 203, "Total lines sent" },
{ 204, "Packets received" },
{ 205, "Packets sent" },
{ 206, "Total uncompressed bytes received" },
{ 207, "Total compresssed bytes received" },
{ 208, "MCCP protocol in use (0=none, 1 or 2)" },
{ 209, "MXP error count" },
{ 210, "MXP tags received" },
{ 211, "MXP entities received" },
{ 212, "Output font height" },
{ 213, "Output font width" },
{ 214, "Input font height" },
{ 215, "Input font width" },
{ 216, "Total bytes received" },
{ 217, "Total bytes sent" },
{ 218, "Count of variables" },
{ 219, "Count of triggers" },
{ 220, "Count of timers" },
{ 221, "Count of aliases" },
{ 222, "Count of queued commands" },
{ 223, "Count of mapper items" },
{ 224, "Count of lines in output window" },
{ 225, "Count of custom MXP elements" },
{ 226, "Count of custom MXP entities" },
{ 227, "Connect phase" },
{ 228, "World IP address (as long)" },
{ 229, "Proxy server IP address (as long)" },
{ 230, "Script execution depth" },
{ 231, "Log file size" },
{ 232, "High-performance timer" },        // double
{ 233, "Trigger execution time" },        // double
{ 234, "Alias execution time" },          // double
{ 235, "Number of world windows" },
{ 236, "Command window start selection column" },
{ 237, "Command window end selection column" },
{ 238, "World window display flags" },
{ 239, "Source of current action" },
{ 240, "Average character width in output window" },
{ 241, "Font height in output window" },
{ 242, "Number of lines received with bad UTF-8 in them" },
{ 243, "Fixed Pitch Font Size" },
{ 244, "Count of triggers evaluated" },
{ 245, "Count of triggers matched" },
{ 246, "Count of aliases evaluated" },
{ 247, "Count of aliases matched" },
{ 248, "Count of timers fired" },
{ 249, "Main frame window height" },
{ 250, "Main frame window width" },
{ 251, "Main toolbar window height" },
{ 252, "Main toolbar window width" },
{ 253, "Game toolbar window height" },
{ 254, "Game toolbar window width" },
{ 255, "Activity toolbar window height" },
{ 256, "Activity toolbar window width" },
{ 257, "Info bar window height" },
{ 258, "Info bar window width" },
{ 259, "Status bar window height" },
{ 260, "Status bar window width" },
{ 261, "World window non-client height" },
{ 262, "World window non-client width" },
{ 263, "World window client height" },
{ 264, "World window client width" },
{ 265, "OS Version - major" },
{ 266, "OS Version - minor" },
{ 267, "OS Version - build number" },
{ 268, "OS Version - platform ID" },
{ 269, "Foreground image mode" },
{ 270, "Background image mode" },
{ 271, "Background colour" },
{ 272, "Text rectangle - left" },
{ 273, "Text rectangle - top" },
{ 274, "Text rectangle - right" },
{ 275, "Text rectangle - bottom" },
{ 276, "Text rectangle - border offset" },
{ 277, "Text rectangle - border width" },
{ 278, "Text rectangle - outside colour" },
{ 279, "Text rectangle - outside style" },
{ 280, "Output window client height" },
{ 281, "Output window client width" },
{ 282, "Text rectangle - border colour" },
{ 283, "Mouse position - X" },
{ 284, "Mouse position - Y" },
{ 285, "Output window available" },
{ 286, "Triggers matched this session" },
{ 287, "Aliases matched this session" },
{ 288, "Timers fired this session" }, 
{ 289, "Last line with IAC/GA or IAC/EOR in it" },
{ 290, "Actual text rectangle - left" },
{ 291, "Actual text rectangle - top" },
{ 292, "Actual text rectangle - right" },
{ 293, "Actual text rectangle - bottom" },
{ 294, "State of keyboard modifiers" },
{ 295, "Times output window redrawn" },
{ 296, "Output window scroll bar position" },
{ 297, "High-resolution timer frequency" },
{ 298, "SQLite3 version number" },
{ 299, "ANSI code-page number" },
{ 300, "OEM code-page number" },

   

// (dates - calculated at runtime)

{ 301, "Time connected" },
{ 302, "Time log file was last flushed to disk" },
{ 303, "When script file was last modified" },
{ 304, "Time now" },
{ 305, "When client started executing" },
{ 306, "When this world was created/opened" },


// more numbers

{ 310, "Newlines received" },


 { 0, "" }, // end of table marker

 };

static void GetWindowWidth (CWnd * pWnd, VARIANT & vaResult, const bool client = false )
  {
  RECT rect;
  if (pWnd->m_hWnd)
    {
    if (client)
      pWnd->GetClientRect(&rect);
    else
      {
      pWnd->GetWindowRect(&rect);
      pWnd->ScreenToClient (&rect);
      }
    }
  else
    rect.right = -1;
  VariantClear (&vaResult);
  vaResult.vt = VT_I4;
  vaResult.lVal = rect.right; 
  } // end of GetWindowWidth

static void GetWindowHeight (CWnd * pWnd, VARIANT & vaResult, const bool client = false )
  {
  RECT rect;
  rect.left = 0;
  rect.top = 0;
  rect.right = 0;
  rect.bottom = 0;

  if (pWnd->m_hWnd)
    {
    if (client)
      pWnd->GetClientRect(&rect);
    else
      {
      pWnd->GetWindowRect(&rect);
      pWnd->ScreenToClient (&rect);
      }
    }
  else
    rect.right = -1;
  VariantClear (&vaResult);
  vaResult.vt = VT_I4;
  vaResult.lVal = rect.bottom; 
  }   // end of GetWindowHeight

// helper function
CMUSHView * CMUSHclientDoc::GetFirstOutputWindow ()
  {
  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
      return  (CMUSHView*)pView;
    }   // end of loop through views

  return NULL;      // not found
  }   // end of CMUSHclientDoc::GetFirstOutputWindow

VARIANT CMUSHclientDoc::GetInfo(long InfoType) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);


  vaResult.vt = VT_EMPTY;   // in case dates are zero

  switch (InfoType)
    {
    case    1: SetUpVariantString (vaResult, m_server); break;
    case    2: SetUpVariantString (vaResult, m_mush_name); break;
    case    3: SetUpVariantString (vaResult, m_name); break;
    case    4: SetUpVariantString (vaResult, m_file_preamble); break;
    case    5: SetUpVariantString (vaResult, m_file_postamble); break;
    case    6: SetUpVariantString (vaResult, m_line_preamble); break;
    case    7: SetUpVariantString (vaResult, m_line_postamble); break;
    case    8: SetUpVariantString (vaResult, m_notes); break;
    case    9: SetUpVariantString (vaResult, m_new_activity_sound); break;
    case   10: SetUpVariantString (vaResult, m_strScriptEditor); break;

    case   11: SetUpVariantString (vaResult, m_strLogFilePreamble); break;
    case   12: SetUpVariantString (vaResult, m_strLogFilePostamble); break;
    case   13: SetUpVariantString (vaResult, m_strLogLinePreambleInput); break;
    case   14: SetUpVariantString (vaResult, m_strLogLinePreambleNotes); break;
    case   15: SetUpVariantString (vaResult, m_strLogLinePreambleOutput); break;
    case   16: SetUpVariantString (vaResult, m_strLogLinePostambleInput); break;
    case   17: SetUpVariantString (vaResult, m_strLogLinePostambleNotes); break;
    case   18: SetUpVariantString (vaResult, m_strLogLinePostambleOutput); break;
    case   19: SetUpVariantString (vaResult, m_strSpeedWalkFiller); break;
    case   20: SetUpVariantString (vaResult, m_font_name); break;

    case   21: SetUpVariantString (vaResult, m_speed_walk_prefix); break;
    case   22: SetUpVariantString (vaResult, m_connect_text); break;
    case   23: SetUpVariantString (vaResult, m_input_font_name); break;
    case   24: SetUpVariantString (vaResult, m_paste_postamble); break;
    case   25: SetUpVariantString (vaResult, m_paste_preamble); break;
    case   26: SetUpVariantString (vaResult, m_pasteline_postamble); break;
    case   27: SetUpVariantString (vaResult, m_pasteline_preamble); break;
    case   28: SetUpVariantString (vaResult, m_strLanguage); break;
    case   29: SetUpVariantString (vaResult, m_strWorldOpen); break;
    case   30: SetUpVariantString (vaResult, m_strWorldClose); break;

    case   31: SetUpVariantString (vaResult, m_strWorldConnect); break;
    case   32: SetUpVariantString (vaResult, m_strWorldDisconnect); break;
    case   33: SetUpVariantString (vaResult, m_strWorldGetFocus); break;
    case   34: SetUpVariantString (vaResult, m_strWorldLoseFocus); break;
    case   35: SetUpVariantString (vaResult, m_strScriptFilename); break;
    case   36: SetUpVariantString (vaResult, m_strScriptPrefix); break;
    case   37: SetUpVariantString (vaResult, m_strAutoSayString); break;
    case   38: SetUpVariantString (vaResult, m_strOverridePrefix); break;
    case   39: SetUpVariantString (vaResult, m_strTabCompletionDefaults); break;
    case   40: SetUpVariantString (vaResult, m_strAutoLogFileName); break;

    case   41: SetUpVariantString (vaResult, m_strRecallLinePreamble); break;
    case   42: SetUpVariantString (vaResult, m_strTerminalIdentification); break;
    case   43: SetUpVariantString (vaResult, m_strMappingFailure); break;
    case   44: SetUpVariantString (vaResult, m_strOnMXP_Start); break;
    case   45: SetUpVariantString (vaResult, m_strOnMXP_Stop); break;
    case   46: SetUpVariantString (vaResult, m_strOnMXP_Error); break;
    case   47: SetUpVariantString (vaResult, m_strOnMXP_OpenTag); break;
    case   48: SetUpVariantString (vaResult, m_strOnMXP_CloseTag); break;
    case   49: SetUpVariantString (vaResult, m_strOnMXP_SetVariable); break;
    case   50: SetUpVariantString (vaResult, m_strBeepSound); break;

    case   51: SetUpVariantString (vaResult, m_logfile_name); break;
    case   52: SetUpVariantString (vaResult, m_strLastImmediateExpression); break;
    case   53: SetUpVariantString (vaResult, m_strStatusMessage); break;
    case   54: SetUpVariantString (vaResult, GetPathName ()); break;
    case   55: SetUpVariantString (vaResult, GetTitle ()); break;

    case   56: SetUpVariantString (vaResult, App.m_strMUSHclientFileName); break;
    case   57: SetUpVariantString (vaResult, App.m_strDefaultWorldFileDirectory); break;
    case   58: SetUpVariantString (vaResult, App.m_strDefaultLogFileDirectory); break;
    case   59:   // scripts are in the execution directory at present
              {
              char fullfilename[MAX_PATH];
              char * p;
              GetModuleFileName (NULL, fullfilename, sizeof (fullfilename));
              p = strrchr (fullfilename, '\\');
              if (p)
                *p = 0;

              SetUpVariantString (vaResult, fullfilename); 
              break;
              }

    case   60: SetUpVariantString (vaResult, App.m_strPluginsDirectory); break;
    case   61: SetUpVariantString (vaResult, inet_ntoa (m_sockAddr.sin_addr)); break;
    case   62: SetUpVariantString (vaResult, inet_ntoa (m_ProxyAddr.sin_addr)); break;
    case   63:
              {
               char szHostName[256];

                if( gethostname(szHostName, sizeof szHostName) == 0 )
                   SetUpVariantString (vaResult, szHostName);

              }
              break;

    case   64:      // current directory
              {
               char * p = _getcwd (NULL, 1024);
               if (p)
                 {
                 CString strPath (p);
                 free (p);
                 if (strPath.Right (1) != "\\")
                   strPath += "\\"; // append trailing backslash
                 SetUpVariantString (vaResult, strPath);
                 }

              }
              break;

    case   65: SetUpVariantString (vaResult, m_strWorldSave); break;
    case   66: SetUpVariantString (vaResult, ExtractDirectory (App.m_strMUSHclientFileName)); break;
    case   67: SetUpVariantString (vaResult, ExtractDirectory (GetPathName ())); break;
    case   68: SetUpVariantString (vaResult, working_dir); break;
    case   69: SetUpVariantString (vaResult, App.m_strTranslatorFile); break;
    case   70: SetUpVariantString (vaResult, App.m_strLocale); break;
    case   71: SetUpVariantString (vaResult, App.m_strFixedPitchFont); break;
    case   72: SetUpVariantString (vaResult, MUSHCLIENT_VERSION); break;
    case   73: SetUpVariantString (vaResult, __DATE__ " " __TIME__); break;
    case   74: SetUpVariantString (vaResult, ExtractDirectory (App.m_strMUSHclientFileName) + "sounds\\"); break;
    case   75: SetUpVariantString (vaResult, m_IAC_subnegotiation_data.c_str ()); break;
    case   76: 
        {
        // for backwards compatibility, return the first special font
        ci_set::const_iterator iter = m_strSpecialFontName.begin ();
        if (iter !=  m_strSpecialFontName.end ())
          SetUpVariantString (vaResult, iter->c_str ()); 
        else
          SetUpVariantString (vaResult, ""); 
        break;
        }
    case   77: SetUpVariantString (vaResult, os_version.szCSDVersion);  break;

    case   78: SetUpVariantString (vaResult, m_strForegroundImageName); break;
    case   79: SetUpVariantString (vaResult, m_strBackgroundImageName); break;
    case   80: SetUpVariantString (vaResult, PNG_LIBPNG_VER_STRING); break;
    case   81: SetUpVariantString (vaResult, PNG_HEADER_VERSION_STRING); break;
    case   82: SetUpVariantString (vaResult, App.m_PreferencesDatabaseName.c_str ()); break;
    case   83: SetUpVariantString (vaResult, sqlite3_libversion ()); break;
    case   84: SetUpVariantString (vaResult, file_browsing_dir); break;
    case   85: SetUpVariantString (vaResult, App.m_strDefaultStateFilesDirectory); break;
    case   86: SetUpVariantString (vaResult, m_strWordUnderMenu); break;
    case   87: SetUpVariantString (vaResult, m_strLastCommandSent); break;
    case   88: SetUpVariantString (vaResult, m_strWindowTitle); break;
    case   89: SetUpVariantString (vaResult, m_strMainWindowTitle); break;
      
    case  101: SetUpVariantBool (vaResult, m_bNoEcho); break;
    case  102: SetUpVariantBool (vaResult, m_bDebugIncomingPackets); break;
    case  103: SetUpVariantBool (vaResult, m_bCompress); break;
    case  104: SetUpVariantBool (vaResult, m_bMXP); break;
    case  105: SetUpVariantBool (vaResult, m_bPuebloActive); break;
    case  106: SetUpVariantBool (vaResult, m_iConnectPhase != eConnectConnectedToMud); break;
    case  107: SetUpVariantBool (vaResult, m_iConnectPhase != eConnectNotConnected && // connecting
                                           m_iConnectPhase != eConnectConnectedToMud); break;
    case  108: SetUpVariantBool (vaResult, m_bDisconnectOK); break;
    case  109: SetUpVariantBool (vaResult, m_bTrace); break;
    case  110: SetUpVariantBool (vaResult, m_bInScriptFileChanged); break;
    case  111: SetUpVariantBool (vaResult, IsModified ()); break;
    case  112: SetUpVariantBool (vaResult, m_bMapping); break;
    case  113: SetUpVariantBool (vaResult, m_pActiveCommandView || m_pActiveOutputView); 
               break;
      
    case  114:   // current view frozen
               if (!(m_pActiveOutputView || m_pActiveCommandView)) 
                 SetUpVariantBool (vaResult, FALSE);
               else if (m_pActiveOutputView)
                 SetUpVariantBool (vaResult, m_pActiveOutputView->m_freeze);
               else 
                 SetUpVariantBool (vaResult, m_pActiveCommandView->m_topview->m_freeze);
               break;

    case  115: SetUpVariantBool (vaResult, App.m_Translator_Lua != NULL); break;
    case  118: SetUpVariantBool (vaResult, m_bVariablesChanged); break;
    case  119: SetUpVariantBool (vaResult, m_ScriptEngine != NULL); break;
    case  120: SetUpVariantBool (vaResult, m_bScrollBarWanted); break;
    case  121: SetUpVariantBool (vaResult, App.m_iCounterFrequency != 0); break;
    case  122: SetUpVariantBool (vaResult, sqlite3_threadsafe() != 0); break;

    case  201: SetUpVariantLong (vaResult, m_total_lines); break;
    case  202: SetUpVariantLong (vaResult, m_new_lines); break;
    case  203: SetUpVariantLong (vaResult, m_nTotalLinesSent); break;
    case  204: SetUpVariantLong (vaResult, (long) m_iInputPacketCount); break;
    case  205: SetUpVariantLong (vaResult, (long) m_iOutputPacketCount); break;
    case  206: SetUpVariantLong (vaResult, (long) m_nTotalUncompressed); break;
    case  207: SetUpVariantLong (vaResult, (long) m_nTotalCompressed); break;
    case  208: SetUpVariantLong (vaResult, m_iMCCP_type); break;
    case  209: SetUpVariantLong (vaResult, (long) m_iMXPerrors); break;
    case  210: SetUpVariantLong (vaResult, (long) m_iMXPtags); break;
      
    case  211: SetUpVariantLong (vaResult, (long) m_iMXPentities); break;
    case  212: SetUpVariantLong (vaResult, m_FontHeight); break;
    case  213: SetUpVariantLong (vaResult, m_FontWidth); break;
    case  214: SetUpVariantLong (vaResult, m_InputFontHeight); break;
    case  215: SetUpVariantLong (vaResult, m_InputFontWidth); break;
    case  216: SetUpVariantLong (vaResult, (long) m_nBytesIn); break;
    case  217: SetUpVariantLong (vaResult, (long) m_nBytesOut); break;
    case  218: SetUpVariantLong (vaResult, GetVariableMap ().GetCount ()); break;
    case  219: SetUpVariantLong (vaResult, GetTriggerMap ().GetCount ()); break;
    case  220: SetUpVariantLong (vaResult, GetTimerMap ().GetCount ()); break;
    case  221: SetUpVariantLong (vaResult, GetAliasMap ().GetCount ()); break;
    case  222: SetUpVariantLong (vaResult, m_QueuedCommandsList.GetCount ()); break;
    case  223: SetUpVariantLong (vaResult, m_strMapList.GetCount ()); break;
    case  224: SetUpVariantLong (vaResult, m_LineList.GetCount ()); break;
    case  225: SetUpVariantLong (vaResult, m_CustomElementMap.GetCount ()); break;
    case  226: SetUpVariantLong (vaResult, m_CustomEntityMap.GetCount ()); break;
    case  227: SetUpVariantLong (vaResult, m_iConnectPhase); break;
    case  228: SetUpVariantLong (vaResult, ntohl (m_sockAddr.sin_addr.s_addr)); break;
    case  229: SetUpVariantLong (vaResult, ntohl (m_ProxyAddr.sin_addr.s_addr)); break;
    case  230: SetUpVariantLong (vaResult, m_iExecutionDepth); break;
    case  231:  // log file size
      {
      if (m_logfile == NULL)
         SetUpVariantLong (vaResult, 0);  // no log file
      else
        {
        fpos_t pos;
        fgetpos (m_logfile, &pos);
        SetUpVariantLong (vaResult, pos);  // log file size
        } // end of log file

      }
      break;

    case 232:
      if (App.m_iCounterFrequency)
        {
        LARGE_INTEGER the_time;
        QueryPerformanceCounter (&the_time);
        double ticks = (double) the_time.QuadPart / (double) App.m_iCounterFrequency;
        SetUpVariantDouble (vaResult, ticks);
        }
      else
        {
        time_t timer;
        time (&timer);
        SetUpVariantDouble (vaResult, (double) timer);
        }

      break;

    case 233:     // time taken doing triggers
      if (App.m_iCounterFrequency)
        {
        LONGLONG iTimeTaken = 0;
        CTrigger * pTrigger;
        POSITION pos;
        double   elapsed_time;
        CString strName;

        // count number of triggers matched
        for (pos = GetTriggerMap ().GetStartPosition(); pos; )
          {
          GetTriggerMap ().GetNextAssoc (pos, strName, pTrigger);
          // calculate time taken to execute triggers
          if (pTrigger->regexp)
            iTimeTaken += pTrigger->regexp->iTimeTaken;
          }

        elapsed_time = ((double) iTimeTaken) / 
                        ((double) App.m_iCounterFrequency);

        SetUpVariantDouble (vaResult, elapsed_time);
        }
      break;

    case 234:     // time taken doing aliases
      if (App.m_iCounterFrequency)
        {
        LONGLONG iTimeTaken = 0;
        CAlias * pAlias;
        POSITION pos;
        double   elapsed_time;
        CString strName;

        // count number of triggers matched
        for (pos = GetAliasMap ().GetStartPosition(); pos; )
          {
          GetAliasMap ().GetNextAssoc (pos, strName, pAlias);
          // calculate time taken to execute triggers
          if (pAlias->regexp)
            iTimeTaken += pAlias->regexp->iTimeTaken;
          }

        elapsed_time = ((double) iTimeTaken) / 
                        ((double) App.m_iCounterFrequency);

        SetUpVariantDouble (vaResult, elapsed_time);
        }
      break;

    case 235:
      {
      long count = 0;

      for(POSITION pos=GetFirstViewPosition();pos!=NULL;)
        {
        CView* pView = GetNextView(pos);

        if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
          count++;
        }   // end of loop

      SetUpVariantLong (vaResult, count);  // number of world windows open

      }   // end of 235
      break;

    case 236:   // command selection start column
      {
        for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	        {
	        CView* pView = GetNextView(pos);
	        
	        if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	        {
		        CSendView* pmyView = (CSendView*)pView;
            int nStartChar;
            int nEndChar;

              // find where cursor is
  
            pmyView->GetEditCtrl().GetSel(nStartChar, nEndChar);	
            SetUpVariantLong (vaResult, nStartChar + 1);  // start column
            break;
  
	          }	  // end of being a CSendView
          }   // end of loop through views
      }
      break;

    case 237:   // command selection end column
      {
        for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	        {
	        CView* pView = GetNextView(pos);
	        
	        if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	        {
		        CSendView* pmyView = (CSendView*)pView;
            int nStartChar;
            int nEndChar;

              // find where cursor is
  
            pmyView->GetEditCtrl().GetSel(nStartChar, nEndChar);
            if (nEndChar <= nStartChar)
              nEndChar = 0;  // zero indicates no text selected
            SetUpVariantLong (vaResult, nEndChar);  // end column
            break;
  
	          }	  // end of being a CSendView
          }   // end of loop through views
      }
      break;

    case 238:
      {
      WINDOWPLACEMENT wp;

      for(POSITION pos=GetFirstViewPosition();pos!=NULL;)
        {
        CView* pView = GetNextView(pos);

        if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
          {
          CSendView* pmyView = (CSendView*)pView;

          pmyView->GetParentFrame ()->GetWindowPlacement(&wp); 
          SetUpVariantLong (vaResult, wp.showCmd);  // window placement flags
          break;

          }	
        }


      }
      break;

    case 239:
          SetUpVariantShort (vaResult, m_iCurrentActionSource);  // see enum
          break;

    case 240:

        SetUpVariantLong (vaResult, m_FontWidth);  // average character width
        break;

    case 241:
        SetUpVariantLong (vaResult, m_FontHeight);  // character height
        break;

    case 242:
        SetUpVariantLong (vaResult, m_iUTF8ErrorCount);  // errors in UTF-8
        break;

    case 243:
        SetUpVariantLong (vaResult, App.m_iFixedPitchFontSize);  
        break;

    case 244:
        SetUpVariantLong (vaResult, m_iTriggersEvaluatedCount);  
        break;

    case 245:
        SetUpVariantLong (vaResult, m_iTriggersMatchedCount);  
        break;

    case 246:
        SetUpVariantLong (vaResult, m_iAliasesEvaluatedCount);  
        break;

    case 247:
        SetUpVariantLong (vaResult, m_iAliasesMatchedCount);  
        break;

    case 248:
        SetUpVariantLong (vaResult, m_iTimersFiredCount);  
        break;

    case 249:
      {
      RECT rect;
      Frame.GetClientRect(&rect);
      SetUpVariantLong (vaResult, rect.bottom);  
      }
      break;

    case 250:
      {
      RECT rect;
      Frame.GetClientRect(&rect);
      SetUpVariantLong (vaResult, rect.right);  
      }
      break;

    case 251: GetWindowHeight (&Frame.m_wndToolBar, vaResult); break;
    case 252: GetWindowWidth  (&Frame.m_wndToolBar, vaResult); break;
    case 253: GetWindowHeight (&Frame.m_wndGameToolBar, vaResult); break;
    case 254: GetWindowWidth  (&Frame.m_wndGameToolBar, vaResult); break;
    case 255: GetWindowHeight (&Frame.m_wndActivityToolBar, vaResult); break;
    case 256: GetWindowWidth  (&Frame.m_wndActivityToolBar, vaResult); break;
    case 257: GetWindowHeight (&Frame.m_wndInfoBar, vaResult); break;
    case 258: GetWindowWidth  (&Frame.m_wndInfoBar, vaResult); break;
    case 259: GetWindowHeight (&Frame.m_wndStatusBar, vaResult); break;
    case 260: GetWindowWidth  (&Frame.m_wndStatusBar, vaResult); break;
    case 261: 
            {
            CChildFrame * pFrame = GetChildFrame ();
            if (pFrame)
              GetWindowHeight (pFrame, vaResult); break;  // non-client height
            break;
            }

    case 262: 
            {
            CChildFrame * pFrame = GetChildFrame ();
            if (pFrame)
              GetWindowWidth  (pFrame, vaResult);   // non-client width
            break;
            }

    case 263: 
            {
            CChildFrame * pFrame = GetChildFrame ();
            if (pFrame)
              GetWindowHeight (pFrame, vaResult, true); break;  // client height
            break;
            }

    case 264: 
            {
            CChildFrame * pFrame = GetChildFrame ();
            if (pFrame)
              GetWindowWidth  (pFrame, vaResult, true);   // client width
            break;
            }

    case 265: SetUpVariantLong (vaResult, os_version.dwMajorVersion); break;
    case 266: SetUpVariantLong (vaResult, os_version.dwMinorVersion); break;
    case 267: SetUpVariantLong (vaResult, os_version.dwBuildNumber);  break;
    case 268: SetUpVariantLong (vaResult, os_version.dwPlatformId);   break;

    case 269: SetUpVariantLong (vaResult, m_iForegroundMode);  break;
    case 270: SetUpVariantLong (vaResult, m_iBackgroundMode);  break;
    case 271: SetUpVariantLong (vaResult, m_iBackgroundColour);  break;

    case 272: SetUpVariantLong (vaResult, m_TextRectangle.left);  break;
    case 273: SetUpVariantLong (vaResult, m_TextRectangle.top);  break;
    case 274: SetUpVariantLong (vaResult, m_TextRectangle.right);  break;
    case 275: SetUpVariantLong (vaResult, m_TextRectangle.bottom);  break;
    case 276: SetUpVariantLong (vaResult, m_TextRectangleBorderOffset);  break;
    case 277: SetUpVariantLong (vaResult, m_TextRectangleBorderWidth);  break;
    case 278: SetUpVariantLong (vaResult, m_TextRectangleOutsideFillColour);  break;
    case 279: SetUpVariantLong (vaResult, m_TextRectangleOutsideFillStyle);  break;

    case 280:
      {
      CMUSHView* pmyView = GetFirstOutputWindow ();
      if (pmyView) 
        {
        RECT rect;
        pmyView->GetClientRect(&rect);
        SetUpVariantLong (vaResult, rect.bottom);
        break;
        }
      }
      break;

    case 281:
      {
      CMUSHView* pmyView = GetFirstOutputWindow ();
      if (pmyView) 
        {
        RECT rect;
        pmyView->GetClientRect(&rect);
        SetUpVariantLong (vaResult, rect.right);
        break;
        }
      }
      break;

    case 282: SetUpVariantLong (vaResult, m_TextRectangleBorderColour);  break;

    case 283: SetUpVariantLong (vaResult, m_lastMousePosition.x);  break; //  last mouse x position
    case 284: SetUpVariantLong (vaResult, m_lastMousePosition.y);  break; //  last mouse y position
    case 285: SetUpVariantBool (vaResult, m_pCurrentLine != NULL); break; //  is output window available?

    case 286: SetUpVariantLong (vaResult, m_iTriggersMatchedThisSessionCount);  break; // Triggers matched this session
    case 287: SetUpVariantLong (vaResult, m_iAliasesMatchedThisSessionCount);   break; // Aliases matched this session   
    case 288: SetUpVariantLong (vaResult, m_iTimersFiredThisSessionCount);      break; // Timers fired this session      
    case 289: SetUpVariantLong (vaResult, m_last_line_with_IAC_GA);             break; // Last line number that had an IAC/GA in it     

    case 290:
      {
      CMUSHView* pmyView = GetFirstOutputWindow ();
      if (pmyView) 
        {
        RECT rect = pmyView->GetTextRectangle ();
        SetUpVariantLong (vaResult, rect.left);
        break;
        }
      }
      break;

    case 291:
      {
      CMUSHView* pmyView = GetFirstOutputWindow ();
      if (pmyView) 
        {
        RECT rect = pmyView->GetTextRectangle ();
        SetUpVariantLong (vaResult, rect.top);
        break;
        }
      }
      break;

    case 292:
      {
      CMUSHView* pmyView = GetFirstOutputWindow ();
      if (pmyView) 
        {
        RECT rect = pmyView->GetTextRectangle ();
        SetUpVariantLong (vaResult, rect.right);
        break;
        }
      }
      break;

    case 293:
      {
      CMUSHView* pmyView = GetFirstOutputWindow ();
      if (pmyView) 
        {
        RECT rect = pmyView->GetTextRectangle ();
        SetUpVariantLong (vaResult, rect.bottom);
        break;
        }
      }
      break;

    case 294: 
      {
      long result = 0;
      if (GetKeyState (VK_SHIFT)    & 0x8000)
        result |= 0x01;
      if (GetKeyState (VK_CONTROL)  & 0x8000)
        result |= 0x02;
      if (GetKeyState (VK_MENU)     & 0x8000)
        result |= 0x04;
      if (GetKeyState (VK_LSHIFT)   & 0x8000)
        result |= 0x08;
      if (GetKeyState (VK_RSHIFT)   & 0x8000)
        result |= 0x10;
      if (GetKeyState (VK_LCONTROL) & 0x8000)
        result |= 0x20;
      if (GetKeyState (VK_RCONTROL) & 0x8000)
        result |= 0x40;
      if (GetKeyState (VK_LMENU)    & 0x8000)
        result |= 0x80;
      if (GetKeyState (VK_RMENU)    & 0x8000)
        result |= 0x100;
      if (GetKeyState (VK_CAPITAL)  & 0x8000)
        result |= 0x200;
       if (GetKeyState (VK_NUMLOCK) & 0x8000)
        result |= 0x400;
      if (GetKeyState (VK_SCROLL)   & 0x8000)
        result |= 0x800;
      // 0x1000 unused for symmetry
      if (GetKeyState (VK_CAPITAL)  & 0x0001)
        result |= 0x2000;
       if (GetKeyState (VK_NUMLOCK) & 0x0001)
        result |= 0x4000;
      if (GetKeyState (VK_SCROLL)   & 0x0001)
        result |= 0x8000;

      if (GetKeyState (VK_LBUTTON)  & 0x8000)
        result |= 0x10000;
       if (GetKeyState (VK_RBUTTON) & 0x8000)
        result |= 0x20000;
      if (GetKeyState (VK_MBUTTON)   & 0x8000)
        result |= 0x40000;

      SetUpVariantLong (vaResult, result); 
      }
      break; // state of virtual keys    

    case 295:
      SetUpVariantLong (vaResult, m_iOutputWindowRedrawCount); 
      break; // state of virtual keys    

    case 296:
      {
      CMUSHView* pmyView = GetFirstOutputWindow ();
      if (pmyView) 
        {
        CPoint pt = pmyView->GetScrollPosition ();
        SetUpVariantLong (vaResult, pt.y);
        break;
        }
      }
      break;

    case 297:
       SetUpVariantDouble (vaResult, App.m_iCounterFrequency); 
       break;

    case 298:
        SetUpVariantLong (vaResult, sqlite3_libversion_number()); 
        break;

    case 299:
        SetUpVariantLong (vaResult, GetACP());    // ANSI code page
        break;

    case 300:
        SetUpVariantLong (vaResult, GetOEMCP());  // OEM code page
        break;


    // dates

    case  301: 
      if (m_tConnectTime.GetTime ())     // only if non-zero, otherwise return empty
        SetUpVariantDate (vaResult, COleDateTime (m_tConnectTime.GetTime ())); 
      break;
    case  302: 
      if (m_LastFlushTime.GetTime ())     // only if non-zero, otherwise return empty      
        SetUpVariantDate (vaResult, COleDateTime (m_LastFlushTime.GetTime ())); 
      break;
    case  303: 
      if (m_timeScriptFileMod.GetTime ())     // only if non-zero, otherwise return empty      
        SetUpVariantDate (vaResult, COleDateTime (m_timeScriptFileMod.GetTime ())); 
      break;      

    case  304: 
      SetUpVariantDate (vaResult, COleDateTime (CTime::GetCurrentTime().GetTime ())); 
      break;      

    case  305:
      SetUpVariantDate (vaResult, COleDateTime (App.m_whenClientStarted.GetTime ())); 
      break;

    case  306:
      SetUpVariantDate (vaResult, COleDateTime (m_whenWorldStarted.GetTime ())); 
      break;

    // more numbers
      
    case 310:
        SetUpVariantLong (vaResult, m_newlines_received);  // newlines received
        break;

    default:
      vaResult.vt = VT_NULL;
      break;

    } // end of switch

	return vaResult;
}  // CMUSHclientDoc::GetInfo




long CMUSHclientDoc::GetSentBytes() 
{
	return (long) m_nBytesOut;
}   // end of CMUSHclientDoc::GetSentBytes

long CMUSHclientDoc::GetReceivedBytes() 
{
	return (long) m_nBytesIn;
}    // end of CMUSHclientDoc::GetReceivedBytes

long CMUSHclientDoc::GetConnectDuration() 
{

  if (m_iConnectPhase != eConnectConnectedToMud)
    return 0;

  CTimeSpan ts = CTime::GetCurrentTime() - m_tConnectTime;
  
	return ts.GetTotalSeconds ();
}   // end of CMUSHclientDoc::GetConnectDuration

double CMUSHclientDoc::GetScriptTime() 
{
  // time taken to execute scripts
  if (App.m_iCounterFrequency <= 0)
    return 0.0;

  double   elapsed_time;
  elapsed_time = ((double) m_iScriptTimeTaken) / 
                 ((double) App.m_iCounterFrequency);

	return elapsed_time;
}   // end of CMUSHclientDoc::GetScriptTime


BSTR CMUSHclientDoc::WorldAddress() 
{
	CString strResult = m_server;

	return strResult.AllocSysString();
}   // end of  CMUSHclientDoc::WorldAddress

long CMUSHclientDoc::WorldPort() 
{

	return m_port;
}   // end of CMUSHclientDoc::WorldPort



// world.GetLineInfo (LineNumber, InfoType) - gets details about the line
//                                     returns "EMPTY" if line out of range
//                                     returns "NULL" if bad info type
//   1: text of line
//   2: length of text
//   3: true if newline, false if not
//   4: true if world.note
//   5: true if player input
//   6: true if line logged
//   7: true if bookmarked
//   8: true if horizontal rule (<hr>)
//   9: date/time line arrived
//  10: actual line number (not line number in buffer)
//  11: count of style runs

VARIANT CMUSHclientDoc::GetLineInfo(long LineNumber, short InfoType) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;

  // check line exists
  if (LineNumber <= 0 || LineNumber > m_LineList.GetCount ())
    return vaResult;

  // get pointer to line in question

CLine * pLine = m_LineList.GetAt (GetLinePosition (LineNumber - 1));

  switch (InfoType)
    {
    case   1: SetUpVariantString (vaResult, CString (pLine->text, pLine->len)); break;
    case   2: SetUpVariantShort  (vaResult, pLine->len); break;
    case   3: SetUpVariantBool   (vaResult, pLine->hard_return); break;
    case   4: SetUpVariantBool   (vaResult, (pLine->flags & COMMENT) != 0); break;
    case   5: SetUpVariantBool   (vaResult, (pLine->flags & USER_INPUT) != 0); break;
    case   6: SetUpVariantBool   (vaResult, (pLine->flags & LOG_LINE) != 0); break;
    case   7: SetUpVariantBool   (vaResult, (pLine->flags & BOOKMARK) != 0); break;
    case   8: SetUpVariantBool   (vaResult, (pLine->flags & HORIZ_RULE) != 0); break;
    case   9: SetUpVariantDate   (vaResult, COleDateTime (pLine->m_theTime.GetTime ())); break;
    case  10: SetUpVariantLong   (vaResult, pLine->m_nLineNumber); break;
    case  11: SetUpVariantLong   (vaResult, pLine->styleList.GetCount ()); break;
    case  12: 
      {
      // high-performance timer
      double ticks = (double) pLine->m_lineHighPerformanceTime.QuadPart / (double) App.m_iCounterFrequency;
      SetUpVariantDouble (vaResult, ticks);
      }
      break;

    case 13:
      {
      LONGLONG iTimeTaken;
      double fElapsedTime;

      // elapsed time from when world started
      iTimeTaken = pLine->m_lineHighPerformanceTime.QuadPart - 
                   m_whenWorldStartedHighPrecision.QuadPart;
    
      if (App.m_iCounterFrequency)
       fElapsedTime = ((double) iTimeTaken) / 
                      ((double) App.m_iCounterFrequency);
      else
       fElapsedTime = pLine->m_theTime.GetTime () - (double) m_whenWorldStarted.GetTime ();
      SetUpVariantDouble (vaResult, fElapsedTime);
      }
      break;

    default:
      vaResult.vt = VT_NULL;
      break;

    } // end of switch

	return vaResult;
} // end of CMUSHclientDoc::GetLineInfo

// world.GetLinesInBufferCount 
//
//  returns actual number of lines in output buffer (not count of lines received)

long CMUSHclientDoc::GetLinesInBufferCount() 
{
	return m_LineList.GetCount ();
}   // end of CMUSHclientDoc::GetLinesInBufferCount


// world.GetStyleInfo (LineNumber, StyleNumber, InfoType) - gets details about the style
//                                     returns "EMPTY" if line or style number out of range
//                                     returns "NULL" if bad info type
//  1: text of style
//  2: length of style run
//  3: starting column of style
//  4: action type - 0=none, 1=send to mud, 2=hyperlink, 3=prompt
//  5: action   (eg. what to send)
//  6: hint     (what to show)
//  7: variable (variable to set)
//  8: true if bold
//  9: true if underlined
// 10: true if blinking
// 11: true if inverse
// 12: true if changed by trigger from original
// 13: true if start of a tag (action is tag name)
// 14: foreground (text) colour in RGB
// 15: background colour in RGB


VARIANT CMUSHclientDoc::GetStyleInfo(long LineNumber, long StyleNumber, short InfoType) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;

  // check line exists
  if (LineNumber <= 0 || LineNumber > m_LineList.GetCount ())
    return vaResult;

  // get pointer to line in question

CLine * pLine = m_LineList.GetAt (GetLinePosition (LineNumber - 1));

  // check style exists
  if (StyleNumber <= 0 || StyleNumber > pLine->styleList.GetCount ())
    return vaResult;

CStyle * pStyle = NULL;
POSITION pos;
int iCol = 0;
int iCount = 1;

  // search for it sequentially so we know the column number, 
  // so we can get its text
  for (pos = pLine->styleList.GetHeadPosition(); pos; iCount++)
    {
    pStyle = pLine->styleList.GetNext (pos);
    if (iCount == StyleNumber)
      break;  // found right one
    
    if (!pos)
      return vaResult;

    iCol += pStyle->iLength; // new column

    } // end of looping looking for it

CString strAction, strHint, strVariable;
CAction * pAction = pStyle ? pStyle->pAction : NULL;

COLORREF colour1,
         colour2;

  switch (InfoType)
    {
    case   1: 
      {
      CString strText = CString (pLine->text, pLine->len);
      SetUpVariantString (vaResult, strText.Mid (iCol, pStyle->iLength)); 
      }
      break;
    case   2: SetUpVariantShort  (vaResult, pStyle->iLength); 
      break;
    case   3: SetUpVariantShort  (vaResult, iCol + 1); 
      break;
    case   4: 
      {
      int iAction = 0;
      switch (pStyle->iFlags & ACTIONTYPE)
        {
        case ACTION_NONE:       iAction = 0; break;
        case ACTION_SEND:       iAction = 1; break;
        case ACTION_HYPERLINK:  iAction = 2; break;
        case ACTION_PROMPT:     iAction = 3; break;
        } // end of switch

      SetUpVariantShort  (vaResult, iAction);
      }
      break;

    case 5:
      if (pAction)
        SetUpVariantString (vaResult,  pAction->m_strAction);
       else
        SetUpVariantString (vaResult,  "");
      break;

    case 6:
      if (pAction)
        SetUpVariantString (vaResult,  pAction->m_strHint);
       else
        SetUpVariantString (vaResult,  "");
      break;

    case 7:
      if (pAction)
        SetUpVariantString (vaResult,  pAction->m_strVariable);
       else
        SetUpVariantString (vaResult,  "");
      break;

    case  8: SetUpVariantBool   (vaResult, (pStyle->iFlags & HILITE) != 0); 
      break;
    case  9: SetUpVariantBool   (vaResult, (pStyle->iFlags & UNDERLINE) != 0); 
      break;
    case 10: SetUpVariantBool   (vaResult, (pStyle->iFlags & BLINK) != 0); 
      break;
    case 11: SetUpVariantBool   (vaResult, (pStyle->iFlags & INVERSE) != 0); 
      break;
    case 12: SetUpVariantBool   (vaResult, (pStyle->iFlags & CHANGED) != 0); 
      break;
    case 13: SetUpVariantBool   (vaResult, (pStyle->iFlags & START_TAG) != 0); 
      break;

    case 14:
          GetStyleRGB (pStyle, colour1, colour2);
          SetUpVariantLong  (vaResult, colour1); 
          break;

    case 15:
          GetStyleRGB (pStyle, colour1, colour2);
          SetUpVariantLong  (vaResult, colour2); 
          break;

    default:
      vaResult.vt = VT_NULL;
      break;

    } // end of switch


	return vaResult;
}    // end of CMUSHclientDoc::GetStyleInfo


long CMUSHclientDoc::GetSelectionStartLine() 
{
  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
  	  {
		  CMUSHView* pmyView = (CMUSHView*)pView;
      long startcol,
           endcol;

      CLine * pStartLine,
            * pEndLine;

      pStartLine = m_LineList.GetAt (GetLinePosition (pmyView->m_selstart_line));
      pEndLine   = m_LineList.GetAt (GetLinePosition (pmyView->m_selend_line));

      startcol  = MIN (pmyView->m_selstart_col, pStartLine->len);
      endcol    = MIN (pmyView->m_selend_col, pEndLine->len);

      // return 0 if no selection
      if (!(pmyView->m_selend_line > pmyView->m_selstart_line || 
                  (pmyView->m_selend_line == pmyView->m_selstart_line && 
                   endcol > startcol)))
        return 0;
      return pmyView->m_selstart_line + 1;
	    }	  // end of being a CMUSHView
    }   // end of loop through views


	return 0;
}   // end of CMUSHclientDoc::GetSelectionStartLine

long CMUSHclientDoc::GetSelectionEndLine() 
{
  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
  	  {
		  CMUSHView* pmyView = (CMUSHView*)pView;
      long startcol,
           endcol;

      CLine * pStartLine,
            * pEndLine;

      pStartLine = m_LineList.GetAt (GetLinePosition (pmyView->m_selstart_line));
      pEndLine   = m_LineList.GetAt (GetLinePosition (pmyView->m_selend_line));

      startcol  = MIN (pmyView->m_selstart_col, pStartLine->len);
      endcol    = MIN (pmyView->m_selend_col, pEndLine->len);

      // return 0 if no selection
      if (!(pmyView->m_selend_line > pmyView->m_selstart_line || 
                  (pmyView->m_selend_line == pmyView->m_selstart_line && 
                   endcol > startcol)))
        return 0;

      return pmyView->m_selend_line + 1;
	    }	  // end of being a CMUSHView
    }   // end of loop through views

	return 0;
}    // end of CMUSHclientDoc::GetSelectionEndLine



long CMUSHclientDoc::GetSelectionStartColumn() 
{
  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
  	  {
		  CMUSHView* pmyView = (CMUSHView*)pView;
      long startcol,
           endcol;

      CLine * pStartLine,
            * pEndLine;

      pStartLine = m_LineList.GetAt (GetLinePosition (pmyView->m_selstart_line));
      pEndLine   = m_LineList.GetAt (GetLinePosition (pmyView->m_selend_line));

      startcol  = MIN (pmyView->m_selstart_col, pStartLine->len);
      endcol    = MIN (pmyView->m_selend_col, pEndLine->len);

      // return 0 if no selection
      if (!(pmyView->m_selend_line > pmyView->m_selstart_line || 
                  (pmyView->m_selend_line == pmyView->m_selstart_line && 
                   endcol > startcol)))
        return 0;

      return startcol + 1;
	    }	  // end of being a CMUSHView
    }   // end of loop through views

	return 0;
}  // end of CMUSHclientDoc::GetSelectionStartColumn

long CMUSHclientDoc::GetSelectionEndColumn() 
{
  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
  	  {
		  CMUSHView* pmyView = (CMUSHView*)pView;
      long startcol,
           endcol;

      CLine * pStartLine,
            * pEndLine;

      pStartLine = m_LineList.GetAt (GetLinePosition (pmyView->m_selstart_line));
      pEndLine   = m_LineList.GetAt (GetLinePosition (pmyView->m_selend_line));

      startcol  = MIN (pmyView->m_selstart_col, pStartLine->len);
      endcol    = MIN (pmyView->m_selend_col, pEndLine->len);

      // return 0 if no selection
      if (!(pmyView->m_selend_line > pmyView->m_selstart_line || 
                  (pmyView->m_selend_line == pmyView->m_selstart_line && 
                   endcol > startcol)))
        return 0;

      return endcol + 1;
	    }	  // end of being a CMUSHView
    }   // end of loop through views

	return 0;
}     // end of CMUSHclientDoc::GetSelectionEndColumn



VARIANT CMUSHclientDoc::GetHostAddress(LPCTSTR HostName) 
{
  COleSafeArray sa;   // for list

  if (strlen (HostName) > 0)
    {

    LPHOSTENT pHost = gethostbyname (HostName);
    long iCount;

    for (iCount = 0; pHost != NULL && pHost->h_addr_list [iCount] != NULL; iCount++ )
      {};   // end of counting them

    if (iCount) // cannot create empty array dimension
      {
      sa.CreateOneDim (VT_VARIANT, iCount);
  
    // put the addresses into the array
      for (long i = 0; pHost != NULL && pHost->h_addr_list [i] != NULL; i++ )
        {
        COleVariant v (inet_ntoa ( *((in_addr *)pHost->h_addr_list[i])));
        sa.PutElement (&i, &v);
        }      // end of looping through each IP address
      } // end of having at least one
    }   // end of non-blank name

	return sa.Detach ();
}    // end of CMUSHclientDoc::GetHostAddress

BSTR CMUSHclientDoc::GetHostName(LPCTSTR IPaddress) 
{
	CString strResult;

  if (strlen (IPaddress) > 0)
    {
    unsigned long ulAddress = inet_addr (IPaddress);
    if (ulAddress != INADDR_NONE)
      {
      LPHOSTENT lpHostEntry = gethostbyaddr ((const char *) &ulAddress, 
                                             sizeof ulAddress, 
                                             AF_INET);
      if (lpHostEntry != NULL)
        strResult =  lpHostEntry->h_name;
      } // end of valid IP address
    }   // end of non-blank name

	return strResult.AllocSysString();
}    // end of CMUSHclientDoc::GetHostName


BSTR CMUSHclientDoc::GetWorldID() 
{
	return m_strWorldID.AllocSysString();
}  // end of CMUSHclientDoc::GetWorldID


/*
Colour types:

Scrollbar = 0
Background = 1
Active caption = 2
Inactive caption = 3
Menu = 4
Window = 5
Window frame = 6
Menu text = 7
Window text = 8
Caption text = 9
Active border = 10
Inactive border = 11
Application workspace = 12
Highlight = 13
Highlight text = 14
Button face = 15
Button shadow = 16
Gray text = 17
Button text = 18
Inactive caption text = 19
Button highlight = 20
3D dark shadow = 21
3D light = 22
Info text = 23
Info bk = 24

*/

long CMUSHclientDoc::GetSysColor(long Index) 
{
  return ::GetSysColor (Index);
}  // end of CMUSHclientDoc::GetSysColor

/*
Info codes:

CX screen = 0
CY screen = 1
CX vscroll = 2
CY hscroll = 3
CY caption = 4
CX border = 5
CY border = 6
CX dialog frame = 7
CY dialog frame = 8
CY v thumb = 9
CX h thumb = 10
CX icon = 11
CY icon = 12
CX cursor = 13
CY cursor = 14
CY menu = 15
CX full screen = 16
CY full screen = 17
CY kanji window = 18
Mouse present = 19
CY vscroll = 20
CX hscroll = 21
Debug = 22
Swap button = 23
Reserved1 = 24
Reserved2 = 25
Reserved3 = 26
Reserved4 = 27
CX min = 28
CY min = 29
CX size = 30
CY size = 31
CX frame = 32
CY frame = 33
CX min track = 34
CY min track = 35
CX double click = 36
CY double click = 37
CX icon spacing = 38
CY icon spacing = 39
Menu drop alignment = 40
Pen windows = 41
DBCS enabled = 42
Cmouse buttons = 43
Secure = 44
CX edge = 45
CY edge = 46
CX minspacing = 47
CY minspacing = 48
CX smicon = 49
CY smicon = 50
CY smcaption = 51
CX smsize = 52
CY smsize = 53
CX menusize = 54
CY menusize = 55
Arrange = 56
CX minimized = 57
CY minimized = 58
CX maxtrack = 59
CY maxtrack = 60
CX maximized = 61
CY maximized = 62
Network = 63
Clean boot = 67
CX drag = 68
CY drag = 69
Show sounds = 70
CX menucheck = 71 
CY menucheck = 72
Slow machine = 73
Mid-east enabled = 74
Mouse wheel present = 75
X virtual screen = 76
Y virtual screen = 77
CX virtual screen = 78
CY virtual screen = 79
C monitors = 80
Same display format = 81

  */

long CMUSHclientDoc::GetSystemMetrics(long Index) 
{
  return ::GetSystemMetrics (Index);
}  // end of CMUSHclientDoc::GetSystemMetrics



/*
 Device Parameters for GetDeviceCaps() 

    DRIVERVERSION 0       Device driver version                    
    TECHNOLOGY    2       Device classification                    
    HORZSIZE      4       Horizontal size in millimeters           
    VERTSIZE      6       Vertical size in millimeters             
    HORZRES       8       Horizontal width in pixels               
    VERTRES       10      Vertical height in pixels                
    BITSPIXEL     12      Number of bits per pixel                 
    PLANES        14      Number of planes                         
    NUMBRUSHES    16      Number of brushes the device has         
    NUMPENS       18      Number of pens the device has            
    NUMMARKERS    20      Number of markers the device has         
    NUMFONTS      22      Number of fonts the device has           
    NUMCOLORS     24      Number of colors the device supports     
    PDEVICESIZE   26      Size required for device descriptor      
    CURVECAPS     28      Curve capabilities                       
    LINECAPS      30      Line capabilities                        
    POLYGONALCAPS 32      Polygonal capabilities                   
    TEXTCAPS      34      Text capabilities                        
    CLIPCAPS      36      Clipping capabilities                    
    RASTERCAPS    38      Bitblt capabilities                      
    ASPECTX       40      Length of the X leg                      
    ASPECTY       42      Length of the Y leg                      
    ASPECTXY      44      Length of the hypotenuse                 
                          
    SHADEBLENDCAPS 45     Shading and blending caps                
                         
    LOGPIXELSX    88      Logical pixels/inch in X                 
    LOGPIXELSY    90      Logical pixels/inch in Y                 
                         
    SIZEPALETTE  104      Number of entries in physical palette    
    NUMRESERVED  106      Number of reserved entries in palette    
    COLORRES     108      Actual color resolution                  

    // Display driver specific

    VREFRESH        116   Current vertical refresh rate of the    
                          display device (for displays only) in Hz
    DESKTOPVERTRES  117   Horizontal width of entire desktop in   
                          pixels                                  
    DESKTOPHORZRES  118   Vertical height of entire desktop in    
                         pixels                                  
    BLTALIGNMENT    119    Preferred blt alignment                 

*/

// get device capabilites
long CMUSHclientDoc::GetDeviceCaps(long Index) 
{
CDC dc;
  dc.CreateCompatibleDC (NULL);
  return dc.GetDeviceCaps (Index);
}   // end of CMUSHclientDoc::GetDeviceCaps


BSTR CMUSHclientDoc::GetNotes() 
{
	CString strResult = m_notes;

	return strResult.AllocSysString();
}  // end of CMUSHclientDoc::GetNotes


extern int_flags_pair error_descriptions [];

BSTR CMUSHclientDoc::ErrorDesc(long Code) 
{
	CString strResult;
  
  strResult.Format ("%ld", Code);
	
  const int_flags_pair *p;
  for(p = error_descriptions; p->val != NULL; p++) 
    {
    if (p->key == Code)
      {
      strResult = Translate (p->val);
      break;
      }
    } 

	return strResult.AllocSysString();
}  // end of CMUSHclientDoc::ErrorDesc


// warning - different in Lua
BSTR CMUSHclientDoc::GetMainWindowPosition() 
{
	CString strResult;

  CWindowPlacement wp;
  Frame.GetWindowPlacement(&wp);  

	windowPositionHelper (strResult, wp.rcNormalPosition);
  return strResult.AllocSysString();
}   // end of CMUSHclientDoc::GetMainWindowPosition

// warning - different in Lua
BSTR CMUSHclientDoc::GetWorldWindowPosition() 
{
  return GetWorldWindowPositionX (1); // first window
}   // end of CMUSHclientDoc::GetWorldWindowPosition



VARIANT CMUSHclientDoc::GetGlobalOption(LPCTSTR Name) 
{
  return App.GetGlobalOption (Name);
}   // end of CMUSHclientDoc::GetGlobalOption

VARIANT CMUSHclientDoc::GetGlobalOptionList() 
{
  return App.GetGlobalOptionList ();
}   // end of CMUSHclientDoc::GetGlobalOptionList

