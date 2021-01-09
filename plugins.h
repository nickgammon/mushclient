/////////////////////////////////////////////////////////////////////////////
//  CPlugin - these are world plugins

#define DEFAULT_PLUGIN_SEQUENCE 5000

// for storing dispatch IDs, and how many times each one is called
// we need a copy constructor and operator =, so it can be moved 
// around in the STL map it is stored in

class CScriptDispatchID
  {
  public:
    // constructor
    CScriptDispatchID (DISPID dispid) : 
                      _dispid (dispid), 
                      _count (0) {};

    // default constructor
    CScriptDispatchID () : 
                      _dispid (DISPID_UNKNOWN), 
                      _count (0) {};

    // copy constructor
    CScriptDispatchID (const CScriptDispatchID & d)
                    : _dispid (d._dispid), 
                      _count (d._count)
                       {};

    // operator =
    const CScriptDispatchID & operator= (const CScriptDispatchID & rhs)
      {
      _dispid = rhs._dispid;
      _count  = rhs._count;
      return *this;
      };

    // returns true if DISPID is valid
    inline bool isvalid () const { return _dispid != DISPID_UNKNOWN; };

    DISPID _dispid;   // the dispatch ID from the COM engine, or  DISPID_UNKNOWN
    __int64 _count;   // count of attempts to call it (if not DISPID_UNKNOWN)
  };  // end of class CScriptDispatchID

typedef map<const string, CScriptDispatchID> CScriptDispatchIDsMap;
typedef CScriptDispatchIDsMap::const_iterator CScriptDispatchIDIterator;

// when we call a script (with ExecutePluginScript) we pass down one of these. It has
// 1. the name (for error messages and for Lua)
// 2. the dispatch ID (from CScriptDispatchID) - to help call the function
// 3. the count of invocations (passed by reference so it can be updated)

class CScriptCallInfo
  {
  public:

  CScriptCallInfo (const string name, CScriptDispatchID & dispid_info)
    :  _dispid_info (dispid_info), _name (name) {};

  CScriptDispatchID & _dispid_info;
  const string _name;

  }; // end of class CScriptCallInfo

class CScriptEngine;

// world plugins
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
  CString m_strCallingPluginID; // during a CallPlugin - the ID of the calling plugin
  short m_iSequence; // evaluation order, lower is sooner
                     // negative sequences get evaluated before the main world triggers/aliases

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
  LONGLONG m_iScriptTimeTaken;  // time taken to execute scripts
  bool m_bSavingStateNow;       // to prevent infinite loops

  // Lua note - for Lua the DISPID is a flag indicating whether or not
  // the routine exists. It is set to DISPID_UNKNOWN if the last call caused an error
  // It will be 1 if the routine exists, and DISPID_UNKNOWN if it doesn't.

  // WARNING! PHP currently uses a DISPID of zero, so that can't be used as a "not found" flag.

  CScriptDispatchIDsMap m_PluginCallbacks;  // maps plugin callback names to their DISPIDs

  // methods

  CPlugin (CMUSHclientDoc * pDoc);  // constructor
  ~CPlugin (); // destructor
  bool SaveState (const bool bScripted = false);
  DISPID GetPluginDispid (const char * sName);
  void ExecutePluginScript (CScriptCallInfo & callinfo);   // no arguments
  bool ExecutePluginScript (CScriptCallInfo & callinfo, 
                            const char * sText);  // 1 argument
  bool ExecutePluginScript (CScriptCallInfo & callinfo, 
                            const long arg1,      // 2 arguments
                            const string sText);
  bool ExecutePluginScript (CScriptCallInfo & callinfo, 
                            const long arg1,      // 3 arguments
                            const long arg2,
                            const string sText);
  bool ExecutePluginScript (CScriptCallInfo & callinfo, 
                            const long arg1,      // 1 number, 3 strings
                            const char * arg2,
                            const char * arg3,
                            const char * arg4);
  void ExecutePluginScriptRtn (CScriptCallInfo & callinfo,
                            CString & strResult);  // taking and returning a string

  };

typedef list<CPlugin*> CPluginList;
typedef CPluginList::iterator PluginListIterator;

// Unary predicate for use in find_if to find a plugin by name
//  ... not case-sensitive
struct compare_plugin_name : binary_function <CPlugin *, CString, bool>
  {
  bool operator() (const CPlugin * p, const CString name) const
    { return p->m_strName.CompareNoCase (name) == 0; };
  };  // end of struct compare_plugin_name

// Unary predicate for use in find_if to find a plugin by plugin ID
//  ... not case-sensitive
struct compare_plugin_id : binary_function <CPlugin *, CString, bool>
  {
  bool operator() (const CPlugin * p, const CString id) const
    { return p->m_strID.CompareNoCase (id) == 0; };
  };  // end of struct compare_plugin_id

// plugin callback routines - start with OnPlugin so that we can advise
// users not to use that string for their own routines

// install / remove                                 
extern const string ON_PLUGIN_INSTALL;              //  "OnPluginInstall"              
extern const string ON_PLUGIN_CLOSE;                //  "OnPluginClose"                
extern const string ON_PLUGIN_LIST_CHANGED;         //  "OnPluginListChanged"          
                                                                                      
// connect / disconnect                                                               
extern const string ON_PLUGIN_CONNECT;              //  "OnPluginConnect"              
extern const string ON_PLUGIN_DISCONNECT;           //  "OnPluginDisconnect"           
                                                                                      
// saving                                                                             
extern const string ON_PLUGIN_SAVE_STATE;           //  "OnPluginSaveState"            
extern const string ON_PLUGIN_WORLD_SAVE;           //  "OnPluginWorldSave"            
                                                                                      
// enable / disable                                                                   
extern const string ON_PLUGIN_ENABLE;               //  "OnPluginEnable"               
extern const string ON_PLUGIN_DISABLE;              //  "OnPluginDisable"              
                                                                                      
// the focus                                                                          
extern const string ON_PLUGIN_GETFOCUS;             //  "OnPluginGetFocus"             
extern const string ON_PLUGIN_LOSEFOCUS;            //  "OnPluginLoseFocus"            
                                                                                      
// capture stuff                                                                      
extern const string ON_PLUGIN_TRACE;                //  "OnPluginTrace"                
extern const string ON_PLUGIN_PACKET_DEBUG;         //  "OnPluginPacketDebug"                
extern const string ON_PLUGIN_BROADCAST;            //  "OnPluginBroadcast"            
extern const string ON_PLUGIN_SCREENDRAW;           //  "OnPluginScreendraw"           
extern const string ON_PLUGIN_SELECTION_CHANGED;    //  "OnPluginSelectionChanged"     
                                                                                      
// sounds                                                                             
extern const string ON_PLUGIN_PLAYSOUND;            //  "OnPluginPlaySound"            
                                                                                      
// stuff received/sent                                                                
extern const string ON_PLUGIN_SEND;                 //  "OnPluginSend"                 
extern const string ON_PLUGIN_SENT;                 //  "OnPluginSent"                 
extern const string ON_PLUGIN_PARTIAL_LINE;         //  "OnPluginPartialLine"          
extern const string ON_PLUGIN_LINE_RECEIVED;        //  "OnPluginLineReceived"         
extern const string ON_PLUGIN_PACKET_RECEIVED;      //  "OnPluginPacketReceived"       
                                                                                      
// telnet negotiation                                                                 
extern const string ON_PLUGIN_TELNET_OPTION;        //  "OnPluginTelnetOption"         
extern const string ON_PLUGIN_TELNET_REQUEST;       //  "OnPluginTelnetRequest"        
extern const string ON_PLUGIN_TELNET_SUBNEGOTIATION;//  "OnPluginTelnetSubnegotiation"
extern const string ON_PLUGIN_IAC_GA;               //  "OnPlugin_IAC_GA"              
                                                                                      
// commands                                                                           
extern const string ON_PLUGIN_COMMAND;              //  "OnPluginCommand"              
extern const string ON_PLUGIN_COMMAND_ENTERED;      //  "OnPluginCommandEntered"       
extern const string ON_PLUGIN_COMMAND_CHANGED;      //  "OnPluginCommandChanged"       
extern const string ON_PLUGIN_TABCOMPLETE;          //  "OnPluginTabComplete"          
                                                                                      
// resizing, ticking, moving, rhythm                                                  
extern const string ON_PLUGIN_WORLD_OUTPUT_RESIZED; //  "OnPluginWorldOutputResized"   
extern const string ON_PLUGIN_TICK;                 //  "OnPluginTick"                 
extern const string ON_PLUGIN_MOUSE_MOVED;          //  "OnPluginMouseMoved"           
                                                                                      
// MXP stuff                                                                          
extern const string ON_PLUGIN_MXP_START;            //  "OnPluginMXPstart"             
extern const string ON_PLUGIN_MXP_STOP;             //  "OnPluginMXPstop"              
extern const string ON_PLUGIN_MXP_OPENTAG;          //  "OnPluginMXPopenTag"           
extern const string ON_PLUGIN_MXP_CLOSETAG;         //  "OnPluginMXPcloseTag"          
extern const string ON_PLUGIN_MXP_SETVARIABLE;      //  "OnPluginMXPsetVariable"       
extern const string ON_PLUGIN_MXP_SETENTITY;        //  "OnPluginMXPsetEntity"         
extern const string ON_PLUGIN_MXP_ERROR;            //  "OnPluginMXPerror"             
                                                                                      
// chat stuff                                                                         
extern const string ON_PLUGIN_CHAT_ACCEPT;          //  "OnPluginChatAccept"           
extern const string ON_PLUGIN_CHAT_MESSAGE;         //  "OnPluginChatMessage"          
extern const string ON_PLUGIN_CHAT_MESSAGE_OUT;     //  "OnPluginChatMessageOut"       
extern const string ON_PLUGIN_CHAT_DISPLAY;         //  "OnPluginChatDisplay"          
extern const string ON_PLUGIN_CHAT_NEWUSER;         //  "OnPluginChatNewUser"          
extern const string ON_PLUGIN_CHAT_USERDISCONNECT;  //  "OnPluginChatUserDisconnect"   

// drawing
extern const string ON_PLUGIN_DRAW_OUTPUT_WINDOW;   //  "OnPluginDrawOutputWindow"

// table of callbacks
extern string PluginCallbacksNames [];

// for plugin callbacks
typedef struct
  {
  char *  pName;      // name, eg. "OnPluginListChanged"
  int     iOffset;    // offset in CPlugin
  }  tPluginCallbackNames;