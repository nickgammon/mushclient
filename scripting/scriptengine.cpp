#include "stdafx.h"
#include "..\mainfrm.h"
#include "..\MUSHclient.h"
#include "..\doc.h"
#include "..\dialogs\ScriptErrorDlg.h"

static CString strProcedure;
static CString strType;
static CString strReason;
static bool bImmediate = true;

// returns true if error
bool CScriptEngine::Execute (DISPID & dispid,  // dispatch ID, will be set to DISPID_UNKNOWN on an error
                              LPCTSTR szProcedure,  // eg. ON_TRIGGER_XYZ
                              const unsigned short iReason,  // value for m_iCurrentActionSource
                              LPCTSTR szType,   // eg. trigger, alias
                              LPCTSTR szReason, // eg. trigger subroutine XXX
                              DISPPARAMS & params,  // parameters
                              long & nInvocationCount,  // count of invocations
                              COleVariant * result    // result of call
                              )
  {


  // If Lua, we may have been called with no arguments, so just do that
  if (L)
    {
    list<double> nparams;
    list<string> sparams;
    bool r;
    bool status = ExecuteLua (dispid, 
                             szProcedure, 
                             iReason,
                             szType, 
                             szReason, 
                             nparams,
                             sparams, 
                             nInvocationCount,
                             NULL, NULL, NULL, &r);


    if (result)
      {
      result->vt = VT_BOOL;
      result->boolVal = r; 
      }
    return status;
    } // have Lua 

  // don't do it if no routine address 
  if (dispid == DISPID_UNKNOWN)
    return false;

  strProcedure = szProcedure;
  strType = szType;
  strReason = szReason;
  bImmediate = false;

  unsigned short iOldStyle = m_pDoc->m_iNoteStyle;
  m_pDoc->m_iNoteStyle = NORMAL;    // back to default style

  HRESULT hr;

  EXCEPINFO ExcepInfo;
  unsigned int ArgErr;
  LARGE_INTEGER start, 
                finish;
  SCRIPTSTATE ss;

  m_pDoc->Trace (TFormat ("Executing %s script \"%s\"", szType, szProcedure));
//  Frame.SetStatusMessageNow (TFormat ("Executing %s subroutine \"%s\"", szType, szProcedure));

  if (m_IActiveScript)
    {
    // new for Python - an error may have caused the script state to change
    hr = m_IActiveScript->GetScriptState (&ss);
  
    if (hr == S_OK)
      {
      // try to put it back to connected
      if (ss != SCRIPTSTATE_CONNECTED)
        hr = m_IActiveScript->SetScriptState (SCRIPTSTATE_CONNECTED);
      }

    if (hr != S_OK)
      {
      ::UMessageBox (TFormat ("Script engine problem invoking subroutine \"%s\" when %s",
                               (LPCTSTR) szProcedure,
                               (LPCTSTR) szReason));
      strProcedure.Empty ();
      strType.Empty ();
      strReason.Empty ();
      bImmediate = true;

      m_pDoc->m_iNoteStyle = iOldStyle;
      return true;
      }
    } // end of having script engine

  if (App.m_iCounterFrequency)
    QueryPerformanceCounter (&start);
  else
    {
    start.QuadPart = 0;
    finish.QuadPart = 0;
    }

  if (iReason != eDontChangeAction)
    m_pDoc->m_iCurrentActionSource = iReason;

  hr = m_pDispatch->Invoke (dispid, IID_NULL, 0, 
                            DISPATCH_METHOD, &params, result, &ExcepInfo, &ArgErr);

  if (iReason != eDontChangeAction)
    m_pDoc->m_iCurrentActionSource = eUnknownActionSource;

  if (hr == S_OK && App.m_iCounterFrequency)
    {
    QueryPerformanceCounter (&finish);
    m_pDoc->m_iScriptTimeTaken += finish.QuadPart - start.QuadPart;
    }

  // put status line back
//  ShowStatusLine ();


  if (hr == S_OK)
    nInvocationCount++;   // count number of times used
  else
    {
    dispid = DISPID_UNKNOWN;   // stop further invocations

    if (hr == 0x800a01c2)   // wrong number of arguments
      ::UMessageBox (TFormat ("Wrong number of arguments for script subroutine \"%s\" when %s"
                                "\n\nWe expected your subroutine to have %i argument%s",
                               (LPCTSTR) szProcedure,
                               (LPCTSTR) szReason,
                               PLURAL (params.cArgs)));
    else
      ::UMessageBox (TFormat ("Unable to invoke script subroutine \"%s\" when %s",
                               (LPCTSTR) szProcedure,
                               (LPCTSTR) szReason));
    }   // end of bad invoke


  strProcedure.Empty ();
  strType.Empty ();
  strReason.Empty ();
  bImmediate = true;

  m_pDoc->m_iNoteStyle = iOldStyle;

  return hr != S_OK;    // true on error

  } // end of CScriptEngine::ExecuteScript
 


STDMETHODIMP CActiveScriptSite::OnScriptError(IActiveScriptError *pscripterror) 
  {
  DWORD dwCookie;
  LONG nChar;
  ULONG nLine;
  BSTR bstr = 0;
  EXCEPINFO ei; 
  ZeroMemory(&ei, sizeof(ei));

  TRACE ("CActiveScriptSite: OnScriptError\n");
  
  pscripterror->GetSourcePosition(&dwCookie, &nLine, &nChar);
  pscripterror->GetSourceLineText(&bstr);
  pscripterror->GetExceptionInfo(&ei);
    
  CScriptErrorDlg dlg;

  if (ei.wCode)
    dlg.m_iError = ei.wCode;
  else
    dlg.m_iError = ei.scode;

  nLine = nLine + 1;  // make 1-relative to be consistent with lua

  dlg.m_strEvent = TFormat ("Execution of line %i column %i",
                                  nLine, nChar + 1);

  dlg.m_strDescription = ei.bstrDescription;

  if (!strProcedure.IsEmpty ())
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
    dlg.m_strCalledBy = Translate ("Immediate execution");

    dlg.m_strDescription += ENDLINE;
    if (bstr)
      {
      dlg.m_strDescription += Translate ("Line in error: ");
      dlg.m_strDescription += ENDLINE;
      dlg.m_strDescription += bstr;
      }
    }

  dlg.m_strRaisedBy = Translate ("No active world");

  if (m_pDoc)
    {
    if (m_pDoc->m_CurrentPlugin)
      {
      dlg.m_strRaisedBy = TFormat ("Plugin: %s (called from world: %s)",
                          (LPCTSTR) m_pDoc->m_CurrentPlugin->m_strName,
                          (LPCTSTR) m_pDoc->m_mush_name);
      }
    else 
      dlg.m_strRaisedBy = TFormat ("World: %s", (LPCTSTR) m_pDoc->m_mush_name);
    }

  if (!m_pDoc || !m_pDoc->m_bScriptErrorsToOutputWindow)
    {
    if (m_pDoc)
      dlg.m_bHaveDoc = true;
    dlg.DoModal ();
    if (m_pDoc && dlg.m_bUseOutputWindow)
      {
      m_pDoc->m_bScriptErrorsToOutputWindow = true;
      m_pDoc->SetModifiedFlag (TRUE);
      }
    }
  else
    {
    m_pDoc->ColourNote (SCRIPTERRORFORECOLOUR, SCRIPTERRORBACKCOLOUR, Translate ("Script error"));
    m_pDoc->ColourNote (SCRIPTERRORFORECOLOUR, SCRIPTERRORBACKCOLOUR, dlg.m_strRaisedBy);
    m_pDoc->ColourNote (SCRIPTERRORFORECOLOUR, SCRIPTERRORBACKCOLOUR, dlg.m_strEvent);
    m_pDoc->ColourNote (SCRIPTERRORFORECOLOUR, SCRIPTERRORBACKCOLOUR, dlg.m_strCalledBy);
    m_pDoc->ColourNote (SCRIPTERRORFORECOLOUR, SCRIPTERRORBACKCOLOUR, dlg.m_strDescription);

// show bad lines?

    if (!bImmediate)
      m_pDoc->ShowErrorLines (nLine);

   
    }  // end of showing in output window
  
  SysFreeString(bstr);
  SysFreeString(ei.bstrSource);
  SysFreeString(ei.bstrDescription);
  SysFreeString(ei.bstrHelpFile);
  
  return S_OK;
  }   // end of CActiveScriptSite::OnScriptError

bool CScriptEngine::CreateScriptEngine (void)
  {
  
 // Lua does not use scripting engine
  if (m_strLanguage.CompareNoCase ("Lua") == 0)
    {
    OpenLua ();
    return false;
    }  // end of Lua

  // not under Wine
 if (bWine)
    return true;

 try
  {


  CLSID clsid;
  OLECHAR wszOutput[101];

  /*

  Note: very, very important!

  To use swprintf successfully, you must specify %S for converting a single-byte
  string to a wide string, not %s. Alternatively, you can specify %hs which means
  the same thing. See reference to wprintf for details.

  If you don't, be prepared for access violations as the swprintf goes mad
  and writes all over memory, depending on whether there are two consecutive
  nulls in the string being printed.

  */

//  CString strFixedLanguage = m_strLanguage;
//
//  strFixedLanguage.MakeLower ();
//
//  if (strFixedLanguage == "rubyscript")
//    swprintf(wszOutput, OLESTR("%S"), "GlobalRubyScript");
//  else
  swprintf(wszOutput, OLESTR("%S"), (LPCTSTR) m_strLanguage.Left (100));
  
  CString strMsg;


  strMsg.Format ("finding CLSID of scripting language \"%s\"", (LPCTSTR) m_strLanguage);

  // javascript: {f414c260-6ac0-11cf-b6d1-00aa00bbbb58}
  // jscript:    {f414c260-6ac0-11cf-b6d1-00aa00bbbb58}   ???
  // python:     {DF630910-1C1D-11d0-AE36-8C0F5E000000}

  if (ShowError (CLSIDFromProgID(wszOutput, &clsid), 
    strMsg))
    return true;
  
  // create an instance of the VBscript/perlscript/jscript engine
    
  if (ShowError (::CoCreateInstance(clsid,  
    NULL, 
    CLSCTX_ALL,   // CLSCTX_INPROC_SERVER, 
    IID_IActiveScript,
    reinterpret_cast<void**>(&m_IActiveScript)),
    "loading scripting engine"))
    return true;

  if (ShowError (m_IActiveScript->QueryInterface(
    IID_IActiveScriptParse, 
    reinterpret_cast<void**>(&m_IActiveScriptParse)),
    "locating parse interface"))
    return true; 

  if (ShowError (m_IActiveScriptParse->InitNew (),
    "initialising scripting engine"))
    return true;

  // create host site object

  m_site = new CActiveScriptSite (m_pDoc->GetIDispatch (TRUE), m_pDoc);

  m_site->AddRef ();
  
  if (ShowError (m_IActiveScript->SetScriptSite (m_site),
    "setting site"))
    return true;

// add world object to engine's namespace
// added SCRIPTITEM_GLOBALMEMBERS in 3.27

WCHAR charWorld[]=L"world";

  if (ShowError (m_IActiveScript->AddNamedItem (charWorld,
    SCRIPTITEM_ISVISIBLE | SCRIPTITEM_ISSOURCE | SCRIPTITEM_GLOBALMEMBERS),
    "adding world to script engine"))
    return true;

//  if (ShowError (m_IActiveScript->AddNamedItem (L"world",
//    SCRIPTITEM_ISVISIBLE | SCRIPTITEM_ISSOURCE | SCRIPTITEM_GLOBALMEMBERS),
//    "adding world to script engine"))
//    return true;

// set state to started

  if (ShowError (m_IActiveScript->SetScriptState (SCRIPTSTATE_STARTED),
    "starting script engine"))
    return true;

// connect outbound objects (ie. world)

  if (ShowError (m_IActiveScript->SetScriptState (SCRIPTSTATE_CONNECTED),
    "connecting script engine"))
    return true;

// get script engine dispatch pointer

  if (ShowError (m_IActiveScript->GetScriptDispatch (0, &m_pDispatch),
    "getting script engine dispatch pointer"))
    return true;

  }   // end of try block

 catch (HRESULT hr)
   {
   ShowError (hr, "starting scripting support");
   DisableScripting ();
   return true;
   }

 catch (...)
   {
   ::TMessageBox ("Something nasty happened whilst initialising the scripting engine");
   throw;
   }

 return false;

  } // end of CScriptEngine::CreateScriptEngine

bool CScriptEngine::Parse (const CString & strCode, const CString & strWhat)
  {

  if (strWhat == "Script file" || strWhat == "Plugin")
    bImmediate = false;
  else
    bImmediate = true;

 // Do Lua differently
  if (L)
    return ParseLua (strCode, strWhat);

  if (!m_IActiveScriptParse || !m_IActiveScript)
    return true;   // no script engine

HRESULT hr;
SCRIPTSTATE ss;

  // new for Python - an error may have caused the script state to change
  hr = m_IActiveScript->GetScriptState (&ss);
  
  if (hr == S_OK)
    {
    // try to put it back to connected
    if (ss != SCRIPTSTATE_CONNECTED)
      hr = m_IActiveScript->SetScriptState (SCRIPTSTATE_CONNECTED);
    }

  if (hr != S_OK)
    {
    ::TMessageBox ("Script engine problem on script parse");
    return true;
    }

  BSTR bstrCode;
  EXCEPINFO ei; 

  bstrCode = strCode.AllocSysString ();
  ZeroMemory(&ei, sizeof(ei));

  if (!bstrCode)
    return true;  // error

  LARGE_INTEGER start, 
                finish;

  if (App.m_iCounterFrequency)
    QueryPerformanceCounter (&start);
  else
    {
    start.QuadPart = 0;
    finish.QuadPart = 0;
    }

  hr = m_IActiveScriptParse->ParseScriptText
                               (bstrCode, 0, 0, 0, 0, 0, 
                               SCRIPTTEXT_ISPERSISTENT |
                                SCRIPTTEXT_ISVISIBLE,
                               NULL, 
                               &ei);

  if (hr == S_OK && 
      App.m_iCounterFrequency)
    {
    QueryPerformanceCounter (&finish);
    m_pDoc->m_iScriptTimeTaken += finish.QuadPart - start.QuadPart;
    }

 ::SysFreeString (bstrCode);

  return hr != S_OK;    // true = error

  } // end of CScriptEngine::ParseScript 


DISPID CScriptEngine::GetDispid (const CString & strName)
    {

 // Do Lua differently
  if (L)
    return GetLuaDispid (strName);

  if (!m_pDispatch)
    return DISPID_UNKNOWN;   // no script engine

BSTR bstrProcedure;
HRESULT hr;
DISPID dispid;

  bstrProcedure = strName.AllocSysString ();

// LCID en_us = MAKELCID (MAKELANGID (LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

  hr = m_pDispatch->GetIDsOfNames (IID_NULL, &bstrProcedure, 1, LOCALE_SYSTEM_DEFAULT, &dispid);    // was LCID of 9
  
  ::SysFreeString (bstrProcedure);

  if (hr != S_OK)
     return DISPID_UNKNOWN;

  return dispid;     // might be zero, the way PHP currently is :)

  } // end of CScriptEngine::GetDispid



bool CScriptEngine::ShowError (const HRESULT hr, const CString strMsg)
  {
DWORD status = 0;
char *formattedmsg;
CString str;

  if (hr == S_OK)
    return false;

  const char * sForeColour = "darkorange";
  const char * sBackColour = "black";

  CScriptErrorDlg dlg;
  dlg.m_iError = hr;
  dlg.m_strEvent = strMsg;
  
  if (!FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, 
                    hr, 
                    LANG_NEUTRAL, 
                    (LPTSTR) &formattedmsg, 
                    0, 
                    NULL))
  {
   status = GetLastError ();
   dlg.m_strDescription.Format ("<<Unable to convert error number %ld>>", hr);
  }
 else
  {
   dlg.m_strDescription.Format ("Error %ld occurred when %s:\n\n%s", 
                hr,
                (LPCTSTR) strMsg,
                formattedmsg);
   LocalFree (formattedmsg);
  }

  DisableScripting ();

  dlg.m_strRaisedBy = "No active world";

  if (m_pDoc)
    {
    if (m_pDoc->m_CurrentPlugin)
      {
      dlg.m_strRaisedBy = "Plugin: " +  m_pDoc->m_CurrentPlugin->m_strName;
      dlg.m_strRaisedBy += " (called from world: " + m_pDoc->m_mush_name + ")";
      }
    else 
      dlg.m_strRaisedBy = "World: " + m_pDoc->m_mush_name;
    }

  if (!m_pDoc || !m_pDoc->m_bScriptErrorsToOutputWindow)
    {
    if (m_pDoc)
      dlg.m_bHaveDoc = true;
    dlg.DoModal ();
    if (m_pDoc && dlg.m_bUseOutputWindow)
      {
      m_pDoc->m_bScriptErrorsToOutputWindow = true;
      m_pDoc->SetModifiedFlag (TRUE);
      }
    }
  else
    {
    m_pDoc->ColourNote (sForeColour, sBackColour, strMsg);
    m_pDoc->ColourNote (sForeColour, sBackColour, dlg.m_strRaisedBy);
//    m_pDoc->ColourNote (sForeColour, sBackColour, dlg.m_strCalledBy);
    m_pDoc->ColourNote (sForeColour, sBackColour, dlg.m_strDescription);
    }

  return true;

  } // end of CScriptEngine::ShowError

void CScriptEngine::DisableScripting (void)
  {

  // Do Lua differently
  if (L)
    {
    CloseLua ();
    return;
    }

  // release engine

  if (m_IActiveScript)
    {
    m_IActiveScript->SetScriptState(SCRIPTSTATE_DISCONNECTED);
    m_IActiveScript->Close ();
    m_IActiveScript->Release ();
    m_IActiveScript = NULL;
    }

  // release parser

  if (m_IActiveScriptParse)
    {
    m_IActiveScriptParse->Release ();
    m_IActiveScriptParse = NULL;
    }

  // release site

  if (m_site)
    {
    m_site->Release ();
    m_site = NULL;
    }

  } // end of CScriptEngine::DisableScripting 

