// methods_utilities.cpp

// Various utility functions

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\..\mainfrm.h"
#include "..\errors.h"
#include "..\..\childfrm.h"
#include "..\..\sendvw.h"
#include "..\..\dialogs\CompleteWordDlg.h"
#include <direct.h>

// Implements:

//    Activate
//    Base64Decode
//    Base64Encode
//    Bookmark
//    ChangeDir
//    Connect
//    CreateGUID
//    DeleteGroup
//    Disconnect
//    EnableGroup
//    FixupEscapeSequences
//    FixupHTML
//    GenerateName
//    GetUniqueID
//    GetUniqueNumber
//    Hash
//    Help
//    MakeRegularExpression
//    Menu
//    MtRand
//    MtSrand
//    Open
//    OpenBrowser
//    ReadNamesFile
//    Replace
//    ResetIP
//    Save
//    SendPkt
//    SetChanged
//    SetNotes
//    SetStatus
//    Simulate
//    StripANSI
//    TranslateDebug
//    TranslateGerman
//    Trim



extern char working_dir [_MAX_PATH];
extern char file_browsing_dir [_MAX_PATH];
extern bool bNamesRead;
extern set<string> LuaFunctionsSet;


CString AnsiCode (short Code) 
{
return CFormat ("%c[%dm", ESC, Code); 
}   // end of AnsiCode

CString StripAnsi (const CString strMessage)
  {
CString strResult;

const char * p,
           * start;
char c;
long length;

  p = start = strMessage;
  while (c = *p)
    {
    if (c == ESC)
      {
      length = p - start;

      // output earlier block
      if (length > 0)
        strResult += CString (start, length);
      p++;    // skip the ESC

      if (*p == '[')
        {
        p++;    // skip the [

        while (*p != 0 && 
               *p != 'm')
          p++;

        if (*p)
          p++;   // end of getting code
        } // end of ESC [ something
       else
         p++; // skip it

      start = p;  // ready to start a new batch
      } // end of ESC something
    else
      p++;  // just keep counting characters

    } // end of processing each character

// get remaining text 
  strResult += start;
  return strResult;
  }   // end of StripAnsi


// strip ANSI codes from a message

BSTR CMUSHclientDoc::StripANSI(LPCTSTR Message) 
{
CString strResult = ::StripAnsi (Message);
	return strResult.AllocSysString();
}


void CMUSHclientDoc::Activate() 
{
  for(POSITION pos=GetFirstViewPosition();pos!=NULL;)
    {
    CView* pView = GetNextView(pos);

    if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
      {
      CSendView* pmyView = (CSendView*)pView;

      if (pmyView->GetParentFrame ()->IsIconic ())
        pmyView->GetParentFrame ()->ShowWindow (SW_RESTORE);

      pmyView->GetParentFrame ()->ActivateFrame ();
      pmyView->m_owner_frame->SetActiveView(pmyView);

      break;

      }	  // end of being a CSendView
    }

}  // end of CMUSHclientDoc::Activate


BSTR CMUSHclientDoc::Replace(LPCTSTR Source, LPCTSTR SearchFor, LPCTSTR ReplaceWith, BOOL Multiple) 
{
	CString strResult;
  CString strSource = Source;
  strResult = ::Replace (strSource, SearchFor, ReplaceWith, Multiple != 0);
	return strResult.AllocSysString();
}   // end of CMUSHclientDoc::Replace

BSTR CMUSHclientDoc::FixupEscapeSequences(LPCTSTR Source) 
{
	CString strResult;
  CString strSource = Source;
  strResult = ::FixupEscapeSequences (strSource);
	return strResult.AllocSysString();
}    // end of CMUSHclientDoc::FixupEscapeSequences

BSTR CMUSHclientDoc::Trim(LPCTSTR Source) 
{
  CString strSource = Source;
  strSource.TrimLeft ();
  strSource.TrimRight ();
  
	return strSource.AllocSysString();
}  // end of CMUSHclientDoc::Trim


long CMUSHclientDoc::GetUniqueNumber() 
{
  return (long) (App.GetUniqueNumber () & 0x7FFFFFFF);
}  // end of CMUSHclientDoc::GetUniqueNumber


BSTR CMUSHclientDoc::FixupHTML(LPCTSTR StringToConvert) 
{
	CString strResult = FixHTMLString (StringToConvert);
	return strResult.AllocSysString();
}   // end of CMUSHclientDoc::FixupHTML


VARIANT CMUSHclientDoc::GenerateName() 
{
	VARIANT vaResult;
	VariantInit(&vaResult);
  vaResult.vt = VT_NULL;

  // if no names file read, return NULL variant
  if (!bNamesRead)
    return vaResult;

  CString strName = ::GenerateName ();

  SetUpVariantString (vaResult, strName);

  return vaResult;

}  // end of CMUSHclientDoc::GenerateName

long CMUSHclientDoc::ReadNamesFile(LPCTSTR FileName) 
{
  if (strlen (FileName) == 0)
    return eNoNameSpecified;

	try
	  {
    ReadNames (FileName, true);
    }
	catch (CException* e)
	  {
		e->Delete();
    return eCouldNotOpenFile;
	  }

	return eOK;
}   // end of CMUSHclientDoc::ReadNamesFile



VARIANT CMUSHclientDoc::Base64Decode(LPCTSTR Text) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);
  CString strResult;

  try 
    {
    strResult = decodeBase64 (Text);
    }   // end of try

  catch(CException* e)
    {
    e->Delete ();
    vaResult.vt = VT_NULL;
  	return vaResult;
    } // end of catch

  SetUpVariantString (vaResult, strResult);

	return vaResult;
}   // end of CMUSHclientDoc::Base64Decode

VARIANT CMUSHclientDoc::Base64Encode(LPCTSTR Text, BOOL MultiLine) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);
  CString strResult;

  try 
    {
    strResult = encodeBase64 (Text, MultiLine);
    }   // end of try

  catch(CException* e)
    {
    e->Delete ();
    vaResult.vt = VT_NULL;
  	return vaResult;
    } // end of catch

  SetUpVariantString (vaResult, strResult);
	return vaResult;
}   // end of CMUSHclientDoc::Base64Encode



BSTR CMUSHclientDoc::CreateGUID() 
{
	CString strResult = GetGUID ();

	return strResult.AllocSysString();
}  // end of CMUSHclientDoc::CreateGUID


BSTR CMUSHclientDoc::Hash(LPCTSTR Text) 
{
	CString strResult;

  SHS_INFO shsInfo;
  shsInit   (&shsInfo);
  shsUpdate (&shsInfo, (unsigned char *) (const char *) Text, 
              strlen (Text));
  shsFinal  (&shsInfo);

  for (int i = 0; i < NUMITEMS (shsInfo.digest); i++)
    strResult += CFormat ("%08x", shsInfo.digest [i]);

	return strResult.AllocSysString();
}   // end of CMUSHclientDoc::Hash



BSTR CMUSHclientDoc::GetUniqueID() 
{
CString strResult = ::GetUniqueID ();

	return strResult.AllocSysString();
}  // end of CMUSHclientDoc::GetUniqueID


BSTR CMUSHclientDoc::TranslateGerman(LPCTSTR Text) 
{
	CString strResult = FixUpGerman (Text);

	return strResult.AllocSysString();
  }  // end of CMUSHclientDoc::TranslateGerman


BSTR CMUSHclientDoc::MakeRegularExpression(LPCTSTR Text) 
{
	return ConvertToRegularExpression (Text).AllocSysString();
}   // end of CMUSHclientDoc::MakeRegularExpression



void CMUSHclientDoc::MtSrand(long Seed) 
{
	init_genrand (Seed);
}   // end of CMUSHclientDoc::MtSrand

/* example Lua code:

  MtSrand (4357)

  print ("------")

  r = {}
  lim = 10
  for j = 1, 100000 do
    i = math.floor (MtRand () * lim)
    r [ i ] = (r [ i ] or 0) + 1 
  end

  tprint (r)

*/

double CMUSHclientDoc::MtRand() 
{
	return genrand ();
}   // end of CMUSHclientDoc::MtRand


long CMUSHclientDoc::ChangeDir(LPCTSTR Path) 
{
  if (_chdir(Path) == 0)
    {
    // find the new working directory

    _getdcwd (0, working_dir, sizeof (working_dir) - 1);

    // make sure directory name ends in a slash

    working_dir [sizeof (working_dir) - 2] = 0;

    if (working_dir [strlen (working_dir) - 1] != '\\')
      strcat (working_dir, "\\");
    
    return true;  // did it OK
    }

	return false;
}    // end of CMUSHclientDoc::ChangeDir


void CMUSHclientDoc::SetStatus(LPCTSTR Message) 
{
  m_strStatusMessage = Message;
  ShowStatusLine (true);    // show it now
}  // end of CMUSHclientDoc::SetStatus


LPDISPATCH CMUSHclientDoc::Open(LPCTSTR FileName) 
{

//CDocument * pDoc = App.m_pWorldDocTemplate->OpenDocumentFile (FileName);
CDocument * pDoc = App.OpenDocumentFile (FileName);

  if (pDoc)
      return pDoc->GetIDispatch (TRUE);

	return NULL;
}   // end of CMUSHclientDoc::Open


// enable or disable a batch of trigers/timers/aliases

long CMUSHclientDoc::EnableGroup(LPCTSTR GroupName, BOOL Enabled) 
{

  long iCount = 0;

  iCount += EnableTriggerGroup (GroupName, Enabled);
  iCount += EnableAliasGroup   (GroupName, Enabled);
  iCount += EnableTimerGroup   (GroupName, Enabled);

	return iCount;
}   // end of EnableGroup


long CMUSHclientDoc::DeleteGroup(LPCTSTR GroupName) 
{
  long iCount = 0;

  iCount += DeleteTriggerGroup (GroupName);
  iCount += DeleteAliasGroup   (GroupName);
  iCount += DeleteTimerGroup   (GroupName);

	return iCount;
}   // end of DeleteGroup



void CMUSHclientDoc::SetChanged(BOOL ChangedFlag) 
{
SetModifiedFlag (ChangedFlag);
}   // end of CMUSHclientDoc::SetChanged



// simulates input from the MUD, for debugging scripts

void CMUSHclientDoc::Simulate(LPCTSTR Text) 
{
   DisplayMsg(Text, strlen (Text), 0);
}   // end of CMUSHclientDoc::Simulate


// resets the IP address, forcing a name-lookup

void CMUSHclientDoc::ResetIP() 
{
  ZeroMemory (&m_sockAddr, sizeof m_sockAddr);
  ZeroMemory (&m_ProxyAddr, sizeof m_ProxyAddr);
}    // end of CMUSHclientDoc::ResetIP



long CMUSHclientDoc::OpenBrowser(LPCTSTR URL) 
{
CString strAction = URL;

    if (strAction.IsEmpty ())
      return eBadParameter;

    // don't let them slip in arbitrary OS commands
    if (strAction.Left (7).CompareNoCase ("http://") != 0 &&
        strAction.Left (8).CompareNoCase ("https://") != 0 &&
        strAction.Left (7).CompareNoCase ("mailto:") != 0)
      return eBadParameter;

    if ((long) ShellExecute (Frame, _T("open"), strAction, NULL, NULL, SW_SHOWNORMAL) <= 32)
      return eCouldNotOpenFile;

	return eOK;
}   // end of CMUSHclientDoc::OpenBrowser



long CMUSHclientDoc::SendPkt(LPCTSTR Packet) 
{

  if (m_iConnectPhase != eConnectConnectedToMud)
    return eWorldClosed;             

  SendPacket (Packet, strlen (Packet));
	return eOK;
}   // end of CMUSHclientDoc::SendPkt



long CMUSHclientDoc::TranslateDebug(LPCTSTR Message) 
{

  // no good without a script
	if (App.m_Translator_Lua == NULL)
    return 1;   // no script

  lua_settop (App.m_Translator_Lua, 0); // pop everything from last time

  // look for Debug function
  lua_getglobal (App.m_Translator_Lua, "Debug");
  if (!lua_isfunction (App.m_Translator_Lua, -1))
    return 2;   // no Debug function

  // argument
  lua_pushstring (App.m_Translator_Lua, Message);

  // call with 1 argument and no result
  if (lua_pcall (App.m_Translator_Lua, 1, 0, 0))
    return 3;   // run-time error

  // wow! finished ok
	return 0;   // ok exit
}   // end of CMUSHclientDoc::TranslateDebug


void CMUSHclientDoc::SetNotes(LPCTSTR Message) 
{
	
  m_notes = Message;
  SetModifiedFlag (TRUE);   // document has changed

}  // end of CMUSHclientDoc::SetNotes



// world.Connect - connects the current world

long CMUSHclientDoc::Connect() 
{

if (m_iConnectPhase == eConnectNotConnected)
  {
  OnConnectionConnect ();
  return eOK;
  }
else
  return eWorldOpen;      // cannot connect, already open
  
} // end of CMUSHclientDoc::Connect

// world.Disconnect - disconnects from the current world

long CMUSHclientDoc::Disconnect() 
{

if (m_iConnectPhase != eConnectNotConnected && 
    m_iConnectPhase != eConnectDisconnecting)
  {
  OnConnectionDisconnect ();
  return eOK;
  }
else
  return eWorldClosed;      // cannot disconnect, already closed

}  // end of CMUSHclientDoc::Disconnect

// world.Save - saves the current world - under a new name if supplied
//              returns zero if no error
BOOL CMUSHclientDoc::Save(LPCTSTR Name) 
{
CString strName = Name;

// if empty, take document default path name [#459]

  if (strName.IsEmpty ())
    strName = m_strPathName;

  return !DoSave (strName, TRUE);

}  // end of CMUSHclientDoc::Save


extern tInternalFunctionsTable InternalFunctionsTable [1];

void CMUSHclientDoc::Help(LPCTSTR Name) 
{
map<string, string> lua_specials;

// special Lua help
  lua_specials ["lua"]   = "lua";
  lua_specials ["lua b"] = "lua_base";
  lua_specials ["lua c"] = "lua_coroutines";
  lua_specials ["lua d"] = "lua_debug";
  lua_specials ["lua i"] = "lua_io";
  lua_specials ["lua m"] = "lua_math";
  lua_specials ["lua o"] = "lua_os";
  lua_specials ["lua p"] = "lua_package";
  lua_specials ["lua r"] = "lua_rex";
  lua_specials ["lua s"] = "lua_string";
  lua_specials ["lua t"] = "lua_tables";
  lua_specials ["lua u"] = "lua_utils";

CString m_strFilter = Name;

  m_strFilter.MakeLower ();
  m_strFilter.TrimLeft ();
  m_strFilter.TrimRight ();

  // a special case - b is used twice
  if (m_strFilter == "lua bc")
    {
    ShowHelp ("DOC_", "lua_bc"); 
    return;
    }

  CString strFunction;

  string sFirst5 = m_strFilter.Left (5);

  map<string, string>::const_iterator it = lua_specials.find (sFirst5);

  if (it != lua_specials.end ())
    {
    ShowHelp ("DOC_", it->second.c_str ()); 
    return;
    }
  else if (!m_strFilter.IsEmpty ())
    {
    // first find direct match on a Lua function
    if (LuaFunctionsSet.find ((LPCTSTR) m_strFilter) != LuaFunctionsSet.end ())
      {
      ShowHelp ("LUA_", m_strFilter);   
      return;
      }

    // then try a world function
    for (int i = 0; InternalFunctionsTable [i].sFunction [0]; i++)
      {
      strFunction = InternalFunctionsTable [i].sFunction;
      strFunction.MakeLower ();

      if (strFunction == m_strFilter)
        {
        ShowHelp ("FNC_", InternalFunctionsTable [i].sFunction);   // back to proper capitalization
        return;
        }
      }
    } // end of non-empty name

  bool bLua = false;
  
  if (GetScriptEngine () && GetScriptEngine ()->L)
    bLua = true;

  // not exact match, show list matching filter
 ShowFunctionslist (m_strFilter, bLua);
}   // end of CMUSHclientDoc::Help




BSTR CMUSHclientDoc::Menu(LPCTSTR Items, LPCTSTR Default) 
{
	CString strResult;
  CSendView* pmyView = NULL;


  for(POSITION pos=GetFirstViewPosition();pos!=NULL;)
    {
    CView* pView = GetNextView(pos);

    if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
      {
      pmyView = (CSendView*)pView;
      break;

      }	  // end of being a CSendView
    }

  if (!pmyView)
    return strResult.AllocSysString();

  CEdit * ctlEdit = & pmyView->GetEditCtrl();

  int nStartChar,
      nEndChar;

  // find the selection range
  ctlEdit->GetSel(nStartChar, nEndChar);

  if (nEndChar < 0)
    nEndChar = nStartChar;

  vector<string> v;

  StringToVector (Items, v, "|");

  int iCount = v.size ();

  // must have at least one item
  if (iCount < 1)
    return strResult.AllocSysString();

  CCompleteWordDlg dlg;
  
  set<string> extraItems;

  for (vector<string>::const_iterator i = v.begin (); i != v.end (); i++)
    extraItems.insert (*i);

  dlg.m_extraItems = &extraItems;
  dlg.m_strDefault = Default;
  dlg.m_bFunctions = false;
  dlg.m_pt = ctlEdit->PosFromChar (nEndChar - 1);  // strangely doesn't work at end of line

  ctlEdit->ClientToScreen(&dlg.m_pt);

  if (dlg.DoModal () == IDOK)
     strResult = dlg.m_strResult;

	return strResult.AllocSysString();
}   // end of CMUSHclientDoc::Menu
