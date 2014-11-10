// plugins.cpp

// Plugins management

#include "stdafx.h"
#include "MUSHclient.h"

#include "doc.h"
#include "dialogs\plugins\PluginWizardSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// plugin callback routines - start with OnPlugin so that we can advise
// users not to use that string for their own routines

// install / remove
const string ON_PLUGIN_INSTALL               ("OnPluginInstall");
const string ON_PLUGIN_CLOSE                 ("OnPluginClose");
const string ON_PLUGIN_LIST_CHANGED          ("OnPluginListChanged");

// connect / disconnect
const string ON_PLUGIN_CONNECT               ("OnPluginConnect");
const string ON_PLUGIN_DISCONNECT            ("OnPluginDisconnect");

// saving
const string ON_PLUGIN_SAVE_STATE            ("OnPluginSaveState");
const string ON_PLUGIN_WORLD_SAVE            ("OnPluginWorldSave");

// enable / disable
const string ON_PLUGIN_ENABLE                ("OnPluginEnable");
const string ON_PLUGIN_DISABLE               ("OnPluginDisable");

// the focus                                        
const string ON_PLUGIN_GETFOCUS              ("OnPluginGetFocus");
const string ON_PLUGIN_LOSEFOCUS             ("OnPluginLoseFocus"); 

// capture stuff
const string ON_PLUGIN_TRACE                 ("OnPluginTrace"); 
const string ON_PLUGIN_BROADCAST             ("OnPluginBroadcast"); 
const string ON_PLUGIN_SCREENDRAW            ("OnPluginScreendraw"); 

// sounds
const string ON_PLUGIN_PLAYSOUND             ("OnPluginPlaySound"); 

// stuff received/sent                
const string ON_PLUGIN_SEND                  ("OnPluginSend"); 
const string ON_PLUGIN_SENT                  ("OnPluginSent"); 
const string ON_PLUGIN_PARTIAL_LINE          ("OnPluginPartialLine");
const string ON_PLUGIN_LINE_RECEIVED         ("OnPluginLineReceived");    
const string ON_PLUGIN_PACKET_RECEIVED       ("OnPluginPacketReceived");    

// telnet negotiation    
const string ON_PLUGIN_TELNET_OPTION         ("OnPluginTelnetOption");    
const string ON_PLUGIN_TELNET_REQUEST        ("OnPluginTelnetRequest");    
const string ON_PLUGIN_TELNET_SUBNEGOTIATION ("OnPluginTelnetSubnegotiation");    
const string ON_PLUGIN_IAC_GA                ("OnPlugin_IAC_GA");  

// commands
const string ON_PLUGIN_COMMAND               ("OnPluginCommand"); 
const string ON_PLUGIN_COMMAND_ENTERED       ("OnPluginCommandEntered"); 
const string ON_PLUGIN_COMMAND_CHANGED       ("OnPluginCommandChanged");    
const string ON_PLUGIN_TABCOMPLETE           ("OnPluginTabComplete"); 

// resizing, ticking, moving, rhythm  
const string ON_PLUGIN_WORLD_OUTPUT_RESIZED  ("OnPluginWorldOutputResized");    
const string ON_PLUGIN_TICK                  ("OnPluginTick");    
const string ON_PLUGIN_MOUSE_MOVED           ("OnPluginMouseMoved");    

// MXP stuff
const string ON_PLUGIN_MXP_START             ("OnPluginMXPstart");
const string ON_PLUGIN_MXP_STOP              ("OnPluginMXPstop");
const string ON_PLUGIN_MXP_OPENTAG           ("OnPluginMXPopenTag");    
const string ON_PLUGIN_MXP_CLOSETAG          ("OnPluginMXPcloseTag");
const string ON_PLUGIN_MXP_SETVARIABLE       ("OnPluginMXPsetVariable");
const string ON_PLUGIN_MXP_SETENTITY         ("OnPluginMXPsetEntity");
const string ON_PLUGIN_MXP_ERROR             ("OnPluginMXPerror");

// chat stuff
const string ON_PLUGIN_CHAT_ACCEPT           ("OnPluginChatAccept");
const string ON_PLUGIN_CHAT_MESSAGE          ("OnPluginChatMessage");
const string ON_PLUGIN_CHAT_MESSAGE_OUT      ("OnPluginChatMessageOut");
const string ON_PLUGIN_CHAT_DISPLAY          ("OnPluginChatDisplay");
const string ON_PLUGIN_CHAT_NEWUSER          ("OnPluginChatNewUser");
const string ON_PLUGIN_CHAT_USERDISCONNECT   ("OnPluginChatUserDisconnect");

// drawing
const string ON_PLUGIN_DRAW_OUTPUT_WINDOW    ("OnPluginDrawOutputWindow");

// possible plugin callbacks
string PluginCallbacksNames [] = {
 
   ON_PLUGIN_BROADCAST,             
     
   ON_PLUGIN_CHAT_ACCEPT,           
   ON_PLUGIN_CHAT_DISPLAY,          
   ON_PLUGIN_CHAT_MESSAGE,          
   ON_PLUGIN_CHAT_MESSAGE_OUT,      
   ON_PLUGIN_CHAT_NEWUSER,          
   ON_PLUGIN_CHAT_USERDISCONNECT,   
     
   ON_PLUGIN_CLOSE,                 
   ON_PLUGIN_COMMAND,               
   ON_PLUGIN_COMMAND_CHANGED,       
   ON_PLUGIN_COMMAND_ENTERED,       
   ON_PLUGIN_CONNECT,               
   ON_PLUGIN_DISABLE,               
   ON_PLUGIN_DISCONNECT,
   ON_PLUGIN_DRAW_OUTPUT_WINDOW,
   ON_PLUGIN_ENABLE,                
   ON_PLUGIN_GETFOCUS,              
   ON_PLUGIN_IAC_GA,                
   ON_PLUGIN_INSTALL,               
   ON_PLUGIN_LINE_RECEIVED,         
   ON_PLUGIN_LIST_CHANGED,          
   ON_PLUGIN_LOSEFOCUS,             
   ON_PLUGIN_MOUSE_MOVED,           

   ON_PLUGIN_MXP_CLOSETAG,          
   ON_PLUGIN_MXP_ERROR,             
   ON_PLUGIN_MXP_OPENTAG,           
   ON_PLUGIN_MXP_SETENTITY,         
   ON_PLUGIN_MXP_SETVARIABLE,       
   ON_PLUGIN_MXP_START,             
   ON_PLUGIN_MXP_STOP,              

   ON_PLUGIN_PACKET_RECEIVED,       
   ON_PLUGIN_PARTIAL_LINE,          
   ON_PLUGIN_PLAYSOUND,             
   ON_PLUGIN_SAVE_STATE,            
   ON_PLUGIN_SCREENDRAW,            
   ON_PLUGIN_SEND,                  
   ON_PLUGIN_SENT,                  
   ON_PLUGIN_TABCOMPLETE,           

   ON_PLUGIN_TELNET_OPTION,         
   ON_PLUGIN_TELNET_REQUEST,        
   ON_PLUGIN_TELNET_SUBNEGOTIATION, 

   ON_PLUGIN_TICK,                  
   ON_PLUGIN_TRACE,                 
   ON_PLUGIN_WORLD_OUTPUT_RESIZED,  
   ON_PLUGIN_WORLD_SAVE,            

  ""   // end of table marker            

  };  // end of PluginCallbacksNames 


// constructor
CPlugin::CPlugin (CMUSHclientDoc * pDoc) 
  { 
  m_pDoc = pDoc;
  m_ScriptEngine = NULL;
  m_bEnabled = true;
  m_VariableMap.InitHashTable (293); // allow for 300 variables in this plugin
  m_AliasMap.InitHashTable (293);    // ditto for aliases
  m_TriggerMap.InitHashTable (293);  // ditto for triggers
  m_TimerMap.InitHashTable (293);    // ditto for timers
  m_tDateInstalled = CTime::GetCurrentTime();  // when plugin loaded

  m_dRequiredVersion = 0.0;
  m_dVersion = 0.0;
  m_bSendToScriptUsed = false;
  m_bGlobal = false;
  m_iLoadOrder = 0;
  m_iScriptTimeTaken = 0;
  m_bSavingStateNow = false;
  m_iSequence = DEFAULT_PLUGIN_SEQUENCE;

  } // end of constructor

// destructor
CPlugin::~CPlugin () 
  {
  // change to this plugin, call function, put current plugin back
  CPlugin * pSavedPlugin = m_pDoc->m_CurrentPlugin;
  m_pDoc->m_CurrentPlugin = this;
  CScriptCallInfo callinfo (ON_PLUGIN_CLOSE, m_PluginCallbacks [ON_PLUGIN_CLOSE]);
  ExecutePluginScript (callinfo);
  m_pDoc->m_CurrentPlugin = pSavedPlugin;

  SaveState ();
  DELETE_MAP (m_TriggerMap, CTrigger); 
  DELETE_MAP (m_AliasMap, CAlias); 
  DELETE_MAP (m_TimerMap, CTimer); 
  DELETE_MAP (m_VariableMap, CVariable); 
  delete m_ScriptEngine;

  // delete our arrays
  for (tStringMapOfMaps::iterator it = m_Arrays.begin (); 
       it != m_Arrays.end ();
       it++)
         {
         tStringToStringMap * m = it->second;
         m->clear ();
         delete m;
         }

  } // end of destructor

DISPID CPlugin::GetPluginDispid (const char * sName)
  {

  if (m_ScriptEngine)
    return m_ScriptEngine->GetDispid (sName);
  else
    return DISPID_UNKNOWN;

  } // end of CPlugin::GetPluginDispid

void CPlugin::ExecutePluginScript (CScriptCallInfo & callinfo)
  {
  DISPID & iRoutine = callinfo._dispid_info._dispid;

  if (m_ScriptEngine && iRoutine != DISPID_UNKNOWN)
    {
    callinfo._dispid_info._count++;

    long nInvocationCount = 0;

    CString strType = TFormat ("Plugin %s", 
                               (LPCTSTR) m_strName);
    CString strReason = TFormat ("Executing plugin %s sub %s", 
                                 (LPCTSTR) m_strName,
                                 callinfo._name.c_str () );

    if (m_ScriptEngine->IsLua ())
      {
      list<double> nparams;
      list<string> sparams;
      m_ScriptEngine->ExecuteLua (iRoutine, 
                                   callinfo._name.c_str (),  
                                   eDontChangeAction,
                                   strType, 
                                   strReason,
                                   nparams,
                                   sparams, 
                                   nInvocationCount); 
      }   // end of Lua
    else
      {
      DISPPARAMS params = { NULL, NULL, 0, 0 };
      m_ScriptEngine->Execute (iRoutine, 
                               callinfo._name.c_str (), 
                               eDontChangeAction,
                               strType,
                               strReason,
                               params, 
                               nInvocationCount, NULL);
      } // end of not Lua
    }   // end of having a script engine


  } // end of CPlugin::ExecutePluginScript 


bool CPlugin::ExecutePluginScript (CScriptCallInfo & callinfo, 
                                   const char * sText)
  {
  DISPID & iRoutine = callinfo._dispid_info._dispid;

  if (m_ScriptEngine && iRoutine != DISPID_UNKNOWN)
    {
    callinfo._dispid_info._count++;

    long nInvocationCount = 0;

    CString strType = TFormat ("Plugin %s", 
                               (LPCTSTR) m_strName);

    CString strReason = TFormat ("Executing plugin %s sub %s", 
                                 (LPCTSTR) m_strName,
                                 callinfo._name.c_str () );

    if (m_ScriptEngine->IsLua ())
      {
      list<double> nparams;
      list<string> sparams;
      sparams.push_back (sText);
      bool result;
      m_ScriptEngine->ExecuteLua (iRoutine, 
                                   callinfo._name.c_str (), 
                                   eDontChangeAction,
                                   strType, 
                                   strReason,
                                   nparams,
                                   sparams, 
                                   nInvocationCount,
                                   NULL, NULL, NULL,
                                   &result);
      return result;
      }   // end of Lua
    else
      {

      enum
        {
        eText,
        eArgCount,     // this MUST be last
        };   

      COleVariant args [eArgCount];
    
      DISPPARAMS params = { args, NULL, eArgCount, 0 };
    
      args [eText] = sText;

      COleVariant result;

      m_ScriptEngine->Execute (iRoutine, 
                               callinfo._name.c_str (), 
                               eDontChangeAction,
                               strType,
                               strReason,
                               params, 
                               nInvocationCount, 
                               &result);

      // see what result was
      if (result.vt != VT_EMPTY)
        {
        result.ChangeType (VT_I4);  // make a long
        if (result.vt == VT_I4)   // conversion successful
          if (result.lVal)        // return if non-zero
            return true;
          else
            return false;
        }  // end of having some sort of result

      }  // end of not Lua
    }   // end of having a script engine

  return true;    // assume they OK'd it

  }  // end of CPlugin::ExecutePluginScript

bool CPlugin::ExecutePluginScript (CScriptCallInfo & callinfo, 
                                  const long arg1,
                                  const string sText)
  {
  DISPID & iRoutine = callinfo._dispid_info._dispid;

  if (m_ScriptEngine && iRoutine != DISPID_UNKNOWN)
    {
    callinfo._dispid_info._count++;

    long nInvocationCount = 0;


    CString strType = TFormat ("Plugin %s", (LPCTSTR) m_strName);
    CString strReason =  TFormat ("Executing plugin %s sub %s", 
                                       (LPCTSTR) m_strName,
                                       callinfo._name.c_str () );

    if (m_ScriptEngine && m_ScriptEngine->IsLua ())
      {
      bool bResult;
      list<double> nparams;
      list<string> sparams;
      nparams.push_back (arg1);
      sparams.push_back (sText);
      m_ScriptEngine->ExecuteLua (iRoutine, 
                                  callinfo._name.c_str (), 
                                  eDontChangeAction,
                                  strType, 
                                  strReason, 
                                  nparams,
                                  sparams, 
                                  nInvocationCount,
                                  NULL, NULL, NULL,
                                  &bResult); 
      return bResult;
      }   // end of Lua

  // WARNING - arguments should appear in REVERSE order to what the sub expects them!
  
    enum
      {
      eText,
      eArg1,
      eArgCount,     // this MUST be last
      };    


    COleVariant args [eArgCount];
    
    DISPPARAMS params = { args, NULL, eArgCount, 0 };
    
    args [eArg1] = arg1;

    // this crazy code is to get 0x00 bytes into telnet subnegotiations ;)

    VARIANT v;
    VariantClear (&v);
    v.vt = VT_BSTR;
    CString str (sText.c_str (), sText.size ());
    v.bstrVal = str.AllocSysString (); 
    args [eText].Attach (v);
    
    COleVariant result;

    m_ScriptEngine->Execute (iRoutine, 
                             callinfo._name.c_str (), 
                             eDontChangeAction,
                             strType,
                             strReason,
                             params, 
                             nInvocationCount, 
                             &result);

    // see what result was
    if (result.vt != VT_EMPTY)
      {
      result.ChangeType (VT_I4);  // make a long
      if (result.vt == VT_I4)   // conversion successful
        if (result.lVal)        // return if non-zero
          return true;
        else
          return false;
      }  // end of having some sort of result

    }   // end of having a script engine

  return true;    // assume they OK'd it

  } // end of CPlugin::ExecutePluginScript

bool CPlugin::ExecutePluginScript (CScriptCallInfo & callinfo, 
                                  const long arg1,
                                  const long arg2,
                                  const string sText)
  {
  DISPID & iRoutine = callinfo._dispid_info._dispid;

  if (m_ScriptEngine && iRoutine != DISPID_UNKNOWN)
    {
    callinfo._dispid_info._count++;
    long nInvocationCount = 0;

    CString strType = TFormat ("Plugin %s", (LPCTSTR) m_strName);
    CString strReason =  TFormat ("Executing plugin %s sub %s", 
                                       (LPCTSTR) m_strName,
                                       callinfo._name.c_str () );

    if (m_ScriptEngine && m_ScriptEngine->IsLua ())
      {
      bool bResult;
      list<double> nparams;
      list<string> sparams;
      nparams.push_back (arg1);
      nparams.push_back (arg2);
      sparams.push_back (sText);
      m_ScriptEngine->ExecuteLua (iRoutine, 
                                  callinfo._name.c_str (), 
                                  eDontChangeAction,
                                  strType, 
                                  strReason, 
                                  nparams,
                                  sparams, 
                                  nInvocationCount,
                                  NULL, NULL, NULL,
                                  &bResult); 
      return bResult;
      }   // end of Lua


  // WARNING - arguments should appear in REVERSE order to what the sub expects them!
  
    enum
      {
      eText,
      eArg2,
      eArg1,
      eArgCount,     // this MUST be last
      };    


    COleVariant args [eArgCount];
    
    DISPPARAMS params = { args, NULL, eArgCount, 0 };
    
    args [eArg1] = arg1;
    args [eArg2] = arg2;
    args [eText] = CString (sText.c_str (), sText.size ());  // yadda yadda, doesn't work with internal 0x00
    
    COleVariant result;

    m_ScriptEngine->Execute (iRoutine, 
                             callinfo._name.c_str (), 
                             eDontChangeAction,
                             strType,
                             strReason,
                             params, 
                             nInvocationCount, 
                             &result);

    // see what result was
    if (result.vt != VT_EMPTY)
      {
      result.ChangeType (VT_I4);  // make a long
      if (result.vt == VT_I4)   // conversion successful
        if (result.lVal)        // return if non-zero
          return true;
        else
          return false;
      }  // end of having some sort of result

    }   // end of having a script engine

  return true;    // assume they OK'd it

  } // end of CPlugin::ExecutePluginScript


bool CPlugin::ExecutePluginScript (CScriptCallInfo & callinfo, 
                                  const long arg1,
                                  const char * arg2,
                                  const char * arg3,
                                  const char * arg4)
  {
  DISPID & iRoutine = callinfo._dispid_info._dispid;

  if (m_ScriptEngine && iRoutine != DISPID_UNKNOWN)
    {
    callinfo._dispid_info._count++;
    long nInvocationCount = 0;

    CString strType = TFormat ("Plugin %s", (LPCTSTR) m_strName);
    CString strReason =  TFormat ("Executing plugin %s sub %s", 
                                       (LPCTSTR) m_strName,
                                       callinfo._name.c_str () );

    if (m_ScriptEngine && m_ScriptEngine->IsLua ())
      {
      bool bResult;
      list<double> nparams;
      list<string> sparams;
      nparams.push_back (arg1);
      sparams.push_back ((LPCTSTR) arg2);
      sparams.push_back ((LPCTSTR) arg3);
      sparams.push_back ((LPCTSTR) arg4);
      m_ScriptEngine->ExecuteLua (iRoutine, 
                                  callinfo._name.c_str (), 
                                  eDontChangeAction,
                                  strType, 
                                  strReason, 
                                  nparams,
                                  sparams, 
                                  nInvocationCount,
                                  NULL, NULL, NULL,
                                  &bResult); 
      return bResult;
      }   // end of Lua


  // WARNING - arguments should appear in REVERSE order to what the sub expects them!
  
    enum
      {
      eArg4,
      eArg3,
      eArg2,
      eArg1,
      eArgCount,     // this MUST be last
      };    


    COleVariant args [eArgCount];
    
    DISPPARAMS params = { args, NULL, eArgCount, 0 };
    
    args [eArg1] = arg1;
    args [eArg2] = arg2;
    args [eArg3] = arg3;
    args [eArg4] = arg4;
    
    COleVariant result;

    m_ScriptEngine->Execute (iRoutine, 
                             callinfo._name.c_str (), 
                             eDontChangeAction,
                             strType,
                             strReason,
                             params, 
                             nInvocationCount, 
                             &result);

    // see what result was
    if (result.vt != VT_EMPTY)
      {
      result.ChangeType (VT_I4);  // make a long
      if (result.vt == VT_I4)   // conversion successful
        if (result.lVal)        // return if non-zero
          return true;
        else
          return false;
      }  // end of having some sort of result

    }   // end of having a script engine

  return true;    // assume they OK'd it

  } // end of CPlugin::ExecutePluginScript

void CPlugin::ExecutePluginScriptRtn (CScriptCallInfo & callinfo, 
                                      CString & strText) 
  {
  DISPID & iRoutine = callinfo._dispid_info._dispid;

  if (m_ScriptEngine && iRoutine != DISPID_UNKNOWN)
    {
    callinfo._dispid_info._count++;

    long nInvocationCount = 0;

    CString strType = TFormat ("Plugin %s", 
                               (LPCTSTR) m_strName);

    CString strReason = TFormat ("Executing plugin %s sub %s", 
                                 (LPCTSTR) m_strName,
                                 callinfo._name.c_str () );

    if (m_ScriptEngine->IsLua ())
      {
      m_ScriptEngine->ExecuteLua (iRoutine,
                                  callinfo._name.c_str (),
                                  eDontChangeAction,
                                  strType,
                                  strReason,
                                  strText,
                                  nInvocationCount,
                                  strText);
      } // end of Lua
    else
      {

      enum
        {
        eText,
        eArgCount,     // this MUST be last
        };   

      COleVariant args [eArgCount];
  
      DISPPARAMS params = { args, NULL, eArgCount, 0 };
  
      args [eText] = strText;

      COleVariant result;

      m_ScriptEngine->Execute (iRoutine, 
                               callinfo._name.c_str (), 
                               eDontChangeAction,
                               strType,
                               strReason,
                               params, 
                               nInvocationCount, 
                               &result);


      // if they returned a string, give it back
      if (result.vt == VT_BSTR)
        strText = result.bstrVal;

      } // end of not Lua

    }   // end of having a script engine

  } // end of CPlugin::ExecutePluginScript 

void RemoveTrailingBackslash (CString& str);

// function prototypes needed for folder browsing

int __stdcall BrowseCallbackProc(
    HWND hwnd,  
    UINT uMsg,  
    LPARAM lParam,  
    LPARAM lpData 
   );

// The field below is needed to initialise the browse directory dialog 
// with the initial directory

extern CString strStartingDirectory;

bool CPlugin::SaveState (const bool bScripted)
  {

  if (!m_bSaveState && !bScripted)
    return true;   // not needed

// need a directory
          
  if (App.m_strDefaultStateFilesDirectory.IsEmpty ())
    return true;
  
CString strFilename = CString (Make_Absolute_Path (App.m_strDefaultStateFilesDirectory));
CFile * f = NULL;
CArchive * ar = NULL;
bool bError = true;
  

// need a world id

  if (m_pDoc->m_strWorldID.IsEmpty ())
    return true;

  // sigh ... check plugin state folder exists

  CFileStatus status;
  CString strError;
  CString strFolder = strFilename;
  RemoveTrailingBackslash (strFolder);     // trailing slash not wanted, thanks

  if (!CFile::GetStatus(strFolder, status))
    {
    strError = "does not exist";
    }
  else
    {   // file exists, is it a writable folder?
    if ((status.m_attribute & CFile::directory) == 0)
      strError = "is not a directory";
    else if ((status.m_attribute & CFile::readOnly) == 1)
      strError = "is not writable";
    }  // end of checking for save state folder

  // if some error, alert the media
  if (!strError.IsEmpty ())
    {
    ::AfxMessageBox ((LPCTSTR) CFormat ("The plugins 'save state' folder:\r\n\r\n"
               "%s\r\n\r\n"
               "%s. Please click OK and then browse for it, or create it.",
               (LPCTSTR) strFilename,
               (LPCTSTR) strError),
               MB_ICONSTOP);

    // BROWSE FOR STATE FOLDER

    // Gets the Shell's default allocator
    LPMALLOC pMalloc;
    if (::SHGetMalloc(&pMalloc) == NOERROR)
    {
      char  pszBuffer[MAX_PATH];
      BROWSEINFO    bi;
      LPITEMIDLIST  pidl;

      // Get help on BROWSEINFO struct - it's got all the bit settings.
      bi.hwndOwner = NULL;
      bi.pidlRoot = NULL;
      bi.pszDisplayName = pszBuffer;
      bi.lpszTitle = "Folder for saving plugin state files";
      bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;

      // if possible, let them create one
      if (!bWine)  
        bi.ulFlags |= BIF_NEWDIALOGSTYLE | BIF_EDITBOX;     // requires CoInitialize
  
      bi.lpfn = BrowseCallbackProc;
      bi.lParam = 0;
      strStartingDirectory = App.m_strPluginsDirectory; // really should be under plugins directory

      // This next call issues the dialog box.
      if ((pidl = ::SHBrowseForFolder(&bi)) != NULL)
      {
        if (::SHGetPathFromIDList(pidl, pszBuffer))
          {
          App.m_strDefaultStateFilesDirectory = pszBuffer;
          App.m_strDefaultStateFilesDirectory += "\\";
          strFilename = App.m_strDefaultStateFilesDirectory;
          // save back to database
          App.SaveGlobalsToDatabase ();

          }

        // Free the PIDL allocated by SHBrowseForFolder.
        pMalloc->Free(pidl);
      }
      // Release the shell's allocator.
      pMalloc->Release();
      }

    }  // end of no save state folder

  CPlugin * oldPlugin = m_pDoc->m_CurrentPlugin;
  m_pDoc->m_CurrentPlugin = this;

  // prevent infinite loops
  m_bSavingStateNow = true;

  CScriptCallInfo callinfo (ON_PLUGIN_SAVE_STATE, m_PluginCallbacks [ON_PLUGIN_SAVE_STATE]);
  ExecutePluginScript (callinfo);

  // are not saving state now
  m_bSavingStateNow = false;

  strFilename += m_pDoc->m_strWorldID;    // world ID
  strFilename += "-";
  strFilename += m_strID;                 // plugin ID
  strFilename += "-state.xml";            // suffix


  try
    {
    f = new CFile (strFilename, 
                    CFile::modeCreate | CFile::modeReadWrite);

    ar = new CArchive(f, CArchive::store);

    CString strComment = TFormat ("Plugin state saved. Plugin: \"%s\". World: \"%s\".",
                (LPCTSTR) m_strName,
                (LPCTSTR) m_pDoc->m_mush_name);

    m_pDoc->Save_World_XML (*ar, XML_VARIABLES, strComment);

    bError = false;

    } // end of try block

  catch (CFileException * e)
    {
    ::UMessageBox (
      TFormat ("Unable to create the plugin save state file: %s",
              (LPCTSTR) strFilename), 
              MB_ICONEXCLAMATION);
    e->Delete ();
    } // end of catching a file exception

  catch (CMemoryException * e)
    {
    ::UMessageBox (
      TFormat ("Insufficient memory to write the plugin save state file: %s",
              (LPCTSTR) strFilename), 
              MB_ICONEXCLAMATION);
    e->Delete ();
    } // end of catching a memory exception

  catch (CArchiveException * e)
    {
    ::UMessageBox (
      TFormat ("There was a problem saving the plugin save state file: %s",
              (LPCTSTR) strFilename), 
              MB_ICONEXCLAMATION);
    e->Delete ();
    } // end of catching an archive exception

  delete ar;      // delete archive
  delete f;       // delete file

  // put old plugin back
  m_pDoc->m_CurrentPlugin = oldPlugin;

  return bError;

  } // end of CPlugin::SaveState 


void CMUSHclientDoc::OnFilePluginwizard() 
{
  // TODO: The property sheet attached to your project
  // via this function is not hooked up to any message
  // handler.  In order to actually use the property sheet,
  // you will need to associate this function with a control
  // in your project such as a menu item or tool bar button.

  CPluginWizardSheet propSheet;

  propSheet.m_Page1.m_doc = this;
  propSheet.m_Page2.m_doc = this;
  propSheet.m_Page3.m_doc = this;
  propSheet.m_Page4.m_doc = this;
  propSheet.m_Page5.m_doc = this;
  propSheet.m_Page6.m_doc = this;
  propSheet.m_Page7.m_doc = this;
  propSheet.m_Page8.m_doc = this;

  // pre-load some stuff
  propSheet.m_Page1.m_strID = ::GetUniqueID ();
  CTime date = CTime::GetCurrentTime();
  propSheet.m_Page1.m_strDateWritten = date.Format ("%Y-%m-%d %H:%M:%S");
  propSheet.m_Page1.m_strVersion = "1.0";
  propSheet.m_Page1.m_bRemoveItems = true;

  myAtoF (MUSHCLIENT_VERSION, &propSheet.m_Page1.m_fRequires); 

  POSITION pos;  
  CString strName;
  int iCount = 0;

  // select everything
  // triggers
  for (pos = m_TriggerMap.GetStartPosition(); pos; )
    {                                               
    CTrigger * t;
    m_TriggerMap.GetNextAssoc (pos, strName, t);  
    t->bSelected = true;
    }                                               
  // aliases
  for (pos = m_AliasMap.GetStartPosition(); pos; )
    {                                               
    CAlias * a;
    m_AliasMap.GetNextAssoc (pos, strName, a);  
    a->bSelected = true;
    }                                               
  // timers
  for (pos = m_TimerMap.GetStartPosition(); pos; )
    {                                               
    CTimer * t;
    m_TimerMap.GetNextAssoc (pos, strName, t);  
    t->bSelected = true;
    }                                               
  // variables
  for (pos = m_VariableMap.GetStartPosition(); pos; iCount++)
    {                                               
    CVariable * v;
    m_VariableMap.GetNextAssoc (pos, strName, v);  
    v->bSelected = true;
    }                                               
  
  propSheet.m_Page2.m_bGenerateHelp = true;
  propSheet.m_Page6.m_bSaveState = iCount > 0;
  propSheet.m_Page7.m_strLanguage = m_strLanguage;
  propSheet.m_Page7.m_bStandardConstants = m_bEnableScripts;

  // load script file
  if (!m_strScriptFilename.IsEmpty ())
    {
    try
      {
      CFile f (m_strScriptFilename, CFile::modeRead | CFile::shareDenyWrite);
      UINT iLength = f.GetLength ();
      if (iLength > 0)
        {
        char * p = propSheet.m_Page7.m_strScript.GetBuffer (iLength + 1);
        if (f.Read (p, iLength) != iLength)
          {
          propSheet.m_Page7.m_strScript.ReleaseBuffer (0);
          ThrowErrorException ("Could not read script file");
          }
        propSheet.m_Page7.m_strScript.ReleaseBuffer (iLength);
        }   // end of non zero-length script file
      }      // end of try block
    catch (CException* e)
      {
        e->ReportError();
        e->Delete();
      }       // end of catch
    }  // end of having a script file

  if (propSheet.DoModal() != IDOK)
    return;

  // write plugin

#define NL "\r\n"

  try
    {

    CString strFilename = propSheet.m_Page1.m_strName + ".xml";

    // now find output file name
    CFileDialog dlg(FALSE,            // FALSE for FileSave
            "xml",            // default extension
            strFilename,    // suggest obvious name
            OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST,
            "Plugin files (*.xml)|*.xml||",
            NULL);
    dlg.m_ofn.lpstrTitle = "Save plugin as";
    // use default plugins directory
    dlg.m_ofn.lpstrInitialDir = Make_Absolute_Path (App.m_strPluginsDirectory);

    ChangeToFileBrowsingDirectory ();
    int nResult = dlg.DoModal();
    ChangeToStartupDirectory ();

    if (nResult != IDOK)
      return;

    strFilename = dlg.GetPathName();

    CFile f (strFilename, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive);
    CArchive ar(&f, CArchive::store);

    // xml prolog
    ar.WriteString ("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" NL);
    // document type is world
    ar.WriteString ("<!DOCTYPE muclient>" NL);

    // when did we save it?
    ar.WriteString (CFormat ("<!-- Saved on %s -->" NL, 
          FixHTMLString (CTime::GetCurrentTime().Format 
          (TranslateTime ("%A, %B %d, %Y, %#I:%M %p")))));

    // which version was it?
    ar.WriteString (CFormat 
          ("<!-- MuClient version %s -->" NL, MUSHCLIENT_VERSION));

    ar.WriteString (CFormat (NL "<!-- Plugin \"%s\" generated by Plugin Wizard -->" NL NL,
                    (LPCTSTR) propSheet.m_Page1.m_strName));

    if (!propSheet.m_Page8.m_strComments.IsEmpty ())
      {
      ar.WriteString ("<!--" NL);
      ar.WriteString (propSheet.m_Page8.m_strComments);
      ar.WriteString (NL "-->" NL NL);
      }

    // ok - ready to go with the plugin data
    ar.WriteString ("<muclient>" NL);
    ar.WriteString ("<plugin" NL);

    Save_XML_string  (ar, "name",           propSheet.m_Page1.m_strName);
    Save_XML_string  (ar, "author",         propSheet.m_Page1.m_strAuthor);
    Save_XML_string  (ar, "id",             propSheet.m_Page1.m_strID);
    Save_XML_string  (ar, "language",       propSheet.m_Page7.m_strLanguage);
    Save_XML_string  (ar, "purpose",        propSheet.m_Page1.m_strPurpose);
    Save_XML_boolean (ar, "save_state",     propSheet.m_Page6.m_bSaveState);
    Save_XML_string  (ar, "date_written",   propSheet.m_Page1.m_strDateWritten);
    Save_XML_double  (ar, "requires",       propSheet.m_Page1.m_fRequires);
    Save_XML_string  (ar, "version",        propSheet.m_Page1.m_strVersion);

    ar.WriteString ("   >" NL);    // end of plugin attributes

    // ---------- description ----------
    if (!propSheet.m_Page2.m_strDescription.IsEmpty ())
      {
      ar.WriteString ("<description trim=\"y\">" NL);
      ar.WriteString ("<![CDATA[" NL);
      ar.WriteString (propSheet.m_Page2.m_strDescription);
      ar.WriteString (NL "]]>" NL);
      ar.WriteString ("</description>" NL);
      }

    // that ends the <plugin> ... </plugin> sequence
    ar.WriteString (NL "</plugin>" NL NL);

    // ---------- constants ----------
    if (propSheet.m_Page7.m_bStandardConstants)
      {
      ar.WriteString (NL "<!--  Get our standard constants -->" NL NL);

      if (propSheet.m_Page7.m_strLanguage.CompareNoCase ("vbscript") == 0)
        ar.WriteString ("<include name=\"constants.vbs\"/>" NL);
      else
      if (propSheet.m_Page7.m_strLanguage.CompareNoCase ("jscript") == 0)
        ar.WriteString ("<include name=\"constants.js\"/>" NL);
      else
      if (propSheet.m_Page7.m_strLanguage.CompareNoCase ("perlscript") == 0)
        ar.WriteString ("<include name=\"constants.pl\"/>" NL);
      else
      if (propSheet.m_Page7.m_strLanguage.CompareNoCase ("python") == 0)
        ar.WriteString ("<include name=\"constants.pys\"/>" NL);
      else
      if (propSheet.m_Page7.m_strLanguage.CompareNoCase ("tclscript") == 0)
        ar.WriteString ("<include name=\"constants.tcl\"/>" NL);
      else
      if (propSheet.m_Page7.m_strLanguage.CompareNoCase ("lua") == 0)
        ar.WriteString ("<include name=\"constants.lua\"/>" NL);
      else
      if (propSheet.m_Page7.m_strLanguage.CompareNoCase ("phpscript") == 0)
        ar.WriteString ("<include name=\"constants.php\"/>" NL);
      else
      if (propSheet.m_Page7.m_strLanguage.CompareNoCase ("rubyscript") == 0)
        ar.WriteString ("<include name=\"constants.rb\"/>" NL);
      }

    // ---------- triggers ----------

    iCount = 0;
    for (pos = m_TriggerMap.GetStartPosition(); pos; )
      {                                               
      CTrigger * t;
      m_TriggerMap.GetNextAssoc (pos, strName, t);  
      if (t->bSelected)
        iCount++;
      }                                               

    if (iCount)
      {
      ar.WriteString (NL "<!--  Triggers  -->" NL NL);
      Save_Header_XML (ar, "triggers", false);
      for (pos = m_TriggerMap.GetStartPosition(); pos; )
        {                                               
        CTrigger * t;
        m_TriggerMap.GetNextAssoc (pos, strName, t);  
        if (t->bSelected)
          Save_One_Trigger_XML (ar, t);
        }                                               
      Save_Footer_XML (ar, "triggers");
      } // end of having some

    // ---------- aliases ----------

    iCount = 0;
    for (pos = m_AliasMap.GetStartPosition(); pos; )
      {                                               
      CAlias * a;
      m_AliasMap.GetNextAssoc (pos, strName, a);  
      if (a->bSelected)
         iCount++;
      }                                               

    if (iCount)
      {    
      ar.WriteString (NL "<!--  Aliases  -->" NL NL);
      Save_Header_XML (ar, "aliases", false);
      for (pos = m_AliasMap.GetStartPosition(); pos; )
        {                                               
        CAlias * a;
        m_AliasMap.GetNextAssoc (pos, strName, a);  
        if (a->bSelected)
          Save_One_Alias_XML (ar, a);
        }                                               
      Save_Footer_XML (ar, "aliases");
      } // end of having some

    // ---------- timers ----------

    iCount = 0;
    for (pos = m_TimerMap.GetStartPosition(); pos; )
      {                                               
      CTimer * t;
      m_TimerMap.GetNextAssoc (pos, strName, t);  
      if (t->bSelected)
         iCount++;
      }                                               

    if (iCount)
      {    
      ar.WriteString (NL "<!--  Timers  -->" NL NL);
      Save_Header_XML (ar, "timers", false);
      for (pos = m_TimerMap.GetStartPosition(); pos; )
        {                                               
        CTimer * t;
        m_TimerMap.GetNextAssoc (pos, strName, t);  
        if (t->bSelected)
          Save_One_Timer_XML (ar, t);
        }                                               
      Save_Footer_XML (ar, "timers");
      } // end of having some

    // ---------- variables ----------

    iCount = 0;
    for (pos = m_VariableMap.GetStartPosition(); pos; )
      {                                               
      CVariable * v;
      m_VariableMap.GetNextAssoc (pos, strName, v);  
      if (v->bSelected)
         iCount++;
      }                                               

    if (iCount)
      {    
      ar.WriteString (NL "<!--  Variables  -->" NL NL);
      Save_Header_XML (ar, "variables", false);
      for (pos = m_VariableMap.GetStartPosition(); pos; )
        {                                               
        CVariable * v;
        m_VariableMap.GetNextAssoc (pos, strName, v);  
        if (v->bSelected)
          Save_One_Variable_XML (ar, v);
        }                                               
      Save_Footer_XML (ar, "variables");
      } // end of having some

    // ---------- script ----------
    if (!propSheet.m_Page7.m_strScript.IsEmpty ())
      {
      ar.WriteString (NL "<!--  Script  -->" NL NL);
      ar.WriteString (NL "<script>" NL);
      ar.WriteString ("<![CDATA[" NL);
      ar.WriteString (propSheet.m_Page7.m_strScript);
      ar.WriteString (NL "]]>" NL);
      ar.WriteString ("</script>" NL NL);
      }

    // ----------- help alias --------
    //  we need:
    //    a) them to want it
    //    b) an alias name (eg. "help me")
    //    c) a description (ie. the help text itself)
    //    d) a scripting language to write it in

    if (propSheet.m_Page2.m_bGenerateHelp &&
        !propSheet.m_Page2.m_strHelpAlias.IsEmpty () &&
        !propSheet.m_Page2.m_strDescription.IsEmpty () &&
        !propSheet.m_Page7.m_strLanguage.IsEmpty ())
      {
      ar.WriteString (NL "<!--  Plugin help  -->" NL NL);
      Save_Header_XML (ar, "aliases", false);
      ar.WriteString ("  <alias" NL);

      Save_XML_string  (ar, "script",  "OnHelp");
      Save_XML_string  (ar, "match",   propSheet.m_Page2.m_strHelpAlias);
      Save_XML_boolean (ar, "enabled", true);
      ar.WriteString ("  >" NL);
      ar.WriteString ("  </alias>" NL);
      Save_Footer_XML (ar, "aliases");
      ar.WriteString (NL "<script>" NL);
      ar.WriteString ("<![CDATA[" NL);

      if (propSheet.m_Page7.m_strLanguage.CompareNoCase ("vbscript") == 0)
        {
        ar.WriteString ("Sub OnHelp (sName, sLine, wildcards)" NL);
        ar.WriteString ("  world.Note world.GetPluginInfo (world.GetPluginID, 3)" NL);
        ar.WriteString ("End Sub" NL);        
        }
      else
      if (propSheet.m_Page7.m_strLanguage.CompareNoCase ("jscript") == 0)
        {
        ar.WriteString ("function OnHelp (sName, sLine, wildcards)" NL);
        ar.WriteString ("  {" NL);        
        ar.WriteString ("  world.Note (world.GetPluginInfo (world.GetPluginID, 3));" NL);
        ar.WriteString ("  }" NL);        
        }
      else
      if (propSheet.m_Page7.m_strLanguage.CompareNoCase ("perlscript") == 0)
        {
        ar.WriteString ("sub OnHelp" NL);
        ar.WriteString ("  {" NL);        
        ar.WriteString ("  my ($sName, $sLine, $wildcards) = @_;" NL);
        ar.WriteString ("  $world->Note ($world->GetPluginInfo ($world->GetPluginID, 3));" NL);
        ar.WriteString ("  }" NL);        
        }
      else
      if (propSheet.m_Page7.m_strLanguage.CompareNoCase ("python") == 0)
        {
        ar.WriteString ("def OnHelp (sName, sLine, wildcards):" NL);
        ar.WriteString ("  world.Note (world.GetPluginInfo (world.GetPluginID, 3))" NL);
        }
      else
      if (propSheet.m_Page7.m_strLanguage.CompareNoCase ("tclscript") == 0)
        {
        ar.WriteString ("def OnHelp (sName, sLine, wildcards):" NL);
        ar.WriteString ("  world.Note (world.GetPluginInfo (world.GetPluginID, 3))" NL);
        }
      else
      if (propSheet.m_Page7.m_strLanguage.CompareNoCase ("lua") == 0)
        {
        ar.WriteString ("function OnHelp ()" NL);
        ar.WriteString ("  world.Note (world.GetPluginInfo (world.GetPluginID (), 3))" NL);
        ar.WriteString ("end" NL);        
        }
      else
      if (propSheet.m_Page7.m_strLanguage.CompareNoCase ("phpscript") == 0)
        {
        ar.WriteString ("function OnHelp ()" NL);
        ar.WriteString ("  {" NL);        
        ar.WriteString ("  world.Note (world.GetPluginInfo (world.GetPluginID (), 3));" NL);
        ar.WriteString ("  }" NL);        
        }
      else
      if (propSheet.m_Page7.m_strLanguage.CompareNoCase ("rubyscript") == 0)
        {
        ar.WriteString ("def self.OnHelp ()" NL);
        ar.WriteString ("  @world.Note (@world.GetPluginInfo (@world.GetPluginID (), 3))" NL);
        ar.WriteString ("  end" NL);        
        }
      
      ar.WriteString ("]]>" NL);
      ar.WriteString ("</script> " NL);

      } // end of generating help

    // last line in XML file
    ar.WriteString (NL "</muclient>" NL);

    ar.Close();

    // If we got this far, let's get rid of the triggers, aliases, timers, and variables
    // from our current namespace, if wanted


    if (propSheet.m_Page1.m_bRemoveItems)
      {
      // ---------- triggers ----------

      iCount = 0;
      for (pos = m_TriggerMap.GetStartPosition(); pos; )
        {                                               
        CTrigger * t;
        m_TriggerMap.GetNextAssoc (pos, strName, t);  
        if (t->bSelected)
          {
          iCount++;
          // delete its pointer
          delete t;
          // now delete its entry
          m_TriggerMap.RemoveKey (strName);
          }   // end of selected trigger
    
        // sort remaining ones, show document modified
        if (iCount)
          {
          m_CurrentPlugin = NULL;
          SortTriggers ();
          SetModifiedFlag (TRUE);   // document has changed
          }

        }  // end of doing all triggers

      // ---------- aliases ----------

      iCount = 0;
      for (pos = m_AliasMap.GetStartPosition(); pos; )
        {                                               
        CAlias * a;
        m_AliasMap.GetNextAssoc (pos, strName, a);  
        if (a->bSelected)
          {
          iCount++;
          // delete its pointer
          delete a;
          // now delete its entry
          m_AliasMap.RemoveKey (strName);
          }   // end of selected Alias
    
        //  show document modified
        if (iCount)
          {
          m_CurrentPlugin = NULL;
          SortAliases ();
          SetModifiedFlag (TRUE);   // document has changed
          }

        }  // end of doing all aliases

      // ---------- timers ----------

      iCount = 0;
      for (pos = m_TimerMap.GetStartPosition(); pos; )
        {                                               
        CTimer * t;
        m_TimerMap.GetNextAssoc (pos, strName, t);  
        if (t->bSelected)
          {
          iCount++;
          // delete its pointer
          delete t;
          // now delete its entry
          m_TimerMap.RemoveKey (strName);
          }   // end of selected Timer
    
        // show document modified
        if (iCount)
          {
          m_CurrentPlugin = NULL;
          SortTimers ();
          SetModifiedFlag (TRUE);   // document has changed
          }

        }  // end of doing all Timers

      // ---------- variables ----------

      iCount = 0;
      for (pos = m_VariableMap.GetStartPosition(); pos; )
        {                                               
        CVariable * v;
        m_VariableMap.GetNextAssoc (pos, strName, v);  
        if (v->bSelected)
          {
          iCount++;
          // delete its pointer
          delete v;
          // now delete its entry
          m_VariableMap.RemoveKey (strName);
          }   // end of selected Variable
    
        // show document modified
        if (iCount)
          SetModifiedFlag (TRUE);   // document has changed

        }  // end of doing all Variables
      } // end of removing wanted
    }      // end of try block
  catch (CException* e)
  {
    e->ReportError();
    e->Delete();
  }       // end of catch

}  // end of CMUSHclientDoc::OnFilePluginwizard




// tell plugins the list of plugins may have changed
void  CMUSHclientDoc::PluginListChanged (void)

  {

static bool bInPluginListChanged = false;

  // don't recurse into infinite loops
  if (bInPluginListChanged)
    return;

  bInPluginListChanged = true;
  SendToAllPluginCallbacks (ON_PLUGIN_LIST_CHANGED);
  bInPluginListChanged = false;

  }    // end CMUSHclientDoc::PluginListChanged 


void CMUSHclientDoc::SendToAllPluginCallbacks (const string & sName)   // no arguments
  {
  CPlugin * pSavedPlugin = m_CurrentPlugin;

  // tell a plugin the message
   for (PluginListIterator pit = m_PluginList.begin (); 
         pit != m_PluginList.end (); 
         ++pit)
    {
    CPlugin * pPlugin = *pit;

    if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
      continue;

    // change to this plugin, call function, put current plugin back
    m_CurrentPlugin = pPlugin;        // so plugin knows who it is
    CScriptCallInfo callinfo (sName, pPlugin->m_PluginCallbacks [sName]);
    pPlugin->ExecutePluginScript (callinfo);
    m_CurrentPlugin = pSavedPlugin;  // back to current plugin

    }   // end of doing each plugin

  } // end of CMUSHclientDoc::SendToAllPluginCallbacks

// this is for when we want the first available plugin to handle something (eg. Trace, Sound)
bool CMUSHclientDoc::SendToFirstPluginCallbacks (const string & sName, const char * sText)   // one argument
  {
  CPlugin * pSavedPlugin = m_CurrentPlugin;

  // tell a plugin the message
   for (PluginListIterator pit = m_PluginList.begin (); 
         pit != m_PluginList.end (); 
         ++pit)
    {
    CPlugin * pPlugin = *pit;

    if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
      continue;

    // change to this plugin, call function, put current plugin back
    m_CurrentPlugin = pPlugin;        // so plugin knows who it is
    CScriptCallInfo callinfo (sName, pPlugin->m_PluginCallbacks [sName]);
    pPlugin->ExecutePluginScript (callinfo, sText); 
    m_CurrentPlugin = pSavedPlugin;   // back to current plugin

    if (callinfo._dispid_info.isvalid ())
      {
      m_CurrentPlugin = pSavedPlugin;
      return true;   // indicate we found it
      }

    }   // end of doing each plugin

  return false;  // didn't find one
  } // end of CMUSHclientDoc::SendToFirstPluginCallbacks


// this is for when each plugin gets a chance to "black ball" an action (like sending a line)
// we only return true if each plugin returned true
bool CMUSHclientDoc::SendToAllPluginCallbacks (const string & sName, 
                                               const char * sText,         // one argument
                                               const bool bStopOnFalse)
  {
  CPlugin * pSavedPlugin = m_CurrentPlugin;
  bool bResult = true;    // assume they OK'd something

  // tell a plugin the message
  for (PluginListIterator pit = m_PluginList.begin (); 
         pit != m_PluginList.end (); 
         ++pit)
    {
    CPlugin * pPlugin = *pit;

    if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
      continue;

    // change to this plugin, call function, put current plugin back
    m_CurrentPlugin = pPlugin;        // so plugin knows who it is
    CScriptCallInfo callinfo (sName, pPlugin->m_PluginCallbacks [sName]);
    if (!pPlugin->ExecutePluginScript (callinfo, sText))
        bResult = false;
    m_CurrentPlugin = pSavedPlugin;  // back to current plugin

    if (bStopOnFalse && !bResult && callinfo._dispid_info.isvalid ())
      return false;

    }   // end of doing each plugin

  return bResult;
  } // end of CMUSHclientDoc::SendToAllPluginCallbacks


// this sends a string to all plugins and allows them to modify it
void CMUSHclientDoc::SendToAllPluginCallbacksRtn (const string & sName, CString & strResult)  // taking and returning a string
  {
  CPlugin * pSavedPlugin = m_CurrentPlugin;

  // tell a plugin the message
  for (PluginListIterator pit = m_PluginList.begin (); 
         pit != m_PluginList.end (); 
         ++pit)
    {
    CPlugin * pPlugin = *pit;

    if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
      continue;

    // change to this plugin, call function, put current plugin back
    m_CurrentPlugin = pPlugin;        // so plugin knows who it is
    CScriptCallInfo callinfo (sName, pPlugin->m_PluginCallbacks [sName]);
    pPlugin->ExecutePluginScriptRtn (callinfo, strResult);
    m_CurrentPlugin = pSavedPlugin;   // back to current plugin

    }   // end of doing each plugin

  } // end of CMUSHclientDoc::SendToAllPluginCallbacks


// this sends a number and a string to all plugins, and optionally stops on a true or false response
bool CMUSHclientDoc::SendToAllPluginCallbacks (const string & sName, 
                                               const long arg1,      // 2 arguments
                                               const string sText,
                                               const bool bStopOnTrue,
                                               const bool bStopOnFalse)
  {
  CPlugin * pSavedPlugin = m_CurrentPlugin;

  // tell a plugin the message
  for (PluginListIterator pit = m_PluginList.begin (); 
         pit != m_PluginList.end (); 
         ++pit)
    {
    CPlugin * pPlugin = *pit;

    if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
      continue;

    // change to this plugin, call function, put current plugin back
    m_CurrentPlugin = pPlugin;        // so plugin knows who it is
    CScriptCallInfo callinfo (sName, pPlugin->m_PluginCallbacks [sName]);
    bool bResult = pPlugin->ExecutePluginScript (callinfo, arg1, sText);
    m_CurrentPlugin = pSavedPlugin;  // back to current plugin

    if (bStopOnTrue && bResult && callinfo._dispid_info.isvalid ())
      return true;

    if (bStopOnFalse && !bResult && callinfo._dispid_info.isvalid ())
      return false;

    }   // end of doing each plugin


  if (bStopOnTrue)
    return false;
  else 
    return true;  // if they wanted to stop on true, assume false and vice-versa

  }  // end of CMUSHclientDoc::SendToAllPluginCallbacks

// this sends two numbers and a string to all plugins, and optionally stops on a true or false response
bool CMUSHclientDoc::SendToAllPluginCallbacks (const string & sName, 
                                               const long arg1,      // 3 arguments
                                               const long arg2,
                                               const string sText,
                                               const bool bStopOnTrue,
                                               const bool bStopOnFalse)
  {
  CPlugin * pSavedPlugin = m_CurrentPlugin;

  // tell a plugin the message
  for (PluginListIterator pit = m_PluginList.begin (); 
         pit != m_PluginList.end (); 
         ++pit)
    {
    CPlugin * pPlugin = *pit;

    if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
      continue;

    // change to this plugin, call function, put current plugin back
    m_CurrentPlugin = pPlugin;        // so plugin knows who it is
    CScriptCallInfo callinfo (sName, pPlugin->m_PluginCallbacks [sName]);
    bool bResult = pPlugin->ExecutePluginScript (callinfo, arg1, arg2, sText);
    m_CurrentPlugin = pSavedPlugin;   // back to current plugin

    if (bStopOnTrue && bResult && callinfo._dispid_info.isvalid ())
      return true;

    if (bStopOnFalse && !bResult && callinfo._dispid_info.isvalid ())
      return false;

    }   // end of doing each plugin

  if (bStopOnTrue)
    return false;
  else 
    return true;  // if they wanted to stop on true, assume false and vice-versa

  }  // end of CMUSHclientDoc::SendToAllPluginCallbacks
