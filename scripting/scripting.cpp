#include "stdafx.h"
#include "..\mainfrm.h"
#include "..\MUSHclient.h"
#include <process.h>    

#include "..\doc.h"
#include "..\dialogs\ImmediateDlg.h"
#include "..\dialogs\world_prefs\MissingEntryPoints.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
//#define new DEBUG_NEW
#endif

// all scripting-related stuff goes here


bool CMUSHclientDoc::CreateScriptEngine() 
{

// if (bWine)     // allow for Lua
//  return true;

if (m_ScriptEngine)
  {
  ::TMessageBox ("Error, scripting already active");
  return false;
  }

  m_bSyntaxErrorOnly = false;

  m_ScriptEngine = new CScriptEngine (this, m_strLanguage);

  if (m_ScriptEngine->CreateScriptEngine ())
    {
    delete m_ScriptEngine;
    m_ScriptEngine = NULL;
    return true;
    }

 // parse the script file

 if (m_strScriptFilename.IsEmpty ())
   return false;


 try
   {

   // open script file
    CFile fileScript (m_strScriptFilename, CFile::modeRead);
   
    char * p = m_strScript.GetBuffer (fileScript.GetLength ());

    // read in the script
    fileScript.Read (p, fileScript.GetLength ());

    // m_strScript now contains the entire script file
    m_strScript.ReleaseBuffer (fileScript.GetLength ());

    // close the file in case they try to modify it
    fileScript.Close ();

    // find when the file was last modified

	  CFileStatus	status;
	  CFile::GetStatus(m_strScriptFilename, status);
    m_timeScriptFileMod = status.m_mtime;

    // monitor the file so we can alert them if its contents change
    CreateMonitoringThread ();

    if (m_ScriptEngine->Parse (m_strScript, "Script file"))
      {
      m_bSyntaxErrorOnly = true;
      DisableScripting ();
      return true;
      }   // end of failure during parsing

   }  // end of try
 catch (CException * e)
   {
    e->ReportError ();
    e->Delete ();
    DisableScripting ();
    return true;  
   }

 

 // resolve all entry points

  // resolve all entry points for triggers, aliases etc.

  FindAllEntryPoints ();

  return false;   // good return

  }  //  end of CMUSHclientDoc::CreateScriptEngine

void CMUSHclientDoc::FindGlobalEntryPoints (CString & strErrorMessage)
  {
 // find entry point for world open, close, connect, disconnect

  m_dispidWorldOpen = GetProcedureDispidHelper (m_strWorldOpen,
                                           "world open", 
                                           "",
                                           strErrorMessage);
  m_dispidWorldClose = GetProcedureDispidHelper (m_strWorldClose,
                                           "world close", 
                                           "",
                                           strErrorMessage);
  m_dispidWorldSave = GetProcedureDispidHelper (m_strWorldSave,
                                           "world save", 
                                           "",
                                           strErrorMessage);
  m_dispidWorldConnect = GetProcedureDispidHelper (m_strWorldConnect,
                                           "world connect", 
                                           "",
                                           strErrorMessage);
  m_dispidWorldDisconnect = GetProcedureDispidHelper (m_strWorldDisconnect,
                                           "world disconnect", 
                                           "",
                                           strErrorMessage);
  m_dispidWorldGetFocus = GetProcedureDispidHelper (m_strWorldGetFocus,
                                           "world get focus", 
                                           "",
                                           strErrorMessage);
  m_dispidWorldLoseFocus = GetProcedureDispidHelper (m_strWorldLoseFocus,
                                           "world lose focus", 
                                           "",
                                           strErrorMessage);
  // MXP ones

  m_dispidOnMXP_Start = GetProcedureDispidHelper (m_strOnMXP_Start,
                                           "MXP startup", 
                                           "",
                                           strErrorMessage);
  m_dispidOnMXP_Stop = GetProcedureDispidHelper (m_strOnMXP_Stop,
                                           "MXP shutdown", 
                                           "",
                                           strErrorMessage);
  m_dispidOnMXP_OpenTag = GetProcedureDispid (m_strOnMXP_OpenTag,
                                           "MXP opening tag", 
                                           "",
                                           strErrorMessage);
  m_dispidOnMXP_CloseTag = GetProcedureDispid (m_strOnMXP_CloseTag,
                                           "MXP closing tag", 
                                           "",
                                           strErrorMessage);
  m_dispidOnMXP_SetVariable = GetProcedureDispid (m_strOnMXP_SetVariable,
                                           "MXP set variable", 
                                           "",
                                           strErrorMessage);
  m_dispidOnMXP_Error = GetProcedureDispid (m_strOnMXP_Error,
                                           "MXP error", 
                                           "",
                                           strErrorMessage);
  } // end of FindGlobalEntryPoints


bool CMUSHclientDoc::FindAllEntryPoints (void)
  {

  CString strErrorMessage;
  POSITION pos;

  if (!m_CurrentPlugin)
    FindGlobalEntryPoints (strErrorMessage);

 // find entry points for triggers

  CTrigger * pTrigger;
  CString strTriggerName;

  for (pos = GetTriggerMap ().GetStartPosition (); pos; )
    {
    GetTriggerMap ().GetNextAssoc (pos, strTriggerName, pTrigger);
    pTrigger->dispid = GetProcedureDispid (pTrigger->strProcedure,
                                           "trigger", 
                                           pTrigger->strLabel,
                                           strErrorMessage);
    }

 // find entry points for Aliases

  CAlias * pAlias;
  CString strAliasName;

  for (pos = GetAliasMap ().GetStartPosition (); pos; )
    {
    GetAliasMap ().GetNextAssoc (pos, strAliasName, pAlias);
    pAlias->dispid = GetProcedureDispid (pAlias->strProcedure,
                                           "alias",
                                           pAlias->strLabel,
                                           strErrorMessage);
    }

 // find entry points for Timers

  CTimer * pTimer;
  CString strTimerName;

  for (pos = GetTimerMap ().GetStartPosition (); pos; )
    {
    GetTimerMap ().GetNextAssoc (pos, strTimerName, pTimer);
    pTimer->dispid = GetProcedureDispid (pTimer->strProcedure,
                                           "timer", 
                                           pTimer->strLabel,
                                           strErrorMessage);
    }

  // no errors, don't display an error dialog box

  if (strErrorMessage.IsEmpty ())
    return false;

  // show the errors

  CMissingEntryPoints dlg;

  dlg.m_strErrorMessage = strErrorMessage;

  dlg.DoModal ();

  return true;
  }  // end of CMUSHclientDoc::FindAllEntryPoints



void CMUSHclientDoc::OnGameImmediate() 
{
CImmediateDlg dlg;

dlg.m_pDoc = this;

// preload with their last expression

dlg.m_strExpression = m_strLastImmediateExpression;

dlg.DoModal ();

// put last expression back, unless they blanked it out

if (!dlg.m_strExpression.IsEmpty ())
  m_strLastImmediateExpression = dlg.m_strExpression;

}

void CMUSHclientDoc::OnUpdateGameImmediate(CCmdUI* pCmdUI) 
{
DoFixMenus (pCmdUI);  // remove accelerators from menus

pCmdUI->Enable (m_ScriptEngine != NULL);

}


// helper function to load a type description from the typelib that
// is bound as a custom resource in this executable
HRESULT LoadTypeInfoFromThisModule(REFIID riid, ITypeInfo **ppti) {
    *ppti = 0;
    char szFileName[MAX_PATH];
    GetModuleFileNameA(0, szFileName, MAX_PATH);
    OLECHAR wszFileName[MAX_PATH];
    mbstowcs(wszFileName, szFileName, MAX_PATH);
    ITypeLib *ptl = 0;
    HRESULT hr = LoadTypeLib(wszFileName, &ptl);
    if (SUCCEEDED(hr)) {
        hr = ptl->GetTypeInfoOfGuid(riid, ppti);
        ptl->Release();
    }
    return hr;
}

// ------------------- disable scripting -------------------------

void CMUSHclientDoc::DisableScripting (void)
  {

  KillThread (m_pThread, m_eventScriptFileChanged);

// release engine

  delete m_ScriptEngine;
  m_ScriptEngine = NULL;

  }   // end of CMUSHclientDoc::DisableScripting

// ------------------- script file change monitoring thread -------------------------

void CMUSHclientDoc::ThreadFunc(LPVOID pParam)
{
  CThreadData*	pData = (CThreadData*) pParam;
	char * strDir = pData->m_strFilename;
  DWORD pDoc = pData->m_pDoc;
	char * p = strrchr (strDir, '\\');
	if (!p)
		p = strrchr (strDir, ':');   // why?
  if (p)
    *p = 0;
	HWND	hWnd = pData->m_hWnd;
	HANDLE	hEvent = pData->m_hEvent;

	delete pData;

  // Get a handle to a file change notification object.
  HANDLE	hChange = ::FindFirstChangeNotification(strDir, TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE);

  delete [] strDir;

  // Return now if ::FindFirstChangeNotification failed.
  if (hChange == INVALID_HANDLE_VALUE)
    return;

	HANDLE	aHandles[2];
	aHandles[0] = hChange;
	aHandles[1] = hEvent;
	BOOL	bContinue = TRUE;

    // Sleep until a file change notification wakes this thread or
    // m_eventScriptFileChanged becomes set indicating it's time for the thread to end.
    while (bContinue)
	{
		switch ((::WaitForMultipleObjects(2, aHandles, FALSE, INFINITE)))
		{
		case 0:
			// Respond to a change notification.
			::PostMessage(hWnd, WM_USER_SCRIPT_FILE_CONTENTS_CHANGED, (WPARAM) pDoc, 0);
			::FindNextChangeNotification(hChange);
			break;

		default:
			// Kill this thread (m_event became signaled).
            bContinue = FALSE;
			break;
		}
	}

	// Close the file change notification handle and return.
	::FindCloseChangeNotification(hChange);
	return;
}

// Create script source file monitoring thread
//
void CMUSHclientDoc::CreateMonitoringThread()
{
  KillThread (m_pThread, m_eventScriptFileChanged);

	CThreadData*	pData = new CThreadData;
	pData->m_strFilename = new char [m_strScriptFilename.GetLength () + 1];
  strcpy (pData->m_strFilename, m_strScriptFilename);
	pData->m_hWnd = Frame.GetSafeHwnd ();
	pData->m_hEvent = m_eventScriptFileChanged;
  pData->m_pDoc = (DWORD) this;
	m_eventScriptFileChanged.ResetEvent();

	m_pThread = (HANDLE) _beginthread (ThreadFunc, 0, pData);
  SetThreadPriority (m_pThread, THREAD_PRIORITY_IDLE);

	// Thread will delete data object
}


// ------------------- handle change to script file -------------------------

void CMUSHclientDoc::OnScriptFileChanged(const bool bForce)
{
  // exit if we are already asking them the question, or if scripting is not active

	if (m_bInScriptFileChanged)
		return;

  if (!m_ScriptEngine && !m_bSyntaxErrorOnly)
    return;

  // return if no script file
  if (m_strScriptFilename.IsEmpty ())
    return;

  // don't reload if they don't want it ever
  if (!bForce && m_nReloadOption == eReloadNever)
    return;

  m_bInScriptFileChanged = true;

	// Check if this script file has changed
	CFileStatus	status;
	CFile::GetStatus(m_strScriptFilename, status);
	if (m_timeScriptFileMod != status.m_mtime)
	  {

		CString	strText;
    strText = TFormat ("The script file \"%s\" has been modified. Do you wish to re-process it?",
      (LPCTSTR) m_strScriptFilename);
    if (bForce || m_nReloadOption == eReloadAlways ||
      ::UMessageBox (strText, MB_YESNO | MB_ICONQUESTION) == IDYES)
      {
      DisableScripting ();
      CreateScriptEngine ();
      } // end of approving modification or wanting it anyway
    } // end of time changing

	m_bInScriptFileChanged = false;
}


DISPID CMUSHclientDoc::GetProcedureDispid (const CString & strName, 
                                           const CString & strType,
                                           const CString & strLabel,
                                           CString & strErrorMessage) 
  {
  // ignore if no script name
  if (strName.IsEmpty () || (GetScriptEngine () == NULL))
    return DISPID_UNKNOWN;

  DISPID dispid = GetScriptEngine ()->GetDispid (strName);

  if (dispid != DISPID_UNKNOWN)
    return dispid;

  // append error message to supplied error message string

  CString strMsg;
  if (strLabel.IsEmpty ())
    strMsg = TFormat ("The %s subroutine named \"%s\" could not be found.",
      (LPCTSTR) strType,
      (LPCTSTR) strName);
  else
    strMsg = TFormat ("The %s (%s) subroutine named \"%s\" could not be found.",
      (LPCTSTR) strType,
      (LPCTSTR) strLabel,
      (LPCTSTR) strName);

  // put in error message preamble
  if (strErrorMessage.IsEmpty ())
    {
    if (m_strScriptFilename.IsEmpty ())
      strErrorMessage = Translate ("You have not specified a script file name:");
    else
      strErrorMessage = TFormat ("There was a problem in script file \"%s\":",
      (LPCTSTR) m_strScriptFilename);
    strErrorMessage += ENDLINE;
    }

  strErrorMessage += ENDLINE;

  strErrorMessage += strMsg;

  return DISPID_UNKNOWN;
  } // end of CMUSHclientDoc::GetProcedureDispid  

DISPID CMUSHclientDoc::GetProcedureDispidHelper (const CString & strName, 
                                                 const CString & strType,
                                                 const CString & strLabel,
                                                 CString & strErrorMessage) 
  {
  // !name will be treated as "send to execute"
  if (!strName.IsEmpty () && strName [0] == '!')
    return DISPID_UNKNOWN;

  // otherwise normal processing
  return GetProcedureDispid (strName, 
                             strType,
                             strLabel,
                             strErrorMessage);

  }

bool CMUSHclientDoc::ExecuteScript (DISPID & dispid,  // dispatch ID, will be zeroed on an error
                                    LPCTSTR szProcedure,  // eg. ON_TRIGGER_XYZ
                                    const unsigned short iReason,  // value for m_iCurrentActionSource
                                    LPCTSTR szType,   // eg. trigger, alias
                                    LPCTSTR szReason, // eg. trigger subroutine XXX
                                    DISPPARAMS & params,  // parameters
                                    long & nInvocationCount,  // count of invocations
                                    COleVariant * result    // result of call
                                    )
  {

  if (GetScriptEngine () && dispid != DISPID_UNKNOWN)
    {

    return GetScriptEngine ()->Execute (dispid, szProcedure, iReason, szType, szReason, 
                                                params, nInvocationCount, result);
    }
  else
    return false;

  } // end of CMUSHclientDoc::ExecuteScript 



void CMUSHclientDoc::ShowErrorLines (const int iLine)  // show script file around the error point
  {

string sScript;
vector<string> v;

  if (m_CurrentPlugin)
    sScript = m_CurrentPlugin->m_strScript;
  else 
    sScript = m_strScript;

  StringToVector (sScript, v, "\n", false);  // break script into lines so we can easily show each one

  // provided wanted line is in the table
  if (!sScript.empty () && v.size () >= iLine)
    {
    ColourNote (SCRIPTERRORFORECOLOUR, SCRIPTERRORBACKCOLOUR, Translate ("Error context in script:"));

    int iStart = iLine - 4;   // start 4 lines back
    if (iStart < 1) 
      iStart = 1;             // can't be lower than first line

    int iEnd = iLine + 4;     // end 4 lines onwards

    if (iEnd > v.size ())
      iEnd = v.size ();       // or at last line in script

    // show that range, marking error line with an asterisk
    for (int i = iStart; i <= iEnd; i++)
       ColourNote (SCRIPTERRORCONTEXTFORECOLOUR, SCRIPTERRORBACKCOLOUR, 
         CFormat ("%4i%s: %s", 
            i,   // line number
            i == iLine ? "*" : " ",   // mark current line
            (v [i - 1]).c_str ()));   // vector is zero-relative

    }  // end of line in range

  }

