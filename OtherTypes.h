// types used by the world document

// wildcard definitions for triggers

#pragma once

#define DEFAULT_TRIGGER_SEQUENCE 100
#define DEFAULT_ALIAS_SEQUENCE 100

/*

Trigger iMatch flag will be as follows:

  BFS    

  B = background colour, see enum in doc.h
  F = foreground colour, see enum in doc.h
  S = style bits, see define in doc.h

*/

#define TRIGGER_MATCH_TEXT 0x0080   // high order of text nibble
#define TRIGGER_MATCH_BACK 0x0800   // high order of back nibble
#define TRIGGER_MATCH_HILITE 0x1000
#define TRIGGER_MATCH_UNDERLINE 0x2000
#define TRIGGER_MATCH_BLINK 0x4000
#define TRIGGER_MATCH_INVERSE 0x8000

// where triggers/aliases/timers get sent

enum { eSendToWorld,        //  0   - send to MUD
       eSendToCommand,      //  1   - put in command window
       eSendToOutput,       //  2   - display in output window
       eSendToStatus,       //  3   - put in status line
       eSendToNotepad,      //  4   - new notepad
       eAppendToNotepad,    //  5   - append to notepad
       eSendToLogFile,      //  6   - put in log file
       eReplaceNotepad,     //  7   - replace notepad
       eSendToCommandQueue, //  8   - queue it
       eSendToVariable,     //  9   - set a variable
       eSendToExecute,      // 10   - re-parse as command
       eSendToSpeedwalk,    // 11   - send to MUD as speedwalk
       eSendToScript,       // 12   - send to script engine
       eSendImmediate,      // 13   - send without queuing
       eSendToScriptAfterOmit, // 14   - send to script, after omitting from output
#ifdef PANE
       eSendToPane,         // 15   - send to a pane window
#endif
//       eSendDelayed,        // ??

       eSendToLast  // THIS MUST BE LAST!!!
      };

// File types
 
enum { WORLD, TRIGGER, ALIAS, COLOUR, MACRO, STRING, TIMER };


// Alias match types

enum { START_OF_LINE, ANYWHERE, EXACT };

// Macro send types

enum { REPLACE_COMMAND, SEND_NOW, ADD_TO_COMMAND };

// String types

enum {
     STR_PAGE_INCOMING_1,
     STR_PAGE_INCOMING_2,
     STR_PAGE_OUTGOING_1,
     STR_PAGE_OUTGOING_2,
     STR_WHISPER_INCOMING_1,
     STR_WHISPER_INCOMING_2,
     STR_WHISPER_OUTGOING_1,
     STR_WHISPER_OUTGOING_2,
     STR_MAIL,
     STR_GAME,
     STR_SET
     }; // end of string types


// macro types
enum {
     MAC_UP,
     MAC_DOWN,
     MAC_NORTH,
     MAC_SOUTH,
     MAC_EAST,
     MAC_WEST,
     MAC_EXAMINE,
     MAC_LOOK,
     MAC_PAGE,
     MAC_SAY,
     MAC_WHISPER,
     MAC_DOING,
     MAC_WHO,
     MAC_DROP,
     MAC_TAKE,
     MAC_F2,
     MAC_F3,
     MAC_F4,
     MAC_F5,
     MAC_F7,
     MAC_F8,
     MAC_F9,
     MAC_F10,
     MAC_F11,
     MAC_F12,
     MAC_SHIFT_F2,
     MAC_SHIFT_F3,
     MAC_SHIFT_F4,
     MAC_SHIFT_F5,
     MAC_SHIFT_F6,
     MAC_SHIFT_F7,
     MAC_SHIFT_F8,
     MAC_SHIFT_F9,
     MAC_SHIFT_F10,
     MAC_SHIFT_F11,
     MAC_SHIFT_F12,
     MAC_CTRL_F2,
     MAC_CTRL_F3,
     MAC_CTRL_F5,
     MAC_CTRL_F7,
     MAC_CTRL_F8,
     MAC_CTRL_F9,
     MAC_CTRL_F10,
     MAC_CTRL_F11,
     MAC_CTRL_F12,
     MAC_LOGOUT,
     MAC_QUIT,

     // new ones - v 3.42
     MAC_ALT_A,
     MAC_ALT_B,
     MAC_ALT_J,
     MAC_ALT_K,
     MAC_ALT_L,
     MAC_ALT_M,
     MAC_ALT_N,
     MAC_ALT_O,
     MAC_ALT_P,
     MAC_ALT_Q,
     MAC_ALT_R,
     MAC_ALT_S,
     MAC_ALT_T,
     MAC_ALT_U,
     MAC_ALT_X,
     MAC_ALT_Y,
     MAC_ALT_Z,

     // new ones - v 3.44
     MAC_F1,
     MAC_CTRL_F1,
     MAC_SHIFT_F1,
     MAC_F6,
     MAC_CTRL_F6,

     MACRO_COUNT    // this must be last!! 
     };  // end of macro types, currently 64 of them

/////////////////////////////////////////////////////////////////////////////
//  CAction - these are actions that are done by a hyperlink

/*

  To save memory we will share these actions between links, and just store
  a pointer to it in the style run.

  Some of them can get fairly long, eg:

  Action - send to MUD: "Wear #24850|look #24850|examine #24850|
                         drop #24850~Wear The Curse of Kral|
                         Look at The Curse of Kral|Examine The Curse of Kral|
                         Drop The Curse of Kral"

  ... and then the above may be repeated for every *character* of the hyperlink
      if they are in different colours.

  Now, assuming that you go to the same rooms more than once, or look at your
  inventory more than once, such items will be cached, by being stored once in the
  CAction list. I use a reference count to know if we can ever get rid of one.

  */

class CAction : public CObject
  {

  public:

  CString m_strAction;   // What to send - multiple ones delimited by |
  CString m_strHint;     // Hint - flyover hint, and prompts for actions
  CString m_strVariable; // which variable to set (FLAG in MXP)
  unsigned long m_iHash;       // for quick lookups - hash of action, hint, variable

  protected: 

  int     m_iRefCount;   // number of users of this item
  CMUSHclientDoc * m_pDoc;  // which document it belongs to

  public:

  CAction (const CString & strAction,
           const CString & strHint,
           const CString & strVariable,
           CMUSHclientDoc * pDoc); // constructor

  // call AddRef when another style uses this action

  void AddRef (void);

  // call Release when a style no longer uses the action

  void Release (void);

  int GetReferenceCount (void) { return m_iRefCount; };

  };

typedef CTypedPtrList <CPtrList, CAction*> CActionList;

/*
  25 May 2001.

  In order to accomodate MXP, and future expansion such as multiple fonts,
  hyperlinks, RGB colours, and so on, I am moving from a straight "style"
  array (at at pre 3.10) to a linked list of styles.

  Each item in the list will describe text that is to be drawn in an identical
  way, and have the same effect (eg. the same hyperlink).

  The number of bytes affected (in "text" in CLine) will be in iLength.

  */

/////////////////////////////////////////////////////////////////////////////
//  CStyle


// values for COLOURTYPE below

#define COLOUR_ANSI     0x0000     //  ANSI colour from ANSI table
#define COLOUR_CUSTOM   0x0100     //  Custom colour from custom table
#define COLOUR_RGB      0x0200     //  RGB colour in iColour
#define COLOUR_RESERVED 0x0300     //  reserved

// values for ACTIONTYPE below

#define ACTION_NONE      0x0000  // do nothing
#define ACTION_SEND      0x0400  // send strAction to MUD
#define ACTION_HYPERLINK 0x0800  // http:// blah or  mailto:blah
#define ACTION_PROMPT    0x0C00  // send strAction to command window

// style flags 

#define NORMAL     0x0000   // a mnemonic way of clearing all attributes
#define HILITE     0x0001   // bold
#define UNDERLINE  0x0002   // underline
#define BLINK      0x0004   // italic??
#define INVERSE    0x0008   // need to invert it
#define CHANGED    0x0010   // colour has been changed by a trigger
#define COLOURTYPE 0x0300   // type of colour in iForeColour/iBackColour, see above
#define ACTIONTYPE 0x0C00   // action type, see above
#define STYLE_BITS 0x0FFF   // everything except START_TAG
#define START_TAG  0x1000   // strAction is tag name - start of tag (eg. <b> )
#define TEXT_STYLE 0x000F   // bold, underline, italic, inverse flags

// eg. <send "command1|command2|command3" hint="click to see menu|Item 1|Item 2|Item 2">this is a menu link</SEND>

#define POPUP_DELIMITER "|"  // delimiter between different popup menu items

class CStyle : public CObject
  {

  public:

  unsigned short iLength;     // how many bytes (letters) are affected in "text"
  unsigned short iFlags;      // see define above
  COLORREF       iForeColour; // RGB foreground colour, or ANSI/custom colour number
  COLORREF       iBackColour; // RGB background colour, or ANSI/custom colour number
  CAction *      pAction;     // what action, if any this item carries out
                              //  - also stores variables  
  CStyle () 
    { 
    iForeColour = WHITE;
    iBackColour = BLACK;
    iLength = iFlags = 0; 
    pAction = NULL;
    };   // constructor

  ~CStyle () 
    {
    if (pAction)
      pAction->Release ();
    };  // destructor

  };

typedef CTypedPtrList <CPtrList, CStyle*> CStyleList;


/////////////////////////////////////////////////////////////////////////////
//  CLine

// bit settings for "flags" below

const int COMMENT = 0x01;        // this is a comment from a script
const int USER_INPUT = 0x02;     // this is echoed user input
const int LOG_LINE = 0x04;       // this line is to be logged
const int BOOKMARK = 0x08;       // line is bookmarked
const int HORIZ_RULE = 0x10;     // line is a horizontal rule

const int NOTE_OR_COMMAND = 0x03;         // for testing if line is an output line or not

class CLine : public CObject
  {

  public:

  bool hard_return;
  unsigned char flags;
  int len;
  int last_space;
  char * text;          // allocated as necessary and then resized
  CStyleList styleList; // list of styles applying to text, see above
  CTime m_theTime;      // time this line arrived
  LARGE_INTEGER m_lineHighPerformanceTime;  
  int iMemoryAllocated; // size of buffer allocated for "text"

  long m_nLineNumber;

  CLine (const long nLineNumber, 
         const unsigned int nWrapColumn,
         const unsigned short iFlags,      
         const COLORREF       iForeColour,
         const COLORREF       iBackColour,
         const bool bUnicode 
         );   // constructor
  ~CLine ();    // destructor

  };

typedef CTypedPtrList <CPtrList, CLine*> CLineList;

/////////////////////////////////////////////////////////////////////////////
//  CAlias

class CAlias : public CObject
  {
  DECLARE_DYNAMIC(CAlias)

  CAlias (void)          // constructor
  {
   bIgnoreCase = FALSE;
   bEnabled = TRUE;
   dispid = DISPID_UNKNOWN;
   nUpdateNumber = 0;
   nInvocationCount = 0;
   nMatched = 0;
   bExpandVariables = FALSE;
   bOmitFromLog = FALSE;
   bOmitFromOutput = FALSE;
   bRegexp = FALSE;
   regexp = NULL;
   iSequence = DEFAULT_ALIAS_SEQUENCE;
   bKeepEvaluating = FALSE;

//   bSpeedWalk = FALSE;    // see iSendTo
//   bQueue = FALSE;        // see iSendTo
//   bSetVariable = FALSE;  // see iSendTo
//   bDelayed = FALSE;
   bMenu = FALSE;
   tWhenMatched = 0;
   bTemporary = false;
   bIncluded = false;
   bSelected = false;
   iSendTo = eSendToWorld;
   bEchoAlias = false;
   bOmitFromCommandHistory = false;
   iUserOption = 0;
   wildcards.resize (MAX_WILDCARDS);
   bExecutingScript = false;
   bOneShot = false;

  };

  ~CAlias () { delete regexp; };

  public:

  CString name;
  CString contents;
  unsigned short bIgnoreCase;  

// new in version 8

  CString strLabel;           // alias label
  CString strProcedure;       // procedure to execute
  unsigned short bEnabled;     // alias enabled
  unsigned short bExpandVariables;     // expand variables (eg. @food)
  unsigned short bOmitFromLog;  // omit from log file?
  unsigned short bRegexp;  // use regular expressions
  unsigned short bOmitFromOutput;    // omit alias from output screen?

// new in version 12

  unsigned short iSequence; // which order, lower is sooner
//  unsigned short bSpeedWalk;    // true means evaluate as speed walk
//  unsigned short bQueue;    // true means queue it rather than send it
  unsigned short bMenu;     // make pop-up menu from this alias?
//  unsigned short bDelayed;    // true means use AddTimer, eg. 5; go east

// in XML version

  CString strGroup;   // group it belongs to
  CString strVariable;    // which variable to set (for send to variable)
//  unsigned short bSetVariable;   // if true, set variable rather than send it
  unsigned short iSendTo;    // where alias is sent to (see enum above)
  unsigned short bKeepEvaluating;  // if true, keep evaluating triggers
  unsigned short bEchoAlias;    // if true, echo alias itself to the output window
  long   iUserOption;           // user-settable flags etc.
  unsigned short bOmitFromCommandHistory;   // omit from command history
  BOOL bOneShot;              // if true, alias only fires once

// computed at run-time
  
  DISPID dispid;                    // dispatch ID for calling the script
  __int64 nUpdateNumber;            // for detecting update clashes
  long  nInvocationCount; // how many times procedure called
  long  nMatched;         // how many times the alias matched
  vector<string> wildcards;   // matching wildcards
  t_regexp * regexp;      // compiled regular expression, if needed
  CTime tWhenMatched;     // when last matched
  bool bTemporary;        // if true, don't save it
  bool bIncluded;       // if true, don't save it
  bool bSelected;       // if true, selected for use in a plugin
  bool bExecutingScript;    // if true, executing a script and cannot be deleted
  CString strInternalName;  // name it is stored in the alias map under
  };

// map for lookup by name
typedef CTypedPtrMap <CMapStringToPtr, CString, CAlias*> CAliasMap;
// array for saving in order
typedef CTypedPtrArray <CPtrArray, CAlias*> CAliasArray;
// list for alias evaluation
typedef CTypedPtrList <CPtrList, CAlias*> CAliasList;
// map for lookup name from pointer
typedef map <CAlias*, string> CAliasRevMap;

/////////////////////////////////////////////////////////////////////////////
//  CTrigger

#define TRIGGER_COLOUR_CHANGE_BOTH 0
#define TRIGGER_COLOUR_CHANGE_FOREGROUND 1
#define TRIGGER_COLOUR_CHANGE_BACKGROUND 2


class CTrigger : public CObject
  {
  DECLARE_DYNAMIC(CTrigger)

  CTrigger (void)          // constructor

    {
     ignore_case = FALSE;
     omit_from_log = false;
     bOmitFromOutput = FALSE;
     bKeepEvaluating = FALSE;
     bExpandVariables = FALSE;
     iSendTo = eSendToWorld;
     bEnabled = TRUE;
     dispid = DISPID_UNKNOWN;
     nUpdateNumber = 0;
     colour = 0;    // custom colour 1
     nInvocationCount = 0;
     iClipboardArg = 0;
     nMatched = 0;
     bRegexp = false;
     bRepeat = false;
     regexp = NULL;
     iSequence = DEFAULT_TRIGGER_SEQUENCE;
     iMatch = 0;
     iStyle = 0;
     bSoundIfInactive = false;
     tWhenMatched = 0;
     bLowercaseWildcard = false;
     bTemporary = false;
     bIncluded = false;
     bSelected = false;
     iUserOption = 0;
     iOtherForeground = 0;
     iOtherBackground = 0;
     bMultiLine = false;
     iLinesToMatch = 0;       
     iColourChangeType = TRIGGER_COLOUR_CHANGE_BOTH;
     wildcards.resize (MAX_WILDCARDS);
     bExecutingScript = false;
     bOneShot = FALSE;

    };

  ~CTrigger () { delete regexp; };

  public:

  CString trigger;
  CString contents;
  CString sound_to_play;
  unsigned short ignore_case;    // if true, not case-sensitive
  unsigned short colour;         // (user) colour to display in
  unsigned short  omit_from_log;  // if true, do not log triggered line

// new in version 7

  unsigned short bOmitFromOutput;  // if true, do not put triggered line in output
  unsigned short bKeepEvaluating;  // if true, keep evaluating triggers
  unsigned short bEnabled;         // if true, this trigger is enabled
  CString strLabel;                // trigger label

// new in version 8

  CString strProcedure;             // procedure to execute
  unsigned short iClipboardArg;     // if non-zero, copy matching wildcard to clipboard

// new in version 10

  unsigned short iSendTo;    // where trigger is sent to (see enum above)
  unsigned short  bRegexp;  // use regular expressions

// new in version 11

  unsigned short  bRepeat;   // repeat on same line until no more matches

// new in version 12

  unsigned short iSequence; // which order, lower is sooner
  unsigned short iMatch;    // match on colour/bold/italic? see define at start of file
  unsigned short iStyle;    // underline, italic, bold
  unsigned short bSoundIfInactive;    // only play sound if window inactive
  unsigned short bExpandVariables;    // expand variables in the trigger

// new in XML

  bool bLowercaseWildcard;    // convert wildcards to lower case (for %1 etc.)
  CString strGroup;   // group it belongs to
  CString strVariable;    // which variable to set (for send to variable)
  long   iUserOption;           // user-settable flags etc.
  COLORREF iOtherForeground;    // "other" foreground colour
  COLORREF iOtherBackground;    // "other" background colour
  unsigned short bMultiLine;    // do we do a multi-line match?
  unsigned short iLinesToMatch; // if so, on how many lines?
  unsigned short iColourChangeType; // does a colour change affect text (1), background (2) or both (0)?
                                    // see define just above?
  BOOL bOneShot;                  // if true, trigger only fires once
#ifdef PANE
  CString strPane;    // which pane to send to (for send to pane)
#endif

// computed at run-time
  
  DISPID dispid;                  // dispatch ID for calling the script
  __int64 nUpdateNumber;          // for detecting update clashes
  long  nInvocationCount;         // how many times procedure called
  long  nMatched;         // how many times the trigger fired
  vector<string> wildcards;   // matching wildcards
  t_regexp * regexp;      // compiled regular expression, if needed
  CTime tWhenMatched;     // when last matched
  bool bTemporary;        // if true, don't save it
  bool bIncluded;       // if true, don't save it
  bool bSelected;       // if true, selected for use in a plugin
  bool bExecutingScript;    // if true, executing a script and cannot be deleted
  CString strInternalName;  // name it is stored in the trigger map under
  };

// map for lookup by name
typedef CTypedPtrMap <CMapStringToPtr, CString, CTrigger*> CTriggerMap;
// array for sequencing evaluation
typedef CTypedPtrArray <CPtrArray, CTrigger*> CTriggerArray;
// list for trigger evaluation
typedef CTypedPtrList <CPtrList, CTrigger*> CTriggerList;
// map for lookup name from pointer
typedef map <CTrigger*, string> CTriggerRevMap;

/////////////////////////////////////////////////////////////////////////////
//  CTimer

class CTimer : public CObject
  {
  DECLARE_DYNAMIC(CTimer)

  public:

    enum 
    { 
      eInterval = 0,
      eAtTime,
    };

  CTimer (void)           // constructor
    {
     nInvocationCount = 0;
     nMatched = 0;
     iAtHour = 0;      
     iAtMinute = 0;    
     fAtSecond = 0;    
     iEveryHour = 0;   
     iEveryMinute = 0; 
     fEverySecond = 0.0; 
     iOffsetHour = 0;  
     iOffsetMinute = 0;
     fOffsetSecond = 0.0;
     iType = eInterval;
     bOneShot = FALSE;
     dispid = DISPID_UNKNOWN;
     tFireTime = CmcDateTime::GetTimeNow();
     bTemporary = false;
//     bSpeedWalk = FALSE;     // see iSendTo
//     bNote = FALSE;          // see iSendTo
     bActiveWhenClosed = FALSE;
     bIncluded = false;
     bSelected = false;
     iSendTo = eSendToWorld;
     iUserOption = 0;
     bOmitFromOutput = false;
     bOmitFromLog = false;
     bExecutingScript = false;

    };

  int iType;             // at or interval, see enum above
  CString strContents;               // what to send when it triggers

  int                    // timer details
      iAtHour,                  // times that happen once a day
      iAtMinute;

  double
      fAtSecond;

  int
      iEveryHour,               // periodical times
      iEveryMinute;

  double
      fEverySecond;

  int
      iOffsetHour,              // offset for periodical times
      iOffsetMinute;

  double
      fOffsetSecond;

  BOOL bEnabled;              // if true, this timer is enabled
  CString strLabel;           // timer label
  CString strProcedure;       // procedure to execute
  BOOL bOneShot;              // if true, timer only fires once
//  BOOL bSpeedWalk;            // do speed walk when timer fires
//  BOOL bNote;                 // do world.note when timer fires
  BOOL bActiveWhenClosed;     // fire when world closed

// in XML version

  CString strGroup;   // group it belongs to
  unsigned short iSendTo;    // where timer is sent to (see enum above)
  CString strVariable;    // which variable to set
  long   iUserOption;           // user-settable flags etc.
  unsigned short bOmitFromOutput;    // omit timer from output screen?
  unsigned short bOmitFromLog;    // omit timer line from log file?

// computed at run-time

  DISPID dispid;                  // dispatch ID for calling the script
  __int64 nUpdateNumber;          // for detecting update clashes
  long  nInvocationCount; // how many times procedure called
  long  nMatched;         // how many times the timer fired

// calculated field - when timer is next to go off (fire)

  CmcDateTime tFireTime;        // when to fire it
  CmcDateTime tWhenFired;     // when last reset/fired
  bool bTemporary;        // if true, don't save it
  bool bIncluded;       // if true, don't save it
  bool bSelected;       // if true, selected for use in a plugin
  bool bExecutingScript;    // if true, executing a script and cannot be deleted

  };

typedef CTypedPtrMap <CMapStringToPtr, CString, CTimer*> CTimerMap;
// map for lookup name from pointer
typedef map <CTimer*, string> CTimerRevMap;


/////////////////////////////////////////////////////////////////////////////
//  CVariable

class CVariable : public CObject
  {
  DECLARE_DYNAMIC(CVariable)

  public:

  CVariable ()
    {
    bSelected = false;
    } // end constructor

  CString strLabel;
  CString strContents;

  // computed at run-time

  __int64 nUpdateNumber;            // for detecting update clashes
  bool bSelected;       // if true, selected for use in a plugin

  };

typedef CTypedPtrMap <CMapStringToPtr, CString, CVariable*> CVariableMap;
typedef CTypedPtrArray <CPtrArray, CVariable*> CVariableArray;

/////////////////////////////////////////////////////////////////////////////
//  CMud - not used?

class CMud : public CObject
  {
  DECLARE_DYNAMIC(CMud)

  CMud (void)          // constructor
  {
  };

  public:

  CString strName;
  CString strAddress;
  long    nPort;
  CString strType;
  CString strComment;

  };

typedef CTypedPtrMap <CMapStringToPtr, CString, CMud*> CMudMap;


/////////////////////////////////////////////////////////////////////////////
//  CArgument - these are arguments to an MXP tag


class CArgument :public CObject
  {

  public:

  CString strName;    // name of argument, eg. color
  CString strValue;   // value of argument, eg. red
  int     iPosition;  // where in argument list it is (first is 1)
  bool    bKeyword;   // true if a keyword (eg. OPEN, EMPTY)
  bool    bUsed;      // true if used

  CArgument (const CString strSourceName,
             const CString strSourceValue,
             const int iSourcePosition)
    {
    strName = strSourceName;
    strValue = strSourceValue;
    iPosition = iSourcePosition;
    bKeyword = false;
    bUsed = false;
    };  // constructor

  CArgument () 
    { 
    iPosition = 0; 
    bKeyword = false; 
    bUsed = false;
    };  // constructor

  };

typedef CTypedPtrList <CPtrList, CArgument*> CArgumentList;


/////////////////////////////////////////////////////////////////////////////
//  CAtomicElement - these are atomic MXP tags that we recognise, eg. <b> 


class CAtomicElement : public CObject
  {

  public:

  CString strName;  // tag name, eg. "bold"
  CString strArgs;  // supported arguments, eg. href,hint
  int     iFlags;   // see defines in mxp.h - secure, command flags
  int     iAction;  // its action, eg. MXP_ACTION_BOLD

  };

typedef CTypedPtrMap <CMapStringToPtr, CString, CAtomicElement*> CAtomicElementMap;

/////////////////////////////////////////////////////////////////////////////
//  CElementItem - these are arguments to an MXP tag


class CElementItem :public CObject
  {

  public:

  CAtomicElement * pAtomicElement;  // pointer to appropriate atomic element
  CArgumentList  ArgumentList;      // list of arguments to this element item

  CElementItem () 
    { 
    pAtomicElement = NULL; 
    }; // constructor

  ~CElementItem () 
    { 
    DELETE_LIST (ArgumentList); 
    }; // destructor

  };

typedef CTypedPtrList <CPtrList, CElementItem*> CElementItemList;


/////////////////////////////////////////////////////////////////////////////
//  CElement - these are user-defined MXP tags that we recognise, eg. <boldcolour>
//  eg. <!ELEMENT boldtext '<COLOR &col;><B>' ATT='col=red'>

class CElement : public CObject
  {

  public:

  CString strName;                  // tag name
  CElementItemList ElementItemList; // what atomic elements it defines  (arg 1)
  CArgumentList    AttributeList;    // list of attributes to this element (ATT="xx")
  int iTag;                         // line tag number (20 - 99)  (TAG=n)
  CString strFlag;                  // which variable to set      (SET x)
  bool bOpen;                       // true if element is open    (OPEN)
  bool bCommand;                    // true if no closing tag     (EMPTY)

  CElement () 
    {
    iTag = 0; 
    bOpen = bCommand = false; 
    };    // constructor

  ~CElement () 
    { 
    DELETE_LIST (ElementItemList);
    DELETE_LIST (AttributeList);
    };  // destructor

  };

typedef CTypedPtrMap <CMapStringToPtr, CString, CElement*> CElementMap;

/////////////////////////////////////////////////////////////////////////////
//  CColours - these are colours that we know about


class CColours : public CObject
  {

  public:

  CString strName;     // Colour name
  COLORREF iColour;    // what its RGB code is

  };

typedef CTypedPtrMap <CMapStringToPtr, CString, CColours*> CColoursMap;



/////////////////////////////////////////////////////////////////////////////
//  CActiveTag - these are outstanding (unclosed) tags


class CActiveTag :public CObject
  {

  public:

  CString strName;    // name of tag we opened
  bool    bSecure;    // was it secure mode at the time?  
  bool    bNoReset;   // protected from reset?

  CActiveTag () 
    { 
    bSecure = false;
    bNoReset = false;
    }; // constructor


  };

typedef CTypedPtrList <CPtrList, CActiveTag*> CActiveTagList;


/////////////////////////////////////////////////////////////////////////////
//  CPlugin - these are world plugins

class CScriptEngine;

class CPlugin :public CObject
  {

  public:

  CString m_strName;          // name of plugin
  CString m_strAuthor;        // who wrote it
  CString m_strPurpose;       // what it does (short description)
  CString m_strDescription;   // what it does (long description)
  CString m_strScript;        // script source  (ie. from <script> tags)
  CString m_strLanguage;      // script language (eg. vbscript)
  CString m_strSource;        // include file that contains this plugin
  CString m_strDirectory;     // directory source is in (m_strSource minus the actual filename)
  CString m_strID;            // unique ID
  CTime   m_tDateWritten;     // date written
  CTime   m_tDateModified;    // date last modified
  double  m_dVersion;         // plugin version
  double  m_dRequiredVersion; // minimum MUSHclient version required
  CTime   m_tDateInstalled;   // date installed

  CScriptEngine * m_ScriptEngine; // script engine for script, if any

  CAliasMap     m_AliasMap;     // aliases     
  CAliasArray   m_AliasArray;   // array of aliases for sequencing
  CAliasRevMap  m_AliasRevMap;   // for getting name back from pointer
  CTriggerMap   m_TriggerMap;   // triggers    
  CTriggerArray m_TriggerArray; // array of triggers for sequencing
  CTriggerRevMap m_TriggerRevMap; // for getting name back from pointer
  CTimerMap     m_TimerMap;     // timers      
  CTimerRevMap  m_TimerRevMap;   // for getting name back from pointer
  CVariableMap  m_VariableMap;  // variables   
  tStringMapOfMaps m_Arrays;    // map of arrays (for scripting)

  bool m_bEnabled;              // true if active (enabled)
  CMUSHclientDoc    * m_pDoc;   // related MUSHclient document
  bool m_bSaveState;            // true to save plugin state
  bool m_bSendToScriptUsed;     // plugin sends to script
  bool m_bGlobal;               // true if plugin was loaded from global prefs
  long m_iLoadOrder;            // sequence in which plugins are processed

  // Lua note - for Lua the DISPID is a flag indicating whether or not
  // the routine exists. It is set to DISPID_UNKNOWN if the last call caused an error
  // It will be 1 if the routine exists, and DISPID_UNKNOWN if it doesn't.

  // WARNING! PHP currently uses a DISPID of zero, so that can't be used as a "not found" flag.

  DISPID m_dispid_plugin_install      ;  //  "OnPluginInstall"    
  DISPID m_dispid_plugin_connect      ;  //  "OnPluginConnect"    
  DISPID m_dispid_plugin_disconnect   ;  //  "OnPluginDisconnect" 
  DISPID m_dispid_plugin_close        ;  //  "OnPluginClose"      
  DISPID m_dispid_plugin_save_state   ;  //  "OnPluginSaveState"  
  DISPID m_dispid_plugin_enable       ;  //  "OnPluginEnable"     
  DISPID m_dispid_plugin_disable      ;  //  "OnPluginDisable"    
  DISPID m_dispid_plugin_command      ;  //  "OnPluginCommand" 
  DISPID m_dispid_plugin_command_entered;  //  "OnPluginCommandEntered" 
  DISPID m_dispid_plugin_get_focus    ;  //  "OnPluginGetFocus"    
  DISPID m_dispid_plugin_lose_focus   ;  //  "OnPluginLoseFocus" 
  DISPID m_dispid_plugin_trace        ;  //  "OnPluginTrace" 
  DISPID m_dispid_plugin_broadcast    ;  //  "OnPluginBroadcast" 
  DISPID m_dispid_plugin_screendraw   ;  //  "OnPluginScreendraw" 
  DISPID m_dispid_plugin_playsound    ;  //  "OnPluginPlaySound" 
  DISPID m_dispid_plugin_tabcomplete  ;  //  "OnPluginTabComplete" 
 // DISPID m_dispid_plugin_tooltip    ;  //  "OnPluginToolTip" 
  DISPID m_dispid_plugin_list_changed ;  //  "OnPluginListChanged"    
  DISPID m_dispid_plugin_tick         ;  //  "OnPluginTick" 
  DISPID m_dispid_plugin_mouse_moved  ;  //  "OnPluginMouseMoved" 
  
  DISPID m_dispid_plugin_send         ;  //  "OnPluginSend"     
  DISPID m_dispid_plugin_sent         ;  //  "OnPluginSent"     
  DISPID m_dispid_plugin_line_received;  //  "OnPluginLineReceived"    
  DISPID m_dispid_plugin_packet_received;  //  "OnPluginPacketReceived" 
  DISPID m_dispid_plugin_partial_line;     //  "OnPluginPartialLine" 
  DISPID m_dispid_plugin_telnet_option;    //  "OnPluginTelnetOption" 
  DISPID m_dispid_plugin_telnet_request;   //  "OnPluginTelnetRequest" 
  DISPID m_dispid_plugin_telnet_subnegotiation;   //  "OnPluginTelnetSubnegotiation" 
  DISPID m_dispid_plugin_IAC_GA;           //  "OnPlugin_IAC_GA" 
  DISPID m_dispid_plugin_on_world_output_resized; // "OnPluginWorldOutputResized"
  DISPID m_dispid_plugin_on_command_changed;  // "OnPluginCommandChanged"
  
	DISPID m_dispid_plugin_OnMXP_Start;       // "OnPluginMXPstart"
	DISPID m_dispid_plugin_OnMXP_Stop;        // "OnPluginMXPstop"
	DISPID m_dispid_plugin_OnMXP_OpenTag;     // "OnPluginMXPopenTag"        
  DISPID m_dispid_plugin_OnMXP_CloseTag;    // "OnPluginMXPcloseTag"
	DISPID m_dispid_plugin_OnMXP_SetVariable; // "OnPluginMXPsetVariable"
	DISPID m_dispid_plugin_OnMXP_SetEntity;   // "OnPluginMXPsetEntity"
	DISPID m_dispid_plugin_OnMXP_Error;       // "OnPluginMXPerror"

  DISPID m_dispid_plugin_On_Chat_Accept;    // "OnPluginChatAccept"
	DISPID m_dispid_plugin_On_Chat_Message;   // "OnPluginChatMessage"
	DISPID m_dispid_plugin_On_Chat_MessageOut;// "OnPluginChatMessageOut"
	DISPID m_dispid_plugin_On_Chat_Display;   // "OnPluginChatDisplay"
	DISPID m_dispid_plugin_On_Chat_NewUser;   // "OnPluginChatNewUser"
	DISPID m_dispid_plugin_On_Chat_UserDisconnect;  // "OnPluginChatUserDisconnect"
  // methods

  CPlugin (CMUSHclientDoc * pDoc);  // constructor
  ~CPlugin (); // destructor
  bool SaveState (void);
  DISPID GetPluginDispid (const char * sName);
  void ExecutePluginScript (const char * sName, 
                            DISPID & iRoutine);   // no arguments
  bool ExecutePluginScript (const char * sName, 
                            DISPID & iRoutine, 
                            const char * sText);  // 1 argument
  bool ExecutePluginScript (const char * sName, 
                            DISPID & iRoutine, 
                            const long arg1,      // 2 arguments
                            const string sText);
  bool ExecutePluginScript (const char * sName, 
                            DISPID & iRoutine, 
                            const long arg1,      // 3 arguments
                            const long arg2,
                            const string sText);
  bool ExecutePluginScript (const char * sName, 
                            DISPID & iRoutine, 
                            const long arg1,      // 1 number, 3 strings
                            const char * arg2,
                            const char * arg3,
                            const char * arg4);
  void ExecutePluginScript (const char * sName,
                            CString & strResult,  // taking and returning a string
                            DISPID & iRoutine); 

  };

typedef CTypedPtrList <CPtrList, CPlugin*> CPluginList;

// for storing map directions, and inverses of them
class CMapDirection
  {
  public:
    // constructor
    CMapDirection (const string sDirectionToLog,
                   const string sDirectionToSend,
                   const string sReverseDirection)
                   :   m_sDirectionToLog (sDirectionToLog), 
                       m_sDirectionToSend (sDirectionToSend),
                       m_sReverseDirection (sReverseDirection) {};
    // default constructor
    CMapDirection () {};

    // copy constructor
    CMapDirection (const CMapDirection & m)
                    : m_sDirectionToLog (m.m_sDirectionToLog), 
                       m_sDirectionToSend (m.m_sDirectionToSend),
                       m_sReverseDirection (m.m_sReverseDirection) {};

    // operator =
    const CMapDirection & operator= (const CMapDirection & rhs)
      {
      m_sDirectionToLog = rhs.m_sDirectionToLog;
      m_sDirectionToSend = rhs.m_sDirectionToSend;
      m_sReverseDirection = rhs.m_sReverseDirection;
      return *this;
      };

    string m_sDirectionToLog;     // eg. "up" becomes "u"
    string m_sDirectionToSend;    // eg. "u" becomes "up"
    string m_sReverseDirection;   // eg. "e" becomes "w"
  };  // end of class CMapDirection

typedef map<string, CMapDirection> CMapDirectionsMap;
typedef map<string, CMapDirection>::const_iterator MapDirectionsIterator;

extern CMapDirectionsMap MapDirectionsMap;

/*
// for panes

class CPaneView;

class CPane
  {
  public:

  CFrameWnd * m_pFrame;   // frame holding view
  CPaneView * m_pView;    // the view belonging to the document

  string  m_sTitle;
  long    m_iLeft;  
  long    m_iTop;  
  long    m_iWidth;
  long    m_iHeight;
  long    m_iFlags;
  
  };

  */

//typedef map<string, CPane *>::iterator PaneMapIterator;

// ----------- here used for Lua in choosing from combo-box

class CKeyValuePair
  {

  public:
    CKeyValuePair (const string sValue) : 
          bNumber_ (false), iKey_ (0.0), sValue_ (sValue) { };  // constructor

  bool   bNumber_; // true if key a number, false if a string

  string sKey_;    // key if string
  double iKey_;    // key if number?

  string sValue_;  // value 

  };   // end of class  CStringValuePair


// for imbedded windows

// fonts
typedef map<string, CFont *> FontMap;
typedef FontMap::iterator FontMapIterator;

// images
typedef map<string, CBitmap *> ImageMap;
typedef ImageMap::iterator ImageMapIterator;


// hotspots
class CHotspot
  {

  public:

  CHotspot () : m_Cursor (0),  m_Flags (0), m_DragFlags (0),
                m_dispid_MouseOver        (DISPID_UNKNOWN),
                m_dispid_CancelMouseOver  (DISPID_UNKNOWN),
                m_dispid_MouseDown        (DISPID_UNKNOWN),
                m_dispid_CancelMouseDown  (DISPID_UNKNOWN),
                m_dispid_MouseUp          (DISPID_UNKNOWN),
                m_dispid_MoveCallback     (DISPID_UNKNOWN),
                m_dispid_ReleaseCallback  (DISPID_UNKNOWN)
                {}  // constructor

  CRect  m_rect;           // where it is

  string m_sMouseOver;       // function to call on mouseover
  string m_sCancelMouseOver; // function to call when mouse moves away or is clicked

  string m_sMouseDown;       // mouse down here  (might cancel mouseover first)
  string m_sCancelMouseDown; // they let go somewhere else
  string m_sMouseUp;         // mouse up following a mouse-down in this hotspot

  string m_sTooltipText;     // tooltip text

  long   m_Cursor;           // what cursor to show
  long   m_Flags;            // flags

  string m_sMoveCallback;    // callback when mouse moves
  string m_sReleaseCallback; // callback when mouse released
  long   m_DragFlags;        // drag-and-drop flags

  // dispids for calling functions from NOT in a plugin (ignored in a plugin)
  DISPID m_dispid_MouseOver;       // function to call on mouseover
  DISPID m_dispid_CancelMouseOver; // function to call when mouse moves away or is clicked
  DISPID m_dispid_MouseDown;       // mouse down here  (might cancel mouseover first)
  DISPID m_dispid_CancelMouseDown; // they let go somewhere else
  DISPID m_dispid_MouseUp;         // mouse up following a mouse-down in this hotspot
  DISPID m_dispid_MoveCallback;    // callback when mouse moves
  DISPID m_dispid_ReleaseCallback; // callback when mouse released

  };   // end of class  CStringValuePair


typedef map<string, CHotspot *> HotspotMap;
typedef HotspotMap::iterator HotspotMapIterator;

// flags

#define MINIWINDOW_DRAW_UNDERNEATH 0x01        // draw underneath scrolling text
#define MINIWINDOW_ABSOLUTE_LOCATION 0x02      // use m_iLeft, m_iTop to position 
#define MINIWINDOW_TRANSPARENT 0x04            // draw transparently, using m_iBackgroundColour 
#define MINIWINDOW_IGNORE_MOUSE 0x08           // ignore mouse-down, mouse-over etc. 

class CMiniWindow
  {
  private:

    CDC dc;                   // our offscreen device context
  	CBitmap *   m_oldBitmap;  // bitmap originally found in CMyMemDC
    CBitmap     m_Bitmap;     // where it all happens
    FontMap     m_Fonts;      // all the fonts they want
    ImageMap    m_Images;     // other images they may want to blt onto the window

    long    m_iWidth;   
    long    m_iHeight;

    CPoint  m_Location;   // left, top position (if absolute position)
    short   m_iPosition;  // where to position it
    long    m_iFlags;     // see defines above
    COLORREF m_iBackgroundColour;

    bool    m_bShow;  // true = draw it


  public:
    CMiniWindow ();   // constructor

    ~CMiniWindow ();  // destructor


  CRect   m_rect;   // where we actually put it
  bool m_bTemporarilyHide;   // no room right now
  HotspotMap  m_Hotspots;   // where we can click with the mouse

  // where we last did things:
  CPoint m_last_mouseposition;
  long   m_last_mouse_update;

  CPoint m_client_mouseposition;

  CTime   m_tDateInstalled;   // date installed

  string  m_sMouseOverHotspot;    // last hotspot we moused over
  string  m_sMouseDownHotspot;    // last hotspot we mouse clicked in

  string  m_sCallbackPlugin;      // plugin we are using

  const long GetWidth ()      const { return m_iWidth; }
  const long GetHeight ()     const { return m_iHeight; }
  const short GetPosition ()  const { return m_iPosition; }
  const bool GetShow ()       const { return m_bShow; }
  const long GetFlags ()      const { return m_iFlags; }
  const CPoint GetLocation () const { return m_Location; }
  const COLORREF GetBackgroundColour () const { return m_iBackgroundColour; }

  void  Create (long Left, long Top, long Width, long Height,
                short Position, long Flags, 
                COLORREF iBackgroundColour);

  void  Show (bool bShow);
  
  CDC*  GetDC (void) { return &dc; }

  long FixRight (const long Right);
  long FixBottom (const long Bottom);

  void Info (long InfoType, VARIANT & vaResult);

  long  RectOp (short Action, long Left, 
                long Top, long Right, long Bottom, 
                long Colour1, long Colour2);

  long  CircleOp (short Action, 
                  long Left, long Top, long Right, long Bottom, 
                  long PenColour, long PenStyle, long PenWidth, 
                  long BrushColour, long BrushStyle,
                  long Extra1, long Extra2, long Extra3, long Extra4);

  long Font (LPCTSTR FontId, LPCTSTR FontName, 
             double Size, 
             BOOL Bold, BOOL Italic, BOOL Underline, BOOL Strikeout, 
             short Charset, short PitchAndFamily);

  void FontInfo (LPCTSTR FontId, long InfoType, VARIANT & vaResult);
  void FontList (VARIANT & vaResult);

  long Text (LPCTSTR FontId,  // which previously-created font
             LPCTSTR Text,   // what to say
             long Left, long Top, long Right, long Bottom, // where to put it
             long Colour,       // colour to show it in
             BOOL Unicode);      // true if UTF8

  long TextWidth (LPCTSTR FontId,  // which previously-created font
                   LPCTSTR Text,   // what to measure
                   BOOL Unicode);  // true if UTF8

  long Line (long x1, long y1, long x2, long y2, 
             long PenColour, long PenStyle, long PenWidth);
    
  long Arc (long Left, long Top, long Right, long Bottom, 
            long x1, long y1, 
            long x2, long y2, 
            long PenColour, long PenStyle, long PenWidth);

  long LoadImage (LPCTSTR ImageId, LPCTSTR FileName);
  long LoadPngImage (LPCTSTR ImageId, LPCTSTR FileName);

  long Write (LPCTSTR FileName);
  long WritePng (LPCTSTR FileName, const BITMAPINFO * bmi, unsigned char * pData);

  long DrawImage(LPCTSTR ImageId, 
                 long Left, long Top, long Right, long Bottom, 
                 short Mode,
                 long SrcLeft, long SrcTop, long SrcRight, long SrcBottom);
  
  void ImageList(VARIANT & vaResult);

  void ImageInfo(LPCTSTR ImageId, long InfoType, VARIANT & vaResult);

  long Bezier(LPCTSTR Points, long PenColour, long PenStyle, long PenWidth);

  long Polygon(LPCTSTR Points, 
               long PenColour, short PenStyle, long PenWidth, 
               long BrushColour, long BrushStyle, 
               BOOL Close,
               BOOL Winding);

  long Position(long Left, long Top, 
                short Position, 
                long Flags); 


  long AddHotspot(CMUSHclientDoc * pDoc,
                   LPCTSTR HotspotId, 
                   string sPluginID,
                   long Left, long Top, long Right, long Bottom, 
                   LPCTSTR MouseOver, 
                   LPCTSTR CancelMouseOver, 
                   LPCTSTR MouseDown, 
                   LPCTSTR CancelMouseDown, 
                   LPCTSTR MouseUp, 
                   LPCTSTR TooltipText,
                   long Cursor, 
                   long Flags);

  long DeleteHotspot(LPCTSTR HotspotId);
  void HotspotList(VARIANT & vaResult);
  long DeleteAllHotspots();

  void HotspotInfo(LPCTSTR HotspotId, 
                  long InfoType, VARIANT & vaResult);

  long ImageOp(short Action, 
              long Left, long Top, long Right, long Bottom, 
              long PenColour, long PenStyle, long PenWidth, 
              long BrushColour, LPCTSTR ImageId, 
              long EllipseWidth, long EllipseHeight);

  long CreateImage(LPCTSTR ImageId, 
                   long Row1, long Row2, long Row3, long Row4, long Row5, long Row6, long Row7, long Row8);

  long BlendImage(LPCTSTR ImageId, 
                  long Left, long Top, long Right, long Bottom, 
                  short Mode, double Opacity, 
                  long SrcLeft, long SrcTop, long SrcRight, long SrcBottom);

  long ImageFromWindow(LPCTSTR ImageId, CMiniWindow * pSrcWindow);

  long Gradient(long Left, long Top, long Right, long Bottom, 
                      long StartColour, long EndColour, 
                      short Mode); 

  long Filter(long Left, long Top, long Right, long Bottom, 
                    short Operation, double Options);

  long SetPixel(long x, long y, long Colour);

  long GetPixel(long x, long y);

  long MergeImageAlpha(LPCTSTR ImageId, LPCTSTR MaskId, 
                      long Left, long Top, long Right, long Bottom, 
                      short Mode, double Opacity, 
                      long SrcLeft, long SrcTop, long SrcRight, long SrcBottom);

  CString Menu(long Left, long Top, LPCTSTR Items, CMUSHView* pView);


  long DragHandler(CMUSHclientDoc * pDoc, 
                   LPCTSTR HotspotId, 
                   string sPluginID,
                   LPCTSTR MoveCallback, 
                   LPCTSTR ReleaseCallback, 
                   long Flags);

  long HotspotTooltip(LPCTSTR HotspotId, 
                  LPCTSTR TooltipText);

  long LoadImageMemory(LPCTSTR ImageId, 
                       unsigned char * Buffer, 
                       const size_t Length,
                       const bool bAlpha);


  long DrawImageAlpha(LPCTSTR ImageId, 
                      long Left, long Top, long Right, long Bottom, 
                      double Opacity, 
                      long SrcLeft, long SrcTop); 

  long GetImageAlpha(LPCTSTR ImageId, 
                      long Left, long Top, long Right, long Bottom, 
                      long SrcLeft, long SrcTop); 

  };

typedef map<string, CMiniWindow *> MiniWindowMap;

typedef MiniWindowMap::iterator MiniWindowMapIterator;
