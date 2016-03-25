// methods_plugins.cpp

// Plugins management

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\errors.h"

// Implements:

//    BroadcastPlugin
//    CallPlugin
//    EnablePlugin
//    GetPluginAliasInfo
//    GetPluginAliasList
//    GetPluginAliasOption
//    GetPluginID
//    GetPluginInfo
//    GetPluginList
//    GetPluginName
//    GetPluginTimerInfo
//    GetPluginTimerList
//    GetPluginTimerOption
//    GetPluginTriggerInfo
//    GetPluginTriggerList
//    GetPluginTriggerOption
//    GetPluginVariable
//    GetPluginVariableList
//    IsPluginInstalled
//    LoadPlugin
//    PluginSupports
//    ReloadPlugin
//    UnloadPlugin
//    SaveState

// gets our own plugin id

BSTR CMUSHclientDoc::GetPluginID() 
{
	CString strResult;

  if (m_CurrentPlugin)
    strResult = m_CurrentPlugin->m_strID;
  
  return strResult.AllocSysString();
}   // end of CMUSHclientDoc::GetPluginID

// gets our own plugin name

BSTR CMUSHclientDoc::GetPluginName() 
{
	CString strResult;

  if (m_CurrentPlugin)
    strResult = m_CurrentPlugin->m_strName;

	return strResult.AllocSysString();
}  // end of CMUSHclientDoc::GetPluginName

// get list of plugins

VARIANT CMUSHclientDoc::GetPluginList() 
{
  COleSafeArray sa;   // for variable list

  CString strVariableName;

  long iCount = 0;
  
  // put the plugins into the array
  if (!m_PluginList.empty ())    // cannot create empty dimension
    {
    sa.CreateOneDim (VT_VARIANT, m_PluginList.size ());

    for (PluginListIterator pit = m_PluginList.begin (); 
         pit != m_PluginList.end (); 
         ++pit)
      {
      CPlugin * p = *pit;

      // the array must be a bloody array of variants, or VBscript kicks up
      COleVariant v (p->m_strID);
      sa.PutElement (&iCount, &v);
      iCount++;
      }      // end of looping through each plugin
    } // end of having at least one

	return sa.Detach ();
}   // end of CMUSHclientDoc::GetPluginList

// get info about a plugin

VARIANT CMUSHclientDoc::GetPluginInfo(LPCTSTR PluginID, short InfoType) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_NULL;

  CPlugin * pPlugin = GetPlugin (PluginID);

  if (!pPlugin)
	  return vaResult;     // plugin not found

  switch (InfoType)
    {
    case   1: SetUpVariantString (vaResult, pPlugin->m_strName); break;
    case   2: SetUpVariantString (vaResult, pPlugin->m_strAuthor); break;
    case   3: SetUpVariantString (vaResult, pPlugin->m_strDescription); break;
    case   4: SetUpVariantString (vaResult, pPlugin->m_strScript); break;
    case   5: SetUpVariantString (vaResult, pPlugin->m_strLanguage); break;
    case   6: SetUpVariantString (vaResult, pPlugin->m_strSource); break;
    case   7: SetUpVariantString (vaResult, pPlugin->m_strID); break;
    case   8: SetUpVariantString (vaResult, pPlugin->m_strPurpose); break;
    case   9: SetUpVariantLong   (vaResult, pPlugin->m_TriggerMap.GetCount ()); break;
    case  10: SetUpVariantLong   (vaResult, pPlugin->m_AliasMap.GetCount ()); break;
    case  11: SetUpVariantLong   (vaResult, pPlugin->m_TimerMap.GetCount ()); break;
    case  12: SetUpVariantLong   (vaResult, pPlugin->m_VariableMap.GetCount ()); break;
    case  13: 
      if (pPlugin->m_tDateWritten.GetTime ())     // only if non-zero, otherwise return empty
        SetUpVariantDate   (vaResult, COleDateTime (pPlugin->m_tDateWritten.GetTime ())); 
      break;
    case  14: 
      if (pPlugin->m_tDateModified.GetTime ())     // only if non-zero, otherwise return empty
        SetUpVariantDate   (vaResult,  COleDateTime (pPlugin->m_tDateModified.GetTime ())); 
      break;
    case 15: SetUpVariantBool (vaResult, pPlugin->m_bSaveState); break;
        // 16: is scripting enabled?
    case 16: SetUpVariantBool (vaResult, pPlugin->m_ScriptEngine != NULL); break;
    case 17: SetUpVariantBool (vaResult, pPlugin->m_bEnabled); break;
    case 18: SetUpVariantDouble (vaResult, pPlugin->m_dRequiredVersion); break;
    case 19: SetUpVariantDouble (vaResult, pPlugin->m_dVersion); break;
    case 20: SetUpVariantString (vaResult, pPlugin->m_strDirectory); break;
    case 21:
      {
      int iCount = 0;

      // first work out what order each plugin is in *now*
      for (PluginListIterator pit = m_PluginList.begin (); 
           pit != m_PluginList.end (); 
           ++pit)
        (*pit)->m_iLoadOrder = ++iCount;

      // now return the order of *this* one
      SetUpVariantLong   (vaResult, pPlugin->m_iLoadOrder); 
      }
      break;

    case  22: SetUpVariantDate   (vaResult, COleDateTime (pPlugin->m_tDateInstalled.GetTime ()));  break;
    case  23: SetUpVariantString (vaResult, pPlugin->m_strCallingPluginID); break;

    case  24:
      {
      double elapsed_time = 0.0;
      if (App.m_iCounterFrequency > 0)
        elapsed_time = ((double) pPlugin->m_iScriptTimeTaken) / 
                       ((double) App.m_iCounterFrequency);

      SetUpVariantDouble (vaResult, elapsed_time); 
      break;
      }

    case 25: SetUpVariantShort (vaResult, pPlugin->m_iSequence); break;

    default:
      vaResult.vt = VT_NULL;
      break;

    } // end of switch

  return vaResult;
}   // end of CMUSHclientDoc::GetPluginInfo

// load a plugin from disk

long CMUSHclientDoc::LoadPlugin(LPCTSTR FileName) 
{

  CPlugin * pCurrentPlugin = m_CurrentPlugin;
  m_CurrentPlugin = NULL;   // otherwise plugin won't load if done from another one

  try
    {
    // load it
    InternalLoadPlugin (FileName);
    } // end of try block

  catch (CFileException * e)
    {
    e->Delete ();
    m_CurrentPlugin = pCurrentPlugin;
    return ePluginFileNotFound;
    } // end of catching a file exception

  catch (CArchiveException* e) 
    {
    e->Delete ();
    m_CurrentPlugin = pCurrentPlugin;
    return eProblemsLoadingPlugin;
    }

  m_CurrentPlugin = pCurrentPlugin;

  PluginListChanged ();

	return eOK;
}    // end of CMUSHclientDoc::LoadPlugin


long CMUSHclientDoc::ReloadPlugin(LPCTSTR PluginID) 
{

// first, find plugin by ID
CPlugin * pPlugin = GetPlugin (PluginID);

  // if not found, try to find by name
  if (pPlugin == NULL && strlen (PluginID) > 0)
    {
    PluginListIterator pit = find_if (m_PluginList.begin (),
                                     m_PluginList.end (),
                                     bind2nd (compare_plugin_name (), PluginID));
    if (pit != m_PluginList.end ())
       pPlugin = *pit;
       
    }    

  if (pPlugin == NULL)
    return eNoSuchPlugin;

  // cannot reload  ourselves
  if (pPlugin == m_CurrentPlugin)
    return eBadParameter;

  PluginListIterator pit = find (m_PluginList.begin (), 
                                 m_PluginList.end (), 
                                 pPlugin);
 
  if (pit == m_PluginList.end () )
    return eNoSuchPlugin;

  CString strName = pPlugin->m_strSource;
  m_PluginList.erase (pit);  // remove from list
  delete pPlugin;   // delete the plugin

  CPlugin * pCurrentPlugin = m_CurrentPlugin;
  m_CurrentPlugin = NULL;   // otherwise plugin won't load if done from another one

  try
    {
    // now reload it
    InternalLoadPlugin (strName);
    } // end of try block

  catch (CFileException * e)
    {
    e->Delete ();
    m_CurrentPlugin = pCurrentPlugin;
    return ePluginFileNotFound;
    } // end of catching a file exception

  catch (CArchiveException* e) 
    {
    e->Delete ();
    m_CurrentPlugin = pCurrentPlugin;
    return eProblemsLoadingPlugin;
    }

  m_CurrentPlugin = pCurrentPlugin;

  PluginListChanged ();

	return eOK;
}  // end of CMUSHclientDoc::ReloadPlugin

// helper routine to find a particular plugin

CPlugin * CMUSHclientDoc::GetPlugin (LPCTSTR PluginID)
  {

  // return if id wrong length
  if (strlen (PluginID) != PLUGIN_UNIQUE_ID_LENGTH)
    return NULL;

  // cater for obscure case where plugin isn't in list
  // (eg. we are closing it)

  if (m_CurrentPlugin && 
      m_CurrentPlugin->m_strID.CompareNoCase (PluginID) == 0)
    return m_CurrentPlugin;

  PluginListIterator pit = find_if (m_PluginList.begin (),
                                    m_PluginList.end (),
                                    bind2nd (compare_plugin_id (), PluginID));
  if (pit != m_PluginList.end ())
     return *pit;

  return NULL;  // not found

  } // end of CMUSHclientDoc::GetPlugin

// Helper routine for getting stuff for other plugins.
//  NB - if PluginID is the null string, use global things

#define GET_PLUGIN_STUFF(what_to_do)  \
	VARIANT vaResult;           \
	VariantInit(&vaResult);     \
  vaResult.vt = VT_NULL;      \
  CPlugin * pPlugin = NULL;   \
  if (strlen (PluginID) > 0)  \
    {                         \
    pPlugin = GetPlugin (PluginID); \
    if (!pPlugin)             \
	    return vaResult;        \
    }                         \
  CPlugin * pOldPlugin = m_CurrentPlugin;  \
  m_CurrentPlugin = pPlugin;               \
  vaResult = what_to_do;                   \
  m_CurrentPlugin = pOldPlugin;            \
	return vaResult;                       


// get variable for a particular plugin

VARIANT CMUSHclientDoc::GetPluginVariable(LPCTSTR PluginID, LPCTSTR VariableName) 
{
GET_PLUGIN_STUFF (GetVariable (VariableName))
}   // end of CMUSHclientDoc::GetPluginVariable

// get variable list for a particular plugin

VARIANT CMUSHclientDoc::GetPluginVariableList(LPCTSTR PluginID) 
{
GET_PLUGIN_STUFF (GetVariableList ())
}   // end of CMUSHclientDoc::GetPluginVariableList

// get trigger list for a particular plugin

VARIANT CMUSHclientDoc::GetPluginTriggerList(LPCTSTR PluginID) 
{
GET_PLUGIN_STUFF (GetTriggerList ())
}    // end of CMUSHclientDoc::GetPluginTriggerList

// get alias list for a particular plugin

VARIANT CMUSHclientDoc::GetPluginAliasList(LPCTSTR PluginID) 
{
GET_PLUGIN_STUFF (GetAliasList ())
}     // end of CMUSHclientDoc::GetPluginAliasList

// get timer list for a particular plugin

VARIANT CMUSHclientDoc::GetPluginTimerList(LPCTSTR PluginID) 
{
GET_PLUGIN_STUFF (GetTimerList ())
}    // end of CMUSHclientDoc::GetPluginTimerList

// get trigger info for a particular plugin

VARIANT CMUSHclientDoc::GetPluginTriggerInfo(LPCTSTR PluginID, LPCTSTR TriggerName, short InfoType) 
{
GET_PLUGIN_STUFF (GetTriggerInfo (TriggerName, InfoType))
}    // end of CMUSHclientDoc::GetPluginTriggerInfo

// get alias info for a particular plugin

VARIANT CMUSHclientDoc::GetPluginAliasInfo(LPCTSTR PluginID, LPCTSTR AliasName, short InfoType) 
{
GET_PLUGIN_STUFF (GetAliasInfo (AliasName, InfoType))
}    // end of CMUSHclientDoc::GetPluginAliasInfo

// get timer info for a particular plugin

VARIANT CMUSHclientDoc::GetPluginTimerInfo(LPCTSTR PluginID, LPCTSTR TimerName, short InfoType) 
{
GET_PLUGIN_STUFF (GetTimerInfo (TimerName, InfoType))
}     // end of CMUSHclientDoc::GetPluginTimerInfo

// call a routine in a plugin, eg.
//    world.CallPlugin "982581e59ab42844527eec80", "Log_And_Send", "blah"

long CMUSHclientDoc::CallPlugin(LPCTSTR PluginID, LPCTSTR Routine, LPCTSTR Argument) 
  {

CPlugin * pPlugin = GetPlugin (PluginID); 

  if (!pPlugin)                            
	  return eNoSuchPlugin;                       

  if (strlen (Routine) == 0 || pPlugin->m_ScriptEngine == NULL)
    return eNoSuchRoutine;

  if (!pPlugin->m_bEnabled)
    return ePluginDisabled;

DISPID iDispid = pPlugin->m_ScriptEngine->GetDispid (Routine);

  if (iDispid == DISPID_UNKNOWN)
    return eNoSuchRoutine;

long nInvocationCount = 0;

  CString strOldCallingPluginID = pPlugin->m_strCallingPluginID;

  pPlugin->m_strCallingPluginID.Empty ();
  
  if (m_CurrentPlugin)
    pPlugin->m_strCallingPluginID = m_CurrentPlugin->m_strID;

  // do this so plugin can find its own state (eg. with GetPluginID)
  CPlugin * pSavedPlugin = m_CurrentPlugin; 
  m_CurrentPlugin = pPlugin;   

  CString strType = TFormat ("Plugin %s", (LPCTSTR) pPlugin->m_strName); 
  CString strReason = TFormat ("Executing plugin %s sub %s", 
                               (LPCTSTR) pPlugin->m_strName,
                               Routine ); 
      
  if (pPlugin->m_ScriptEngine->IsLua ())
    {
    list<double> nparams;
    list<string> sparams;
    sparams.push_back (Argument);
    pPlugin->m_ScriptEngine->ExecuteLua (iDispid, 
                                         Routine, 
                                         eDontChangeAction,
                                         strType, 
                                         strReason, 
                                         nparams,
                                         sparams, 
                                         nInvocationCount); 
    }   // end of Lua
  else
    {
  // WARNING - arguments should appear in REVERSE order to what the sub expects them!

    enum
      {
      eArgument,
      eArgCount,     // this MUST be last
      };    

    COleVariant args [eArgCount];
    DISPPARAMS params = { args, NULL, eArgCount, 0 };

    args [eArgument] = Argument;


    pPlugin->m_ScriptEngine->Execute (iDispid, 
                             Routine, 
                             eDontChangeAction,
                             strType,
                             strReason,                             
                             params, 
                             nInvocationCount, 
                             NULL);
    } // not Lua

  m_CurrentPlugin = pSavedPlugin;

  pPlugin->m_strCallingPluginID = strOldCallingPluginID;

  if (iDispid == DISPID_UNKNOWN)
    return eErrorCallingPluginRoutine;

	return eOK;
}    // end of CMUSHclientDoc::CallPlugin

// does a plugin support a routine, eg.
//  i = world.PluginSupports ("982581e59ab42844527eec80", "Log_And_Send")

long CMUSHclientDoc::PluginSupports(LPCTSTR PluginID, LPCTSTR Routine) 
{
CPlugin * pPlugin = GetPlugin (PluginID); 

  if (!pPlugin)                            
	  return eNoSuchPlugin;                       

  if (strlen (Routine) == 0)
    return eNoSuchRoutine;

DISPID iDispid = pPlugin->m_ScriptEngine->GetDispid (Routine);

  if (iDispid == DISPID_UNKNOWN)
    return eNoSuchRoutine;

	return eOK;
}   // end ofCMUSHclientDoc::PluginSupports

// is a plugin installed? eg
//  i = world.IsPluginInstalled ("982581e59ab42844527eec80")

BOOL CMUSHclientDoc::IsPluginInstalled(LPCTSTR PluginID) 
{
return GetPlugin (PluginID) != NULL;
}    // end of CMUSHclientDoc::IsPluginInstalled

// if I am a plugin, save my state

long CMUSHclientDoc::SaveState() 
{

  if (!m_CurrentPlugin)                            
	  return eNotAPlugin;                       

  // if we are already saving the state, don't do it again
  if (m_CurrentPlugin->m_bSavingStateNow) 
    return ePluginCouldNotSaveState;

  // save the state
  if (m_CurrentPlugin->SaveState (true))
    return ePluginCouldNotSaveState;

  return eOK;
}   // end of CMUSHclientDoc::SaveState


long CMUSHclientDoc::EnablePlugin(LPCTSTR PluginID, BOOL Enabled) 
{
CPlugin * pPlugin = GetPlugin (PluginID); 

  if (!pPlugin)                            
	  return eNoSuchPlugin;                       

  if (pPlugin->m_bEnabled == (Enabled != 0))
    return eOK;   // already same state

  pPlugin->m_bEnabled = Enabled != 0;

  CPlugin * pSavedPlugin = m_CurrentPlugin;

  // otherwise plugin won't know who itself is
  m_CurrentPlugin = pPlugin;

  if (pPlugin->m_bEnabled)
    {
    CScriptCallInfo callinfo (ON_PLUGIN_ENABLE, pPlugin->m_PluginCallbacks [ON_PLUGIN_ENABLE]);
    pPlugin->ExecutePluginScript (callinfo); 
    }
  else
    {
    CScriptCallInfo callinfo (ON_PLUGIN_DISABLE, pPlugin->m_PluginCallbacks [ON_PLUGIN_DISABLE]);
    pPlugin->ExecutePluginScript (callinfo); 
    }
  
  m_CurrentPlugin = pSavedPlugin;

  return eOK;
}   // end of CMUSHclientDoc::EnablePlugin


VARIANT CMUSHclientDoc::GetPluginTriggerOption(LPCTSTR PluginID, LPCTSTR TriggerName, LPCTSTR OptionName) 
{
GET_PLUGIN_STUFF (GetTriggerOption (TriggerName, OptionName))
}   // end of CMUSHclientDoc::GetPluginTriggerOption

VARIANT CMUSHclientDoc::GetPluginAliasOption(LPCTSTR PluginID, LPCTSTR AliasName, LPCTSTR OptionName) 
{
GET_PLUGIN_STUFF (GetAliasOption (AliasName, OptionName))
}      // end of CMUSHclientDoc::GetPluginAliasOption

VARIANT CMUSHclientDoc::GetPluginTimerOption(LPCTSTR PluginID, LPCTSTR TimerName, LPCTSTR OptionName) 
{
GET_PLUGIN_STUFF (GetTimerOption (TimerName, OptionName))
}     // end of  CMUSHclientDoc::GetPluginTimerOption


long CMUSHclientDoc::BroadcastPlugin(long Message, LPCTSTR Text) 
{
  CPlugin * pSavedPlugin = m_CurrentPlugin;
  long iCount = 0;

  CString strCurrentID;
  CString strCurrentName;

  if (pSavedPlugin)
    {
     strCurrentID = pSavedPlugin->m_strID;
     strCurrentName = pSavedPlugin->m_strName;
    }

  // tell a plugin the message
  for (PluginListIterator pit = m_PluginList.begin (); 
       pit != m_PluginList.end (); 
       ++pit)
    {
    CPlugin * pPlugin = *pit;

    if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
      continue;

    // don't broadcast to ourselves - could cause indefinite loop
    if (pPlugin == pSavedPlugin)
      continue;

    CScriptCallInfo callinfo (ON_PLUGIN_BROADCAST, pPlugin->m_PluginCallbacks [ON_PLUGIN_BROADCAST]);
    m_CurrentPlugin = pPlugin;

    // see what the plugin makes of this,
    pPlugin->ExecutePluginScript (callinfo,
                                  Message, 
                                  (LPCTSTR) strCurrentID,
                                  (LPCTSTR) strCurrentName,
                                  Text); 

    if (callinfo._dispid_info.isvalid ())
      iCount++;

    }   // end of doing each plugin

  m_CurrentPlugin = pSavedPlugin;

	return iCount;
}  // end of CMUSHclientDoc::BroadcastPlugin



long CMUSHclientDoc::UnloadPlugin(LPCTSTR PluginID) 
{
// first, find plugin by ID
CPlugin * pPlugin = GetPlugin (PluginID);

  // if not found, try to find by name
  if (pPlugin == NULL && strlen (PluginID) > 0)
    {
    PluginListIterator pit = find_if (m_PluginList.begin (),
                                     m_PluginList.end (),
                                     bind2nd (compare_plugin_name (), PluginID));
    if (pit != m_PluginList.end ())
       pPlugin = *pit;
       
    }    

  if (pPlugin == NULL)
    return eNoSuchPlugin;

  // cannot delete  ourselves
  if (pPlugin == m_CurrentPlugin)
    return eBadParameter;

  PluginListIterator pit = find (m_PluginList.begin (), 
                                 m_PluginList.end (), 
                                 pPlugin);
 
  if (pit == m_PluginList.end () )
    return eNoSuchPlugin;

  m_PluginList.erase (pit);  // remove from list
  delete pPlugin;   // delete the plugin

  PluginListChanged ();
  SetModifiedFlag (TRUE);   // document has now changed

	return eOK;
  
  }
