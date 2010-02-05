#pragma once

#ifdef USE_LUA

extern "C"
  {
  #include "lua.h"
  #include "lualib.h"
  #include "lauxlib.h"
  }

#include "paneline.h"

#define DOCUMENT_STATE "mushclient.document"
#define WORLD_LIBRARY "world"

void LuaError (lua_State *L, 
               LPCTSTR strEvent = "Run-time error",
               LPCTSTR strProcedure = "",
               LPCTSTR strType = "",
               LPCTSTR strReason = "",
               CMUSHclientDoc * pDoc = NULL);

#endif // USE_LUA

class CScriptEngine : public CObject
  {

  public:
    CScriptEngine (CMUSHclientDoc * pDoc,
                   const CString strLanguage) // constructor
      {
      m_pDoc = pDoc;
      m_strLanguage = strLanguage;
      m_IActiveScript = NULL;
      m_IActiveScriptParse = NULL;
      m_site = NULL;
      m_pDispatch = NULL;
#ifdef USE_LUA
      L = NULL;
#endif // USE_LUA
      };  // end of constructor

    ~CScriptEngine () // destructor
      {
      DisableScripting ();
      }; // end of destuctor

  bool CreateScriptEngine (void);
  bool Parse (const CString & strCode, const CString & strWhat);

  DISPID GetDispid (const CString & strName);
  DISPID GetLuaDispid (const CString & strName);

  // returns true if script error
  bool Execute (DISPID & dispid,  // dispatch ID, will be set to DISPID_UNKNOWN on an error
                LPCTSTR szProcedure,  // eg. ON_TRIGGER_XYZ
                const unsigned short iReason,  // value for m_iCurrentActionSource
                LPCTSTR szType,   // eg. trigger, alias
                LPCTSTR szReason, // eg. trigger subroutine XXX
                DISPPARAMS & params,  // parameters
                long & nInvocationCount,  // count of invocations
                COleVariant * result    // result of call
                );
  bool ShowError (const HRESULT result, const CString strMsg);
  void DisableScripting (void);

#ifdef USE_LUA

  void OpenLua ();
  void OpenLuaDelayed ();
  void CloseLua ();

//  void RegisterLuaRoutines ();

  bool ParseLua (const CString & strCode, const CString & strWhat);
  // returns true if script error
  bool ExecuteLua (DISPID & dispid,          // dispatch ID, will be set to DISPID_UNKNOWN on an error
                   LPCTSTR szProcedure,      // eg. ON_TRIGGER_XYZ
                   const unsigned short iReason,  // value for m_iCurrentActionSource
                   LPCTSTR szType,           // eg. trigger, alias
                   LPCTSTR szReason,         // eg. trigger subroutine XXX
                   list<double> & nparams,   // list of number parameters
                   list<string> & sparams,   // list of string parameters
                   long & nInvocationCount,  // count of invocations
                   const t_regexp * regexp = NULL,  // regular expression (for triggers, aliases)
                   map<string, string> * table = NULL, // map of other things
                   CPaneLine * paneline = NULL,        // and the line (for triggers)
                   bool * result = NULL);              // where to put result

  // returns true if script error
  bool ExecuteLua (DISPID & dispid,          // dispatch ID, will be set to DISPID_UNKNOWN on an error
                   LPCTSTR szProcedure,      // eg. ON_TRIGGER_XYZ
                   const unsigned short iReason,  // value for m_iCurrentActionSource
                   LPCTSTR szType,           // eg. trigger, alias
                   LPCTSTR szReason,         // eg. trigger subroutine XXX
                   CString strParam,         // string parameter
                   long & nInvocationCount,  // count of invocations
                   CString & result);        // where to put result

  // return value is return from call

  const bool IsLua () const { return L != NULL; }
  lua_State           * L;                  // Lua state

#endif // USE_LUA

  private:

  IActiveScript       * m_IActiveScript;          // VBscript interface
  IActiveScriptParse  * m_IActiveScriptParse;     // parser
  CActiveScriptSite   * m_site;                   // our local site (world object)
  IDispatch           * m_pDispatch;              // script engine dispatch pointer

  CMUSHclientDoc      * m_pDoc;                   // related MUSHclient document

  CString               m_strLanguage;        // language, (vbscript, jscript, perlscript)

  };

int RegisterLuaRoutines (lua_State * L);
int DisableDLLs (lua_State * L);
#include "lua_helpers.h"
