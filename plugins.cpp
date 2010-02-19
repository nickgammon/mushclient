#include "stdafx.h"
#include "MUSHclient.h"

#include "doc.h"
#include "dialogs\plugins\PluginWizardSheet.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

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

  // no dispatch IDs yet
  m_dispid_plugin_install       = DISPID_UNKNOWN;
  m_dispid_plugin_connect       = DISPID_UNKNOWN;
  m_dispid_plugin_disconnect    = DISPID_UNKNOWN;
  m_dispid_plugin_close         = DISPID_UNKNOWN;
  m_dispid_plugin_save_state    = DISPID_UNKNOWN;
  m_dispid_plugin_enable        = DISPID_UNKNOWN;
  m_dispid_plugin_disable       = DISPID_UNKNOWN; 
  m_dispid_plugin_command       = DISPID_UNKNOWN;
  m_dispid_plugin_command_entered       = DISPID_UNKNOWN;
  m_dispid_plugin_get_focus     = DISPID_UNKNOWN;
  m_dispid_plugin_lose_focus    = DISPID_UNKNOWN;
  m_dispid_plugin_trace         = DISPID_UNKNOWN;
  m_dispid_plugin_broadcast     = DISPID_UNKNOWN;
  m_dispid_plugin_screendraw    = DISPID_UNKNOWN;
  m_dispid_plugin_playsound     = DISPID_UNKNOWN;
  m_dispid_plugin_tabcomplete   = DISPID_UNKNOWN;
//  m_dispid_plugin_tooltip       = DISPID_UNKNOWN;
  m_dispid_plugin_tick          = DISPID_UNKNOWN;
  m_dispid_plugin_mouse_moved   = DISPID_UNKNOWN;

  m_dispid_plugin_send          = DISPID_UNKNOWN;
  m_dispid_plugin_sent          = DISPID_UNKNOWN;
  m_dispid_plugin_line_received = DISPID_UNKNOWN;
  m_dispid_plugin_packet_received= DISPID_UNKNOWN;
  m_dispid_plugin_telnet_option  = DISPID_UNKNOWN;
  m_dispid_plugin_telnet_request = DISPID_UNKNOWN;
  m_dispid_plugin_telnet_subnegotiation = DISPID_UNKNOWN;
  m_dispid_plugin_IAC_GA         = DISPID_UNKNOWN;
  m_dispid_plugin_partial_line          = DISPID_UNKNOWN;
  m_dispid_plugin_on_world_output_resized = DISPID_UNKNOWN;
  m_dispid_plugin_on_command_changed = DISPID_UNKNOWN;

  m_dispid_plugin_OnMXP_Start     = DISPID_UNKNOWN;      
  m_dispid_plugin_OnMXP_Stop      = DISPID_UNKNOWN;       
  m_dispid_plugin_OnMXP_OpenTag   = DISPID_UNKNOWN;    
  m_dispid_plugin_OnMXP_CloseTag  = DISPID_UNKNOWN;   
  m_dispid_plugin_OnMXP_SetVariable = DISPID_UNKNOWN;
  m_dispid_plugin_OnMXP_SetEntity = DISPID_UNKNOWN;
  m_dispid_plugin_OnMXP_Error     = DISPID_UNKNOWN;      

  m_dispid_plugin_On_Chat_Accept  = DISPID_UNKNOWN;
  m_dispid_plugin_On_Chat_Message = DISPID_UNKNOWN;
  m_dispid_plugin_On_Chat_MessageOut = DISPID_UNKNOWN;
  m_dispid_plugin_On_Chat_Display = DISPID_UNKNOWN;

  } // end of constructor

// destructor
CPlugin::~CPlugin () 
  {
  CPlugin * pSavedPlugin = m_pDoc->m_CurrentPlugin;    
  ExecutePluginScript (ON_PLUGIN_CLOSE, m_dispid_plugin_close);
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

void CPlugin::ExecutePluginScript (const char * sName, DISPID & iRoutine)
  {

  if (m_ScriptEngine && iRoutine != DISPID_UNKNOWN)
    {
    // do this so plugin can find its own state (eg. with GetPluginID)
    CPlugin * pSavedPlugin = m_pDoc->m_CurrentPlugin; 
    m_pDoc->m_CurrentPlugin = this;   

    long nInvocationCount = 0;

    CString strType = TFormat ("Plugin %s", 
                               (LPCTSTR) m_strName);
    CString strReason = TFormat ("Executing plugin %s sub %s", 
                                 (LPCTSTR) m_strName,
                                 sName );

    if (m_ScriptEngine->IsLua ())
      {
      list<double> nparams;
      list<string> sparams;
      m_ScriptEngine->ExecuteLua (iRoutine, 
                                   sName,  
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
                               sName, 
                               eDontChangeAction,
                               strType,
                               strReason,
                               params, 
                               nInvocationCount, NULL);
      }
    m_pDoc->m_CurrentPlugin = pSavedPlugin;
    }   // end of having a script engine


  } // end of CPlugin::ExecutePluginScript 


bool CPlugin::ExecutePluginScript (const char * sName, 
                                   DISPID & iRoutine, 
                                   const char * sText)
  {

  if (m_ScriptEngine && iRoutine != DISPID_UNKNOWN)
    {
    // do this so plugin can find its own state (eg. with GetPluginID)
    CPlugin * pSavedPlugin = m_pDoc->m_CurrentPlugin; 
    m_pDoc->m_CurrentPlugin = this;   

    
    long nInvocationCount = 0;

    CString strType = TFormat ("Plugin %s", 
                               (LPCTSTR) m_strName);

    CString strReason = TFormat ("Executing plugin %s sub %s", 
                                 (LPCTSTR) m_strName,
                                 sName );

    if (m_ScriptEngine->IsLua ())
      {
      list<double> nparams;
      list<string> sparams;
      sparams.push_back (sText);
      bool result;
      m_ScriptEngine->ExecuteLua (iRoutine, 
                                   sName, 
                                   eDontChangeAction,
                                   strType, 
                                   strReason,
                                   nparams,
                                   sparams, 
                                   nInvocationCount,
                                   NULL, NULL, NULL,
                                   &result);
      m_pDoc->m_CurrentPlugin = pSavedPlugin;
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
                               sName, 
                               eDontChangeAction,
                               strType,
                               strReason,
                               params, 
                               nInvocationCount, 
                               &result);

      m_pDoc->m_CurrentPlugin = pSavedPlugin;

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

bool CPlugin::ExecutePluginScript (const char * sName, 
                                  DISPID & iRoutine, 
                                  const long arg1,
                                  const string sText)
  {
  if (m_ScriptEngine && iRoutine != DISPID_UNKNOWN)
    {
    // do this so plugin can find its own state (eg. with GetPluginID)
    CPlugin * pSavedPlugin = m_pDoc->m_CurrentPlugin; 
    m_pDoc->m_CurrentPlugin = this;   

    long nInvocationCount = 0;


    CString strType = TFormat ("Plugin %s", (LPCTSTR) m_strName);
    CString strReason =  TFormat ("Executing plugin %s sub %s", 
                                       (LPCTSTR) m_strName,
                                       sName );

    if (m_ScriptEngine && m_ScriptEngine->IsLua ())
      {
      bool bResult;
      list<double> nparams;
      list<string> sparams;
      nparams.push_back (arg1);
      sparams.push_back (sText);
      m_ScriptEngine->ExecuteLua (iRoutine, 
                                  sName, 
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
    v.bstrVal = CString (sText.c_str (), sText.size ()).AllocSysString (); 
    args [eText].Attach (v);

    
    COleVariant result;

    m_ScriptEngine->Execute (iRoutine, 
                             sName, 
                             eDontChangeAction,
                             strType,
                             strReason,
                             params, 
                             nInvocationCount, 
                             &result);

    m_pDoc->m_CurrentPlugin = pSavedPlugin;

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

bool CPlugin::ExecutePluginScript (const char * sName, 
                                  DISPID & iRoutine, 
                                  const long arg1,
                                  const long arg2,
                                  const string sText)
  {
  if (m_ScriptEngine && iRoutine != DISPID_UNKNOWN)
    {
    long nInvocationCount = 0;

    // do this so plugin can find its own state (eg. with GetPluginID)
    CPlugin * pSavedPlugin = m_pDoc->m_CurrentPlugin; 
    m_pDoc->m_CurrentPlugin = this;   


    CString strType = TFormat ("Plugin %s", (LPCTSTR) m_strName);
    CString strReason =  TFormat ("Executing plugin %s sub %s", 
                                       (LPCTSTR) m_strName,
                                       sName );

    if (m_ScriptEngine && m_ScriptEngine->IsLua ())
      {
      bool bResult;
      list<double> nparams;
      list<string> sparams;
      nparams.push_back (arg1);
      nparams.push_back (arg2);
      sparams.push_back (sText);
      m_ScriptEngine->ExecuteLua (iRoutine, 
                                  sName, 
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
                             sName, 
                             eDontChangeAction,
                             strType,
                             strReason,
                             params, 
                             nInvocationCount, 
                             &result);

    m_pDoc->m_CurrentPlugin = pSavedPlugin;

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


bool CPlugin::ExecutePluginScript (const char * sName, 
                                  DISPID & iRoutine, 
                                  const long arg1,
                                  const char * arg2,
                                  const char * arg3,
                                  const char * arg4)
  {
  if (m_ScriptEngine && iRoutine != DISPID_UNKNOWN)
    {
    long nInvocationCount = 0;

    // do this so plugin can find its own state (eg. with GetPluginID)
    CPlugin * pSavedPlugin = m_pDoc->m_CurrentPlugin; 
    m_pDoc->m_CurrentPlugin = this;   


    CString strType = TFormat ("Plugin %s", (LPCTSTR) m_strName);
    CString strReason =  TFormat ("Executing plugin %s sub %s", 
                                       (LPCTSTR) m_strName,
                                       sName );

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
                                  sName, 
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
                             sName, 
                             eDontChangeAction,
                             strType,
                             strReason,
                             params, 
                             nInvocationCount, 
                             &result);

    m_pDoc->m_CurrentPlugin = pSavedPlugin;

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

void CPlugin::ExecutePluginScript (const char * sName, 
                                   CString & strText,
                                   DISPID & iRoutine) 
  {

  if (m_ScriptEngine && iRoutine != DISPID_UNKNOWN)
    {
    // do this so plugin can find its own state (eg. with GetPluginID)
    CPlugin * pSavedPlugin = m_pDoc->m_CurrentPlugin; 
    m_pDoc->m_CurrentPlugin = this;   

    long nInvocationCount = 0;

    CString strType = TFormat ("Plugin %s", 
                               (LPCTSTR) m_strName);

    CString strReason = TFormat ("Executing plugin %s sub %s", 
                                 (LPCTSTR) m_strName,
                                 sName );

    if (m_ScriptEngine->IsLua ())
      {
      m_ScriptEngine->ExecuteLua (iRoutine,
                                  sName,
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
                               sName, 
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

    m_pDoc->m_CurrentPlugin = pSavedPlugin;

    }   // end of having a script engine

  } // end of CPlugin::ExecutePluginScript 

bool CPlugin::SaveState (void)
  {

  if (!m_bSaveState)
    return true;   // not needed
  
CString strFilename = CString (Make_Absolute_Path (App.m_strPluginsDirectory)) + "state\\";
CFile * f = NULL;
CArchive * ar = NULL;
bool bError = true;
  
// need a directory
          
  if (App.m_strPluginsDirectory.IsEmpty ())
    return true;

// need a world id

  if (m_pDoc->m_strWorldID.IsEmpty ())
    return true;

  ExecutePluginScript (ON_PLUGIN_SAVE_STATE, m_dispid_plugin_save_state);

  strFilename += m_pDoc->m_strWorldID;    // world ID
  strFilename += "-";
  strFilename += m_strID;                 // plugin ID
  strFilename += "-state.xml";            // suffix

  CPlugin * oldPlugin = m_pDoc->m_CurrentPlugin;

  try
    {
    f = new CFile (strFilename, 
                    CFile::modeCreate | CFile::modeReadWrite);

    ar = new CArchive(f, CArchive::store);

    CString strComment = TFormat ("Plugin state saved. Plugin: \"%s\". World: \"%s\".",
                (LPCTSTR) m_strName,
                (LPCTSTR) m_pDoc->m_mush_name);

    m_pDoc->m_CurrentPlugin = this;
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
  	CFileDialog	dlg(FALSE,						// FALSE for FileSave
					  "xml",						// default extension
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

    CFile	f (strFilename, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive);
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

}

