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
#define STRIKEOUT  0x0020   // strike-though? (strikeout)
#define COLOURTYPE 0x0300   // type of colour in iForeColour/iBackColour, see above
#define ACTIONTYPE 0x0C00   // action type, see above
#define STYLE_BITS 0x0FFF   // everything except START_TAG
#define START_TAG  0x1000   // strAction is tag name - start of tag (eg. <b> )
#define TEXT_STYLE 0x002F   // bold, underline, italic, inverse, strike-through flags

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
  char * text;          // allocated as necessary and then resized
  CStyleList styleList; // list of styles applying to text, see above
  CTime m_theTime;      // time this line arrived
  LARGE_INTEGER m_lineHighPerformanceTime;  
  int iMemoryAllocated; // size of buffer allocated for "text"

  long m_nLineNumber;
  short m_iPreambleOffset;  // how far in the preamble took us

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

// when adding new fields, update operator==
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

  bool operator== (const CAlias & rhs) const;

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


// when adding new fields, update operator==
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

  bool operator== (const CTrigger & rhs) const;

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

// when adding new fields, update operator==
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

  bool operator== (const CTimer & rhs) const;

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

  unsigned long nCreateSequence;  // for keeping timers in sequence of creation

// calculated field - when timer is next to go off (fire)

  CmcDateTime tFireTime;        // when to fire it
  CmcDateTime tWhenFired;     // when last reset/fired
  bool bTemporary;        // if true, don't save it
  bool bIncluded;       // if true, don't save it
  bool bSelected;       // if true, selected for use in a plugin
  bool bExecutingScript;    // if true, executing a script and cannot be deleted

  static unsigned long GetNextTimerSequence () { return nNextCreateSequence++; }

  private:
    static unsigned long nNextCreateSequence;

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
typedef CMapDirectionsMap::const_iterator MapDirectionsIterator;

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

