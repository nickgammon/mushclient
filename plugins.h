/////////////////////////////////////////////////////////////////////////////
//  CPlugin - these are world plugins

// for storing dispatch IDs, and how many times each one is called
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
typedef map<const string, CScriptDispatchID>::const_iterator CScriptDispatchIDIterator;

class CScriptCallInfo
  {
  public:

  CScriptCallInfo (const string name, CScriptDispatchID & dispid_info)
    :  _dispid_info (dispid_info), _name (name) {};

  CScriptDispatchID & _dispid_info;
  const string _name;

  }; // end of class CScriptCallInfo

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
  CString m_strCallingPluginID; // during a CallPlugin - the ID of the calling plugin

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

typedef CTypedPtrList <CPtrList, CPlugin*> CPluginList;

// plugin callback routines - start with OnPlugin so that we can advise
// users not to use that string for their own routines

// install / remove
extern const string ON_PLUGIN_INSTALL;
extern const string ON_PLUGIN_CLOSE;
extern const string ON_PLUGIN_LIST_CHANGED;

// connect / disconnect
extern const string ON_PLUGIN_CONNECT;
extern const string ON_PLUGIN_DISCONNECT;

// saving
extern const string ON_PLUGIN_SAVE_STATE;
extern const string ON_PLUGIN_WORLD_SAVE;

// enable / disable
extern const string ON_PLUGIN_ENABLE;
extern const string ON_PLUGIN_DISABLE;

// the focus                                 
extern const string ON_PLUGIN_GETFOCUS;
extern const string ON_PLUGIN_LOSEFOCUS;

// capture stuff
extern const string ON_PLUGIN_TRACE;
extern const string ON_PLUGIN_BROADCAST;
extern const string ON_PLUGIN_SCREENDRAW;

// sounds
extern const string ON_PLUGIN_PLAYSOUND;

// stuff received/sent                
extern const string ON_PLUGIN_SEND;
extern const string ON_PLUGIN_SENT;
extern const string ON_PLUGIN_PARTIAL_LINE;
extern const string ON_PLUGIN_LINE_RECEIVED;
extern const string ON_PLUGIN_PACKET_RECEIVED;

// telnet negotiation    
extern const string ON_PLUGIN_TELNET_OPTION;
extern const string ON_PLUGIN_TELNET_REQUEST;
extern const string ON_PLUGIN_TELNET_SUBNEGOTIATION;
extern const string ON_PLUGIN_IAC_GA;

// commands
extern const string ON_PLUGIN_COMMAND;
extern const string ON_PLUGIN_COMMAND_ENTERED;
extern const string ON_PLUGIN_COMMAND_CHANGED;
extern const string ON_PLUGIN_TABCOMPLETE;

// resizing, ticking, moving, rhythm  
extern const string ON_PLUGIN_WORLD_OUTPUT_RESIZED;
extern const string ON_PLUGIN_TICK;
extern const string ON_PLUGIN_MOUSE_MOVED;

// MXP stuff
extern const string ON_PLUGIN_MXP_START;
extern const string ON_PLUGIN_MXP_STOP;
extern const string ON_PLUGIN_MXP_OPENTAG;
extern const string ON_PLUGIN_MXP_CLOSETAG;
extern const string ON_PLUGIN_MXP_SETVARIABLE;
extern const string ON_PLUGIN_MXP_SETENTITY;
extern const string ON_PLUGIN_MXP_ERROR;

// chat stuff
extern const string ON_PLUGIN_CHAT_ACCEPT;
extern const string ON_PLUGIN_CHAT_MESSAGE;
extern const string ON_PLUGIN_CHAT_MESSAGE_OUT;
extern const string ON_PLUGIN_CHAT_DISPLAY;
extern const string ON_PLUGIN_CHAT_NEWUSER;
extern const string ON_PLUGIN_CHAT_USERDISCONNECT;

// table of callbacks
extern string PluginCallbacksNames [];
