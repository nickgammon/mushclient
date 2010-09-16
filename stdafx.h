// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

// disable some warnings (especially at warning level 4)

#pragma warning (disable : 4018)  // '<' : signed/unsigned mismatch
#pragma warning (disable : 4100)  // unreferenced formal parameter
#pragma warning (disable : 4127)  // conditional expression is constant
#pragma warning (disable : 4201)  // nonstandard extension used : nameless struct/union
#pragma warning (disable : 4244)  // conversion from 'int ' to 'char ', possible loss of data
#pragma warning (disable : 4244)  // conversion from 'int' to 'unsigned short', possible loss of data
#pragma warning (disable : 4503)  // decorated name length exceeded, name was truncated
#pragma warning (disable : 4505)  // unreferenced local function has been removed
#pragma warning (disable : 4511)  // copy constructor could not be generated
#pragma warning (disable : 4512)  // assignment operator could not be generated
#pragma warning (disable : 4514)  // unreferenced inline function has been removed
#pragma warning (disable : 4611)  // interaction between '_setjmp' and C++ object destruction is non-portable
#pragma warning (disable : 4663)  // C++ language change: to explicitly specialize class template yadda yadda
#pragma warning (disable : 4706)  // assignment within conditional expression
#pragma warning (disable : 4786)  // identifier was truncated to 'number' characters in the debug information
#pragma warning (disable : 4800)  // forcing value to bool 'true' or 'false' (performance warning)


#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define HAVE_CONFIG_H   // for PCRE
      
#pragma warning( push, 3)
#include <afx.h>

#ifdef _DEBUG
  #define _CRTDBG_MAP_ALLOC
  #include <stdlib.h>
  #include <crtdbg.h>

  #define _INC_MALLOC	     // exclude standard memory alloc procedures
#endif

#define USE_PYTHON    // Nick's "use Python" extension ;)
#define USE_TCL       // Nick's "use TCL" extension ;)
#define USE_PHP       // Nick's "use PHP" extension ;)
#define USE_RUBY      // Nick's "use Ruby" extension ;)

//#define PANE  // use pane windows
//#define USE_EXTRA_PANE     // 3rd pane for messages and stuff

#ifdef USE_EXTRA_PANE 
  #define EXTRA_PANE 0
  #define OUTPUT_PANE 1
  #define COMMAND_PANE 2
#else
  #define OUTPUT_PANE 0
  #define COMMAND_PANE 1
#endif

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC OLE automation classes
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <winsock2.h>
#include <afxsock.h>		// MFC socket extensions
#include <afxtempl.h>
#include <activscp.h>
#include <afxmt.h>
#include <afxpriv.h>

#include "format.h"
#include "exceptions.h"

#include <string>
#include <vector>
#include <deque>
#include <stack>
#include <map>      
#include <set>      
#include <list>      
#include <algorithm>  
#include <functional>
#include <iostream>
#include <iterator>
#include <sstream>
#pragma warning (pop)

extern "C"
  {
  #include "lua.h"
  #include "lauxlib.h"
  }

using namespace std ;

bool IsStringNumber (const string & s, const bool bSigned = false);

#include "regexp.h"
#include "mcdatetime.h" // MUSHclient date/time class

#include "sqlite3\sqlite3.h"  // SQLite3 database 

#define PREFERENCES_DATABASE_FILE "mushclient_prefs.sqlite" 

// less_than function for arrays
// first checks if the array key is an number, and if so returns
// a numeric less otherwise, a string less
// a number is assumed < a string
struct array_less : binary_function <string, string, bool>
  {

  bool operator() (const string & s1, const string & s2) const
    {
    bool bS1number = IsStringNumber (s1, true),
         bS2number = IsStringNumber (s2, true);

    // if both numbers, compare them as numbers

    if (bS1number && bS2number)
      return _atoi64 (s1.c_str ()) < _atoi64 (s2.c_str ());

    // if both strings, compare them as strings

    if (!bS1number && !bS2number)
      return s1 < s2;
    
    // a number is less than a string (eg. "1" < "a" )

    if (bS1number)
      return true;

    // otherwise >=
    return false;

    }
  }; // end of array_less

// keys compare numericly if possible
typedef map<string, string, array_less> tStringToStringMap;
typedef map<string, tStringToStringMap *, array_less> tStringMapOfMaps;

//
// Note - to compile you will also need the zlib library at the
//        same level as MUSHclient.
//
//  eg.   cvs get mushclient
//        cvs get zlib
//
#include "zlib\zlib.h"

extern "C"
  {
#include "scripting\shs.h"
  }

class CActivityDlg;
class CMUSHView;
class CMUSHclientDoc;

//#define USE_REALLOC   // use malloc/realloc/free for line text

// The define below gets the current app and casts it to the correct type

#define App (*((CMUSHclientApp*)AfxGetApp()))

// The define below gets the current frame and casts it to the correct type

#define Frame (*((CMainFrame*)App.m_pMainWnd))

// colours

enum { BLACK = 0, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE };

#define SAMECOLOUR 65535

#define NO_COLOUR 0xFFFFFFFF   // for COLORREF 

#define NOSOUNDLIT "(No sound)"  

// helper define for appending an "s" to plural amounts, eg. trigger(s)
#define PLURAL(arg) (arg), (arg) == 1 ? "" : "s"
// helper define for appending an "ies" to plural amounts, eg. entity/entities
#define PLURALIE(arg) (arg), (arg) == 1 ? "y" : "ies"
// helper define for appending an "es" to plural amounts, eg. alias(es)
#define PLURALES(arg) (arg), (arg) == 1 ? "" : "es"

// number of items in an array
#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))

#define DEFAULT_CHAT_PORT 4050
#define DEFAULT_CHAT_NAME "Name-not-set"

// function prototypes for minimum string validation in dialog boxes

void AFXAPI DDV_MinMaxString(CDataExchange* pDX, CString& value, int nMinChars, int nMaxChars);
void AFXAPI DDX_MinMaxString(CDataExchange* pDX, int nIDC, CString& value);
void AFXAPI DDV_MinMaxCBString(CDataExchange* pDX, CString& value, int nMinChars, int nMaxChars);
void AFXAPI DDX_MinMaxCBString(CDataExchange* pDX, int nIDC, CString& value);

// clipboard helper function

BOOL putontoclipboard (const CString & data, const bool bUnicode = false);
BOOL GetClipboardContents (CString & strClipboard, const bool bUnicode, const bool bWarning = true);

// label checking

BOOL CheckLabel (const CString & strLabel, const bool bScript = false);

// message used for wrapping up host name resolution. Note that WM_USER and a couple of other
// low WM_USER messages are used by MFC

#define WM_USER_HOST_NAME_RESOLVED (WM_USER + 1000)
#define WM_USER_SCRIPT_FILE_CONTENTS_CHANGED (WM_USER + 1001)
#define WM_USER_SHOW_TIPS (WM_USER + 1003)

// tray stuff

#define WM_USER_TRAY_ICON_ID 1       
#define WM_USER_TRAY_ICON_MESSAGE (WM_APP + 1)

#define MAX_CUSTOM 16    // maximum custom colours at present (up to 256)
#define OTHER_CUSTOM 16  // colour for triggers which represents 'other'

// Nick's bit manipulations routines, that require bit numbers, not masks
#define CHECK_BIT_NUMBER(flag, bit) (((flag) & (1 << bit)) != 0)
#define SET_BIT_NUMBER(flag, bit) ((flag) |= (1 << bit))
#define CLEAR_BIT_NUMBER(flag, bit) ((flag) &= ~(1 << bit))
#define TOGGLE_BIT_NUMBER(flag, bit) ((flag) ^= (1 << bit))

// ==================== GENERAL FIND TEXT ROUTINES ==================================

// class used by FindRoutine

class CProgressDlg;

class CFindInfo : public CObject
  {
  public:

  CFindInfo ()
    {
    m_bCanGoBackwards = true;
    m_bForwards = true;
    m_bMatchCase = false;
    m_bRegexp = false;
    m_bUTF8 = false;
    m_pFindPosition = NULL;
    m_pProgressDlg = NULL;
    m_iStartColumn = 0;
    m_iEndColumn = 0;
    m_nTotalLines = 0;
    m_nCurrentLine = 0;
    m_iControlColumns = 0;
    m_regexp = NULL;
    };                 // constructor

  ~CFindInfo () { delete m_regexp; };

  CString m_strTitle;           // title for find box
  bool m_bCanGoBackwards;       // can we find backwards?
  bool m_bForwards;             // direction
  bool m_bMatchCase;            // whether to match on case
  bool m_bAgain;                // repeat last find?
  bool m_bRegexp;               // regular expression?
  bool m_bUTF8;                 // is world UTF-8?
  int  m_iStartColumn;          // first column we found it on
  int  m_iEndColumn;            // last column we found it on
  long m_nTotalLines;           // maximum limit of search
  long m_nCurrentLine;          // which line we are at
  POSITION m_pFindPosition;     // what position we currently at
  CProgressDlg * m_pProgressDlg;// progress dialog
  int  m_iControlColumns;       // number of columns in the control
  t_regexp * m_regexp;          // compiled regular expression
  CStringList m_strFindStringList;  // previous things we found
  };

// prototype for "get next line" callback for find routine
// return TRUE if could not get another line

typedef bool (*GetNextLine) (const CObject * pObject,
                             CFindInfo & FindInfo,
                             CString & strLine);

// prototype for "initiate search" callback for find routine
  
typedef void (*InitiateSearch) (const CObject * pObject,
                                CFindInfo & FindInfo); 

// find routine - returns TRUE if wanted text found

bool FindRoutine (const CObject * pObject,       // passed back to callback routines
                  CFindInfo & FindInfo,          // details about find
                  const InitiateSearch pInitiateSearch, // how to re-initiate a find
                  const GetNextLine pGetNextLine);


// find-and-replace for strings

CString Replace (const CString strSource, 
                 const CString strTarget,
                 const CString strReplacement,
                 const bool bAll = true);    

int InitZlib (z_stream & m_zCompress);

void ReadNames (const LPCTSTR sName, const bool bNoDialog = false);
CString GenerateName (void);

CString GetText (const CWnd & theWindow);

// which OS do we have here?

extern bool bWinNT;
extern bool bWin95;
extern bool bWin98;
extern bool bWine;

BOOL Set_Up_Set_Strings (const int set_type,
                         CString & suggested_name,
                         CString & filter,
                         CString & title,
                         CString & suggested_extension);

CString ConvertToRegularExpression (const CString & strMatchString,
                                    const bool bWholeLine = true,
                                    const bool bMakeAsterisksWildcards = true);


// Helper routine for setting the font in certain description windows to a
// fixed-pitch courier, to make editing easier.

typedef CFont * ptrCFont;     // for changing fonts in edit windows

void FixFont (ptrCFont & pFont, 
              CEdit & editctrl,
              const CString strName,
              const int iSize,
              const int iWeight,
              const DWORD iCharset);

// for escaping out things like \r in a trigger
CString FixupEscapeSequences (const CString & strSource);

// translates "send to" numbers
CString GetSendToString (const unsigned short iWhere);

// translates connection status to text
CString GetConnectionStatus (const int iStatus);

// fixes German sequences

CString FixUpGerman (const CString strMessage);

// find our host name and IP address(es)

void GetHostNameAndAddresses (CString & strHostName, CString & strAddresses);

// for fixing up HTML text
CString FixHTMLString (const CString strToFix);
CString FixHTMLMultilineString (const CString strToFix);


// for tooltips
BOOL OnNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
bool CreateTextWindow (const char * sText,
                       const char * sTitle,
                       CMUSHclientDoc * pDoc,
                       __int64 iUniqueDocumentNumber,
                       const CString strFontName,
                       const LONG     iFontSize,
                       const LONG     iFontWeight,
                       const DWORD    iFontCharset,
                       const COLORREF textColour,
                       const COLORREF backColour,
                       const CString  strSearchString,
                       const CString  strRecallLinePreamble,
                       const bool bMatchCase,
                       const bool bRegexp,
                       const bool bCommands,
                       const bool bOutput,
                       const bool bNotes,
                       const int iLines,
                       const int iNotepadType);

CString ConvertSetTypeToString (const int set_type);
unsigned long MakeActionHash (const CString & strAction,
                              const CString & strHint,
                              const CString & strVariable);
bool GetClipboardColour (COLORREF & colour);
bool GetSelection (CEdit * pEdit, CString & strSelection);

// forum codes

CString QuoteForumCodes(const CString & strText) ;

// different ways of saving things
void Save_XML_number  (CArchive& ar, 
                       const char * sName, 
                       const long iNumber, 
                       const bool sameline = false);
void Save_XML_boolean (CArchive& ar, 
                       const char * sName, 
                       const long iNumber, 
                       const bool sameline = false);
void Save_XML_string  (CArchive& ar, 
                       const char * sName, 
                       const CString & str, 
                       const bool sameline = false);
void Save_XML_multi   (CArchive& ar, 
                       const char * sName, 
                       const CString & str);
void Save_XML_date    (CArchive& ar, 
                       const char * sName, 
                       const CTime & date, 
                       const bool sameline = false);
void Save_XML_colour  (CArchive& ar, 
                       const char * sName, 
                       const COLORREF & colour, 
                       const bool sameline = false);
void Save_XML_double    (CArchive& ar, 
                       const char * sName, 
                       const double & fNumber,
                       const bool sameline = false);

bool IsArchiveXML (CArchive& ar); // returns true if archive turns out to be XML
bool IsClipboardXML (void); // returns true if clipboard turns out to be XML
void SeeIfBase64 (CString & strText);
CString decodeBase64(const char *coded);
CString encodeBase64(const char * plaintext, const bool bMultiLine);
CString ColourToName (const COLORREF colour);
CString GetGUID (void);
CString GetUniqueID (void);
bool IsNumber (const CString & str, const bool bAllowSign = false);
void MakeRandomNumber (CMUSHclientDoc * pDoc, SHS_INFO & shsInfo);
void ShutDownSocket (CAsyncSocket & s);
CString ExtractDirectory (CString str);
CString RemoveFinalSlash (CString str);

#define PLUGIN_UNIQUE_ID_LENGTH 24

/////////////////////////////////////////////////////////////////////////////
//  Stuff for the file-change monitoring code

struct CThreadData
{
	char *	m_strFilename;    // which file to monitor
	HWND	m_hWnd;             // window to post event to
  DWORD m_pDoc;             // which document it belongs to
	HANDLE	m_hEvent;         // event which will become signalled
};

void ThreadFunc(LPVOID pParam);
void KillThread (HANDLE & pThread, CEvent & eventFileChanged);
HANDLE CreateMonitoringThread(const char * sName, DWORD pDoc, CEvent & eventFileChanged);

#define WM_USER_FILE_CONTENTS_CHANGED (WM_USER + 1001)

#define REGISTRATION_TIMER_ID 0x1001
#define SPLASH_SCREEN_TIMER_ID 0x1002
#define ACTIVITY_TIMER_ID 0x1003
#define COMMAND_QUEUE_TIMER_ID 0x1004
#define UNREGISTERED_DELAY_TIMER_ID 0x1005
#define TICK_TIMER_ID 0x1006

// macro for deleting everything in a map
// first, copy to a list so we don't have it in the list
// after it is deleted (eg. for a timer to find)
#define DELETE_MAP(mapname, pointertype)  \
do {                      \
  pointertype * pointer;  \
  CString strName;        \
  CTypedPtrList <CPtrList, pointertype*> mylist;    \
  POSITION pos;                                     \
  for (pos = mapname.GetStartPosition(); pos; )     \
    {                                               \
     mapname.GetNextAssoc (pos, strName, pointer);  \
     mylist.AddTail (pointer);                      \
    }                                               \
  mapname.RemoveAll();                              \
  for (pos = mylist.GetHeadPosition(); pos; )       \
     delete mylist.GetNext (pos);                   \
  mylist.RemoveAll ();                              \
  } while (false)
    
// macro for deleting everything in a list
#define DELETE_LIST(listname)  \
  do {                         \
  while (!listname.IsEmpty ())  \
      delete listname.RemoveHead (); \
  } while (false)

// bit manipulation
#define IS_SET(var, bit)	(((var) & (bit)) != 0)
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)	((var) ^= (bit))

class CStyle;

CStyle * GetNewStyle (const char * filename, const long linenumber);
void DeleteStyle (CStyle * pStyle, const char * filename, const long linenumber);

// plugin callback routines - start with OnPlugin so that we can advise
// users not to use that string for their own routines

#define ON_PLUGIN_INSTALL     "OnPluginInstall"
#define ON_PLUGIN_CONNECT     "OnPluginConnect"
#define ON_PLUGIN_DISCONNECT  "OnPluginDisconnect"
#define ON_PLUGIN_CLOSE       "OnPluginClose"
#define ON_PLUGIN_SAVE_STATE  "OnPluginSaveState"
#define ON_PLUGIN_WORLD_SAVE  "OnPluginWorldSave"
#define ON_PLUGIN_ENABLE      "OnPluginEnable"
#define ON_PLUGIN_DISABLE     "OnPluginDisable"
#define ON_PLUGIN_COMMAND     "OnPluginCommand" 
#define ON_PLUGIN_COMMAND_ENTERED     "OnPluginCommandEntered" 
#define ON_PLUGIN_GETFOCUS    "OnPluginGetFocus"
#define ON_PLUGIN_LOSEFOCUS   "OnPluginLoseFocus" 
#define ON_PLUGIN_TRACE       "OnPluginTrace" 
#define ON_PLUGIN_BROADCAST   "OnPluginBroadcast" 
#define ON_PLUGIN_SCREENDRAW  "OnPluginScreendraw" 
#define ON_PLUGIN_PLAYSOUND   "OnPluginPlaySound" 
#define ON_PLUGIN_TABCOMPLETE "OnPluginTabComplete" 
#define ON_PLUGIN_LIST_CHANGED "OnPluginListChanged"

//#define ON_PLUGIN_TOOLTIP     "OnPluginToolTip" 

// stuff received/send
#define ON_PLUGIN_SEND        "OnPluginSend" 
#define ON_PLUGIN_SENT        "OnPluginSent" 
#define ON_PLUGIN_LINE_RECEIVED "OnPluginLineReceived"    
#define ON_PLUGIN_PACKET_RECEIVED "OnPluginPacketReceived"    
#define ON_PLUGIN_PARTIAL_LINE "OnPluginPartialLine"    
#define ON_PLUGIN_TELNET_OPTION "OnPluginTelnetOption"    
#define ON_PLUGIN_TELNET_REQUEST "OnPluginTelnetRequest"    
#define ON_PLUGIN_TELNET_SUBNEGOTIATION "OnPluginTelnetSubnegotiation"    
#define ON_PLUGIN_IAC_GA "OnPlugin_IAC_GA"    
#define ON_PLUGIN_WORLD_OUTPUT_RESIZED "OnPluginWorldOutputResized"    
#define ON_PLUGIN_TICK "OnPluginTick"    
#define ON_PLUGIN_MOUSE_MOVED "OnPluginMouseMoved"    
#define ON_PLUGIN_COMMAND_CHANGED "OnPluginCommandChanged"    

// MXP stuff
#define ON_PLUGIN_MXP_START       "OnPluginMXPstart"
#define ON_PLUGIN_MXP_STOP        "OnPluginMXPstop"
#define ON_PLUGIN_MXP_OPENTAG     "OnPluginMXPopenTag"    
#define ON_PLUGIN_MXP_CLOSETAG    "OnPluginMXPcloseTag"
#define ON_PLUGIN_MXP_SETVARIABLE "OnPluginMXPsetVariable"
#define ON_PLUGIN_MXP_SETENTITY   "OnPluginMXPsetEntity"
#define ON_PLUGIN_MXP_ERROR       "OnPluginMXPerror"

// chat stuff
#define ON_PLUGIN_CHAT_ACCEPT     "OnPluginChatAccept"
#define ON_PLUGIN_CHAT_MESSAGE    "OnPluginChatMessage"
#define ON_PLUGIN_CHAT_MESSAGE_OUT "OnPluginChatMessageOut"
#define ON_PLUGIN_CHAT_DISPLAY    "OnPluginChatDisplay"
#define ON_PLUGIN_CHAT_NEWUSER    "OnPluginChatNewUser"
#define ON_PLUGIN_CHAT_USERDISCONNECT  "OnPluginChatUserDisconnect"

// ANSI Colour Codes

#define ANSI_RESET             0 
#define ANSI_BOLD              1
#define ANSI_BLINK             3 
#define ANSI_UNDERLINE         4 
#define ANSI_SLOW_BLINK        5
#define ANSI_FAST_BLINK        6 
#define ANSI_INVERSE           7 

#define ANSI_CANCEL_BOLD      22 
#define ANSI_CANCEL_BLINK     23 
#define ANSI_CANCEL_UNDERLINE 24 
#define ANSI_CANCEL_SLOW_BLINK  25
#define ANSI_CANCEL_INVERSE   27 

#define ANSI_TEXT_BLACK       30 
#define ANSI_TEXT_RED         31 
#define ANSI_TEXT_GREEN       32 
#define ANSI_TEXT_YELLOW      33 
#define ANSI_TEXT_BLUE        34 
#define ANSI_TEXT_MAGENTA     35 
#define ANSI_TEXT_CYAN        36 
#define ANSI_TEXT_WHITE       37 
#define ANSI_TEXT_256_COLOUR  38
         
#define ANSI_SET_FOREGROUND_DEFAULT 39
                     
#define ANSI_BACK_BLACK       40 
#define ANSI_BACK_RED         41 
#define ANSI_BACK_GREEN       42 
#define ANSI_BACK_YELLOW      43 
#define ANSI_BACK_BLUE        44 
#define ANSI_BACK_MAGENTA     45 
#define ANSI_BACK_CYAN        46 
#define ANSI_BACK_WHITE       47 
#define ANSI_BACK_256_COLOUR  48

#define ANSI_SET_BACKGROUND_DEFAULT 49

// for mapping command IDs to names (for accelerators)

typedef struct 
  {
  int						iCommandID;			 // eg. ID_EDIT_SELECT_ALL
  const char *	sCommandName;    // eg. "SelectAll"
  } tCommandIDMapping;

// for mapping virtuals to names (for accelerators)

typedef struct 
  {
  int						iVirtualKey;     // eg. VK_ESCAPE
  const char *	sKeyName;		     // eg. "Esc"
  } tVirtualKeyMapping;

CString CommandIDToString (const int ID);
int StringToCommandID (const CString str);

// keypad indexes

enum
  {
  eKeypad_0            , 
  eKeypad_1            , 
  eKeypad_2            , 
  eKeypad_3            , 
  eKeypad_4            , 
  eKeypad_5            , 
  eKeypad_6            , 
  eKeypad_7            , 
  eKeypad_8            , 
  eKeypad_9            , 
  eKeypad_Dot          , 
  eKeypad_Slash        , 
  eKeypad_Star         , 
  eKeypad_Dash         , 
  eKeypad_Plus         , 
  eCtrl_Keypad_0       , 
  eCtrl_Keypad_1       , 
  eCtrl_Keypad_2       , 
  eCtrl_Keypad_3       , 
  eCtrl_Keypad_4       , 
  eCtrl_Keypad_5       , 
  eCtrl_Keypad_6       , 
  eCtrl_Keypad_7       , 
  eCtrl_Keypad_8       , 
  eCtrl_Keypad_9       , 
  eCtrl_Keypad_Dot     , 
  eCtrl_Keypad_Slash   , 
  eCtrl_Keypad_Star    , 
  eCtrl_Keypad_Dash    , 
  eCtrl_Keypad_Plus    , 

  eKeypad_Max_Items   // this must be last   !!!
  };

// The following macros set and clear, respectively, given bits
// of the C runtime library debug flag, as specified by a bitmask.
#ifdef   _DEBUG
  #define  SET_CRT_DEBUG_FIELD(a) \
              _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
  #define  CLEAR_CRT_DEBUG_FIELD(a) \
              _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
//  #define NEWSTYLE GetNewStyle (__FILE__, __LINE__)
//  #define DELETESTYLE(arg) DeleteStyle (arg, __FILE__, __LINE__)
  #define NEWSTYLE new CStyle
  #define DELETESTYLE(arg) delete arg

#else
  #define  SET_CRT_DEBUG_FIELD(a)   ((void) 0)
  #define  CLEAR_CRT_DEBUG_FIELD(a) ((void) 0)

  #define NEWSTYLE new CStyle
  #define DELETESTYLE(arg) delete arg
#endif

#define SPACES " \t\r\n"
#define SPELLCHECKFUNCTION "spellcheck"

// there is a clash in the MS compiler for min so I do my own here
template <typename T> 
inline T minimum (const T a, const T b)
  {
  return (a < b) ? a : b;
  } // end of minimum

// there is a clash in the MS compiler for max so I do my own here
template <typename T> 
inline T maximum (const T a, const T b)
  {
  return (a > b) ? a : b;
  } // end of maximum

inline string trim_right (const string & s, const string & t = SPACES)
  { 
  string d (s); 
  string::size_type i (d.find_last_not_of (t));
  if (i == string::npos)
    return "";
  else
   return d.erase (d.find_last_not_of (t) + 1) ; 
  }  // end of trim_right

inline string trim_left (const string & s, const string & t = SPACES) 
  { 
  string d (s); 
  return d.erase (0, s.find_first_not_of (t)) ; 
  }  // end of trim_left

inline string trim (const string & s, const string & t = SPACES)
  { 
  string d (s); 
  return trim_left (trim_right (d, t), t) ; 
  }  // end of trim

// string find-and-replace
inline string FindAndReplace
  (const string& source, const string target, const string replacement)
  {
  string str = source;
  string::size_type pos = 0,   // where we are now
                    found;     // where the found data is
  while ((found = str.find (target, pos)) != string::npos)
    {
    str.replace (found, target.size (), replacement);
    pos = found + replacement.size ();
    }
  return str;
  };   // end of FindAndReplace

string GetWord (string & s, 
                const string delim = " ",
                const bool trim_spaces = true);

void StringToVector (const string s, 
                     vector<string> & v,
                     const string delim = " ", 
                     const bool trim_spaces = true);

string VectorToString (const vector<string> & v, 
                       const string delim = " ");

void StringToKeyCode (const CString strKey,
                      BYTE & fVirt,
                      WORD & key);

CString KeyCodeToString (const BYTE fVirt, const WORD key);

void ShowFunctionslist (CString & strSelection, int nStartChar = 0, int nEndChar = 0, const bool bLua = false);
CString GetSelectedFunction (CString & strSelection, int & nStartChar, int & nEndChar);
void FunctionMenu (CEdit & editctrl, const bool bLua, set<string> * extraItems = NULL, const bool bFunctions = true);
void ShowHelp (const CString strPrefix, const CString strTopic);

string tolower (const string & s);

void FindMatchingBrace (CEdit & ctlEdit, const bool bSelect);

void GetButtonSize (CWnd & ctlWnd, int & iHeight, int & iWidth);

void metaphone (const char *name, char * metaph, int metalen);
int EditDistance (const string source, const string target);

const char * Make_Absolute_Path (CString strFileName);
const char * Convert_PCRE_Runtime_Error (const int iError);

lua_State *MakeLuaState (void);

// send a window to the front
bool SendToFront (const char * name);

// 1 = invert, 2 = lighter, 3 = darker, 4 = less colour, 5 = more colour

enum
  { 
  ADJUST_COLOUR_NO_OP = 0,
  ADJUST_COLOUR_INVERT,   // 1
  ADJUST_COLOUR_LIGHTER,  // 2
  ADJUST_COLOUR_DARKER,   // 3
  ADJUST_COLOUR_LESS_COLOUR, // 4
  ADJUST_COLOUR_MORE_COLOUR // 5
  };


// script numbers - must correspond to script combo-box in
// scripting configuration and plugin creation
enum
  {
  SCRIPT_VBSCRIPT = 0,
  SCRIPT_JSCRIPT = 1,
  SCRIPT_PERL = 2,
  SCRIPT_PYTHON = 3,
  SCRIPT_TCL = 4,
  SCRIPT_LUA = 5,
  SCRIPT_PHP = 6,
  SCRIPT_RUBY = 7
  };

long AdjustColour (long Colour, short Method);

void SetDefaultAnsiColours (COLORREF * normalcolour, COLORREF * boldcolour);
void SetDefaultCustomColours (COLORREF * customtext, COLORREF * customback);

// safe_for_each.  Apply a function to every element of a range.
// should handle correctly an element deleting itself
// See Josuttis p 205.

template <class ITER, class F> inline
F safe_for_each (ITER first, ITER last, F func)
  {
  while (first != last)
    func (*first++);
  return func;
  }

#define MUSHCLIENT_FONT_CHARSET DEFAULT_CHARSET
#define MUSHCLIENT_FONT_FAMILY FIXED_PITCH

#define WINDOW_TABS_NONE 0
#define WINDOW_TABS_TOP 1
#define WINDOW_TABS_BOTTOM 2

// in file  mt19937-2.c
extern "C"
  {
//  void sgenrand (unsigned long seed);
//  double genrand (void);    // obsolete now
  void init_genrand (unsigned long seed);
  double genrand (void) ;
  void init_by_array(unsigned long init_key[], int key_length);
  }

// if timer interval <= 0 use this
#define MIN_TIMER_INTERVAL 100      // milliseconds

// stuff we borrow from PCRE

extern "C" int _pcre_valid_utf8(const unsigned char  *buf, int length);
extern "C" int _pcre_ord2utf8(int cvalue, unsigned char *buffer);
extern "C" const unsigned char _pcre_utf8_table4[];

// i18n stuff

// display a message box, using UTF-8 (not straight ASCII)
int UMessageBox (const char * sText, UINT nType = MB_OK, const char * = "MUSHclient");
// translate lpszText and then call UMessageBox
int TMessageBox (const char * sText, UINT nType = MB_OK);
// translate text - result is statically allocated - do not attempt to modify or store it
const char * TranslateGeneric (const char * sText, const char * sSection);
// translate formatted text - result is statically allocated - do not attempt to modify or store it
const char * TFormat (const char * sFormat, ...);
// translate a static string
#define Translate(string) TranslateGeneric (string, "messages")
// translate a date/time string
#define TranslateTime(string) TranslateGeneric (string, "times")
// translate a heading string (eg. for dialogs lists)
#define TranslateHeading(string) TranslateGeneric (string, "headings")

// check a brush is valid, der
long ValidateBrushStyle (const long BrushStyle, 
                          const long PenColour, 
                          const long BrushColour, 
                          CBrush & br);

// strings marked with this macro are to be translated in the localization file
// however not at runtime (ie. they are in a table, and the table lookup will be translated)
#define Translate_NoOp(string) (string)

#define ALIASES_USE_UTF8 0

#define IsCTRLpressed()  ( (GetKeyState(VK_CONTROL) & (1 << (sizeof(SHORT)*8-1))) != 0 )
#define IsSHIFTpressed() ( (GetKeyState(VK_SHIFT) & (1 << (sizeof(SHORT)*8-1))) != 0   )

// ascii to double
int myAtoF(const char *z, double *pResult);

// colour stuff

#define CLAMP(x) (((x) < 0) ? 0 : (x > 255) ? 255 : (unsigned char) (x))


// see: http://www.nathanm.com/photoshop-blending-math/
// and: http://www.pegtop.net/delphi/articles/blendmodes/                   

#define uint8 unsigned char
#define sqr(x) ((x)*(x))    // square of x

// A = blend, B = base

#define Blend_Normal(A,B)	((uint8)(A))
#define Blend_Lighten(A,B)	((uint8)((B > A) ? B:A))
#define Blend_Darken(A,B)	((uint8)((B > A) ? A:B))
#define Blend_Multiply(A,B)	((uint8)((A * B) / 255))
#define Blend_Average(A,B)	((uint8)((A + B) / 2))
#define Blend_Add(A,B)	((uint8)((A + B > 255) ? 255:(A + B)))
#define Blend_Subtract(A,B)	((uint8)((A + B < 255) ? 0:(A + B - 255)))
#define Blend_Difference(A,B)	((uint8)(abs(A - B)))
#define Blend_Negation(A,B)	((uint8)(255 - abs(255 - A - B)))
#define Blend_Screen(A,B)	((uint8)(255 - (((255 - A) * (255 - B)) >> 8)))
#define Blend_Exclusion(A,B)	((uint8)(A + B - 2 * A * B / 255))
#define Blend_Overlay(A,B)	((uint8)((B < 128) ? (2 * A * B / 255):(255 - 2 * (255 - A) * (255 - B) / 255)))
#define Blend_SoftLight(A,B)	((uint8) ((A * B) >> 8) +  ((B * (255 - (((255 - B) * (255-A)) >> 8) - ((A * B) >> 8) )) >> 8) )           // c = ((A * B) >> 8)
#define Blend_HardLight(A,B)	(Blend_Overlay(B,A))
#define Blend_ColorDodge(A,B)	((uint8)((A == 255) ? A:((((unsigned long) B) << 8 ) / (255 - A) > 255) ? 255:((((unsigned long) B) << 8 ) / (255 - A))))
#define Blend_ColorBurn(A,B)	((uint8)((A == 0) ? 0:((255 - (( ((unsigned long)(255 - B)) << 8 ) / A)) < 0) ? 0:(255 - (( ((unsigned long)(255 - B)) << 8 ) / A))))
#define Blend_LinearDodge(A,B)	(Blend_Add(A,B))
#define Blend_LinearBurn(A,B)	(Blend_Subtract(A,B))
#define Blend_LinearLight(A,B)	((uint8)(A < 128) ? Blend_LinearBurn((2 * A),B):Blend_LinearDodge((2 * (A - 128)),B))
#define Blend_VividLight(A,B)	((uint8)(A < 128) ? Blend_ColorBurn((2 * A),B):Blend_ColorDodge((2 * (A - 128)),B))
#define Blend_PinLight(A,B)	((uint8)(A < 128) ? Blend_Darken((2 * A),B):Blend_Lighten((2 *(A - 128)),B))
#define Blend_HardMix(A,B)	((uint8)(A < 255 - B) ? 0:255)
#define Blend_Reflect(A,B)	((uint8)((B == 255) ? B:((A * A / (255 - B) > 255) ? 255:(A * A / (255 - B)))))
#define Blend_Glow(A,B)	(Blend_Reflect(B,A))
#define Blend_Phoenix(A,B)	((uint8)(min(A,B) - max(A,B) + 255))
#define Blend_Opacity(A,B,F,O)	((uint8)(O * F(A,B) + (1 - O) * B))

#define Simple_Opacity(B,X,O)  ((uint8)(O * X + (1 - O) * B))

#define Blend_InverseColorDodge(A,B)	Blend_ColorDodge (B, A)
#define Blend_InverseColorBurn(A,B)	Blend_ColorBurn (B, A)
#define Blend_Freeze(A,B)	((uint8)((A == 0) ? A:((255 - sqr(255 - B)/ A < 0) ? 0:(255 - sqr(255 - B) / A))))
#define Blend_Heat(A,B)	 Blend_Freeze (B, A)
#define Blend_Stamp(A,B)	((uint8)((B + 2*A - 256 < 0) ? 0 : (B + 2*A - 256 > 255) ? 255 : (B + 2*A - 256)    ))
#define Blend_Interpolate(A,B)	((uint8)((cos_table [A] + cos_table [B]> 255) ? 255 : (cos_table [A] + cos_table [B])  ))

#define Blend_Xor(A,B)	((uint8) A ^ B )
#define Blend_And(A,B)	((uint8) A & B )
#define Blend_Or(A,B)	((uint8) A | B )
#define Blend_A(A,B)	((uint8) (A))
#define Blend_B(A,B)	((uint8) (B))


long LoadPng (LPCTSTR FileName, HBITMAP & hbmp);  // load a PNG file into a bitmap
long LoadPngMemory (unsigned char * Buffer, const size_t Length, HBITMAP & hbmp, const bool bAlpha);  // load a PNG from memory into a bitmap

void ChangeToFileBrowsingDirectory ();
void ChangeToStartupDirectory ();

#define SCRIPTERRORFORECOLOUR "orangered"  // "darkorange"
#define SCRIPTERRORBACKCOLOUR "black"
#define SCRIPTERRORCONTEXTFORECOLOUR "burlywood"

/* These macros are the standard way of turning unquoted text into C strings.
They allow macros like PCRE_MAJOR to be defined without quotes, which is
convenient for user programs that want to test its value. */

#define STRING(a)  # a
#define XSTRING(s) STRING(s)

#define DEBUG_PLUGIN_ID "138a692642ab4f9e7a1af63b"  // special plugin ID for Debug "summary"
bool IsPluginID (const char * sID);
bool IsSubName (const char * sName);

typedef struct 
  {
  const char *  sFunction;			// name of function (eg. "WindowShow")
  const char *	sArgs;          // arguments (eg. "(Window, Showflag)")
  } tInternalFunctionsTable;

// flag not in my version of the API
#ifndef BIF_NEWDIALOGSTYLE
  #define BIF_NEWDIALOGSTYLE 0x00000040
#endif 
