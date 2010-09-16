// xml_load_world.cpp : XML world serialization (loading)
//

#include "stdafx.h"
#include "..\MUSHclient.h"
#include "..\doc.h"
#include "..\MUSHview.h"
#include "..\genprint.h"
#include "..\mainfrm.h"

/*

Basic expected document structure ...

<?xml version="1.0" encoding="US-ASCII"?>
<!DOCTYPE muclient>

<muclient>
<world
  ... world single-line attributes (eg. ip address) ...
>

  ... world multi-line tags (eg. <connect_text> ... </connect_text> ...

</world>

<triggers>

  ... individual triggers here ... (eg. <trigger> ... </trigger> )

</triggers>

<aliases>

  ... individual aliases here ... (eg. <alias> ... </alias> )

</aliases>

<timers>

  ... individual timers here ... (eg. <timer> ... </timer> )

</timers>

<macros>

  ... individual macros here ... (eg. <macro> ... </macro> )

</macros>

<variables>

  ... individual variables here ... (eg. <variable name="blah">contents</variable> )

</variables>

<colours>
 <ansi>
   <normal>
    ... individual colours here ... (eg. <colour ... /> )
   </normal
   <bold>
    ... individual colours here ... (eg. <colour ... /> )
   </bold
 </ansi>
 <custom>
    ... individual colours here ... (eg. <colour ... />  )
 </custom>
</colours>

<keypad>

  ... individual keys here ... (eg. <keys> ... </keys> )

</keypad>

<printing>
 <ansi>
   <normal>
    ... individual styles here ... (eg. <style ... /> )
   </normal
   <bold>
    ... individual styles here ... (eg. <style ... /> )
   </bold
 </ansi>
</printing>

</muclient>

The order isn't important, as long as everything stays well-formed (XML-wise).

Sections can be omitted, and if no <muclient> tag is found we start looking at
the root level. ie.

  You can start like this:  <triggers> blah blah </triggers>

*/

#define O(arg) offsetof (CPlugin, arg)

tPluginCallbackNames PluginCallbacksTable [] = {

//{ ON_PLUGIN_TOOLTIP,              O(m_dispid_plugin_tooltip) },
  
{ ON_PLUGIN_BROADCAST,              O(m_dispid_plugin_broadcast) }, 
      
{ ON_PLUGIN_CHAT_ACCEPT,            O(m_dispid_plugin_On_Chat_Accept) },        
{ ON_PLUGIN_CHAT_DISPLAY,           O(m_dispid_plugin_On_Chat_Display) },        
{ ON_PLUGIN_CHAT_MESSAGE,           O(m_dispid_plugin_On_Chat_Message) },        
{ ON_PLUGIN_CHAT_MESSAGE_OUT,       O(m_dispid_plugin_On_Chat_MessageOut) },        
{ ON_PLUGIN_CHAT_NEWUSER,           O(m_dispid_plugin_On_Chat_NewUser) },        
{ ON_PLUGIN_CHAT_USERDISCONNECT,    O(m_dispid_plugin_On_Chat_UserDisconnect) },  
      
{ ON_PLUGIN_CLOSE,                  O(m_dispid_plugin_close) },        
{ ON_PLUGIN_COMMAND,                O(m_dispid_plugin_command) },        
{ ON_PLUGIN_COMMAND_CHANGED,        O(m_dispid_plugin_on_command_changed) },        
{ ON_PLUGIN_COMMAND_ENTERED,        O(m_dispid_plugin_command_entered) },        
{ ON_PLUGIN_CONNECT,                O(m_dispid_plugin_connect) },        
{ ON_PLUGIN_DISABLE,                O(m_dispid_plugin_disable) },        
{ ON_PLUGIN_DISCONNECT,             O(m_dispid_plugin_disconnect) },        
{ ON_PLUGIN_ENABLE,                 O(m_dispid_plugin_enable) },        
{ ON_PLUGIN_GETFOCUS,               O(m_dispid_plugin_get_focus) },        
{ ON_PLUGIN_IAC_GA,                 O(m_dispid_plugin_IAC_GA) },        
{ ON_PLUGIN_INSTALL,                O(m_dispid_plugin_install) },        
{ ON_PLUGIN_LINE_RECEIVED,          O(m_dispid_plugin_line_received) },        
{ ON_PLUGIN_LIST_CHANGED,           O(m_dispid_plugin_list_changed) },        
{ ON_PLUGIN_LOSEFOCUS,              O(m_dispid_plugin_lose_focus) },        
{ ON_PLUGIN_MOUSE_MOVED,            O(m_dispid_plugin_mouse_moved) },        

{ ON_PLUGIN_MXP_CLOSETAG,           O(m_dispid_plugin_OnMXP_CloseTag) },        
{ ON_PLUGIN_MXP_ERROR,              O(m_dispid_plugin_OnMXP_Error) },        
{ ON_PLUGIN_MXP_OPENTAG,            O(m_dispid_plugin_OnMXP_OpenTag) },        
{ ON_PLUGIN_MXP_SETENTITY,          O(m_dispid_plugin_OnMXP_SetEntity) },        
{ ON_PLUGIN_MXP_SETVARIABLE,        O(m_dispid_plugin_OnMXP_SetVariable) },        
{ ON_PLUGIN_MXP_START,              O(m_dispid_plugin_OnMXP_Start) },        
{ ON_PLUGIN_MXP_STOP,               O(m_dispid_plugin_OnMXP_Stop) },        

{ ON_PLUGIN_PACKET_RECEIVED,        O(m_dispid_plugin_packet_received) },        
{ ON_PLUGIN_PARTIAL_LINE,           O(m_dispid_plugin_partial_line) },        
{ ON_PLUGIN_PLAYSOUND,              O(m_dispid_plugin_playsound) },        
{ ON_PLUGIN_SAVE_STATE,             O(m_dispid_plugin_save_state) },        
{ ON_PLUGIN_SCREENDRAW,             O(m_dispid_plugin_screendraw) },        
{ ON_PLUGIN_SEND,                   O(m_dispid_plugin_send) },        
{ ON_PLUGIN_SENT,                   O(m_dispid_plugin_sent) },        
{ ON_PLUGIN_TABCOMPLETE,            O(m_dispid_plugin_tabcomplete) },        

{ ON_PLUGIN_TELNET_OPTION,          O(m_dispid_plugin_telnet_option) },        
{ ON_PLUGIN_TELNET_REQUEST,         O(m_dispid_plugin_telnet_request) },        
{ ON_PLUGIN_TELNET_SUBNEGOTIATION,  O(m_dispid_plugin_telnet_subnegotiation) },        

{ ON_PLUGIN_TICK,                   O(m_dispid_plugin_tick) },        
{ ON_PLUGIN_TRACE,                  O(m_dispid_plugin_trace) },        
{ ON_PLUGIN_WORLD_OUTPUT_RESIZED,   O(m_dispid_plugin_on_world_output_resized) },        
{ ON_PLUGIN_WORLD_SAVE,             O(m_dispid_plugin_world_save) },        

{NULL}   // end of table marker            

  };  // end of PluginCallbacksTable 


extern tConfigurationNumericOption OptionsTable [];
extern tConfigurationAlphaOption AlphaOptionsTable [];
extern UINT iLineLastItemFound;
UINT iErrorCount;

#define DEFAULTS_LIT "defaults"
#define CLIPBOARD_NAME "Clipboard"

// helper macros for getting values and passing down whether we want defaults or not

#define GET_BOOL_USHORT(name, var)  \
        Get_XML_boolean_ushort (node, name, var, bUseDefault)

#define GET_BOOL(name, var)  \
        Get_XML_boolean (node, name, var, bUseDefault)

#define GET_BOOL_BOOL(name, var)  \
        Get_XML_boolean_BOOL (node, name, var, bUseDefault)

#define GET_BOOL_USHORT_MASK(name, var, mask)  \
        Get_XML_boolean_ushort_mask (node, name, var, mask, bUseDefault)

#define GET_BOOL_INT_MASK(name, var, mask)  \
        Get_XML_boolean_int_mask (node, name, var, mask, bUseDefault)

#define GET_STRING(name, var)  \
        Get_XML_string (node, name, var, bUseDefault)

#define GET_STRING_TRIM(name, var) \
        Get_XML_string (node, name, var, bUseDefault, true)

#define GET_USHORT(name, var, lo, hi) \
        Get_XML_ushort (node, name, var, bUseDefault, lo, hi)

#define GET_INT(name, var, lo, hi) \
        Get_XML_int (node, name, var, bUseDefault, lo, hi)

#define GET_LONG(name, var, lo, hi) \
        Get_XML_long (node, name, var, bUseDefault, lo, hi)

#define GET_DOUBLE(name, var, lo, hi) \
        Get_XML_double (node, name, var, bUseDefault, lo, hi)

#define GET_COLOUR(name, var) \
        Get_XML_colour (node, name, var, bUseDefault)

// these load loops are to reduce tedium in doing loops within loops
// eg. for triggers we might have multiple <triggers> tags, and inside
//     each one, multiple <trigger> tags.

#define LOAD_LOOP(parent, type, node)  \
  for (POSITION pos = (parent).ChildrenList.GetHeadPosition (); pos; )     \
    {                                \
    CXMLelement * node = (parent).ChildrenList.GetNext (pos);    \
    if (node->strName == type && !node->bUsed)        \
      {                    \
      node->bUsed = true; \
      iLineLastItemFound = node->iLine;

#define END_LOAD_LOOP } }

#define GET_VERSION_AND_DEFAULTS(node) \
  long iVersion = GetDateAndVersion (*node); \
  bool bUseDefault;  \
  Get_XML_boolean (*node, DEFAULTS_LIT, bUseDefault, false); \
  if (iFlags & LOAD_INCLUDE)  \
     bUseDefault = true;


#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static CString strFileName;

// get date and version to avoid "not used" warnings

long GetDateAndVersion (CXMLelement & node)
  {
CString strVersion, 
        strDate;
long iVersion = VERSION;    // default version is current one
  
  Get_XML_number (node, "world_file_version", iVersion, true, 0); // lower limit zero
  Get_XML_string (node, "muclient_version", strVersion, true);
  Get_XML_string (node, "date_saved", strDate, true);

  return iVersion;
  } // end of GetDateAndVersion

void CMUSHclientDoc::HandleLoadException (const char * sComment, CException* e)
  {
  char sMessage [1000];
  e->GetErrorMessage (sMessage, sizeof sMessage);
  e->Delete ();
  LoadError (sComment, sMessage);
  }

#define TIMER_TEST 0

#if TIMER_TEST

#define START_TIMING QueryPerformanceCounter (&start)

#define STOP_TIMING(what) \
  do  { \
  LONGLONG iTimeTaken;    \
  QueryPerformanceCounter (&finish);  \
  iTimeTaken = finish.QuadPart - start.QuadPart;  \
  double fTime = ((double) iTimeTaken) /      \
                 ((double) iCounterFrequency);    \
                 ::UMessageBox (TFormat ("Time taken to %s = %15.8f seconds\n", what, fTime));     \
  } while (false)

#endif

UINT CMUSHclientDoc::Load_World_XML (CArchive& ar, 
                                     const unsigned long iMask,
                                     const unsigned long iFlags,
                                      UINT * piTriggers,
                                      UINT * piAliases,
                                      UINT * piTimers,
                                      UINT * piMacros,
                                      UINT * piVariables,
                                      UINT * piColours,
                                      UINT * piKeypad,
                                      UINT * piPrinting)
  {
CXMLparser parser;
CXMLelement * pMuClientElement = NULL;
UINT count = 0;
UINT iTriggers = 0;
UINT iAliases = 0;
UINT iTimers = 0;
UINT iMacros = 0;
UINT iVariables = 0;
UINT iColours = 0;
UINT iKeypad = 0;
UINT iPrinting = 0;

CPlugin * pCurrentPlugin = m_CurrentPlugin;

iErrorCount = 0;

#if TIMER_TEST
LARGE_INTEGER start, 
              finish;
LARGE_INTEGER large_int_frequency;

QueryPerformanceFrequency (&large_int_frequency);
LONGLONG iCounterFrequency = large_int_frequency.QuadPart;
#endif

  try 
    {
    strFileName = ar.GetFile ()->GetFilePath ();
    
    if (strFileName.IsEmpty ())
      strFileName = CLIPBOARD_NAME;

    iLineLastItemFound = 0;

#if TIMER_TEST
    START_TIMING;
#endif
    parser.BuildStructure (ar.GetFile ());

#if TIMER_TEST
    STOP_TIMING (CFormat ("parse XML for %s", (LPCTSTR) strFileName));
#endif

    // find <?xml> tag so we don't get a warning
    FindNode (parser.m_xmlRoot, "?xml");

    // if we have a <muclient> tag, use contents of that
    pMuClientElement = FindNode (parser.m_xmlRoot, "muclient");
    if (pMuClientElement)
      CheckUsed (parser.m_xmlRoot);   // check no tags outside <muclient>
    else
      pMuClientElement = &parser.m_xmlRoot;

    if ((iFlags & LOAD_INCLUDE) == 0)
      {   // not include file
      // if not doing an include, we don't have a tree of include files yet
      m_strCurrentIncludeFileList.RemoveAll ();
      // root document is at head of tree
      m_strCurrentIncludeFileList.AddTail (strFileName);
      }

    if (iMask & XML_NO_PLUGINS)
      {
      if (FindNode (*pMuClientElement, "plugin")) 
        ThrowErrorException ("Plugin not expected here. Use File -> Plugins to load plugins");
      }

    if (iMask & XML_PLUGINS)
      if (!(iMask & XML_NO_PLUGINS))
        {    // plugin expected now
        Load_Plugin_XML (*pMuClientElement);    // get <plugin> details
        }

    // if they really wanted a plugin, warn them if none found
    if ((iMask & XML_PLUGINS) && !m_CurrentPlugin)
      ThrowErrorException ("No plugin found");

    // load include files first  (however not from clipboard)
    if (!ar.GetFile ()->GetFilePath ().IsEmpty ())
      Load_Includes_XML (*pMuClientElement,
                    iMask,
                    iFlags,
                    false,    // except plugins
                    piTriggers,  
                    piAliases,   
                    piTimers,    
                    piMacros,    
                    piVariables, 
                    piColours,   
                    piKeypad,    
                    piPrinting);  

    if (iMask & XML_GENERAL)
      Load_General_XML    (*pMuClientElement, iFlags);

    if (iMask & XML_TRIGGERS)
      count += iTriggers = Load_Triggers_XML   (*pMuClientElement, iMask, iFlags);

    if (iMask & XML_ALIASES)
      count += iAliases = Load_Aliases_XML    (*pMuClientElement, iMask, iFlags);

    if (iMask & XML_TIMERS)
      count += iTimers = Load_Timers_XML     (*pMuClientElement, iMask, iFlags);

    if (iMask & XML_MACROS)
      count += iMacros = Load_Macros_XML     (*pMuClientElement, iFlags);

    if (iMask & XML_VARIABLES)
      count += iVariables = Load_Variables_XML  (*pMuClientElement, iMask, iFlags);

    if (iMask & XML_COLOURS)
      count += iColours = Load_Colours_XML    (*pMuClientElement, iFlags);

    if (iMask & XML_KEYPAD)
      count += iKeypad = Load_Keypad_XML     (*pMuClientElement, iFlags);

    if (iMask & XML_PRINTING)
      count += iPrinting = Load_Printing_XML   (*pMuClientElement, iFlags);

    // get scripts belonging to this plugin
    if (m_CurrentPlugin)
      {
      ResetAllTimers (GetTimerMap ());  // and reset plugin timers
      Load_Scripts_XML (*pMuClientElement);
      }
    else     // can't have plugins inside a plugin
      {
      // we have to do plugins later, so that we know the world ID, amongst other things
      // now load include files that include plugins
      if (!ar.GetFile ()->GetFilePath ().IsEmpty ())
        Load_Includes_XML (*pMuClientElement,
                      iMask,
                      iFlags,
                      true,    // only plugins
                      piTriggers,  
                      piAliases,   
                      piTimers,    
                      piMacros,    
                      piVariables, 
                      piColours,   
                      piKeypad,    
                      piPrinting);  


      // tell all plugins the world has now loaded all plugins
      PluginListChanged ();

      }     // not inside a plugin

    CheckUsed (*pMuClientElement);   // check we used all attributes

    // return totals to caller

    if (piTriggers)
      *piTriggers += iTriggers;
    if (piAliases)
      *piAliases += iAliases;
    if (piTimers)
      *piTimers += iTimers;
    if (piMacros)
      *piMacros += iMacros;
    if (piVariables)
      *piVariables += iVariables;
    if (piColours)
      *piColours += iColours;
    if (piKeypad)
      *piKeypad += iKeypad;
    if (piPrinting)
      *piPrinting += iPrinting;

    // now that triggers, etc. are loaded, try to find entry points for plugins
    if (m_CurrentPlugin && !pCurrentPlugin)   // *this* file had a plugin
      {

      m_CurrentPlugin->m_strScript.TrimLeft ();
      m_CurrentPlugin->m_strScript.TrimRight ();

      if (!m_CurrentPlugin->m_strScript.IsEmpty () || m_CurrentPlugin->m_bSendToScriptUsed)
        {

        m_CurrentPlugin->m_ScriptEngine = new CScriptEngine (this, m_CurrentPlugin->m_strLanguage);

        if (m_CurrentPlugin->m_ScriptEngine->CreateScriptEngine ())
          ThrowErrorException ("Could not initialise script engine");

        if (m_CurrentPlugin->m_ScriptEngine->Parse (m_CurrentPlugin->m_strScript, "Plugin"))
          ThrowErrorException ("Error parsing script");
      
        if (FindAllEntryPoints ())
          ThrowErrorException ("Could not find all required script routines");


        // find all plugin callbacks by looping through table

        for (int i = 0; PluginCallbacksTable [i].pName; i++)
          {
          const char * p = (const char *) m_CurrentPlugin + PluginCallbacksTable [i].iOffset;
          * (int *) p = m_CurrentPlugin->GetPluginDispid (PluginCallbacksTable [i].pName);
          }     // end of for each callback

        // note if we need to call these routines

        m_bPluginProcessesOpenTag       = m_CurrentPlugin->m_dispid_plugin_OnMXP_OpenTag      != DISPID_UNKNOWN;
        m_bPluginProcessesCloseTag      = m_CurrentPlugin->m_dispid_plugin_OnMXP_CloseTag     != DISPID_UNKNOWN;
        m_bPluginProcessesSetVariable   = m_CurrentPlugin->m_dispid_plugin_OnMXP_SetVariable  != DISPID_UNKNOWN; 
        m_bPluginProcessesSetEntity     = m_CurrentPlugin->m_dispid_plugin_OnMXP_SetEntity    != DISPID_UNKNOWN;
        m_bPluginProcessesError         = m_CurrentPlugin->m_dispid_plugin_OnMXP_Error        != DISPID_UNKNOWN;

        }  // end of having a script

      // add to world plugins
      m_PluginList.AddTail (m_CurrentPlugin);

      // now call the OnInstall routine (once it is in the list)

//      ::TMessageBox ("Plugin OnInstall");
      
      m_CurrentPlugin->ExecutePluginScript (ON_PLUGIN_INSTALL, m_CurrentPlugin->m_dispid_plugin_install);

      }   // end of having a plugin

    m_CurrentPlugin = pCurrentPlugin;   // finished processing this plugin, if it was one

/*
      if (iErrorCount)
      {
       CString strWarning = TFormat ("%lu warning%s %s generated. See warning window for details.",
                              PLURAL (iErrorCount),
                              iErrorCount == 1 ? "was" : "were");

      ::UMessageBox (strWarning, MB_ICONEXCLAMATION);
      }
*/

    }

  catch(CException* e)
    {
    if (m_CurrentPlugin && !pCurrentPlugin)   // *this* file had a plugin
      delete m_CurrentPlugin;   // throw away plugin, if parsing problem in it
    m_CurrentPlugin = pCurrentPlugin;   
    if (iLineLastItemFound == 0)
      iLineLastItemFound = parser.m_xmlLine;
    HandleLoadException ("Cannot load", e);
    AfxThrowArchiveException (CArchiveException::badSchema);
    }

  return count;
  } // end of CMUSHclientDoc::Load_World_XML

void CMUSHclientDoc::LoadError (const char * sType, const char * sMessage, UINT iLine)
  {

  CString strTitle = "[WARNING] ";
  strTitle += strFileName;

  ColourNote (SCRIPTERRORFORECOLOUR, SCRIPTERRORBACKCOLOUR, strTitle);

  // line defaults to last attribute line
  if (iLine == 0)
    iLine = iLineLastItemFound;

  CString str = TFormat ("Line %4i: %s (%s)%s",
                      iLine,  // which line
                      sMessage,     // what message
                      sType,  // type of thing (eg, trigger)
                      ENDLINE);

  ColourNote (SCRIPTERRORCONTEXTFORECOLOUR, SCRIPTERRORBACKCOLOUR, str);

  iErrorCount++;
  }   // end of CMUSHclientDoc::LoadError

// warns of unused attributes

/*

  Checking unused attributes/nodes is partly to help me debug,
  and partly to help world-file writers to know if they slipped in
  an attribute (eg. misspelt) that wasn't processed.

  There are two parts to it ...

  1. Checking attributes are used, eg.   <node blah="6"> ... </node>

     This checks that "blah" was used somewhere.

  2. Checking children were used, eg.  <node>  <child> ... </child>  </node>

     This checks that "<child>" was used somewhere.

  */

void CMUSHclientDoc::CheckUsed (CXMLelement & node)
  {
  POSITION pos;
  CString strName;
  CAttribute * attribute;

/*
  Each tag can have comments as a child
*/

  Load_Comments_XML (node);

  // check attributes of this tag
  for (pos = node.AttributeMap.GetStartPosition(); pos; )
    {
    node.AttributeMap.GetNextAssoc (pos, strName, attribute);
    if (attribute->bUsed)
      continue;

    LoadError (node.strName, 
      TFormat ("Attribute not used: %s=\"%s\"",
                (LPCTSTR) strName,
                (LPCTSTR) attribute->strValue),
                attribute->iLine);

    }      // end of checking each attribute

  // look for unused children

  for (pos = node.ChildrenList.GetHeadPosition (); pos; )
    {
    CXMLelement * pElement = node.ChildrenList.GetNext (pos);
    if (pElement->bUsed)
      continue;

    LoadError (node.strName, 
      TFormat ("Tag not used: <%s>",
                (LPCTSTR) pElement->strName),
                pElement->iLine);

    } // end of checking each tag

  } // end of CMUSHclientDoc::CheckUsed 

void CMUSHclientDoc::Load_Includes_XML (CXMLelement & parent,
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
                                      UINT * piPrinting)
  {

CStringList strIncludeList;
POSITION lpos;

  // save current hierarchy 
  for (lpos = m_strCurrentIncludeFileList.GetHeadPosition (); lpos; )
    strIncludeList.AddTail (m_strCurrentIncludeFileList.GetNext (lpos));

  // see if we have "include" tag(s)

  LOAD_LOOP (parent, "include", pIncludeElement);
  
    try
      {
      Load_One_Include_XML (*pIncludeElement, 
                            iMask,
                            iFlags,
                            bPlugins,
                            piTriggers,
                            piAliases,
                            piTimers,
                            piMacros,
                            piVariables,
                            piColours,
                            piKeypad,
                            piPrinting);

      }
    catch (CException* e)
      {
      HandleLoadException ("Not loaded", e);
      } // end of catch

    CheckUsed (*pIncludeElement);

  END_LOAD_LOOP;

  // restore current include list tree
  m_strCurrentIncludeFileList.RemoveAll ();
  for (lpos = strIncludeList.GetHeadPosition (); lpos; )
    m_strCurrentIncludeFileList.AddTail (strIncludeList.GetNext (lpos));

  } // end of CMUSHclientDoc::Load_Includes_XML

void CMUSHclientDoc::Load_One_Include_XML (CXMLelement & node, 
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
                                      UINT * piPrinting)
  {
CString strSavedFileName = strFileName;
int iSavedLineLastItemFound = iLineLastItemFound;
int iSavedErrorCount = iErrorCount;
CString strOriginalName;

bool bUseDefault = false;
CFile * f = NULL;
CArchive * ar = NULL;
CXMLparser parser;
bool bPlugin;

  try
    {
    GET_STRING_TRIM ("name", strFileName);
    GET_BOOL ("plugin", bPlugin);

    // make sure they specified something
    if (strFileName.IsEmpty ())
      ThrowErrorException ("Name of include file not specified");

    // don't want the name "clipboard"
    if (strFileName.CompareNoCase (CLIPBOARD_NAME) == 0)
      ThrowErrorException ("Name of include file cannot be \"%s\"",
                           (LPCTSTR) CLIPBOARD_NAME);

    strOriginalName = strFileName;

    strFileName = ::Replace (strFileName, "$PLUGINSDEFAULTDIR", RemoveFinalSlash (Make_Absolute_Path (App.m_strPluginsDirectory))); 
    strFileName = ::Replace (strFileName, "$PROGRAMDIR", 
                             RemoveFinalSlash (ExtractDirectory (App.m_strMUSHclientFileName)));
    strFileName = ::Replace (strFileName, "$WORLDDIR", 
                             RemoveFinalSlash (ExtractDirectory (GetPathName ())));

    if (m_CurrentPlugin)
      strFileName = ::Replace (strFileName, "$PLUGINDIR", 
                               RemoveFinalSlash (ExtractDirectory (m_CurrentPlugin->m_strSource)));


    // use relative path if required ...

    if (!(
        isalpha ((unsigned char) strFileName [0]) &&
        strFileName [1] == ':'               // eg. c: blah
        ) &&      
        strFileName [0] != '\\' &&           // eg. \mydir\blah or \\server\blah
        strFileName [0] != '/' &&            // eg. /mydir/blah or //server/blah
        strFileName.Left (3) != "..\\" &&    // eg. ..\blah\blah
        strFileName.Left (3) != "../" &&     // eg. ../blah/blah
        strFileName.Left (2) != "./" &&      // eg. ./blah
        strFileName.Left (2) != ".\\")       // eg. .\blah
      if (m_CurrentPlugin || bPlugins)
        strFileName = CString (Make_Absolute_Path (App.m_strPluginsDirectory)) + strFileName;
      else
        strFileName = CString (Make_Absolute_Path (App.m_strDefaultWorldFileDirectory)) + strFileName;
   
    // do plugins and non-plugins at different times
    if (bPlugin == bPlugins)
      {
      if (!bPlugin && !m_CurrentPlugin)
        {
        // don't include it twice
        if (m_strCurrentIncludeFileList.Find (strFileName))
          ThrowErrorException ("File \"%s\" has already been included.",
                                (LPCTSTR) strFileName);

        m_strCurrentIncludeFileList.AddTail (strFileName);

        // if root level include, remember in world document
        if (!(iFlags & LOAD_INCLUDE))
           m_strIncludeFileList.AddTail (strOriginalName);
        }   // plugins are in a different list

      f = new CFile (strFileName, CFile::modeRead | CFile::shareDenyWrite);

      ar = new CArchive(f, CArchive::load);

      if (IsArchiveXML (*ar))
        {

        try
          {
          Load_World_XML (*ar, 
                          iMask | (bPlugin ? XML_PLUGINS : XML_INCLUDES),
                          iFlags | (bPlugin ? 0 : LOAD_INCLUDE),
                          piTriggers,
                          piAliases,
                          piTimers,
                          piMacros,
                          piVariables,
                          piColours,
                          piKeypad,
                          piPrinting);
          }
        catch (CArchiveException* e)
          {
          e->Delete ();
          ThrowErrorException ("Error in file \"%s\"",
                               (LPCTSTR) strFileName);
          } // end of catch

        } 
      else
        ThrowErrorException ("File \"%s\" is not an XML file",
                             (LPCTSTR) strFileName);

      } // end of plugin flag being what we want
    else
      if (!bPlugins)
        node.bUsed = false;    // so we get it next time

    } // end of try

  catch(CException*)
    {
    strFileName = strSavedFileName;
    iLineLastItemFound = iSavedLineLastItemFound;
    iErrorCount = iSavedErrorCount;
    delete ar;
    delete f;
    throw;
    }

  strFileName = strSavedFileName;
  iLineLastItemFound = iSavedLineLastItemFound;
  iErrorCount = iSavedErrorCount;
  delete ar;
  delete f;

  } // end of  CMUSHclientDoc::Load_One_Include_XML

#pragma warning(push)
#pragma warning(disable : 4189)     // warning C4189: 'iVersion' : local variable is initialized but not referenced
void CMUSHclientDoc::Load_General_XML (CXMLelement & parent, 
                                       const unsigned long iFlags)
  {
  // see if we have "world" tag(s)

  LOAD_LOOP (parent, "world", pWorldElement);

    GET_VERSION_AND_DEFAULTS (pWorldElement);

    // worlds consist of:
    // 1. single-line alpha attributes, eg.  script_language="VBscript"
    // 2. numeric attributes, eg. wrap_column="80"
    // 3. multi-line alpha children, eg. <notes>blah</notes>
    
    if (m_CurrentPlugin)
      ThrowErrorException ("<world> tag cannot be used inside a plugin");

    // get numeric ones first, so we know if various options are enabled
    Load_World_Numeric_Options_XML (*pWorldElement, bUseDefault, iFlags);
    Load_World_Single_Line_Alpha_Options_XML (*pWorldElement, bUseDefault, iFlags);
    Load_World_Multi_Line_Alpha_Options_XML (*pWorldElement, bUseDefault, iFlags);

    CheckUsed (*pWorldElement);

    // check world ID is unique  (maybe they duplicated a disk file)  
    for (POSITION pos = App.m_pWorldDocTemplate->GetFirstDocPosition(); pos != NULL; )
      {
      CMUSHclientDoc * pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(pos);
      if (pDoc == this) 
        continue;   // don't compare to ourselves

      if (pDoc->m_strWorldID == m_strWorldID)
        {
        m_strWorldID = GetUniqueID ();      // so change it ;)
        SetModifiedFlag ();
        }

      }      // end of looping through each world

  END_LOAD_LOOP;

  } // end of CMUSHclientDoc::Load_General_XML
#pragma warning(pop)

void CMUSHclientDoc::Load_World_Numeric_Options_XML (CXMLelement & parent,
                                                    bool bUseDefault,
                                                    const unsigned long iFlags)
  {

  for (int i = 0; OptionsTable [i].pName; i++)
    {
    try
      {
      COLORREF cValue;
      if (OptionsTable [i].iFlags & OPT_RGB_COLOUR)
        {
        if (Get_XML_colour (parent, 
                            OptionsTable [i].pName, 
                            cValue,
 //  version 3.83           bUseDefault || m_NumericConfiguration [i]->bInclude,
                            TRUE  //  version 3.83 
                            ))
          SetOptionItem (i, cValue, true, (iFlags & LOAD_INCLUDE) != 0);
        /*       version 3.83
        else // not found, use default?
          if (!(bUseDefault || m_NumericConfiguration [i]->bInclude))
             SetOptionItem (i, 0, true, false);       // then take zero
        */
        }   // end of RGB colour
      else
      if (OptionsTable [i].iMinimum == 0 &&
          OptionsTable [i].iMaximum == 0)
        {
        bool bValue;
        if (Get_XML_boolean (parent, 
                            OptionsTable [i].pName, 
                            bValue,
                            bUseDefault || m_NumericConfiguration [i]->bInclude
                            ))
          SetOptionItem (i, bValue, true, (iFlags & LOAD_INCLUDE) != 0);
        else // not found, use default?
          if (!(bUseDefault || m_NumericConfiguration [i]->bInclude))
            SetOptionItem (i, 0, true, false);        // then take zero
  
        }   // end of boolean 
      else
        {   // not RGB colour or boolean
        long iValue;
        if (Get_XML_number (parent, 
                            OptionsTable [i].pName, 
                            iValue,
 //  version 4.30           bUseDefault || m_NumericConfiguration [i]->bInclude,
                            TRUE,  //  version 4.30 
                            OptionsTable [i].iMinimum,
                            OptionsTable [i].iMaximum))
          SetOptionItem (i, iValue, true, (iFlags & LOAD_INCLUDE) != 0);
        /*    version 4.30
        else // not found, use default?
          if (!(bUseDefault || m_NumericConfiguration [i]->bInclude))
            {             // then take zero, if we can
            if (0 < OptionsTable [i].iMinimum ||
                0 > OptionsTable [i].iMaximum)
               ThrowErrorException ("Value of zero not acceptable here");

            SetOptionItem (i, 0, true, false);

            }  // end of taking zero for unspecified variables
*/

        } // end of not RGB colour or boolean
      }
    catch (CException* e)
      {
      HandleLoadException (TFormat ("option '%s' not set", (LPCTSTR) OptionsTable [i].pName), e);
      } // end of catch
    }

  } // end of CMUSHclientDoc::Load_World_Numeric_Options_XML

void CMUSHclientDoc::Load_World_Single_Line_Alpha_Options_XML (CXMLelement & parent,
                                                              bool bUseDefault,
                                                              const unsigned long iFlags)
  {

  CString strValue;
  for (int i = 0; AlphaOptionsTable [i].pName; i++)
    if (!(AlphaOptionsTable [i].iFlags & OPT_MULTLINE))
      {
      try
        {
        if (Get_XML_string (parent, 
                            AlphaOptionsTable [i].pName, 
                            strValue, 
                            bUseDefault || m_AlphaConfiguration [i]->bInclude,
                            !(AlphaOptionsTable [i].iFlags & OPT_KEEP_SPACES)))
          {
          if (AlphaOptionsTable [i].iFlags & OPT_PASSWORD)
            {
            bool bEncoded;
            Get_XML_boolean (parent, 
                             CFormat ("%s_base64", (LPCTSTR) AlphaOptionsTable [i].pName), 
                             bEncoded, false);
            if (bEncoded)
              {
              strValue.TrimLeft ();
              strValue.TrimRight ();
              strValue = decodeBase64 (strValue);  // base 64 decoding
              }  // end of being base 64 encoded
            }    // end of maybe being encoded

          if (AlphaOptionsTable [i].iFlags & OPT_WORLD_ID)
            {
            if (strValue.GetLength () != PLUGIN_UNIQUE_ID_LENGTH)
               ThrowErrorException ("World \"id\" field must be %i characters long", 
                                    PLUGIN_UNIQUE_ID_LENGTH);

            // make sure hex characters
            const char * p = strValue;
            for ( ; *p ; p++)
              if (!isxdigit (*p))
               ThrowErrorException ("World \"id\" field must be %i hex characters",
                                    PLUGIN_UNIQUE_ID_LENGTH);

            strValue.MakeLower ();
            }   // end of world ID

      // found it - set value
          SetAlphaOptionItem (i, strValue, true, (iFlags & LOAD_INCLUDE) != 0);
          } // end of found
        else
          if (!(bUseDefault || m_AlphaConfiguration [i]->bInclude))
            if (AlphaOptionsTable [i].iFlags & OPT_WORLD_ID)
              SetAlphaOptionItem (i, ::GetUniqueID (), true, false);   // get world ID, not a blank one
            else
              SetAlphaOptionItem (i, "", true, false);

        } // end of try block
      catch (CException* e)
        {
        HandleLoadException (TFormat ("option '%s' not set", (LPCTSTR) AlphaOptionsTable [i].pName), e);
        } // end of catch
      }  // end of single-line options

  } // end of CMUSHclientDoc::Load_World_Single_Line_Alpha_Options_XML 


void CMUSHclientDoc::Load_World_Multi_Line_Alpha_Options_XML (CXMLelement & parent,
                                                              bool bUseDefault,
                                                              const unsigned long iFlags)
  {

  CString strValue;
  for (int i = 0; AlphaOptionsTable [i].pName; i++)
    if (AlphaOptionsTable [i].iFlags & OPT_MULTLINE)
      {
      try
        {
        CString strValue;
        if (FindNodeContents (parent, 
                              AlphaOptionsTable [i].pName, 
                              strValue, 
                              bUseDefault || m_AlphaConfiguration [i]->bInclude))
           SetAlphaOptionItem (i, strValue, true, (iFlags & LOAD_INCLUDE) != 0);
        else
          if (!(bUseDefault || m_AlphaConfiguration [i]->bInclude))
            SetAlphaOptionItem (i, "", true, false);

        } // end of try block
      catch (CException* e)
        {
        HandleLoadException (TFormat ("option '%s' not set", (LPCTSTR) AlphaOptionsTable [i].pName), e);
        } // end of catch
      }   // end of mult-line options
  } // end of CMUSHclientDoc::Load_World_Multi_Line_Alpha_Options_XML 



UINT CMUSHclientDoc::Load_Triggers_XML (CXMLelement & parent, 
                                       const unsigned long iMask,
                                       const unsigned long iFlags)
  {
  UINT count = 0;

  LOAD_LOOP (parent, "triggers", pTriggers);

    GET_VERSION_AND_DEFAULTS (pTriggers);

    LOAD_LOOP (*pTriggers, "trigger", pElement);
  
    try
      {
      Load_One_Trigger_XML (*pElement, iMask, iVersion, bUseDefault, iFlags);
      count++;
      }
    catch (CException* e)
      {
      HandleLoadException ("trigger not loaded", e);
      } // end of catch

    END_LOAD_LOOP;

    CheckUsed (*pTriggers);   // check we used all attributes

  END_LOAD_LOOP;

  SortTriggers ();

  return count;
  }   // end of CMUSHclientDoc::Load_Triggers_XML


void CMUSHclientDoc::Load_One_Trigger_XML (CXMLelement & node, 
                                           const unsigned long iMask,
                                           const long iVersion, 
                                           bool bUseDefault, 
                                           const unsigned long iFlags)
  {
CTrigger * t = new CTrigger;
CString strTriggerName;
CString strVariable;

  try
    {
    Get_XML_boolean (node, DEFAULTS_LIT, bUseDefault, true); // see if they want defaults
  
    Get_XML_ushort  (node, "clipboard_arg",     t->iClipboardArg, bUseDefault, 0, 10);

    // even if they say "don't take default", the absence of a colour
    // gives the default behaviour of "no change"

    // colour 17 (MAX_CUSTOM + 1) will be the "other" colour
    // see    other_text_colour / other_back_colour
    if (Get_XML_ushort  (node, "custom_colour", t->colour, true, 0, MAX_CUSTOM + 1))
      if (t->colour == 0)
        t->colour = SAMECOLOUR;
      else
        t->colour--;    // make zero relative
    else  // colour not supplied
      t->colour = SAMECOLOUR;

    GET_BOOL_USHORT  ("enabled",           t->bEnabled);
    GET_BOOL_USHORT  ("expand_variables",  t->bExpandVariables);
    GET_STRING_TRIM  ("group",             t->strGroup);
    GET_STRING_TRIM  ("variable",          t->strVariable);
#ifdef PANE
    GET_STRING_TRIM  ("pane",              t->strPane);
#endif // PANE
    GET_BOOL_USHORT  ("ignore_case",       t->ignore_case);
    GET_BOOL_USHORT  ("keep_evaluating",   t->bKeepEvaluating);
    GET_USHORT       ("lines_to_match",    t->iLinesToMatch, 0, MAX_RECENT_LINES);
    GET_USHORT       ("colour_change_type",t->iColourChangeType, TRIGGER_COLOUR_CHANGE_BOTH, TRIGGER_COLOUR_CHANGE_BACKGROUND);
    GET_STRING       ("match",             t->trigger);
    GET_BOOL_USHORT  ("multi_line",        t->bMultiLine);
    GET_STRING_TRIM  ("name",              t->strLabel);
    GET_BOOL_USHORT  ("omit_from_log",     t->omit_from_log);
    GET_BOOL_USHORT  ("omit_from_output",  t->bOmitFromOutput);
    GET_BOOL_USHORT  ("regexp",            t->bRegexp);
    GET_BOOL_USHORT  ("repeat",            t->bRepeat);
    GET_STRING_TRIM  ("script",            t->strProcedure);
    GET_USHORT       ("send_to",           t->iSendTo, 0, eSendToLast - 1);
    GET_USHORT       ("sequence",          t->iSequence, 0, 10000);
    GET_STRING_TRIM  ("sound",             t->sound_to_play);
    GET_BOOL_USHORT  ("sound_if_inactive", t->bSoundIfInactive);
    GET_BOOL         ("lowercase_wildcard", t->bLowercaseWildcard);
    GET_BOOL         ("temporary",         t->bTemporary);
    GET_LONG         ("user",              t->iUserOption, LONG_MIN, LONG_MAX);
    GET_BOOL_BOOL    ("one_shot",          t->bOneShot);

    // remember if they send to script
    if ((t->iSendTo == eSendToScript || t->iSendTo == eSendToScriptAfterOmit) && m_CurrentPlugin)
      m_CurrentPlugin->m_bSendToScriptUsed = true;

    GET_BOOL_USHORT_MASK ("make_bold",       t->iStyle, HILITE);
    GET_BOOL_USHORT_MASK ("make_italic",     t->iStyle, BLINK);
    GET_BOOL_USHORT_MASK ("make_underline",  t->iStyle, UNDERLINE);

    unsigned short iColour;
    if (GET_USHORT  ("text_colour",       iColour, 0, 15))
      {
      t->iMatch &= ~0x0F0;  // remove old colour
      t->iMatch |= iColour << 4;
      }

    if (GET_USHORT  ("back_colour",       iColour, 0, 15))
      {
      t->iMatch &= ~0xF00;  // remove old colour
      t->iMatch |= iColour << 8;
      }

    GET_BOOL_USHORT_MASK ("bold",              t->iMatch, HILITE);
    GET_BOOL_USHORT_MASK ("match_back_colour", t->iMatch, TRIGGER_MATCH_BACK);
    GET_BOOL_USHORT_MASK ("match_bold",        t->iMatch, TRIGGER_MATCH_HILITE);
    GET_BOOL_USHORT_MASK ("match_inverse",     t->iMatch, TRIGGER_MATCH_INVERSE);
    GET_BOOL_USHORT_MASK ("match_italic",      t->iMatch, TRIGGER_MATCH_BLINK);
    GET_BOOL_USHORT_MASK ("match_text_colour", t->iMatch, TRIGGER_MATCH_TEXT);
    GET_BOOL_USHORT_MASK ("match_underline",   t->iMatch, TRIGGER_MATCH_UNDERLINE);
    GET_BOOL_USHORT_MASK ("inverse",           t->iMatch, INVERSE);
    GET_BOOL_USHORT_MASK ("italic",            t->iMatch, BLINK);

    GET_COLOUR ("other_text_colour",           t->iOtherForeground);
    GET_COLOUR ("other_back_colour",           t->iOtherBackground);

    // find <send> text
    FindNodeContents (node, "send", t->contents, bUseDefault);

    // various checks ...

    strTriggerName = t->strLabel;
    strVariable = t->strVariable;

    // conversion from older versions - make variable name same as label if necessary
    if (strVariable.IsEmpty () && !strTriggerName.IsEmpty ())
      if (t->iSendTo == eSendToVariable)
        strVariable = t->strVariable = strTriggerName;

    // if variable is empty, can't have 'send to variable'
    if (strVariable.IsEmpty ())
      {
      if (t->iSendTo == eSendToVariable)
          ThrowErrorException ("Must have variable for 'send to variable'");
      }
    else
      // otherwise check variable name OK
      if (CheckObjectName (strVariable))
        ThrowErrorException ("Invalid trigger variable \"%s\" ", 
                             (LPCTSTR) t->strVariable);

      
    if (!strTriggerName.IsEmpty ())
      if (CheckObjectName (strTriggerName))
        ThrowErrorException ("Invalid trigger label \"%s\" ", 
                             (LPCTSTR) t->strLabel);

    // must have trigger match text
    if (t->trigger.IsEmpty ())
      ThrowErrorException ("Must have trigger match text");

  // generate a name if it doesn't have one

    if (strTriggerName.IsEmpty () || strTriggerName.Left (1) == "*")
      strTriggerName.Format ("*trigger%s", (LPCTSTR) App.GetUniqueString ());
    else
      strTriggerName.MakeLower ();

  // check for duplicate labels

    CTrigger * trigger_check;
    if (GetTriggerMap ().Lookup (strTriggerName, trigger_check))
      {
      if (iMask & XML_OVERWRITE)
        delete trigger_check;
      else
        ThrowErrorException ("Duplicate trigger label \"%s\" ", 
                             strTriggerName);
      }   // end of duplicate

  // all triggers are now regular expressions

    CString strRegexp; 

    if (t->bRegexp)
      strRegexp = t->trigger;
    else
      {
      // check for multiple asterisks - one day

      strRegexp = ConvertToRegularExpression (t->trigger);
      }

  // compile regular expression

    try
      {
      t->regexp = regcomp (strRegexp, (t->ignore_case ? PCRE_CASELESS : 0) |
                            (t->bMultiLine  ? PCRE_MULTILINE : 0) |
                            (m_bUTF_8 ? PCRE_UTF8 : 0)
        );
      }
    // catch regexp error and rethrow with proper message
    catch (CException* e)
      {
      char sMessage [500];
      e->GetErrorMessage (sMessage, sizeof sMessage);
      e->Delete ();
      ThrowErrorException ("Error \"%s\" processing regular expression \"%s\"", 
                           sMessage,
                           (LPCTSTR) strRegexp);
      } // end of catch


    // remember if this was from an include
    t->bIncluded = (iFlags & LOAD_INCLUDE) != 0;
    } // end of try

  catch(CException*)
    {
    delete t; // get rid of trigger
    throw;
    }

  // now add to our internal trigger map

  t->nUpdateNumber    = App.GetUniqueNumber ();   // for concurrency checks
  t->strInternalName  = strTriggerName;    // for deleting one-shot triggers
  GetTriggerMap ().SetAt (strTriggerName, t);

  CheckUsed (node);   // check we used all attributes

  } // end of CMUSHclientDoc::Load_One_Trigger_XML


UINT CMUSHclientDoc::Load_Aliases_XML (CXMLelement & parent, 
                                       const unsigned long iMask,
                                       const unsigned long iFlags)
  {
UINT count = 0;

  LOAD_LOOP (parent, "aliases", pAliases);

    GET_VERSION_AND_DEFAULTS (pAliases);

    LOAD_LOOP (*pAliases, "alias", pElement);

      try
        {
        Load_One_Alias_XML (*pElement, iMask, iVersion, bUseDefault, iFlags);
        count++;
        }
      catch (CException* e)
        {
        HandleLoadException ("alias not loaded", e);
        } // end of catch

    END_LOAD_LOOP;

    CheckUsed (*pAliases);   // check we used all attributes

  END_LOAD_LOOP;

  SortAliases ();

  return count;
  }   // end of CMUSHclientDoc::Load_Aliases_XML


void CMUSHclientDoc::Load_One_Alias_XML (CXMLelement & node, 
                                         const unsigned long iMask,
                                         const long iVersion, 
                                         bool bUseDefault, 
                                         const unsigned long iFlags)
  {
CAlias * a = new CAlias;
CString strAliasName;
CString strVariable;

  try
    {
    Get_XML_boolean (node, DEFAULTS_LIT, bUseDefault, true); // see if they want defaults

    GET_STRING_TRIM ("name",              a->strLabel);
    GET_STRING_TRIM ("script",            a->strProcedure);
    GET_STRING      ("match",             a->name);
    GET_BOOL_USHORT ("enabled",           a->bEnabled);

    GET_BOOL_USHORT ("expand_variables",  a->bExpandVariables);
    GET_STRING_TRIM ("group",             a->strGroup);
    GET_STRING_TRIM ("variable",          a->strVariable);
    GET_BOOL_USHORT ("omit_from_log",     a->bOmitFromLog);
    GET_BOOL_USHORT ("regexp",            a->bRegexp);
    GET_USHORT      ("send_to",           a->iSendTo, 0, eSendToLast - 1);
    GET_BOOL_USHORT ("omit_from_command_history", a->bOmitFromCommandHistory);
    GET_BOOL_USHORT ("omit_from_output",  a->bOmitFromOutput);
    GET_BOOL_USHORT ("menu",              a->bMenu);
    GET_BOOL_USHORT ("ignore_case",       a->bIgnoreCase);
    GET_BOOL_USHORT ("keep_evaluating",   a->bKeepEvaluating);
    GET_USHORT      ("sequence",          a->iSequence, 0, 10000);
    GET_BOOL_USHORT ("echo_alias",        a->bEchoAlias);
    GET_LONG        ("user",              a->iUserOption, LONG_MIN, LONG_MAX);
    GET_BOOL        ("temporary",         a->bTemporary);
    GET_BOOL_BOOL   ("one_shot",          a->bOneShot);

    
    // remember if they send to script
    if ((a->iSendTo == eSendToScript || a->iSendTo == eSendToScriptAfterOmit) && m_CurrentPlugin)
      m_CurrentPlugin->m_bSendToScriptUsed = true;

    // for backwards compatibility, see if these booleans are set (for pre 3.35)

    bool bSpeedWalk = false;
    bool bVariable = false;
    bool bQueue = false;

    // note - in 3.35 onwards these are incorporated into iSendTo

    GET_BOOL         ("speed_walk", bSpeedWalk);
    GET_BOOL         ("set_variable", bVariable);
    GET_BOOL         ("queue", bQueue);

    if (bSpeedWalk)
      {
      if (a->iSendTo != eSendToWorld && a->iSendTo != eSendToSpeedwalk)
          ThrowErrorException ("'speed_walk' incompatible with 'send_to'");
      a->iSendTo = eSendToSpeedwalk;
      }

    if (bVariable)
      {
      if (a->iSendTo != eSendToWorld && a->iSendTo != eSendToVariable)
          ThrowErrorException ("'set_variable' incompatible with 'send_to'");
      a->iSendTo = eSendToVariable;
      }

    if (bQueue)
      {
      if (a->iSendTo != eSendToWorld && a->iSendTo != eSendToCommandQueue)
          ThrowErrorException ("'queue' incompatible with 'send_to'");
      a->iSendTo = eSendToCommandQueue;
      }

    // find <send> text
    FindNodeContents (node, "send", a->contents, bUseDefault);

    // various checks ...

    strAliasName = a->strLabel;
    strVariable = a->strVariable;

    // if variable is empty, can't have 'send to label'
    if (strVariable.IsEmpty ())
      {
      if (a->iSendTo == eSendToVariable)
          ThrowErrorException ("Must have variable name for 'send to variable'");
      }
    else
      // otherwise check variable name OK
      if (CheckObjectName (strVariable))
        ThrowErrorException ("Invalid alias variable \"%s\" ", 
                             (LPCTSTR) a->strVariable);

    // check label OK
    if (!strAliasName.IsEmpty ())
      if (CheckObjectName (strAliasName))
        ThrowErrorException ("Invalid alias label \"%s\" ", 
                             (LPCTSTR) a->strLabel);

    // must have alias match text
    if (a->name.IsEmpty ())
      ThrowErrorException ("Must have alias match text");

    // generate a name if it doesn't have one

    if (strAliasName.IsEmpty () || strAliasName.Left (1) == "*")
      strAliasName.Format ("*alias%s", (LPCTSTR) App.GetUniqueString ());
    else
      strAliasName.MakeLower ();

    // check for duplicate labels

    CAlias * alias_check;
    if (GetAliasMap ().Lookup (strAliasName, alias_check))
      {
      if (iMask & XML_OVERWRITE)
        delete alias_check;
      else
        ThrowErrorException ("Duplicate alias label \"%s\" ", 
                             strAliasName);
      } // end of duplicate

  // all aliases are now regular expressions

    CString strRegexp; 

    if (a->bRegexp)
      strRegexp = a->name;
    else
      {
      // check for multiple asterisks - one day

      strRegexp = ConvertToRegularExpression (a->name);
      }

  // compile regular expression

    try
      {
      a->regexp = regcomp (strRegexp, (a->bIgnoreCase ? PCRE_CASELESS : 0)
#if ALIASES_USE_UTF8
                                  | (m_bUTF_8 ? PCRE_UTF8 : 0)
#endif // ALIASES_USE_UTF8
        );
      }
    // catch regexp error and rethrow with proper message
    catch (CException* e)
      {
      char sMessage [500];
      e->GetErrorMessage (sMessage, sizeof sMessage);
      e->Delete ();
      ThrowErrorException ("Error \"%s\" processing regular expression \"%s\"", 
                           sMessage,
                           (LPCTSTR) strRegexp);
      } // end of catch

    // remember if this was from an include
    a->bIncluded = (iFlags & LOAD_INCLUDE) != 0;

    } // end of try

  catch(CException*)
    {
    delete a; // get rid of alias
    throw;
    }

  // now add to our internal alias map

  a->nUpdateNumber    = App.GetUniqueNumber ();   // for concurrency checks
  a->strInternalName  = strAliasName;    // for deleting one-shot aliases
  GetAliasMap ().SetAt (strAliasName, a);

  CheckUsed (node);   // check we used all attributes

  } // end of CMUSHclientDoc::Load_One_Alias_XML


UINT CMUSHclientDoc::Load_Timers_XML (CXMLelement & parent, 
                                      const unsigned long iMask,
                                       const unsigned long iFlags)
  {
UINT count = 0;

  LOAD_LOOP (parent, "timers", pTimers);

    GET_VERSION_AND_DEFAULTS (pTimers);

    LOAD_LOOP (*pTimers, "timer", pElement);

    try
      {
      Load_One_Timer_XML (*pElement, iMask, iVersion, bUseDefault, iFlags);
      count++;
      }
    catch (CException* e)
      {
      HandleLoadException ("timer not loaded", e);
      } // end of catch

    END_LOAD_LOOP;

    CheckUsed (*pTimers);   // check we used all attributes

  END_LOAD_LOOP;

  SortTimers ();

  return count;
  }   // end of CMUSHclientDoc::Load_Timers_XML


void CMUSHclientDoc::Load_One_Timer_XML (CXMLelement & node, 
                                         const unsigned long iMask,
                                         const long iVersion, 
                                         bool bUseDefault, 
                                         const unsigned long iFlags)
  {
CTimer * t = new CTimer;
CString strTimerName,
        strVariable;

  try
    {
    Get_XML_boolean (node, DEFAULTS_LIT, bUseDefault, true); // see if they want defaults

    GET_STRING_TRIM  ("name",           t->strLabel);
    GET_STRING_TRIM  ("script",         t->strProcedure);
    GET_BOOL_BOOL    ("enabled",        t->bEnabled);

    bool bAtTime = false;
    Get_XML_boolean (node, "at_time", bAtTime, bUseDefault);
    
    if (bAtTime)
      t->iType = CTimer::eAtTime;
    else
      t->iType = CTimer::eInterval;

    GET_STRING_TRIM  ("group",          t->strGroup);
    GET_STRING_TRIM  ("variable",       t->strVariable);
    GET_BOOL_BOOL    ("one_shot",       t->bOneShot);
    GET_BOOL_BOOL    ("active_closed",  t->bActiveWhenClosed);
    GET_USHORT       ("send_to",        t->iSendTo, 0, eSendToLast - 1);
    GET_BOOL_USHORT  ("omit_from_output",  t->bOmitFromOutput);
    GET_BOOL_USHORT  ("omit_from_log",  t->bOmitFromLog);
                            
    // remember if they send to script
    if ((t->iSendTo == eSendToScript || t->iSendTo == eSendToScriptAfterOmit) && m_CurrentPlugin)
      m_CurrentPlugin->m_bSendToScriptUsed = true;

    if (t->iType == CTimer::eAtTime)
      {
      GET_INT  ("hour",    t->iAtHour,   0, 23);
      GET_INT  ("minute",  t->iAtMinute, 0, 59 );
      GET_DOUBLE  ("second", t->fAtSecond, 0, 59.9999 );
      }
    else
      {
      GET_INT  ("hour",    t->iEveryHour,   0, 23);
      GET_INT  ("minute",  t->iEveryMinute, 0, 59 );
      GET_DOUBLE  ("second", t->fEverySecond, 0, 59.9999 );
      if (!(t->iEveryHour || t->iEveryMinute || t->fEverySecond > 0.0))
        ThrowErrorException ("Timer for 'every' time cannot have zero interval");
      }


    GET_INT  ("offset_hour",       t->iOffsetHour,   0, 23);
    GET_INT  ("offset_minute",     t->iOffsetMinute, 0, 59);
    GET_DOUBLE  ("offset_second",  t->fOffsetSecond, 0, 59.9999);

    GET_LONG ("user",              t->iUserOption, LONG_MIN, LONG_MAX);
    GET_BOOL ("temporary",         t->bTemporary);


    // for backwards compatibility, see if these booleans are set (for pre 3.35)

    bool bSpeedWalk = false;
    bool bNote = false;

    // note - in 3.35 onwards these are incorporated into iSendTo

    GET_BOOL         ("speed_walk", bSpeedWalk);
    GET_BOOL         ("note", bNote);

    if (bSpeedWalk)
      {
      if (t->iSendTo != eSendToWorld && t->iSendTo != eSendToSpeedwalk)
          ThrowErrorException ("'speed_walk' incompatible with 'send_to'");
      t->iSendTo = eSendToSpeedwalk;
      }

    if (bNote)
      {
      if (t->iSendTo != eSendToWorld && t->iSendTo != eSendToOutput)
          ThrowErrorException ("'note' incompatible with 'send_to'");
      t->iSendTo = eSendToOutput;
      }

    // find <send> text
    FindNodeContents (node, "send", t->strContents, bUseDefault);

    // various checks ...

    strTimerName = t->strLabel;
    strVariable = t->strVariable;

    // if variable is empty, can't have 'send to label'
    if (strVariable.IsEmpty ())
      {
      if (t->iSendTo == eSendToVariable)
          ThrowErrorException ("Must have variable name for 'send to variable'");
      }
    else
      // otherwise check variable name OK
      if (CheckObjectName (strVariable))
        ThrowErrorException ("Invalid timer variable \"%s\" ", 
                             (LPCTSTR) t->strVariable);

    // check label OK
    if (!strTimerName.IsEmpty ())
      if (CheckObjectName (strTimerName))
        ThrowErrorException ("Invalid timer label \"%s\" ", 
                             (LPCTSTR) t->strLabel);

    // generate a name if it doesn't have one

    if (strTimerName.IsEmpty () || strTimerName.Left (1) == "*")
      strTimerName.Format ("*timer%s", (LPCTSTR) App.GetUniqueString ());
    else
      strTimerName.MakeLower ();

    // check for duplicate labels

    CTimer * timer_check;
    if (GetTimerMap ().Lookup (strTimerName, timer_check))
      {
      if (iMask & XML_OVERWRITE)
        delete timer_check;
      else
        ThrowErrorException ("Duplicate timer label \"%s\" ", 
                             strTimerName);
      } // end of duplciate

    // remember if this was from an include
    t->bIncluded = (iFlags & LOAD_INCLUDE) != 0;

    } // end of try

  catch(CException*)
    {
    delete t; // get rid of timer
    throw;
    }

  // now add to our internal timer map

  t->nUpdateNumber    = App.GetUniqueNumber ();   // for concurrency checks
  GetTimerMap ().SetAt (strTimerName, t);

  ResetOneTimer (t);    // make sure it is reset

  CheckUsed (node);   // check we used all attributes

  } // end of CMUSHclientDoc::Load_One_Timer_XML


UINT CMUSHclientDoc::Load_Macros_XML (CXMLelement & parent, 
                                       const unsigned long iFlags)
  {
UINT count = 0;

  LOAD_LOOP (parent, "macros", pMacros);

    GET_VERSION_AND_DEFAULTS (pMacros);

    if (m_CurrentPlugin)
      ThrowErrorException ("<macros> tag cannot be used inside a plugin");

    LOAD_LOOP (*pMacros, "macro", pElement);

    try
      {
      Load_One_Macro_XML (*pElement, iVersion, bUseDefault, iFlags);
      count++;
      }
    catch (CException* e)
      {
      HandleLoadException ("macro not loaded", e);
      } // end of catch

    END_LOAD_LOOP;

    CheckUsed (*pMacros);   // check we used all attributes

  END_LOAD_LOOP;      
      
  return count;
  }   // end of CMUSHclientDoc::Load_Macros_XML

extern CString strMacroDescriptions [MACRO_COUNT];

void CMUSHclientDoc::Load_One_Macro_XML (CXMLelement & node, 
                                         const long iVersion, 
                                         bool bUseDefault, 
                                         const unsigned long iFlags)
  {
CString strName,
        strType,
        strContents;

  Get_XML_boolean (node, DEFAULTS_LIT, bUseDefault, true); // see if they want defaults

  GET_STRING_TRIM   ("name", strName);
  GET_STRING_TRIM   ("type", strType);

  // find <send> text
  FindNodeContents (node, "send", strContents, true);

  // scan descriptions in the hope we find it :)

  int i;
  for (i = 0; i < NUMITEMS (strMacroDescriptions); i++)
    {
    if (strMacroDescriptions [i] == strName)
      break;
    } // end of finding which one

  if (i >= NUMITEMS (strMacroDescriptions))
    ThrowErrorException ("Macro named \"%s\" not recognised", (LPCTSTR) strName);

  unsigned short iType = REPLACE_COMMAND;

  if (strType == "replace")
    iType = REPLACE_COMMAND;
  else  if (strType == "send_now")
    iType = SEND_NOW;
  else  if (strType == "insert")
    iType = ADD_TO_COMMAND;
  else 
    if (strType.IsEmpty ()) 
      ThrowErrorException ("Macro type must be specified", (LPCTSTR) strType);
    else
      ThrowErrorException ("Macro type \"%s\" not recognised", (LPCTSTR) strType);

  m_macros [i] = strContents;
  m_macro_type [i] = iType;

  CheckUsed (node);   // check we used all attributes

  } // end of CMUSHclientDoc::Load_One_Macro_XML


UINT CMUSHclientDoc::Load_Variables_XML (CXMLelement & parent, 
                                         const unsigned long iMask,
                                         const unsigned long iFlags)
  {
UINT count = 0;

  LOAD_LOOP (parent, "variables", pVariables);

    GET_VERSION_AND_DEFAULTS (pVariables);

    LOAD_LOOP (*pVariables, "variable", pElement);

    try
      {
      Load_One_Variable_XML (*pElement, iMask, iVersion, bUseDefault, iFlags);
      count++;
      }
    catch (CException* e)
      {
      HandleLoadException ("variable not loaded", e);
      } // end of catch

    END_LOAD_LOOP;

    CheckUsed (*pVariables);   // check we used all attributes

  END_LOAD_LOOP;      

  return count;
  }   // end of CMUSHclientDoc::Load_Variables_XML

void CMUSHclientDoc::Load_One_Variable_XML (CXMLelement & node, 
                                            const unsigned long iMask,
                                            const long iVersion, 
                                            bool bUseDefault, 
                                            const unsigned long iFlags)
  {                           

CVariable * v = new CVariable;
CString strVariableName;
CString strNewContents;
bool bTrim;
      
  try
    {
    Get_XML_boolean (node, DEFAULTS_LIT, bUseDefault, true); // see if they want defaults
    
    Get_XML_string   (node, "name", v->strLabel, true);
    
    // see if they want spaces trimmed
    Get_XML_boolean (node, "trim", bTrim, false);

    // contents are between <variable> and </variable>
    strNewContents = node.strContent;

    if (bTrim)
      {
      strNewContents.TrimLeft ("\n\r\t ");
      strNewContents.TrimRight ("\n\r\t ");
      }

    strVariableName = v->strLabel;

    // return if bad name
    if (CheckObjectName (strVariableName))
        ThrowErrorException ("Invalid variable name \"%s\" ", 
                             (LPCTSTR) strVariableName);

    // get rid of old variable, if any
    CVariable * variable_item;
    if (GetVariableMap ().Lookup (strVariableName, variable_item))
      {
      // don't warn if new contents are the same :)
      if (variable_item->strContents != strNewContents)
        if (!(iMask & XML_OVERWRITE))
          LoadError (strVariableName, 
                     "overwriting existing variable contents",
                     node.iLine);
      delete variable_item;
      }

    v->strContents = strNewContents;

    } // end of try

  catch(CException*)
    {
    delete v; // get rid of variable
    throw;
    }

  GetVariableMap ().SetAt (strVariableName, v);

  CheckUsed (node);   // check we used all attributes

  } // end of CMUSHclientDoc::Load_One_Variable_XML


// colour groups

enum
  {
  eAnsiNormal,
  eAnsiBold,
  eCustom,
  };

UINT CMUSHclientDoc::Load_Colours_XML (CXMLelement & parent, 
                                       const unsigned long iFlags)
  {
UINT count = 0;

  LOAD_LOOP (parent, "colours", pColours);

    GET_VERSION_AND_DEFAULTS (pColours);

    if (m_CurrentPlugin)
      ThrowErrorException ("<colours> tag cannot be used inside a plugin");

    // look for ANSI group
    CXMLelement * pAnsi = FindNode (*pColours, "ansi");

    if (pAnsi)
      {
      // do normal and bold
      for (int i = 0; i < 2; i++)
        {

        // look for normal/bold groups

        CXMLelement * pNormal_Bold = FindNode (*pAnsi, i == 0 ? "normal" : "bold");

        if (pNormal_Bold)
          {

          LOAD_LOOP (*pNormal_Bold, "colour", pElement);

          try
            {
            Load_One_Colour_XML (*pElement, 
                                  iVersion, 
                                  i == 0 ? eAnsiNormal : eAnsiBold,
                                  bUseDefault, 
                                  iFlags);
            count++;
            }
          catch (CException* e)
            {
            HandleLoadException ("colour not loaded", e);
            } // end of catch
        
           END_LOAD_LOOP;

          CheckUsed (*pNormal_Bold);   // check we used all attributes
          }   // end of having normal or bold there
        } // end of normal/bold

      CheckUsed (*pAnsi);   // check we used all attributes
      }   // end of having ANSI colours

    // look for custom group
    CXMLelement * pCustom = FindNode (*pColours, "custom");

    if (pCustom)
      {
      LOAD_LOOP (*pCustom, "colour", pElement);

      try
          {
          Load_One_Colour_XML (*pElement, iVersion, eCustom, bUseDefault, iFlags);
          count++;
          }
        catch (CException* e)
          {
          HandleLoadException ("colour not loaded", e);
          } // end of catch

      END_LOAD_LOOP;

      CheckUsed (*pCustom);   // check we used all attributes
      }   // end of having custom colours


    CheckUsed (*pColours);   // check we used all attributes

  END_LOAD_LOOP;      

  return count;
  }   // end of CMUSHclientDoc::Load_Colours_XML

// iGroup tells us where to put the colour :)  - see enum above

void CMUSHclientDoc::Load_One_Colour_XML (CXMLelement & node, 
                                          const long iVersion, 
                                          const int iGroup,
                                          bool bUseDefault,
                                          const unsigned long iFlags)
  {
long iSequence;
int iMax;

  Get_XML_boolean (node, DEFAULTS_LIT, bUseDefault, true); // see if they want defaults


  if (iGroup == eCustom)
    iMax = NUMITEMS (m_customtext);
  else
    iMax = NUMITEMS (m_normalcolour);

  // we have to know which one to use
  if (!Get_XML_number   (node, "seq", iSequence, false, 1, iMax))
    ThrowErrorException ("Colour sequence (\"seq\") must be specified");

  iSequence--;    // make zero-relative

  switch (iGroup)
    {
    case eAnsiNormal:
      GET_COLOUR   ("rgb", m_normalcolour [iSequence]);
      break;

    case eAnsiBold:
      GET_COLOUR   ("rgb", m_boldcolour [iSequence]);
      break;

    case eCustom:
      GET_STRING_TRIM ("name", m_strCustomColourName [iSequence]);
      GET_COLOUR      ("text", m_customtext [iSequence]);
      GET_COLOUR      ("back", m_customback [iSequence]);
      break;
      
    default:
      ThrowErrorException ("Internal error, colour group %i not known", iGroup);

    } // end of switch

  CheckUsed (node);   // check we used all attributes

  } // end of CMUSHclientDoc::Load_One_Colour_XML


UINT CMUSHclientDoc::Load_Keypad_XML (CXMLelement & parent, 
                                       const unsigned long iFlags)
  {
UINT count = 0;

  LOAD_LOOP (parent, "keypad", pKeypad);

    GET_VERSION_AND_DEFAULTS (pKeypad);

    if (m_CurrentPlugin)
      ThrowErrorException ("<keypad> tag cannot be used inside a plugin");

    LOAD_LOOP (*pKeypad, "key", pElement);

    try
      {
      Load_One_Key_XML (*pElement, iVersion, bUseDefault, iFlags);
      count++;
      }
    catch (CException* e)
      {
      HandleLoadException ("key not loaded", e);
      } // end of catch

    END_LOAD_LOOP;

    CheckUsed (*pKeypad);   // check we used all attributes

  END_LOAD_LOOP;

  return count;
  }   // end of CMUSHclientDoc::Load_Keypad_XML


extern CString strKeypadNames [eKeypad_Max_Items];

void CMUSHclientDoc::Load_One_Key_XML (CXMLelement & node, 
                                       const long iVersion, 
                                       bool bUseDefault, 
                                       const unsigned long iFlags)
  {
CString strName,
        strContents;

  Get_XML_boolean (node, DEFAULTS_LIT, bUseDefault, true); // see if they want defaults

  Get_XML_string   (node, "name", strName, true);

  // find <send> text - trim whitespace
  FindNodeContents (node, "send", strContents, true);

  // scan descriptions in the hope we find it :)

  int i;
  for (i = 0; i < NUMITEMS (strKeypadNames); i++)
    {
    if (strKeypadNames [i] == strName)
      break;
    } // end of finding which one

  if (i >= NUMITEMS (strKeypadNames))
    ThrowErrorException ("Key named \"%s\" not recognised", (LPCTSTR) strName);

  m_keypad [i] = strContents;

  CheckUsed (node);   // check we used all attributes

  } // end of CMUSHclientDoc::Load_One_Key_XML


UINT CMUSHclientDoc::Load_Printing_XML (CXMLelement & parent, 
                                       const unsigned long iFlags)
  {
UINT count = 0;

  LOAD_LOOP (parent, "printing", pPrinting);

    GET_VERSION_AND_DEFAULTS (pPrinting);

    if (m_CurrentPlugin)
      ThrowErrorException ("<printing> tag cannot be used inside a plugin");

  // look for ANSI group
  CXMLelement * pAnsi = FindNode (*pPrinting, "ansi");

  if (pAnsi)
    {
    // do normal and bold
    for (int i = 0; i < 2; i++)
      {

      // look for ANSI group
      CXMLelement * pNormal_Bold = FindNode (*pAnsi, i == 0 ? "normal" : "bold");

      if (pNormal_Bold)
        {

        LOAD_LOOP (*pNormal_Bold, "style", pElement);

          try
            {
            Load_One_Print_Colour_XML (*pElement, 
                                  iVersion, 
                                  i == 0 ? eAnsiNormal : eAnsiBold,
                                  bUseDefault,
                                  iFlags);
            count++;
            }
          catch (CException* e)
            {
            HandleLoadException ("style not loaded", e);
            } // end of catch
        END_LOAD_LOOP;

        CheckUsed (*pNormal_Bold);   // check we used all attributes

        }   // end of having normal or bold there

      } // end of normal/bold

    CheckUsed (*pAnsi);   // check we used all attributes
    }


    CheckUsed (*pPrinting);   // check we used all attributes

  END_LOAD_LOOP;

  return count;
  }   // end of CMUSHclientDoc::Load_Printing_XML

// iGroup tells us where to put the colour :)  - see enum above

void CMUSHclientDoc::Load_One_Print_Colour_XML (CXMLelement & node, 
                                          const long iVersion, 
                                          const int iGroup,
                                          bool bUseDefault, 
                                          const unsigned long iFlags)
  {
long iSequence;

  Get_XML_boolean (node, DEFAULTS_LIT, bUseDefault, true); // see if they want defaults

  // we have to know which one to use
  if (!Get_XML_number   (node, "seq", iSequence, false, 1, NUMITEMS (m_nNormalPrintStyle)))
    ThrowErrorException ("Style sequence (\"seq\") must be specified");

  iSequence--;    // make zero-relative

  switch (iGroup)
    {
    case eAnsiNormal:
      GET_BOOL_INT_MASK  ("bold",       m_nNormalPrintStyle [iSequence], FONT_BOLD);
      GET_BOOL_INT_MASK  ("italic",     m_nNormalPrintStyle [iSequence], FONT_ITALIC);
      GET_BOOL_INT_MASK  ("underline",  m_nNormalPrintStyle [iSequence], FONT_UNDERLINE);
      break;

    case eAnsiBold:
      GET_BOOL_INT_MASK  ("bold",       m_nBoldPrintStyle [iSequence], FONT_BOLD);
      GET_BOOL_INT_MASK  ("italic",     m_nBoldPrintStyle [iSequence], FONT_ITALIC);
      GET_BOOL_INT_MASK  ("underline",  m_nBoldPrintStyle [iSequence], FONT_UNDERLINE);
      break;

    default:
      ThrowErrorException ("Internal error, style group %i not known", iGroup);

    } // end of switch

  CheckUsed (node);   // check we used all attributes

  } // end of CMUSHclientDoc::Load_One_Print_Colour_XML


#pragma warning(push)
#pragma warning(disable : 4189)     // warning C4189: 'iVersion' : local variable is initialized but not referenced
void CMUSHclientDoc::Load_Comments_XML (CXMLelement & parent)
  {
CString strComment;
int iFlags = 0; // for use by GET_VERSION_AND_DEFAULTS macro

  LOAD_LOOP (parent, "comment", pComments);

    GET_VERSION_AND_DEFAULTS (pComments);
    
    strComment += pComments->strContent;

  END_LOAD_LOOP;

  strComment.TrimLeft ();
  strComment.TrimRight ();

  if (strComment.IsEmpty ())
    return;   // no comment

  strComment += ENDLINE;    // better finish on a new line

  CString strTitle = "XML import notes - ";
  strTitle += strFileName;

  AppendToTheNotepad (strTitle, 
                      strComment,                 // start new line
                      false,   // append
                      eNotepadXMLcomments);

  ActivateNotepad (strTitle);

  } // end of CMUSHclientDoc::Load_Comments_XML
#pragma warning(pop)

void CMUSHclientDoc::Load_Plugin_XML (CXMLelement & parent)
  {

  // see if we have "plugin" tag(s)

  LOAD_LOOP (parent, "plugin", node);
  
    try
      {
      double dActualVersion;
      myAtoF (MUSHCLIENT_VERSION,  &dActualVersion);

      if (m_CurrentPlugin)
        ThrowErrorException ("Can only have one plugin per file");

      m_CurrentPlugin = new CPlugin (this);

      if (strFileName.Find ("\\") == -1 &&
          strFileName.Find ("/") == -1)
        strFileName = CString (Make_Absolute_Path (App.m_strPluginsDirectory)) + strFileName;

      m_CurrentPlugin->m_strSource = strFileName;
      // work out directory
      m_CurrentPlugin->m_strDirectory = ExtractDirectory (strFileName);

      Get_XML_string (*node, "name", m_CurrentPlugin->m_strName, false, true);
      Get_XML_string (*node, "author", m_CurrentPlugin->m_strAuthor, false, true);
      Get_XML_string (*node, "language", m_CurrentPlugin->m_strLanguage, false, true);
      Get_XML_string (*node, "id", m_CurrentPlugin->m_strID, false, true);
      Get_XML_string (*node, "purpose", m_CurrentPlugin->m_strPurpose, false, true);
      Get_XML_date   (*node, "date_written", m_CurrentPlugin->m_tDateWritten, false);
      Get_XML_date   (*node, "date_modified", m_CurrentPlugin->m_tDateModified, false);
      Get_XML_boolean (*node, "save_state", m_CurrentPlugin->m_bSaveState, false);
      Get_XML_double (*node, "requires", m_CurrentPlugin->m_dRequiredVersion, false, 0.0, 1000.0);
      Get_XML_double (*node, "version", m_CurrentPlugin->m_dVersion, false, 0.0);
      
      // little message for the status bar
      Frame.SetStatusMessageNow (TFormat ("%s loading plugin %s ...",
                                  (LPCTSTR) m_mush_name,
                                  (LPCTSTR) m_CurrentPlugin->m_strName));

      // check we can support this plugin
      if (m_CurrentPlugin->m_dRequiredVersion > dActualVersion)
         ThrowErrorException ("Plugin requires MUSHclient version %g or above",
                              m_CurrentPlugin->m_dRequiredVersion);

      // check name
      if (m_CurrentPlugin->m_strName.IsEmpty ())
         ThrowErrorException ("Plugin must have a name");

      CString strTemp = m_CurrentPlugin->m_strName;   // stop name going to lower case
      if (CheckObjectName (strTemp))
         ThrowErrorException ("Plugin name is invalid");

      if (m_CurrentPlugin->m_strName.GetLength () > 32)
         ThrowErrorException ("Plugin name cannot be > 32 characters");

      // check author
      if (m_CurrentPlugin->m_strAuthor.GetLength () > 32)
         ThrowErrorException ("Plugin author name cannot be > 32 characters");

      // check purpose
      if (m_CurrentPlugin->m_strPurpose.GetLength () > 100)
         ThrowErrorException ("Plugin purpose cannot be > 100 characters");

      // check unique ID
      if (m_CurrentPlugin->m_strID.IsEmpty ())
         ThrowErrorException ("Plugin must have a unique \"id\" field");

      if (m_CurrentPlugin->m_strID.GetLength () != PLUGIN_UNIQUE_ID_LENGTH)
         ThrowErrorException ("Plugin \"id\" field must be %i characters long", 
                              PLUGIN_UNIQUE_ID_LENGTH);

      // make sure hex characters
      const char * p = m_CurrentPlugin->m_strID;
      for ( ; *p ; p++)
        if (!isxdigit (*p))
         ThrowErrorException ("Plugin \"id\" field must be %i hex characters",
                              PLUGIN_UNIQUE_ID_LENGTH);

      m_CurrentPlugin->m_strID.MakeLower ();

      // check ID is unique
      for (POSITION pos = m_PluginList.GetHeadPosition(); pos; )
        {
        CPlugin * p = m_PluginList.GetNext (pos);
        if (m_CurrentPlugin->m_strID == p->m_strID)
           ThrowErrorException ("The plugin '%s' is already loaded.", p->m_strName);
        }      // end of looping through each plugins

      // check language
      m_CurrentPlugin->m_strLanguage.MakeLower ();
      if (!m_CurrentPlugin->m_strLanguage.IsEmpty ())
        if (m_CurrentPlugin->m_strLanguage != "vbscript" &&
            m_CurrentPlugin->m_strLanguage != "vbscript.encode" &&
            m_CurrentPlugin->m_strLanguage != "jscript" &&
            m_CurrentPlugin->m_strLanguage != "jscript.encode" &&
            m_CurrentPlugin->m_strLanguage != "perlscript" 
#ifdef USE_PYTHON
            && m_CurrentPlugin->m_strLanguage != "python"
#endif
#ifdef USE_TCL
            && m_CurrentPlugin->m_strLanguage != "tclscript"
#endif
            && m_CurrentPlugin->m_strLanguage != "lua"
#ifdef USE_PHP
            && m_CurrentPlugin->m_strLanguage != "phpscript"
#endif
#ifdef USE_RUBY
            && m_CurrentPlugin->m_strLanguage != "rubyscript"
#endif
            )
            ThrowErrorException ("Language \"%s\" not supported",
                  (LPCTSTR) m_CurrentPlugin->m_strLanguage);

      // find <description> text
      FindNodeContents (*node, "description", m_CurrentPlugin->m_strDescription, false);

      CString strSavedFileName = strFileName;
      int iSavedLineLastItemFound = iLineLastItemFound;
      int iSavedErrorCount = iErrorCount;

      CFile * f = NULL;
      CArchive * ar = NULL;
      CXMLparser parser;

      strFileName = CString (Make_Absolute_Path (App.m_strDefaultStateFilesDirectory));
      // need a directory
        
      if (App.m_strPluginsDirectory.IsEmpty ())
        ThrowErrorException ("No plugins directory specified");

      // need a world id

      if (m_strWorldID.IsEmpty ())
        ThrowErrorException ("No world ID specified");

      strFileName += m_strWorldID;    // world ID
      strFileName += "-";
      strFileName += m_CurrentPlugin->m_strID;                 // plugin ID
      strFileName += "-state.xml";            // suffix

//      ::TMessageBox ("Plugin Load State");

      try
        {
        f = new CFile (strFileName, CFile::modeRead | CFile::shareDenyWrite);

        ar = new CArchive(f, CArchive::load);

        if (IsArchiveXML (*ar))
          {

          try
            {
            Load_World_XML (*ar, 
                            XML_VARIABLES | XML_NO_PLUGINS | XML_OVERWRITE,
                            0);
            }
          catch (CArchiveException* e)
            {
            e->Delete ();
            ThrowErrorException ("Error processing plugin state file \"%s\"",
                                 (LPCTSTR) strFileName);
            } // end of catch

          } 
        else
          {
          // ignore empty state files
          if (ar->GetFile ()->GetLength () > 0)
            ThrowErrorException ("Plugin state \"%s\" is not an XML file",
                                 (LPCTSTR) strFileName);

          }  // end of not archive
        } // end of try

      catch(CFileException*)
        {
        // silently ignore no state file :)
//          HandleLoadException ("plugin state file not found, assuming empty", e);
        }

      catch(CException*)
        {
        strFileName = strSavedFileName;
        iLineLastItemFound = iSavedLineLastItemFound;
        iErrorCount = iSavedErrorCount;
        delete ar;
        delete f;
        throw;
        }

      strFileName = strSavedFileName;
      iLineLastItemFound = iSavedLineLastItemFound;
      iErrorCount = iSavedErrorCount;
      delete ar;
      delete f;


      }
    catch (CException*)
      {
      delete m_CurrentPlugin; // get rid of plugin
      m_CurrentPlugin = NULL;     // no plugin active right now
      throw;  // don't process this file any more
      } // end of catch

    CheckUsed (*node);

  END_LOAD_LOOP;

  }   // end of CMUSHclientDoc::Load_Plugin_XML 


void CMUSHclientDoc::Load_Scripts_XML (CXMLelement & parent)
  {

  // see if we have "script" tag(s)

  LOAD_LOOP (parent, "script", pScriptElement);
  
    try
      {
      if (m_CurrentPlugin->m_strLanguage.IsEmpty ())
        ThrowErrorException ("Cannot have <script> tag if no language specified");

      m_CurrentPlugin->m_strScript += pScriptElement->strContent;
      m_CurrentPlugin->m_strScript += ENDLINE;        // ensure trailing newline

      }
    catch (CException* e)
      {
      HandleLoadException ("script not loaded", e);
      } // end of catch

    CheckUsed (*pScriptElement);

  END_LOAD_LOOP;


  }   // end of CMUSHclientDoc::Load_Scripts_XML 



void CMUSHclientDoc::InternalLoadPlugin (const CString & strName)
  {
CFile * f = NULL;
CArchive * ar = NULL;

  try
    {
    f = new CFile (strName, CFile::modeRead | CFile::shareDenyWrite);

    ar = new CArchive(f, CArchive::load);

    // do it
    Load_World_XML (*ar, 
                            XML_TRIGGERS | 
                            XML_ALIASES |
                            XML_TIMERS |
                            XML_VARIABLES |
                            XML_PLUGINS, 
                            0);           // load flags
                             
    SetModifiedFlag (TRUE);   // document has now changed


    } // end of try block

  catch (CException *)
    {
    delete ar;      // delete archive
    delete f;       // delete file
    throw;
    } // end of catching a file exception

  delete ar;      // delete archive
  delete f;       // delete file


  } // end of CMUSHclientDoc::InternalLoadPlugin 
