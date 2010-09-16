#include "stdafx.h"
#include "..\mainfrm.h"
#include "..\MUSHclient.h"
#include "..\doc.h"
#include "..\dialogs\ScriptErrorDlg.h"
#include <io.h>  // for popen
#include <fcntl.h>  // for popen
#include "..\pcre\config.h"
#include "..\pcre\pcre_internal.h"
#include "..\luacom\luacom.h"

set<string> LuaFunctionsSet;
set<string> LuaTablesSet;

extern "C"
  {
  LUALIB_API int luaopen_rex(lua_State *L);
  LUALIB_API int luaopen_bits(lua_State *L);
  LUALIB_API int luaopen_compress(lua_State *L);
  LUALIB_API int luaopen_bc(lua_State *L);
  LUALIB_API int luaopen_lsqlite3(lua_State *L);
  LUALIB_API int luaopen_lpeg (lua_State *L);
  }

//LUALIB_API int luaopen_trie(lua_State *L);

LUALIB_API int luaopen_progress_dialog(lua_State *L);

static void BuildOneLuaFunction (lua_State * L, const char * sTableName)
  {
  lua_settop (L, 0);    // get rid of stuff lying around

  // get global table
  lua_getglobal (L, sTableName);  

  const int table = 1;

  if (!lua_istable (L, table))
    return;  // aha! caught you!

  // standard Lua table iteration
  for (lua_pushnil (L); lua_next (L, table) != 0; lua_pop (L, 1))
    {
    // extract function names
    if (lua_isfunction (L, -1))
      {
      string sName (luaL_checkstring (L, -2));

      // don't add stuff like __index
      if (sName [0] != '_')
        {
        string sFullName;

        // global table will not have _G prefix
        if (strcmp (sTableName, "_G") != 0)
          {
          sFullName= sTableName;
          sFullName += ".";
          }

        // now the name
        sFullName += sName;

        // add to set
        LuaFunctionsSet.insert (sFullName);
        }  // not prefixed by _
      }   // end if function
    }  // end for loop

  lua_pop (L, 1); // get rid of global table now
  } // end of BuildOneLuaFunction

static void BuildOneLuaTable (lua_State * L, const char * sTableName)
  {
  lua_settop (L, 0);    // get rid of stuff lying around

  // get global table
  lua_getglobal (L, sTableName);  

  const int table = 1;

  if (!lua_istable (L, table))
    return;  // aha! caught you!

  // standard Lua table iteration
  for (lua_pushnil (L); lua_next (L, table) != 0; lua_pop (L, 1))
    {
    if (lua_isstring (L, -2))
      {

      string sName (lua_tostring (L, -2));

      string sFullName = sTableName;
      sFullName += "." + sName;   // table.entry, eg. sendto.world

      // add to set
      LuaTablesSet.insert (sFullName);
      } // if string key
  }  // end for loop

  lua_pop (L, 1); // get rid of global table now
  } // end of BuildOneLuaTable


// for world completion, and help lookup, find all functions in the following tables
static void BuildLuaFunctions (lua_State * L)
  {
  const char * table_names [] = {
      "_G",
      "string",
      "package",
      "os",
      "io",
      "bc",
      "progress",
      "bit",
      "rex",
      "utils",
      "table",
      "math",
      "debug",
      "coroutine",
      "lpeg",
      "sqlite3",

      // add more tables here

      ""   // end of table marker
    };

  // add all functions from each table
  for (int i = 0; table_names [i] [0]; i++)
    BuildOneLuaFunction (L, table_names [i]);

  // experimental function - don't show it
  LuaFunctionsSet.erase ("newproxy");

  } // end of BuildLuaFunctions

// for word completion, find all entries in the following tables
static void BuildLuaTables (lua_State * L)
  {
  const char * table_names [] = {
      "trigger_flag",
      "alias_flag",
      "timer_flag",
      "custom_colour",
      "error_code",
//      "error_desc",   // don't do this, wrong table type
      "sendto",
      "miniwin",

      // add more tables here

      ""   // end of table marker
    };

  // add all functions from each table
  for (int i = 0; table_names [i] [0]; i++)
    BuildOneLuaTable (L, table_names [i]);

  } // end of BuildLuaTables

// sigh - luacom_open returns void
int luacom_open_glue (lua_State *L)
  {
  luacom_open (L);
  return 0;
  }

// I need this extra function to avoid:
// Compiler Error C2712
// cannot use __try in functions that require object unwinding

void CScriptEngine::OpenLuaDelayed ()
  {
  L = MakeLuaState();   /* opens Lua */
  if (!L)
    return;         // can't open Lua

  luaL_openlibs (L);           // open all standard Lua libraries

  // call as Lua function because we need the environment
  lua_pushcfunction(L, RegisterLuaRoutines);
  lua_pushstring(L, DOCUMENT_STATE);  /* push address */
  lua_pushlightuserdata(L, (void *)m_pDoc);    /* push value */
  lua_call(L, 2, 0);

  luaopen_rex (L);    // regular expression library
  luaopen_bits (L);   // bit manipulation library
  luaopen_compress (L);  // compression (utils) library
  luaopen_progress_dialog (L);    // progress dialog
//  luaopen_trie (L);   // open trie library
  luaopen_bc (L);   // open bc library   
  luaopen_lsqlite3 (L);   // open sqlite library

  lua_pushcfunction (L, luaopen_lpeg);   // open lpeg library
  lua_call (L, 0, 0);

  lua_settop(L, 0);   // clear stack

  // for function-name completion, and help
  if (LuaFunctionsSet.empty ())
    {
    BuildLuaFunctions (L);
    BuildLuaTables (L);
    }

  // unless they explicitly enable it, remove ability to load DLLs
  DisableDLLs (L);

  // add luacom to package.preload
  lua_getglobal (L, LUA_LOADLIBNAME);  // package table

  if (lua_istable (L, -1))    // if it exists and is a table
    {
    lua_getfield (L, -1, "preload");  // get preload table inside it

    if (lua_istable (L, -1))   // preload table exists
      {
      lua_pushcfunction(L, luacom_open_glue);   // luacom open
      lua_setfield(L, -2, "luacom");          
      } // have package.preload table

    lua_pop (L, 1);   // get rid of preload table from stack
    } // have package table

  lua_pop (L, 1);   // get rid of package table from stack

  // this is so useful I am adding it in (see check.lua)
  ParseLua ( \
  "function check (result)  \
    if result ~= error_code.eOK then\
      error (error_desc [result] or \
             string.format (\"Unknown error code: %i\", result), 2) end; end", 
  "Check function");

  // preliminary sand-box stuff
  m_pDoc->m_iCurrentActionSource = eLuaSandbox;
  ParseLua (App.m_strLuaScript, "Sandbox");

  m_pDoc->m_iCurrentActionSource = eUnknownActionSource;

  lua_settop(L, 0);   // clear stack

  } // end of OpenLuaDelayed

void CScriptEngine::OpenLua ()
  {

   OpenLuaDelayed ();

  }   // end of CScriptEngine::OpenLua

void CScriptEngine::CloseLua ()
  {
  if (L)
    {
    lua_close (L);
    L = NULL;
    }
  }  // end of CScriptEngine::CloseLua

// send some code to Lua to be parsed
bool CScriptEngine::ParseLua (const CString & strCode, const CString & strWhat)
  {

  // safety check ;)
  if (!L)
    return true;

  LARGE_INTEGER start, 
                finish;

  if (App.m_iCounterFrequency)
    QueryPerformanceCounter (&start);
  else
    {
    start.QuadPart = 0;
    finish.QuadPart = 0;
    }


  // note - an error here is a *compile* error
  if (luaL_loadbuffer(L, strCode, strCode.GetLength (), strWhat))
    {
    LuaError (L, "Compile error", "", "", "", m_pDoc);
    return true;
    }

  int error = CallLuaWithTraceBack (L, 0, 0);

  // note - an error here is a *runtime* error
  if (error)
    {
    LuaError (L, "Run-time error", "", "", "", m_pDoc);
    return true;
    }

  lua_settop(L, 0);   // clear stack

// -----------------

  if (App.m_iCounterFrequency)
    {
    QueryPerformanceCounter (&finish);
    m_pDoc->m_iScriptTimeTaken += finish.QuadPart - start.QuadPart;
    }

  return false;

  }

// For Lua we will simply use the DISPID as a flag to indicate if we found
// the function or not, to speed up subsequent calls (calls to non-existent 
// functions). Also, if the function later has an error we set the DISPID
// to DISPID_UNKNOWN as a flag to not call it continuously.

// Version 3.75+ supports dotted functions (eg. string.gsub)

DISPID CScriptEngine::GetLuaDispid (const CString & strName)
  {
  return (L && FindLuaFunction (L, strName))
          ? 1 : DISPID_UNKNOWN;  // if known 1 is flag, otherwise DISPID_UNKNOWN
       
  }


void LuaError (lua_State *L, 
                LPCTSTR strEvent,
                LPCTSTR strProcedure,
                LPCTSTR strType,
                LPCTSTR strReason,
                CMUSHclientDoc * pDoc)
  {    
  CScriptErrorDlg dlg;

  if (!strlen (strProcedure) == 0)
    {
    dlg.m_strCalledBy = "Function/Sub: ";
    dlg.m_strCalledBy += strProcedure;
    dlg.m_strCalledBy += " called by ";
    dlg.m_strCalledBy += strType;
    dlg.m_strCalledBy += ENDLINE;
    dlg.m_strCalledBy += "Reason: ";
    dlg.m_strCalledBy += strReason;
    }
  else
    {
    dlg.m_strCalledBy = "Immediate execution";

/*    dlg.m_strDescription += ENDLINE;
    dlg.m_strDescription += "Line in error: ";
    dlg.m_strDescription += ENDLINE;
    dlg.m_strDescription += bstr;
*/
    }

  dlg.m_strEvent = strEvent;
  dlg.m_strDescription = lua_tostring(L, -1);
  lua_settop(L, 0);   // clear stack

  dlg.m_strRaisedBy = "No active world";

  if (pDoc)
    {
    if (pDoc->m_CurrentPlugin)
      {
      dlg.m_strRaisedBy = "Plugin: " +  pDoc->m_CurrentPlugin->m_strName;
      dlg.m_strRaisedBy += " (called from world: " + pDoc->m_mush_name + ")";
      }
    else 
      dlg.m_strRaisedBy = "World: " + pDoc->m_mush_name;
    }

  if (!pDoc || !pDoc->m_bScriptErrorsToOutputWindow)
    {
    if (pDoc)
      dlg.m_bHaveDoc = true;
    dlg.DoModal ();
    if (pDoc && dlg.m_bUseOutputWindow)
      {
      pDoc->m_bScriptErrorsToOutputWindow = true;
      pDoc->SetModifiedFlag (TRUE);
      }

    }
  else
    {
    pDoc->ColourNote (SCRIPTERRORFORECOLOUR, SCRIPTERRORBACKCOLOUR, strEvent);
    pDoc->ColourNote (SCRIPTERRORFORECOLOUR, SCRIPTERRORBACKCOLOUR, dlg.m_strRaisedBy);
    pDoc->ColourNote (SCRIPTERRORFORECOLOUR, SCRIPTERRORBACKCOLOUR, dlg.m_strCalledBy);
    pDoc->ColourNote (SCRIPTERRORFORECOLOUR, SCRIPTERRORBACKCOLOUR, dlg.m_strDescription);

// show bad lines?

    bool bImmediate = true;
    int nLine = 0;

    if (dlg.m_strDescription.Left (18) == "[string \"Plugin\"]:")
      {
      bImmediate = false;
      nLine = atoi (dlg.m_strDescription.Mid (18));
      }
    else if (dlg.m_strDescription.Left (23) == "[string \"Script file\"]:")
      {
      bImmediate = false;
      nLine = atoi (dlg.m_strDescription.Mid (23));
      }

    if (!bImmediate)
      pDoc->ShowErrorLines (nLine);
    
    }  // end of showing in output window

  }   // end of LuaError

// returns true if error
bool CScriptEngine::ExecuteLua (DISPID & dispid,  // dispatch ID, will be set to DISPID_UNKNOWN on an error
                                LPCTSTR szProcedure,      // eg. ON_TRIGGER_XYZ
                                const unsigned short iReason,  // value for m_iCurrentActionSource
                                LPCTSTR szType,           // eg. trigger, alias
                                LPCTSTR szReason,         // eg. trigger subroutine XXX
                                list<double> & nparams,   // list of number parameters
                                list<string> & sparams,   // list of string parameters
                                long & nInvocationCount,  // count of invocations
                                const t_regexp * regexp,  // regular expression (for triggers, aliases)
                                map<string, string> * table,   // map of other things
                                CPaneLine * paneline,     // and the line (for triggers)
                                bool * result)            // where to put result


  {

  // safety check ;)
  if (!L)
    return false;

  // don't do it if previous problems
  if (dispid == DISPID_UNKNOWN)
    return false;

  lua_settop (L, 0);  // start with empty stack

  LARGE_INTEGER start, 
                finish;

  m_pDoc->Trace (TFormat ("Executing %s script \"%s\"", szType, szProcedure));

  if (App.m_iCounterFrequency)
    QueryPerformanceCounter (&start);
  else
    {
    start.QuadPart = 0;
    finish.QuadPart = 0;
    }
             
  unsigned short iOldStyle = m_pDoc->m_iNoteStyle;
  m_pDoc->m_iNoteStyle = NORMAL;    // back to default style

  if (!GetNestedFunction (L, szProcedure, true))
    {
    dispid = DISPID_UNKNOWN;   // stop further invocations
    return true;    // error return
    }

  int paramCount = 0;

  // push all supplied number parameters
  for (list<double>::const_iterator niter = nparams.begin ();
       niter != nparams.end ();
       niter++)
     lua_pushnumber (L, *niter);

  paramCount += nparams.size ();

  // push all supplied string parameters
  for (list<string>::const_iterator siter = sparams.begin ();
       siter != sparams.end ();
       siter++)
     lua_pushlstring (L, siter->c_str (), siter->size ());

  paramCount += sparams.size ();

// if we have a regular expression, push the wildcards

  if (regexp)
    {
    int i;
    int ncapt;
    int namecount;
    unsigned char *name_table;
    int name_entry_size;
    unsigned char *tabptr;
    lua_newtable(L);                                                            
    paramCount++;   // we have one more parameter to the call
    pcre_fullinfo(regexp->m_program, regexp->m_extra, PCRE_INFO_CAPTURECOUNT, &ncapt);

    for (i = 0; i <= ncapt; i++) 
      {
      string wildcard (regexp->GetWildcard (i));
      lua_pushlstring (L, wildcard.c_str (), wildcard.size ());
      lua_rawseti (L, -2, i);
    }

    /* now do named subpatterns  */
    pcre_fullinfo(regexp->m_program, regexp->m_extra, PCRE_INFO_NAMECOUNT, &namecount);
    if (namecount > 0)
      {
      pcre_fullinfo(regexp->m_program, regexp->m_extra, PCRE_INFO_NAMETABLE, &name_table);
      pcre_fullinfo(regexp->m_program, regexp->m_extra, PCRE_INFO_NAMEENTRYSIZE, &name_entry_size);
      tabptr = name_table;
      set<string> found_strings;
      for (i = 0; i < namecount; i++, tabptr += name_entry_size) 
        {
        int n = (tabptr[0] << 8) | tabptr[1];
        const unsigned char * name = tabptr + 2;
        // if duplicates were possible then ...
        if ((regexp->m_program->options & (PCRE_DUPNAMES | PCRE_JCHANGED)) != 0)
          {
          // this code is to ensure that we don't find a match (eg. mob = Kobold)
          // and then if duplicates were allowed, replace Kobold with false.

          string sName = (LPCTSTR) name;

          // for duplicate names, see if we already added this name
          if (found_strings.find (sName) != found_strings.end ())
            {
            // do not replace if this one is out of range
            if (n < 0 || n > ncapt)
              continue;
            } // end of duplicate
          else
            found_strings.insert (sName);
          }

        lua_pushstring (L, (LPCTSTR) name);
        if (n >= 0 && n <= ncapt) 
          {
          string wildcard (regexp->GetWildcard (n));
          lua_pushlstring (L, wildcard.c_str (), wildcard.size ());
          }
        else
          lua_pushnil (L);  /* n out of range */
        lua_settable (L, -3);
        
        }   // end of wildcard loop
      } // end of having named wildcards
    } // end of having a regexp

  if (table)
    {
    lua_newtable (L);                                                            
    paramCount++;   // we have one more parameter to the call

    // add each item to the table
    for (map<string, string>::const_iterator iter = table->begin ();
         iter != table->end ();
         iter++)
           {
           lua_pushstring (L, iter->first.c_str ());
           lua_pushstring (L, iter->second.c_str ());
           lua_settable(L, -3);
           }  // end of doing each one

    } // end of having an optional table

  if (paneline)
    {
    lua_newtable(L);                                                            
    paramCount++;   // we have one more parameter to the call
    int i = 1;          // style run number

    for (CPaneStyleVector::iterator style_it = paneline->m_vStyles.begin (); 
         style_it != paneline->m_vStyles.end (); 
         style_it++, i++)
      {
      lua_newtable(L);                                                            
      MakeTableItem     (L, "text",         (*style_it)->m_sText); 
      MakeTableItem     (L, "length",       (*style_it)->m_sText.length ());  
      MakeTableItem     (L, "textcolour",   (*style_it)->m_cText);  
      MakeTableItem     (L, "backcolour",   (*style_it)->m_cBack);  
      MakeTableItem     (L, "style",        (*style_it)->m_iStyle); 

      lua_rawseti (L, -2, i);  // set table item as number of style
      }

    }   // end of having an optional style run thingo

  if (iReason != eDontChangeAction)
    m_pDoc->m_iCurrentActionSource = iReason;

  int error = CallLuaWithTraceBack (L, paramCount, LUA_MULTRET);

  if (iReason != eDontChangeAction)
    m_pDoc->m_iCurrentActionSource = eUnknownActionSource;

// -----------------

  m_pDoc->m_iNoteStyle = iOldStyle;

  if (error)
    {
    dispid = DISPID_UNKNOWN;   // stop further invocations
    LuaError (L, "Run-time error", szProcedure, szType, szReason, m_pDoc);
    return true;    // error return
    }

  nInvocationCount++;   // count number of times used

  if (App.m_iCounterFrequency)
    {
    QueryPerformanceCounter (&finish);
    m_pDoc->m_iScriptTimeTaken += finish.QuadPart - start.QuadPart;
    }

  if (result)
    {
    *result = true;

    // if a boolean result wanted, return it

    if (lua_gettop (L) > 0)
      {
      if (lua_type (L, 1) == LUA_TBOOLEAN)
        *result = lua_toboolean (L, 1);
      else
        *result = lua_tonumber (L, 1);  // I use number rather than boolean
                                       // because 0 is considered true in Lua
      }
    }   // end of result wanted

  lua_settop (L, 0);  // discard any results now
  
  return false;   // no error
  } // end of CScriptEngine::ExecuteLua 

// returns true if script error
bool CScriptEngine::ExecuteLua (DISPID & dispid,          // dispatch ID, will be set to DISPID_UNKNOWN on an error
                               LPCTSTR szProcedure,      // eg. ON_TRIGGER_XYZ
                               const unsigned short iReason,  // value for m_iCurrentActionSource
                               LPCTSTR szType,           // eg. trigger, alias
                               LPCTSTR szReason,         // eg. trigger subroutine XXX
                               CString strParam,         // string parameter
                               long & nInvocationCount,  // count of invocations
                               CString & result)         // where to put result
  {

  // safety check ;)
  if (!L)
    return false;

  // don't do it if previous problems
  if (dispid == DISPID_UNKNOWN)
    return false;

  lua_settop (L, 0);  // start with empty stack

  LARGE_INTEGER start, 
                finish;

  m_pDoc->Trace (TFormat ("Executing %s script \"%s\"", szType, szProcedure));

  if (App.m_iCounterFrequency)
    QueryPerformanceCounter (&start);
  else
    {
    start.QuadPart = 0;
    finish.QuadPart = 0;
    }
             
  unsigned short iOldStyle = m_pDoc->m_iNoteStyle;
  m_pDoc->m_iNoteStyle = NORMAL;    // back to default style

  if (!GetNestedFunction (L, szProcedure, true))
    {
    dispid = DISPID_UNKNOWN;   // stop further invocations
    return true;    // error return
    }

  lua_pushstring (L, strParam);    // the solitary argument

  if (iReason != eDontChangeAction)
    m_pDoc->m_iCurrentActionSource = iReason;

  int error = CallLuaWithTraceBack (L, 1, LUA_MULTRET);

  if (iReason != eDontChangeAction)
    m_pDoc->m_iCurrentActionSource = eUnknownActionSource;

  m_pDoc->m_iNoteStyle = iOldStyle;

  if (error)
    {
    dispid = DISPID_UNKNOWN;   // stop further invocations
    LuaError (L, "Run-time error", szProcedure, szType, szReason, m_pDoc);
    return true;    // error return
    }

  nInvocationCount++;   // count number of times used

  if (App.m_iCounterFrequency)
    {
    QueryPerformanceCounter (&finish);
    m_pDoc->m_iScriptTimeTaken += finish.QuadPart - start.QuadPart;
    }

  if (lua_gettop (L) > 0 && lua_isstring (L, 1))
    {
    // get result

    size_t textLength;
    const char * text = luaL_checklstring (L, 1, &textLength);

    result = CString (text, textLength);
    }

  lua_settop (L, 0);  // discard any results now

  return false;   // no error

  } // end of CScriptEngine::ExecuteLua 


// bugfix thanks to Twisol (previous version didn't handle bad debug table properly)
void GetTracebackFunction (lua_State *L)
  {
  // get debug table
  lua_getfield (L, LUA_GLOBALSINDEX, LUA_DBLIBNAME);

  // if it is a table then find traceback function
  if (lua_istable (L, -1))
    {
    lua_getfield (L, -1, "traceback");
    
    // remove debug table, leave traceback function
    lua_remove (L, -2);

    // if it is indeed a function, well and good
    if (lua_isfunction (L, -1))
      return; 
    }

  // not a table, or debug.traceback not a function, get rid of it
  lua_pop (L, 1);

  // return nil to caller
  lua_pushnil (L);
  return; 

  }

int CallLuaWithTraceBack (lua_State *L, const int iArguments, const int iReturn)
  {

  int error;
  int base = lua_gettop (L) - iArguments;  /* function index */
  GetTracebackFunction (L);
  if (lua_isnil (L, -1))
    {
    lua_pop (L, 1);   // pop non-existent function
    error = lua_pcall (L, iArguments, iReturn, 0);
    }  
  else
    {
    lua_insert (L, base);  /* put it under chunk and args */
    error = lua_pcall (L, iArguments, iReturn, base);
    lua_remove (L, base);  /* remove traceback function */
    }

  return error;
  }  // end of CallLuaWithTraceBack

//-------------------------------------------------------------------------------
// stuff for popen, pclose
//-------------------------------------------------------------------------------

// see: http://lua-users.org/wiki/PipesOnWindows

/*

  This is all pretty crappy.

  I don't like the way it builds up a command (eg. cmd.exe /c blah blah )

  We need to tap into the metatable for io to handle the close correctly.

  And, it just crashes when I test it. ;)

  */


#undef POPEN_STUFF_WHICH_DOESNT_WORK

#ifdef POPEN_STUFF_WHICH_DOESNT_WORK
/*------------------------------------------------------------------------------
  Globals for the Routines pt_popen() / pt_pclose()
------------------------------------------------------------------------------*/
static HANDLE my_pipein[2], my_pipeout[2], my_pipeerr[2];
static char   my_popenmode = ' ';

static int
my_pipe(HANDLE *readwrite)
{
  SECURITY_ATTRIBUTES sa;

  sa.nLength = sizeof(sa);          /* Length in bytes */
  sa.bInheritHandle = 1;            /* the child must inherit these handles */
  sa.lpSecurityDescriptor = NULL;

  if (! CreatePipe (&readwrite[0],&readwrite[1],&sa,1 << 13))
  {
    errno = -1; /* EMFILE; que? */
    return -1;
  }

  return 0;
}

/*------------------------------------------------------------------------------
  Replacement for 'popen()' under WIN32.
  NOTE: if cmd contains '2>&1', we connect the standard error file handle
    to the standard output file handle.
------------------------------------------------------------------------------*/
FILE *
pt_popen(const char *cmd, const char *mode)
{
  FILE *fptr = (FILE *)0;
  PROCESS_INFORMATION piProcInfo;
  STARTUPINFO siStartInfo;
  int success, redirect_error = 0;
  char cmd_buff[2048];
  char *err2out;

  const char *shell_cmd = getenv("COMSPEC");
  if (! shell_cmd) shell_cmd = "cmd";
  strcpy(cmd_buff,shell_cmd);
  strcat(cmd_buff," /c ");
  strcat(cmd_buff,cmd);

  my_pipein[0]   = INVALID_HANDLE_VALUE;
  my_pipein[1]   = INVALID_HANDLE_VALUE;
  my_pipeout[0]  = INVALID_HANDLE_VALUE;
  my_pipeout[1]  = INVALID_HANDLE_VALUE;
  my_pipeerr[0]  = INVALID_HANDLE_VALUE;
  my_pipeerr[1]  = INVALID_HANDLE_VALUE;

  if (!mode || !*mode)
    goto finito;

  my_popenmode = *mode;
  if (my_popenmode != 'r' && my_popenmode != 'w')
    goto finito;

  /*
   * Shall we redirect stderr to stdout ? */
  if ((err2out = strstr("2>&1",cmd)) != NULL) {
     /* this option doesn't apply to win32 shells, so we clear it out! */
     strncpy(err2out,"    ",4);
     redirect_error = 1;
  }

  /*
   * Create the Pipes... */
  if (my_pipe(my_pipein)  == -1 ||
      my_pipe(my_pipeout) == -1)
    goto finito;
  if (!redirect_error && my_pipe(my_pipeerr) == -1)
    goto finito;

  /*
   * Now create the child process */
  ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
  siStartInfo.cb           = sizeof(STARTUPINFO);
  siStartInfo.hStdInput    = my_pipein[0];
  siStartInfo.hStdOutput   = my_pipeout[1];
  if (redirect_error)
    siStartInfo.hStdError  = my_pipeout[1];
  else
    siStartInfo.hStdError  = my_pipeerr[1];
  siStartInfo.dwFlags    = STARTF_USESTDHANDLES;

  success = CreateProcess(NULL,
     (LPTSTR)cmd_buff,  // command line 
     NULL,              // process security attributes 
     NULL,              // primary thread security attributes 
     TRUE,              // handles are inherited 
     DETACHED_PROCESS,  // creation flags: without window (?)
     NULL,              // use parent's environment 
     NULL,              // use parent's current directory 
     &siStartInfo,      // STARTUPINFO pointer 
     &piProcInfo);      // receives PROCESS_INFORMATION 

  if (!success)
    goto finito;

  /*
   * These handles listen to the Child process */
  CloseHandle(my_pipein[0]);  my_pipein[0]  = INVALID_HANDLE_VALUE;
  CloseHandle(my_pipeout[1]); my_pipeout[1] = INVALID_HANDLE_VALUE;
  CloseHandle(my_pipeerr[1]); my_pipeerr[1] = INVALID_HANDLE_VALUE;

  if (my_popenmode == 'r')
    fptr = _fdopen(_open_osfhandle((long)my_pipeout[0],_O_BINARY),"r");
  else
    fptr = _fdopen(_open_osfhandle((long)my_pipein[1],_O_BINARY),"w");

finito:
  if (!fptr)
  {
    if (my_pipein[0]  != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipein[0]);
    if (my_pipein[1]  != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipein[1]);
    if (my_pipeout[0] != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipeout[0]);
    if (my_pipeout[1] != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipeout[1]);
    if (my_pipeerr[0] != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipeerr[0]);
    if (my_pipeerr[1] != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipeerr[1]);
  }
  return fptr;

}

/*------------------------------------------------------------------------------
  Replacement for 'pclose()' under WIN32
------------------------------------------------------------------------------*/
int
pt_pclose(FILE *fle)
{
  if (fle)
  {
    (void)fclose(fle);

    CloseHandle(my_pipeerr[0]);
    if (my_popenmode == 'r')
      CloseHandle(my_pipein[1]);
    else
     CloseHandle(my_pipeout[0]);
    return 0;
  }
  return -1;
}


// copies from Lua source

static FILE **newfile (lua_State *L) {
  FILE **pf = (FILE **)lua_newuserdata(L, sizeof(FILE *));
  *pf = NULL;  /* file handle is currently `closed' */
  luaL_getmetatable(L, LUA_FILEHANDLE);
  lua_setmetatable(L, -2);
  return pf;
}

static int pushresult (lua_State *L, int i, const char *filename) {
  int en = errno;  /* calls to Lua API may change this value */
  if (i) {
    lua_pushboolean(L, 1);
    return 1;
  }
  else {
    lua_pushnil(L);
    if (filename)
      lua_pushfstring(L, "%s: %s", filename, strerror(en));
    else
      lua_pushfstring(L, "%s", strerror(en));
    lua_pushinteger(L, en);
    return 3;
  }
}

int win_io_popen (lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  const char *mode = luaL_optstring(L, 2, "r");
  FILE **pf = newfile(L);
  *pf = pt_popen(filename, mode);
  return (*pf == NULL) ? pushresult(L, 0, filename) : 1;
}

#define topfile(L)	((FILE **)luaL_checkudata(L, 1, LUA_FILEHANDLE))

int win_io_pclose (lua_State *L) {
  FILE **p = topfile(L);
  int ok = pt_pclose(*p) != -1;
  *p = NULL;
  return pushresult(L, ok, NULL);
}

#endif // POPEN_STUFF_WHICH_DOESNT_WORK

