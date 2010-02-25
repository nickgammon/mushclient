// doc.h : interface of the CMUSHclientDoc class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "worldsock.h"
#include "chatsock.h"
#include "chatlistensock.h"
#include "dialogs\ColourComboBox.h"
#include "dialogs\world_prefs\prefspropertypages.h"
#include "TimerWnd.h"
#include "xml\xmlparse.h"
#include "paneline.h"

#define COMPRESS_BUFFER_LENGTH 1024   // size of decompression buffer

// ============================================================================

// New versions - things to change

#define THISVERSION 450                       // Step 1.
const CString MUSHCLIENT_VERSION = "4.50";    // Step 2.
// Step 3. Don't forget VERSION resource in Resources tab
// Step 4. Remember: README.TXT 

// ============================================================================

#define MY_WEB_PAGE "http://www.gammon.com.au/mushclient/" 
#define CHANGES_WEB_PAGE "http://www.gammon.com.au/scripts/showrelnote.php" 
#define MUD_LIST "http://www.gammon.com.au/links/muds.htm" 
#define BUG_REPORT_PAGE "http://www.gammon.com.au/forum/?bbtopic_id=5"
#define FORUM_URL "http://www.gammon.com.au/forum/"
#define ORDERS_PAGE "http://www.gammon.com.au/fee.htm" 
#define REGCODE_URL "http://www.gammon.com.au/regcode.htm" 
#define MUSHCLIENT_FORUM_URL "http://www.gammon.com.au/forum/?bbsection_id=1"
#define MUSHCLIENT_FUNCTIONS_URL "http://www.gammon.com.au/mushclient/functions.htm"
#define DOCUMENTATION_PAGE "http://www.gammon.com.au/scripts/doc.php"
#define REGEXP_PAGE "http://www.gammon.com.au/mushclient/regexp.htm"
#define PLUGINS_PAGE "http://www.gammon.com.au/mushclient/plugins/"

#define MAX_LINE_WIDTH 500    // max line we will wrap to
#define MAX_RECENT_LINES 200  // maximum recent lines we can use for multi-line triggers

#pragma warning(disable: 4800) // disable warning about bool being forced to BOOL

// program constants

const int JUMP_SIZE = 100;     // how many lines we discard when we reach maximum lines
const char ENDLINE [] = "\r\n";         // line terminator
const char SIGNATURE [] = "WinMUSH";    // world file signature
const int VERSION = 15;                  // version number written to world file
#define MAX_EXECUTION_DEPTH 20          // number of nested calls to world.Execute we permit

#define MAX(arg1, arg2) (((arg1) > (arg2)) ? (arg1) : (arg2))
#define MIN(arg1, arg2) (((arg1) < (arg2)) ? (arg1) : (arg2))

// definitions for Pueblo - see:
//   http://www.chaco.com/pueblo/doc/manual/puebloPueblo_Enhancers_Guide.html

// this string identifies a Pueblo-offering server
#define PUEBLO_ID_STRING1 "This world is Pueblo 1.10 enhanced."
#define PUEBLO_ID_STRING2 "This world is Pueblo 1.0 Enhanced"
#define PUEBLO_ID_STRING3 "This world is Pueblo 2.50 enhanced."
// this string identifies that we will use Pueblo - replace %s with a 32-character hash
#define PUEBLO_REPLY1 "PUEBLOCLIENT %s md5=\"%s\""
#define PUEBLO_REPLY2 "PUEBLOCLIENT"
// this string identifies that Pueblo starts after it is received
#define PUEBLO_START "</xch_mudtext>"

#define ORIGINAL_MACRO_COUNT 60

class CSendView;
class CMUSHView;
class CChildFrame;
class CSendView;
class CTextDocument;
class UDPsocket;

#define ESC '\x1B'

// ANSI/MXP phases

enum { NONE,          // normal text
       HAVE_ESC,      // just received an escape
       DOING_CODE,    // process an ANSI escape sequence
       HAVE_IAC,      // just received a TELNET IAC (interpret as command)
       HAVE_WILL,     // just received a TELNET WILL
       HAVE_WONT,     // just received a TELNET WONT
       HAVE_DO,       // just received a TELNET DO
       HAVE_DONT,     // just received a TELNET DONT
       HAVE_SB,       // just received a TELNET IAC SB
       HAVE_SUBNEGOTIATION,  // received TELNET IAC SB c  (collecting data, awaiting IAC SE)
       HAVE_SUBNEGOTIATION_IAC,  // received TELNET IAC SB c  <data> IAC  (awaiting IAC or SE)
       HAVE_COMPRESS,  // received TELNET IAC COMPRESS
       HAVE_COMPRESS_WILL, // received TELNET IAC COMPRESS WILL
       HAVE_FOREGROUND_256_START,   // received ESC[38;
       HAVE_FOREGROUND_256_FINISH,  // received ESC[38;5;
       HAVE_BACKGROUND_256_START,   // received ESC[48;
       HAVE_BACKGROUND_256_FINISH,  // received ESC[48;5;


       // mxp modes
       HAVE_MXP_ELEMENT, // collecting element, eg. < xxxxx >. Starts on <, stops on >
       HAVE_MXP_COMMENT, // collecting comment, eg. <!-- xxxxx -->. Starts on <!--, stops on -->
       HAVE_MXP_QUOTE,   // collecting quote inside element, eg. <color='red'>
       HAVE_MXP_ENTITY,  // collecting entity, eg. &gt; . Starts on &, stops on ;

       // mxp special collection modes following a special escape sequence
       HAVE_MXP_ROOM_NAME,  // the line is parsed as the name of a room.  
       HAVE_MXP_ROOM_DESCRIPTION,  // the line is parsed as a description of a room.    
       HAVE_MXP_ROOM_EXITS,  // the line is parsed as an exit line for a room.    
       HAVE_MXP_WELCOME,  // This text is sent from the MUD at the beginning of a session to welcome the user to the MUD.    
       };
        

// TELNET escape sequences  - see RFC 854

enum { WILL_END_OF_RECORD      = 0x19, // see RFC 885  */
       EOR                = 0xEF, // see RFC 885
       SE                 = 0xF0, // end of subnegotiation
       NOP                = 0xF1, // no operation
       DATA_MARK          = 0xF2, // see RFC 854
       BREAK              = 0xF3, // BRK
       INTERRUPT_PROCESS  = 0xF4, // IP
       ABORT_OUTPUT       = 0xF5, // AO
       ARE_YOU_THERE      = 0xF6, // AYT
       ERASE_CHARACTER    = 0xF7, // EC
       ERASE_LINE         = 0xF8, // EL
       GO_AHEAD           = 0xF9, // GA
       SB                 = 0xFA, // subnegotiation
       WILL               = 0xFB, 
       WONT               = 0xFC,
       DO                 = 0xFD,
       DONT               = 0xFE,
       IAC                = 0xFF
      };

// values for m_iConnectPhase - for connecting via Socks proxy or directly
//  - this is so we know what we are up to

enum {    
      eConnectNotConnected,           // 0: not connected and not attempting connection
      eConnectMudNameLookup,          // 1: finding address of MUD
      eConnectProxyNameLookup,        // 2: finding address of proxy server
      eConnectConnectingToMud,        // 3: connecting to MUD (no proxy server)
      eConnectConnectingToProxy,      // 4: connecting to proxy server
      eConnectAwaitingProxyResponse1, // 5: sent SOCKS authentication method, awaiting confirmation
      eConnectAwaitingProxyResponse2, // 6: sent SOCKS username/password, awaiting confirmation
      eConnectAwaitingProxyResponse3, // 7: sent SOCKS connect details, awaiting confirmation
      eConnectConnectedToMud,         // 8: connected, we can play now
      eConnectDisconnecting,          // 9: in process of disconnecting, don't attempt to reconnect
  };
      
// values for m_iSocksProcessing - type of proxy server

enum {
      eProxyServerNone,     // no proxy server
      eProxyServerSocks4,   // SOCKS 4
      eProxyServerSocks5,   // SOCKS 5

      // This must be last !!!

      eProxyServerLast,
  };

// values for m_iCurrentActionSource - why script is currently running

enum {
      eDontChangeAction = 999,      // leave current action alone (was -1, but must be unsigned)
      eUnknownActionSource = 0,     // no particular reason, could be plugin saving 
      eUserTyping = 1,              // user typed something in the command area and pressed <Enter>
      eUserMacro = 2,               // user typed a macro  (eg. F2)
      eUserKeypad = 3,              // user used the numeric keypad
      eUserAccelerator = 4,         // user used an accelerator key
      eUserMenuAction = 5,          // item chosen from pop-up menu
      eTriggerFired = 6,            // trigger fired
      eTimerFired = 7,              // timer fired
      eInputFromServer = 8,         // input arrived (eg. packet received)
      eWorldAction = 9,             // some sort of world action (eg. world open, connect, got focus)
      eLuaSandbox = 10,             // executing Lua sandbox
      eHotspotCallback = 11,        // miniwindow hotspot callback
  };

// MCCP (Mud Client Compression Protocol) stuff

// NB 85 is MCCP v1, 86 is MCCP v2

#define TELOPT_ECHO     1     //just in case you want to know what 1 is :)
#define TELOPT_NAWS 31        // Negotiate About Window Size
#define TELOPT_CHARSET 42     // Negotiate About Character Set
#define TELOPT_TERMINAL_TYPE 24  // want to know terminal type
#define TELOPT_COMPRESS 85   // telet negotiation code for starting compression v1
#define TELOPT_COMPRESS2 86  // telet negotiation code for starting compression v2
#define TELOPT_MUD_SPECIFIC 102  // telet negotiation code MUD-specific negotiations
#define SUPPORT_VERSIONS 0   // MCCP support-versions query
#define VERSION_IS 1         // MCCP version number
#define VERSION_NONE 0   // see web page below for details
#define VERSION_2_1 1   

// see http://www.randomly.org/projects/MCCP/protocol.html

#define SGA 3                 // suppress go-ahead

#define TELOPT_MSP 90   // telet negotiation code for MUD Sound Protocol (MSP)
#define TELOPT_MXP 91   // telet negotiation code for MUD Extension Protocol (MXP)
#define TELOPT_ZMP 93   // http://zmp.sourcemud.org/spec.shtml
#define TELOPT_ATCP 200   // http://www.ironrealms.com/rapture/manual/files/FeatATCP-txt.html


              
// bits for m_iFlags1

#define FLAGS1_ArrowRecallsPartial             0x0001
#define FLAGS1_CtrlZGoesToEndOfBuffer          0x0002
#define FLAGS1_CtrlPGoesToPreviousCommand      0x0004
#define FLAGS1_CtrlNGoesToNextCommand          0x0008
#define FLAGS1_HyperlinkAddsToCommandHistory   0x0010
#define FLAGS1_EchoHyperlinkInOutputWindow     0x0020
#define FLAGS1_AutoWrapWindowWidth             0x0040
#define FLAGS1_NAWS                            0x0080
#define FLAGS1_Pueblo                          0x0100
#define FLAGS1_NoEchoOff                       0x0200
#define FLAGS1_UseCustomLinkColour             0x0400
#define FLAGS1_MudCanChangeLinkColour          0x0800
#define FLAGS1_UnderlineHyperlinks             0x1000
#define FLAGS1_MudCanRemoveUnderline           0x2000
//#define FLAGS1_AssumeSecureMode                0x4000 

// bits for m_iFlags2

#define FLAGS2_AlternativeInverse              0x0001
#define FLAGS2_ShowConnectDisconnect           0x0002
#define FLAGS2_IgnoreMXPcolourChanges          0x0004
#define FLAGS2_Custom16isDefaultColour         0x0008
#define FLAGS2_LogInColour                     0x0010
#define FLAGS2_LogRaw                          0x0020


// queued commands echo and send flags - lowercase versions do not log

#define QUEUE_WITH_ECHO               'E'
#define QUEUE_WITHOUT_ECHO            'N'
#define IMMEDIATE_WITH_ECHO           'I'
#define IMMEDIATE_WITHOUT_ECHO        'W'

#define QUEUE_WITH_ECHO_NOLOG         'e'
#define QUEUE_WITHOUT_ECHO_NOLOG      'n'
#define IMMEDIATE_WITH_ECHO_NOLOG     'i'
#define IMMEDIATE_WITHOUT_ECHO_NOLOG  'w'


// reload script file options
enum {
      eReloadConfirm,
      eReloadAlways,
      eReloadNever,
  };

// used for iNotepadType

enum {
     eNotepadNormal,
     eNotepadMXPdebug,
     eNotepadTrigger,
     eNotepadRecall,
     eNotepadScript,
     eNotepadPacketDebug,
     eNotepadLineInfo,
     eNotepadWorldLoadError,
     eNotepadXMLcomments,
     eNotepadPluginInfo,
  };  

// values for auto-connect (m_connect_now)

enum
  {
  eNoAutoConnect,
  eConnectMUSH,
  eConnectDiku,
  eConnectMXP,

  eConnectTypeMax,  // this must be last!
  };  

// values for <OL> and <UL> tags
// see m_iListMode

enum
  {
  eNoList,
  eOrderedList,
  eUnorderedList,
  };

  // configuration page numbers

  enum
    {
    ePageGroup0,     // name
    ePageGeneral,
    ePageConnecting,
    ePageLogging,
    ePageTimers,
    ePageChat,
    ePageInfo,
    ePageNotes,
    ePageGroup00,    // appearance
    ePageOutput,
    ePageMXP,
    ePageANSIColours,
    ePageCustomColours,
    ePageTriggers,
//    ePageSounds,
    ePagePrinting,
    ePageGroup000,   // input
    ePageCommands,
    ePageAliases,
    ePageKeypad,
    ePageMacros,
    ePageAutosay,
//    ePageGroup0000, // paste
    ePagePaste,
    ePageSend,
    ePageGroup00000, // scripting
    ePageScripts,
    ePageVariables,
    };

// MXP modes - see m_iMXP_mode (based on <esc> [ x z where x is the mode)

  enum 
    {
    eMXP_open=0,   // only MXP commands in the "open" category are allowed. 
    eMXP_secure=1, // all tags and commands in MXP are allowed within the line. 
    eMXP_locked=2, // no MXP or HTML commands are allowed in the line.  The line is not parsed for any tags at all.  
    eMXP_reset=3,  // close all open tags
    eMXP_secure_once=4,  // next tag is secure only
    eMXP_perm_open=5,   // open mode until mode change 
    eMXP_perm_secure=6, // secure mode until mode change
    eMXP_perm_locked=7, // locked mode until mode change

    eMXP_room_name=10,  // the line is parsed as the name of a room.  
    eMXP_room_description=11,  // the line is parsed as a description of a room.    
    eMXP_room_exits=12,  // the line is parsed as an exit line for a room.    
    eMXP_welcome=19,  // This text is sent from the MUD at the beginning of a session to welcome the user to the MUD.    
//20 to 99 - user defined
    };

  // use mxp options  - see m_iUseMXP
enum {
     eOnCommandMXP,   // turn on after <IAC><SB><MXP><IAC><SE>
     eQueryMXP,       // turn on after option query
     eUseMXP,         // always on
     eNoMXP,          // always off
  };

// MXP debug levels
enum {
   DBG_NONE,         // no messages
   DBG_ERROR,        // only errors
   DBG_WARNING,      // errors and warnings
   DBG_INFO,         // errors, warnings and info
   DBG_ALL,          // a whole lot of useless stuff
  };

#define OPT_CUSTOM_COLOUR       0x000001    // colour number  (add 1 to colour to save, subtract 1 to load)
#define OPT_RGB_COLOUR          0x000002    // colour is RGB colour
#define OPT_DOUBLE              0x000004    // option is a double
#define OPT_UPDATE_VIEWS        0x000100    // if changed, update all views
#define OPT_UPDATE_INPUT_FONT   0x000200    // if changed, update input font
#define OPT_UPDATE_OUTPUT_FONT  0x000400    // if changed, update output font
#define OPT_FIX_OUTPUT_BUFFER   0x000800    // if changed, rework output buffer size
#define OPT_FIX_WRAP_COLUMN     0x001000    // if changed, wrap column has changed
#define OPT_FIX_SPEEDWALK_DELAY 0x002000    // if changed, speedwalk delay has changed
#define OPT_USE_MXP             0x004000    // if changed, use_mxp has changed
#define OPT_PLUGIN_CANNOT_READ  0x100000    // plugin may not read its value
#define OPT_PLUGIN_CANNOT_WRITE 0x200000    // plugin may not write its value
#define OPT_PLUGIN_CANNOT_RW    0x300000    // plugin may not read or write its value
#define OPT_CANNOT_WRITE        0x400000    // cannot be changed by any script
#define OPT_SERVER_CAN_WRITE    0x800000    // CAN be changed by <recommend_option> tag

// for debug.options and MXP <option> tag
typedef struct
  {
  char *  pName;      // name, eg. "logoutput"
  double  iDefault;   // original (default) value
  int     iOffset;    // offset in CMUSHclientDoc
  int     iLength;    // length of field (ie. 1, 2, 4, 8 bytes)
  double  iMinimum;   // minimum size it can be
  double  iMaximum;   // maximum size it can be, if both zero, assume boolean
  int     iFlags;     // colours etc, see above
  }  tConfigurationNumericOption;

// for XML load/save
typedef struct
  {
  char *  pName;      // name, eg. "server"
  char *  sDefault;   // original (default) value
  int     iOffset;    // offset in CMUSHclientDoc
  int     iFlags;     // flags, see above
  }  tConfigurationAlphaOption;


#define OPT_MULTLINE      0x000001    // multi-line option (eg. world notes)
#define OPT_KEEP_SPACES   0x000002    // preserve leading/trailing spaces
#define OPT_PASSWORD      0x000004    // use base 64 encoding
#define OPT_COMMAND_STACK 0x000008    // this is the command stack character
#define OPT_WORLD_ID      0x000010    // this is the world ID

// also can have values above: OPT_UPDATE_VIEWS, 
//                             OPT_UPDATE_INPUT_FONT, 
//                             OPT_UPDATE_OUTPUT_FONT   etc.


// for Lua tables mainly
typedef struct { const char* key; int val; } flags_pair;
typedef struct { int key; char * val; } int_flags_pair;

typedef struct 
  {
  int						iInfoType;			 // which info number it is
  const char *	sDescription;    // eg. "Server name"
  } tInfoTypeMapping;


// for SQLite databases
typedef struct 
  {
  sqlite3 *db;   // current database pointer
  sqlite3_stmt *pStmt;  // current prepared statement, NULL if none
  bool bValidRow;  // true if last call to DatabaseStep returned SQLITE_ROW                                                 
  string db_name;  // name of database when opened
  int iColumns;    // number of columns from last prepared statement
  } tDatabase;

typedef map<string, tDatabase *> tDatabaseMap;
typedef tDatabaseMap::iterator tDatabaseMapIterator;

class ScriptItem 
  {
  public:

  // constructor
  ScriptItem (CPlugin * pPlugin,
              const string sText, 
              const string sSource) :
        pWhichPlugin  (pPlugin), 
        sScriptText   (sText), 
        sScriptSource (sSource) {};

  CPlugin * pWhichPlugin;        // which plugin
  const string sScriptText;      // the script to execute
  const string sScriptSource;    // what it is, eg. "Trigger X"
  };

typedef list<ScriptItem> ScriptItemMap;

class OneShotItem 
  {
  public:

  // constructor
  OneShotItem (CPlugin * pPlugin,
              const string sKey) :
        pWhichPlugin  (pPlugin), 
        sItemKey   (sKey) {};

  CPlugin * pWhichPlugin;     // which plugin
  const string sItemKey;      // the key to delete
  };

typedef list<OneShotItem> OneShotItemMap;

// option get/set stuff

double GetBaseOptionItem (const int iItem,
                        tConfigurationNumericOption BaseOptionsTable [],
                        const int iItemCount,
                        char * pBase);

CString GetBaseAlphaOptionItem (const int iItem,
                                tConfigurationAlphaOption BaseAlphaOptionsTable [],
                                const int iItemCount,
                                char * pBase);


long SetBaseOptionItem (const int iItem,
                        tConfigurationNumericOption BaseOptionsTable [],
                        const int iItemCount,
                        char * pBase,
                        double Value,
                        bool & bChanged);

long SetBaseAlphaOptionItem (const int iItem,
                        tConfigurationAlphaOption BaseAlphaOptionsTable [],
                        const int iItemCount,
                        char * pBase,
                        CString & strValue,
                        bool & bChanged);

long FindBaseOption (LPCTSTR OptionName, 
                    tConfigurationNumericOption BaseOptionsTable [],
                    int & iItem);

long FindBaseAlphaOption (LPCTSTR OptionName, 
                    tConfigurationAlphaOption BaseAlphaOptionsTable [],
                    int & iItem);


class CNumericConfiguration : public CObject
  {
  public:

    long iValue;    // loaded value
    bool bInclude;  // true if from include file

    void Reset (void)  { iValue = 0; bInclude = false; }
    CNumericConfiguration () { Reset (); }

  }; // end of  CConfiguration

typedef CTypedPtrArray <CPtrArray, CNumericConfiguration*> CNumericConfigurationArray;

class CAlphaConfiguration : public CObject
  {
  public:

    CString sValue;   // loaded value
    bool bInclude;    // true if from include file

    void Reset (void)  { sValue.Empty (); bInclude = false; }
    CAlphaConfiguration () { Reset (); }

  }; // end of  CAlphaConfiguration

typedef CTypedPtrArray <CPtrArray, CAlphaConfiguration*> CAlphaConfigurationArray;

#ifdef PANE
  class CPaneView;
  typedef map<string, CPaneView *> CPaneMap;
  typedef map<string, CPaneView *>::iterator PaneMapIterator;
#endif // PANE

class CMUSHclientDoc : public CDocument
{
  friend class CActivityView;
  friend class CMapDlg;
  friend class CWorldSocket;
  friend class CPluginsDlg;
  friend class CSendView;
  friend class CMUSHView;
  friend class CChatSocket;
  friend class CChatListenSocket;
  friend class CChatListDlg;
  friend class CMUSHclientApp;
  friend class CScriptEngine;
  
  protected: // create from serialization only
	CMUSHclientDoc();
	DECLARE_DYNCREATE(CMUSHclientDoc)

// Attributes
public:
	CWorldSocket* m_pSocket;

// stuff saved to disk

  CString m_server;
  CString m_mush_name;
  CString m_name;
  CString m_password;
  CString m_file_postamble;
  CString m_file_preamble ;
  CString m_line_postamble;
  CString m_line_preamble ;
  CString m_notes;
  CString m_new_activity_sound;
  CString m_strScriptEditor;
  CString m_strScriptEditorArgument;
  CString m_strLogFilePreamble;
  CString m_strSpeedWalkFiller;

  unsigned short m_wrap;
  unsigned short m_port;
  unsigned short m_timestamps;
  unsigned short m_connect_now;
  unsigned short m_match_width;
  unsigned short m_enable_aliases;
  unsigned short m_enable_triggers;
  unsigned short m_bEnableTimers;
  

  unsigned short m_display_my_input;    
  unsigned short m_echo_colour;
  unsigned short m_enable_beeps;
  unsigned short m_enable_trigger_sounds;
  unsigned short m_indent_paras;
  unsigned short m_bSaveWorldAutomatically;
  unsigned short m_bLineInformation;
  unsigned short m_bStartPaused;


  LONG    m_font_height;               
  CString m_font_name;   
  LONG    m_font_weight;                
  DWORD   m_font_charset;

  // normal (ANSI) colours
  COLORREF m_normalcolour [8];
  COLORREF m_boldcolour [8];

  // custom (user-defined) colours for triggers etc.
  COLORREF m_customtext [MAX_CUSTOM];
  COLORREF m_customback [MAX_CUSTOM];

//  unsigned short m_log_input;
//  unsigned short m_bLogNotes;
  unsigned short m_bEscapeDeletesInput;
  unsigned short m_bArrowsChangeHistory;
  unsigned short m_bConfirmOnPaste;

  unsigned short m_version;

  CString m_macros [MACRO_COUNT];                // for function keys
  unsigned short m_macro_type [MACRO_COUNT];     // see macro send types above
  CString m_macro_name [MACRO_COUNT];

// numeric keypad stuff
            
  CString m_keypad [eKeypad_Max_Items];  // 30 at present

  unsigned short m_keypad_enable;

// other interesting things

  unsigned short m_enable_speed_walk;
  CString m_speed_walk_prefix;

  unsigned short m_enable_command_stack;
//  unsigned char m_command_stack_character;
  CString m_strCommandStackCharacter;

  CString m_connect_text;

  CAliasMap m_AliasMap;
  CAliasArray m_AliasArray;       // array of aliases for sequencing
  CAliasRevMap m_AliasRevMap;     // for getting name back from pointer
  CTriggerMap m_TriggerMap;       
  CTriggerArray m_TriggerArray;   // array of triggers for sequencing
  CTriggerRevMap m_TriggerRevMap; // for getting name back from pointer
  CTimerMap m_TimerMap;
  CTimerRevMap m_TimerRevMap;     // for getting name back from pointer


// new in version 7

  COLORREF m_input_text_colour;
  COLORREF m_input_background_colour;

  LONG    m_input_font_height;               
  CString m_input_font_name;   
  BYTE    m_input_font_italic;            
  LONG    m_input_font_weight;                
  DWORD   m_input_font_charset;

// new in version 8

  LONG    m_maxlines;   // maximum lines in scrollback buffer
  LONG    m_nHistoryLines;  // maximum lines in command history
  unsigned short  m_nWrapColumn;    // column to wrap at

  CString m_paste_postamble;
  CString m_paste_preamble ;
  CString m_pasteline_postamble;
  CString m_pasteline_preamble ;

	CString	m_strLanguage;              // script language
	unsigned short  m_bEnableScripts;   // enable scripting
	COLORREF m_iHyperlinkColour;        // colour for hyperlinks
	CString	m_strWorldOpen;             // handler on world open
	CString	m_strWorldClose;            // handler on world close
	CString	m_strWorldSave;             // handler on world save
	CString	m_strWorldConnect;          // handler on world connect
	CString	m_strWorldDisconnect;       // handler on world disconnect
	CString	m_strScriptFilename;        // script filename
  CString m_strScriptPrefix;          // prefix to invoke scripts from the command window


  unsigned short m_iNoteTextColour;         // colour for note 
  unsigned short m_bKeepCommandsOnSameLine; // commands stay on same line as prompt from MUD

  CVariableMap m_VariableMap;        // program variables (map)

  CString         m_strAutoSayString;
  unsigned short  m_bEnableAutoSay;
  unsigned short  m_bExcludeMacros;
  unsigned short  m_bExcludeNonAlpha;
  CString         m_strOverridePrefix;     
  unsigned short  m_bConfirmBeforeReplacingTyping;
  unsigned short  m_bReEvaluateAutoSay; // send auto-say through command interpreter

  int m_nNormalPrintStyle [8];        // when printing, whether to use bold, underline etc.
  int m_nBoldPrintStyle [8];          // when printing, whether to use bold, underline etc.

  
// new in version 9

	unsigned short  m_bShowBold;   // show bold, italic, underline in fonts
	unsigned short  m_bAltArrowRecallsPartial;     // alt+up arrow recalls partially entered command
  unsigned short  m_iPixelOffset;     // pixel offset of text from side of window
  unsigned short  m_bAutoFreeze;    // freeze if not at bottom of buffer  
  unsigned short  m_bAutoRepeat;    // auto repeat last command 
  unsigned short  m_bDisableCompression;      // don't allow compressed worlds
  unsigned short  m_bLowerCaseTabCompletion;  // tab complete words in lower case    
  unsigned short  m_bDoubleClickInserts;      // double-click inserts word into command buffer
  unsigned short  m_bConfirmOnSend;        // confirm preamble, postamble on file send
  unsigned short  m_bTranslateGerman;        // translate German character sequences


// new in version 10

  CString m_strTabCompletionDefaults;          // initial words to check for tab completion
  unsigned int m_iTabCompletionLines;          // lines to search
  unsigned short m_bTabCompletionSpace;        // insert space after word?
  CString m_strAutoLogFileName;          // name of file to log to
  CString m_strLogLinePreambleOutput;          // put at start of each log line - output
  CString m_strLogLinePreambleInput;          // put at start of each log line - input
  CString m_strLogLinePreambleNotes;          // put at start of each log line - world.note
  CString m_strWorldGetFocus;          // script called when the world gets the focus
  CString m_strWorldLoseFocus;         // script called when the world loses the focus
  CString m_strLogFilePostamble;          // written when log file closed
  CString m_strRecallLinePreamble;       // line preamble for recall window

  unsigned short  m_bPasteCommentedSoftcode;   // are we pasting commented softcode?
  unsigned short  m_bFileCommentedSoftcode;    // are we sending commented softcode?
  unsigned short  m_bShowItalic;     // italic text to be shown in italic?
  unsigned short  m_bShowUnderline;     // underlined text to be underlined?
  unsigned short  m_bFlashIcon;     // flash icon on taskbar for new activity?
  unsigned short  m_bArrowKeysWrap;     // arrow keys wrap command history?
  unsigned short  m_bSpellCheckOnSend;        // spell check when you send something

  LONG  m_nPasteDelay;           // delay in milliseconds before pasting each line
  LONG  m_nFileDelay;            // delay in milliseconds before sending each line
  LONG  m_nReloadOption;      // option for reloading changed script file
  LONG  m_bUseDefaultOutputFont;            // Default output font overrides
  LONG  m_bSaveDeletedCommand;            // save deleted command in command history on <esc>
  LONG  m_bTranslateBackslashSequences;            // interpret \n \r etc. on a send
  LONG  m_bEditScriptWithNotepad;            // use inbuilt notepad for editing scripts
  LONG  m_bWarnIfScriptingInactive;       // warn if we can't invoke a script
  LONG  m_nPasteDelayPerLines;            // how many lines to send before the delay
  LONG  m_nFileDelayPerLines;            // how many lines to send before the delay

  // new in version 11

	unsigned short m_bDoubleClickSends;   // double-click sends to MUD
	unsigned short m_bWriteWorldNameToLog;    // write world name to log file
	unsigned short m_bSendEcho;
	unsigned short m_bPasteEcho;
	unsigned short m_bUseDefaultColours;
	unsigned short m_bUseDefaultTriggers;
	unsigned short m_bUseDefaultAliases;
	unsigned short m_bUseDefaultMacros;
	unsigned short m_bUseDefaultTimers;
	unsigned short m_bUseDefaultInputFont;

  CString m_strCustomColourName [255];    // custom colour names
  CString m_strTerminalIdentification;    // for telnet negotiation, defaults to ANSI

  // new in version 14

  CString m_strLogLinePostambleOutput;      // put at end of each log line - output
  CString m_strLogLinePostambleInput;       // put at end of each log line - input
  CString m_strLogLinePostambleNotes;       // put at end of each log line - world.note
  CString m_strMappingFailure;              // message for mapping failure

// callback routines for MXP
  CString m_strOnMXP_Start;                 // MXP starting up
  CString m_strOnMXP_Stop;                  // MXP closing down
  CString m_strOnMXP_Error;                 // MXP error
  CString m_strOnMXP_OpenTag;               // MXP tag open
  CString m_strOnMXP_CloseTag;              // MXP tag close
  CString m_strOnMXP_SetVariable;           // MXP variable set

  CString m_strBeepSound;                   // sound to play for beeps

	unsigned short m_bLogHTML;    // convert special HTML sequences (eg. < and > )
	unsigned short m_bUnpauseOnSend;    // cancel pause on next send to MUD
	unsigned short m_iSpeedWalkDelay;    // delay in ms for speed walking
	unsigned short m_bMapFailureRegexp;    // for expansion
	unsigned short m_iFlags1;       // various flags, see below
	unsigned short m_iFlags2;       // more flags
	unsigned short m_iUseMXP;       // whether to use MXP - see enum above
	unsigned short m_iMXPdebugLevel;     // level of MXP debugging


  CString m_strWorldID;           // unique ID for this world

  // new in version 15 

  unsigned short m_bAlwaysRecordCommandHistory; // record command history even if echo off
  unsigned short m_bCopySelectionToClipboard;   // automatically copy selection
  unsigned short m_bCarriageReturnClearsLine;   // \r character empties current line
  unsigned short m_bSendMXP_AFK_Response;       // reply to <afk> query?
  unsigned short m_bMudCanChangeOptions;        // server may recommend options
//  unsigned short m_bShowGridLinesInListViews;   // list views show grid
  unsigned short m_bEnableSpamPrevention;       // send spam filler every x lines
  unsigned short m_iSpamLineCount;              // send spam filler after this many lines the same
  CString        m_strSpamMessage;              // what spam filler to send

  unsigned short m_bDoNotShowOutstandingLines;  // don't show outstanding lines in tabbed windows
  unsigned short m_bDoNotTranslateIACtoIACIAC;  // don't translate outgoing IAC to IAC/IAC

  // new in 3.36

  CString m_strProxyServerName;     // name/IP address of proxy server
  unsigned short m_iProxyServerPort;  // proxy server port
  unsigned short m_iSocksProcessing;  // 0 = none, 1 = Socks 4, 2 = Socks 5
  CString m_strProxyUserName;     // user name for proxy authentication
  CString m_strProxyPassword;     // password for proxy authentication

  // more new stuff

  unsigned short m_bAutoCopyInHTML;     // auto-copy to clipboard in HTML
  unsigned short m_iLineSpacing;        // line spacing, 0 to use font height
  unsigned short m_bUTF_8;              // want UTF-8 support
  unsigned short m_bConvertGAtoNewline; // convert IAC/GA or IAC/EOR to a newline
  unsigned short m_iCurrentActionSource;// what caused the current script to run

  CString m_strTriggersFilter;          // Lua code to filter triggers
  CString m_strAliasesFilter;           // Lua code to filter aliases
  CString m_strTimersFilter;            // Lua code to filter timers
  CString m_strVariablesFilter;         // Lua code to filter variables


  unsigned short m_bScriptErrorsToOutputWindow;     // write script errors in window

  unsigned short m_bAutoResizeCommandWindow;   // change command window height automatically
  CString m_strEditorWindowName;               // What window to bring to the front after editing a text file
  unsigned short m_iAutoResizeMinimumLines;    // command window minimum number of lines
  unsigned short m_iAutoResizeMaximumLines;    // command window maximum number of lines
  unsigned short m_bDoNotAddMacrosToCommandHistory;    // macros not into command history
  unsigned short m_bSendKeepAlives;            // set the socket option SO_KEEPALIVE on the TCP/IP connections

  // end of stuff saved to disk **************************************************************

  // stuff from pre version 11, read from disk but not saved

  unsigned short m_page_colour;
  unsigned short m_whisper_colour;
  unsigned short m_mail_colour;
  unsigned short m_game_colour;
  unsigned short m_chat_colour;         

  unsigned short m_remove_channels1;
  unsigned short m_remove_channels2;
  unsigned short m_remove_pages;
  unsigned short m_remove_whispers;
  unsigned short m_remove_set;
  unsigned short m_remove_mail;
  unsigned short m_remove_game;

  // end of pre version 11 stuff


  // from m_iFlags1;

  BOOL m_bArrowRecallsPartial;    // arrow key (without ALT) recalls partial command
  BOOL m_bCtrlZGoesToEndOfBuffer;       // Ctrl+Z goes to end of output buffer
  BOOL m_bCtrlPGoesToPreviousCommand;   // Ctrl+P gets previous command
  BOOL m_bCtrlNGoesToNextCommand;       // Ctrl+N gets next command
  BOOL m_bHyperlinkAddsToCommandHistory;  // MXP hyperlink goes into history
  BOOL m_bEchoHyperlinkInOutputWindow;    // MXP hyperlink is echoed
  BOOL m_bAutoWrapWindowWidth;          // auto wrap to current window width
  BOOL m_bNAWS;                         // Negotiate About Window Size
  BOOL m_bPueblo;                       // Allow Pueblo worlds
  BOOL m_bNoEchoOff;                    // ignore attempts to stop command echoing
  BOOL m_bUseCustomLinkColour       ;    // use our custom link colour
  BOOL m_bMudCanChangeLinkColour    ;    // however MUD can override it
  BOOL m_bUnderlineHyperlinks       ;    // underline our hyperlinks
  BOOL m_bMudCanRemoveUnderline     ;    // however MUD can remove the underline


  // end of m_iFlags1


  // from m_iFlags2;

  BOOL m_bAlternativeInverse;     // alternative way of displaying bold inverse
  BOOL m_bShowConnectDisconnect;  // show message in output window on connect/disconnect
  BOOL m_bIgnoreMXPcolourChanges; // don't let MXP/Pueblo change output colours
  BOOL m_bCustom16isDefaultColour;  // user custom colour 16 after ANSI reset
  BOOL m_bLogInColour;            // HTML logging is in colour
  BOOL m_bLogRaw;                 // write raw input from mud to log file

  // end of m_iFlags2

  BOOL m_bNAWS_wanted;     // server wants NAWS messages
  BOOL m_bCHARSET_wanted;  // server wants CHARSET messages
  bool m_bLoaded;          // true if we have loaded from disk
  bool m_bSelected;        // true if selected in Send To All Worlds
  bool m_bVariablesChanged; // true if variables have changed

  bool m_bNoEcho; // set if we get IAC WILL ECHO
                  // cleared if we get IAC WONT ECHO
  bool m_bDebugIncomingPackets;   // set if we want to display all incoming text

  __int64 m_iInputPacketCount;   // count of packets received
  __int64 m_iOutputPacketCount;   // count of packets sent
  long m_iUTF8ErrorCount;      // count of lines with bad UTF8

  long m_iTriggersEvaluatedCount;    // how many triggers we evaluated
  long m_iTriggersMatchedCount;      // how many triggers matched
  long m_iAliasesEvaluatedCount;     // how many aliases we evaluated 
  long m_iAliasesMatchedCount;       // how many aliases matched      
  long m_iTimersFiredCount;          // how many timers fired

  long m_iTriggersMatchedThisSessionCount;      // how many triggers matched this connection
  long m_iAliasesMatchedThisSessionCount;       // how many aliases matched this connection      
  long m_iTimersFiredThisSessionCount;          // how many timers fired this connection

  int m_last_prefs_page;
  unsigned short m_bConfigEnableTimers;   // used when doing world config

  CSendView * m_pActiveCommandView;
  CMUSHView * m_pActiveOutputView;

// list of output buffer lines

  CLineList m_LineList;

  CLine * m_pCurrentLine;   // the line we are currently receiving

  CString m_strCurrentLine;   // line currently received from MUD (excluding control codes)

// list of actions for style runs

  CActionList m_ActionList;

// we will record the position of each JUMP_SIZE lines, to speed up lookups

  POSITION * m_pLinePositions;  // array of line positions

  long m_total_lines;
  long m_new_lines;   // lines they haven't read yet (if not active view)
  long m_nTotalLinesSent;   // lines sent this connection
  long m_nTotalLinesReceived;  // lines they received this connection
  long m_last_line_with_IAC_GA;

  CTime m_tConnectTime;       // time we connected to the world
  CTime m_tLastPlayerInput;   // time the player last typed something (for <afk> )
  CTimeSpan m_tsConnectDuration;  // timespan we have been connected

  CTime m_tStatusTime;    // time of line that mouse was over
  CPoint m_lastMousePosition;  // where mouse last was over the world window

  int m_view_number;  // sequence in activity view

  int m_phase;        // telnet negotiation phase

  // MCCP (MUD Client Compression Protocol) stuff
  z_stream m_zCompress;         // compression structure
  bool m_bCompress;             // true if decompressing
  bool m_bCompressInitOK;       // true if OK to decompress
  Bytef * m_CompressInput;        // input buffer for decompresion
  Bytef * m_CompressOutput;       // output buffer for decompresion
  __int64 m_nTotalUncompressed;
  __int64 m_nTotalCompressed;
  LONGLONG m_iCompressionTimeTaken;   // time taken to decompress


  int m_iMCCP_type;   // MCCP protocol type in use: 0 = none, 1 = v1, 2 = v2
  bool m_bSupports_MCCP_2;    // if true we have agreed to support MCCP v 2

  // end MCCP stuff

  // Telnet subnegotiation

  int m_subnegotiation_type;         // what type of subnegotiation we are currently in (0 to 255)
  string m_IAC_subnegotiation_data;  // last string x from MUD: IAC SB c x IAC SE
  bool m_bClient_IAC_DO [256];     // client sent IAC DO x
  bool m_bClient_IAC_DONT [256];     // client sent IAC DONT x
  bool m_bClient_IAC_WILL [256];     // client sent IAC WILL x
  bool m_bClient_IAC_WONT [256];     // client sent IAC WONT x

  // MSP (MUD Sound Protocol) stuff
  bool m_bMSP;  // true if using MSP

  // end MSP stuff

  // MXP (MUD Extension Protocol) stuff

  bool m_bMXP;       // true if using MXP at present
  bool m_bPuebloActive;    // Pueblo mode :)
  CString  m_iPuebloLevel;    // ie. 1.0, 1.10
  bool m_bPreMode;    // <PRE> tag active
  int m_iMXP_mode;   // current tag security (eg. eMXP_open)
  int m_iMXP_defaultMode; // default tag mode (after newline)
  int m_iMXP_previousMode; // previous mode before mode 4 (secure-once mode)
  bool m_bInParagraph; // discard newlines (wrap)
  bool m_bMXP_script;   // in script collection mode
  bool m_bSuppressNewline;        // newline does NOT start a new line

  // NB - lists are being done in a hurry - I should really allow for nesting them
  int m_iListMode;      // see enum above - what sort of list we are displaying
  int m_iListCount;     // for ordered list - how far we are through

  CString m_strMXPstring;         // string we are currently collecting, eg. <bol...
  CString m_strMXPtagcontents;    // stuff *inside* a tag, for use with &text;
  char    m_cMXPquoteTerminator;  // ' or "

  // These two are defined with !ELEMENT and !ENTITY
  CElementMap         m_CustomElementMap;   // Custom MXP elements 
  CMapStringToString  m_CustomEntityMap;    // Custom MXP entities

  CActiveTagList  m_ActiveTagList;  // MXP elements currently active (eg. <bold> )

  char m_cLastChar;   // last incoming character (for <p> stuff)

  CString m_strLastDebugWorldInput;

  int m_iLastOutstandingTagCount;

  CString m_strPuebloMD5; // Pueblo hash string

  // MXP stats

  __int64 m_iMXPerrors;
  __int64 m_iMXPtags;
  __int64 m_iMXPentities;

  // end MXP stuff

  int m_code;             // current ANSI code (colour, hilite etc.)

  int m_lastGoTo;         // last line we went to

  bool  m_bWorldClosing;    // true if world is closing

// we save the current style here on any style change *from the mud*
// we don't want to mix up notes/user input with mud-set styles

  // these values are set by RememberStyle

  // current style
  unsigned short m_iFlags;      // see define in othertypes.h
  COLORREF       m_iForeColour; // RGB foreground colour, or ANSI/custom colour number
  COLORREF       m_iBackColour; // RGB background colour, or ANSI/custom colour number

  bool      m_bNotesInRGB;         // are notes in RGB mode?  (ie. world.note, world.tell)
  COLORREF  m_iNoteColourFore;     // RGB notes text colour
  COLORREF  m_iNoteColourBack;     // RGB notes background colour
  unsigned short  m_iNoteStyle;    // notes style: HILITE, UNDERLINE, BLINK, INVERSE       

  FILE * m_logfile;
  CString m_logfile_name;
  CTime m_LastFlushTime;

  CFont * m_font [8];     // 8 fonts - normal, bold, italic, bold-normal etc.
  int m_FontHeight,
      m_FontWidth; 

  CFont * m_input_font;
  int m_InputFontHeight,
      m_InputFontWidth; 

  __int64 m_nBytesIn;
  __int64 m_nBytesOut;

// sockets stuff

	SOCKADDR_IN m_sockAddr;       // MUD address/port
	SOCKADDR_IN m_ProxyAddr;      // Proxy address/port

  HANDLE      m_hNameLookup;
  char *      m_pGetHostStruct;
  int         m_iConnectPhase;    // see enum above


// chatting

  CChatSocketList m_ChatList;           // list of chat calls
  CString         m_strOurChatName;     // who we are on chat lists
  unsigned  short m_IncomingChatPort;   // default 4050
  unsigned  short m_bAcceptIncomingChatConnections;   // do we listen for incoming chats?
  unsigned  short m_bIgnoreChatColours;   // throw away incoming ANSI colours
  unsigned  short m_bValidateIncomingCalls; // ask before accepting chat connections
  unsigned  short m_bAutoAllowSnooping;   // once snooping is permitted don't ask again
  unsigned  short m_bAutoAllowFiles;      // don't query if we can send a particular file
//  unsigned  short m_bChatSumchecksToSender;  // tell sender sumchecks too
  COLORREF        m_cChatForegroundColour;  // default colour for chats
  COLORREF        m_cChatBackgroundColour;  //  - background for above
  CChatListenSocket * m_pChatListenSocket;  // for listening for chat connections
  long            m_iNextChatID;            // next available chat session ID
  CString         m_strLastMessageSent;   // the last message we sent to everybody
  CTime           m_tLastMessageTime;     // time we sent it
  CString         m_strLastGroupMessageSent;   // the last message we sent to a group
  CTime           m_tLastGroupMessageTime;     // time we sent it
  CString         m_strChatMessagePrefix;   // <CHAT> or whatever
  CString         m_strChatSaveDirectory;     // where to save files
  long            m_iMaxChatLinesPerMessage;   // max lines per chat message
  long            m_iMaxChatBytesPerMessage;   // max bytes per chat message


  unsigned short  m_iLastOutgoingChatPort;
  CString         m_strLastOutgoingChatAddress;

  set<string>     m_ExtraShiftTabCompleteItems;  // for Shift+Tab completion
  bool            m_bTabCompleteFunctions;

// scripting

  CScriptEngine * m_ScriptEngine;

  bool m_bSyntaxErrorOnly;
  bool m_bDisconnectOK;
  bool m_bTrace;
  bool m_bInSendToScript;
  LONGLONG m_iScriptTimeTaken;        // time taken to execute scripts

  CString m_strLastImmediateExpression;

	HANDLE	m_pThread;			// Notification thread
	CEvent  m_eventScriptFileChanged;		// script file changed thread event
  bool m_bInScriptFileChanged;
  CTime m_timeScriptFileMod;

  CString m_strStatusMessage;   // "ready" or user-supplied message
  CTime m_tStatusDisplayed; // when we last displayed the status line

  CString m_strScript;    // script as read from file

  // Lua note - for Lua the DISPID is a flag indicating whether or not
  // the routine exists. It is set to DISPID_UNKNOWN if the last call caused an error
  // For Lua, it will be 1 for "routine exists" and DISPID_UNKNOWN if not.

	DISPID	m_dispidWorldOpen;             // handler on world open
	DISPID	m_dispidWorldClose;            // handler on world close
	DISPID	m_dispidWorldSave;             // handler on world save
	DISPID	m_dispidWorldConnect;          // handler on world connect
	DISPID	m_dispidWorldDisconnect;       // handler on world disconnect
	DISPID	m_dispidWorldGetFocus;         // handler on world getting focus
	DISPID	m_dispidWorldLoseFocus;        // handler on world losing focus
	DISPID	m_dispidOnMXP_Start;           // handler on MXP startup
	DISPID	m_dispidOnMXP_Stop;            // handler on MXP shutdown
	DISPID	m_dispidOnMXP_OpenTag;         // handler on MXP opening tag, eg. <send>           
  DISPID	m_dispidOnMXP_CloseTag;        // handler on MXP closing tag, eg. </send>
	DISPID	m_dispidOnMXP_SetVariable;     // handler on MXP set variable, eg. set hp
	DISPID	m_dispidOnMXP_Error;           // handler on MXP error (parse etc.)

  bool    m_bPluginProcessesOpenTag;     // does a plugin handle the open tag?
  bool    m_bPluginProcessesCloseTag;    // does a plugin handle the close tag?
  bool    m_bPluginProcessesSetVariable; // does a plugin handle set variable?
  bool    m_bPluginProcessesSetEntity;   // does a plugin handle set entity?
  bool    m_bPluginProcessesError;       // does a plugin handle the error?

  // listening for UDP packets

  map<int, UDPsocket *> m_UDPsocketMap;

// stuff for finding in the output buffer

  CFindInfo m_DisplayFindInfo;

// other find info stuff

  CFindInfo m_RecallFindInfo;
  CFindInfo m_TriggersFindInfo;
  CFindInfo m_AliasesFindInfo;
  CFindInfo m_MacrosFindInfo;
  CFindInfo m_TimersFindInfo;
  CFindInfo m_VariablesFindInfo;
  CFindInfo m_NotesFindInfo;

  // flags for recall window
  bool m_bRecallCommands;
  bool m_bRecallOutput;
  bool m_bRecallNotes; 

  __int64 m_iUniqueDocumentNumber;

  CStringList m_strMapList;
  t_regexp * m_MapFailureRegexp;
  CString m_strSpecialForwards;
  CString m_strSpecialBackwards;

  CTimerWnd * m_pTimerWnd;    // for speed walk delays
	CStringList m_QueuedCommandsList;  // queue of delayed commands (for speed walking)
  bool m_bShowingMapperStatus;  // if set, don't change status line for 5 seconds
  CStringList m_strIncludeFileList;         // list of root level include files
  CStringList m_strCurrentIncludeFileList;  // list of current tree of include files

  CNumericConfigurationArray m_NumericConfiguration;
  CAlphaConfigurationArray m_AlphaConfiguration;

  CPluginList m_PluginList;     // plugins
  CPlugin *   m_CurrentPlugin;  // plugin currently active, NULL if none
  bool        m_bPluginProcessingCommand; // plugin is doing ON_PLUGIN_COMMAND
  bool        m_bPluginProcessingSend; // plugin is doing ON_PLUGIN_SEND
  bool        m_bPluginProcessingSent; // plugin is doing ON_PLUGIN_SENT

  CString     m_strLastCommandSent;   // for spam prevention
  int         m_iLastCommandCount;    // number of times last command sent
  int         m_iExecutionDepth;      // count of nested calls to world.Execute
  bool        m_bOmitFromCommandHistory;  // alias wants to omit from command history?

  tStringMapOfMaps m_Arrays;    // map of arrays (for scripting)

  deque<string> m_sRecentLines; // for multi-line triggers


  CString m_strSpecialFontName;
  HANDLE  m_hSpecialFontHandle;

  long AddSpecialFont (LPCTSTR PathName);
  void RemoveSpecialFont (void);

  // background image
  CString m_strBackgroundImageName;
  CBitmap m_BackgroundBitmap;
  int  m_iBackgroundMode;

  COLORREF m_iBackgroundColour;

  // foreground image
  CString m_strForegroundImageName;
  CBitmap m_ForegroundBitmap;
  int  m_iForegroundMode;

  // all our cute little miniwindows
  MiniWindowMap m_MiniWindows;

  // all our databases
  tDatabaseMap m_Databases;

  // text rectangle
  CRect m_TextRectangle;
  long m_TextRectangleBorderOffset, 
       m_TextRectangleBorderColour, 
       m_TextRectangleBorderWidth, 
       m_TextRectangleOutsideFillColour, 
       m_TextRectangleOutsideFillStyle;


  // for DirectSound
  LPDIRECTSOUNDBUFFER m_pDirectSoundSecondaryBuffer [MAX_SOUND_BUFFERS];
  long PlaySoundHelper(short Buffer, LPCTSTR FileName, BOOL Loop, double Volume, double Pan, LPCTSTR MemoryBuffer, size_t MemLen);


  WORD m_nextAcceleratorCommand;  // next free command
  map<long, WORD> m_AcceleratorToCommandMap;  // command to send for accelerator
  map<WORD, string> m_CommandToMacroMap;      // what that command does
  map<WORD, short> m_CommandToSendToMap;      // where it goes (eg. 0 = world, 1 = command etc.)
  HACCEL m_accelerator;

  // for mapping colours to colours
  map<COLORREF, COLORREF> m_ColourTranslationMap;

#ifdef PANE
  // for pane windows

  CPaneMap m_PaneMap;
#endif // PANE

  protected:

// Operations
public:
	BOOL ConnectSocket(void);
	void ProcessPendingRead();
	void DoSendMsg(const CString& strText, 
                 const bool bEchoIt,
                 const bool bLogIt);
	void SendMsg(CString strText, 
               const bool bEchoIt,
               const bool bQueueIt,
               const bool bLogIt);
	void ReceiveMsg();
	void DisplayMsg(LPCTSTR lpszText, int size, const int flags);
  void AddToLine (LPCTSTR lpszText, const int flags);
  void StartNewLine_KeepPreviousStyle (const int flags);
  void Phase_ESC (const unsigned char c);            
  void Phase_ANSI (const unsigned char c);           
  void Phase_IAC (unsigned char & c);            
  void Phase_WILL (const unsigned char c);           
  void Phase_WONT (const unsigned char c);           
  void Phase_DO (const unsigned char c);             
  void Phase_DONT (const unsigned char c);           
  void Phase_SB (const unsigned char c); 
  void Phase_SUBNEGOTIATION (const unsigned char c); 
  void Phase_SUBNEGOTIATION_IAC (const unsigned char c); 
  void Phase_COMPRESS (const unsigned char c);    
  void Phase_COMPRESS_WILL (const unsigned char c);    
  bool Handle_Telnet_Request (const int iNumber, const string sType);
  void Handle_IAC_GA ();

  void Handle_TELOPT_COMPRESS2 ();
  void Handle_TELOPT_MUD_SPECIFIC ();
  void Handle_TELOPT_MXP ();
  void Handle_TELOPT_TERMINAL_TYPE ();
  void Handle_TELOPT_CHARSET ();

  void Send_IAC_DO (const unsigned char c);
  void Send_IAC_DONT (const unsigned char c);
  void Send_IAC_WILL (const unsigned char c);
  void Send_IAC_WONT (const unsigned char c);


  // mxp collection phases
  void Phase_MXP_ELEMENT (const unsigned char c);            
  void Phase_MXP_COMMENT (const unsigned char c);            
  void Phase_MXP_QUOTE (const unsigned char c);           
  void Phase_MXP_ENTITY (const unsigned char c);            
  void Phase_MXP_COLLECT_SPECIAL (const unsigned char c);           

  // other MXP processing

  void MXP_unterminated_element (const char * pReason);
  void MXP_mode_change (int iNewMode);
  void MXP_collected_element (void);
  void MXP_collected_entity (void);
  void MXP_error (const int iLevel, const long iMessageNumber, CString strMessage);
  void MXP_Definition (CString strTag);
  void MXP_Element (CString strName, CString strTag);
  void MXP_Entity  (CString strName, CString strTag);
  void MXP_Attlist (CString strName, CString strTag);
  void MXP_StartTag (CString strTag);
  void MXP_EndTag (CString strTag);
  void MXP_CloseTag (CString strTag, const bool bOpen = false);
  void MXP_CloseOpenTags (void);
  void MXP_CloseAllTags (void);
  void MXP_On (const bool bPueblo = false, const bool bManual = false);     // turning MXP/Pueblo on
  void MXP_Off (const bool bCompletely = false);  // turning MXP off
  void MXP_OpenAtomicTag (const CString strTag,   // name
                          int iAction,            // action code
                          CStyle * pStyle,        // style it should modify
                          CString & strAction,    // new action
                          CString & strHint,      // new hint
                          CString & strVariable,  // new variable
                          CArgumentList & ArgumentList);  // args
  void MXP_CloseAtomicTag (const int iAction, 
                           const CString & strText,
                           const POSITION firstlinepos,
                           const POSITION firststylepos);
  CString MXP_GetEntity (CString & strName);
  bool BuildArgumentList (CArgumentList & ArgumentList, 
                          CString strTag);

  void CheckArgumentsUsed (const CString strTag, CArgumentList & ArgumentList);
  bool MXP_StartTagScript (const CString & strName, 
                           const CString & strArguments,
                           CArgumentList & ArgumentList);

  void AddToCommandHistory (LPCTSTR Message);

  // simple test to see if we are in secure mode right now
  inline bool MXP_Secure (void)
    { 
    return m_iMXP_mode == eMXP_secure ||
           m_iMXP_mode == eMXP_secure_once ||
           m_iMXP_mode == eMXP_perm_secure;
    }

  // simple test to see if we are in open mode right now
  inline bool MXP_Open (void)
    { 
    return m_iMXP_mode == eMXP_open ||
           m_iMXP_mode == eMXP_perm_open;
    }
  
  // cancels secure-once mode
  inline void MXP_Restore_Mode (void)
    {
    if (m_iMXP_mode == eMXP_secure_once)
      m_iMXP_mode = m_iMXP_previousMode; 
    }

  void Debug_MUD (const char * sCaption, const char * sData);
  void Debug_Packets (LPCTSTR sCaption, LPCTSTR lpszText, int size, const __int64 iNumber);
  void SendPacket (const char * lpBuf, const int nBufLen);   // low-level send
  void SendPacket (const unsigned char * lpBuf, const int nBufLen);   // low-level send

  // finds an action, adding it if necessary
  CAction * GetAction (const CString & strAction,
                       const CString & strHint,
                       const CString & strVariable);

  bool CheckConnected (void);
  bool FindStyle (const CLine * pLine,         // which line
                  const int iWantedCol,  // which column we want
                  int & iCol,            // which column style goes up to
                  CStyle * & pStyle,     // return style pointer
                  POSITION & foundpos) const;  // and its position

  void GetStyleRGB (CStyle * pStyle, COLORREF & colour1, COLORREF & colour2) const;

  CStyle * AddStyle (const unsigned short iFlags,
                     const COLORREF       iForeColour,
                     const COLORREF       iBackColour,
                     const int            iLength = 0,
                     CString              strAction = "",
                     CString              strHint = "",
                     CString              strVariable = "",
                     CLine *              pLine = NULL);

  CStyle * AddStyle (const unsigned short iFlags,
                     const COLORREF       iForeColour,
                     const COLORREF       iBackColour,
                     const int            iLength = 0,
                     CAction *            pAction = NULL,
                     CLine *              pLine = NULL);

  void RememberStyle (const CStyle * pStyle); 

  void StartNewLine (const bool hard_break, const int flags);
  bool ProcessPreviousLine (void);
  void SendLineToPlugin (void);
  void SetNewLineColour (const int flags);

  void ProcessOneTriggerSequence (CString & strCurrentLine,
                            CPaneLine & StyledLine,
                            CString & strResponse,
                            const POSITION prevpos,
                            bool & bNoLog,
                            bool & bNoOutput,
                            bool & bChangedColour,
                            CTriggerList & triggerList,
                            CString & strExtraOutput,
                            ScriptItemMap & mapDeferredScripts,
                            OneShotItemMap & mapOneShotItems);

  bool ProcessOneAliasSequence (const CString strCurrentLine,
                            const bool bCountThem,
                            bool & bOmitFromLog,
                            bool & bEchoAlias,
                            CAliasList & AliasList,
                            OneShotItemMap & mapOneShotItems);

  void WriteToLog (const char * text, size_t len);
  void WriteToLog (const CString & strText);
  void LogLineInHTMLcolour (POSITION startpos);
  void LogCommand (const char * text);

  BOOL OpenSession (void);
  void SetUpOutputWindow (void);

  bool m_bInPlaySoundFilePlugin;
  bool m_bInCancelSoundFilePlugin;
  bool PlaySoundFile (CString strSound);
  void CancelSound (void);

  CChildFrame * GetChildFrame (void);

  void ChangeFont ( const int nHeight, 
                    const char * lpszFacename,
                    const int nWeight, 
                    const DWORD iFontCharset,
                    const BOOL bShowBold,
                    const BOOL bShowItalic,
                    const BOOL bShowUnderline,
                    const unsigned short iLineSpacing);
  void ChangeInputFont ( const int nHeight, 
                         const char * lpszFacename,
                         const int nWeight, 
                         const DWORD iCharset,
                         const BYTE bItalic);

  void setupstrings (void);

  void Serialize_World_XML (CArchive& ar);

// bits for world saving mask
#define XML_GENERAL    0x0001
#define XML_TRIGGERS   0x0002
#define XML_ALIASES    0x0004
#define XML_TIMERS     0x0008
#define XML_MACROS     0x0010
#define XML_VARIABLES  0x0020
#define XML_COLOURS    0x0040
#define XML_KEYPAD     0x0080
#define XML_PRINTING   0x0100
#define XML_INCLUDES   0x0200
#define XML_PLUGINS    0x0400
#define XML_NO_PLUGINS 0x0800
#define XML_OVERWRITE  0x1000

  // XML saving
  void Save_World_XML (CArchive& ar, 
                       const unsigned long iMask,
                       CString strComment = "");

  void Save_Header_XML (CArchive& ar, const char * sName, const bool bShowVersion = true);
  void Save_Footer_XML (CArchive& ar, const char * sName);
  void Save_General_XML (CArchive& ar);
  void Save_Triggers_XML (CArchive& ar);
  void Save_One_Trigger_XML (CArchive& ar, CTrigger * t);
  void Save_Aliases_XML (CArchive& ar);
  void Save_One_Alias_XML (CArchive& ar, CAlias * a);
  void Save_Variables_XML (CArchive& ar);
  void Save_One_Variable_XML (CArchive& ar, CVariable * v);
  void Save_Timers_XML (CArchive& ar);
  void Save_One_Timer_XML (CArchive& ar, CTimer * t);
  void Save_Macros_XML (CArchive& ar);
  void Save_One_Macro_XML (CArchive& ar, const int iIndex);
  void Save_Colours_XML (CArchive& ar);
  void Save_Keypad_XML (CArchive& ar);
  void Save_One_Keypad_XML (CArchive& ar, const int iIndex);
  void Save_Printing_XML (CArchive& ar);
  void Save_Version_And_Date_XML (CArchive& ar);
  void Save_Includes_XML (CArchive& ar);
  void Save_Plugins_XML (CArchive& ar);
  void Save_World_Numeric_Options_XML (CArchive& ar);
  void Save_World_Single_Line_Alpha_Options_XML (CArchive& ar);
  void Save_World_Multi_Line_Alpha_Options_XML (CArchive& ar);

#define LOAD_INCLUDE        0x01    // loading an include file
#define LOAD_OVERWRITE      0x02    // overwrites existing values (triggers, etc.)
#define LOAD_KEEP           0x04    // use include file in preference to non-include
#define LOAD_WARN           0x08    // warn if clash

  // XML loading
  UINT Load_World_XML (CArchive& ar, 
                       const unsigned long iMask,
                       const unsigned long iFlags = 0,
                        UINT * piTriggers = NULL,
                        UINT * piAliases = NULL,
                        UINT * piTimers = NULL,
                        UINT * piMacros = NULL,
                        UINT * piVariables = NULL,
                        UINT * piColours = NULL,
                        UINT * piKeypad = NULL,
                        UINT * piPrinting = NULL);

  void LoadError (const char * sType, const char * sMessage, UINT iLine = 0);
  void CheckUsed (CXMLelement & node);
  void HandleLoadException (const char * sComment, CException* e);

  void Load_Includes_XML (CXMLelement & parent,
                      const unsigned long iMask,
                      const unsigned long iFlags,
                      const bool bPlugins,  // only if plugin="y"
                      UINT * piTriggers,
                      UINT * piAliases,
                      UINT * piTimers,
                      UINT * piMacros,
                      UINT * piVariables,
                      UINT * piColours,
                      UINT * piKeypad,
                      UINT * piPrinting);
  void Load_One_Include_XML (CXMLelement & node,
                      const unsigned long iMask,
                      const unsigned long iFlags,
                      const bool bPlugins,  // only if plugin="y"
                      UINT * piTriggers,
                      UINT * piAliases,
                      UINT * piTimers,
                      UINT * piMacros,
                      UINT * piVariables,
                      UINT * piColours,
                      UINT * piKeypad,
                      UINT * piPrinting);

  void Load_General_XML (CXMLelement & parent, 
    const unsigned long iFlags);
  UINT Load_Triggers_XML (CXMLelement & parent, 
    const unsigned long iMask,
    const unsigned long iFlags);
  void Load_One_Trigger_XML (CXMLelement & node, 
    const unsigned long iMask,
    const long iVersion, 
    bool bUseDefault, 
    const unsigned long iFlags);
  UINT Load_Aliases_XML (CXMLelement & parent, 
    const unsigned long iMask,
    const unsigned long iFlags);
  void Load_One_Alias_XML (CXMLelement & node, 
    const unsigned long iMask,
    const long iVersion, 
    bool bUseDefault, 
    const unsigned long iFlags);
  UINT Load_Variables_XML (CXMLelement & parent, 
    const unsigned long iMask,
    const unsigned long iFlags);
  void Load_One_Variable_XML (CXMLelement & node, 
    const unsigned long iMask,
    const long iVersion, 
    bool bUseDefault, 
    const unsigned long iFlags);
  UINT Load_Timers_XML (CXMLelement & parent, 
    const unsigned long iMask,
    const unsigned long iFlags);
  void Load_One_Timer_XML (CXMLelement & node, 
    const unsigned long iMask,
    const long iVersion, 
    bool bUseDefault, 
    const unsigned long iFlags);
  UINT Load_Macros_XML (CXMLelement & parent, 
    const unsigned long iFlags);
  void Load_One_Macro_XML (CXMLelement & node, 
    const long iVersion, 
    bool bUseDefault, 
    const unsigned long iFlags);
  UINT Load_Colours_XML (CXMLelement & parent, 
    const unsigned long iFlags);
  void Load_One_Colour_XML (CXMLelement & node, 
    const long iVersion, 
    const int iGroup, 
    bool bUseDefault, 
    const unsigned long iFlags);
  UINT Load_Keypad_XML (CXMLelement & parent, 
    const unsigned long iFlags);
  void Load_One_Key_XML (CXMLelement & node, 
    const long iVersion, 
    bool bUseDefault, 
    const unsigned long iFlags);
  UINT Load_Printing_XML (CXMLelement & parent, 
    const unsigned long iFlags);
  void Load_One_Print_Colour_XML (CXMLelement & node, 
    const long iVersion, 
    const int iGroup, 
    bool bUseDefault, 
    const unsigned long iFlags);
  void Load_Version_And_Date_XML (CXMLelement & parent);
  void Load_Comments_XML (CXMLelement & parent);
  void Load_Plugin_XML (CXMLelement & parent);
  void Load_Scripts_XML (CXMLelement & parent);
  void Load_World_Numeric_Options_XML (CXMLelement & parent, 
                              bool bUseDefault,
                              const unsigned long iFlags);
  void Load_World_Single_Line_Alpha_Options_XML (CXMLelement & parent, 
                              bool bUseDefault,
                              const unsigned long iFlags);
  void Load_World_Multi_Line_Alpha_Options_XML (CXMLelement & parent, 
                              bool bUseDefault,
                              const unsigned long iFlags);

  void InternalLoadPlugin (const CString & strName);

  // set up trigger array after adding a trigger or two
  void SortTriggers (void);
  // set up alias array after adding an alias or two
  void SortAliases (void);
  // set up timer reverse map after adding timers
  void SortTimers (void);


  BOOL Load_Set (const int set_type, 
                 CString strFileName,
                 CWnd * parent_window);  
  BOOL Save_Set (const int set_type, 
                 CWnd * parent_window);  
  
  BOOL EvaluateCommand (const CString & input, 
                        const bool bCountThem,
                        bool & bOmitFromLog,
                        const bool bTest = false);

  CTrigger * EvaluateTrigger (const CString & input, 
                              CString & output, 
                              int & iItem,
                              int & iStartCol,
                              int & iEndCol);

  CString FixSendText (const CString strSource, 
                            const int iSendTo,
                            const t_regexp * regexp,    // regular expression (for triggers, aliases)
                            const char * sLanguage, // language for send-to-script
                            const bool bMakeWildcardsLower,
                            const bool bExpandVariables,
                            const bool bExpandWildcards,
                            const bool bFixRegexps, // convert \ to \\ for instance
                            const bool bIsRegexp,   // true = regexp trigger
                            const bool bThrowExceptions);   // throw exception on error

#ifdef PANE

  void SendToPane (const CString strSource,      // what to send (eg. %1 = %2)
                   const CPaneLine & StyledLine, // the styled text to be sent
                   const string sPaneName,       // which pane
                   const t_regexp * regexp,      // regular expression (for triggers, aliases)
                   const bool bMakeWildcardsLower, // force wildcards to lower-case?
                   const bool bExpandVariables); // expand variables (eg. @foo)

  void TriggerToPane (CTrigger * pTrigger, CPaneLine & StyledLine);

  CPaneView * GetPane (LPCTSTR Name);

#endif // PANE

  const COLORREF TranslateColour (const COLORREF & source) const;

  void OnConnect(int nErrorCode);
  void HostNameResolved (WPARAM wParam, LPARAM lParam);
  const char * GetSocketError (int nError);
  bool LookupHostName (LPCTSTR sName);
  void InitiateConnection (void);
  void ConnectionEstablished (void);

  int ProcessProxyResponse1 (char * buff, const int count);
  int ProcessProxyResponse2 (char * buff, const int count);
  int ProcessProxyResponse3 (char * buff, const int count);
  bool CheckExpectedProxyResponse (const char cExpected, const char cReceived);
  void SendProxyConnectSequence (void);
  bool SendProxyUserNameAndPassword (void);

  BOOL FixUpOutputBuffer (int nNewBufferSize);
  bool SendToMushHelper (CFile * f, 
                         CString & strPreamble,
                         CString & strLinePreamble,
                         CString & strLinePostamble,
                         CString & strPostamble,
                         const BOOL bCommentedSoftcode,
                         const long nLineDelay,
                         const long nPerLines,
                         const BOOL bConfirm,
                         const BOOL bEcho);

  POSITION GetLinePosition (long nLine);
  long GetLastLine (void);
  void RemoveChunk (void);

  void InterpretANSIcode (const int iCode);
  void Interpret256ANSIcode (const int iCode);
  void ResetOneTimer (CTimer * timer_item);
  void ResetAllTimers (CTimerMap & TimerMap);
  void CheckTimerList (CTimerMap & TimerMap);
  void CheckTimers (void);
  void CheckTickTimers (void);

  virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace);

  virtual HACCEL GetDefaultAccelerator();

  void SendWindowSizes (const int iNewWidth);
//  void ShowHyperlink (const CString strLink);
  CString DelayedSend(const CString strMessage, const bool bEchoIt);
  void SendTo (const unsigned short iWhere, 
               const CString strSendText,
               const bool bOmitFromOutput,
               const bool bOmitFromLog,
               const CString strDescription,
               const CString strVariable,
               CString & strOutput);

  // send a chat message to all connections
  int SendChatMessageToAll 
              (const int iMessage,         // message type (see enum)
               const CString strMessage,   // the message
               const bool bUnlessIgnoring, // don't send if we are ignoring them
               const bool bServed,         // if true, to incoming only
               const bool bNotServed ,      // if true, to outgoing only
               const long nExceptID,           // not to this ID
               const CString strGroup,        // group to send to
               const long iStamp);                 // zChat stamp to use, 0 = our connection
  
  CChatSocket * GetChatSocket (const long nID) const; // find a chat socket
  long ChatCallGeneral (LPCTSTR Server, long Port, const bool zChat);

// scripting support

  bool CreateScriptEngine();
  void DisableScripting (void);
  void CreateMonitoringThread();
	static void ThreadFunc(LPVOID pParam);
  void OnScriptFileChanged(const bool bForce = false);
  DISPID GetProcedureDispid (const CString & strName, 
                             const CString & strType,
                             const CString & strLabel,
                             CString & strErrorMessage);
  DISPID GetProcedureDispidHelper (const CString & strName, 
                             const CString & strType,
                             const CString & strLabel,
                             CString & strErrorMessage);
  bool SeeIfHandlerCanExecute (const CString & strName);
  bool FindAllEntryPoints (void);
  void FindGlobalEntryPoints (CString & strErrorMessage);

  void SetUpVariantString (VARIANT & tVariant, const CString & strContents);
  void SetUpVariantShort  (VARIANT & tVariant, const short iContents);
  void SetUpVariantBool   (VARIANT & tVariant, const BOOL iContents);
  void SetUpVariantLong   (VARIANT & tVariant, const long iContents);
  void SetUpVariantDate   (VARIANT & tVariant, const COleDateTime iContents);
  void SetUpVariantDouble (VARIANT & tVariant, const double dContents);
  long CheckObjectName    (CString & strObjectName, const bool bConform = true);

  const bool CheckScriptingAvailable (const char * sWhat,
                                      const DISPID dispid,
                                      const char * sScript);

  bool ExecuteScript (DISPID & dispid,  // dispatch ID, will be set to DISPID_UNKNOWN on an error
                      LPCTSTR szProcedure,  // eg. ON_TRIGGER_XYZ
                      const unsigned short iReason,  // value for m_iCurrentActionSource
                      LPCTSTR szType,   // eg. trigger, alias
                      LPCTSTR szReason, // eg. invoking trigger subroutine XXX
                      DISPPARAMS & params,  // parameters
                      long & nInvocationCount,  // count of invocations
                      COleVariant * result = NULL   // result from function call
                      );

  void ExecuteTriggerScript (CTrigger * trigger_item,
                             const CString strCurrentLine,
                             CPaneLine & StyledLine);

  bool ExecuteAliasScript (CAlias * alias_item,
                             const CString strCurrentLine);
                               
  void ExecuteHotspotScript (DISPID & dispid,  // dispatch ID, will be set to DISPID_UNKNOWN on an error
                            LPCTSTR szProcedure,      // what name was in the hotspot callback
                            long iFlags,              // flags: ctrl, shift, whatever
                            LPCTSTR szHotspotID       // which hotspot
                            );

  void ShowErrorLines (const int iLine);  // show script file around the error point

  void ShowStatusLine (const bool bNow = false); // update the status line

  void ShowOptions (void);
  void ShowAlphaOptions (void);

  CPlugin * GetPlugin (LPCTSTR PluginID);

  // load from document into property page

  void LoadPrefsP1  (CPrefsP1  &page1);
  void LoadPrefsP2  (CPrefsP2  &page2);
  void LoadPrefsP3  (CPrefsP3  &page3);
  void LoadPrefsP4  (CPrefsP4  &page4);
  void LoadPrefsP5  (CPrefsP5  &page5);
  void LoadPrefsP6  (CPrefsP6  &page6);
  void LoadPrefsP7  (CPrefsP7  &page7);
  void LoadPrefsP8  (CPrefsP8  &page8);
  void LoadPrefsP9  (CPrefsP9  &page9);
  void LoadPrefsP10 (CPrefsP10 &page10);
  void LoadPrefsP11 (CPrefsP11 &page11);
  void LoadPrefsP12 (CPrefsP12 &page12);
  void LoadPrefsP13 (CPrefsP13 &page13);
  void LoadPrefsP14 (CPrefsP14 &page14);
  void LoadPrefsP15 (CPrefsP15 &page15);
  void LoadPrefsP16 (CPrefsP16 &page16);
  void LoadPrefsP17 (CPrefsP17 &page17);
  void LoadPrefsP18 (CPrefsP18 &page18);
  void LoadPrefsP19 (CPrefsP19 &page19);
  void LoadPrefsP20 (CPrefsP20 &page20);
  void LoadPrefsP21 (CPrefsP21 &page21);
  void LoadPrefsP22 (CPrefsP22 &page22);
  void LoadPrefsP23 (CPrefsP23 &page23);

  // check if document has changed by comparing to property page

  bool ChangedPrefsP1  (CPrefsP1  &page1);
  bool ChangedPrefsP2  (CPrefsP2  &page2);
  bool ChangedPrefsP3  (CPrefsP3  &page3);
  bool ChangedPrefsP4  (CPrefsP4  &page4);
  bool ChangedPrefsP5  (CPrefsP5  &page5);
  bool ChangedPrefsP6  (CPrefsP6  &page6);
  bool ChangedPrefsP7  (CPrefsP7  &page7);
  bool ChangedPrefsP8  (CPrefsP8  &page8);
  bool ChangedPrefsP9  (CPrefsP9  &page9);
  bool ChangedPrefsP10 (CPrefsP10 &page10);
  bool ChangedPrefsP11 (CPrefsP11 &page11);
  bool ChangedPrefsP12 (CPrefsP12 &page12);
  bool ChangedPrefsP13 (CPrefsP13 &page13);
  bool ChangedPrefsP14 (CPrefsP14 &page14);
  bool ChangedPrefsP15 (CPrefsP15 &page15);
  bool ChangedPrefsP16 (CPrefsP16 &page16);
  bool ChangedPrefsP17 (CPrefsP17 &page17);
  bool ChangedPrefsP18 (CPrefsP18 &page18);
  bool ChangedPrefsP19 (CPrefsP19 &page19);
  bool ChangedPrefsP20 (CPrefsP20 &page20);
  bool ChangedPrefsP21 (CPrefsP21 &page21);
  bool ChangedPrefsP22 (CPrefsP22 &page22);
  bool ChangedPrefsP23 (CPrefsP23 &page23);

  // unload from property page into document

  void SavePrefsP1  (CPrefsP1  &page1);
  void SavePrefsP2  (CPrefsP2  &page2);
  void SavePrefsP3  (CPrefsP3  &page3);
  void SavePrefsP4  (CPrefsP4  &page4);
  void SavePrefsP5  (CPrefsP5  &page5);
  void SavePrefsP6  (CPrefsP6  &page6);
  void SavePrefsP7  (CPrefsP7  &page7);
  void SavePrefsP8  (CPrefsP8  &page8);
  void SavePrefsP9  (CPrefsP9  &page9);
  void SavePrefsP10 (CPrefsP10 &page10);
  void SavePrefsP11 (CPrefsP11 &page11);
  void SavePrefsP12 (CPrefsP12 &page12);
  void SavePrefsP13 (CPrefsP13 &page13);
  void SavePrefsP14 (CPrefsP14 &page14);
  void SavePrefsP15 (CPrefsP15 &page15);
  void SavePrefsP16 (CPrefsP16 &page16);
  void SavePrefsP17 (CPrefsP17 &page17);
  void SavePrefsP18 (CPrefsP18 &page18);
  void SavePrefsP19 (CPrefsP19 &page19);
  void SavePrefsP20 (CPrefsP20 &page20);
  void SavePrefsP21 (CPrefsP21 &page21);
  void SavePrefsP22 (CPrefsP22 &page22);
  void SavePrefsP23 (CPrefsP23 &page23);

  CString AppendMUDName (const CString strName);

  bool GamePreferences (const int iPage); 

  bool LoggingInput (void) { return m_log_input; };

  long DoSetVariable (LPCTSTR VariableName, LPCTSTR Contents)
    { return SetVariable (VariableName, Contents); };

  void Trace (LPCTSTR lpszFormat, ...);

  void Screendraw  (const long iType,
                    const long iLog,
                    const char * sText);

  void PluginListChanged (void);

  CString RecallText (const CString strSearchString,   // what to search for
                      const bool bMatchCase,
                      const bool bRegexp,          // and other params
                      const bool bCommands,
                      const bool bOutput,
                      const bool bNotes,
                      const int  iLines,
                      const CString strRecallLinePreamble);

  bool SwitchToNotepad (void);
  bool AppendToTheNotepad (const CString strTitle,
                           const CString strText,
                           const bool bReplace = false,
                           const int  iNotepadType = eNotepadNormal);

  CTextDocument * FindNotepad (const CString strTitle);

  void DoRecallText (void);
  CString DoEvaluateSpeedwalk(LPCTSTR SpeedWalkString);
  static CString DoReverseSpeedwalk(LPCTSTR SpeedWalkString);

  void AddToMap (CString str);
  void ShowQueuedCommands (void);
  CString CalculateSpeedWalkString (const bool bOmitComments = false);
  void DrawMappingStatusLine (void);
  
  void ClearOutput (void);
  CString FormatTime (const CTime theTime, 
                      LPCTSTR pFormat, 
                      const bool bFixHTML = false);

  long GetOptionIndex (LPCTSTR OptionName);
  long GetOptionItem (const int iItem);
  long SetOptionItem (const int iItem, 
                      long Value, 
                      const bool bDoSpecial,
                      const bool bInclude);
  void SetDefaults (const bool bDoSpecial);

  long GetAlphaOptionIndex (LPCTSTR OptionName);
  CString GetAlphaOptionItem (const int iItem);
  long SetAlphaOptionItem (const int iItem, 
                          LPCTSTR sValue, 
                          const bool bDoSpecial,
                          const bool bInclude);
  void SetAlphaDefaults (const bool bDoSpecial);

  void AllocateConfigurationArrays (void);
  void DeleteConfigurationArrays (void);
  void ResetConfigurationArrays (void);

  // helper routines to get the appropriate map
  CTriggerMap & GetTriggerMap (void)
    {
    if (m_CurrentPlugin)
      return m_CurrentPlugin->m_TriggerMap;
    else
      return m_TriggerMap;
    };

  CTriggerArray & GetTriggerArray (void)
    {
    if (m_CurrentPlugin)
      return m_CurrentPlugin->m_TriggerArray;
    else
      return m_TriggerArray;
    };

  CTriggerRevMap & GetTriggerRevMap (void)
    {
    if (m_CurrentPlugin)
      return m_CurrentPlugin->m_TriggerRevMap;
    else
      return m_TriggerRevMap;
    };

  CAliasMap & GetAliasMap (void)
    {
    if (m_CurrentPlugin)
      return m_CurrentPlugin->m_AliasMap;
    else
      return m_AliasMap;
    };

  CAliasArray & GetAliasArray (void)
    {
    if (m_CurrentPlugin)
      return m_CurrentPlugin->m_AliasArray;
    else
      return m_AliasArray;
    };

  CAliasRevMap & GetAliasRevMap (void)
    {
    if (m_CurrentPlugin)
      return m_CurrentPlugin->m_AliasRevMap;
    else
      return m_AliasRevMap;
    };

  CTimerMap & GetTimerMap (void)
    {
    if (m_CurrentPlugin)
      return m_CurrentPlugin->m_TimerMap;
    else
      return m_TimerMap;
    };

  CTimerRevMap & GetTimerRevMap (void)
    {
    if (m_CurrentPlugin)
      return m_CurrentPlugin->m_TimerRevMap;
    else
      return m_TimerRevMap;
    };

  CVariableMap & GetVariableMap (void)
    {
    if (m_CurrentPlugin)
      return m_CurrentPlugin->m_VariableMap;
    else
      return m_VariableMap;
    };

  CScriptEngine * GetScriptEngine (void)
    {
    if (m_CurrentPlugin)
      return m_CurrentPlugin->m_ScriptEngine;
    else
      return m_ScriptEngine;
    };

  tStringMapOfMaps & GetArrayMap (void)
    {
    if (m_CurrentPlugin)
      return m_CurrentPlugin->m_Arrays;
    else
      return m_Arrays;
    };

  CString GetLanguage (void)
    {
    if (m_CurrentPlugin)
      return m_CurrentPlugin->m_strLanguage;
    else
      return m_strLanguage;
    };

  void DoFixMenus(CCmdUI* pCmdUI);

  long WindowLoadImageMemory(LPCTSTR Name, 
                             LPCTSTR ImageId, 
                             unsigned char * Buffer, 
                             const size_t Length,
                             const bool bAlpha);

  void EditFileWithEditor (CString strName);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMUSHclientDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	protected:
	virtual BOOL SaveModified();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMUSHclientDoc();
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	short m_bLogNotes;
	short m_log_input;
	short m_bLogOutput;

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CMUSHclientDoc)
	afx_msg void OnFileLogsession();
	afx_msg void OnUpdateFileLogsession(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatuslineLines(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatuslineMushname(CCmdUI* pCmdUI);
	afx_msg void OnGameWraplines();
	afx_msg void OnUpdateGameWraplines(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatuslineTime(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatuslineLog(CCmdUI* pCmdUI);
	afx_msg void OnGamePreferences();
	afx_msg void OnGamePastefile();
	afx_msg void OnConnectionConnect();
	afx_msg void OnUpdateConnectionConnect(CCmdUI* pCmdUI);
	afx_msg void OnConnectionDisconnect();
	afx_msg void OnUpdateConnectionDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnEditPastetomush();
	afx_msg void OnUpdateSessionOpen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPastetomush(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGamePreferences(CCmdUI* pCmdUI);
	afx_msg void OnGameImmediate();
	afx_msg void OnUpdateGameImmediate(CCmdUI* pCmdUI);
	afx_msg void OnGameAutosay();
	afx_msg void OnUpdateGameAutosay(CCmdUI* pCmdUI);
	afx_msg void OnConnectDisconnect();
	afx_msg void OnUpdateConnectDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnGameReloadScriptFile();
	afx_msg void OnUpdateGameReloadScriptFile(CCmdUI* pCmdUI);
	afx_msg void OnDisplayClearOutput();
	afx_msg void OnGameConfigureMudaddress();
	afx_msg void OnGameConfigureNameAndPassword();
	afx_msg void OnGameConfigureLogging();
	afx_msg void OnGameConfigureInfo();
	afx_msg void OnGameConfigureNotes();
	afx_msg void OnGameConfigureColours();
	afx_msg void OnGameConfigurePrinting();
	afx_msg void OnGameConfigureOutput();
	afx_msg void OnGameConfigureHighlighting();
	afx_msg void OnGameConfigureCommands();
	afx_msg void OnGameConfigureKeypad();
	afx_msg void OnGameConfigureMacros();
	afx_msg void OnGameConfigureAutosay();
	afx_msg void OnGameConfigurePastetoworld();
	afx_msg void OnGameConfigureSendfile();
	afx_msg void OnGameConfigureScripting();
	afx_msg void OnGameConfigureVariables();
	afx_msg void OnGameConfigureTriggers();
	afx_msg void OnGameConfigureAliases();
	afx_msg void OnGameConfigureTimers();
	afx_msg void OnGameResetalltimers();
	afx_msg void OnUpdateGameResetalltimers(CCmdUI* pCmdUI);
	afx_msg void OnGameEditscriptfile();
	afx_msg void OnUpdateGameEditscriptfile(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGameConfigureMudaddress(CCmdUI* pCmdUI);
	afx_msg void OnDisplayRecalltext();
	afx_msg void OnGameConfigureCustomColours();
	afx_msg void OnFileReloaddefaults();
	afx_msg void OnUpdateFileReloaddefaults(CCmdUI* pCmdUI);
	afx_msg void OnGameTrace();
	afx_msg void OnUpdateGameTrace(CCmdUI* pCmdUI);
	afx_msg void OnEditFliptonotepad();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveAs();
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnGameMapper();
	afx_msg void OnUpdateGameMapper(CCmdUI* pCmdUI);
	afx_msg void OnInputDiscardqueuedcommands();
	afx_msg void OnUpdateInputDiscardqueuedcommands(CCmdUI* pCmdUI);
	afx_msg void OnGameDomapperspecial();
	afx_msg void OnUpdateGameDomapperspecial(CCmdUI* pCmdUI);
	afx_msg void OnFilePreferences();
	afx_msg void OnDebugWorldInput();
	afx_msg void OnDisplayNocommandecho();
	afx_msg void OnUpdateDisplayNocommandecho(CCmdUI* pCmdUI);
	afx_msg void OnEditDebugincomingpackets();
	afx_msg void OnUpdateEditDebugincomingpackets(CCmdUI* pCmdUI);
	afx_msg void OnGameConfigureMxppueblo();
	afx_msg void OnFileImport();
	afx_msg void OnFilePlugins();
	afx_msg void OnFilePluginwizard();
	afx_msg void OnGameDomappercomment();
	afx_msg void OnUpdateGameDomappercomment(CCmdUI* pCmdUI);
	afx_msg void OnGameChatsessions();
	afx_msg void OnGameConfigureChat();
	afx_msg void OnGameResetconnectedtime();
	afx_msg void OnDisplayStopsoundplaying();
	afx_msg void OnInputKeyname();
	afx_msg void OnEditConvertclipboardforumcodes();
	//}}AFX_MSG
	afx_msg void OnFixMenus(CCmdUI* pCmdUI);
  DECLARE_MESSAGE_MAP()

// if you get a compile error:
//    C2371: 'm_display_my_input' : redefinition; different basic types
// delete the lines below:   short m_display_my_input; 
//	                         short m_log_input;
//	                         short m_bLogOutput;

// ======================================================================
// When adding script functions, see the comments at the end of methods.cpp
//   - these relate to updating the Lua glue routines, and the list of available functions.
// ======================================================================

public:

// This will probably occur after adding new methods or properties.

    // Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CMUSHclientDoc)
	afx_msg void OnLogNotesChanged();
	afx_msg void OnLogInputChanged();
	afx_msg void OnLogOutputChanged();
	BOOL m_bMapping;
	afx_msg void OnMappingChanged();
	BOOL m_bRemoveMapReverses;
	afx_msg void OnRemoveMapReversesChanged();
	afx_msg short GetNoteColour();
	afx_msg void SetNoteColour(short nNewValue);
	afx_msg BOOL GetTrace();
	afx_msg void SetTrace(BOOL bNewValue);
	afx_msg short GetSpeedWalkDelay();
	afx_msg void SetSpeedWalkDelay(short nNewValue);
	afx_msg long GetNoteColourFore();
	afx_msg void SetNoteColourFore(long nNewValue);
	afx_msg long GetNoteColourBack();
	afx_msg void SetNoteColourBack(long nNewValue);
	afx_msg BOOL GetEchoInput();
	afx_msg void SetEchoInput(BOOL bNewValue);
	afx_msg long GetLineCount();
	afx_msg BOOL IsConnected();
	afx_msg BSTR WorldName();
	afx_msg void Note(LPCTSTR Message);
	afx_msg long Send(LPCTSTR Message);
	afx_msg long Sound(LPCTSTR SoundFileName);
	afx_msg long DeleteTrigger(LPCTSTR TriggerName);
	afx_msg long AddTrigger(LPCTSTR TriggerName, LPCTSTR MatchText, LPCTSTR ResponseText, long Flags, short Colour, short Wildcard, LPCTSTR SoundFileName, LPCTSTR ScriptName);
	afx_msg long EnableTrigger(LPCTSTR TriggerName, BOOL Enabled);
	afx_msg long GetTrigger(LPCTSTR TriggerName, VARIANT FAR* MatchText, VARIANT FAR* ResponseText, VARIANT FAR* Flags, VARIANT FAR* Colour, VARIANT FAR* Wildcard, VARIANT FAR* SoundFileName, VARIANT FAR* ScriptName);
	afx_msg long IsTrigger(LPCTSTR TriggerName);
	afx_msg VARIANT GetTriggerList();
	afx_msg VARIANT GetVariable(LPCTSTR VariableName);
	afx_msg long SetVariable(LPCTSTR VariableName, LPCTSTR Contents);
	afx_msg VARIANT GetVariableList();
	afx_msg BOOL Save(LPCTSTR Name);
	afx_msg long Connect();
	afx_msg long Disconnect();
	afx_msg long DeleteAlias(LPCTSTR AliasName);
	afx_msg long EnableAlias(LPCTSTR AliasName, BOOL Enabled);
	afx_msg long GetAlias(LPCTSTR AliasName, VARIANT FAR* MatchText, VARIANT FAR* ResponseText, VARIANT FAR* Parameter, VARIANT FAR* Flags, VARIANT FAR* ScriptName);
	afx_msg VARIANT GetAliasList();
	afx_msg long IsAlias(LPCTSTR AliasName);
	afx_msg long CloseLog();
	afx_msg long OpenLog(LPCTSTR LogFileName, BOOL Append);
	afx_msg long WriteLog(LPCTSTR Message);
	afx_msg BOOL IsLogOpen();
	afx_msg long EnableTimer(LPCTSTR TimerName, BOOL Enabled);
	afx_msg long AddAlias(LPCTSTR AliasName, LPCTSTR MatchText, LPCTSTR ResponseText, long Flags, LPCTSTR ScriptName);
	afx_msg long DeleteVariable(LPCTSTR VariableName);
	afx_msg void ResetTimers();
	afx_msg void SetStatus(LPCTSTR Message);
	afx_msg long SetCommand(LPCTSTR Message);
	afx_msg BSTR GetNotes();
	afx_msg void SetNotes(LPCTSTR Message);
	afx_msg void Redraw();
	afx_msg long ResetTimer(LPCTSTR TimerName);
	afx_msg void SetOutputFont(LPCTSTR FontName, short PointSize);
	afx_msg void SetInputFont(LPCTSTR FontName, short PointSize, short Weight, BOOL Italic);
	afx_msg BOOL SendToNotepad(LPCTSTR Title, LPCTSTR Contents);
	afx_msg BOOL AppendToNotepad(LPCTSTR Title, LPCTSTR Contents);
	afx_msg BOOL ActivateNotepad(LPCTSTR Title);
	afx_msg void Activate();
	afx_msg LPDISPATCH GetWorld(LPCTSTR WorldName);
	afx_msg VARIANT GetWorldList();
	afx_msg BSTR FixupHTML(LPCTSTR StringToConvert);
	afx_msg BSTR Replace(LPCTSTR Source, LPCTSTR SearchFor, LPCTSTR ReplaceWith, BOOL Multiple);
	afx_msg BSTR FixupEscapeSequences(LPCTSTR Source);
	afx_msg BSTR Trim(LPCTSTR Source);
	afx_msg BOOL ReplaceNotepad(LPCTSTR Title, LPCTSTR Contents);
	afx_msg VARIANT GetAliasInfo(LPCTSTR AliasName, short InfoType);
	afx_msg VARIANT GetTriggerInfo(LPCTSTR TriggerName, short InfoType);
	afx_msg BSTR EvaluateSpeedwalk(LPCTSTR SpeedWalkString);
	afx_msg BSTR ReverseSpeedwalk(LPCTSTR SpeedWalkString);
	afx_msg long DeleteTimer(LPCTSTR TimerName);
	afx_msg long IsTimer(LPCTSTR TimerName);
	afx_msg VARIANT GetTimerList();
	afx_msg long GetTimer(LPCTSTR TimerName, VARIANT FAR* Hour, VARIANT FAR* Minute, VARIANT FAR* Second, VARIANT FAR* ResponseText, VARIANT FAR* Flags, VARIANT FAR* ScriptName);
	afx_msg VARIANT GetTimerInfo(LPCTSTR TimerName, short InfoType);
	afx_msg long GetUniqueNumber();
	afx_msg long Queue(LPCTSTR Message, BOOL Echo);
	afx_msg long DiscardQueue();
	afx_msg VARIANT GenerateName();
	afx_msg long ReadNamesFile(LPCTSTR FileName);
	afx_msg long AddToMapper(LPCTSTR Direction, LPCTSTR Reverse);
	afx_msg long GetMappingCount();
	afx_msg VARIANT GetMappingItem(long Item);
	afx_msg VARIANT GetMappingString();
	afx_msg long DeleteLastMapItem();
	afx_msg long DeleteAllMapItems();
	afx_msg long GetSentBytes();
	afx_msg long GetReceivedBytes();
	afx_msg long GetConnectDuration();
	afx_msg double GetScriptTime();
	afx_msg BSTR WorldAddress();
	afx_msg long WorldPort();
	afx_msg void DeleteCommandHistory();
	afx_msg void DeleteOutput();
	afx_msg void Tell(LPCTSTR Message);
	afx_msg void NoteColourRGB(long Foreground, long Background);
	afx_msg void NoteColourName(LPCTSTR Foreground, LPCTSTR Background);
	afx_msg BSTR Version();
	afx_msg void Reset();
	afx_msg long GetOption(LPCTSTR OptionName);
	afx_msg VARIANT GetOptionList();
	afx_msg long SetOption(LPCTSTR OptionName, long Value);
	afx_msg VARIANT Debug(LPCTSTR Command);
	afx_msg void Pause(BOOL Flag);
	afx_msg LPDISPATCH Open(LPCTSTR FileName);
	afx_msg VARIANT GetLineInfo(long LineNumber, short InfoType);
	afx_msg long GetLinesInBufferCount();
	afx_msg VARIANT GetStyleInfo(long LineNumber, long StyleNumber, short InfoType);
	afx_msg long GetSelectionStartLine();
	afx_msg long GetSelectionEndLine();
	afx_msg long GetSelectionStartColumn();
	afx_msg long GetSelectionEndColumn();
	afx_msg VARIANT GetCommandList(long Count);
	afx_msg BSTR PushCommand();
	afx_msg void SelectCommand();
	afx_msg BSTR GetCommand();
	afx_msg long AddTriggerEx(LPCTSTR TriggerName, LPCTSTR MatchText, LPCTSTR ResponseText, long Flags, short Colour, short Wildcard, LPCTSTR SoundFileName, LPCTSTR ScriptName, short SendTo, short Sequence);
	afx_msg VARIANT GetQueue();
	afx_msg long DeleteTemporaryTriggers();
	afx_msg long DeleteTemporaryAliases();
	afx_msg long DeleteTemporaryTimers();
	afx_msg VARIANT GetInfo(long InfoType);
	afx_msg VARIANT GetAlphaOptionList();
	afx_msg long SetAlphaOption(LPCTSTR OptionName, LPCTSTR Value);
	afx_msg VARIANT GetAlphaOption(LPCTSTR OptionName);
	afx_msg long ColourNameToRGB(LPCTSTR Name);
	afx_msg BSTR RGBColourToName(long Colour);
	afx_msg VARIANT Base64Decode(LPCTSTR Text);
	afx_msg VARIANT Base64Encode(LPCTSTR Text, BOOL MultiLine);
	afx_msg VARIANT GetLoadedValue(LPCTSTR OptionName);
	afx_msg VARIANT GetDefaultValue(LPCTSTR OptionName);
	afx_msg VARIANT GetCurrentValue(LPCTSTR OptionName);
	afx_msg void ColourNote(LPCTSTR TextColour, LPCTSTR BackgroundColour, LPCTSTR Text);
	afx_msg void ColourTell(LPCTSTR TextColour, LPCTSTR BackgroundColour, LPCTSTR Text);
	afx_msg BSTR CreateGUID();
	afx_msg BSTR Hash(LPCTSTR Text);
	afx_msg BSTR GetUniqueID();
	afx_msg VARIANT GetPluginVariable(LPCTSTR PluginID, LPCTSTR VariableName);
	afx_msg BSTR GetPluginID();
	afx_msg VARIANT GetPluginList();
	afx_msg VARIANT GetPluginInfo(LPCTSTR PluginID, short InfoType);
	afx_msg long LoadPlugin(LPCTSTR FileName);
	afx_msg VARIANT GetPluginVariableList(LPCTSTR PluginID);
	afx_msg VARIANT GetPluginTriggerList(LPCTSTR PluginID);
	afx_msg VARIANT GetPluginAliasList(LPCTSTR PluginID);
	afx_msg VARIANT GetPluginTimerList(LPCTSTR PluginID);
	afx_msg VARIANT GetPluginTriggerInfo(LPCTSTR PluginID, LPCTSTR TriggerName, short InfoType);
	afx_msg VARIANT GetPluginAliasInfo(LPCTSTR PluginID, LPCTSTR AliasName, short InfoType);
	afx_msg VARIANT GetPluginTimerInfo(LPCTSTR PluginID, LPCTSTR TimerName, short InfoType);
	afx_msg BOOL IsPluginInstalled(LPCTSTR PluginID);
	afx_msg long SaveState();
	afx_msg long CallPlugin(LPCTSTR PluginID, LPCTSTR Routine, LPCTSTR Argument);
	afx_msg long PluginSupports(LPCTSTR PluginID, LPCTSTR Routine);
	afx_msg long EnablePlugin(LPCTSTR PluginID, BOOL Enabled);
	afx_msg BSTR GetPluginName();
	afx_msg long LogSend(LPCTSTR Message);
	afx_msg long AddMapperComment(LPCTSTR Comment);
	afx_msg long SendPush(LPCTSTR Message);
	afx_msg long EnableGroup(LPCTSTR GroupName, BOOL Enabled);
	afx_msg long EnableTriggerGroup(LPCTSTR GroupName, BOOL Enabled);
	afx_msg long EnableAliasGroup(LPCTSTR GroupName, BOOL Enabled);
	afx_msg long EnableTimerGroup(LPCTSTR GroupName, BOOL Enabled);
	afx_msg void ShowInfoBar(BOOL Visible);
	afx_msg void Info(LPCTSTR Message);
	afx_msg void InfoFont(LPCTSTR FontName, short Size, short Style);
	afx_msg long DeleteGroup(LPCTSTR GroupName);
	afx_msg long DeleteTriggerGroup(LPCTSTR GroupName);
	afx_msg long DeleteAliasGroup(LPCTSTR GroupName);
	afx_msg long DeleteTimerGroup(LPCTSTR GroupName);
	afx_msg void InfoClear();
	afx_msg void InfoColour(LPCTSTR Name);
	afx_msg void InfoBackground(LPCTSTR Name);
	afx_msg VARIANT GetTriggerOption(LPCTSTR TriggerName, LPCTSTR OptionName);
	afx_msg long SetTriggerOption(LPCTSTR TriggerName, LPCTSTR OptionName, LPCTSTR Value);
	afx_msg VARIANT GetAliasOption(LPCTSTR AliasName, LPCTSTR OptionName);
	afx_msg VARIANT GetTimerOption(LPCTSTR TimerName, LPCTSTR OptionName);
	afx_msg long SetAliasOption(LPCTSTR AliasName, LPCTSTR OptionName, LPCTSTR Value);
	afx_msg long SetTimerOption(LPCTSTR TimerName, LPCTSTR OptionName, LPCTSTR Value);
	afx_msg long SaveNotepad(LPCTSTR Title, LPCTSTR FileName, BOOL ReplaceExisting);
	afx_msg long CloseNotepad(LPCTSTR Title, BOOL QuerySave);
	afx_msg BSTR GetEntity(LPCTSTR Name);
	afx_msg long Execute(LPCTSTR Command);
	afx_msg long ChatCall(LPCTSTR Server, long Port);
	afx_msg void AnsiNote(LPCTSTR Text);
	afx_msg long ChatNameChange(LPCTSTR NewName);
	afx_msg long ChatAcceptCalls(short Port);
	afx_msg void ChatStopAcceptingCalls();
	afx_msg VARIANT GetChatList();
	afx_msg VARIANT GetChatInfo(long ChatID, short InfoType);
	afx_msg long ChatEverybody(LPCTSTR Message, BOOL Emote);
	afx_msg long ChatPersonal(LPCTSTR Who, LPCTSTR Message, BOOL Emote);
	afx_msg long ChatID(long ID, LPCTSTR Message, BOOL Emote);
	afx_msg long ChatGroup(LPCTSTR Group, LPCTSTR Message, BOOL Emote);
	afx_msg long ChatPing(long ID);
	afx_msg long ChatPeekConnections(long ID);
	afx_msg long ChatRequestConnections(long ID);
	afx_msg long ChatDisconnect(long ID);
	afx_msg long ChatMessage(long ID, short Message, LPCTSTR Text);
	afx_msg BSTR ANSI(short Code);
	afx_msg BSTR StripANSI(LPCTSTR Message);
	afx_msg VARIANT GetChatOption(long ID, LPCTSTR OptionName);
	afx_msg long SetChatOption(long ID, LPCTSTR OptionName, LPCTSTR Value);
	afx_msg void ChatNote(short NoteType, LPCTSTR Message);
	afx_msg long ChatGetID(LPCTSTR Who);
	afx_msg long ChatDisconnectAll();
	afx_msg long ChatPasteText(long ID);
	afx_msg long ChatSendFile(long ID, LPCTSTR FileName);
	afx_msg long ChatStopFileTransfer(long ID);
	afx_msg long ChatPasteEverybody();
	afx_msg VARIANT GetWorldIdList();
	afx_msg LPDISPATCH GetWorldById(LPCTSTR WorldID);
	afx_msg BSTR GetHostName(LPCTSTR IPaddress);
	afx_msg VARIANT GetHostAddress(LPCTSTR HostName);
	afx_msg BSTR GetClipboard();
	afx_msg long ImportXML(LPCTSTR XML);
	afx_msg long DoCommand(LPCTSTR Command);
	afx_msg VARIANT GetInternalCommandsList();
	afx_msg long ChatCallzChat(LPCTSTR Server, long Port);
	afx_msg long GetNotepadLength(LPCTSTR Title);
	afx_msg BSTR GetNotepadText(LPCTSTR Title);
	afx_msg long SendImmediate(LPCTSTR Message);
	afx_msg long AdjustColour(long Colour, short Method);
	afx_msg long SendNoEcho(LPCTSTR Message);
	afx_msg BSTR ExportXML(short Type, LPCTSTR Name);
	afx_msg void Hyperlink(LPCTSTR Action, LPCTSTR Text, LPCTSTR Hint, LPCTSTR TextColour, LPCTSTR BackColour, BOOL URL);
	afx_msg void NoteStyle(short Style);
	afx_msg short GetNoteStyle();
	afx_msg long GetFrame();
	afx_msg long ArrayCreate(LPCTSTR Name);
	afx_msg long ArrayDelete(LPCTSTR Name);
	afx_msg VARIANT ArrayListAll();
	afx_msg long ArrayImport(LPCTSTR Name, LPCTSTR Values, LPCTSTR Delimiter);
	afx_msg VARIANT ArrayGet(LPCTSTR Name, LPCTSTR Key);
	afx_msg long ArraySet(LPCTSTR Name, LPCTSTR Key, LPCTSTR Value);
	afx_msg VARIANT ArrayExport(LPCTSTR Name, LPCTSTR Delimiter);
	afx_msg VARIANT ArrayListKeys(LPCTSTR Name);
	afx_msg long ArrayClear(LPCTSTR Name);
	afx_msg long ArrayDeleteKey(LPCTSTR Name, LPCTSTR Key);
	afx_msg BOOL ArrayExists(LPCTSTR Name);
	afx_msg BOOL ArrayKeyExists(LPCTSTR Name, LPCTSTR Key);
	afx_msg VARIANT ArrayExportKeys(LPCTSTR Name, LPCTSTR Delimiter);
	afx_msg long ArrayCount();
	afx_msg long ArraySize(LPCTSTR Name);
	afx_msg VARIANT ArrayGetFirstKey(LPCTSTR Name);
	afx_msg VARIANT ArrayGetLastKey(LPCTSTR Name);
	afx_msg VARIANT ArrayListValues(LPCTSTR Name);
	afx_msg void ResetStatusTime();
	afx_msg void EnableMapping(BOOL Enabled);
	afx_msg void ActivateClient();
	afx_msg BSTR RemoveBacktracks(LPCTSTR Path);
	afx_msg VARIANT GetTriggerWildcard(LPCTSTR TriggerName, LPCTSTR WildcardName);
	afx_msg BSTR GetRecentLines(long Count);
	afx_msg VARIANT GetAliasWildcard(LPCTSTR AliasName, LPCTSTR WildcardName);
	afx_msg long ReloadPlugin(LPCTSTR PluginID);
	afx_msg BSTR TranslateGerman(LPCTSTR Text);
	afx_msg BSTR GetWorldID();
	afx_msg long Accelerator(LPCTSTR Key, LPCTSTR Send);
	afx_msg VARIANT AcceleratorList();
	afx_msg void MapColour(long Original, long Replacement);
	afx_msg VARIANT SpellCheck(LPCTSTR Text);
	afx_msg long GetMapColour(long Which);
	afx_msg VARIANT MapColourList();
	afx_msg BSTR MakeRegularExpression(LPCTSTR Text);
	afx_msg long SpellCheckCommand(long StartCol, long EndCol);
	afx_msg void MoveMainWindow(long Left, long Top, long Width, long Height);
	afx_msg void MoveWorldWindow(long Left, long Top, long Width, long Height);
	afx_msg long MoveNotepadWindow(LPCTSTR Title, long Left, long Top, long Width, long Height);
	afx_msg long GetSysColor(long Index);
	afx_msg long GetSystemMetrics(long Index);
	afx_msg BSTR GetMainWindowPosition();
	afx_msg BSTR GetWorldWindowPosition();
	afx_msg BSTR GetNotepadWindowPosition(LPCTSTR Title);
	afx_msg long UdpSend(LPCTSTR IP, short Port, LPCTSTR Text);
	afx_msg long UdpListen(LPCTSTR IP, short Port, LPCTSTR Script);
	afx_msg long NotepadColour(LPCTSTR Title, LPCTSTR TextColour, LPCTSTR BackgroundColour);
	afx_msg long NotepadFont(LPCTSTR Title, LPCTSTR FontName, short Size, short Style, short Charset);
	afx_msg void MtSrand(long Seed);
	afx_msg double MtRand();
	afx_msg void SetChanged(BOOL ChangedFlag);
	afx_msg VARIANT UdpPortList();
	afx_msg long AddTimer(LPCTSTR TimerName, short Hour, short Minute, double Second, LPCTSTR ResponseText, long Flags, LPCTSTR ScriptName);
	afx_msg long DoAfter(double Seconds, LPCTSTR SendText);
	afx_msg long DoAfterNote(double Seconds, LPCTSTR NoteText);
	afx_msg long DoAfterSpeedWalk(double Seconds, LPCTSTR SendText);
	afx_msg long DoAfterSpecial(double Seconds, LPCTSTR SendText, short SendTo);
	afx_msg void NoteHr();
	afx_msg void MoveWorldWindowX(long Left, long Top, long Width, long Height, short Which);
	afx_msg BSTR GetWorldWindowPositionX(short Which);
	afx_msg void SetEntity(LPCTSTR Name, LPCTSTR Contents);
	afx_msg BSTR GetXMLEntity(LPCTSTR Entity);
	afx_msg long GetUdpPort(long First, long Last);
	afx_msg void SetClipboard(LPCTSTR Text);
	afx_msg long BroadcastPlugin(long Message, LPCTSTR Text);
	afx_msg long PickColour(long Suggested);
	afx_msg void TraceOut(LPCTSTR Message);
	afx_msg BSTR ErrorDesc(long Code);
	afx_msg BSTR PasteCommand(LPCTSTR Text);
	afx_msg void Help(LPCTSTR Name);
	afx_msg void Simulate(LPCTSTR Text);
	afx_msg void ResetIP();
	afx_msg long AddSpellCheckWord(LPCTSTR OriginalWord, LPCTSTR ActionCode, LPCTSTR ReplacementWord);
	afx_msg long OpenBrowser(LPCTSTR URL);
	afx_msg void DeleteLines(long Count);
	afx_msg BOOL NotepadSaveMethod(LPCTSTR Title, short Method);
	afx_msg long ChangeDir(LPCTSTR Path);
	afx_msg long SendPkt(LPCTSTR Packet);
	afx_msg BSTR Metaphone(LPCTSTR Word, short Length);
	afx_msg long EditDistance(LPCTSTR Source, LPCTSTR Target);
	afx_msg long FlushLog();
	afx_msg BOOL Transparency(long Key, short Amount);
	afx_msg VARIANT SpellCheckDlg(LPCTSTR Text);
	afx_msg void SetWorldWindowStatus(short Parameter);
	afx_msg BSTR GetCustomColourName(short WhichColour);
	afx_msg long SetCustomColourName(short WhichColour, LPCTSTR Name);
	afx_msg VARIANT GetNotepadList(BOOL All);
	afx_msg long TranslateDebug(LPCTSTR Message);
	afx_msg long SendSpecial(LPCTSTR Message, BOOL Echo, BOOL Queue, BOOL Log, BOOL History);
	afx_msg VARIANT GetGlobalOption(LPCTSTR Name);
	afx_msg VARIANT GetGlobalOptionList();
	afx_msg VARIANT GetPluginTriggerOption(LPCTSTR PluginID, LPCTSTR TriggerName, LPCTSTR OptionName);
	afx_msg VARIANT GetPluginAliasOption(LPCTSTR PluginID, LPCTSTR AliasName, LPCTSTR OptionName);
	afx_msg VARIANT GetPluginTimerOption(LPCTSTR PluginID, LPCTSTR TimerName, LPCTSTR OptionName);
	afx_msg long AcceleratorTo(LPCTSTR Key, LPCTSTR Send, short SendTo);
	afx_msg long StopSound(short Buffer);
	afx_msg long GetSoundStatus(short Buffer);
	afx_msg long PlaySound(short Buffer, LPCTSTR FileName, BOOL Loop, double Volume, double Pan);
	afx_msg long SetCommandWindowHeight(short Height);
	afx_msg long SetToolBarPosition(short Which, BOOL Float, short Side, long Top, long Left);
	afx_msg long ShiftTabCompleteItem(LPCTSTR Item);
	afx_msg long NotepadReadOnly(LPCTSTR Title, BOOL ReadOnly);
	afx_msg long AddFont(LPCTSTR PathName);
	afx_msg long SetBackgroundImage(LPCTSTR FileName, short Mode);
	afx_msg long SetForegroundImage(LPCTSTR FileName, short Mode);
	afx_msg long SetBackgroundColour(long Colour);
	afx_msg long WindowRectOp(LPCTSTR Name, short Action, long Left, long Top, long Right, long Bottom, long Colour1, long Colour2);
	afx_msg long WindowShow(LPCTSTR Name, BOOL Show);
	afx_msg long WindowTextWidth(LPCTSTR Name, LPCTSTR FontId, LPCTSTR Text, BOOL Unicode);
	afx_msg VARIANT WindowFontInfo(LPCTSTR Name, LPCTSTR FontId, long InfoType);
	afx_msg long WindowLine(LPCTSTR Name, long x1, long y1, long x2, long y2, long PenColour, long PenStyle, long PenWidth);
	afx_msg long WindowArc(LPCTSTR Name, long Left, long Top, long Right, long Bottom, long x1, long y1, long x2, long y2, long PenColour, long PenStyle, long PenWidth);
	afx_msg VARIANT WindowList();
	afx_msg VARIANT WindowFontList(LPCTSTR Name);
	afx_msg VARIANT WindowInfo(LPCTSTR Name, long InfoType);
	afx_msg long WindowLoadImage(LPCTSTR Name, LPCTSTR ImageId, LPCTSTR FileName);
	afx_msg VARIANT WindowImageList(LPCTSTR Name);
	afx_msg long WindowText(LPCTSTR Name, LPCTSTR FontId, LPCTSTR Text, long Left, long Top, long Right, long Bottom, long Colour, BOOL Unicode);
	afx_msg long WindowDrawImage(LPCTSTR Name, LPCTSTR ImageId, long Left, long Top, long Right, long Bottom, short Mode, long SrcLeft, long SrcTop, long SrcRight, long SrcBottom);
	afx_msg VARIANT WindowImageInfo(LPCTSTR Name, LPCTSTR ImageId, long InfoType);
	afx_msg long GetDeviceCaps(long Index);
	afx_msg long WindowBezier(LPCTSTR Name, LPCTSTR Points, long PenColour, long PenStyle, long PenWidth);
	afx_msg long WindowCreate(LPCTSTR Name, long Left, long Top, long Width, long Height, short Position, long Flags, long BackgroundColour);
	afx_msg long WindowPosition(LPCTSTR Name, long Left, long Top, short Position, long Flags);
	afx_msg long WindowDeleteHotspot(LPCTSTR Name, LPCTSTR HotspotId);
	afx_msg VARIANT WindowHotspotList(LPCTSTR Name);
	afx_msg long WindowDeleteAllHotspots(LPCTSTR Name);
	afx_msg VARIANT WindowHotspotInfo(LPCTSTR Name, LPCTSTR HotspotId, long InfoType);
	afx_msg long WindowPolygon(LPCTSTR Name, LPCTSTR Points, long PenColour, long PenStyle, long PenWidth, long BrushColour, long BrushStyle, BOOL Close, BOOL Winding);
	afx_msg long WindowAddHotspot(LPCTSTR Name, LPCTSTR HotspotId, long Left, long Top, long Right, long Bottom, LPCTSTR MouseOver, LPCTSTR CancelMouseOver, LPCTSTR MouseDown, LPCTSTR CancelMouseDown, LPCTSTR MouseUp, LPCTSTR TooltipText, long Cursor, long Flags);
	afx_msg long WindowCircleOp(LPCTSTR Name, short Action, long Left, long Top, long Right, long Bottom, long PenColour, long PenStyle, long PenWidth, long BrushColour, long BrushStyle, long Extra1, long Extra2, long Extra3, long Extra4);
	afx_msg long WindowCreateImage(LPCTSTR Name, LPCTSTR ImageId, long Row1, long Row2, long Row3, long Row4, long Row5, long Row6, long Row7, long Row8);
	afx_msg long WindowImageOp(LPCTSTR Name, short Action, long Left, long Top, long Right, long Bottom, long PenColour, long PenStyle, long PenWidth, long BrushColour, LPCTSTR ImageId, long EllipseWidth, long EllipseHeight);
	afx_msg long WindowFont(LPCTSTR Name, LPCTSTR FontId, LPCTSTR FontName, double Size, BOOL Bold, BOOL Italic, BOOL Underline, BOOL Strikeout, short Charset, short PitchAndFamily);
	afx_msg long WindowBlendImage(LPCTSTR Name, LPCTSTR ImageId, long Left, long Top, long Right, long Bottom, short Mode, double Opacity, long SrcLeft, long SrcTop, long SrcRight, long SrcBottom);
	afx_msg long WindowImageFromWindow(LPCTSTR Name, LPCTSTR ImageId, LPCTSTR SourceWindow);
	afx_msg void Repaint();
	afx_msg long TextRectangle(long Left, long Top, long Right, long Bottom, long BorderOffset, long BorderColour, long BorderWidth, long OutsideFillColour, long OutsideFillStyle);
	afx_msg long WindowGradient(LPCTSTR Name, long Left, long Top, long Right, long Bottom, long StartColour, long EndColour, short Mode);
	afx_msg long WindowFilter(LPCTSTR Name, long Left, long Top, long Right, long Bottom, short Operation, double Options);
	afx_msg long WindowSetPixel(LPCTSTR Name, long x, long y, long Colour);
	afx_msg long WindowGetPixel(LPCTSTR Name, long x, long y);
	afx_msg long WindowWrite(LPCTSTR Name, LPCTSTR FileName);
	afx_msg long FilterPixel(long Pixel, short Operation, double Options);
	afx_msg long BlendPixel(long Blend, long Base, short Mode, double Opacity);
	afx_msg long WindowMergeImageAlpha(LPCTSTR Name, LPCTSTR ImageId, LPCTSTR MaskId, long Left, long Top, long Right, long Bottom, short Mode, double Opacity, long SrcLeft, long SrcTop, long SrcRight, long SrcBottom);
	afx_msg long WindowDelete(LPCTSTR Name);
	afx_msg BSTR WindowMenu(LPCTSTR Name, long Left, long Top, LPCTSTR Items);
	afx_msg long SetCommandSelection(long First, long Last);
	afx_msg long WindowDragHandler(LPCTSTR Name, LPCTSTR HotspotId, LPCTSTR MoveCallback, LPCTSTR ReleaseCallback, long Flags);
	afx_msg long SetCursor(long Cursor);
	afx_msg long DatabaseClose(LPCTSTR Name);
	afx_msg long DatabasePrepare(LPCTSTR Name, LPCTSTR Sql);
	afx_msg long DatabaseFinalize(LPCTSTR Name);
	afx_msg long DatabaseColumns(LPCTSTR Name);
	afx_msg long DatabaseStep(LPCTSTR Name);
	afx_msg BSTR DatabaseError(LPCTSTR Name);
	afx_msg BSTR DatabaseColumnName(LPCTSTR Name, long Column);
	afx_msg long DatabaseColumnType(LPCTSTR Name, long Column);
	afx_msg BSTR DatabaseColumnText(LPCTSTR Name, long Column);
	afx_msg VARIANT DatabaseColumnValue(LPCTSTR Name, long Column);
	afx_msg long DatabaseOpen(LPCTSTR Name, LPCTSTR Filename, long Flags);
	afx_msg long DatabaseTotalChanges(LPCTSTR Name);
	afx_msg long DatabaseChanges(LPCTSTR Name);
	afx_msg BSTR DatabaseLastInsertRowid(LPCTSTR Name);
	afx_msg VARIANT DatabaseList();
	afx_msg VARIANT DatabaseInfo(LPCTSTR Name, long InfoType);
	afx_msg long DatabaseExec(LPCTSTR Name, LPCTSTR Sql);
	afx_msg VARIANT DatabaseColumnNames(LPCTSTR Name);
	afx_msg VARIANT DatabaseColumnValues(LPCTSTR Name);
	afx_msg long DatabaseReset(LPCTSTR Name);
	afx_msg void FlashIcon();
	afx_msg long WindowHotspotTooltip(LPCTSTR Name, LPCTSTR HotspotId, LPCTSTR TooltipText);
	afx_msg long WindowDrawImageAlpha(LPCTSTR Name, LPCTSTR ImageId, long Left, long Top, long Right, long Bottom, double Opacity, long SrcLeft, long SrcTop);
	afx_msg long GetNormalColour(short WhichColour);
	afx_msg void SetNormalColour(short WhichColour, long nNewValue);
	afx_msg long GetBoldColour(short WhichColour);
	afx_msg void SetBoldColour(short WhichColour, long nNewValue);
	afx_msg long GetCustomColourText(short WhichColour);
	afx_msg void SetCustomColourText(short WhichColour, long nNewValue);
	afx_msg long GetCustomColourBackground(short WhichColour);
	afx_msg void SetCustomColourBackground(short WhichColour, long nNewValue);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()

};

/////////////////////////////////////////////////////////////////////////////


// MXP-oriented utilities

bool GetWord (CString & strResult, CString & str);
CString GetArgument (const CArgumentList & ArgumentList, 
                     CString strName, 
                     const int iPosition, 
                     const bool bLowerCase = false);
bool GetKeyword (CArgumentList & ArgumentList, 
                     CString strName);
bool IsNumeric (const CString & str);
bool IsValidName (const CString & str);
bool SetColour (const CString & strName, COLORREF & iColour);
CString GetAction (const CString & str);
CString GetHint (const CString & str);
void StringToList (const CString str, const char * delim, CStringList & thelist);
void ListToString (const CStringList & thelist, const char * delim, CString & str);


#define MAKE_STRING(msg) \
   (((ostringstream&) (ostringstream() << boolalpha << msg)).str())

// for timing things

class timer
  {
  LARGE_INTEGER start;
  LONGLONG iCounterFrequency;

  const string sReason;

  public:

    // constructor remembers time it was constructed
    timer (const string s) : sReason (s)
      {
      LARGE_INTEGER large_int_frequency;  
      QueryPerformanceFrequency (&large_int_frequency);
      iCounterFrequency = large_int_frequency.QuadPart;
      QueryPerformanceCounter (&start);
      TRACE (MAKE_STRING ("Start     : " << left << sReason << endl).c_str ());
      };

    // destructor gets current time, displays difference
    ~timer ()
      {
      LARGE_INTEGER finish;
      LONGLONG iTimeTaken;    
      QueryPerformanceCounter (&finish);  
      iTimeTaken = finish.QuadPart - start.QuadPart; 
      double fTime = ((double) iTimeTaken) /      
                     ((double) iCounterFrequency);    

      // TRACE or ::AfxMessageBox (for release builds)

      TRACE (MAKE_STRING ( "Time taken: " << left << sReason << " = "
           << fixed << fTime << " seconds." << endl ).c_str ());

      }
  };    // end of class timer
