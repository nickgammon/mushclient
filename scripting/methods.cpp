#include "stdafx.h"
#include "..\MUSHclient.h"

//=============================================================
// IMPORTANT - when adding methods here remember to add the 'glue'
//             routine in lua_methods.cpp and add it to the library table
//=============================================================
#include "..\doc.h"
#include "..\MUSHview.h"
#include "..\mainfrm.h"
#include "..\childfrm.h"
#include "..\sendvw.h"
#include "..\TextView.h"
#include "..\TextDocument.h"
#include "..\dialogs\ChooseNotepadDlg.h"
#include "..\dialogs\ColourPickerDlg.h"
#include "..\MakeWindowTransparent.h"

#include "errors.h"
#include "..\flags.h"
#include "..\mxp\mxp.h"
#include "..\color.h"
#include "..\winplace.h"
#include "..\UDPsocket.h"
#include <direct.h>
#include "..\dmetaph.h"

#include "..\png\png.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern bool bNamesRead;

extern tConfigurationNumericOption OptionsTable [];
extern tConfigurationAlphaOption AlphaOptionsTable [];
extern tCommandIDMapping CommandIDs [];
extern CString strMacroDescriptions [MACRO_COUNT];
extern CString strKeypadNames [eKeypad_Max_Items];

extern char working_dir [_MAX_PATH];
extern char file_browsing_dir [_MAX_PATH];

extern set<string> LuaFunctionsSet;


// ==============================================================================

// These are the methods exposed to external programs


// ******************************************************************************
// **************************** WORLDS ******************************************
// ******************************************************************************

// world.Send - sends the text to the world

long CMUSHclientDoc::Send(LPCTSTR Message) 
{
  if (m_iConnectPhase != eConnectConnectedToMud)
    return eWorldClosed;             

  // cannot change what we are sending in OnPluginSent
  if (m_bPluginProcessingSent)
    return eItemInUse;

  SendMsg (Message, m_display_my_input, false, false);  // don't queue it

	return eOK;

}

// send disregarding the queue (ie. queue jump)

long CMUSHclientDoc::SendImmediate(LPCTSTR Message) 
{
  if (m_iConnectPhase != eConnectConnectedToMud)
    return eWorldClosed;             

  // cannot change what we are sending in OnPluginSent
  if (m_bPluginProcessingSent)
    return eItemInUse;

  DoSendMsg (Message, m_display_my_input, m_log_input); 

	return eOK;
}

long CMUSHclientDoc::SendNoEcho(LPCTSTR Message) 
{
  if (m_iConnectPhase != eConnectConnectedToMud)
    return eWorldClosed;             

  // cannot change what we are sending in OnPluginSent
  if (m_bPluginProcessingSent)
    return eItemInUse;

  SendMsg (Message, false, false, false);  // don't queue it, don't echo it

	return eOK;
}


void CMUSHclientDoc::AddToCommandHistory (LPCTSTR Message)
  {

  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	  {
		  CSendView* pmyView = (CSendView*)pView;

      if (strlen (Message) > 0 && strcmp (Message, pmyView->m_last_command) != 0)
        {
        if (pmyView->m_inputcount >= m_nHistoryLines)
          {
          pmyView->m_msgList.RemoveHead ();   // keep max of "m_nHistoryLines" previous commands
          pmyView->m_HistoryFindInfo.m_nCurrentLine--;     // adjust for a "find again"
          if (pmyView->m_HistoryFindInfo.m_nCurrentLine < 0)
            pmyView->m_HistoryFindInfo.m_nCurrentLine = 0;
          }   // end of buffer full
        else
          pmyView->m_inputcount++;
        pmyView->m_msgList.AddTail (Message);
        pmyView->m_last_command = Message;
        } // end of different from last one

      break;
	    }	  // end of being a CSendView
    }   // end of loop through views

  } // end of CMUSHclientDoc::AddToCommandHistory

long CMUSHclientDoc::SendPush(LPCTSTR Message) 
{
  if (m_iConnectPhase != eConnectConnectedToMud)
    return eWorldClosed;             

  // cannot change what we are sending in OnPluginSent
  if (m_bPluginProcessingSent)
    return eItemInUse;

  SendMsg (Message, m_display_my_input, false, false);  // don't queue it

  AddToCommandHistory (Message);

	return eOK;
}   // end of SendPush

long CMUSHclientDoc::SendSpecial(LPCTSTR Message, BOOL Echo, BOOL Queue, BOOL Log, BOOL History) 
{
  if (m_iConnectPhase != eConnectConnectedToMud)
    return eWorldClosed;             

  // cannot change what we are sending in OnPluginSent
  if (m_bPluginProcessingSent)
    return eItemInUse;

  // send with appropriate options
  SendMsg (Message, Echo, Queue, Log);  

  if (History)
     AddToCommandHistory (Message);

	return eOK;

}


long CMUSHclientDoc::LogSend(LPCTSTR Message) 
{
long iResult = Send (Message);  // send it

  if (iResult == eOK)
    LogCommand (Message);   // and log it

	return iResult;
}

// world.GetLineCount - returns the count of lines received by this world

long CMUSHclientDoc::GetLineCount() 
{
	return m_total_lines;
}


// world.IsConnected - returns true if the world is open

BOOL CMUSHclientDoc::IsConnected() 
{
	return m_iConnectPhase == eConnectConnectedToMud;
}


// world.worldname - returns the name of the current world

BSTR CMUSHclientDoc::WorldName() 
{
	CString strResult;

  strResult = m_mush_name;

	return strResult.AllocSysString();
}


// world.version - returns MUSHclient version

BSTR CMUSHclientDoc::Version() 
{
	CString strResult = MUSHCLIENT_VERSION;

	return strResult.AllocSysString();
}


// world.note - makes a comment in the output buffer (with newline)

void CMUSHclientDoc::Note(LPCTSTR Message) 
{
  CString strMsg = Message;

  if (strMsg.Right (2) != ENDLINE)
    strMsg += ENDLINE;      // add a new line if necessary

  Tell (strMsg);
}

// world.tell - makes a comment in the output buffer (without newline)

void CMUSHclientDoc::Tell(LPCTSTR Message) 
{

  // don't muck around if empty message
  if (Message [0] == 0)
    return;

  // if output buffer doesn't exist yet, remember note for later
  if (m_pCurrentLine == NULL)
    {
    COLORREF fore = m_iNoteColourFore, 
             back = m_iNoteColourBack;

    // need to do this in case a normal Note follows a ColourNote ...

    // select correct colour, if needed, from custom pallette
    if (!m_bNotesInRGB)
      {
      if (m_iNoteTextColour >= 0 && m_iNoteTextColour < MAX_CUSTOM)
        {   
        fore = m_customtext [m_iNoteTextColour];
        back = m_customback [m_iNoteTextColour];
        }  // end of notes in custom colour
      }

    m_OutstandingLines.push_back (CPaneStyle (Message, fore, back, m_iNoteStyle));
    return;
    }

  // If current line is not a note line, force a line change (by displaying
  // an empty string), so that the style change is on the note line and not
  // the back of the previous line. This has various implications, including
  // the way that "bleed colour to edge" will work. Otherwise it bleeds the
  // forthcoming note colour to the edge of the previous (mud) line.

  if ((m_pCurrentLine->flags & NOTE_OR_COMMAND) != COMMENT)
    DisplayMsg ("", 0, COMMENT);

CStyle * pOldStyle = NULL;

  if (!m_pCurrentLine->styleList.IsEmpty ())
    pOldStyle = m_pCurrentLine->styleList.GetTail ();

  if (m_bNotesInRGB)
    {
    // change style if we need to
    if (!(pOldStyle &&
        (pOldStyle->iFlags & COLOURTYPE) == COLOUR_RGB &&
        pOldStyle->iForeColour == m_iNoteColourFore &&
        pOldStyle->iBackColour == m_iNoteColourBack &&
        (pOldStyle->iFlags & TEXT_STYLE) == m_iNoteStyle
        ))
        AddStyle (COLOUR_RGB | m_iNoteStyle, 
                  m_iNoteColourFore, m_iNoteColourBack, 0, NULL);
    }   // end or RGB notes
  else
    {
    // finally found Poremenos's bug - he was doing a world.colournote
    // with the note colour as  -1 (65535) which gave an access violation
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      // change style if we need to
      if (!(pOldStyle &&
          (pOldStyle->iFlags & COLOURTYPE) == COLOUR_ANSI &&
          pOldStyle->iForeColour == WHITE &&
          pOldStyle->iBackColour == BLACK &&
          (pOldStyle->iFlags & TEXT_STYLE) == m_iNoteStyle
          ))
          AddStyle (COLOUR_ANSI | m_iNoteStyle, 
                    WHITE, BLACK, 0, NULL);
      }
    else
      {
      // change style if we need to
      if (!(pOldStyle &&
          (pOldStyle->iFlags & COLOURTYPE) == COLOUR_CUSTOM &&
          pOldStyle->iForeColour == m_iNoteTextColour &&
          pOldStyle->iBackColour == BLACK &&
          (pOldStyle->iFlags & TEXT_STYLE) == m_iNoteStyle
          ))
          AddStyle (COLOUR_CUSTOM | m_iNoteStyle, 
                    m_iNoteTextColour, BLACK, 0, NULL);
      } // not samecolour
    } // end of palette notes

  DisplayMsg (Message, strlen (Message), COMMENT);
}


// world.Sound - plays a sound

long CMUSHclientDoc::Sound(LPCTSTR SoundFileName) 
{
  if (SoundFileName [0] == 0)
    return eNoNameSpecified;

  if (!PlaySoundFile (SoundFileName))
  	return eCannotPlaySound;

  return eOK;
}

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
  
}

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

}

// world.Save - saves the current world - under a new name if supplied
//              returns zero if no error
BOOL CMUSHclientDoc::Save(LPCTSTR Name) 
{
CString strName = Name;

// if empty, take document default path name [#459]

  if (strName.IsEmpty ())
    strName = m_strPathName;

  return !DoSave (strName, TRUE);

}

long CMUSHclientDoc::OpenLog(LPCTSTR LogFileName, BOOL Append) 
{

  if (m_logfile)
  	return eLogFileAlreadyOpen;

  m_logfile_name = LogFileName;

  // if no log file name, use auto-log file name if we can

  if (m_logfile_name.IsEmpty ())
    {
    CTime theTime;
    theTime = CTime::GetCurrentTime();

    m_logfile_name = FormatTime (theTime, m_strAutoLogFileName, false);

    }

  // no file name? can't open it then
  if (m_logfile_name.IsEmpty ())
    return eCouldNotOpenFile;

  m_logfile = fopen (m_logfile_name, Append ? "a+" : "w");
	
  // close and re-open to make sure it is in the disk directory
  if (m_logfile)
    {
    fclose (m_logfile);
    m_logfile = fopen (m_logfile_name, Append ? "a+" : "w");
    }

	if (m_logfile)
    return eOK;
  else
    return eCouldNotOpenFile;

}

long CMUSHclientDoc::CloseLog() 
{

  if (m_logfile)
    {

  // write log file Postamble if wanted

    if (!m_strLogFilePostamble.IsEmpty () && !m_bLogRaw)
      {
      CTime theTime;
      theTime = CTime::GetCurrentTime();

      // allow %n for newline
      CString strPostamble = ::Replace (m_strLogFilePostamble, "%n", "\n");

      // allow for time-substitution strings
      strPostamble = FormatTime (theTime, strPostamble, m_bLogHTML);

      // this is open in text mode, don't want \r\r\n
      strPostamble.Replace (ENDLINE, "\n");

      WriteToLog (strPostamble); 
      WriteToLog ("\n", 1);
      }

    fclose (m_logfile);
    m_logfile = NULL;
    return eOK;
    }

	return eLogFileNotOpen;
}


long CMUSHclientDoc::WriteLog(LPCTSTR Message) 
{
  if (m_logfile)
    {

    CString strMessage = Message;

    // append newline if there isn't one already
    if (strMessage.Right (2) != "\n")
      strMessage += "\n";

    size_t count;
    size_t len = strMessage.GetLength ();

    count = fwrite (strMessage, 1, len, m_logfile);

    if (count != len)
      return eLogFileBadWrite;

    return eOK;
    }

	return eLogFileNotOpen;
}

BOOL CMUSHclientDoc::IsLogOpen() 
{
	return m_logfile != NULL;
}

// ******************************************************************************
// **************************** TRIGGERS ****************************************
// ******************************************************************************

// world.DeleteTrigger (trigger_name) - deletes the named trigger

long CMUSHclientDoc::DeleteTrigger(LPCTSTR TriggerName) 
{
CString strTriggerName = TriggerName;
CTrigger * trigger_item;

  // trim spaces from name, make lower-case
  CheckObjectName (strTriggerName, false);
 
  if (!GetTriggerMap ().Lookup (strTriggerName, trigger_item))
    return eTriggerNotFound;

  // can't if executing a script
  if (trigger_item->bExecutingScript)
    return eItemInUse;

  bool bTemporary = trigger_item->bTemporary;

  // the trigger seems to exist - delete its pointer
  delete trigger_item;

  // now delete its entry
  if (!GetTriggerMap ().RemoveKey (strTriggerName))
    return eTriggerNotFound;

  SortTriggers ();

  if (!m_CurrentPlugin && !bTemporary) // plugin mods don't really count
    SetModifiedFlag (TRUE);   // document has changed
  return eOK;
}

// world.AddTriggerEx (trigger_name, ...) - adds the named trigger

long CMUSHclientDoc::AddTriggerEx(LPCTSTR TriggerName, 
                                  LPCTSTR MatchText, 
                                  LPCTSTR ResponseText, 
                                  long Flags, 
                                  short Colour, 
                                  short Wildcard, 
                                  LPCTSTR SoundFileName, 
                                  LPCTSTR ScriptName, 
                                  short SendTo, 
                                  short Sequence) 
{
CString strTriggerName = TriggerName;
CTrigger * trigger_item;
DISPID dispid = DISPID_UNKNOWN;
long nStatus;
bool bReplace = false;

  // allow blank names, assign one :)
  if (strTriggerName.IsEmpty ())
    strTriggerName.Format ("*trigger%s", (LPCTSTR) App.GetUniqueString ());
  else
    // return if bad name
    if (nStatus = CheckObjectName (strTriggerName))
      return nStatus;

  // if it already exists, error
  if (GetTriggerMap ().Lookup (strTriggerName, trigger_item))
    if (Flags & eReplace)
      bReplace = true;
    else
      return eTriggerAlreadyExists;

  // cannot have null match text
  if (strlen (MatchText) == 0)
    return eTriggerCannotBeEmpty;

  // check sequence

  if (Sequence < 0 || Sequence > 10000)
    return eTriggerSequenceOutOfRange;

  // check send to
  if (SendTo < 0 || SendTo >= eSendToLast) 
    return eTriggerSendToInvalid; 

  // must have a label for 'send to label'
  if (SendTo == eSendToVariable)
    if (CheckObjectName (strTriggerName))
       return eTriggerLabelNotSpecified;

// get trigger dispatch ID
  
  if (GetScriptEngine () && strlen (ScriptName) != 0)
    {
    CString strMessage;
    dispid = GetProcedureDispid (ScriptName, "trigger", TriggerName, strMessage);
    if (dispid == DISPID_UNKNOWN)
      return eScriptNameNotLocated;
    }

  t_regexp * regexp = NULL;

  CString strRegexp; 

  if (Flags & eTriggerRegularExpression)
    strRegexp = MatchText;
  else
    strRegexp = ConvertToRegularExpression (MatchText);

  // compile regular expression
  try 
    {
    regexp = regcomp (strRegexp, (Flags & eIgnoreCase ? PCRE_CASELESS : 0) | (m_bUTF_8 ? PCRE_UTF8 : 0));
    }   // end of try
  catch(CException* e)
    {
    e->Delete ();
    return eBadRegularExpression;
    } // end of catch

  // trigger replacement wanted
  if (bReplace)
    {
    // the trigger seems to exist - delete its pointer
    delete trigger_item;

    // now delete its entry
    GetTriggerMap ().RemoveKey (strTriggerName);
    }

  // create new trigger item and insert in trigger map
  GetTriggerMap ().SetAt (strTriggerName, trigger_item = new CTrigger);

  if ((Flags & eTemporary) == 0)
    if (!m_CurrentPlugin) // plugin mods don't really count
      SetModifiedFlag (TRUE);

  trigger_item->nUpdateNumber    = App.GetUniqueNumber ();   // for concurrency checks
  trigger_item->strInternalName  = strTriggerName;    // for deleting one-shot triggers

  trigger_item->trigger          = MatchText;
  trigger_item->contents         = ResponseText;
  trigger_item->colour           = Colour;
  trigger_item->ignore_case      = (Flags & eIgnoreCase) != 0;
  trigger_item->bOmitFromOutput  = (Flags & eOmitFromOutput) != 0;
  trigger_item->bKeepEvaluating  = (Flags & eKeepEvaluating) != 0;
  trigger_item->omit_from_log    = (Flags & eOmitFromLog) != 0;    
  trigger_item->bEnabled         = (Flags & eEnabled) != 0;
  trigger_item->bRegexp          = (Flags & eTriggerRegularExpression) != 0;
  trigger_item->bExpandVariables = (Flags & eExpandVariables) != 0;
  trigger_item->bTemporary       = (Flags & eTemporary) != 0;
  trigger_item->bLowercaseWildcard       = (Flags & eLowercaseWildcard) != 0;
  trigger_item->bOneShot         = (Flags & eTriggerOneShot) != 0;
  trigger_item->strProcedure     = ScriptName;
  trigger_item->strLabel         = TriggerName;
  trigger_item->iClipboardArg    = Wildcard;
  trigger_item->sound_to_play    = SoundFileName;
  trigger_item->dispid           = dispid;
  trigger_item->regexp           = regexp;
  trigger_item->iSendTo          = SendTo;
  trigger_item->iSequence        = Sequence;
  trigger_item->strVariable      = TriggerName;   // kludge


  if (Colour < 0 || Colour >= MAX_CUSTOM)
    trigger_item->colour = SAMECOLOUR;
  if (Wildcard < 0 || Wildcard > 10)
    trigger_item->iClipboardArg = 0;

  SortTriggers ();

	return eOK;
}

// world.AddTrigger (trigger_name, ...) - adds the named trigger

long CMUSHclientDoc::AddTrigger(LPCTSTR TriggerName, 
                                LPCTSTR MatchText, 
                                LPCTSTR ResponseText, 
                                long Flags, 
                                short Colour, 
                                short Wildcard, 
                                LPCTSTR SoundFileName, 
                                LPCTSTR ScriptName) 
{
  
return AddTriggerEx (TriggerName, 
                      MatchText, 
                      ResponseText, 
                      Flags, 
                      Colour, 
                      Wildcard, 
                      SoundFileName, 
                      ScriptName, 
                      eSendToWorld, 
                      DEFAULT_TRIGGER_SEQUENCE);
}

// world.EnableTrigger (trigger_name, bEnableFlag) - enables or disables the named trigger

long CMUSHclientDoc::EnableTrigger(LPCTSTR TriggerName, BOOL Enabled) 
{
CString strTriggerName = TriggerName;
CTrigger * trigger_item;

  // trim spaces from name, make lower-case
  CheckObjectName (strTriggerName, false);

  if (!GetTriggerMap ().Lookup (strTriggerName, trigger_item))
    return eTriggerNotFound;

  if (trigger_item->bEnabled && Enabled)
    return eOK;   // already enabled, document hasn't changed

  if (!trigger_item->bEnabled && !Enabled)
    return eOK;   // already not enabled, document hasn't changed


  trigger_item->bEnabled = Enabled != 0;                // set enabled flag
  trigger_item->nUpdateNumber   = App.GetUniqueNumber ();   // for concurrency checks

  if (!m_CurrentPlugin) // plugin mods don't really count
    SetModifiedFlag (TRUE);   // document has changed
  return eOK;
}

// world.GetTrigger (trigger_name, ...) - gets details about the named trigger

long CMUSHclientDoc::GetTrigger(LPCTSTR TriggerName, 
                                VARIANT FAR* MatchText, 
                                VARIANT FAR* ResponseText, 
                                VARIANT FAR* Flags, 
                                VARIANT FAR* Colour, 
                                VARIANT FAR* Wildcard, 
                                VARIANT FAR* SoundFileName, 
                                VARIANT FAR* ScriptName) 
{
CString strTriggerName = TriggerName;
CTrigger * trigger_item;

  // trim spaces from name, make lower-case
  CheckObjectName (strTriggerName, false);

  if (!GetTriggerMap ().Lookup (strTriggerName, trigger_item))
    return eTriggerNotFound;

  SetUpVariantString (*MatchText, trigger_item->trigger);
  SetUpVariantString (*ResponseText, trigger_item->contents);
  SetUpVariantString (*ScriptName, trigger_item->strProcedure);
  SetUpVariantString (*SoundFileName, trigger_item->sound_to_play);
  SetUpVariantShort  (*Colour, trigger_item->colour);
  if (trigger_item->colour == SAMECOLOUR)
    SetUpVariantShort  (*Colour, -1);

  short iFlags = 0;

  if (trigger_item->ignore_case) 
    iFlags |= eIgnoreCase;
  if (trigger_item->bOmitFromOutput)
    iFlags |= eOmitFromOutput;  
  if (trigger_item->bKeepEvaluating) 
    iFlags |= eKeepEvaluating;   
  if (trigger_item->omit_from_log) 
    iFlags |= eOmitFromOutput;   
  if (trigger_item->bEnabled) 
    iFlags |= eEnabled;  
  if (trigger_item->bRegexp) 
    iFlags |= eTriggerRegularExpression;  
  if (trigger_item->bLowercaseWildcard) 
    iFlags |= eLowercaseWildcard;  
  if (trigger_item->bOneShot) 
    iFlags |= eTriggerOneShot;

  SetUpVariantShort  (*Flags, iFlags);
  SetUpVariantShort  (*Wildcard, trigger_item->iClipboardArg);

  return eOK;
}

// world.IsTrigger (trigger_name) - returns eOK if the trigger exists

long CMUSHclientDoc::IsTrigger(LPCTSTR TriggerName) 
{
CString strTriggerName = TriggerName;
CTrigger * trigger_item;

  // trim spaces from name, make lower-case
  CheckObjectName (strTriggerName, false);

  if (!GetTriggerMap ().Lookup (strTriggerName, trigger_item))
    return eTriggerNotFound;

	return eOK;
}

// world.GetTriggerList - returns a variant array which is a list of trigger names

VARIANT CMUSHclientDoc::GetTriggerList() 
{
  COleSafeArray sa;   // for wildcard list

  CString strTriggerName;
  CTrigger * trigger_item;
  long iCount = 0;
  POSITION pos;

  iCount = GetTriggerMap ().GetCount ();

  if (iCount) // cannot create empty array dimension
    {
    sa.CreateOneDim (VT_VARIANT, iCount);
  
    for (iCount = 0, pos = GetTriggerMap ().GetStartPosition(); pos; iCount++)
      {
      GetTriggerMap ().GetNextAssoc (pos, strTriggerName, trigger_item);

      // the array must be a bloody array of variants, or VBscript kicks up
      COleVariant v (strTriggerName);
      sa.PutElement (&iCount, &v);
      }      // end of looping through each trigger
    } // end of having at least one

	return sa.Detach ();
}


// ******************************************************************************
// **************************** VARIABLES ***************************************
// ******************************************************************************

// world.SetVariable (variable_name, new_contents) - sets the named variable's contents
//                                                 - creating it if necessary

long CMUSHclientDoc::SetVariable(LPCTSTR VariableName, LPCTSTR Contents) 
{
CString strVariableName = VariableName;
CVariable * variable_item;
long nStatus;

  // return if bad name
  if (nStatus = CheckObjectName (strVariableName))
    return nStatus;

  // get rid of old variable, if any
  if (GetVariableMap ().Lookup (strVariableName, variable_item))
    delete variable_item;

  // create new variable item and insert in variable map
  GetVariableMap ().SetAt (strVariableName, variable_item = new CVariable);
  m_bVariablesChanged = true;
//  SetModifiedFlag (TRUE); // set flag instead now
  variable_item->nUpdateNumber = App.GetUniqueNumber ();   // for concurrency checks

  // set up variable item contents
  variable_item->strLabel = VariableName;
  variable_item->strContents = Contents;

	return eOK;
}

// world.GetVariable (variable_name) - gets details about the named variable
//                                     returns "EMPTY" variable if not there
//                                     returns "NULL" variable if a bad name

VARIANT CMUSHclientDoc::GetVariable(LPCTSTR VariableName) 
{
CString strVariableName = VariableName;
CVariable * variable_item;
VARIANT vaResult;

  VariantInit(&vaResult);

  vaResult.vt = VT_NULL;

  // return if bad name, if so return NULL
  if (CheckObjectName (strVariableName))
    return vaResult;

  vaResult.vt = VT_EMPTY;

  // see if variable exists, if not return EMPTY
  if (!GetVariableMap ().Lookup (strVariableName, variable_item))
	  return vaResult;

  SetUpVariantString (vaResult, variable_item->strContents);

  return vaResult;

}

// world.GetVariableList - returns a variant array which is a list of variable names

VARIANT CMUSHclientDoc::GetVariableList() 
{
  COleSafeArray sa;   // for variable list

  CString strVariableName;
  CVariable * variable_item;

  POSITION pos;
  long iCount;
  
  // put the Variables into the array
  if (!GetVariableMap ().IsEmpty ())    // cannot create empty dimension
    {
    sa.CreateOneDim (VT_VARIANT, GetVariableMap ().GetCount ());

    for (iCount = 0, pos = GetVariableMap ().GetStartPosition(); pos; )
      {
      GetVariableMap ().GetNextAssoc (pos, strVariableName, variable_item);

      // the array must be a bloody array of variants, or VBscript kicks up
      COleVariant v (strVariableName);
      sa.PutElement (&iCount, &v);
      iCount++;
      }      // end of looping through each Variable
    } // end of having at least one

	return sa.Detach ();
}

// world.DeleteVariable (variable_name) - deletes the named variable

long CMUSHclientDoc::DeleteVariable(LPCTSTR VariableName) 
{
CString strVariableName = VariableName;
CVariable * variable_item;
long nStatus;

  // return if bad name
  if (nStatus = CheckObjectName (strVariableName))
    return nStatus;

  if (!GetVariableMap ().Lookup (strVariableName, variable_item))
    return eVariableNotFound;

  // the variable seems to exist - delete its pointer
  delete variable_item;

  // now delete its entry
  if (!GetVariableMap ().RemoveKey (strVariableName))
    return eVariableNotFound;

  if (!m_CurrentPlugin) // plugin mods don't really count
    SetModifiedFlag (TRUE);   // document has changed

	return eOK;
} // end of DeleteVariable

// ******************************************************************************
// **************************** ALIASES  ****************************************
// ******************************************************************************

// world.DeleteAlias (alias_name) - deletes the named alias


long CMUSHclientDoc::DeleteAlias(LPCTSTR AliasName) 
{
CString strAliasName = AliasName;
CAlias * alias_item;

  // trim spaces from name, make lower-case
  CheckObjectName (strAliasName, false);

  if (!GetAliasMap ().Lookup (strAliasName, alias_item))
    return eAliasNotFound;

  // can't if executing a script
  if (alias_item->bExecutingScript)
    return eItemInUse;

  bool bTemporary = alias_item->bTemporary;

  // the alias seems to exist - delete its pointer
  delete alias_item;

  // now delete its entry
  if (!GetAliasMap ().RemoveKey (strAliasName))
    return eAliasNotFound;

  SortAliases ();

  if (!m_CurrentPlugin && !bTemporary) // plugin mods don't really count
    SetModifiedFlag (TRUE);   // document has changed
  return eOK;
}

// world.AddAlias (alias_name, ...) - adds the named alias

long CMUSHclientDoc::AddAlias(LPCTSTR AliasName, 
                              LPCTSTR MatchText, 
                              LPCTSTR ResponseText, 
                              long Flags, 
                              LPCTSTR ScriptName) 
{
CString strAliasName = AliasName;
CAlias * alias_item;
DISPID dispid = DISPID_UNKNOWN;
long nStatus;
bool bReplace = false;

  // allow blank name, allocate one
  if (strAliasName.IsEmpty ())
    strAliasName.Format ("*alias%s", (LPCTSTR) App.GetUniqueString ());
  else
    // return if bad name
    if (nStatus = CheckObjectName (strAliasName))
      return nStatus;

  // if it already exists, error
  if (GetAliasMap ().Lookup (strAliasName, alias_item))
    if (Flags & eReplace)
      bReplace = true;
    else
      return eAliasAlreadyExists;

  // cannot have null match text
  if (strlen (MatchText) == 0)
    return eAliasCannotBeEmpty;

// get alias dispatch ID
  
  if (GetScriptEngine () && strlen (ScriptName) != 0)
    {
    CString strMessage;
    dispid = GetProcedureDispid (ScriptName, "alias", AliasName, strMessage);
    if (dispid == DISPID_UNKNOWN)
      return eScriptNameNotLocated;
    }

  t_regexp * regexp = NULL;

  CString strRegexp; 

  if (Flags & eAliasRegularExpression)
    strRegexp = MatchText;
  else
    strRegexp = ConvertToRegularExpression (MatchText);

  // compile regular expression
  try 
    {
    regexp = regcomp (strRegexp, (Flags & eIgnoreAliasCase ? PCRE_CASELESS : 0)
#if ALIASES_USE_UTF8
                                  | (m_bUTF_8 ? PCRE_UTF8 : 0)
#endif // ALIASES_USE_UTF8
        );
    }   // end of try
  catch(CException* e)
    {
    e->Delete ();
    return eBadRegularExpression;
    } // end of catch

  // alias replacement wanted
  if (bReplace)
    {
    // the alias seems to exist - delete its pointer
    delete alias_item;

    // now delete its entry
    GetAliasMap ().RemoveKey (strAliasName);
    }

  // create new alias item and insert in alias map
  GetAliasMap ().SetAt (strAliasName, alias_item = new CAlias);

  if ((Flags & eTemporary) == 0)
    if (!m_CurrentPlugin) // plugin mods don't really count
      SetModifiedFlag (TRUE);

  alias_item->nUpdateNumber    = App.GetUniqueNumber ();   // for concurrency checks
  alias_item->strInternalName  = strAliasName;    // for deleting one-shot aliases

  alias_item->name             = MatchText;
  alias_item->contents         = ResponseText;
  alias_item->bIgnoreCase      = (Flags & eIgnoreAliasCase) != 0;
//  alias_item->bDelayed         = (Flags & eDelayed) != 0;
  alias_item->bEnabled         = (Flags & eEnabled) != 0;
  alias_item->bOmitFromLog     = (Flags & eOmitFromLogFile) != 0;
  alias_item->bRegexp          = (Flags & eAliasRegularExpression) != 0;
  alias_item->bOmitFromOutput  = (Flags & eAliasOmitFromOutput) != 0;
  alias_item->bExpandVariables = (Flags & eExpandVariables) != 0;
  alias_item->bMenu            = (Flags & eAliasMenu) != 0;
  alias_item->bTemporary       = (Flags & eTemporary) != 0;
  alias_item->bOneShot         = (Flags & eAliasOneShot) != 0;
  alias_item->strProcedure     = ScriptName;
  alias_item->strLabel         = AliasName;
  alias_item->dispid           = dispid;
  alias_item->regexp           = regexp;

  // fix up "sendto" appropriately

  if (Flags & eAliasSpeedWalk)
     alias_item->iSendTo = eSendToSpeedwalk;
  else if (Flags & eAliasQueue)
     alias_item->iSendTo = eSendToCommandQueue;

  SortAliases ();

	return eOK;
}

// world.EnableAlias (alias_name, bEnableFlag) - enables or disables the named alias

long CMUSHclientDoc::EnableAlias(LPCTSTR AliasName, BOOL Enabled) 
{
CString strAliasName = AliasName;
CAlias * alias_item;

  // trim spaces from name, make lower-case
  CheckObjectName (strAliasName, false);

  if (!GetAliasMap ().Lookup (strAliasName, alias_item))
    return eAliasNotFound;

  if (alias_item->bEnabled && Enabled)
    return eOK;   // already enabled, document hasn't changed

  if (!alias_item->bEnabled && !Enabled)
    return eOK;   // already not enabled, document hasn't changed


  alias_item->bEnabled = Enabled != 0;                // set enabled flag
  alias_item->nUpdateNumber   = App.GetUniqueNumber ();   // for concurrency checks

  if (!m_CurrentPlugin) // plugin mods don't really count
    SetModifiedFlag (TRUE);   // document has changed
  return eOK;
}

// world.GetAlias (alias_name, ...) - gets details about the named alias

long CMUSHclientDoc::GetAlias(LPCTSTR AliasName, 
                              VARIANT FAR* MatchText, 
                              VARIANT FAR* ResponseText, 
                              VARIANT FAR* Parameter, 
                              VARIANT FAR* Flags, 
                              VARIANT FAR* ScriptName) 
{
CString strAliasName = AliasName;
CAlias * alias_item;

  // trim spaces from name, make lower-case
  CheckObjectName (strAliasName, false);

  if (!GetAliasMap ().Lookup (strAliasName, alias_item))
    return eAliasNotFound;

  SetUpVariantString (*MatchText, alias_item->name);
  SetUpVariantString (*ResponseText, alias_item->contents);
  SetUpVariantString (*ScriptName, alias_item->strProcedure);
  SetUpVariantString (*Parameter, "");

  short iFlags = 0;

  if (alias_item->bEnabled) 
    iFlags |= eEnabled;
  if (alias_item->bIgnoreCase) 
    iFlags |= eIgnoreAliasCase;
//  if (alias_item->bDelayed) 
//    iFlags |= eDelayed;
  if (alias_item->bOmitFromLog) 
    iFlags |= eOmitFromLogFile;
  if (alias_item->bRegexp) 
    iFlags |= eAliasRegularExpression;
  if (alias_item->bOmitFromOutput) 
    iFlags |= eAliasOmitFromOutput;
  if (alias_item->bExpandVariables) 
    iFlags |= eExpandVariables;
  if (alias_item->iSendTo == eSendToSpeedwalk) 
    iFlags |= eAliasSpeedWalk;
  if (alias_item->iSendTo == eSendToCommandQueue) 
    iFlags |= eAliasQueue;
  if (alias_item->bMenu) 
    iFlags |= eAliasMenu;
  if (alias_item->bTemporary) 
    iFlags |= eTemporary;
  if (alias_item->bOneShot) 
    iFlags |= eAliasOneShot;
  
  SetUpVariantShort  (*Flags, iFlags);

  return eOK;
}

VARIANT CMUSHclientDoc::GetAliasList() 
{
  COleSafeArray sa;   // for wildcard list

  CString strAliasName;
  CAlias * alias_item;
  long iCount = 0;
  POSITION pos;

  iCount = GetAliasMap ().GetCount ();

  if (iCount) // cannot create empty array dimension
    {
    sa.CreateOneDim (VT_VARIANT, iCount);
  
    // put the named aliases into the array
    for (iCount = 0, pos = GetAliasMap ().GetStartPosition(); pos; iCount++)
      {
      GetAliasMap ().GetNextAssoc (pos, strAliasName, alias_item);

      // the array must be a bloody array of variants, or VBscript kicks up
      COleVariant v (strAliasName);
      sa.PutElement (&iCount, &v);
        
      }      // end of looping through each alias
    } // end of having at least one

	return sa.Detach ();
}

// world.IsAlias (alias_name) - returns eOK if the alias exists

long CMUSHclientDoc::IsAlias(LPCTSTR AliasName) 
{
CString strAliasName = AliasName;
CAlias * alias_item;

  // trim spaces from name, make lower-case
  CheckObjectName (strAliasName, false);

  if (!GetAliasMap ().Lookup (strAliasName, alias_item))
    return eAliasNotFound;

	return eOK;
}



// world.EnableTimer (Timer_name, bEnableFlag) - enables or disables the named timer

long CMUSHclientDoc::EnableTimer(LPCTSTR TimerName, BOOL Enabled) 
{
CString strTimerName = TimerName;
CTimer * Timer_item;

  // trim spaces from name, make lower-case
  CheckObjectName (strTimerName, false);

  if (!GetTimerMap ().Lookup (strTimerName, Timer_item))
    return eTimerNotFound;

  if (Timer_item->bEnabled && Enabled)
    return eOK;   // already enabled, document hasn't changed

  if (!Timer_item->bEnabled && !Enabled)
    return eOK;   // already not enabled, document hasn't changed


  Timer_item->bEnabled = Enabled != 0;                // set enabled flag
  Timer_item->nUpdateNumber   = App.GetUniqueNumber ();   // for concurrency checks

  if (!m_CurrentPlugin) // plugin mods don't really count
    SetModifiedFlag (TRUE);   // document has changed
  return eOK;
}

// world.ResetTimers - resets all timers

void CMUSHclientDoc::ResetTimers() 
{
  ResetAllTimers (GetTimerMap ());
}


void CMUSHclientDoc::SetStatus(LPCTSTR Message) 
{
  m_strStatusMessage = Message;
  ShowStatusLine (true);    // show it now
}

long CMUSHclientDoc::SetCommand(LPCTSTR Message) 
{

  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	  {
		  CSendView* pmyView = (CSendView*)pView;

      CString strCurrent;

      pmyView->GetEditCtrl().GetWindowText (strCurrent);
      if (strCurrent.IsEmpty ())
        {
        pmyView->GetEditCtrl().ReplaceSel (Message, TRUE);
        pmyView->NotifyPluginCommandChanged ();
        return eOK;
        }   // end of command being empty
      else
        return eCommandNotEmpty;
	    }	  // end of being a CSendView
    }   // end of loop through views

	return eOK;
}

BSTR CMUSHclientDoc::PasteCommand(LPCTSTR Text) 
{
CString strResult;
int nStartChar,
    nEndChar;

  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	  {
		  CSendView* pmyView = (CSendView*)pView;

      CString strCurrent;

      // find the selection range
      pmyView->GetEditCtrl().GetSel(nStartChar, nEndChar);
      // get window text
      pmyView->GetEditCtrl().GetWindowText (strCurrent);
      pmyView->GetEditCtrl().ReplaceSel (Text, TRUE);
      pmyView->NotifyPluginCommandChanged ();

      // get selection if any
      if (nEndChar > nStartChar)
        strResult = strCurrent.Mid (nStartChar, nEndChar - nStartChar);
      break;

	    }	  // end of being a CSendView
    }   // end of loop through views

	return strResult.AllocSysString();
}


void CMUSHclientDoc::OnGameEditscriptfile() 
{

if (m_bEditScriptWithNotepad)
  {
  CTextDocument * pNewDoc =
    (CTextDocument *) App.OpenDocumentFile (m_strScriptFilename);

  if (pNewDoc)
    {
    pNewDoc->m_pRelatedWorld         = this;
    pNewDoc->m_iUniqueDocumentNumber = m_iUniqueDocumentNumber;   
    pNewDoc->SetTheFont ();
    }
  else
    ::TMessageBox("Unable to edit the script file.", 
                    MB_ICONEXCLAMATION);
  return;
  }   // end of using inbuilt notepad

  EditFileWithEditor (m_strScriptFilename);


	
}

void CMUSHclientDoc::OnUpdateGameEditscriptfile(CCmdUI* pCmdUI) 
{
  DoFixMenus (pCmdUI);  // remove accelerators from menus
  pCmdUI->Enable (!m_strScriptFilename.IsEmpty ());
	
}

BSTR CMUSHclientDoc::GetNotes() 
{
	CString strResult = m_notes;

	return strResult.AllocSysString();
}

void CMUSHclientDoc::SetNotes(LPCTSTR Message) 
{
	
  m_notes = Message;
  SetModifiedFlag (TRUE);   // document has changed

}

void CMUSHclientDoc::OnUpdateGameConfigureMudaddress(CCmdUI* pCmdUI) 
{
  DoFixMenus (pCmdUI);  // remove accelerators from menus
  pCmdUI->Enable (m_iConnectPhase == eConnectNotConnected ||
                  m_iConnectPhase == eConnectConnectedToMud);   // no changing of address whilst connecting	
}



short CMUSHclientDoc::GetNoteColour() 
{
  if (m_bNotesInRGB)
    return -1;

	return m_iNoteTextColour == SAMECOLOUR ? 0 : m_iNoteTextColour + 1;
}

void CMUSHclientDoc::SetNoteColour(short nNewValue) 
{
  if (nNewValue >= 0 && nNewValue <= MAX_CUSTOM)
    {
    m_iNoteTextColour = nNewValue - 1;
    m_bNotesInRGB = false;
    }
}

long CMUSHclientDoc::GetNoteColourFore() 
{
  if (m_bNotesInRGB)
    return m_iNoteColourFore;

  if (m_iNoteTextColour == SAMECOLOUR)
    {
    if (m_bCustom16isDefaultColour)
      return m_customtext [15];
    else
      return m_normalcolour [WHITE];
    } // not same colour
  else
  	return m_customtext [m_iNoteTextColour];
}

void CMUSHclientDoc::SetNoteColourFore(long nNewValue) 
{
// convert background to RGB if necessary

  if (!m_bNotesInRGB)
    {
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      if (m_bCustom16isDefaultColour)
        m_iNoteColourBack = m_customback [15];
      else
        m_iNoteColourBack = m_normalcolour [BLACK];
      } // not same colour
    else
  	  m_iNoteColourBack = m_customback [m_iNoteTextColour];
    }  // end of not notes in RGB

  m_bNotesInRGB = true;
  m_iNoteColourFore = nNewValue & 0x00FFFFFF;

}

long CMUSHclientDoc::GetNoteColourBack() 
{
  if (m_bNotesInRGB)
    return m_iNoteColourBack;

  if (m_iNoteTextColour == SAMECOLOUR)
    {
    if (m_bCustom16isDefaultColour)
      return m_customback [15];
    else
      return m_normalcolour [BLACK];
    } // not same colour
  else
  	return m_customback [m_iNoteTextColour];
}

void CMUSHclientDoc::SetNoteColourBack(long nNewValue) 
{
// convert foreground to RGB if necessary
  if (!m_bNotesInRGB)
    {
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      if (m_bCustom16isDefaultColour)
        m_iNoteColourFore = m_customtext [15];
      else
        m_iNoteColourFore = m_normalcolour [WHITE];
      } // not same colour
    else
  	  m_iNoteColourFore = m_customtext [m_iNoteTextColour];
    }  // end of not notes in RGB

  m_bNotesInRGB = true;
  m_iNoteColourBack = nNewValue & 0x00FFFFFF;

}

void CMUSHclientDoc::NoteColourRGB(long Foreground, long Background) 
{
  m_bNotesInRGB = true;
  m_iNoteColourFore = Foreground & 0x00FFFFFF;
  m_iNoteColourBack = Background & 0x00FFFFFF;
}

void CMUSHclientDoc::NoteColourName(LPCTSTR Foreground, LPCTSTR Background) 
{
// convert background to RGB if necessary
  if (!m_bNotesInRGB)
    {
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      if (m_bCustom16isDefaultColour)
        {
        m_iNoteColourFore = m_customtext [15];
        m_iNoteColourBack = m_customback [15];
        }
      else
        {
        m_iNoteColourFore = m_normalcolour [WHITE];
        m_iNoteColourBack = m_normalcolour [BLACK];
        }
      }   // end of notes were "same colour" (as what?)
    else
      // just normal custom colours - however check in range just in case
      if (m_iNoteTextColour >= 0 && m_iNoteTextColour < MAX_CUSTOM)
        {   // notes were a custom colour
        m_iNoteColourFore = m_customtext [m_iNoteTextColour];
        m_iNoteColourBack = m_customback [m_iNoteTextColour];
        }  // end of notes in custom colour
      else
        {   // white on black - shouldn't get here
        m_iNoteColourFore = RGB (255, 255, 255);
        m_iNoteColourBack = RGB (0, 0, 0);
        }  // end of notes in custom colour

    }   // end of not notes in RGB already

  m_bNotesInRGB = true;
  SetColour (Foreground, m_iNoteColourFore);
  SetColour (Background, m_iNoteColourBack);
}


long CMUSHclientDoc::GetNormalColour(short WhichColour) 
{
  if (WhichColour < 1 || WhichColour > 8)
    return 0;  
	return m_normalcolour [WhichColour - 1];
}

void CMUSHclientDoc::SetNormalColour(short WhichColour, long nNewValue) 
{
  if (WhichColour < 1 || WhichColour > 8)
    return;  
	m_normalcolour [WhichColour - 1] = nNewValue & 0x00FFFFFF;
}

long CMUSHclientDoc::GetBoldColour(short WhichColour) 
{
  if (WhichColour < 1 || WhichColour > 8)
    return 0;  
	return m_boldcolour [WhichColour - 1];
}

void CMUSHclientDoc::SetBoldColour(short WhichColour, long nNewValue) 
{
  if (WhichColour < 1 || WhichColour > 8)
    return;  
	m_boldcolour [WhichColour - 1] = nNewValue & 0x00FFFFFF;
}

long CMUSHclientDoc::GetCustomColourText(short WhichColour) 
{
  if (WhichColour < 1 || WhichColour > MAX_CUSTOM)
    return 0;  
	return m_customtext [WhichColour - 1];
}

void CMUSHclientDoc::SetCustomColourText(short WhichColour, long nNewValue) 
{
  if (WhichColour < 1 || WhichColour > MAX_CUSTOM)
    return;  
	m_customtext [WhichColour - 1] = nNewValue & 0x00FFFFFF;
}

long CMUSHclientDoc::GetCustomColourBackground(short WhichColour) 
{
  if (WhichColour < 1 || WhichColour > MAX_CUSTOM)
    return 0;  
	return m_customback [WhichColour - 1];
}

void CMUSHclientDoc::SetCustomColourBackground(short WhichColour, long nNewValue) 
{
  if (WhichColour < 1 || WhichColour > MAX_CUSTOM)
    return;  
	m_customback [WhichColour - 1] = nNewValue & 0x00FFFFFF;
}

void CMUSHclientDoc::Redraw() 
{
  UpdateAllViews (NULL);
}

void CMUSHclientDoc::OnLogNotesChanged() 
{
  m_bLogNotes = m_bLogNotes != 0;   // make boolean
}


void CMUSHclientDoc::OnLogInputChanged() 
{
  m_log_input = m_log_input != 0;   // make boolean
}


void CMUSHclientDoc::OnFileReloaddefaults() 
{

  CAliasMap dummy_alias;
  CTriggerMap dummy_trigger;
  CTimerMap dummy_timer;

  if (m_bUseDefaultColours && !App.m_strDefaultColoursFile.IsEmpty ())
    Load_Set (COLOUR, App.m_strDefaultColoursFile, &Frame);

  if (m_bUseDefaultTriggers && !App.m_strDefaultTriggersFile.IsEmpty ())
    Load_Set (TRIGGER, App.m_strDefaultTriggersFile, &Frame);

  if (m_bUseDefaultAliases && !App.m_strDefaultAliasesFile.IsEmpty ())
    Load_Set (ALIAS, App.m_strDefaultAliasesFile, &Frame);

  if (m_bUseDefaultTimers && !App.m_strDefaultTimersFile.IsEmpty ())
    Load_Set (TIMER, App.m_strDefaultTimersFile, &Frame);

  if (m_bUseDefaultMacros && !App.m_strDefaultMacrosFile.IsEmpty ())
    Load_Set (MACRO, App.m_strDefaultMacrosFile, &Frame);

  if (m_bUseDefaultInputFont && !App.m_strDefaultInputFont.IsEmpty ())
    {
    m_input_font_height = App.m_iDefaultInputFontHeight; 
    m_input_font_name = App.m_strDefaultInputFont;   
    m_input_font_italic = App.m_iDefaultInputFontItalic; 
    m_input_font_weight = App.m_iDefaultInputFontWeight;
    m_input_font_charset = App.m_iDefaultInputFontCharset;
    ChangeInputFont (m_input_font_height, 
                    m_input_font_name, 
                    m_input_font_weight, 
                    m_input_font_charset,
                    m_input_font_italic);

    }   // end of input font override

  if (m_bUseDefaultOutputFont && !App.m_strDefaultOutputFont.IsEmpty ())
    {
    m_font_height = App.m_iDefaultOutputFontHeight; 
    m_font_name = App.m_strDefaultOutputFont;   
    m_font_weight = FW_NORMAL;
    m_font_charset = App.m_iDefaultOutputFontCharset;
    ChangeFont (m_font_height, 
                m_font_name, 
                m_font_weight, 
                m_font_charset,
                m_bShowBold,
                m_bShowItalic,
                m_bShowUnderline,
                m_iLineSpacing);
    }   // end of output font override

  UpdateAllViews (NULL);
  
}

void CMUSHclientDoc::OnUpdateFileReloaddefaults(CCmdUI* pCmdUI) 
{
  DoFixMenus (pCmdUI);  // remove accelerators from menus
  pCmdUI->Enable ( 
     (m_bUseDefaultColours && !App.m_strDefaultColoursFile.IsEmpty ()) ||
     (m_bUseDefaultTriggers && !App.m_strDefaultTriggersFile.IsEmpty ()) ||
     (m_bUseDefaultAliases && !App.m_strDefaultAliasesFile.IsEmpty ()) ||
     (m_bUseDefaultTimers && !App.m_strDefaultTimersFile.IsEmpty ()) ||
     (m_bUseDefaultMacros && !App.m_strDefaultMacrosFile.IsEmpty ()) ||
     (m_bUseDefaultInputFont && !App.m_strDefaultInputFont.IsEmpty ()) ||
     (m_bUseDefaultOutputFont && !App.m_strDefaultOutputFont.IsEmpty ()));
	
}

long CMUSHclientDoc::ResetTimer(LPCTSTR TimerName) 
{
CString strTimerName = TimerName;
CTimer * Timer_item;

  // trim spaces from name, make lower-case
  CheckObjectName (strTimerName, false);

  if (!GetTimerMap ().Lookup (strTimerName, Timer_item))
    return eTimerNotFound;

  ResetOneTimer (Timer_item);

  return eOK;
}


// routine for doing traces

void CMUSHclientDoc::Trace (LPCTSTR lpszFormat, ...)
{         
  // do nothing if not tracing

  if (!m_bTrace)
    return;

	ASSERT(AfxIsValidString(lpszFormat, FALSE));

CString strMsg;

	va_list argList;
	va_start(argList, lpszFormat);
	strMsg.FormatV(lpszFormat, argList);
	va_end(argList);

  // see if a plugin will handle trace message
  CPlugin * pSavedPlugin = m_CurrentPlugin;
  m_CurrentPlugin = NULL;

  m_bTrace = false;  // stop infinite loops, where we report that the trace script was called

  // tell a plugin the trace message
  for (POSITION pluginpos = m_PluginList.GetHeadPosition(); pluginpos; )
    {
    CPlugin * pPlugin = m_PluginList.GetNext (pluginpos);


    if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
      continue;

    // see what the plugin makes of this,
    pPlugin->ExecutePluginScript (ON_PLUGIN_TRACE,
                                  strMsg,  
                                  pPlugin->m_dispid_plugin_trace); 

    if (pPlugin->m_dispid_plugin_trace != DISPID_UNKNOWN)
      {
      m_CurrentPlugin = pSavedPlugin;
      m_bTrace = true;
      return;   // sent to plugin? don't display it
      }

    }   // end of doing each plugin

  m_CurrentPlugin = pSavedPlugin;
  m_bTrace = true;

  strMsg += ENDLINE;      // add a new line

CString strFullMsg = "TRACE: ";

  strFullMsg += strMsg;


  DisplayMsg (strFullMsg, strFullMsg.GetLength (), COMMENT);

}


void CMUSHclientDoc::TraceOut(LPCTSTR Message) 
{
  Trace (Message);
}

void CMUSHclientDoc::OnGameTrace() 
{

  if (m_bTrace)
    {
    Trace ("Trace off");
    m_bTrace = false;
    }
  else
    {
    // if half-filled line, flush it out
    if (m_pCurrentLine && m_pCurrentLine->len > 0)
       StartNewLine (true, m_pCurrentLine->flags);
    m_bTrace = true;
    Trace ("Trace on");
    }

	
}

void CMUSHclientDoc::OnUpdateGameTrace(CCmdUI* pCmdUI) 
{	
  DoFixMenus (pCmdUI);  // remove accelerators from menus
  pCmdUI->Enable ();
  pCmdUI->SetCheck (m_bTrace);
}


// this is for my testing of getting RTF data from the clipboard

/*
void CMUSHclientDoc::OnEditTest() 
{
CString strMessage;

UINT dFormat = 0;

// Open clipboard

  if (!::OpenClipboard(Frame.GetSafeHwnd ()) )
       {
       ::TMessageBox( "Cannot open the Clipboard" );
       return;
       }

  char buf [1000];
  int  iLen;

  do
    {
    dFormat = EnumClipboardFormats (dFormat);

    if (dFormat == 0)
      break;

    iLen = GetClipboardFormatName (dFormat, buf, sizeof buf);

    CString strName = CString (buf, iLen);

    strMessage += CFormat ("Format = %04X %s\r\n", dFormat, (LPCTSTR) strName);
    } while (dFormat);


 // Close the clipboard

  CloseClipboard();

  CString strContents;

  GetClipboardContents (strContents, 0xC0B6);       // Rich Text Format

  strMessage += strContents;

  CRecallDlg * notesDlg = new CRecallDlg (RGB (255, 255, 255));
  notesDlg->m_textColour = RGB (0, 0, 0);
  notesDlg->m_backColour = RGB (255, 255, 255);
  notesDlg->m_strFontName = App.m_strDefaultInputFont;
  notesDlg->m_iFontSize = App.m_iDefaultInputFontHeight;
  notesDlg->m_iFontWeight = App.m_iDefaultInputFontWeight;
  notesDlg->m_bReadOnly = TRUE;

  notesDlg->m_strText = strMessage;            // what it says
  notesDlg->m_strTitle = "Test";   // set up dialog title
  notesDlg->Create (ID_RECALL_WINDOW, NULL);  // create it
  notesDlg->ShowWindow(SW_SHOW);  // and, finally, show it - dialog will delete itself
	
}

  */

BOOL CMUSHclientDoc::GetTrace() 
{
	return m_bTrace;
}

void CMUSHclientDoc::SetTrace(BOOL bNewValue) 
{

bNewValue = bNewValue != 0;   // make boolean

// if they are changing the value - go ahead and do it
if (bNewValue && !m_bTrace ||
    !bNewValue && m_bTrace)
  OnGameTrace ();

}


void CMUSHclientDoc::SetOutputFont(LPCTSTR FontName, short PointSize) 
{
  ChangeFont (PointSize, 
              FontName, 
              FW_NORMAL, 
              m_font_charset,
              m_bShowBold,
              m_bShowItalic,
              m_bShowUnderline,
              m_iLineSpacing);

}

void CMUSHclientDoc::SetInputFont(LPCTSTR FontName, short PointSize, short Weight, BOOL Italic) 
{

Italic = Italic != 0; // make boolean

  ChangeInputFont (PointSize, 
                  FontName, 
                  Weight, 
                  DEFAULT_CHARSET,
                  Italic);

}


bool CMUSHclientDoc::SwitchToNotepad (void)
  {
  int iCount = 0;

  for (POSITION docPos = App.m_pNormalDocTemplate->GetFirstDocPosition();
      docPos != NULL; )
    {
    CTextDocument * pTextDoc = (CTextDocument *) App.m_pWorldDocTemplate->GetNextDoc(docPos);

    // ignore unrelated worlds
    if (pTextDoc->m_pRelatedWorld == this &&
       pTextDoc->m_iUniqueDocumentNumber == m_iUniqueDocumentNumber)
      iCount++;
    } // end of doing each document

  if (iCount)
    {
    CChooseNotepadDlg dlg;

    dlg.m_pWorld = this;

    if (dlg.DoModal () != IDOK)
      return true;   // they gave up

    if (dlg.m_pTextDocument)  // they chose an existing one
      {
      // activate the view
      POSITION pos=dlg.m_pTextDocument->GetFirstViewPosition();

      if (pos)
        {
        CView* pView = dlg.m_pTextDocument->GetNextView(pos);

        if (pView->IsKindOf(RUNTIME_CLASS(CTextView)))
          {
          CTextView* pmyView = (CTextView*)pView;

          pmyView->GetParentFrame ()->ActivateFrame ();
          pmyView->GetParentFrame ()->SetActiveView(pmyView);
          return true;
          } // end of having the right type of view
        }   // end of having a view
      
      }   // end of choosing an existing document

    }   // end of having other documents


  return false;
  }   // end of CMUSHclientDoc::SwitchToNotepad

bool CMUSHclientDoc::AppendToTheNotepad (const CString strTitle,
                                      const CString strText,
                                      const bool bReplace,
                                      const int  iNotepadType)
  {
CTextDocument * pTextDoc = FindNotepad (strTitle);

  if (pTextDoc)
    {
    // append to the view
    POSITION pos=pTextDoc->GetFirstViewPosition();

    if (pos)
      {
      CView* pView = pTextDoc->GetNextView(pos);

      if (pView->IsKindOf(RUNTIME_CLASS(CTextView)))
        {
        CTextView* pmyView = (CTextView*)pView;

        // find actual window length for appending [#422]

        int iLength = pmyView->GetWindowTextLength ();

        if (bReplace)
          pmyView->GetEditCtrl ().SetSel (0, -1, FALSE);
        else
          pmyView->GetEditCtrl ().SetSel (iLength, iLength, FALSE);
        pmyView->GetEditCtrl ().ReplaceSel (strText);
        return true;
        } // end of having the right type of view
      }   // end of having a view
    return false;
    } // end of having an existing notepad document

  BOOL bOK = CreateTextWindow (strText,     // contents
                      strTitle,     // title
                      this,   // document
                      m_iUniqueDocumentNumber,      // document number
                      m_input_font_name,
                      m_input_font_height,
                      m_input_font_weight,
                      m_input_font_charset,
                      m_input_text_colour,
                      m_input_background_colour,
                      "",       // search string
                      "",       // line preamble
                      false,
                      false,
                      false,
                      false,  
                      false,
                      false,
                      iNotepadType
                      );

  // re-activate the main window
  Activate ();

  return bOK;
  }   // end of CMUSHclientDoc::AppendToTheNotepad

BOOL CMUSHclientDoc::SendToNotepad(LPCTSTR Title, LPCTSTR Contents) 
{
  return CreateTextWindow (Contents,     // contents
                      Title,     // title
                      this,   // document
                      m_iUniqueDocumentNumber,      // document number
                      m_input_font_name,
                      m_input_font_height,
                      m_input_font_weight,
                      m_input_font_charset,
                      m_input_text_colour,
                      m_input_background_colour,
                      "",       // search string
                      "",       // line preamble
                      false,
                      false,
                      false,
                      false,  
                      false,
                      false,
                      eNotepadScript
                      );
}

BOOL CMUSHclientDoc::AppendToNotepad(LPCTSTR Title, LPCTSTR Contents) 
{
  return AppendToTheNotepad (Title, Contents, false);   // append mode
}

BOOL CMUSHclientDoc::ActivateNotepad(LPCTSTR Title) 
{
CTextDocument * pTextDoc = FindNotepad (Title);

  if (pTextDoc)
    {
    // activate the view
    POSITION pos=pTextDoc->GetFirstViewPosition();

    if (pos)
      {
      CView* pView = pTextDoc->GetNextView(pos);

      if (pView->IsKindOf(RUNTIME_CLASS(CTextView)))
        {
        CTextView* pmyView = (CTextView*)pView;
        pmyView->GetParentFrame ()->ActivateFrame ();
        pmyView->GetParentFrame ()->SetActiveView(pmyView);
        return true;
        } // end of having the right type of view
      }   // end of having a view
    } // end of having an existing notepad document
 return false;
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

}


LPDISPATCH CMUSHclientDoc::GetWorld(LPCTSTR WorldName) 
{
  for (POSITION docPos = App.m_pWorldDocTemplate->GetFirstDocPosition();
      docPos != NULL; )
    {
    CMUSHclientDoc * pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(docPos);
  
    if (pDoc->m_mush_name.CompareNoCase (WorldName) == 0)
      return pDoc->GetIDispatch (TRUE);
    } // end of doing each document

	return NULL;
}

VARIANT CMUSHclientDoc::GetWorldList() 
{
  COleSafeArray sa;   // for list

  CMUSHclientDoc * pDoc;
  long iCount = 0;
  POSITION pos;

  // count number of worlds
  for (pos = App.m_pWorldDocTemplate->GetFirstDocPosition(); pos != NULL; iCount++)
    pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(pos);

  if (iCount) // cannot create empty array dimension
    {
    sa.CreateOneDim (VT_VARIANT, iCount);
  
    // put the worlds into the array
    for (iCount = 0, pos = App.m_pWorldDocTemplate->GetFirstDocPosition(); pos != NULL; iCount++)
      {
      pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(pos);

      // the array must be a bloody array of variants, or VBscript kicks up
      COleVariant v (pDoc->m_mush_name);
      sa.PutElement (&iCount, &v);
      }      // end of looping through each world
    } // end of having at least one

	return sa.Detach ();
}


VARIANT CMUSHclientDoc::GetWorldIdList() 
{
  COleSafeArray sa;   // for list

  CMUSHclientDoc * pDoc;
  long iCount = 0;
  POSITION pos;

  // count number of worlds
  for (pos = App.m_pWorldDocTemplate->GetFirstDocPosition(); pos != NULL; iCount++)
    pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(pos);

  if (iCount) // cannot create empty array dimension
    {
    sa.CreateOneDim (VT_VARIANT, iCount);
  
    // put the worlds into the array
    for (iCount = 0, pos = App.m_pWorldDocTemplate->GetFirstDocPosition(); pos != NULL; iCount++)
      {
      pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(pos);

      // the array must be a bloody array of variants, or VBscript kicks up
      COleVariant v (pDoc->m_strWorldID);
      sa.PutElement (&iCount, &v);
      }      // end of looping through each world
    } // end of having at least one

	return sa.Detach ();
}

LPDISPATCH CMUSHclientDoc::GetWorldById(LPCTSTR WorldID) 
{
  for (POSITION docPos = App.m_pWorldDocTemplate->GetFirstDocPosition();
      docPos != NULL; )
    {
    CMUSHclientDoc * pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(docPos);
  
    if (pDoc->m_strWorldID.CompareNoCase (WorldID) == 0)
      return pDoc->GetIDispatch (TRUE);
    } // end of doing each document

	return NULL;
}


BSTR CMUSHclientDoc::FixupHTML(LPCTSTR StringToConvert) 
{
	CString strResult = FixHTMLString (StringToConvert);
	return strResult.AllocSysString();
}

void CMUSHclientDoc::OnLogOutputChanged() 
{
	// TODO: Add notification handler code

}

BSTR CMUSHclientDoc::Replace(LPCTSTR Source, LPCTSTR SearchFor, LPCTSTR ReplaceWith, BOOL Multiple) 
{
	CString strResult;
  CString strSource = Source;
  strResult = ::Replace (strSource, SearchFor, ReplaceWith, Multiple != 0);
	return strResult.AllocSysString();
}

BSTR CMUSHclientDoc::FixupEscapeSequences(LPCTSTR Source) 
{
	CString strResult;
  CString strSource = Source;
  strResult = ::FixupEscapeSequences (strSource);
	return strResult.AllocSysString();
}

BSTR CMUSHclientDoc::Trim(LPCTSTR Source) 
{
  CString strSource = Source;
  strSource.TrimLeft ();
  strSource.TrimRight ();
  
	return strSource.AllocSysString();
}


BOOL CMUSHclientDoc::ReplaceNotepad(LPCTSTR Title, LPCTSTR Contents) 
{
  return AppendToTheNotepad (Title, Contents, true);    // replace mode
}

// world.GetAliasInfo (alias_name, info_type) - gets details about the named alias
//                                     returns "EMPTY" alias if not there
//                                     returns "NULL" alias if a bad name

VARIANT CMUSHclientDoc::GetAliasInfo(LPCTSTR AliasName, short InfoType) 
{
CString strAliasName = AliasName;
CAlias * alias_item;

	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_NULL;

  // trim spaces from name, make lower-case
  CheckObjectName (strAliasName, false);

  vaResult.vt = VT_EMPTY;

  // see if alias exists, if not return EMPTY
  if (!GetAliasMap ().Lookup (strAliasName, alias_item))
	  return vaResult;

  switch (InfoType)
    {
    case   1: SetUpVariantString (vaResult, alias_item->name); break;
    case   2: SetUpVariantString (vaResult, alias_item->contents); break;
    case   3: SetUpVariantString (vaResult, alias_item->strProcedure); break;
    case   4: SetUpVariantBool   (vaResult, alias_item->bOmitFromLog); break;
    case   5: SetUpVariantBool   (vaResult, alias_item->bOmitFromOutput); break;
    case   6: SetUpVariantBool   (vaResult, alias_item->bEnabled); break;
    case   7: SetUpVariantBool   (vaResult, alias_item->bRegexp); break;
    case   8: SetUpVariantBool   (vaResult, alias_item->bIgnoreCase); break;
    case   9: SetUpVariantBool   (vaResult, alias_item->bExpandVariables); break;
    case  10: SetUpVariantLong   (vaResult, alias_item->nInvocationCount); break;
    case  11: SetUpVariantLong   (vaResult, alias_item->nMatched); break;
    case  12: SetUpVariantLong   (vaResult, alias_item->bMenu); break;
    case  13: 
      if (alias_item->tWhenMatched.GetTime ())     // only if non-zero, otherwise return empty
        SetUpVariantDate   (vaResult, COleDateTime (alias_item->tWhenMatched.GetTime ())); 
      break;
    case  14: SetUpVariantBool   (vaResult, alias_item->bTemporary); break;
    case  15: SetUpVariantBool   (vaResult, alias_item->bIncluded); break;
    case  16: SetUpVariantString (vaResult, alias_item->strGroup); break;
    case  17: SetUpVariantString (vaResult, alias_item->strVariable); break;
    case  18: SetUpVariantLong   (vaResult, alias_item->iSendTo); break;
    case  19: SetUpVariantBool   (vaResult, alias_item->bKeepEvaluating); break;
    case  20: SetUpVariantLong   (vaResult, alias_item->iSequence); break;
    case  21: SetUpVariantBool   (vaResult, alias_item->bEchoAlias); break;
    case  22: SetUpVariantBool   (vaResult, alias_item->bOmitFromCommandHistory); break;
    case  23: SetUpVariantLong   (vaResult, alias_item->iUserOption); break;
//    case  ??: SetUpVariantBool   (vaResult, alias_item->bDelayed); break;

    case  24: // number of matches to regexp
      if (alias_item->regexp)      
        SetUpVariantLong   (vaResult, alias_item->regexp->m_iCount);
      else
        SetUpVariantLong   (vaResult, 0);
      break;

    case  25: // last matching string
      if (alias_item->regexp)      
        SetUpVariantString   (vaResult, alias_item->regexp->m_sTarget.c_str ());
      else
        SetUpVariantString   (vaResult, "");
      break;

    case  26: SetUpVariantBool   (vaResult, alias_item->bExecutingScript); break;
    case  27: SetUpVariantBool   (vaResult, alias_item->dispid != DISPID_UNKNOWN); break;

    case  28: 
      if (alias_item->regexp && alias_item->regexp->m_program == NULL)      
        SetUpVariantLong   (vaResult, alias_item->regexp->m_iExecutionError);
      else
        SetUpVariantLong   (vaResult, 0);
      break;
    case   29: SetUpVariantBool   (vaResult, alias_item->bOneShot); break;

    case  30:
      if (alias_item->regexp && App.m_iCounterFrequency)
        {
        double elapsed_time = ((double) alias_item->regexp->iTimeTaken) / 
                              ((double) App.m_iCounterFrequency);

        SetUpVariantDouble (vaResult, elapsed_time);
        }
      break;

    case  31:
      if (alias_item->regexp)
        SetUpVariantLong   (vaResult, alias_item->regexp->m_iMatchAttempts);
      break;

    case 101: SetUpVariantString (vaResult, alias_item->wildcards [1].c_str ()); break;
    case 102: SetUpVariantString (vaResult, alias_item->wildcards [2].c_str ()); break;
    case 103: SetUpVariantString (vaResult, alias_item->wildcards [3].c_str ()); break;
    case 104: SetUpVariantString (vaResult, alias_item->wildcards [4].c_str ()); break;
    case 105: SetUpVariantString (vaResult, alias_item->wildcards [5].c_str ()); break;
    case 106: SetUpVariantString (vaResult, alias_item->wildcards [6].c_str ()); break;
    case 107: SetUpVariantString (vaResult, alias_item->wildcards [7].c_str ()); break;
    case 108: SetUpVariantString (vaResult, alias_item->wildcards [8].c_str ()); break;
    case 109: SetUpVariantString (vaResult, alias_item->wildcards [9].c_str ()); break;
    case 110: SetUpVariantString (vaResult, alias_item->wildcards [0].c_str ()); break;

    default:
      vaResult.vt = VT_NULL;
      break;

    } // end of switch

  return vaResult;
}

// world.GetTriggerInfo (trigger_name, info_type) - gets details about the named trigger
//                                     returns "EMPTY" trigger if not there
//                                     returns "NULL" trigger if a bad name

VARIANT CMUSHclientDoc::GetTriggerInfo(LPCTSTR TriggerName, short InfoType) 
{
CString strTriggerName = TriggerName;
CTrigger * trigger_item;

	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_NULL;

  // trim spaces from name, make lower-case
  CheckObjectName (strTriggerName, false);

  vaResult.vt = VT_EMPTY;

  // see if trigger exists, if not return EMPTY
  if (!GetTriggerMap ().Lookup (strTriggerName, trigger_item))
	  return vaResult;

  switch (InfoType)
    {
    case   1: SetUpVariantString (vaResult, trigger_item->trigger); break;
    case   2: SetUpVariantString (vaResult, trigger_item->contents); break;
    case   3: SetUpVariantString (vaResult, trigger_item->sound_to_play); break;
    case   4: SetUpVariantString (vaResult, trigger_item->strProcedure); break;
    case   5: SetUpVariantBool   (vaResult, trigger_item->omit_from_log); break;
    case   6: SetUpVariantBool   (vaResult, trigger_item->bOmitFromOutput); break;
    case   7: SetUpVariantBool   (vaResult, trigger_item->bKeepEvaluating); break;
    case   8: SetUpVariantBool   (vaResult, trigger_item->bEnabled); break;
    case   9: SetUpVariantBool   (vaResult, trigger_item->bRegexp); break;
    case  10: SetUpVariantBool   (vaResult, trigger_item->ignore_case); break;
    case  11: SetUpVariantBool   (vaResult, trigger_item->bRepeat); break;
    case  12: SetUpVariantBool   (vaResult, trigger_item->bSoundIfInactive); break;
    case  13: SetUpVariantBool   (vaResult, trigger_item->bExpandVariables); break;
    case  14: SetUpVariantShort  (vaResult, trigger_item->iClipboardArg); break;
    case  15: SetUpVariantShort  (vaResult, trigger_item->iSendTo); break;
    case  16: SetUpVariantShort  (vaResult, trigger_item->iSequence); break;
    case  17: SetUpVariantShort  (vaResult, trigger_item->iMatch); break;
    case  18: SetUpVariantShort  (vaResult, trigger_item->iStyle); break;
    case  19: SetUpVariantShort  (vaResult, trigger_item->colour); break;
    case  20: SetUpVariantLong   (vaResult, trigger_item->nInvocationCount); break;
    case  21: SetUpVariantLong   (vaResult, trigger_item->nMatched); break;
    case  22: 
      if (trigger_item->tWhenMatched.GetTime ())     // only if non-zero, otherwise return empty
        SetUpVariantDate   (vaResult, COleDateTime (trigger_item->tWhenMatched.GetTime ())); 
      break;
    case  23: SetUpVariantBool   (vaResult, trigger_item->bTemporary); break;
    case  24: SetUpVariantBool   (vaResult, trigger_item->bIncluded); break;
    case  25: SetUpVariantBool   (vaResult, trigger_item->bLowercaseWildcard); break;
    case  26: SetUpVariantString (vaResult, trigger_item->strGroup); break;
    case  27: SetUpVariantString (vaResult, trigger_item->strVariable); break;
    case  28: SetUpVariantLong   (vaResult, trigger_item->iUserOption); break;
    case  29: SetUpVariantLong   (vaResult, trigger_item->iOtherForeground); break;
    case  30: SetUpVariantLong   (vaResult, trigger_item->iOtherBackground); break;
    case  31: // number of matches to regexp
      if (trigger_item->regexp)      
        SetUpVariantLong   (vaResult, trigger_item->regexp->m_iCount);
      else
        SetUpVariantLong   (vaResult, 0);
      break;

    case  32: // last matching string
      if (trigger_item->regexp)      
        SetUpVariantString   (vaResult, trigger_item->regexp->m_sTarget.c_str ());
      else
        SetUpVariantString   (vaResult, "");
      break;
    case  33: SetUpVariantBool   (vaResult, trigger_item->bExecutingScript); break;
    case  34: SetUpVariantBool   (vaResult, trigger_item->dispid != DISPID_UNKNOWN); break;
    case  35: 
      if (trigger_item->regexp && trigger_item->regexp->m_program == NULL)      
        SetUpVariantLong   (vaResult, trigger_item->regexp->m_iExecutionError);
      else
        SetUpVariantLong   (vaResult, 0);
      break;
    case   36: SetUpVariantBool   (vaResult, trigger_item->bOneShot); break;

    case  37:
      if (trigger_item->regexp && App.m_iCounterFrequency)
        {
        double   elapsed_time;

        elapsed_time = ((double) trigger_item->regexp->iTimeTaken) / 
                       ((double) App.m_iCounterFrequency);

        SetUpVariantDouble (vaResult, elapsed_time);
        }
      break;

    case  38:
      if (trigger_item->regexp)
        SetUpVariantLong   (vaResult, trigger_item->regexp->m_iMatchAttempts);
      break;

#ifdef PANE
    case  38: SetUpVariantString (vaResult, trigger_item->strPane); break;
#endif // PANE

    case 101: SetUpVariantString (vaResult, trigger_item->wildcards [1].c_str ()); break;
    case 102: SetUpVariantString (vaResult, trigger_item->wildcards [2].c_str ()); break;
    case 103: SetUpVariantString (vaResult, trigger_item->wildcards [3].c_str ()); break;
    case 104: SetUpVariantString (vaResult, trigger_item->wildcards [4].c_str ()); break;
    case 105: SetUpVariantString (vaResult, trigger_item->wildcards [5].c_str ()); break;
    case 106: SetUpVariantString (vaResult, trigger_item->wildcards [6].c_str ()); break;
    case 107: SetUpVariantString (vaResult, trigger_item->wildcards [7].c_str ()); break;
    case 108: SetUpVariantString (vaResult, trigger_item->wildcards [8].c_str ()); break;
    case 109: SetUpVariantString (vaResult, trigger_item->wildcards [9].c_str ()); break;
    case 110: SetUpVariantString (vaResult, trigger_item->wildcards [0].c_str ()); break;
    
    default:
      vaResult.vt = VT_NULL;
      break;

    } // end of switch

  return vaResult;
}

// rewritten 17th August 2002 to make it easier to follow and less buggy :)
//  (and support comments)

CString MakeSpeedWalkErrorString (LPCTSTR sMsg)
  {
  CString strResult = "*";

  strResult += sMsg;

  return strResult;

  } // end of MakeSpeedWalkErrorString

CString CMUSHclientDoc::DoEvaluateSpeedwalk(LPCTSTR SpeedWalkString) 
{
CString strResult,
        str;    // temporary string
int count; 
const char * p = SpeedWalkString;

  while (*p)  // until string runs out
    {
    // bypass spaces
    while (isspace (*p))
      p++;

    // bypass comments
    if (*p == '{')
      {
      while (*p && *p != '}')
        p++;

      if (*p != '}')
        return MakeSpeedWalkErrorString (Translate ("Comment code of \'{\' not terminated by a \'}\'"));
      p++;  // skip } symbol
      continue;  // back to start of loop
      }  // end of comment

    // get counter, if any
    count = 0;
    while (isdigit (*p))
      {
      count = (count * 10) + (*p++ - '0');
      if (count > 99)
        return MakeSpeedWalkErrorString (Translate ("Speed walk counter exceeds 99"));
      }   // end of having digit(s)

    // no counter, assume do once
    if (count == 0)
      count = 1;

    // bypass spaces after counter
    while (isspace (*p))
      p++;

    if (count > 1 && *p == 0)
      return MakeSpeedWalkErrorString (Translate ("Speed walk counter not followed by an action"));

    if (count > 1 && *p == '{')
      return MakeSpeedWalkErrorString (Translate ("Speed walk counter may not be followed by a comment"));

    // might have had trailing space
    if (*p == 0)
      break;

    if (strchr ("COLK", toupper (*p)))
      {
      if (count > 1)
        return MakeSpeedWalkErrorString (Translate ("Action code of C, O, L or K must not follow "
                         "a speed walk count (1-99)"));

      switch (toupper (*p++))
        {
        case 'C': strResult += "close "; break;
        case 'O': strResult += "open "; break;
        case 'L': strResult += "lock "; break;
        case 'K': strResult += "unlock "; break;
        } // end of switch

      // bypass spaces after open/close/lock/unlock
      while (isspace (*p))
        p++;

      if (*p == 0 || 
          toupper (*p) == 'F' || 
          *p == '{')
        return MakeSpeedWalkErrorString (Translate ("Action code of C, O, L or K must be followed "
                         "by a direction"));

      } // end of C, O, L, K
      
      // work out which direction we are going
    switch (toupper (*p))
      {
      case 'N': 
      case 'S': 
      case 'E': 
      case 'W': 
      case 'U': 
      case 'D': 
        // we know it will be in the list - look up the direction to send
        str = MapDirectionsMap [tolower (string (1, *p))].m_sDirectionToSend.c_str ();
        break;

      case 'F': str = m_strSpeedWalkFiller; break;
      case '(':     // special string (eg. (ne/sw) )
            {
            str.Empty ();
            for (p++ ; *p && *p != ')'; )
              str += *p++;   // add to string

            if (*p != ')')
              return MakeSpeedWalkErrorString (Translate ("Action code of \'(\' not terminated by a \')\'"));
            int iSlash = str.Find ("/");  // only use up to the slash
            if (iSlash != -1)
              str = str.Left (iSlash);
            }
            break;    // end of (blahblah/blah blah)
      default: 
            return MakeSpeedWalkErrorString (TFormat ("*Invalid direction \'%c\' in speed walk, must be "
                             "N, S, E, W, U, D, F, or (something)",
                             *p));
      } // end of switch on character

    p++;  // bypass whatever that character was (or the trailing bracket)

    // output required number of times
    for (int j = 0; j < count; j++) 
      strResult += str + ENDLINE;

    } // end of processing each character

	return strResult;
}


BSTR CMUSHclientDoc::EvaluateSpeedwalk(LPCTSTR SpeedWalkString) 
  {
CString strResult = DoEvaluateSpeedwalk (SpeedWalkString);
	return strResult.AllocSysString();
  }

CString CMUSHclientDoc::DoReverseSpeedwalk(LPCTSTR SpeedWalkString) 
{
CString strResult,  // build result here
        str,        // temporary string
        strAction;  // eg. open, close, lock, unlock
int count; 
const char * p = SpeedWalkString;

  while (*p)  // until string runs out
    {
    // preserve spaces
    while (isspace (*p))
      {
      switch (*p)
        {
        case '\r': break;    // discard carriage returns
        case '\n': strResult = ENDLINE + strResult;  // newline
                   break;
        default:   strResult = *p + strResult;
                   break;
        } // end of switch

      p++;
      } // end of preserving spaces

    // preserve comments
    if (*p == '{')
      {
      str.Empty ();
      for (; *p && *p != '}'; )
        str += *p++;   // add to string

      if (*p != '}')
        return MakeSpeedWalkErrorString (Translate ("Comment code of \'{\' not terminated by a \'}\'"));

      p++;  // skip } symbol

      str += "}";

      strResult = str + strResult;
      continue;  // back to start of loop
      }  // end of comment

    // get counter, if any
    count = 0;
    while (isdigit (*p))
      {
      count = (count * 10) + (*p++ - '0');
      if (count > 99)
        return MakeSpeedWalkErrorString (Translate ("Speed walk counter exceeds 99"));
      }   // end of having digit(s)

    // no counter, assume do once
    if (count == 0)
      count = 1;

    // bypass spaces after counter
    while (isspace (*p))
      p++;

    if (count > 1 && *p == 0)
      return MakeSpeedWalkErrorString (Translate ("Speed walk counter not followed by an action"));

    if (count > 1 && *p == '{')
      return MakeSpeedWalkErrorString (Translate ("Speed walk counter may not be followed by a comment"));

    // might have had trailing space
    if (*p == 0)
      break;

    if (strchr ("COLK", toupper (*p)))
      {
      if (count > 1)
        return MakeSpeedWalkErrorString (Translate ("Action code of C, O, L or K must not follow "
                         "a speed walk count (1-99)"));

      strAction = *p++;   // remember action

      // bypass spaces after open/close/lock/unlock
      while (isspace (*p))
        p++;

      if (*p == 0 || 
          toupper (*p) == 'F' || 
          *p == '{')
        return MakeSpeedWalkErrorString (Translate ("Action code of C, O, L or K must be followed "
                         "by a direction"));

      } // end of C, O, L, K
    else
      strAction.Empty ();  // no action
      
      // work out which direction we are going
    switch (toupper (*p))
      {
      case 'N': 
      case 'S': 
      case 'E': 
      case 'W': 
      case 'U': 
      case 'D': 
      case 'F': 
        // we know it will be in the list - look up the reverse direction
        str = MapDirectionsMap [tolower(string (1, *p))].m_sReverseDirection.c_str ();
        break;

      case '(':     // special string (eg. (ne/sw) )
            {
            str.Empty ();
            for (p++ ; *p && *p != ')'; )
              str += (char) tolower (*p++);   // add to string

            if (*p != ')')
              return MakeSpeedWalkErrorString (Translate ("Action code of \'(\' not terminated by a \')\'"));
            int iSlash = str.Find ("/");  // only use up to the slash
            // if no slash try to convert whole thing (eg. ne becomes sw)
            if (iSlash == -1)
              {
              MapDirectionsIterator i = MapDirectionsMap.find ((LPCTSTR) str);
              if (i != MapDirectionsMap.end ())
                str = i->second.m_sReverseDirection.c_str ();
              }
            else
              {
              CString strLeftPart = str.Left (iSlash);
              CString strRightPart = str.Mid (iSlash + 1);
              str = strRightPart + "/" +  strLeftPart; // swap parts
              }

            str = "(" + str;
            str += ")";
            }
            break;    // end of (blahblah/blah blah)
      default: 
            return MakeSpeedWalkErrorString (TFormat ("Invalid direction \'%c\' in speed walk, must be "
                             "N, S, E, W, U, D, F, or (something)",
                             *p));
      } // end of switch on character

    p++;  // bypass whatever that character was (or the trailing bracket)

    // output it
    if (count > 1)
      strResult = CFormat ("%i%s%s",       // counter, action, string
                           count,
                           (LPCTSTR) strAction,  // eg. open, close, lock, unlock
                           (LPCTSTR) str)  // direction
                           + strResult;

    else
      strResult =  strAction +  str + strResult;

    } // end of processing each character

	return strResult;
}

BSTR CMUSHclientDoc::ReverseSpeedwalk(LPCTSTR SpeedWalkString) 
  {
	CString strResult = DoReverseSpeedwalk (SpeedWalkString);
	return strResult.AllocSysString();
  }

long CMUSHclientDoc::AddTimer(LPCTSTR TimerName, 
                              short Hour, 
                              short Minute, 
                              double Second, 
                              LPCTSTR ResponseText, 
                              long Flags, 
                              LPCTSTR ScriptName) 
{
CString strTimerName = TimerName;
CTimer * timer_item;
DISPID dispid = DISPID_UNKNOWN;
long nStatus;
bool bReplace = false;

  if (strTimerName.IsEmpty ())
    strTimerName.Format ("*timer%s", (LPCTSTR) App.GetUniqueString ());
  else
    // return if bad name
    if (nStatus = CheckObjectName (strTimerName))
      return nStatus;

  // if it already exists, error
  if (GetTimerMap ().Lookup (strTimerName, timer_item))
    if (Flags & eReplace)
      bReplace = true;
    else
      return eTimerAlreadyExists;


  if (Hour < 0 || Hour > 23)
    return eTimeInvalid;

  if (Minute < 0 || Minute > 59)
    return eTimeInvalid;

  if (Second < 0.0 || Second > 59.9999)
    return eTimeInvalid;

  // can't have a zero time for "every" timers
  if (((Hour == 0) && (Minute == 0) && (Second == 0.0)) &&
      !(Flags & eAtTime))
    return eTimeInvalid;

// get timer dispatch ID
  
  if (GetScriptEngine () && strlen (ScriptName) != 0)
    {
    CString strMessage;
    dispid = GetProcedureDispid (ScriptName, "timer", TimerName, strMessage);
    if (dispid == DISPID_UNKNOWN)
      return eScriptNameNotLocated;
    }

  // timer replacement wanted
  if (bReplace)
    {
    // the timer seems to exist - delete its pointer
    delete timer_item;

    // now delete its entry
    GetTimerMap ().RemoveKey (strTimerName);
    }

  // create new timer item and insert in timer map
  GetTimerMap ().SetAt (strTimerName, timer_item = new CTimer);
 
  if ((Flags & eTemporary) == 0)
    if (!m_CurrentPlugin) // plugin mods don't really count
      SetModifiedFlag (TRUE);

  timer_item->nUpdateNumber    = App.GetUniqueNumber ();   // for concurrency checks

  if (Flags & eAtTime)
    {
    timer_item->iAtHour = Hour;
    timer_item->iAtMinute = Minute;
    timer_item->fAtSecond = Second;
    timer_item->iType  = CTimer::eAtTime;
    }
  else
    {
    timer_item->iEveryHour = Hour;
    timer_item->iEveryMinute = Minute;
    timer_item->fEverySecond = Second;
    timer_item->iType  = CTimer::eInterval;
    }

  timer_item->strContents      = ResponseText;
  timer_item->bEnabled         = (Flags & eEnabled) != 0;
  timer_item->bOneShot         = (Flags & eOneShot) != 0;
  timer_item->bTemporary       = (Flags & eTemporary) != 0;
  timer_item->bActiveWhenClosed = (Flags & eActiveWhenClosed) != 0;
  timer_item->strProcedure     = ScriptName;
  timer_item->strLabel         = TimerName;
  timer_item->dispid           = dispid;

  if (Flags & eTimerSpeedWalk)
    timer_item->iSendTo = eSendToSpeedwalk;
  else if (Flags & eTimerNote)
    timer_item->iSendTo = eSendToOutput;

  ResetOneTimer (timer_item);

  SortTimers ();

	return eOK;
}

long CMUSHclientDoc::DeleteTimer(LPCTSTR TimerName) 
{
CString strTimerName = TimerName;
CTimer * timer_item;

  // trim spaces from name, make lower-case
  CheckObjectName (strTimerName, false);

  if (!GetTimerMap ().Lookup (strTimerName, timer_item))
    return eTimerNotFound;

  // can't if executing a script
  if (timer_item->bExecutingScript)
    return eItemInUse;

  // the timer seems to exist - delete its pointer
  delete timer_item;

  // now delete its entry
  if (!GetTimerMap ().RemoveKey (strTimerName))
    return eTimerNotFound;

  SortTimers ();

  if (!m_CurrentPlugin) // plugin mods don't really count
    SetModifiedFlag (TRUE);   // document has changed
  return eOK;
}

long CMUSHclientDoc::IsTimer(LPCTSTR TimerName) 
{
CString strTimerName = TimerName;
CTimer * timer_item;

  // trim spaces from name, make lower-case
  CheckObjectName (strTimerName, false);

  if (!GetTimerMap ().Lookup (strTimerName, timer_item))
    return eTimerNotFound;

	return eOK;
}

VARIANT CMUSHclientDoc::GetTimerList() 
{
  COleSafeArray sa;   // for wildcard list

  CString strTimerName;
  CTimer * timer_item;
  long iCount = 0;
  POSITION pos;

  iCount = GetTimerMap ().GetCount ();

  if (iCount) // cannot create empty array dimension
    {
    sa.CreateOneDim (VT_VARIANT, iCount);
  
    for (iCount = 0, pos = GetTimerMap ().GetStartPosition(); pos; iCount++)
      {
      GetTimerMap ().GetNextAssoc (pos, strTimerName, timer_item);

      // the array must be a bloody array of variants, or VBscript kicks up
      COleVariant v (strTimerName);
      sa.PutElement (&iCount, &v);
      }      // end of looping through each timer
    } // end of having at least one

	return sa.Detach ();
}

long CMUSHclientDoc::GetTimer(LPCTSTR TimerName, 
                              VARIANT FAR* Hour, 
                              VARIANT FAR* Minute, 
                              VARIANT FAR* Second, 
                              VARIANT FAR* ResponseText, 
                              VARIANT FAR* Flags, 
                              VARIANT FAR* ScriptName) 
{
CString strTimerName = TimerName;
CTimer * timer_item;

  // trim spaces from name, make lower-case
  CheckObjectName (strTimerName, false);

  if (!GetTimerMap ().Lookup (strTimerName, timer_item))
    return eTimerNotFound;

  if (timer_item->iType == CTimer::eAtTime)
    {
    SetUpVariantShort  (*Hour, timer_item->iAtHour);
    SetUpVariantShort  (*Minute, timer_item->iAtMinute);
    SetUpVariantDouble  (*Second, timer_item->fAtSecond);
    }
  else
    {
    SetUpVariantShort  (*Hour, timer_item->iEveryHour);
    SetUpVariantShort  (*Minute, timer_item->iEveryMinute);
    SetUpVariantDouble  (*Second, timer_item->fEverySecond);
    }

  SetUpVariantString (*ResponseText, timer_item->strContents);
  SetUpVariantString (*ScriptName, timer_item->strProcedure);

  short iFlags = 0;

  if (timer_item->bEnabled) 
    iFlags |= eEnabled;
  if (timer_item->bOneShot) 
    iFlags |= eOneShot;
  if (timer_item->iSendTo == eSendToSpeedwalk) 
    iFlags |= eTimerSpeedWalk;
  if (timer_item->iSendTo == eSendToOutput) 
    iFlags |= eTimerNote;
  if (timer_item->bActiveWhenClosed) 
    iFlags |= eActiveWhenClosed;
  
  SetUpVariantShort  (*Flags, iFlags);

  return eOK;
}

VARIANT CMUSHclientDoc::GetTimerInfo(LPCTSTR TimerName, short InfoType) 
{
CString strTimerName = TimerName;
CTimer * timer_item;

	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_NULL;

  // trim spaces from name, make lower-case
  CheckObjectName (strTimerName, false);

  vaResult.vt = VT_EMPTY;

  // see if timer exists, if not return EMPTY
  if (!GetTimerMap ().Lookup (strTimerName, timer_item))
	  return vaResult;

  switch (InfoType)
    {
    case   1: if (timer_item->iType == CTimer::eAtTime)
                SetUpVariantShort  (vaResult, timer_item->iAtHour); 
              else
                SetUpVariantShort  (vaResult, timer_item->iEveryHour); 
              break;
    case   2: if (timer_item->iType == CTimer::eAtTime)
                SetUpVariantShort  (vaResult, timer_item->iAtMinute); 
              else
                SetUpVariantShort  (vaResult, timer_item->iEveryMinute); 
              break;
    case   3: if (timer_item->iType == CTimer::eAtTime)
                SetUpVariantDouble  (vaResult, timer_item->fAtSecond); 
              else
                SetUpVariantDouble  (vaResult, timer_item->fEverySecond); 
              break;
    case   4: SetUpVariantString (vaResult, timer_item->strContents); break;
    case   5: SetUpVariantString (vaResult, timer_item->strProcedure); break;
    case   6: SetUpVariantBool   (vaResult, timer_item->bEnabled); break;
    case   7: SetUpVariantBool   (vaResult, timer_item->bOneShot); break;
    case   8: SetUpVariantBool   (vaResult, timer_item->iType == CTimer::eAtTime); break;
    case   9: SetUpVariantLong   (vaResult, timer_item->nInvocationCount); break;
    case  10: SetUpVariantLong   (vaResult, timer_item->nMatched); break;
    case  11: 
      if (timer_item->tWhenFired.GetTime ())     // only if non-zero, otherwise return empty
        SetUpVariantDate   (vaResult, COleDateTime (timer_item->tWhenFired.GetTime ())); 
      break;
    case  12: 
      if (timer_item->tFireTime.GetTime ())     // only if non-zero, otherwise return empty
        SetUpVariantDate   (vaResult, COleDateTime (timer_item->tFireTime.GetTime ())); 
      break;
    case  13: 
      {
      CmcDateTime tDue = CmcDateTime (timer_item->tFireTime.GetTime ());
      CmcDateTime tNow = CmcDateTime::GetTimeNow ();
      if (tDue < tNow)
         SetUpVariantDouble   (vaResult, 0);  // due immediately
      else
        {
        CmcDateTimeSpan ts = tDue - tNow;
        SetUpVariantDouble   (vaResult, ts.GetTotalSeconds ());  // how many seconds to go
        }
      }
      break;
    case  14: SetUpVariantBool   (vaResult, timer_item->bTemporary); break;
    case  15: SetUpVariantBool   (vaResult, timer_item->iSendTo == eSendToSpeedwalk); break;
    case  16: SetUpVariantBool   (vaResult, timer_item->iSendTo == eSendToOutput); break;
    case  17: SetUpVariantBool   (vaResult, timer_item->bActiveWhenClosed); break;
    case  18: SetUpVariantBool   (vaResult, timer_item->bIncluded); break;
    case  19: SetUpVariantString (vaResult, timer_item->strGroup); break;
    case  20: SetUpVariantLong   (vaResult, timer_item->iSendTo); break;
    case  21: SetUpVariantLong   (vaResult, timer_item->iUserOption); break;
    case  22: SetUpVariantString (vaResult, timer_item->strLabel); break;
    case  23: SetUpVariantBool   (vaResult, timer_item->bOmitFromOutput); break;
    case  24: SetUpVariantBool   (vaResult, timer_item->bOmitFromLog); break;
    case  25: SetUpVariantBool   (vaResult, timer_item->bExecutingScript); break;
    case  26: SetUpVariantBool   (vaResult, timer_item->dispid != DISPID_UNKNOWN); break;

    default:
      vaResult.vt = VT_NULL;
      break;

    } // end of switch

  return vaResult;
}

long CMUSHclientDoc::GetUniqueNumber() 
{
  return (long) (App.GetUniqueNumber () & 0x7FFFFFFF);
}


long CMUSHclientDoc::Queue(LPCTSTR Message, BOOL Echo) 
{
  if (m_iConnectPhase != eConnectConnectedToMud)
    return eWorldClosed;             

  // cannot change what we are sending in OnPluginSent
  if (m_bPluginProcessingSent)
    return eItemInUse;

  SendMsg (Message, Echo != 0, true, false);
	return eOK;
}

long CMUSHclientDoc::DiscardQueue() 
{
long iCount = m_QueuedCommandsList.GetCount ();
     
  m_QueuedCommandsList.RemoveAll ();	
  ShowQueuedCommands ();    // update status line

	return iCount;
}

short CMUSHclientDoc::GetSpeedWalkDelay() 
{
	return m_iSpeedWalkDelay;
}

void CMUSHclientDoc::SetSpeedWalkDelay(short nNewValue) 
{
  if (m_pTimerWnd)
    m_pTimerWnd->ChangeTimerRate (nNewValue);
  m_iSpeedWalkDelay = nNewValue;
}


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

}

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
}


void CMUSHclientDoc::OnInputDiscardqueuedcommands() 
{
  m_QueuedCommandsList.RemoveAll ();	
  ShowQueuedCommands ();    // update status line
	
}

void CMUSHclientDoc::OnUpdateInputDiscardqueuedcommands(CCmdUI* pCmdUI) 
{
  DoFixMenus (pCmdUI);  // remove accelerators from menus
  pCmdUI->SetText (TFormat ("&Discard %i Queued Command%s\tCtrl+D",
                    PLURAL (m_QueuedCommandsList.GetCount ())));
  	
  pCmdUI->Enable (!m_QueuedCommandsList.IsEmpty ());
	
}



void CMUSHclientDoc::ShowQueuedCommands (void)
  {

  // show previous status line
  if (m_QueuedCommandsList.IsEmpty ())
    {
    ShowStatusLine (true);    // show it now
    return;    
    }

  CString strQueued = "Queued: ";
  const int MAX_SHOWN = 50;

  CString str;
  CString strLastDir;

  int iCount = 0;
  POSITION pos;

  for (pos = m_QueuedCommandsList.GetHeadPosition (); 
      pos && strQueued.GetLength () < MAX_SHOWN; )
    {
    // get next direction from list
    str = m_QueuedCommandsList.GetNext (pos).Mid (1);
    
    // empty lines look a bit silly
    if (str.IsEmpty ())
      continue;

    // if same as before, count them
    if (str == strLastDir)
      iCount++;
    else
      {
      // add to string
      // if direction is not a single char (eg. ne) then we must put it in brackets
      if (strLastDir.GetLength () > 1)
        strLastDir = "(" + strLastDir + ")";
      if (iCount == 1)
        strQueued += strLastDir + " ";
      else if (iCount > 1)
        strQueued += CFormat ("%i%s ", iCount, (LPCTSTR) strLastDir);

      strLastDir = str;
      iCount = 1;
      }   // end of a direction change                                

    }   // end of processing each command

  // output final command
  // if direction is not a single char (eg. ne) then we must put it in brackets
  if (strLastDir.GetLength () > 1)
    strLastDir = "(" + strLastDir + ")";
  if (iCount == 1)
    strQueued += strLastDir + " ";
  else if (iCount > 1)
    strQueued += CFormat ("%i%s ", iCount, (LPCTSTR) strLastDir);

  if (pos)
    strQueued += " ...";

  Frame.SetStatusMessageNow (strQueued);
  m_tStatusDisplayed = CTime::GetCurrentTime ();
  }


void CMUSHclientDoc::OnMappingChanged() 
{
	// TODO: Add notification handler code

}

void CMUSHclientDoc::OnRemoveMapReversesChanged() 
{
	// TODO: Add notification handler code

}

long CMUSHclientDoc::AddToMapper(LPCTSTR Direction, LPCTSTR Reverse) 
{
  // look for bad characters
  if (_tcspbrk (Direction, "{}()/\\"))  
    return eBadMapItem;

  if (_tcspbrk (Reverse, "{}()/\\"))  
    return eBadMapItem;

  // can't both be empty
  if (Direction [0] == 0 && Reverse [0] == 0)
    return eBadMapItem;

  CString strDirection;

  strDirection = Direction;
  strDirection += "/";
  strDirection += Reverse;

  m_strMapList.AddTail (strDirection);
	
  DrawMappingStatusLine ();    // update status line

  return eOK;
}


long CMUSHclientDoc::AddMapperComment(LPCTSTR Comment) 
{
  // look for bad characters
  if (_tcspbrk (Comment, "{}()/\\"))  
    return eBadMapItem;

  CString strDirection;

  strDirection = "{";
  strDirection += Comment;
  strDirection += "}";

  m_strMapList.AddTail (strDirection);
	
  DrawMappingStatusLine ();    // update status line

  return eOK;
}


long CMUSHclientDoc::GetMappingCount() 
{
	return m_strMapList.GetCount ();
}

VARIANT CMUSHclientDoc::GetMappingItem(long Item) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_NULL;

  if (Item < 0 || Item >= m_strMapList.GetCount ())
    return vaResult;

  POSITION pos = m_strMapList.FindIndex (Item);

  if (pos == NULL)
    return vaResult;

  CString strItem = m_strMapList.GetAt (pos);

  SetUpVariantString (vaResult, strItem);

	return vaResult;
}

VARIANT CMUSHclientDoc::GetMappingString() 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  CString str = CalculateSpeedWalkString ();

  SetUpVariantString (vaResult, str);

	return vaResult;
}


long CMUSHclientDoc::DeleteLastMapItem() 
{

  if (m_strMapList.IsEmpty ())
    return eNoMapItems;

  m_strMapList.RemoveTail ();

  DrawMappingStatusLine ();    // update status line

	return eOK;
}

long CMUSHclientDoc::DeleteAllMapItems() 
{
  if (m_strMapList.IsEmpty ())
    return eNoMapItems;

  m_strMapList.RemoveAll ();

  DrawMappingStatusLine ();    // update status line

	return eOK;
}

long CMUSHclientDoc::GetSentBytes() 
{
	return (long) m_nBytesOut;
}

long CMUSHclientDoc::GetReceivedBytes() 
{
	return (long) m_nBytesIn;
}

long CMUSHclientDoc::GetConnectDuration() 
{

  if (m_iConnectPhase != eConnectConnectedToMud)
    return 0;

  CTimeSpan ts = CTime::GetCurrentTime() - m_tConnectTime;
  
	return ts.GetTotalSeconds ();
}

double CMUSHclientDoc::GetScriptTime() 
{
  // time taken to execute scripts
  if (App.m_iCounterFrequency <= 0)
    return 0.0;

  double   elapsed_time;
  elapsed_time = ((double) m_iScriptTimeTaken) / 
                 ((double) App.m_iCounterFrequency);

	return elapsed_time;
}


BSTR CMUSHclientDoc::WorldAddress() 
{
	CString strResult = m_server;

	return strResult.AllocSysString();
}

long CMUSHclientDoc::WorldPort() 
{

	return m_port;
}


void CMUSHclientDoc::DeleteCommandHistory() 
{

  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	  {
		  CSendView* pmyView = (CSendView*)pView;

      // OK, do it ...
	    pmyView->m_msgList.RemoveAll ();
      pmyView->m_HistoryPosition = NULL;
      pmyView->m_inputcount = 0;
      pmyView->m_HistoryFindInfo.m_pFindPosition = NULL;
      pmyView->m_HistoryFindInfo.m_nCurrentLine = 0;
      pmyView->m_HistoryFindInfo.m_bAgain = FALSE;  
      pmyView->m_strPartialCommand.Empty ();
      pmyView->m_last_command.Empty ();

      }	  // end of being a CSendView
    }   // end of loop through views

}


void CMUSHclientDoc::DeleteOutput() 
{
  ClearOutput ();
}


// reset MXP
void CMUSHclientDoc::Reset() 
{
MXP_Off ();
}

// set pause to true or false for all views
void CMUSHclientDoc::Pause(BOOL Flag) 
{
  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
  	  {
		  CMUSHView* pmyView = (CMUSHView*)pView;
      pmyView->m_freeze = Flag != 0;
	    }	  // end of being a CMUSHView
    }   // end of loop through views


}


LPDISPATCH CMUSHclientDoc::Open(LPCTSTR FileName) 
{

//CDocument * pDoc = App.m_pWorldDocTemplate->OpenDocumentFile (FileName);
CDocument * pDoc = App.OpenDocumentFile (FileName);

  if (pDoc)
      return pDoc->GetIDispatch (TRUE);

	return NULL;
}


// world.GetLineInfo (LineNumber, InfoType) - gets details about the line
//                                     returns "EMPTY" if line out of range
//                                     returns "NULL" if bad info type
//   1: text of line
//   2: length of text
//   3: true if newline, false if not
//   4: true if world.note
//   5: true if player input
//   6: true if line logged
//   7: true if bookmarked
//   8: true if horizontal rule (<hr>)
//   9: date/time line arrived
//  10: actual line number (not line number in buffer)
//  11: count of style runs

VARIANT CMUSHclientDoc::GetLineInfo(long LineNumber, short InfoType) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;

  // check line exists
  if (LineNumber <= 0 || LineNumber > m_LineList.GetCount ())
    return vaResult;

  // get pointer to line in question

CLine * pLine = m_LineList.GetAt (GetLinePosition (LineNumber - 1));

  switch (InfoType)
    {
    case   1: SetUpVariantString (vaResult, CString (pLine->text, pLine->len)); break;
    case   2: SetUpVariantShort  (vaResult, pLine->len); break;
    case   3: SetUpVariantBool   (vaResult, pLine->hard_return); break;
    case   4: SetUpVariantBool   (vaResult, (pLine->flags & COMMENT) != 0); break;
    case   5: SetUpVariantBool   (vaResult, (pLine->flags & USER_INPUT) != 0); break;
    case   6: SetUpVariantBool   (vaResult, (pLine->flags & LOG_LINE) != 0); break;
    case   7: SetUpVariantBool   (vaResult, (pLine->flags & BOOKMARK) != 0); break;
    case   8: SetUpVariantBool   (vaResult, (pLine->flags & HORIZ_RULE) != 0); break;
    case   9: SetUpVariantDate   (vaResult, COleDateTime (pLine->m_theTime.GetTime ())); break;
    case  10: SetUpVariantLong   (vaResult, pLine->m_nLineNumber); break;
    case  11: SetUpVariantLong   (vaResult, pLine->styleList.GetCount ()); break;
    case  12: 
      {
      // high-performance timer
      double ticks = (double) pLine->m_lineHighPerformanceTime.QuadPart / (double) App.m_iCounterFrequency;
      SetUpVariantDouble (vaResult, ticks);
      }
      break;

    default:
      vaResult.vt = VT_NULL;
      break;

    } // end of switch

	return vaResult;
} // end of CMUSHclientDoc::GetLineInfo

// world.GetLinesInBufferCount 
//
//  returns actual number of lines in output buffer (not count of lines received)

long CMUSHclientDoc::GetLinesInBufferCount() 
{
	return m_LineList.GetCount ();
}


// world.GetStyleInfo (LineNumber, StyleNumber, InfoType) - gets details about the style
//                                     returns "EMPTY" if line or style number out of range
//                                     returns "NULL" if bad info type
//  1: text of style
//  2: length of style run
//  3: starting column of style
//  4: action type - 0=none, 1=send to mud, 2=hyperlink, 3=prompt
//  5: action   (eg. what to send)
//  6: hint     (what to show)
//  7: variable (variable to set)
//  8: true if bold
//  9: true if underlined
// 10: true if blinking
// 11: true if inverse
// 12: true if changed by trigger from original
// 13: true if start of a tag (action is tag name)
// 14: foreground (text) colour in RGB
// 15: background colour in RGB


VARIANT CMUSHclientDoc::GetStyleInfo(long LineNumber, long StyleNumber, short InfoType) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;

  // check line exists
  if (LineNumber <= 0 || LineNumber > m_LineList.GetCount ())
    return vaResult;

  // get pointer to line in question

CLine * pLine = m_LineList.GetAt (GetLinePosition (LineNumber - 1));

  // check style exists
  if (StyleNumber <= 0 || StyleNumber > pLine->styleList.GetCount ())
    return vaResult;

CStyle * pStyle = NULL;
POSITION pos;
int iCol = 0;
int iCount = 1;

  // search for it sequentially so we know the column number, 
  // so we can get its text
  for (pos = pLine->styleList.GetHeadPosition(); pos; iCount++)
    {
    pStyle = pLine->styleList.GetNext (pos);
    if (iCount == StyleNumber)
      break;  // found right one
    
    if (!pos)
      return vaResult;

    iCol += pStyle->iLength; // new column

    } // end of looping looking for it

CString strAction, strHint, strVariable;
CAction * pAction = pStyle ? pStyle->pAction : NULL;

COLORREF colour1,
         colour2;

  switch (InfoType)
    {
    case   1: 
      {
      CString strText = CString (pLine->text, pLine->len);
      SetUpVariantString (vaResult, strText.Mid (iCol, pStyle->iLength)); 
      }
      break;
    case   2: SetUpVariantShort  (vaResult, pStyle->iLength); 
      break;
    case   3: SetUpVariantShort  (vaResult, iCol + 1); 
      break;
    case   4: 
      {
      int iAction = 0;
      switch (pStyle->iFlags & ACTIONTYPE)
        {
        case ACTION_NONE:       iAction = 0; break;
        case ACTION_SEND:       iAction = 1; break;
        case ACTION_HYPERLINK:  iAction = 2; break;
        case ACTION_PROMPT:     iAction = 3; break;
        } // end of switch

      SetUpVariantShort  (vaResult, iAction);
      }
      break;

    case 5:
      if (pAction)
        SetUpVariantString (vaResult,  pAction->m_strAction);
       else
        SetUpVariantString (vaResult,  "");
      break;

    case 6:
      if (pAction)
        SetUpVariantString (vaResult,  pAction->m_strHint);
       else
        SetUpVariantString (vaResult,  "");
      break;

    case 7:
      if (pAction)
        SetUpVariantString (vaResult,  pAction->m_strVariable);
       else
        SetUpVariantString (vaResult,  "");
      break;

    case  8: SetUpVariantBool   (vaResult, (pStyle->iFlags & HILITE) != 0); 
      break;
    case  9: SetUpVariantBool   (vaResult, (pStyle->iFlags & UNDERLINE) != 0); 
      break;
    case 10: SetUpVariantBool   (vaResult, (pStyle->iFlags & BLINK) != 0); 
      break;
    case 11: SetUpVariantBool   (vaResult, (pStyle->iFlags & INVERSE) != 0); 
      break;
    case 12: SetUpVariantBool   (vaResult, (pStyle->iFlags & CHANGED) != 0); 
      break;
    case 13: SetUpVariantBool   (vaResult, (pStyle->iFlags & START_TAG) != 0); 
      break;

    case 14:
          GetStyleRGB (pStyle, colour1, colour2);
          SetUpVariantLong  (vaResult, colour1); 
          break;

    case 15:
          GetStyleRGB (pStyle, colour1, colour2);
          SetUpVariantLong  (vaResult, colour2); 
          break;

    default:
      vaResult.vt = VT_NULL;
      break;

    } // end of switch


	return vaResult;
}


long CMUSHclientDoc::GetSelectionStartLine() 
{
  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
  	  {
		  CMUSHView* pmyView = (CMUSHView*)pView;
      long startcol,
           endcol;

      CLine * pStartLine,
            * pEndLine;

      pStartLine = m_LineList.GetAt (GetLinePosition (pmyView->m_selstart_line));
      pEndLine   = m_LineList.GetAt (GetLinePosition (pmyView->m_selend_line));

      startcol  = MIN (pmyView->m_selstart_col, pStartLine->len);
      endcol    = MIN (pmyView->m_selend_col, pEndLine->len);

      // return 0 if no selection
      if (!(pmyView->m_selend_line > pmyView->m_selstart_line || 
                  (pmyView->m_selend_line == pmyView->m_selstart_line && 
                   endcol > startcol)))
        return 0;
      return pmyView->m_selstart_line + 1;
	    }	  // end of being a CMUSHView
    }   // end of loop through views


	return 0;
}

long CMUSHclientDoc::GetSelectionEndLine() 
{
  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
  	  {
		  CMUSHView* pmyView = (CMUSHView*)pView;
      long startcol,
           endcol;

      CLine * pStartLine,
            * pEndLine;

      pStartLine = m_LineList.GetAt (GetLinePosition (pmyView->m_selstart_line));
      pEndLine   = m_LineList.GetAt (GetLinePosition (pmyView->m_selend_line));

      startcol  = MIN (pmyView->m_selstart_col, pStartLine->len);
      endcol    = MIN (pmyView->m_selend_col, pEndLine->len);

      // return 0 if no selection
      if (!(pmyView->m_selend_line > pmyView->m_selstart_line || 
                  (pmyView->m_selend_line == pmyView->m_selstart_line && 
                   endcol > startcol)))
        return 0;

      return pmyView->m_selend_line + 1;
	    }	  // end of being a CMUSHView
    }   // end of loop through views

	return 0;
}



long CMUSHclientDoc::GetSelectionStartColumn() 
{
  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
  	  {
		  CMUSHView* pmyView = (CMUSHView*)pView;
      long startcol,
           endcol;

      CLine * pStartLine,
            * pEndLine;

      pStartLine = m_LineList.GetAt (GetLinePosition (pmyView->m_selstart_line));
      pEndLine   = m_LineList.GetAt (GetLinePosition (pmyView->m_selend_line));

      startcol  = MIN (pmyView->m_selstart_col, pStartLine->len);
      endcol    = MIN (pmyView->m_selend_col, pEndLine->len);

      // return 0 if no selection
      if (!(pmyView->m_selend_line > pmyView->m_selstart_line || 
                  (pmyView->m_selend_line == pmyView->m_selstart_line && 
                   endcol > startcol)))
        return 0;

      return startcol + 1;
	    }	  // end of being a CMUSHView
    }   // end of loop through views

	return 0;
}

long CMUSHclientDoc::GetSelectionEndColumn() 
{
  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
  	  {
		  CMUSHView* pmyView = (CMUSHView*)pView;
      long startcol,
           endcol;

      CLine * pStartLine,
            * pEndLine;

      pStartLine = m_LineList.GetAt (GetLinePosition (pmyView->m_selstart_line));
      pEndLine   = m_LineList.GetAt (GetLinePosition (pmyView->m_selend_line));

      startcol  = MIN (pmyView->m_selstart_col, pStartLine->len);
      endcol    = MIN (pmyView->m_selend_col, pEndLine->len);

      // return 0 if no selection
      if (!(pmyView->m_selend_line > pmyView->m_selstart_line || 
                  (pmyView->m_selend_line == pmyView->m_selstart_line && 
                   endcol > startcol)))
        return 0;

      return endcol + 1;
	    }	  // end of being a CMUSHView
    }   // end of loop through views

	return 0;
}

// world.GetCommandList - returns a variant array which is a list of "count" recent commands

VARIANT CMUSHclientDoc::GetCommandList(long Count) 
{
  COleSafeArray sa;   // for command list

  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	  {
		  CSendView* pmyView = (CSendView*)pView;

      POSITION pos;
      long iCount;
  
      // put the commands into the array
      if (!pmyView->m_msgList.IsEmpty ())    // cannot create empty dimension
        {
        if (Count <= 0)
          Count = pmyView->m_msgList.GetCount ();    // zero means all
        else
          Count = MIN (Count, pmyView->m_msgList.GetCount ());  // don't exceed what we have

        sa.CreateOneDim (VT_VARIANT, Count);

        for (iCount = 0, pos = pmyView->m_msgList.GetTailPosition(); 
            pos && iCount < Count; 
            )
          {
          CString strLine = pmyView->m_msgList.GetPrev (pos);

          // the array must be a bloody array of variants, or VBscript kicks up
          COleVariant v (strLine);
          sa.PutElement (&iCount, &v);
          iCount++;
          }      // end of looping through each command
        } // end of having at least one

	    }	  // end of being a CSendView
    }   // end of loop through views

	return sa.Detach ();
}

// world.PushCommand - pushes the command in the command window onto the command
//                     stack and then blanks it out
//  returns command that was pushed

BSTR CMUSHclientDoc::PushCommand() 
{
	CString strCommand;

  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	  {
		  CSendView* pmyView = (CSendView*)pView;

      // find what the command is
      pmyView->GetEditCtrl().GetWindowText (strCommand);

      // save the echo flag
      bool bSaveNoEcho = m_bNoEcho;
      m_bNoEcho = false;  // make sure we save it

      // remember it
      pmyView->AddToCommandHistory (strCommand);

      // restore the command
      m_bNoEcho = bSaveNoEcho;

      pmyView->GetEditCtrl().SetSel (0, -1);   // select all
      pmyView->GetEditCtrl().ReplaceSel ("", TRUE);   // blank it out
      pmyView->NotifyPluginCommandChanged ();

      }	  // end of being a CSendView
    }   // end of loop through views

	return strCommand.AllocSysString();
}

// world.SelectCommand - selects the command in the command window

void CMUSHclientDoc::SelectCommand() 
{
  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	  {
		  CSendView* pmyView = (CSendView*)pView;

      pmyView->GetEditCtrl().SetSel (0, -1);   // select all

      }	  // end of being a CSendView
    }   // end of loop through views

}

// world.GetCommand - gets the command in the command window

BSTR CMUSHclientDoc::GetCommand() 
{
	CString strCommand;

  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	  {
		  CSendView* pmyView = (CSendView*)pView;

      // find what the command is
      pmyView->GetEditCtrl().GetWindowText (strCommand);

      }	  // end of being a CSendView
    }   // end of loop through views


	return strCommand.AllocSysString();
}

// world.DoAfter - does a command after n seconds
//   eg. world.DoAfter 10, "go west"

long CMUSHclientDoc::DoAfter(double Seconds, LPCTSTR SendText) 
{
  return DoAfterSpecial (Seconds, SendText, eSendToWorld);
}

// world.DoAfterSpeedWalk - does a speedwalk after n seconds
//   eg. world.DoAfterSpeedWalk 10, "4n 5w"

long CMUSHclientDoc::DoAfterSpeedWalk(double Seconds, LPCTSTR SendText) 
{
  return DoAfterSpecial (Seconds, SendText, eSendToSpeedwalk);
}


// world.DoAfterNote - does a note after n seconds
//   eg. world.DoAfterNote 10, "tick"

long CMUSHclientDoc::DoAfterNote(double Seconds, LPCTSTR NoteText) 
{
  return DoAfterSpecial (Seconds, NoteText, eSendToOutput);
}

// world.GetQueue - returns a variant array which is a list of queued commands

VARIANT CMUSHclientDoc::GetQueue() 
{
  COleSafeArray sa;   // for command list

POSITION pos;
long iCount;

  // put the queued commands into the array
  if (!m_QueuedCommandsList.IsEmpty ())    // cannot create empty dimension
    {

    sa.CreateOneDim (VT_VARIANT, m_QueuedCommandsList.GetCount ());

    for (iCount = 0, pos = m_QueuedCommandsList.GetHeadPosition(); 
        pos; 
        )
      {
      CString strLine = m_QueuedCommandsList.GetNext (pos);

      // the array must be a bloody array of variants, or VBscript kicks up
      CString s = strLine.Mid (1);
      COleVariant v (s);  // drop echo flag
      sa.PutElement (&iCount, &v);
      iCount++;
      }      // end of looping through each command
    } // end of having at least one

	return sa.Detach ();
}


long CMUSHclientDoc::DeleteTemporaryTriggers() 
{
long iCount = 0;
POSITION pos;
CString strTriggerName;
CTrigger * trigger_item;

  for (pos = GetTriggerMap ().GetStartPosition(); pos; )
    {
    GetTriggerMap ().GetNextAssoc (pos, strTriggerName, trigger_item);
    if (trigger_item->bTemporary)
      {
      delete trigger_item;
      GetTriggerMap ().RemoveKey (strTriggerName);
      iCount++;
      }
    }   // end of deleting triggers

  SortTriggers ();

	return iCount;
}

long CMUSHclientDoc::DeleteTemporaryAliases() 
{
long iCount = 0;
POSITION pos;
CString strAliasName;
CAlias * alias_item;

  for (pos = GetAliasMap ().GetStartPosition(); pos; )
    {
    GetAliasMap ().GetNextAssoc (pos, strAliasName, alias_item);
    if (alias_item->bTemporary)
      {
      delete alias_item;
      GetAliasMap ().RemoveKey (strAliasName);
      iCount++;
      }
    }   // end of deleting aliases

  SortAliases ();

	return iCount;
}

long CMUSHclientDoc::DeleteTemporaryTimers() 
{
long iCount = 0;
POSITION pos;
CString strTimerName;
CTimer * timer_item;

  for (pos = GetTimerMap ().GetStartPosition(); pos; )
    {
    GetTimerMap ().GetNextAssoc (pos, strTimerName, timer_item);
    if (timer_item->bTemporary)
      {
      delete timer_item;
      GetTimerMap ().RemoveKey (strTimerName);
      iCount++;
      }
    }   // end of deleting timers

  if (iCount)
    SortTimers ();

	return iCount;
}

tInfoTypeMapping InfoTypes [] =
 
   {

//   (strings - configuration)

  { 1, "Server name" },
  { 2, "World name" },
  { 3, "Character name" },
  { 4, "Send to world - file preamble" },
  { 5, "Send to world - file postamble" },
  { 6, "Send to world - line preamble" },
  { 7, "Send to world - line postamble" },
  { 8, "World notes" },
  { 9, "Sound on new activity" },
 { 10, "Scripts editor" },
 { 11, "Log file preamble" },
 { 12, "Log file postamble" },
 { 13, "Log file line preamble - player input" },
 { 14, "Log file line preamble - notes" },
 { 15, "Log file line preamble - MUD output" },
 { 16, "Log file line postamble - player input" },
 { 17, "Log file line postamble - notes" },
 { 18, "Log file line postamble - MUD output" },
 { 19, "Speed Walk Filler" },
 { 20, "Output window font name" },
 { 21, "Speed walk prefix" },
 { 22, "Text sent on connecting" },
 { 23, "Input font name" },
 { 24, "Paste to world - file preamble" },
 { 25, "Paste to world - file postamble" },
 { 26, "Paste to world - line preamble" },
 { 27, "Paste to world - line postamble" },
 { 28, "Scripting language" },
 { 29, "Script function for world open" },
 { 30, "Script function for world close" },
 { 31, "Script function for world connect" },
 { 32, "Script function for world disconnect" },
 { 31, "Script function for world get focus" },
 { 32, "Script function for world lose focus" },
 { 35, "Script file name" },
 { 36, "Scripting prefix" },
 { 37, "Auto-say string" },
 { 38, "Auto-say override" },
 { 39, "Tab-completion defaults" },
 { 40, "Auto-log file name" },
 { 41, "Recall window - line preamble" },
 { 42, "Terminal ID (telnet negotiation)" },
 { 43, "Mapping failure message" },
 { 44, "Script function for MXP starting up" },
 { 45, "Script function for MXP closing down" },
 { 46, "Script function for MXP error" },
 { 47, "Script function for MXP tag open" },
 { 48, "Script function for MXP tag close" },
 { 49, "Script function for MXP variable set" },
 { 50, "Sound to play for beeps" },

// (strings - calculated at runtime)
 
 { 51, "Current log file name" },
 { 52, "Last \"immediate\" script expression" },
 { 53, "Current status line message" },
 { 54, "World file pathname" },
 { 55, "World title" },
 { 56, "MUSHclient application path name" },
 { 57, "World files default path (directory)" },
 { 58, "Log files default path (directory)" },
 { 59, "Script files default path (directory)" },
 { 60, "Plugin files default path (directory)" },
 { 61, "World IP address" },
 { 62, "Proxy server IP address" },
 { 63, "Host name (name of this PC)" },
 { 64, "Current directory path" },
 { 65, "Script function for world save" },
 { 66, "MUSHclient application directory" },
 { 67, "World file directory" },
 { 68, "Startup directory" },
 { 69, "Translation file" },
 { 70, "Locale" },
 { 71, "Fixed Pitch Font" },
 { 72, "MUSHclient version" },
 { 73, "MUSHclient compilation date/time" },
 { 74, "Sounds directory" },
 { 75, "Last telnet subnegotiation string received" },
 { 76, "Special font pathname" },
 { 77, "OS Version - service pack" },
 { 78, "Foreground image filename" },
 { 79, "Background image filename" },
 { 80, "LibPNG version number" },
 { 81, "LibPNG header version string" },
 { 82, "Preferences database pathname" },
 { 83, "SQLite3 version" },
 { 84, "File browsing directory" },
 { 85, "State files path (directory)" },


 
// (booleans - calculated at runtime)
 
{ 101, "No Echo" },
{ 102, "Debug incoming packets" },
{ 103, "Decompressing" },
{ 104, "MXP active" },
{ 105, "Pueblo active" },
{ 106, "World close flag" },
{ 107, "Currently-connecting" },
{ 108, "OK-to-disconnect" },
{ 109, "Trace" },
{ 110, "Script file changed" },
{ 111, "World file is modified" },
{ 112, "Automapper active" },
{ 113, "World is active" },
{ 114, "Output window paused" },
{ 115, "Localization active" },
{ 118, "Variables have changed" },
{ 119, "Script engine active" },
{ 120, "Scroll bar visible" },


// (numbers (longs) - calculated at runtime)

{ 201, "Total lines received" },
{ 202, "Lines received but not yet seen (new lines)" },
{ 203, "Total lines sent" },
{ 204, "Packets received" },
{ 205, "Packets sent" },
{ 206, "Total uncompressed bytes received" },
{ 207, "Total compresssed bytes received" },
{ 208, "MCCP protocol in use (0=none, 1 or 2)" },
{ 209, "MXP error count" },
{ 210, "MXP tags received" },
{ 211, "MXP entities received" },
{ 212, "Output font height" },
{ 213, "Output font width" },
{ 214, "Input font height" },
{ 215, "Input font width" },
{ 216, "Total bytes received" },
{ 217, "Total bytes sent" },
{ 218, "Count of variables" },
{ 219, "Count of triggers" },
{ 220, "Count of timers" },
{ 221, "Count of aliases" },
{ 222, "Count of queued commands" },
{ 223, "Count of mapper items" },
{ 224, "Count of lines in output window" },
{ 225, "Count of custom MXP elements" },
{ 226, "Count of custom MXP entities" },
{ 227, "Connect phase" },
{ 228, "World IP address (as long)" },
{ 229, "Proxy server IP address (as long)" },
{ 230, "Script execution depth" },
{ 231, "Log file size" },
{ 232, "High-performance timer" },        // double
{ 233, "Trigger execution time" },        // double
{ 234, "Alias execution time" },          // double
{ 235, "Number of world windows" },
{ 236, "Command window start selection column" },
{ 237, "Command window end selection column" },
{ 238, "World window display flags" },
{ 239, "Source of current action" },
{ 240, "Average character width in output window" },
{ 241, "Font height in output window" },
{ 242, "Number of lines received with bad UTF-8 in them" },
{ 243, "Fixed Pitch Font Size" },
{ 244, "Count of triggers evaluated" },
{ 245, "Count of triggers matched" },
{ 246, "Count of aliases evaluated" },
{ 247, "Count of aliases matched" },
{ 248, "Count of timers fired" },
{ 249, "Main frame window height" },
{ 250, "Main frame window width" },
{ 251, "Main toolbar window height" },
{ 252, "Main toolbar window width" },
{ 253, "Game toolbar window height" },
{ 254, "Game toolbar window width" },
{ 255, "Activity toolbar window height" },
{ 256, "Activity toolbar window width" },
{ 257, "Info bar window height" },
{ 258, "Info bar window width" },
{ 259, "Status bar window height" },
{ 260, "Status bar window width" },
{ 261, "World window non-client height" },
{ 262, "World window non-client width" },
{ 263, "World window client height" },
{ 264, "World window client width" },
{ 265, "OS Version - major" },
{ 266, "OS Version - minor" },
{ 267, "OS Version - build number" },
{ 268, "OS Version - platform ID" },
{ 269, "Foreground image mode" },
{ 270, "Background image mode" },
{ 271, "Background colour" },
{ 272, "Text rectangle - left" },
{ 273, "Text rectangle - top" },
{ 274, "Text rectangle - right" },
{ 275, "Text rectangle - bottom" },
{ 276, "Text rectangle - border offset" },
{ 277, "Text rectangle - border width" },
{ 278, "Text rectangle - outside colour" },
{ 279, "Text rectangle - outside style" },
{ 280, "Output window client height" },
{ 281, "Output window client width" },
{ 282, "Text rectangle - border colour" },
{ 283, "Mouse position - X" },
{ 284, "Mouse position - Y" },
{ 285, "Output window available" },
{ 286, "Triggers matched this session" },
{ 287, "Aliases matched this session" },
{ 288, "Timers fired this session" }, 
{ 289, "Last line with IAC/GA or IAC/EOR in it" },
{ 290, "Actual text rectangle - left" },
{ 291, "Actual text rectangle - top" },
{ 292, "Actual text rectangle - right" },
{ 293, "Actual text rectangle - bottom" },
{ 294, "State of keyboard modifiers" },
{ 295, "Times output window redrawn" },
{ 296, "Output window scroll bar position" },
   

// (dates - calculated at runtime)

{ 301, "Time connected" },
{ 302, "Time log file was last flushed to disk" },
{ 303, "When script file was last modified" },
{ 304, "Time now" },
{ 305, "When client started executing" },
{ 306, "When this world was created/opened" },


 { 0, "" }, // end of table marker

 };

static void GetWindowWidth (CWnd * pWnd, VARIANT & vaResult, const bool client = false )
  {
  RECT rect;
  if (pWnd->m_hWnd)
    {
    if (client)
      pWnd->GetClientRect(&rect);
    else
      {
      pWnd->GetWindowRect(&rect);
      pWnd->ScreenToClient (&rect);
      }
    }
  else
    rect.right = -1;
  VariantClear (&vaResult);
  vaResult.vt = VT_I4;
  vaResult.lVal = rect.right; 
  } // end of GetWindowWidth

static void GetWindowHeight (CWnd * pWnd, VARIANT & vaResult, const bool client = false )
  {
  RECT rect;
  rect.left = 0;
  rect.top = 0;
  rect.right = 0;
  rect.bottom = 0;

  if (pWnd->m_hWnd)
    {
    if (client)
      pWnd->GetClientRect(&rect);
    else
      {
      pWnd->GetWindowRect(&rect);
      pWnd->ScreenToClient (&rect);
      }
    }
  else
    rect.right = -1;
  VariantClear (&vaResult);
  vaResult.vt = VT_I4;
  vaResult.lVal = rect.bottom; 
  }   // end of GetWindowHeight

OSVERSIONINFO ver;

static void GetOSVersion (void)
  {
  // see which OS we are using
  memset(&ver, 0, sizeof(ver));
  ver.dwOSVersionInfoSize = sizeof(ver);
  ::GetVersionEx (&ver);
  } // GetOSVersion

// helper function
CMUSHView * CMUSHclientDoc::GetFirstOutputWindow ()
  {
  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
      return  (CMUSHView*)pView;
    }   // end of loop through views

  return NULL;      // not found
  }

VARIANT CMUSHclientDoc::GetInfo(long InfoType) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);


  vaResult.vt = VT_EMPTY;   // in case dates are zero

  switch (InfoType)
    {
    case    1: SetUpVariantString (vaResult, m_server); break;
    case    2: SetUpVariantString (vaResult, m_mush_name); break;
    case    3: SetUpVariantString (vaResult, m_name); break;
    case    4: SetUpVariantString (vaResult, m_file_preamble); break;
    case    5: SetUpVariantString (vaResult, m_file_postamble); break;
    case    6: SetUpVariantString (vaResult, m_line_preamble); break;
    case    7: SetUpVariantString (vaResult, m_line_postamble); break;
    case    8: SetUpVariantString (vaResult, m_notes); break;
    case    9: SetUpVariantString (vaResult, m_new_activity_sound); break;
    case   10: SetUpVariantString (vaResult, m_strScriptEditor); break;

    case   11: SetUpVariantString (vaResult, m_strLogFilePreamble); break;
    case   12: SetUpVariantString (vaResult, m_strLogFilePostamble); break;
    case   13: SetUpVariantString (vaResult, m_strLogLinePreambleInput); break;
    case   14: SetUpVariantString (vaResult, m_strLogLinePreambleNotes); break;
    case   15: SetUpVariantString (vaResult, m_strLogLinePreambleOutput); break;
    case   16: SetUpVariantString (vaResult, m_strLogLinePostambleInput); break;
    case   17: SetUpVariantString (vaResult, m_strLogLinePostambleNotes); break;
    case   18: SetUpVariantString (vaResult, m_strLogLinePostambleOutput); break;
    case   19: SetUpVariantString (vaResult, m_strSpeedWalkFiller); break;
    case   20: SetUpVariantString (vaResult, m_font_name); break;

    case   21: SetUpVariantString (vaResult, m_speed_walk_prefix); break;
    case   22: SetUpVariantString (vaResult, m_connect_text); break;
    case   23: SetUpVariantString (vaResult, m_input_font_name); break;
    case   24: SetUpVariantString (vaResult, m_paste_postamble); break;
    case   25: SetUpVariantString (vaResult, m_paste_preamble); break;
    case   26: SetUpVariantString (vaResult, m_pasteline_postamble); break;
    case   27: SetUpVariantString (vaResult, m_pasteline_preamble); break;
    case   28: SetUpVariantString (vaResult, m_strLanguage); break;
    case   29: SetUpVariantString (vaResult, m_strWorldOpen); break;
    case   30: SetUpVariantString (vaResult, m_strWorldClose); break;

    case   31: SetUpVariantString (vaResult, m_strWorldConnect); break;
    case   32: SetUpVariantString (vaResult, m_strWorldDisconnect); break;
    case   33: SetUpVariantString (vaResult, m_strWorldGetFocus); break;
    case   34: SetUpVariantString (vaResult, m_strWorldLoseFocus); break;
    case   35: SetUpVariantString (vaResult, m_strScriptFilename); break;
    case   36: SetUpVariantString (vaResult, m_strScriptPrefix); break;
    case   37: SetUpVariantString (vaResult, m_strAutoSayString); break;
    case   38: SetUpVariantString (vaResult, m_strOverridePrefix); break;
    case   39: SetUpVariantString (vaResult, m_strTabCompletionDefaults); break;
    case   40: SetUpVariantString (vaResult, m_strAutoLogFileName); break;

    case   41: SetUpVariantString (vaResult, m_strRecallLinePreamble); break;
    case   42: SetUpVariantString (vaResult, m_strTerminalIdentification); break;
    case   43: SetUpVariantString (vaResult, m_strMappingFailure); break;
    case   44: SetUpVariantString (vaResult, m_strOnMXP_Start); break;
    case   45: SetUpVariantString (vaResult, m_strOnMXP_Stop); break;
    case   46: SetUpVariantString (vaResult, m_strOnMXP_Error); break;
    case   47: SetUpVariantString (vaResult, m_strOnMXP_OpenTag); break;
    case   48: SetUpVariantString (vaResult, m_strOnMXP_CloseTag); break;
    case   49: SetUpVariantString (vaResult, m_strOnMXP_SetVariable); break;
    case   50: SetUpVariantString (vaResult, m_strBeepSound); break;

    case   51: SetUpVariantString (vaResult, m_logfile_name); break;
    case   52: SetUpVariantString (vaResult, m_strLastImmediateExpression); break;
    case   53: SetUpVariantString (vaResult, m_strStatusMessage); break;
    case   54: SetUpVariantString (vaResult, GetPathName ()); break;
    case   55: SetUpVariantString (vaResult, GetTitle ()); break;

    case   56: SetUpVariantString (vaResult, App.m_strMUSHclientFileName); break;
    case   57: SetUpVariantString (vaResult, App.m_strDefaultWorldFileDirectory); break;
    case   58: SetUpVariantString (vaResult, App.m_strDefaultLogFileDirectory); break;
    case   59:   // scripts are in the execution directory at present
              {
              char fullfilename[MAX_PATH];
              char * p;
              GetModuleFileName (NULL, fullfilename, sizeof (fullfilename));
              p = strrchr (fullfilename, '\\');
              if (p)
                *p = 0;

              SetUpVariantString (vaResult, fullfilename); 
              break;
              }

    case   60: SetUpVariantString (vaResult, App.m_strPluginsDirectory); break;
    case   61: SetUpVariantString (vaResult, inet_ntoa (m_sockAddr.sin_addr)); break;
    case   62: SetUpVariantString (vaResult, inet_ntoa (m_ProxyAddr.sin_addr)); break;
    case   63:
              {
               char szHostName[256];

                if( gethostname(szHostName, sizeof szHostName) == 0 )
                   SetUpVariantString (vaResult, szHostName);

              }
              break;

    case   64:      // current directory
              {
               char * p = _getcwd (NULL, 1024);
               if (p)
                 {
                 CString strPath (p);
                 free (p);
                 if (strPath.Right (1) != "\\")
                   strPath += "\\"; // append trailing backslash
                 SetUpVariantString (vaResult, strPath);
                 }

              }
              break;

    case   65: SetUpVariantString (vaResult, m_strWorldSave); break;
    case   66: SetUpVariantString (vaResult, ExtractDirectory (App.m_strMUSHclientFileName)); break;
    case   67: SetUpVariantString (vaResult, ExtractDirectory (GetPathName ())); break;
    case   68: SetUpVariantString (vaResult, working_dir); break;
    case   69: SetUpVariantString (vaResult, App.m_strTranslatorFile); break;
    case   70: SetUpVariantString (vaResult, App.m_strLocale); break;
    case   71: SetUpVariantString (vaResult, App.m_strFixedPitchFont); break;
    case   72: SetUpVariantString (vaResult, MUSHCLIENT_VERSION); break;
    case   73: SetUpVariantString (vaResult, __DATE__ " " __TIME__); break;
    case   74: SetUpVariantString (vaResult, ExtractDirectory (App.m_strMUSHclientFileName) + "sounds\\"); break;
    case   75: SetUpVariantString (vaResult, m_IAC_subnegotiation_data.c_str ()); break;
    case   76: SetUpVariantString (vaResult, m_strSpecialFontName); break;

    case 77:
      {
      GetOSVersion ();
      SetUpVariantString (vaResult, ver.szCSDVersion);  
      break;
      }

    case   78: SetUpVariantString (vaResult, m_strForegroundImageName); break;
    case   79: SetUpVariantString (vaResult, m_strBackgroundImageName); break;
    case   80: SetUpVariantString (vaResult, PNG_LIBPNG_VER_STRING); break;
    case   81: SetUpVariantString (vaResult, PNG_HEADER_VERSION_STRING); break;
    case   82: SetUpVariantString (vaResult, App.m_PreferencesDatabaseName.c_str ()); break;
    case   83: SetUpVariantString (vaResult, sqlite3_libversion ()); break;
    case   84: SetUpVariantString (vaResult, file_browsing_dir); break;
    case   85: SetUpVariantString (vaResult, App.m_strDefaultStateFilesDirectory); break;


    case  101: SetUpVariantBool (vaResult, m_bNoEcho); break;
    case  102: SetUpVariantBool (vaResult, m_bDebugIncomingPackets); break;
    case  103: SetUpVariantBool (vaResult, m_bCompress); break;
    case  104: SetUpVariantBool (vaResult, m_bMXP); break;
    case  105: SetUpVariantBool (vaResult, m_bPuebloActive); break;
    case  106: SetUpVariantBool (vaResult, m_iConnectPhase != eConnectConnectedToMud); break;
    case  107: SetUpVariantBool (vaResult, m_iConnectPhase != eConnectNotConnected && // connecting
                                           m_iConnectPhase != eConnectConnectedToMud); break;
    case  108: SetUpVariantBool (vaResult, m_bDisconnectOK); break;
    case  109: SetUpVariantBool (vaResult, m_bTrace); break;
    case  110: SetUpVariantBool (vaResult, m_bInScriptFileChanged); break;
    case  111: SetUpVariantBool (vaResult, IsModified ()); break;
    case  112: SetUpVariantBool (vaResult, m_bMapping); break;
    case  113: SetUpVariantBool (vaResult, m_pActiveCommandView || m_pActiveOutputView); 
               break;
      
    case  114:   // current view frozen
               if (!(m_pActiveOutputView || m_pActiveCommandView)) 
                 SetUpVariantBool (vaResult, FALSE);
               else if (m_pActiveOutputView)
                 SetUpVariantBool (vaResult, m_pActiveOutputView->m_freeze);
               else 
                 SetUpVariantBool (vaResult, m_pActiveCommandView->m_topview->m_freeze);
               break;

    case  115: SetUpVariantBool (vaResult, App.m_Translator_Lua != NULL); break;
    case  118: SetUpVariantBool (vaResult, m_bVariablesChanged); break;
    case  119: SetUpVariantBool (vaResult, m_ScriptEngine != NULL); break;
    case  120: SetUpVariantBool (vaResult, m_bScrollBarWanted); break;


    case  201: SetUpVariantLong (vaResult, m_total_lines); break;
    case  202: SetUpVariantLong (vaResult, m_new_lines); break;
    case  203: SetUpVariantLong (vaResult, m_nTotalLinesSent); break;
    case  204: SetUpVariantLong (vaResult, (long) m_iInputPacketCount); break;
    case  205: SetUpVariantLong (vaResult, (long) m_iOutputPacketCount); break;
    case  206: SetUpVariantLong (vaResult, (long) m_nTotalUncompressed); break;
    case  207: SetUpVariantLong (vaResult, (long) m_nTotalCompressed); break;
    case  208: SetUpVariantLong (vaResult, m_iMCCP_type); break;
    case  209: SetUpVariantLong (vaResult, (long) m_iMXPerrors); break;
    case  210: SetUpVariantLong (vaResult, (long) m_iMXPtags); break;
      
    case  211: SetUpVariantLong (vaResult, (long) m_iMXPentities); break;
    case  212: SetUpVariantLong (vaResult, m_FontHeight); break;
    case  213: SetUpVariantLong (vaResult, m_FontWidth); break;
    case  214: SetUpVariantLong (vaResult, m_InputFontHeight); break;
    case  215: SetUpVariantLong (vaResult, m_InputFontWidth); break;
    case  216: SetUpVariantLong (vaResult, (long) m_nBytesIn); break;
    case  217: SetUpVariantLong (vaResult, (long) m_nBytesOut); break;
    case  218: SetUpVariantLong (vaResult, GetVariableMap ().GetCount ()); break;
    case  219: SetUpVariantLong (vaResult, GetTriggerMap ().GetCount ()); break;
    case  220: SetUpVariantLong (vaResult, GetTimerMap ().GetCount ()); break;
    case  221: SetUpVariantLong (vaResult, GetAliasMap ().GetCount ()); break;
    case  222: SetUpVariantLong (vaResult, m_QueuedCommandsList.GetCount ()); break;
    case  223: SetUpVariantLong (vaResult, m_strMapList.GetCount ()); break;
    case  224: SetUpVariantLong (vaResult, m_LineList.GetCount ()); break;
    case  225: SetUpVariantLong (vaResult, m_CustomElementMap.GetCount ()); break;
    case  226: SetUpVariantLong (vaResult, m_CustomEntityMap.GetCount ()); break;
    case  227: SetUpVariantLong (vaResult, m_iConnectPhase); break;
    case  228: SetUpVariantLong (vaResult, ntohl (m_sockAddr.sin_addr.s_addr)); break;
    case  229: SetUpVariantLong (vaResult, ntohl (m_ProxyAddr.sin_addr.s_addr)); break;
    case  230: SetUpVariantLong (vaResult, m_iExecutionDepth); break;
    case  231:  // log file size
      {
      if (m_logfile == NULL)
         SetUpVariantLong (vaResult, 0);  // no log file
      else
        {
        fpos_t pos;
        fgetpos (m_logfile, &pos);
        SetUpVariantLong (vaResult, pos);  // log file size
        } // end of log file

      }
      break;

    case 232:
      if (App.m_iCounterFrequency)
        {
        LARGE_INTEGER the_time;
        QueryPerformanceCounter (&the_time);
        double ticks = (double) the_time.QuadPart / (double) App.m_iCounterFrequency;
        SetUpVariantDouble (vaResult, ticks);
        }
      break;

    case 233:     // time taken doing triggers
      if (App.m_iCounterFrequency)
        {
        LONGLONG iTimeTaken = 0;
        CTrigger * pTrigger;
        POSITION pos;
        double   elapsed_time;
        CString strName;

        // count number of triggers matched
        for (pos = GetTriggerMap ().GetStartPosition(); pos; )
          {
          GetTriggerMap ().GetNextAssoc (pos, strName, pTrigger);
          // calculate time taken to execute triggers
          if (pTrigger->regexp)
            iTimeTaken += pTrigger->regexp->iTimeTaken;
          }

        elapsed_time = ((double) iTimeTaken) / 
                        ((double) App.m_iCounterFrequency);

        SetUpVariantDouble (vaResult, elapsed_time);
        }
      break;

    case 234:     // time taken doing aliases
      if (App.m_iCounterFrequency)
        {
        LONGLONG iTimeTaken = 0;
        CAlias * pAlias;
        POSITION pos;
        double   elapsed_time;
        CString strName;

        // count number of triggers matched
        for (pos = GetAliasMap ().GetStartPosition(); pos; )
          {
          GetAliasMap ().GetNextAssoc (pos, strName, pAlias);
          // calculate time taken to execute triggers
          if (pAlias->regexp)
            iTimeTaken += pAlias->regexp->iTimeTaken;
          }

        elapsed_time = ((double) iTimeTaken) / 
                        ((double) App.m_iCounterFrequency);

        SetUpVariantDouble (vaResult, elapsed_time);
        }
      break;

    case 235:
      {
      long count = 0;

      for(POSITION pos=GetFirstViewPosition();pos!=NULL;)
        {
        CView* pView = GetNextView(pos);

        if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
          count++;
        }   // end of loop

      SetUpVariantLong (vaResult, count);  // number of world windows open

      }   // end of 235
      break;

    case 236:   // command selection start column
      {
        for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	        {
	        CView* pView = GetNextView(pos);
	        
	        if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	        {
		        CSendView* pmyView = (CSendView*)pView;
            int nStartChar;
            int nEndChar;

              // find where cursor is
  
            pmyView->GetEditCtrl().GetSel(nStartChar, nEndChar);	
            SetUpVariantLong (vaResult, nStartChar + 1);  // start column
            break;
  
	          }	  // end of being a CSendView
          }   // end of loop through views
      }
      break;

    case 237:   // command selection end column
      {
        for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	        {
	        CView* pView = GetNextView(pos);
	        
	        if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	        {
		        CSendView* pmyView = (CSendView*)pView;
            int nStartChar;
            int nEndChar;

              // find where cursor is
  
            pmyView->GetEditCtrl().GetSel(nStartChar, nEndChar);
            if (nEndChar <= nStartChar)
              nEndChar = 0;  // zero indicates no text selected
            SetUpVariantLong (vaResult, nEndChar);  // end column
            break;
  
	          }	  // end of being a CSendView
          }   // end of loop through views
      }
      break;

    case 238:
      {
      WINDOWPLACEMENT wp;

      for(POSITION pos=GetFirstViewPosition();pos!=NULL;)
        {
        CView* pView = GetNextView(pos);

        if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
          {
          CSendView* pmyView = (CSendView*)pView;

          pmyView->GetParentFrame ()->GetWindowPlacement(&wp); 
          SetUpVariantLong (vaResult, wp.showCmd);  // window placement flags
          break;

          }	
        }


      }
      break;

    case 239:
          SetUpVariantShort (vaResult, m_iCurrentActionSource);  // see enum
          break;

    case 240:

        SetUpVariantLong (vaResult, m_FontWidth);  // average character width
        break;

    case 241:
        SetUpVariantLong (vaResult, m_FontHeight);  // character height
        break;

    case 242:
        SetUpVariantLong (vaResult, m_iUTF8ErrorCount);  // errors in UTF-8
        break;

    case 243:
        SetUpVariantLong (vaResult, App.m_iFixedPitchFontSize);  
        break;

    case 244:
        SetUpVariantLong (vaResult, m_iTriggersEvaluatedCount);  
        break;

    case 245:
        SetUpVariantLong (vaResult, m_iTriggersMatchedCount);  
        break;

    case 246:
        SetUpVariantLong (vaResult, m_iAliasesEvaluatedCount);  
        break;

    case 247:
        SetUpVariantLong (vaResult, m_iAliasesMatchedCount);  
        break;

    case 248:
        SetUpVariantLong (vaResult, m_iTimersFiredCount);  
        break;

    case 249:
      {
      RECT rect;
      Frame.GetClientRect(&rect);
      SetUpVariantLong (vaResult, rect.bottom);  
      }
      break;

    case 250:
      {
      RECT rect;
      Frame.GetClientRect(&rect);
      SetUpVariantLong (vaResult, rect.right);  
      }
      break;

    case 251: GetWindowHeight (&Frame.m_wndToolBar, vaResult); break;
    case 252: GetWindowWidth  (&Frame.m_wndToolBar, vaResult); break;
    case 253: GetWindowHeight (&Frame.m_wndGameToolBar, vaResult); break;
    case 254: GetWindowWidth  (&Frame.m_wndGameToolBar, vaResult); break;
    case 255: GetWindowHeight (&Frame.m_wndActivityToolBar, vaResult); break;
    case 256: GetWindowWidth  (&Frame.m_wndActivityToolBar, vaResult); break;
    case 257: GetWindowHeight (&Frame.m_wndInfoBar, vaResult); break;
    case 258: GetWindowWidth  (&Frame.m_wndInfoBar, vaResult); break;
    case 259: GetWindowHeight (&Frame.m_wndStatusBar, vaResult); break;
    case 260: GetWindowWidth  (&Frame.m_wndStatusBar, vaResult); break;
    case 261: 
            {
            CChildFrame * pFrame = GetChildFrame ();
            if (pFrame)
              GetWindowHeight (pFrame, vaResult); break;  // non-client height
            break;
            }

    case 262: 
            {
            CChildFrame * pFrame = GetChildFrame ();
            if (pFrame)
              GetWindowWidth  (pFrame, vaResult);   // non-client width
            break;
            }

    case 263: 
            {
            CChildFrame * pFrame = GetChildFrame ();
            if (pFrame)
              GetWindowHeight (pFrame, vaResult, true); break;  // client height
            break;
            }

    case 264: 
            {
            CChildFrame * pFrame = GetChildFrame ();
            if (pFrame)
              GetWindowWidth  (pFrame, vaResult, true);   // client width
            break;
            }

    case 265:
      {
      GetOSVersion ();
      SetUpVariantLong (vaResult, ver.dwMajorVersion);  
      break;
      }

    case 266:
      {
      GetOSVersion ();
      SetUpVariantLong (vaResult, ver.dwMinorVersion);  
      break;
      }

    case 267:
      {
      GetOSVersion ();
      SetUpVariantLong (vaResult, ver.dwBuildNumber);  
      break;
      }

    case 268:
      {
      GetOSVersion ();
      SetUpVariantLong (vaResult, ver.dwPlatformId);  
      break;
      }

    case 269: SetUpVariantLong (vaResult, m_iForegroundMode);  break;
    case 270: SetUpVariantLong (vaResult, m_iBackgroundMode);  break;
    case 271: SetUpVariantLong (vaResult, m_iBackgroundColour);  break;

    case 272: SetUpVariantLong (vaResult, m_TextRectangle.left);  break;
    case 273: SetUpVariantLong (vaResult, m_TextRectangle.top);  break;
    case 274: SetUpVariantLong (vaResult, m_TextRectangle.right);  break;
    case 275: SetUpVariantLong (vaResult, m_TextRectangle.bottom);  break;
    case 276: SetUpVariantLong (vaResult, m_TextRectangleBorderOffset);  break;
    case 277: SetUpVariantLong (vaResult, m_TextRectangleBorderWidth);  break;
    case 278: SetUpVariantLong (vaResult, m_TextRectangleOutsideFillColour);  break;
    case 279: SetUpVariantLong (vaResult, m_TextRectangleOutsideFillStyle);  break;

    case 280:
      {
      CMUSHView* pmyView = GetFirstOutputWindow ();
      if (pmyView) 
        {
        RECT rect;
        pmyView->GetClientRect(&rect);
        SetUpVariantLong (vaResult, rect.bottom);
        break;
        }
      }
      break;

    case 281:
      {
      CMUSHView* pmyView = GetFirstOutputWindow ();
      if (pmyView) 
        {
        RECT rect;
        pmyView->GetClientRect(&rect);
        SetUpVariantLong (vaResult, rect.right);
        break;
        }
      }
      break;

    case 282: SetUpVariantLong (vaResult, m_TextRectangleBorderColour);  break;

    case 283: SetUpVariantLong (vaResult, m_lastMousePosition.x);  break; //  last mouse x position
    case 284: SetUpVariantLong (vaResult, m_lastMousePosition.y);  break; //  last mouse y position
    case 285: SetUpVariantBool (vaResult, m_pCurrentLine != NULL); break; //  is output window available?

    case 286: SetUpVariantLong (vaResult, m_iTriggersMatchedThisSessionCount);  break; // Triggers matched this session
    case 287: SetUpVariantLong (vaResult, m_iAliasesMatchedThisSessionCount);   break; // Aliases matched this session   
    case 288: SetUpVariantLong (vaResult, m_iTimersFiredThisSessionCount);      break; // Timers fired this session      
    case 289: SetUpVariantLong (vaResult, m_last_line_with_IAC_GA);             break; // Last line number that had an IAC/GA in it     

    case 290:
      {
      CMUSHView* pmyView = GetFirstOutputWindow ();
      if (pmyView) 
        {
        RECT rect = pmyView->GetTextRectangle ();
        SetUpVariantLong (vaResult, rect.left);
        break;
        }
      }
      break;

    case 291:
      {
      CMUSHView* pmyView = GetFirstOutputWindow ();
      if (pmyView) 
        {
        RECT rect = pmyView->GetTextRectangle ();
        SetUpVariantLong (vaResult, rect.top);
        break;
        }
      }
      break;

    case 292:
      {
      CMUSHView* pmyView = GetFirstOutputWindow ();
      if (pmyView) 
        {
        RECT rect = pmyView->GetTextRectangle ();
        SetUpVariantLong (vaResult, rect.right);
        break;
        }
      }
      break;

    case 293:
      {
      CMUSHView* pmyView = GetFirstOutputWindow ();
      if (pmyView) 
        {
        RECT rect = pmyView->GetTextRectangle ();
        SetUpVariantLong (vaResult, rect.bottom);
        break;
        }
      }
      break;

    case 294: 
      {
      long result = 0;
      if (GetKeyState (VK_SHIFT)    & 0x8000)
        result |= 0x01;
      if (GetKeyState (VK_CONTROL)  & 0x8000)
        result |= 0x02;
      if (GetKeyState (VK_MENU)     & 0x8000)
        result |= 0x04;
      if (GetKeyState (VK_LSHIFT)   & 0x8000)
        result |= 0x08;
      if (GetKeyState (VK_RSHIFT)   & 0x8000)
        result |= 0x10;
      if (GetKeyState (VK_LCONTROL) & 0x8000)
        result |= 0x20;
      if (GetKeyState (VK_RCONTROL) & 0x8000)
        result |= 0x40;
      if (GetKeyState (VK_LMENU)    & 0x8000)
        result |= 0x80;
      if (GetKeyState (VK_RMENU)    & 0x8000)
        result |= 0x100;
      if (GetKeyState (VK_CAPITAL)  & 0x8000)
        result |= 0x200;
       if (GetKeyState (VK_NUMLOCK) & 0x8000)
        result |= 0x400;
      if (GetKeyState (VK_SCROLL)   & 0x8000)
        result |= 0x800;
      // 0x1000 unused for symmetry
      if (GetKeyState (VK_CAPITAL)  & 0x0001)
        result |= 0x2000;
       if (GetKeyState (VK_NUMLOCK) & 0x0001)
        result |= 0x4000;
      if (GetKeyState (VK_SCROLL)   & 0x0001)
        result |= 0x8000;

      if (GetKeyState (VK_LBUTTON)  & 0x8000)
        result |= 0x10000;
       if (GetKeyState (VK_RBUTTON) & 0x8000)
        result |= 0x20000;
      if (GetKeyState (VK_MBUTTON)   & 0x8000)
        result |= 0x40000;

      SetUpVariantLong (vaResult, result); 
      }
      break; // state of virtual keys    

    case 295:
      SetUpVariantLong (vaResult, m_iOutputWindowRedrawCount); 
      break; // state of virtual keys    

    case 296:
      {
      CMUSHView* pmyView = GetFirstOutputWindow ();
      if (pmyView) 
        {
        CPoint pt = pmyView->GetScrollPosition ();
        SetUpVariantLong (vaResult, pt.y);
        break;
        }
      }
      break;

    case  301: 
      if (m_tConnectTime.GetTime ())     // only if non-zero, otherwise return empty
        SetUpVariantDate (vaResult, COleDateTime (m_tConnectTime.GetTime ())); 
      break;
    case  302: 
      if (m_LastFlushTime.GetTime ())     // only if non-zero, otherwise return empty      
        SetUpVariantDate (vaResult, COleDateTime (m_LastFlushTime.GetTime ())); 
      break;
    case  303: 
      if (m_timeScriptFileMod.GetTime ())     // only if non-zero, otherwise return empty      
        SetUpVariantDate (vaResult, COleDateTime (m_timeScriptFileMod.GetTime ())); 
      break;      

    case  304: 
      SetUpVariantDate (vaResult, COleDateTime (CTime::GetCurrentTime().GetTime ())); 
      break;      

    case  305:
      SetUpVariantDate (vaResult, COleDateTime (App.m_whenClientStarted.GetTime ())); 
      break;

    case  306:
      SetUpVariantDate (vaResult, COleDateTime (m_whenWorldStarted.GetTime ())); 
      break;

    default:
      vaResult.vt = VT_NULL;
      break;

    } // end of switch

	return vaResult;
}  // GetInfo


BOOL CMUSHclientDoc::GetEchoInput() 
{
	return m_display_my_input != 0;
}

void CMUSHclientDoc::SetEchoInput(BOOL bNewValue) 
{
  m_display_my_input = bNewValue != 0;
}


long CMUSHclientDoc::ColourNameToRGB(LPCTSTR Name) 
{
COLORREF cValue;

  if (SetColour (Name, cValue)) 
    return -1;

	return cValue;
}

BSTR CMUSHclientDoc::RGBColourToName(long Colour) 
{
	CString strResult = ColourToName (Colour);

	return strResult.AllocSysString();
}

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
}

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
}


VARIANT CMUSHclientDoc::GetLoadedValue(LPCTSTR OptionName) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_NULL;    // default if not found

int i = GetOptionIndex (OptionName);

  if (i != -1)
    {
    if (m_CurrentPlugin &&
        (OptionsTable [i].iFlags & OPT_PLUGIN_CANNOT_READ))
    	return vaResult;  // not available to plugin
    SetUpVariantLong (vaResult, m_NumericConfiguration [i]->iValue);
    }  // end of found
  else
    {
    i = GetAlphaOptionIndex (OptionName);
    if (i != -1)
      {
      if (m_CurrentPlugin &&
          (AlphaOptionsTable [i].iFlags & OPT_PLUGIN_CANNOT_READ))
    	  return vaResult;  // not available to plugin
      SetUpVariantString (vaResult, m_AlphaConfiguration [i]->sValue);
      }    // end of found
    }
	
	return vaResult;
}

VARIANT CMUSHclientDoc::GetDefaultValue(LPCTSTR OptionName) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_NULL;    // default if not found

int i = GetOptionIndex (OptionName);

  if (i != -1)
    {
    if (m_CurrentPlugin &&
        (OptionsTable [i].iFlags & OPT_PLUGIN_CANNOT_READ))
    	return vaResult;  // not available to plugin
    SetUpVariantLong (vaResult, OptionsTable [i].iDefault);
    }  // end of found
  else
    {
    i = GetAlphaOptionIndex (OptionName);
    if (i != -1)
      {
      if (m_CurrentPlugin &&
          (AlphaOptionsTable [i].iFlags & OPT_PLUGIN_CANNOT_READ))
    	  return vaResult;  // not available to plugin
      SetUpVariantString (vaResult, AlphaOptionsTable [i].sDefault);
      }  // end of found
    }

	return vaResult;
}

VARIANT CMUSHclientDoc::GetCurrentValue(LPCTSTR OptionName) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_NULL;    // default if not found

int i = GetOptionIndex (OptionName);

  if (i != -1)
    {
    if (m_CurrentPlugin &&
        (OptionsTable [i].iFlags & OPT_PLUGIN_CANNOT_READ))
    	return vaResult;  // not available to plugin
    SetUpVariantLong (vaResult, GetOptionItem (i));
    }  // end of found
  else
    {
    i = GetAlphaOptionIndex (OptionName);
    if (i != -1)
      {
      if (m_CurrentPlugin &&
          (AlphaOptionsTable [i].iFlags & OPT_PLUGIN_CANNOT_READ))
    	  return vaResult;  // not available to plugin
      SetUpVariantString (vaResult, GetAlphaOptionItem (i));
      }  // end of found
    }

	return vaResult;
}


void CMUSHclientDoc::ColourNote(LPCTSTR TextColour, 
                                LPCTSTR BackgroundColour, 
                                LPCTSTR Text) 
{
  CString strMsg = Text;

  if (strMsg.Right (2) != ENDLINE)
    strMsg += ENDLINE;      // add a new line if necessary

  ColourTell (TextColour, BackgroundColour, strMsg);

}

void CMUSHclientDoc::ColourTell(LPCTSTR TextColour, LPCTSTR BackgroundColour, LPCTSTR Text) 
{

// save old colours
bool bOldNotesInRGB = m_bNotesInRGB;
COLORREF iOldNoteColourFore = m_iNoteColourFore;
COLORREF iOldNoteColourBack = m_iNoteColourBack;

// change colours
  NoteColourName(TextColour, BackgroundColour);

// do the tell
  Tell (Text);

// put the colours back
if (bOldNotesInRGB)
  {
  m_iNoteColourFore = iOldNoteColourFore;
  m_iNoteColourBack = iOldNoteColourBack;
  }
else  
  m_bNotesInRGB = false;

}


BSTR CMUSHclientDoc::CreateGUID() 
{
	CString strResult = GetGUID ();

	return strResult.AllocSysString();
}


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
}



BSTR CMUSHclientDoc::GetUniqueID() 
{
CString strResult = ::GetUniqueID ();

	return strResult.AllocSysString();
}

// gets our own plugin id

BSTR CMUSHclientDoc::GetPluginID() 
{
	CString strResult;

  if (m_CurrentPlugin)
    strResult = m_CurrentPlugin->m_strID;
  
  return strResult.AllocSysString();
}

// gets our own plugin name

BSTR CMUSHclientDoc::GetPluginName() 
{
	CString strResult;

  if (m_CurrentPlugin)
    strResult = m_CurrentPlugin->m_strName;

	return strResult.AllocSysString();
}

// get list of plugins

VARIANT CMUSHclientDoc::GetPluginList() 
{
  COleSafeArray sa;   // for variable list

  CString strVariableName;

  POSITION pos;
  long iCount;
  
  // put the plugins into the array
  if (!m_PluginList.IsEmpty ())    // cannot create empty dimension
    {
    sa.CreateOneDim (VT_VARIANT, m_PluginList.GetCount ());

    for (iCount = 0, pos = m_PluginList.GetHeadPosition(); pos; )
      {
      CPlugin * p = m_PluginList.GetNext (pos);

      // the array must be a bloody array of variants, or VBscript kicks up
      COleVariant v (p->m_strID);
      sa.PutElement (&iCount, &v);
      iCount++;
      }      // end of looping through each plugin
    } // end of having at least one

	return sa.Detach ();
}

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
      for (POSITION pos = m_PluginList.GetHeadPosition(); pos; )
        {
        CPlugin * p = m_PluginList.GetNext (pos);
        p->m_iLoadOrder = ++iCount;
        }      // end of looping through each plugin

      // now return the order of *this* one
      SetUpVariantLong   (vaResult, pPlugin->m_iLoadOrder); 
      }
      break;

    case  22: SetUpVariantDate   (vaResult, COleDateTime (pPlugin->m_tDateInstalled.GetTime ()));  break;
    case  23: SetUpVariantString (vaResult, pPlugin->m_strCallingPluginID); break;

    default:
      vaResult.vt = VT_NULL;
      break;

    } // end of switch

  return vaResult;
}

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
}    // end of LoadPlugin


long CMUSHclientDoc::ReloadPlugin(LPCTSTR PluginID) 
{

// first, find plugin by ID
CPlugin * pPlugin = GetPlugin (PluginID);

  // if not found, try to find by name
  if (pPlugin == NULL && strlen (PluginID) > 0)
    {
    // see if plugin exists in list of plugins for this document
    for (POSITION pos = m_PluginList.GetHeadPosition(); pos; )
      {
      pPlugin = m_PluginList.GetNext (pos);
      if (pPlugin->m_strName.CompareNoCase (PluginID) == 0)
        break;
      pPlugin = NULL;
      }      // end of looping through each plugins
    }    

  if (pPlugin == NULL)
    return eNoSuchPlugin;

  // cannot reload  ourselves
  if (pPlugin == m_CurrentPlugin)
    return eBadParameter;

  POSITION pos = m_PluginList.Find (pPlugin);

  if (!pos)
    return eNoSuchPlugin;

  CString strName = pPlugin->m_strSource;
  m_PluginList.RemoveAt (pos);  // remove from list
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
}  // end of ReloadPlugin

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

  // see if plugin exists in list of plugins for this document
  for (POSITION pos = m_PluginList.GetHeadPosition(); pos; )
    {
    CPlugin * pPlugin = m_PluginList.GetNext (pos);
    if (pPlugin->m_strID.CompareNoCase (PluginID) == 0)
      return pPlugin;
    }      // end of looping through each plugins

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
}

// get variable list for a particular plugin

VARIANT CMUSHclientDoc::GetPluginVariableList(LPCTSTR PluginID) 
{
GET_PLUGIN_STUFF (GetVariableList ())
}

// get trigger list for a particular plugin

VARIANT CMUSHclientDoc::GetPluginTriggerList(LPCTSTR PluginID) 
{
GET_PLUGIN_STUFF (GetTriggerList ())
}

// get alias list for a particular plugin

VARIANT CMUSHclientDoc::GetPluginAliasList(LPCTSTR PluginID) 
{
GET_PLUGIN_STUFF (GetAliasList ())
}

// get timer list for a particular plugin

VARIANT CMUSHclientDoc::GetPluginTimerList(LPCTSTR PluginID) 
{
GET_PLUGIN_STUFF (GetTimerList ())
}

// get trigger info for a particular plugin

VARIANT CMUSHclientDoc::GetPluginTriggerInfo(LPCTSTR PluginID, LPCTSTR TriggerName, short InfoType) 
{
GET_PLUGIN_STUFF (GetTriggerInfo (TriggerName, InfoType))
}

// get alias info for a particular plugin

VARIANT CMUSHclientDoc::GetPluginAliasInfo(LPCTSTR PluginID, LPCTSTR AliasName, short InfoType) 
{
GET_PLUGIN_STUFF (GetAliasInfo (AliasName, InfoType))
}

// get timer info for a particular plugin

VARIANT CMUSHclientDoc::GetPluginTimerInfo(LPCTSTR PluginID, LPCTSTR TimerName, short InfoType) 
{
GET_PLUGIN_STUFF (GetTimerInfo (TimerName, InfoType))
}

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
}

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
}

// is a plugin installed? eg
//  i = world.IsPluginInstalled ("982581e59ab42844527eec80")

BOOL CMUSHclientDoc::IsPluginInstalled(LPCTSTR PluginID) 
{
return GetPlugin (PluginID) != NULL;
}

// if I am a plugin, save my state

long CMUSHclientDoc::SaveState() 
{

  if (!m_CurrentPlugin)                            
	  return eNotAPlugin;                       

  if (m_CurrentPlugin->SaveState (true))
    return ePluginCouldNotSaveState;

  return eOK;
}   // end of SaveState


long CMUSHclientDoc::EnablePlugin(LPCTSTR PluginID, BOOL Enabled) 
{
CPlugin * pPlugin = GetPlugin (PluginID); 

  if (!pPlugin)                            
	  return eNoSuchPlugin;                       

  if (pPlugin->m_bEnabled == (Enabled != 0))
    return eOK;   // already same state

  pPlugin->m_bEnabled = Enabled != 0;

  if (pPlugin->m_bEnabled)
    pPlugin->ExecutePluginScript (ON_PLUGIN_ENABLE, pPlugin->m_dispid_plugin_enable); 
  else
    pPlugin->ExecutePluginScript (ON_PLUGIN_DISABLE, pPlugin->m_dispid_plugin_disable); 
  
  return eOK;
}   // end of EnablePlugin


// enable or disable a batch of trigers/timers/aliases

long CMUSHclientDoc::EnableGroup(LPCTSTR GroupName, BOOL Enabled) 
{

  long iCount = 0;

  iCount += EnableTriggerGroup (GroupName, Enabled);
  iCount += EnableAliasGroup   (GroupName, Enabled);
  iCount += EnableTimerGroup   (GroupName, Enabled);

	return iCount;
}   // end of EnableGroup


long CMUSHclientDoc::EnableTriggerGroup(LPCTSTR GroupName, BOOL Enabled) 
{
  CString strTriggerName;
  CTrigger * trigger_item;
  long iCount = 0;
  POSITION pos;
  
  // no group name, affect nothing
  if (strlen (GroupName) == 0)
    return 0;

  // do triggers
  for (pos = GetTriggerMap ().GetStartPosition(); pos; )
    {
    GetTriggerMap ().GetNextAssoc (pos, strTriggerName, trigger_item);
    if (trigger_item->strGroup == GroupName)
      {
      trigger_item->bEnabled = Enabled != 0;
      iCount++;
      }
    }   // end of triggers

  if (iCount)
    if (!m_CurrentPlugin) // plugin mods don't really count
      SetModifiedFlag (TRUE);   // document has changed

  return iCount;
}   // end of EnableTriggerGroup

long CMUSHclientDoc::EnableAliasGroup(LPCTSTR GroupName, BOOL Enabled) 
{
  CString strAliasName;
  CAlias * alias_item;
  long iCount = 0;
  POSITION pos;

  // no group name, affect nothing
  if (strlen (GroupName) == 0)
    return 0;

  // do aliases
  for (pos = GetAliasMap ().GetStartPosition(); pos; )
    {
    GetAliasMap ().GetNextAssoc (pos, strAliasName, alias_item);
    if (alias_item->strGroup == GroupName)
      {
      alias_item->bEnabled = Enabled != 0;
      iCount++;
      }
    }   // end of aliases

  if (iCount)
    if (!m_CurrentPlugin) // plugin mods don't really count
      SetModifiedFlag (TRUE);   // document has changed

  return iCount;

}   // end of EnableAliasGroup

long CMUSHclientDoc::EnableTimerGroup(LPCTSTR GroupName, BOOL Enabled) 
{
  CString strTimerName;
  CTimer * timer_item;
  long iCount = 0;
  POSITION pos;

  // no group name, affect nothing
  if (strlen (GroupName) == 0)
    return 0;

  // count timers
  for (pos = GetTimerMap ().GetStartPosition(); pos; )
    {
    GetTimerMap ().GetNextAssoc (pos, strTimerName, timer_item);
    if (timer_item->strGroup == GroupName)
      {
      timer_item->bEnabled = Enabled != 0;
      iCount++;
      }
    }   // end of timers

  if (iCount)
    if (!m_CurrentPlugin) // plugin mods don't really count
      SetModifiedFlag (TRUE);   // document has changed

  return iCount;

}   // end of EnableTimerGroup


void CMUSHclientDoc::ShowInfoBar(BOOL Visible) 
{
  if (Frame.m_wndInfoBar.m_hWnd)
  	Frame.ShowControlBar(&Frame.m_wndInfoBar, Visible != 0, FALSE);
} // end of ShowInfoBar


void CMUSHclientDoc::InfoClear() 
{
  if (!Frame.m_wndInfoBar.m_hWnd)
    return;

  CRichEditCtrl * pRichEdit = (CRichEditCtrl *) Frame.m_wndInfoBar.GetDlgItem (IDC_INFOTEXT);

  // back to default colour, font etc.
  pRichEdit->SetDefaultCharFormat (Frame.m_defaultInfoBarFormat);

  // select all
  pRichEdit->SetSel (0, -1);

  // delete everything
  pRichEdit->ReplaceSel ("");


} // end of InfoClear


void CMUSHclientDoc::Info(LPCTSTR Message) 
{
  if (!Frame.m_wndInfoBar.m_hWnd)
    return;

  CRichEditCtrl * pRichEdit = (CRichEditCtrl *) Frame.m_wndInfoBar.GetDlgItem (IDC_INFOTEXT);

  pRichEdit->ReplaceSel (Message);

}   // end of Info

/*

  Style bits:

    1 = bold
    2 = italic
    4 = underline
    8 = strikeout

*/


void CMUSHclientDoc::InfoFont(LPCTSTR FontName, short Size, short Style) 
{
  if (!Frame.m_wndInfoBar.m_hWnd)
    return;


CHARFORMAT cf;

 CRichEditCtrl * pRichEdit = (CRichEditCtrl *) Frame.m_wndInfoBar.GetDlgItem (IDC_INFOTEXT);

 memset (&cf, 0, sizeof (cf));
 cf.cbSize = sizeof (cf);

 // font name provided?
 if (strlen (FontName) > 0)
   {
   cf.dwMask = CFM_FACE;
   strncpy (cf.szFaceName, FontName, sizeof (cf.szFaceName) - 1);

   // do this so fonts like webdings, marlett and so on work properly
   // without it, such fonts would take (say) "g" and substitute a "g" from
   // the default font, not show the "g" equivalent in the selected font

   cf.dwMask |= CFM_CHARSET;
   cf.bCharSet = SYMBOL_CHARSET;

   }

 // size provided?
 if (Size > 0)
   {
   cf.dwMask |= CFM_SIZE;
   cf.yHeight  = Size * 20; // points
   }

 // style

 cf.dwMask |= CFM_BOLD | CFM_ITALIC | CFM_STRIKEOUT | CFM_UNDERLINE;

 if (Style & 1)   // bold
  cf.dwEffects |= CFE_BOLD;

 if (Style & 2)   // italic
  cf.dwEffects |= CFE_ITALIC;

 if (Style & 4)   // underline
  cf.dwEffects |= CFE_UNDERLINE;

 if (Style & 8)   // strikeout
  cf.dwEffects |= CFE_STRIKEOUT;

 pRichEdit->SetSelectionCharFormat (cf);


}   // end of InfoFont


void CMUSHclientDoc::InfoColour(LPCTSTR Name) 
{
  if (!Frame.m_wndInfoBar.m_hWnd)
    return;

COLORREF cValue;

CHARFORMAT cf;

 CRichEditCtrl * pRichEdit = (CRichEditCtrl *) Frame.m_wndInfoBar.GetDlgItem (IDC_INFOTEXT);

 // do foreground colour if possible
 if (SetColour (Name, cValue)) 
   return;

 memset (&cf, 0, sizeof (cf));
 cf.cbSize = sizeof (cf);
 cf.dwMask = CFM_COLOR;
 cf.crTextColor = cValue;
 pRichEdit->SetSelectionCharFormat (cf);

} // end of InfoColour

void CMUSHclientDoc::InfoBackground(LPCTSTR Name) 
{
  if (!Frame.m_wndInfoBar.m_hWnd)
    return;

  COLORREF cValue;

 CRichEditCtrl * pRichEdit = (CRichEditCtrl *) Frame.m_wndInfoBar.GetDlgItem (IDC_INFOTEXT);

 // do background colour if possible
 if (SetColour (Name, cValue)) 
   return;

 pRichEdit->SetBackgroundColor (FALSE, cValue);

} // end of InfoBackground

/*  DIDN'T WORK PROPERLY

void CMUSHclientDoc::InfoHeight(long Pixels) 
{
  if (!Frame.m_wndInfoBar.m_hWnd)
    return;

  CRichEditCtrl * pRichEdit = (CRichEditCtrl *) Frame.m_wndInfoBar.GetDlgItem (IDC_INFOTEXT);

 Frame.m_wndInfoBar.GetParent ()->SetWindowPos (NULL, 0, 0, 3000, Pixels, 
    SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED );

  pRichEdit->SetWindowPos (NULL, 0, 0, 3000, Pixels, 
    SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED );

} // end of InfoHeight

*/

long CMUSHclientDoc::DeleteGroup(LPCTSTR GroupName) 
{
  long iCount = 0;

  iCount += DeleteTriggerGroup (GroupName);
  iCount += DeleteAliasGroup   (GroupName);
  iCount += DeleteTimerGroup   (GroupName);

	return iCount;
}   // end of DeleteGroup

long CMUSHclientDoc::DeleteTriggerGroup(LPCTSTR GroupName) 
{
  CString strTriggerName;
  CTrigger * trigger_item;
  POSITION pos;
  
  // no group name, affect nothing
  if (strlen (GroupName) == 0)
    return 0;

  vector<string> vToDelete;

  // do triggers
  for (pos = GetTriggerMap ().GetStartPosition(); pos; )
    {
    GetTriggerMap ().GetNextAssoc (pos, strTriggerName, trigger_item);
    if (trigger_item->strGroup == GroupName)
      {

      // can't if executing a script
      if (trigger_item->bExecutingScript)
        continue;

      // delete its pointer
      delete trigger_item;

      // remember to delete from trigger map
      vToDelete.push_back ((LPCTSTR) strTriggerName);

      }
    }   // end of triggers

  // now delete from map, do it this way in case deleting whilst looping throws things out
  for (vector<string>::const_iterator it = vToDelete.begin (); 
       it != vToDelete.end ();
       it++)
      GetTriggerMap ().RemoveKey (it->c_str ());
 
  if (!vToDelete.empty ())
    {
    SortTriggers ();
    if (!m_CurrentPlugin) // plugin mods don't really count
      SetModifiedFlag (TRUE);   // document has changed
    }

  return vToDelete.size ();
}   // end of DeleteTriggerGroup

long CMUSHclientDoc::DeleteAliasGroup(LPCTSTR GroupName) 
{
  CString strAliasName;
  CAlias * alias_item;
  POSITION pos;

  // no group name, affect nothing
  if (strlen (GroupName) == 0)
    return 0;

  vector<string> vToDelete;

  // do aliases
  for (pos = GetAliasMap ().GetStartPosition(); pos; )
    {
    GetAliasMap ().GetNextAssoc (pos, strAliasName, alias_item);
    if (alias_item->strGroup == GroupName)
      {

      // can't if executing a script
      if (alias_item->bExecutingScript)
        continue;

      delete alias_item;

      // remember to delete from alias map
      vToDelete.push_back ((LPCTSTR) strAliasName);

      }
    }   // end of aliases

  // now delete from map, do it this way in case deleting whilst looping throws things out
  for (vector<string>::const_iterator it = vToDelete.begin (); 
       it != vToDelete.end ();
       it++)
      GetAliasMap ().RemoveKey (it->c_str ());

  if (!vToDelete.empty ())
    {
    SortAliases ();
    if (!m_CurrentPlugin) // plugin mods don't really count
      SetModifiedFlag (TRUE);   // document has changed
    }

  return vToDelete.size ();

}   // end of DeleteAliasGroup

long CMUSHclientDoc::DeleteTimerGroup(LPCTSTR GroupName) 
{
  CString strTimerName;
  CTimer * timer_item;
  POSITION pos;

  // no group name, affect nothing
  if (strlen (GroupName) == 0)
    return 0;

  vector<string> vToDelete;

  // count timers
  for (pos = GetTimerMap ().GetStartPosition(); pos; )
    {
    GetTimerMap ().GetNextAssoc (pos, strTimerName, timer_item);
    if (timer_item->strGroup == GroupName)
      {

      // can't if executing a script
      if (timer_item->bExecutingScript)
        continue;

      delete timer_item;

      // remember to delete from timer map
      vToDelete.push_back ((LPCTSTR) strTimerName);
      }
    }   // end of timers

  // now delete from map, do it this way in case deleting whilst looping throws things out
  for (vector<string>::const_iterator it = vToDelete.begin (); 
       it != vToDelete.end ();
       it++)
      GetTimerMap ().RemoveKey (it->c_str ());

  if (!vToDelete.empty ())
    {
    SortTimers ();
    if (!m_CurrentPlugin) // plugin mods don't really count
      SetModifiedFlag (TRUE);   // document has changed
    }

  return vToDelete.size ();
}   // end of DeleteTimerGroup


#define TO(arg) offsetof (CTrigger, arg), sizeof (((CTrigger *)NULL)->arg)

//-----config name ----------------  default ---variable name ---- min, max, flags ---
tConfigurationNumericOption TriggerOptionsTable [] = {

// numbers
  
{"clipboard_arg",        0,             TO(iClipboardArg), 0, 10},       
{"colour_change_type",   0,             TO(iColourChangeType), TRIGGER_COLOUR_CHANGE_BOTH, TRIGGER_COLOUR_CHANGE_BACKGROUND},               
// MAX_CUSTOM + 1 to allow for "other" colour        
{"custom_colour",        0,             TO(colour), 0, MAX_CUSTOM + 1, OPT_CUSTOM_COLOUR},               
{"lines_to_match",       0,             TO(iLinesToMatch), 0, MAX_RECENT_LINES},               
{"match_style",          0,             TO(iMatch), 0, 0xFFFF},               
{"new_style",            0,             TO(iStyle), 0, 7},               
{"other_text_colour",    0,             TO(iOtherForeground), 0, 0xFFFFFF, OPT_RGB_COLOUR },               
{"other_back_colour",    0,             TO(iOtherBackground), 0, 0xFFFFFF, OPT_RGB_COLOUR },               
{"send_to",              eSendToWorld,  TO(iSendTo), 0, eSendToLast - 1},               
{"sequence",             0,             TO(iSequence), 0, 10000},               
{"user",                 0,             TO(iUserOption), LONG_MIN, LONG_MAX},               

// boolean

{"enabled",              false, TO(bEnabled)},               
{"expand_variables",     false, TO(bExpandVariables)},               
{"ignore_case",          false, TO(ignore_case), 0, 0, OPT_CANNOT_WRITE},               
{"keep_evaluating",      false, TO(bKeepEvaluating)},               
{"multi_line",           false, TO(bMultiLine)},               
{"omit_from_log",        false, TO(omit_from_log)},               
{"omit_from_output",     false, TO(bOmitFromOutput)},               
{"regexp",               false, TO(bRegexp), 0, 0, OPT_CANNOT_WRITE},               
{"repeat",               false, TO(bRepeat)},               
{"sound_if_inactive",    false, TO(bSoundIfInactive)},               
{"lowercase_wildcard",   false, TO(bLowercaseWildcard)},
{"temporary",            false, TO(bTemporary)},
{"one_shot",             false, TO(bOneShot)},               
               

{NULL}   // end of table marker            

  };  // end of TriggerOptionsTable 

#define TA(arg) offsetof (CTrigger, arg)
                    
// first  optional argument: multiline flag
// second optional argument: preserve spaces flag (defaults to true for multiline)

tConfigurationAlphaOption TriggerAlphaOptionsTable  [] =
{
// CAN'T CHANGE LABEL {"name",         "",         TA(strLabel)},
{"group",        "",         TA(strGroup)},
{"match",        "",         TA(trigger)},
{"script",       "",         TA(strProcedure)},
{"sound",        "",         TA(sound_to_play)},
{"send",         "",         TA(contents), OPT_MULTLINE},
{"variable",     "",         TA(strVariable)},

{NULL}  // end of table marker
};    // end of TriggerAlphaOptionsTable


VARIANT CMUSHclientDoc::GetTriggerOption(LPCTSTR TriggerName, LPCTSTR OptionName) 
{
CString strTriggerName = TriggerName;
CTrigger * trigger_item;

	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_NULL;

  // trim spaces from name, make lower-case
  CheckObjectName (strTriggerName, false);

  vaResult.vt = VT_EMPTY;

  // see if trigger exists, if not return EMPTY
  if (!GetTriggerMap ().Lookup (strTriggerName, trigger_item))
	  return vaResult;

int iItem;
int iResult = FindBaseOption (OptionName, TriggerOptionsTable, iItem);

  
  if (iResult == eOK)
    {

    // numeric option

    if (m_CurrentPlugin &&
        (TriggerOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_READ))
    	return vaResult;  // not available to plugin

    long Value =  GetBaseOptionItem (iItem, 
                              TriggerOptionsTable, 
                              NUMITEMS (TriggerOptionsTable),
                              (char *) trigger_item);  

    SetUpVariantLong (vaResult, Value);
    }  // end of found numeric option
  else
    { // not numeric option, try alpha
    int iResult = FindBaseAlphaOption (OptionName, TriggerAlphaOptionsTable, iItem);
    if (iResult == eOK)
      {

      // alpha option

      if (m_CurrentPlugin &&
          (TriggerAlphaOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_READ))
    	  return vaResult;  // not available to plugin

      CString strValue =  GetBaseAlphaOptionItem (iItem, 
                                                 TriggerAlphaOptionsTable,
                                                 NUMITEMS (TriggerAlphaOptionsTable),
                                                 (char *) trigger_item);

      SetUpVariantString (vaResult, strValue);
      }  // end of found
    }

	return vaResult;
} // end of GetTriggerOption

VARIANT CMUSHclientDoc::GetPluginTriggerOption(LPCTSTR PluginID, LPCTSTR TriggerName, LPCTSTR OptionName) 
{
GET_PLUGIN_STUFF (GetTriggerOption (TriggerName, OptionName))
}

long CMUSHclientDoc::SetTriggerOption(LPCTSTR TriggerName, LPCTSTR OptionName, LPCTSTR Value) 
{
CString strTriggerName = TriggerName;
CString strValue = Value;
CTrigger * trigger_item;

  // trim spaces from name, make lower-case
  CheckObjectName (strTriggerName, false);

  if (!GetTriggerMap ().Lookup (strTriggerName, trigger_item))
    return eTriggerNotFound;

CString strOptionName = OptionName;

  strOptionName.MakeLower();
  strOptionName.TrimLeft ();
  strOptionName.TrimRight ();

int iItem;
int iResult = FindBaseOption (strOptionName, TriggerOptionsTable, iItem);

bool bChanged;

  if (iResult == eOK)
    {
    // this is a numeric option
    
    // for boolean options, accept "y" or "n"
    if (TriggerOptionsTable [iItem].iMinimum == 0 &&
      TriggerOptionsTable [iItem].iMaximum == 0)
      {
      if (strValue == "Y" || strValue == "y")
        Value = "1";
      else if (strValue == "N" || strValue == "n")
        Value = "0";
      }

    if (!IsNumber (Value, true))
       return eOptionOutOfRange;

    long iValue = atol (Value);

    if (m_CurrentPlugin &&
        (TriggerOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_WRITE))
    	return ePluginCannotSetOption;  // not available to plugin

    if (TriggerOptionsTable [iItem].iFlags & OPT_CANNOT_WRITE)
    	return ePluginCannotSetOption;  // not available for writing at all    

    iResult = SetBaseOptionItem (iItem,
                        TriggerOptionsTable,
                        NUMITEMS (TriggerOptionsTable),
                        (char *) trigger_item, 
                        iValue,
                        bChanged);

    if (bChanged)
      {
      if (!m_CurrentPlugin) // plugin mods don't really count
        SetModifiedFlag (TRUE);   // document has changed
      trigger_item->nUpdateNumber    = App.GetUniqueNumber ();   // for concurrency checks
      }

    if (strOptionName == "sequence")
      SortTriggers ();

    return iResult;

    }  // end of found
  else
    { // not numeric option, try alpha
    int iResult = FindBaseAlphaOption (strOptionName, TriggerAlphaOptionsTable, iItem);
    if (iResult == eOK)
      {

      // alpha option

      if (m_CurrentPlugin &&
          (TriggerAlphaOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_WRITE))
    	  return ePluginCannotSetOption;  // not available to plugin

      if (TriggerAlphaOptionsTable [iItem].iFlags & OPT_CANNOT_WRITE)
    	  return ePluginCannotSetOption;  // not available for writing at all    

      // ------ preliminary validation before setting the option

      // cannot have null match text
      if (strOptionName == "match" || 
          strOptionName == "ignore_case" ||
          strOptionName == "multi_line")
        {
        if (strValue.IsEmpty ())
          return eTriggerCannotBeEmpty;

        t_regexp * regexp = NULL;

        CString strRegexp; 

        if (trigger_item->bRegexp)
          strRegexp = strValue;
        else
          strRegexp = ConvertToRegularExpression (strValue);

        // compile regular expression
        try 
          {
          regexp = regcomp (strRegexp, (trigger_item->ignore_case ? PCRE_CASELESS : 0) |
                                       (trigger_item->bMultiLine  ? PCRE_MULTILINE : 0) |
                                       (m_bUTF_8 ? PCRE_UTF8 : 0)
                                       );
          }   // end of try
        catch(CException* e)
          {
          e->Delete ();
          return eBadRegularExpression;
          } // end of catch
      
        delete trigger_item->regexp;    // get rid of old one
        trigger_item->regexp = regexp;

        } // end of option "match"  
      else if (strOptionName == "script")
        {
  
        // get trigger dispatch ID

        if (GetScriptEngine () && !strValue.IsEmpty ())
          {
          DISPID dispid = DISPID_UNKNOWN;
          CString strMessage;
          dispid = GetProcedureDispid (strValue, "trigger", TriggerName, strMessage);
          if (dispid == DISPID_UNKNOWN)
            return eScriptNameNotLocated;
          trigger_item->dispid  = dispid;   // update dispatch ID
          }
        } // end of option "script"


      // set the option now

      iResult = SetBaseAlphaOptionItem (iItem,
                        TriggerAlphaOptionsTable,
                        NUMITEMS (TriggerAlphaOptionsTable),
                        (char *) trigger_item,  
                        strValue,
                        bChanged);

      if (bChanged)
        {
        if (!m_CurrentPlugin) // plugin mods don't really count
          SetModifiedFlag (TRUE);   // document has changed
        trigger_item->nUpdateNumber    = App.GetUniqueNumber ();   // for concurrency checks
        }

      return iResult;
      }  // end of found alpha option
    }  // end of not numeric option

 return eUnknownOption;
}   // end of SetTriggerOption


#define AO(arg) offsetof (CAlias, arg), sizeof (((CAlias *)NULL)->arg)

//-----config name ----------------  default ---variable name ---- min, max, flags ---
tConfigurationNumericOption AliasOptionsTable [] = {

// numbers

{"send_to",              eSendToWorld, AO(iSendTo), 0, eSendToLast - 1},               
{"sequence",             0,            AO(iSequence), 0, 10000},               
{"user",                 0,            AO(iUserOption), LONG_MIN, LONG_MAX},               

// boolean

{"enabled",              false, AO(bEnabled)},               
{"expand_variables",     false, AO(bExpandVariables)},               
{"ignore_case",          false, AO(bIgnoreCase), 0, 0, OPT_CANNOT_WRITE}, 
{"omit_from_log",        false, AO(bOmitFromLog)},               
{"omit_from_command_history", false, AO(bOmitFromCommandHistory)},               
{"omit_from_output",     false, AO(bOmitFromOutput)},               
{"regexp",               false, AO(bRegexp), 0, 0, OPT_CANNOT_WRITE},               
{"menu",                 false, AO(bMenu)},               
{"keep_evaluating",      false, AO(bKeepEvaluating)},               
{"echo_alias",           false, AO(bEchoAlias)},               
{"temporary",            false, AO(bTemporary)},
{"one_shot",             false, AO(bOneShot)},               

{NULL}   // end of table marker            

  };  // end of AliasOptionsTable 

#define AA(arg) offsetof (CAlias, arg)
                    
// first  optional argument: multiline flag
// second optional argument: preserve spaces flag (defaults to true for multiline)

tConfigurationAlphaOption AliasAlphaOptionsTable  [] =
{
// CAN'T CHANGE LABEL {"name",         "",         TA(strLabel)},
{"group",        "",         AA(strGroup)},
{"match",        "",         AA(name)},
{"script",       "",         AA(strProcedure)},
{"send",         "",         AA(contents), OPT_MULTLINE},
{"variable",     "",         AA(strVariable)},

{NULL}  // end of table marker
};    // end of AliasAlphaOptionsTable


VARIANT CMUSHclientDoc::GetAliasOption(LPCTSTR AliasName, LPCTSTR OptionName) 
{
CString strAliasName = AliasName;
CAlias * Alias_item;

	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_NULL;

  // trim spaces from name, make lower-case
  CheckObjectName (strAliasName, false);

  vaResult.vt = VT_EMPTY;

  // see if Alias exists, if not return EMPTY
  if (!GetAliasMap ().Lookup (strAliasName, Alias_item))
	  return vaResult;

int iItem;
int iResult = FindBaseOption (OptionName, AliasOptionsTable, iItem);

  
  if (iResult == eOK)
    {

    // numeric option

    if (m_CurrentPlugin &&
        (AliasOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_READ))
    	return vaResult;  // not available to plugin

    long Value =  GetBaseOptionItem (iItem, 
                              AliasOptionsTable, 
                              NUMITEMS (AliasOptionsTable),
                              (char *) Alias_item);  

    SetUpVariantLong (vaResult, Value);
    }  // end of found numeric option
  else
    { // not numeric option, try alpha
    int iResult = FindBaseAlphaOption (OptionName, AliasAlphaOptionsTable, iItem);
    if (iResult == eOK)
      {

      // alpha option

      if (m_CurrentPlugin &&
          (AliasAlphaOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_READ))
    	  return vaResult;  // not available to plugin

      CString strValue =  GetBaseAlphaOptionItem (iItem, 
                                                 AliasAlphaOptionsTable,
                                                 NUMITEMS (AliasAlphaOptionsTable),
                                                 (char *) Alias_item);

      SetUpVariantString (vaResult, strValue);
      }  // end of found
    }

	return vaResult;
}   // end of GetAliasOption

VARIANT CMUSHclientDoc::GetPluginAliasOption(LPCTSTR PluginID, LPCTSTR AliasName, LPCTSTR OptionName) 
{
GET_PLUGIN_STUFF (GetAliasOption (AliasName, OptionName))
}

long CMUSHclientDoc::SetAliasOption(LPCTSTR AliasName, LPCTSTR OptionName, LPCTSTR Value) 
{
CString strAliasName = AliasName;
CString strValue = Value;
CAlias * Alias_item;

  // trim spaces from name, make lower-case
  CheckObjectName (strAliasName, false);

  if (!GetAliasMap ().Lookup (strAliasName, Alias_item))
    return eAliasNotFound;

CString strOptionName = OptionName;

  strOptionName.MakeLower();
  strOptionName.TrimLeft ();
  strOptionName.TrimRight ();

int iItem;
int iResult = FindBaseOption (strOptionName, AliasOptionsTable, iItem);

bool bChanged;

  if (iResult == eOK)
    {
    // this is a numeric option

    // for boolean options, accept "y" or "n"
    if (AliasOptionsTable [iItem].iMinimum == 0 &&
      AliasOptionsTable [iItem].iMaximum == 0)
      {
      if (strValue == "Y" || strValue == "y")
        Value = "1";
      else if (strValue == "N" || strValue == "n")
        Value = "0";
      }

    if (!IsNumber (Value, true))
       return eOptionOutOfRange;

    long iValue = atol (Value);

    if (m_CurrentPlugin &&
        (AliasOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_WRITE))
    	return ePluginCannotSetOption;  // not available to plugin

    if (AliasOptionsTable [iItem].iFlags & OPT_CANNOT_WRITE)
    	return ePluginCannotSetOption;  // not available for writing at all    

    iResult = SetBaseOptionItem (iItem,
                        AliasOptionsTable,
                        NUMITEMS (AliasOptionsTable),
                        (char *) Alias_item, 
                        iValue,
                        bChanged);

    if (bChanged)
      {
      if (!m_CurrentPlugin) // plugin mods don't really count
        SetModifiedFlag (TRUE);   // document has changed
      Alias_item->nUpdateNumber    = App.GetUniqueNumber ();   // for concurrency checks
      }

    if (strOptionName == "sequence")
      SortAliases ();

    return iResult;

    }  // end of found
  else
    { // not numeric option, try alpha
    int iResult = FindBaseAlphaOption (strOptionName, AliasAlphaOptionsTable, iItem);
    if (iResult == eOK)
      {

      // alpha option

      if (m_CurrentPlugin &&
          (AliasAlphaOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_WRITE))
    	  return ePluginCannotSetOption;  // not available to plugin

      if (AliasAlphaOptionsTable [iItem].iFlags & OPT_CANNOT_WRITE)
    	  return ePluginCannotSetOption;  // not available for writing at all    

      // ------ preliminary validation before setting the option

      // cannot have null match text
      if (strOptionName == "match")
        {
        if (strValue.IsEmpty ())
          return eAliasCannotBeEmpty;

        t_regexp * regexp = NULL;

        CString strRegexp; 

        if (Alias_item->bRegexp)
          strRegexp = strValue;
        else
          strRegexp = ConvertToRegularExpression (strValue);

        // compile regular expression
        try 
          {
          regexp = regcomp (strRegexp, (Alias_item->bIgnoreCase ? PCRE_CASELESS : 0)
#if ALIASES_USE_UTF8
                             | (m_bUTF_8 ? PCRE_UTF8 : 0)
#endif // ALIASES_USE_UTF8
              );
          }   // end of try
        catch(CException* e)
          {
          e->Delete ();
          return eBadRegularExpression;
          } // end of catch
      
        delete Alias_item->regexp;    // get rid of old one
        Alias_item->regexp = regexp;

        } // end of option "match"  
      else if (strOptionName == "script")
        {
  
        // get Alias dispatch ID

        if (GetScriptEngine () && !strValue.IsEmpty ())
          {
          DISPID dispid = DISPID_UNKNOWN;
          CString strMessage;
          dispid = GetProcedureDispid (strValue, "Alias", AliasName, strMessage);
          if (dispid == DISPID_UNKNOWN)
            return eScriptNameNotLocated;
          Alias_item->dispid  = dispid;   // update dispatch ID
          }
        } // end of option "script"


      // set the option now

      iResult = SetBaseAlphaOptionItem (iItem,
                        AliasAlphaOptionsTable,
                        NUMITEMS (AliasAlphaOptionsTable),
                        (char *) Alias_item,  
                        strValue,
                        bChanged);

      if (bChanged)
        {
        if (!m_CurrentPlugin) // plugin mods don't really count
          SetModifiedFlag (TRUE);   // document has changed
        Alias_item->nUpdateNumber    = App.GetUniqueNumber ();   // for concurrency checks
        }

      return iResult;
      }  // end of found alpha option
    }  // end of not numeric option

 return eUnknownOption;
}   // end of SetAliasOption

#define TMRO(arg) offsetof (CTimer, arg), sizeof (((CTimer *)NULL)->arg)

//-----config name ----------------  default ---variable name ---- min, max, flags ---
tConfigurationNumericOption TimerOptionsTable [] = {

// numbers


{"hour",           0, TMRO(iAtHour),    0, 23},               
{"minute",         0, TMRO(iAtMinute),  0, 59},               
{"second",         0, TMRO(fAtSecond),  0, 59.9999, OPT_DOUBLE},               
{"offset_hour",    0, TMRO(iOffsetHour),    0, 23},               
{"offset_minute",  0, TMRO(iOffsetMinute),  0, 59},               
{"offset_second",  0, TMRO(fOffsetSecond),  0, 59.9999, OPT_DOUBLE},               
{"send_to",        eSendToWorld,  TMRO(iSendTo), 0, eSendToLast - 1},               
{"user",           0, TMRO(iUserOption), LONG_MIN, LONG_MAX},               

// boolean

{"enabled",              false, TMRO(bEnabled)},               
{"at_time",              false, TMRO(iType)},               
{"one_shot",             false, TMRO(bOneShot)},               
{"omit_from_output",     false, TMRO(bOmitFromOutput)},               
{"omit_from_log",        false, TMRO(bOmitFromLog)},               
{"active_closed",        false, TMRO(bActiveWhenClosed)},               
{"temporary",            false, TMRO(bTemporary)},

{NULL}   // end of table marker            

  };  // end of TimerOptionsTable 

#define TMRA(arg) offsetof (CTimer, arg)
                    
// first  optional argument: multiline flag
// second optional argument: preserve spaces flag (defaults to true for multiline)

tConfigurationAlphaOption TimerAlphaOptionsTable  [] =
{
// CAN'T CHANGE LABEL {"name",         "",         TA(strLabel)},
{"group",        "",         TMRA(strGroup)},
{"script",       "",         TMRA(strProcedure)},
{"send",         "",         TMRA(strContents), OPT_MULTLINE},
{"variable",     "",         TMRA(strVariable)},

{NULL}  // end of table marker
};    // end of TimerAlphaOptionsTable

VARIANT CMUSHclientDoc::GetTimerOption(LPCTSTR TimerName, LPCTSTR OptionName) 
{
CString strTimerName = TimerName;
CTimer * Timer_item;

	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_NULL;

  // trim spaces from name, make lower-case
  CheckObjectName (strTimerName, false);

  vaResult.vt = VT_EMPTY;

  // see if Timer exists, if not return EMPTY
  if (!GetTimerMap ().Lookup (strTimerName, Timer_item))
	  return vaResult;


CString strOptionName = OptionName;

  strOptionName.MakeLower();
  strOptionName.TrimLeft ();
  strOptionName.TrimRight ();

int iItem;
int iResult = FindBaseOption (strOptionName, TimerOptionsTable, iItem);

  
  if (iResult == eOK)
    {

    // numeric option

    if (m_CurrentPlugin &&
        (TimerOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_READ))
    	return vaResult;  // not available to plugin

    double Value;
    
    // this is a pest!
    if (strOptionName == "hour")
      {
      if (Timer_item->iType == CTimer::eInterval)
        Value = Timer_item->iEveryHour;
      else
        Value = Timer_item->iAtHour;
      } // end of option "hour"
    else if (strOptionName == "minute")
      {
      if (Timer_item->iType == CTimer::eInterval)
        Value = Timer_item->iEveryMinute;
      else
        Value = Timer_item->iAtMinute;
      } // end of option "minute"
    else if (strOptionName == "second")
      {
      if (Timer_item->iType == CTimer::eInterval)
        Value = Timer_item->fEverySecond;
      else
        Value = Timer_item->fAtSecond;
      } // end of option "second"
    else
      Value =  GetBaseOptionItem (iItem, 
                                TimerOptionsTable, 
                                NUMITEMS (TimerOptionsTable),
                                (char *) Timer_item);  

    SetUpVariantDouble (vaResult, Value);
    }  // end of found numeric option
  else
    { // not numeric option, try alpha
    int iResult = FindBaseAlphaOption (OptionName, TimerAlphaOptionsTable, iItem);
    if (iResult == eOK)
      {

      // alpha option

      if (m_CurrentPlugin &&
          (TimerAlphaOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_READ))
    	  return vaResult;  // not available to plugin

      CString strValue =  GetBaseAlphaOptionItem (iItem, 
                                                 TimerAlphaOptionsTable,
                                                 NUMITEMS (TimerAlphaOptionsTable),
                                                 (char *) Timer_item);

      SetUpVariantString (vaResult, strValue);
      }  // end of found
    }

	return vaResult;
} // end of GetTimerOption


VARIANT CMUSHclientDoc::GetPluginTimerOption(LPCTSTR PluginID, LPCTSTR TimerName, LPCTSTR OptionName) 
{
GET_PLUGIN_STUFF (GetTimerOption (TimerName, OptionName))
}

long CMUSHclientDoc::SetTimerOption(LPCTSTR TimerName, LPCTSTR OptionName, LPCTSTR Value) 
{
CString strTimerName = TimerName;
CString strValue = Value;
CTimer * Timer_item;

  // trim spaces from name, make lower-case
  CheckObjectName (strTimerName, false);

  if (!GetTimerMap ().Lookup (strTimerName, Timer_item))
    return eTimerNotFound;

CString strOptionName = OptionName;

  strOptionName.MakeLower();
  strOptionName.TrimLeft ();
  strOptionName.TrimRight ();

int iItem;
int iResult = FindBaseOption (strOptionName, TimerOptionsTable, iItem);

bool bChanged;

  if (iResult == eOK)
    {
    // this is a numeric option
    
    // for boolean options, accept "y" or "n"
    if (TimerOptionsTable [iItem].iMinimum == 0 &&
      TimerOptionsTable [iItem].iMaximum == 0)
      {
      if (strValue == "Y" || strValue == "y")
        Value = "1";
      else if (strValue == "N" || strValue == "n")
        Value = "0";
      }

    long iValue = 0;
    double fValue = 0;

    if (strOptionName == "second")
      fValue = atof (Value);
    else
      {
      if (!IsNumber (Value, true))
         return eOptionOutOfRange;

      iValue = atol (Value);
      }

    if (m_CurrentPlugin &&
        (TimerOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_WRITE))
    	return ePluginCannotSetOption;  // not available to plugin

    if (TimerOptionsTable [iItem].iFlags & OPT_CANNOT_WRITE)
    	return ePluginCannotSetOption;  // not available for writing at all    

    iResult = SetBaseOptionItem (iItem,
                        TimerOptionsTable,
                        NUMITEMS (TimerOptionsTable),
                        (char *) Timer_item, 
                        iValue,
                        bChanged);

    if (bChanged)
      {
      if (!m_CurrentPlugin) // plugin mods don't really count
        SetModifiedFlag (TRUE);   // document has changed
      Timer_item->nUpdateNumber    = App.GetUniqueNumber ();   // for concurrency checks
      }

    if (iResult == eOK && Timer_item->iType == CTimer::eInterval)
      {  // need to set "every" time, not "at" time
      if (strOptionName == "hour")
        {
        Timer_item->iEveryHour = iValue;
        ResetOneTimer (Timer_item);
        } // end of option "hour"
      else if (strOptionName == "minute")
        {
        Timer_item->iEveryMinute = iValue; 
        ResetOneTimer (Timer_item);
        } // end of option "minute"
      else if (strOptionName == "second")
        {
        Timer_item->fEverySecond = fValue;
        ResetOneTimer (Timer_item);
        } // end of option "second"

      } // end of need to fiddle with hour/minute/second

    // need to reset if we are changing this
    if (strOptionName == "at_time" && bChanged)
      {
      // copy from at to every or vice-versa
      if (Timer_item->iType == CTimer::eInterval)
        {
        Timer_item->iEveryHour    = Timer_item->iAtHour;
        Timer_item->iEveryMinute  = Timer_item->iAtHour;
        Timer_item->fEverySecond  = Timer_item->fAtSecond;
        }
      else
        {
        Timer_item->iAtHour    = Timer_item->iEveryHour;
        Timer_item->iAtMinute  = Timer_item->iEveryHour;
        Timer_item->fAtSecond  = Timer_item->fEverySecond;
        }
       ResetOneTimer (Timer_item);
      }
    
    return iResult;

    }  // end of found
  else
    { // not numeric option, try alpha
    int iResult = FindBaseAlphaOption (strOptionName, TimerAlphaOptionsTable, iItem);
    if (iResult == eOK)
      {

      // alpha option

      if (m_CurrentPlugin &&
          (TimerAlphaOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_WRITE))
    	  return ePluginCannotSetOption;  // not available to plugin

      if (TimerAlphaOptionsTable [iItem].iFlags & OPT_CANNOT_WRITE)
    	  return ePluginCannotSetOption;  // not available for writing at all    

      // ------ preliminary validation before setting the option

      if (strOptionName == "script")
        {
  
        // get Timer dispatch ID

        if (GetScriptEngine () && !strValue.IsEmpty ())
          {
          DISPID dispid = DISPID_UNKNOWN;
          CString strMessage;
          dispid = GetProcedureDispid (strValue, "Timer", TimerName, strMessage);
          if (dispid == DISPID_UNKNOWN)
            return eScriptNameNotLocated;
          Timer_item->dispid  = dispid;   // update dispatch ID
          }
        } // end of option "script"


      // set the option now

      iResult = SetBaseAlphaOptionItem (iItem,
                        TimerAlphaOptionsTable,
                        NUMITEMS (TimerAlphaOptionsTable),
                        (char *) Timer_item,  
                        strValue,
                        bChanged);

      if (bChanged)
        {
        if (!m_CurrentPlugin) // plugin mods don't really count
          SetModifiedFlag (TRUE);   // document has changed
        Timer_item->nUpdateNumber    = App.GetUniqueNumber ();   // for concurrency checks
        }

      return iResult;
      }  // end of found alpha option
    }  // end of not numeric option

 return eUnknownOption;
} // end of SetTimerOption



CTextDocument * CMUSHclientDoc::FindNotepad (const CString strTitle)
  {
CTextDocument * pTextDoc = NULL;

  for (POSITION docPos = App.m_pNormalDocTemplate->GetFirstDocPosition();
      docPos != NULL; )
    {
    pTextDoc = (CTextDocument *) App.m_pWorldDocTemplate->GetNextDoc(docPos);

    // ignore unrelated worlds
    if (pTextDoc->m_pRelatedWorld == this &&
       pTextDoc->m_iUniqueDocumentNumber == m_iUniqueDocumentNumber &&
       pTextDoc->m_strTitle.CompareNoCase (strTitle) == 0)
      return pTextDoc;      // right title, world, document number

    } // end of doing each document


  return NULL;    // not found
  }


long CMUSHclientDoc::SaveNotepad(LPCTSTR Title, LPCTSTR FileName, BOOL ReplaceExisting) 
{
CTextDocument * pTextDoc = FindNotepad (Title);

  if (pTextDoc)
    return pTextDoc->DoSave(FileName, ReplaceExisting);

 return false;
}



long CMUSHclientDoc::CloseNotepad(LPCTSTR Title, BOOL QuerySave) 
{
CTextDocument * pTextDoc = FindNotepad (Title);

  if (pTextDoc)
    {
      // see if they want to save it
    if (QuerySave)
	    if (!pTextDoc->SaveModified())
		    return false;

    // saved OK, let's close it
    pTextDoc->OnCloseDocument ();
    
    return true;
    } // end of having an existing notepad document


return false;
}


BSTR CMUSHclientDoc::GetEntity(LPCTSTR Name) 
{
	CString strResult;

  m_CustomEntityMap.Lookup (Name, strResult);

	return strResult.AllocSysString();
}

long CMUSHclientDoc::Execute(LPCTSTR Command) 
{

// remember current plugin
CPlugin *  pCurrentPlugin = m_CurrentPlugin;

// stop an alias from recalling itself indefinitely

if (++m_iExecutionDepth > MAX_EXECUTION_DEPTH)
  {
  m_iExecutionDepth--;
  return eCommandsNestedTooDeeply;
  }

CString strFixedCommand = Command;

// huh?  ASSERT (m_CurrentPlugin == NULL); 

m_CurrentPlugin = NULL;

// look for scripting prefix
if (!m_strScriptPrefix.IsEmpty () &&    // and we *have* a script prefix
    strFixedCommand.Left (m_strScriptPrefix.GetLength ()) == 
                          m_strScriptPrefix)  // and it matches
  {
  CString strCommand = strFixedCommand.Mid (m_strScriptPrefix.GetLength ());

  // if scripting enabled, do it
  if (m_bEnableScripts)
    {
    m_bInSendToScript = false;   // they can do DeleteLines here I think

    if (m_ScriptEngine)      // scripting might be enabled, but not OK to run
      m_ScriptEngine->Parse (strCommand, "Command line");
    else
      ColourNote ("white", "red", 
          Translate ("Scripting is not active yet, or script file had a parse error."));

    m_bInSendToScript = true;
    
    m_iExecutionDepth--;
    m_CurrentPlugin = pCurrentPlugin;  // restore whatever plugin we are in
    return eOK;
    }

  // warn if they seem to want to script but it is not enabled

  strCommand.TrimLeft ();

  if (strCommand.Left (6).CompareNoCase ("world.") == 0 ||
      strCommand.Left (8).CompareNoCase ("$world->") == 0)
   ColourNote ("white", "darkblue", 
    Translate ("Warning - you appear to be doing a script command but scripting is not enabled."));

  }      // end of having a scripting prefix


// do command stacking first, so that other things (like aliases) work within command-stack

// ---------------------- COMMAND STACKING ------------------------------

if (m_enable_command_stack && 
 !m_strCommandStackCharacter.IsEmpty ())
   {
   // new in version 3.74 - command stack character at start of line disables command stacking
   if (!strFixedCommand.IsEmpty () &&
       strFixedCommand [0] == m_strCommandStackCharacter [0])
     strFixedCommand.Delete (0, 1);   // delete command stack character itself
   else
     {
     // still want command stacking
     CString strTwoStacks =  m_strCommandStackCharacter [0];
     strTwoStacks +=  m_strCommandStackCharacter [0];
     // convert two command stacks in a row to 0x01  (eg. ;;)
     strFixedCommand = ::Replace (strFixedCommand, strTwoStacks, "\x01");
     // convert any remaining command stacks to a newline
     strFixedCommand.Replace (CString (m_strCommandStackCharacter [0]), ENDLINE);
     // replace any 0x01 with one command stack character
     strFixedCommand.Replace ('\x01', m_strCommandStackCharacter [0]);
     }
   }

CStringList strList;

// break up command into a list, terminated by newlines
StringToList (strFixedCommand, ENDLINE, strList);

// if list is empty, make sure we send at least one empty line
if (strList.IsEmpty ())
  strList.AddTail (""); 

for (POSITION command_pos = strList.GetHeadPosition (); command_pos; )
  {
  CString str = strList.GetNext (command_pos);


  CString strOriginalMessage = str;

  //  TRACE1 ("SendCommand = %s\n", str);

  bool bOmitFromLog = false;

  if (!m_bPluginProcessingCommand)
      {
      m_bPluginProcessingCommand = true;  // so we don't go into a loop
      bool bOK = true;
      // tell each plugin what we are about to Command
      for (POSITION pos = m_PluginList.GetHeadPosition(); pos; )
        {
        CPlugin * pPlugin = m_PluginList.GetNext (pos);

        if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
          continue;

        // see what the plugin makes of this, excluding the ENDLINE
        if (!pPlugin->ExecutePluginScript (ON_PLUGIN_COMMAND, pPlugin->m_dispid_plugin_command, str))
          bOK = false;
        }   // end of doing each plugin

      m_bPluginProcessingCommand = false;

      if (!bOK)
        continue;   // plugin doesn't want to send it
      }

  // empty line - just send it

    if (str.IsEmpty ())
      {
      // pressing <enter> might be trying to connect
      if (CheckConnected ())
        {
        m_iExecutionDepth--;
        m_CurrentPlugin = pCurrentPlugin;  // restore whatever plugin we are in
        return eWorldClosed;
        }

      SendMsg (ENDLINE, m_display_my_input, false, LoggingInput ()); 
      
      continue;

      }
    
  if (EvaluateCommand (str, true, bOmitFromLog))
    break;    // error (eg. connection not open, don't keep at it)
 
  }   // end of processing each line individually

// this command has completed, so we can reduce the execution depth

  m_iExecutionDepth--;
  m_CurrentPlugin = pCurrentPlugin;  // restore whatever plugin we are in
	return eOK;
}   // end of CMUSHclientDoc::Execute

long CMUSHclientDoc::DoAfterSpecial(double Seconds, LPCTSTR SendText, short SendTo) 
{

int iHours,
    iMinutes;

  // sanity check
  if (Seconds < 0.1)
    return eTimeInvalid;

  // fiddle seconds into hours/mins/seconds

  iHours = Seconds / 3600;
  Seconds = Seconds - (iHours * 3600);
  iMinutes = Seconds / 60;
  Seconds = Seconds - (iMinutes * 60);

// a really big number would make hours > 23

  if (iHours > 23)
    return eTimeInvalid;

  // check they are sending to somewhere valid

  if (SendTo < 0 || SendTo >= eSendToLast)
    return eOptionOutOfRange;

CString strTimerName;
CTimer * timer_item;

  // this is a temporary unlabelled timer, make up a name
  strTimerName.Format ("*timer%s", (LPCTSTR) App.GetUniqueString ());

  if (iHours < 0 || iHours > 23)
    return eTimeInvalid;

  if (iMinutes < 0 || iMinutes > 59)
    return eTimeInvalid;

  if (Seconds < 0 || Seconds > 59.9999)
    return eTimeInvalid;

  // create new timer item and insert in timer map
  GetTimerMap ().SetAt (strTimerName, timer_item = new CTimer);

  timer_item->nUpdateNumber    = App.GetUniqueNumber ();   // for concurrency checks

  timer_item->iEveryHour = iHours;
  timer_item->iEveryMinute = iMinutes;
  timer_item->fEverySecond = Seconds;
  timer_item->iType  = CTimer::eInterval;

  timer_item->strContents      = SendText;
  timer_item->bEnabled         = true;
  timer_item->bOneShot         = true;
  timer_item->bTemporary       = true;
  timer_item->bActiveWhenClosed = true;

  timer_item->iSendTo = SendTo;

  ResetOneTimer (timer_item);

  SortTimers ();

	return eOK;
}

long CMUSHclientDoc::ChatCallGeneral (LPCTSTR Server, long Port, const bool zChat)
  {
  // if not chat name, try our character name
  if (m_strOurChatName.IsEmpty () && !m_name.IsEmpty ())
    {
    m_strOurChatName = m_name;
    SetModifiedFlag (TRUE);   // document has changed
    }
  
  // still empty? use a default
  if (m_strOurChatName.IsEmpty ())
    m_strOurChatName = DEFAULT_CHAT_NAME;

  // default to port 4050 if none supplied
  if (Port == 0)
    Port = DEFAULT_CHAT_PORT;

CChatSocket * pSocket = new CChatSocket (this);

  if (zChat)
    {
    pSocket->m_iChatConnectionType = eChatZMud;
    pSocket->m_iFileBlockSize = 1024;
    }
  else
    pSocket->m_iChatConnectionType = eChatMudMaster;

	if (!pSocket->Create(0,
                         SOCK_STREAM,
                         FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE,
                         NULL))
	  {
		delete pSocket;
		return eCannotCreateChatSocket;
	  }     // end of can't create socket

  pSocket->AsyncSelect ();

  pSocket->m_strServerName = Server;
  pSocket->m_ServerAddr.sin_port = htons((u_short)Port); 

	pSocket->m_ServerAddr.sin_family = AF_INET;
	pSocket->m_ServerAddr.sin_addr.s_addr = inet_addr(Server);

// if address is INADDR_NONE then address was a name, not a number

	if (pSocket->m_ServerAddr.sin_addr.s_addr == INADDR_NONE)
	 {
    pSocket->m_pGetHostStruct = new char [MAXGETHOSTSTRUCT];

    if (!pSocket->m_pGetHostStruct)
      {
  		delete pSocket;
      return eCannotLookupDomainName;
      }

    if (Frame.GetSafeHwnd ())   // forget it if we don't have a window yet
      pSocket->m_hNameLookup = WSAAsyncGetHostByName (Frame.GetSafeHwnd (),
                                                     WM_USER_HOST_NAME_RESOLVED,
                                                     Server,
                                                     pSocket->m_pGetHostStruct,
                                                     MAXGETHOSTSTRUCT);

   if (!pSocket->m_hNameLookup)
     {
		  delete pSocket;
      return eCannotLookupDomainName;
     }

    m_ChatList.AddTail (pSocket);
  	return eOK;

	 }   // end of address not being an IP address


// the name was a dotted IP address - just make the connection

  m_ChatList.AddTail (pSocket);

  pSocket->MakeCall ();
  return eOK;   // OK for now, eh?

  }

long CMUSHclientDoc::ChatCall(LPCTSTR Server, long Port) 
{
  return ChatCallGeneral (Server, Port, false);  // not zChat
} // end of CMUSHclientDoc::ChatCall

long CMUSHclientDoc::ChatCallzChat(LPCTSTR Server, long Port) 
{
  return ChatCallGeneral (Server, Port, true);  // zChat
}


void CMUSHclientDoc::AnsiNote(LPCTSTR Text) 
{
// save old colours
bool bOldNotesInRGB = m_bNotesInRGB;
COLORREF iOldNoteColourFore = m_iNoteColourFore;
COLORREF iOldNoteColourBack = m_iNoteColourBack;
unsigned short iOldNoteStyle = m_iNoteStyle;

bool bBold = false;
bool bInverse = false;
bool bItalic = false;
bool bUnderline = false;
int iCurrentForeGround = WHITE;
int iCurrentBackGround = BLACK;

m_iNoteStyle = NORMAL;   // start off with normal style

const char * p,
           * start;
char c;
long length;

  p = start = Text;
  while (c = *p)
    {
    if (c == ESC)
      {
      length = p - start;

      // output earlier block
      if (length > 0)
        Tell (CString (start, length));
      p++;    // skip the ESC

      if (*p == '[')
        {
        p++;    // skip the [

        int iCode = 0;
        while (isdigit (*p) || *p == ';' || *p == 'm')
          {
          if (isdigit (c = *p))
            {
            iCode *= 10;
            iCode += c - '0';
            }
          else
            if (c == ';' || c == 'm')
              {
              switch (iCode)
                {
                // reset colours to defaults
                case ANSI_RESET:
                   iCurrentForeGround = WHITE;
                   iCurrentBackGround = BLACK;
                   bBold = false;     
                   bInverse = false;
                   bItalic = false;   
                   bUnderline = false;
                   break;

                // bold
                case ANSI_BOLD:
                   bBold = true;
                   break;

                // inverse
                case ANSI_INVERSE:
                   bInverse = true;
                   break;

                // blink
                case ANSI_BLINK:
                case ANSI_SLOW_BLINK:
                case ANSI_FAST_BLINK:
                   bItalic = true;
                   break;

                // underline
                case ANSI_UNDERLINE:
                   bUnderline = true;
                   break;

                // not bold
                case ANSI_CANCEL_BOLD:
                   bBold = false;
                   break;

                // not inverse
                case ANSI_CANCEL_INVERSE:
                   bInverse = false;
                   break;

                // not blink
                case ANSI_CANCEL_BLINK:
                case ANSI_CANCEL_SLOW_BLINK:
                   bItalic = false;
                   break;

                // not underline
                case ANSI_CANCEL_UNDERLINE:
                   bUnderline = false;
                   break;

                // different foreground colour
                case ANSI_TEXT_BLACK:
                case ANSI_TEXT_RED    :
                case ANSI_TEXT_GREEN  :
                case ANSI_TEXT_YELLOW :
                case ANSI_TEXT_BLUE   :
                case ANSI_TEXT_MAGENTA:
                case ANSI_TEXT_CYAN   :
                case ANSI_TEXT_WHITE  :
                   iCurrentForeGround = iCode - ANSI_TEXT_BLACK;
                   break;

                // different background colour
                case ANSI_BACK_BLACK  :
                case ANSI_BACK_RED    :
                case ANSI_BACK_GREEN  :
                case ANSI_BACK_YELLOW :
                case ANSI_BACK_BLUE   :
                case ANSI_BACK_MAGENTA:
                case ANSI_BACK_CYAN   :
                case ANSI_BACK_WHITE  :
                   iCurrentBackGround = iCode - ANSI_BACK_BLACK;
                   break;

                } // end of switch

              m_iNoteStyle = NORMAL;

              // select colours
              if (bBold)
                {
                SetNoteColourFore (m_boldcolour [iCurrentForeGround]);
                SetNoteColourBack (m_normalcolour [iCurrentBackGround]);
                m_iNoteStyle |= HILITE;
                }
              else
                {
                SetNoteColourFore (m_normalcolour [iCurrentForeGround]);
                SetNoteColourBack (m_normalcolour [iCurrentBackGround]);
                }

              // select other style bits
              if (bInverse)
                m_iNoteStyle |= INVERSE;

              if (bItalic)
                m_iNoteStyle |= BLINK;

              if (bUnderline)
                m_iNoteStyle |= UNDERLINE;

              p++;  // skip m or ;
              }   // end of ESC [ nn ; or ESC [ nn m

          if (c == ';')
            iCode = 0;
          else
            if (c == 'm')
              break;
            else
              p++;    // next character
          } // end of getting code
        } // end of ESC [ something
       else
         p++; // skip it

      start = p;  // ready to start a new batch
      } // end of ESC something
    else
      p++;  // just keep counting characters

    } // end of processing each character

// output remaining text  - and newline
Note (start);

// put the colours back
if (bOldNotesInRGB)
  {
  m_iNoteColourFore = iOldNoteColourFore;
  m_iNoteColourBack = iOldNoteColourBack;
  }
else  
  m_bNotesInRGB = false;

// put style back
m_iNoteStyle = iOldNoteStyle;

} // end of CMUSHclientDoc::AnsiNote


static CString AnsiCode (short Code) 
{
return CFormat ("%c[%dm", ESC, Code); 
}

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
               *p != ';' && 
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
  }

long CMUSHclientDoc::ChatNameChange(LPCTSTR NewName) 
{
CString strNewName = NewName;

  strNewName.TrimLeft ();
  strNewName.TrimRight ();

  if (strNewName.IsEmpty ())
    return eBadParameter;

  if (m_strOurChatName.IsEmpty ())
    m_strOurChatName = "<no name>";

  ChatNote (eChatNameChange,
              TFormat ("Your chat name changed from %s to %s", 
                     (LPCTSTR) m_strOurChatName, 
                     (LPCTSTR) strNewName));

  if (m_strOurChatName != strNewName)
    SetModifiedFlag (TRUE);   // document has changed

  m_strOurChatName = strNewName;

  SendChatMessageToAll (CHAT_NAME_CHANGE, strNewName, false, false, false, 0, "", 0);
	return eOK;
}

long CMUSHclientDoc::ChatEverybody(LPCTSTR Message, BOOL Emote) 
{
CString strMessage;

  if (Emote)
    strMessage = CFormat ("\n%s%s%s %s%s\n",
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                              (LPCTSTR) m_strOurChatName,
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED));   
    else
    strMessage = TFormat ("\n%s chats to everybody, '%s%s%s%s'\n",
                              (LPCTSTR) m_strOurChatName,
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED));   

  if (SendChatMessageToAll (CHAT_TEXT_EVERYBODY,
                                  strMessage,
                                  true,    // unless ignoring them
                                  false,
                                  false,
                                  0,    // don't ignore us
                                  "",   // no group
                                  0))   // use our stamp
  {
  if (Emote)
    ChatNote (eChatOutgoingEverybody,
              TFormat ("You emote to everybody: %s%s%s %s%s", 
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                              (LPCTSTR) m_strOurChatName,
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED)));   
  else
    ChatNote (eChatOutgoingEverybody,
              TFormat ("You chat to everybody, '%s%s%s%s'", 
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED)));   
  return eOK;
  }
else
  {
  ChatNote (eChatOutgoingEverybody, 
            Translate ("No (relevant) chat connections."));
  return eNoChatConnections;
  }
}


long CMUSHclientDoc::ChatID(long ID, LPCTSTR Message, BOOL Emote) 
{

  // quiet failure on ID zero - probably caused by a ChatGetID lookup failure
  if (ID == 0)
    return eChatIDNotFound;

CString strMessage;

  CChatSocket * pSocket = GetChatSocket (ID); // find them

  if (!pSocket)
    {
    ChatNote (eChatOutgoingPersonal, TFormat ("Chat ID %i is not connected.", ID));
	  return eChatIDNotFound;
    }

  if (Emote)  
   strMessage = TFormat ("\nTo you, %s%s%s %s%s\n",
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                              (LPCTSTR) m_strOurChatName,
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED)   
                               );
  else
    strMessage = TFormat ("\n%s chats to you, '%s%s%s%s'\n",
                              (LPCTSTR) m_strOurChatName,
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED)   
                               );

  pSocket->m_iCountOutgoingPersonal++;
  pSocket->SendChatMessage (CHAT_TEXT_PERSONAL, strMessage);

  if (Emote)  
    ChatNote (eChatOutgoingPersonal,
              TFormat ("You emote to %s: %s%s%s %s%s", 
                  (LPCTSTR) pSocket->m_strRemoteUserName, 
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                              (LPCTSTR) m_strOurChatName,
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED)));   
  else
    ChatNote (eChatOutgoingPersonal,
              TFormat ("You chat to %s, '%s%s%s%s'", 
                        (LPCTSTR) pSocket->m_strRemoteUserName, 
                        (LPCTSTR) AnsiCode (ANSI_BOLD),  
                        (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                         Message,
                        (LPCTSTR) AnsiCode (ANSI_TEXT_RED)));   
  return eOK;

}


long CMUSHclientDoc::ChatPersonal(LPCTSTR Who, LPCTSTR Message, BOOL Emote) 
{
int iCount = 0;

  if (strlen (Who) == 0)
    return eBadParameter;

  for (POSITION chatpos = m_ChatList.GetHeadPosition (); chatpos; )
    {
    CChatSocket * pSocket = m_ChatList.GetNext (chatpos);
    if (pSocket->m_iChatStatus == eChatConnected)
      {
      if (pSocket->m_strRemoteUserName.CompareNoCase (Who) == 0)
        {
        if (ChatID (pSocket->m_iChatID, Message, Emote) == eOK)
          iCount++; // count matches
        }
      }   // end of chat session not being deleted
    } // end of all chat sessions

  if (iCount == 0)
    {
    ChatNote (eChatOutgoingPersonal, TFormat ("%s is not connected.", Who));
	  return eChatPersonNotFound;
    }
  else
    if (iCount > 1)
      ChatNote (eChatOutgoingPersonal, TFormat ("%i matches.", iCount));

  return eOK;
}

long CMUSHclientDoc::ChatGroup(LPCTSTR Group, LPCTSTR Message, BOOL Emote) 
{
  if (strlen (Group) == 0)
    return eBadParameter;

  CString strMessage;
  
  if (Emote)
    strMessage = TFormat ("%-15s\nTo the group, %s%s%s %s%s\n",
                                Group,
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                              (LPCTSTR) m_strOurChatName,
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED));   
  else
    strMessage = TFormat ("%-15s\n%s chats to the group, '%s%s%s%s'\n",
                                Group,
                                (LPCTSTR) m_strOurChatName,
                                (LPCTSTR) AnsiCode (ANSI_BOLD),  
                                (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                                 Message,
                                (LPCTSTR) AnsiCode (ANSI_TEXT_RED));   

if (SendChatMessageToAll (CHAT_TEXT_GROUP,
                                  strMessage,
                                  true,      // unless ignoring them
                                  false,     // to all connections
                                  false,
                                  0,
                                  Group,
                                  0))    
  {
  if (Emote)
    ChatNote (eChatOutgoingGroup,
              TFormat ("You emote to the group %s: %s%s%s %s%s", 
                              Group, 
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                              (LPCTSTR) m_strOurChatName,
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED)));   
  else
    ChatNote (eChatOutgoingGroup,
              TFormat ("You chat to the group %s, '%s%s%s%s'", 
                               Group, 
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED)));   

  return eOK;
  }
else
  {
  ChatNote (eChatOutgoingGroup,
              TFormat ("No chat connections in the group %s.", Group));
  return eNoChatConnections;
  }

}

// Does a note in the chat colour - including processing ANSI codes

void CMUSHclientDoc::ChatNote(short NoteType, LPCTSTR Message) 
{

  CString strMessage = Message;

  // strip leading \n
  if (strMessage.GetLength () >= 1 && strMessage.Left (1) == '\n')
    strMessage = strMessage.Mid (1);

  // strip trailing \n
  if (strMessage.GetLength () >= 1 && strMessage.Right (1) == '\n')
    strMessage = strMessage.Left (strMessage.GetLength () - 1);

  // check message is not too long for what we accept - bytes

  if (m_iMaxChatBytesPerMessage > 0)
    {
    if (strMessage.GetLength () > m_iMaxChatBytesPerMessage)
      {
      strMessage = strMessage.Left (m_iMaxChatBytesPerMessage);
      strMessage += TFormat ("\n[Chat message truncated, exceeds %i bytes]", 
            m_iMaxChatBytesPerMessage);
      }
    }   // end of byte count check wanted

  // check message is not too long for what we accept - lines
  
  if (m_iMaxChatLinesPerMessage > 0)
    {
    int iLines = 0;
    char * p;

    for (p = strMessage.GetBuffer (strMessage.GetLength ());
         *p;
         p++)
      {
      if (*p == '\n')
        if (++iLines >= m_iMaxChatLinesPerMessage)
          {
          *p = 0;
          break;    // stop scanning for newlines
          }
      } // end of buffer scan
    strMessage.ReleaseBuffer (-1);
    if (iLines >= m_iMaxChatLinesPerMessage && p [1])
      strMessage += TFormat ("\n[Chat message truncated, exceeds %i lines]", 
            m_iMaxChatLinesPerMessage);
    }   // end of line count check wanted

  CPlugin * pSavedPlugin = m_CurrentPlugin;
  // tell each plugin what we are about to display
  for (POSITION pluginpos = m_PluginList.GetHeadPosition(); pluginpos; )
    {
    CPlugin * pPlugin = m_PluginList.GetNext (pluginpos);

    if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
      continue;

    // see what the plugin makes of this,
    if (!pPlugin->ExecutePluginScript (ON_PLUGIN_CHAT_DISPLAY, 
                          pPlugin->m_dispid_plugin_On_Chat_Display, 
                          NoteType,     // message number
                          string (strMessage)    // message text
                          ))
      {
      m_CurrentPlugin = pSavedPlugin;
      return;   // false means plugin handled it 
      }
    }   // end of doing each plugin
  m_CurrentPlugin = pSavedPlugin;


// save old colours - because we switch to the chat colour below
bool bOldNotesInRGB = m_bNotesInRGB;
COLORREF iOldNoteColourFore = m_iNoteColourFore;
COLORREF iOldNoteColourBack = m_iNoteColourBack;

  if (m_cChatForegroundColour == m_cChatBackgroundColour) 
    NoteColourRGB (RGB (255, 0, 0), RGB (0, 0, 0));
  else
    NoteColourRGB (m_cChatForegroundColour, m_cChatBackgroundColour);

  // get rid of unwanted incoming colour codes
  if (m_bIgnoreChatColours)
    strMessage = ::StripAnsi (strMessage);
  
  AnsiNote (m_strChatMessagePrefix + strMessage);

  // put the colours back
  if (bOldNotesInRGB)
    {
    m_iNoteColourFore = iOldNoteColourFore;
    m_iNoteColourBack = iOldNoteColourBack;
    }
  else  
    m_bNotesInRGB = false;

}

long CMUSHclientDoc::ChatAcceptCalls(short Port) 
{

  if (m_pChatListenSocket)
    return eChatAlreadyListening;

  if (Port && m_IncomingChatPort != Port)
    {
    SetModifiedFlag (TRUE);   // document has changed
    m_IncomingChatPort = Port;
    }

  if (m_IncomingChatPort == 0)
    m_IncomingChatPort = DEFAULT_CHAT_PORT;

// set flag for next session if necessary
 if (!m_bAcceptIncomingChatConnections)
   {
   SetModifiedFlag (TRUE);   // document has changed
   m_bAcceptIncomingChatConnections = true;
   }

 Frame.SetStatusMessageNow (TFormat ("Accepting chat calls on port %d",
                              m_IncomingChatPort));

 m_pChatListenSocket = new CChatListenSocket (this);

	if (!m_pChatListenSocket->Create (m_IncomingChatPort,
                         SOCK_STREAM,
                         FD_ACCEPT | FD_CLOSE ,
                         NULL))
	  {
    int nError = GetLastError ();
		delete m_pChatListenSocket;
    m_pChatListenSocket = NULL;
    ChatNote (eChatConnection,
              TFormat (
              "Cannot accept calls on port %i, code = %i (%s)", 
                    m_IncomingChatPort, 
                    nError,
                    GetSocketError (nError)));
		return eCannotCreateChatSocket;
	  }     // end of can't create socket

  m_pChatListenSocket->Listen ();

  ChatNote (eChatConnection,
            TFormat (
    "Listening for chat connections on port %d", m_IncomingChatPort));

	return eOK;
}    // end of CMUSHclientDoc::ChatAcceptCalls

void CMUSHclientDoc::ChatStopAcceptingCalls() 
{
if (m_pChatListenSocket)
  {
  ShutDownSocket (*m_pChatListenSocket);

  ChatNote (eChatConnection, "Stopped accepting chat connections.");
  delete m_pChatListenSocket;
  m_pChatListenSocket = NULL;

  // remember they don't want them
  SetModifiedFlag (TRUE);   // document has changed
  m_bAcceptIncomingChatConnections = false;
  }

}

// get list of chat IDs
VARIANT CMUSHclientDoc::GetChatList() 
{
  COleSafeArray sa;   // for list

  long iCount = 0;
  POSITION pos;

  // count ones not marked for deletion
  for (pos = m_ChatList.GetHeadPosition (); pos; )
    {
    CChatSocket * pSocket = m_ChatList.GetNext (pos);
    if (pSocket->m_iChatStatus == eChatConnected)
      iCount++;
    } // end of all chat sessions

  if (iCount) // cannot create empty array dimension
    {
    sa.CreateOneDim (VT_VARIANT, iCount);
  
    // put the chat sessions into the array
      for (iCount = 0, pos = m_ChatList.GetHeadPosition (); pos; )
        {
        CChatSocket * pSocket = m_ChatList.GetNext (pos);

        if (pSocket->m_iChatStatus == eChatConnected)
          {
          // the array must be a bloody array of variants, or VBscript kicks up
          COleVariant v (pSocket->m_iChatID);
          sa.PutElement (&iCount, &v);
          iCount++;
          }
        }      // end of looping through each chat session
    } // end of having at least one

	return sa.Detach ();
}


// get info about one chat ID
VARIANT CMUSHclientDoc::GetChatInfo(long ChatID, short InfoType) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_NULL;

  CChatSocket * pSocket = GetChatSocket (ChatID);

  if (pSocket)
    switch (InfoType)
      {
       case   1: SetUpVariantString (vaResult, pSocket->m_strServerName); break;
       case   2: SetUpVariantString (vaResult, pSocket->m_strRemoteUserName); break;
       case   3: SetUpVariantString (vaResult, pSocket->m_strGroup); break;
       case   4: SetUpVariantString (vaResult, pSocket->m_strRemoteVersion); break;
       case   5: SetUpVariantString (vaResult, pSocket->m_strAllegedAddress); break;
       case   6: SetUpVariantString (vaResult, inet_ntoa (pSocket->m_ServerAddr.sin_addr)); break;
       case   7: SetUpVariantLong   (vaResult, ntohs (pSocket->m_ServerAddr.sin_port)); break;
       case   8: SetUpVariantLong   (vaResult, pSocket->m_iAllegedPort); break;
       case   9: SetUpVariantLong   (vaResult, pSocket->m_iChatStatus); break;
       case  10: SetUpVariantLong   (vaResult, pSocket->m_iChatConnectionType); break;
       case  11: SetUpVariantLong   (vaResult, pSocket->m_iChatID); break;
       case  12: SetUpVariantBool   (vaResult, pSocket->m_bIncoming); break;
       case  13: SetUpVariantBool   (vaResult, pSocket->m_bCanSnoop); break;
       case  14: SetUpVariantBool   (vaResult, pSocket->m_bYouAreSnooping); break;
       case  15: SetUpVariantBool   (vaResult, pSocket->m_bHeIsSnooping); break;
       case  16: SetUpVariantBool   (vaResult, pSocket->m_bCanSendCommands); break;
       case  17: SetUpVariantBool   (vaResult, pSocket->m_bPrivate); break;
       case  18: SetUpVariantBool   (vaResult, pSocket->m_bCanSendFiles); break;
       case  19: SetUpVariantBool   (vaResult, pSocket->m_bIgnore); break;
       case  20: SetUpVariantDouble (vaResult, pSocket->m_fLastPingTime); break;
         
       case  21: 
         if (pSocket->m_tWhenStarted.GetTime ())     // only if non-zero, otherwise return empty
           SetUpVariantDate   (vaResult, COleDateTime (pSocket->m_tWhenStarted.GetTime ())); 
         break;
       case  22: 
         if (pSocket->m_tLastIncoming.GetTime ())     // only if non-zero, otherwise return empty
           SetUpVariantDate   (vaResult, COleDateTime (pSocket->m_tLastIncoming.GetTime ())); 
         break;
       case  23: 
         if (pSocket->m_tLastOutgoing.GetTime ())     // only if non-zero, otherwise return empty
           SetUpVariantDate   (vaResult, COleDateTime (pSocket->m_tLastOutgoing.GetTime ())); 
         break;

       case  24: 
         if (pSocket->m_tStartedFileTransfer.GetTime ())     // only if non-zero, otherwise return empty
           SetUpVariantDate   (vaResult, COleDateTime (pSocket->m_tStartedFileTransfer.GetTime ())); 
         break;

       case  25: SetUpVariantBool   (vaResult, pSocket->m_bDoingFileTransfer); break;
       case  26: SetUpVariantBool   (vaResult, pSocket->m_bSendFile); break;
       case  27: SetUpVariantString (vaResult, pSocket->m_strSenderFileName); break;
       case  28: SetUpVariantString (vaResult, pSocket->m_strOurFileName); break;
       case  29: SetUpVariantLong   (vaResult, pSocket->m_iFileSize); break;
       case  30: SetUpVariantLong   (vaResult, pSocket->m_iFileBlocks); break;
       case  31: SetUpVariantLong   (vaResult, pSocket->m_iBlocksTransferred); break;
       case  32: SetUpVariantLong   (vaResult, pSocket->m_iFileBlockSize); break;
       case  33: SetUpVariantLong   (vaResult, pSocket->m_iCountIncomingPersonal); break;
       case  34: SetUpVariantLong   (vaResult, pSocket->m_iCountIncomingAll); break;
       case  35: SetUpVariantLong   (vaResult, pSocket->m_iCountIncomingGroup); break;
       case  36: SetUpVariantLong   (vaResult, pSocket->m_iCountOutgoingPersonal); break;
       case  37: SetUpVariantLong   (vaResult, pSocket->m_iCountOutgoingAll); break;
       case  38: SetUpVariantLong   (vaResult, pSocket->m_iCountOutgoingGroup); break;
       case  39: SetUpVariantLong   (vaResult, pSocket->m_iCountMessages); break;
       case  40: SetUpVariantLong   (vaResult, pSocket->m_iCountFileBytesIn); break;
       case  41: SetUpVariantLong   (vaResult, pSocket->m_iCountFileBytesOut); break;
       case  42: SetUpVariantLong   (vaResult, pSocket->m_zChatStamp); break;
       case  43: SetUpVariantString (vaResult, pSocket->m_strEmailAddress); break;
       case  44: SetUpVariantString (vaResult, pSocket->m_strPGPkey); break;
       case  45: SetUpVariantShort  (vaResult, pSocket->m_zChatStatus); break;
       case  46: SetUpVariantLong   (vaResult, pSocket->m_iUserOption); break;
         
      default:
        vaResult.vt = VT_EMPTY;
        break;

      } // end of switch

  return vaResult;
}


long CMUSHclientDoc::ChatPing(long ID) 
{
  CChatSocket * pSocket = GetChatSocket (ID);
  if (!pSocket)
    return eChatIDNotFound;
  
  if (App.m_iCounterFrequency)
    QueryPerformanceCounter (&pSocket->m_iPingStartTime);

  CString strMessage = CTime::GetCurrentTime().Format (TranslateTime ("%A, %B %d, %Y, %#I:%M %p"));

  pSocket->SendChatMessage (CHAT_PING_REQUEST, strMessage);

	return eOK;
}


long CMUSHclientDoc::ChatPeekConnections(long ID) 
{
  CChatSocket * pSocket = GetChatSocket (ID);
  if (!pSocket)
    return eChatIDNotFound;
  
  pSocket->SendChatMessage (CHAT_PEEK_CONNECTIONS, "");

	return eOK;
}

long CMUSHclientDoc::ChatRequestConnections(long ID) 
{
  CChatSocket * pSocket = GetChatSocket (ID);
  if (!pSocket)
    return eChatIDNotFound;
  
  pSocket->SendChatMessage (CHAT_REQUEST_CONNECTIONS, "");

	return eOK;
}


long CMUSHclientDoc::ChatDisconnect(long ID) 
{

  CChatSocket * pSocket = GetChatSocket (ID);
  if (!pSocket)
    return eChatIDNotFound;

  ChatNote (eChatConnection,
            TFormat ("Connection to %s dropped.",
            (LPCTSTR) pSocket->m_strRemoteUserName));

  pSocket->OnClose (0);    // close connection

	return eOK;
}

long CMUSHclientDoc::ChatDisconnectAll() 
{
  int iCount = 0;
  for (POSITION pos = m_ChatList.GetHeadPosition (); pos; )
    {
    CChatSocket * pSocket = m_ChatList.GetNext (pos);
    if (!pSocket->m_bDeleteMe)
      {
      iCount++;
      pSocket->OnClose (0);    // close connection
      }
    } // end of all chat sessions
  ChatNote (eChatConnection,
            TFormat ("%i connection%s closed.", 
            PLURAL (iCount)));

	return iCount;
}

// generic chat message sender, for plugins to fool around with
long CMUSHclientDoc::ChatMessage(long ID, short Message, LPCTSTR Text) 
{
  CChatSocket * pSocket = GetChatSocket (ID);
  if (!pSocket)
    return eChatIDNotFound;
  
  pSocket->SendChatMessage (Message, Text);

	return eOK;
}

/*

  Generate colour code ESC [ code m

  - intended for use with chat system where you send ANSI codes to other clients

// Misc
 0: reset
 1: bold
 4: underline
 7: inverse

22: cancel bold
24: cancel underline
27: cancel inverse

// Foreground
30: Black  
31: Red    
32: Green  
33: Yellow 
34: Blue   
35: Magenta
36: Cyan   
37: White  

// Background
40: Black  
41: Red    
42: Green  
43: Yellow 
44: Blue   
45: Magenta
46: Cyan   
47: White 
 
*/


BSTR CMUSHclientDoc::ANSI(short Code) 
{
CString strResult = CFormat ("%c[%dm", ESC, Code); 

	return strResult.AllocSysString();
}


// strip ANSI codes from a message

BSTR CMUSHclientDoc::StripANSI(LPCTSTR Message) 
{
CString strResult = ::StripAnsi (Message);
	return strResult.AllocSysString();
}

#define CHATO(arg) offsetof (CChatSocket, arg), sizeof (((CChatSocket *)NULL)->arg)

//-----config name ----------------  default ---variable name ---- min, max, flags ---
tConfigurationNumericOption ChatOptionsTable [] = {

// boolean

{"can_send_commands",    false, CHATO(m_bCanSendCommands)},               
{"can_send_files",       false, CHATO(m_bCanSendFiles)},               
{"can_snoop",            false, CHATO(m_bCanSnoop)},               
{"ignore",               false, CHATO(m_bIgnore)},               
{"served",               false, CHATO(m_bIncoming)},               
{"private",              false, CHATO(m_bPrivate)},

// numbers 
               
{"user",                 0, CHATO(m_iUserOption), LONG_MIN, LONG_MAX},               

{NULL}   // end of table marker            

  };  // end of ChatOptionsTable 

#define CHATA(arg) offsetof (CChatSocket, arg)
                    
// first  optional argument: multiline flag
// second optional argument: preserve spaces flag (defaults to true for multiline)

tConfigurationAlphaOption ChatAlphaOptionsTable  [] =
{
{"server",      "",         CHATA(m_strServerName), OPT_CANNOT_WRITE},
{"username",    "",         CHATA(m_strRemoteUserName), OPT_CANNOT_WRITE},
{"group",       "",         CHATA(m_strGroup)},
{"version",     "",         CHATA(m_strRemoteVersion), OPT_CANNOT_WRITE},
{"address",     "",         CHATA(m_strAllegedAddress), OPT_CANNOT_WRITE},

{NULL}  // end of table marker
};    // end of ChatAlphaOptionsTable



VARIANT CMUSHclientDoc::GetChatOption(long ID, LPCTSTR OptionName) 
{
CChatSocket * chat_item;

	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;

  chat_item = GetChatSocket (ID);
  if (!chat_item)
	  return vaResult;

int iItem;
int iResult = FindBaseOption (OptionName, ChatOptionsTable, iItem);

  
  if (iResult == eOK)
    {

    // numeric option

    if (m_CurrentPlugin &&
        (ChatOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_READ))
    	return vaResult;  // not available to plugin

    long Value =  GetBaseOptionItem (iItem, 
                              ChatOptionsTable, 
                              NUMITEMS (ChatOptionsTable),
                              (char *) chat_item);  

    SetUpVariantLong (vaResult, Value);
    }  // end of found numeric option
  else
    { // not numeric option, try alpha
    int iResult = FindBaseAlphaOption (OptionName, ChatAlphaOptionsTable, iItem);
    if (iResult == eOK)
      {

      // alpha option

      if (m_CurrentPlugin &&
          (ChatAlphaOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_READ))
    	  return vaResult;  // not available to plugin

      CString strValue =  GetBaseAlphaOptionItem (iItem, 
                                                 ChatAlphaOptionsTable,
                                                 NUMITEMS (ChatAlphaOptionsTable),
                                                 (char *) chat_item);

      SetUpVariantString (vaResult, strValue);
      }  // end of found
    }

	return vaResult;
}

long CMUSHclientDoc::SetChatOption(long ID, LPCTSTR OptionName, LPCTSTR Value) 
{
CString strValue = Value;
CChatSocket * chat_item;

  chat_item = GetChatSocket (ID);
  if (!chat_item)
    return eChatIDNotFound;

CString strOptionName = OptionName;

  strOptionName.MakeLower();
  strOptionName.TrimLeft ();
  strOptionName.TrimRight ();

int iItem;
int iResult = FindBaseOption (strOptionName, ChatOptionsTable, iItem);

bool bChanged;

  if (iResult == eOK)
    {
    // this is a numeric option
    
    // for boolean options, accept "y" or "n"
    if (ChatOptionsTable [iItem].iMinimum == 0 &&
      ChatOptionsTable [iItem].iMaximum == 0)
      {
      if (strValue == "Y" || strValue == "y")
        Value = "1";
      else if (strValue == "N" || strValue == "n")
        Value = "0";
      }

    if (!IsNumber (Value, true))
       return eOptionOutOfRange;

    long iValue = atol (Value);

    if (m_CurrentPlugin &&
        (ChatOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_WRITE))
    	return ePluginCannotSetOption;  // not available to plugin

    if (ChatOptionsTable [iItem].iFlags & OPT_CANNOT_WRITE)
    	return ePluginCannotSetOption;  // not available for writing at all    

    iResult = SetBaseOptionItem (iItem,
                        ChatOptionsTable,
                        NUMITEMS (ChatOptionsTable),
                        (char *) chat_item, 
                        iValue,
                        bChanged);

    if (iResult == eOK && bChanged)
      {
      if (strOptionName == "can_send_commands")
         if (iValue)
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("You can now send %s commands", 
                (LPCTSTR) m_strOurChatName));
      else
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("You can no longer send %s commands", 
                (LPCTSTR) m_strOurChatName));
      else
      if (strOptionName == "can_send_files")
         if (iValue)
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("You can now send %s files", 
                (LPCTSTR) m_strOurChatName));
      else
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("You can no longer send %s files", 
                (LPCTSTR) m_strOurChatName));
      else
      if (strOptionName == "can_snoop")
         if (iValue)
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("You can now snoop %s", 
                (LPCTSTR) m_strOurChatName));
      else
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("You can no longer snoop %s", 
                (LPCTSTR) m_strOurChatName));
      else
      if (strOptionName == "ignore")
         if (iValue)
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("%s is ignoring you", 
                (LPCTSTR) m_strOurChatName));
      else
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("%s is no longer ignoring you", 
                (LPCTSTR) m_strOurChatName));
      else
      if (strOptionName == "private")
         if (iValue)
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("%s has marked your connection as private", 
                (LPCTSTR) m_strOurChatName));
      else
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("%s has marked your connection as public", 
                (LPCTSTR) m_strOurChatName));

      }  // end of succesfully changing the option

    return iResult;

    }  // end of found
  else
    { // not numeric option, try alpha
    int iResult = FindBaseAlphaOption (strOptionName, ChatAlphaOptionsTable, iItem);
    if (iResult == eOK)
      {

      // alpha option

      if (m_CurrentPlugin &&
          (ChatAlphaOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_WRITE))
    	  return ePluginCannotSetOption;  // not available to plugin

      if (ChatAlphaOptionsTable [iItem].iFlags & OPT_CANNOT_WRITE)
    	  return ePluginCannotSetOption;  // not available for writing at all    

      // ------ preliminary validation before setting the option

      // set the option now

      iResult = SetBaseAlphaOptionItem (iItem,
                        ChatAlphaOptionsTable,
                        NUMITEMS (ChatAlphaOptionsTable),
                        (char *) chat_item,  
                        strValue,
                        bChanged);

    if (iResult == eOK && bChanged)
      {

      if (strOptionName == "group")
        if (strlen (Value) > 0)
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("%s has added you to the group %s", 
                (LPCTSTR) m_strOurChatName, Value));
        else
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("%s has removed you from the chat group", 
                (LPCTSTR) m_strOurChatName));
      }  // end of succesfully changing the option

      return iResult;
      }  // end of found alpha option
    }  // end of not numeric option

 return eUnknownOption;
}

// for scripting, a simple routine to convert a name to an ID
// (so we can use the same alias to work on a name or number)

long CMUSHclientDoc::ChatGetID(LPCTSTR Who) 
{

CString strWho = Who;

  strWho.TrimLeft ();
  strWho.TrimRight ();

  if (strWho.IsEmpty ())
    return 0;   // can't find a blank name

  // for scripts, assume if they supply a number they already know the ID
  if (IsNumber (strWho))
    {
    // however, still warn if they aren't there
    long id = atol (strWho);
    if (GetChatSocket (id) == NULL)
      {
      ChatNote (eChatInformation, 
                TFormat ("Chat ID %ld is not connected.", 
                        id));
      return 0;
      }
    else
      return id;
    }    // end of numeric person

  for (POSITION chatpos = m_ChatList.GetHeadPosition (); chatpos; )
    {
    CChatSocket * pSocket = m_ChatList.GetNext (chatpos);
    if (pSocket->m_iChatStatus == eChatConnected)
      if (pSocket->m_strRemoteUserName.CompareNoCase (strWho) == 0)
        return pSocket->m_iChatID;
    } // end of all chat sessions

  ChatNote (eChatInformation, 
            TFormat ("Cannot find connection \"%s\".", 
                    (LPCTSTR) strWho));

  return 0;  // not found
}

// "chats" the clipboard to a remote user (eg. for pasting aliases etc.)
long CMUSHclientDoc::ChatPasteText(long ID) 
{

  // quiet failure on ID zero - probably caused by a ChatGetID lookup failure
  if (ID == 0)
    return eChatIDNotFound;

  CChatSocket * pSocket = GetChatSocket (ID); // find them

  if (!pSocket)
    {
    ChatNote (eChatOutgoingPersonal, TFormat ("Chat ID %i is not connected.", ID));
	  return eChatIDNotFound;
    }

CString strContents; 

  if (!GetClipboardContents (strContents, m_bUTF_8, false))
    return eClipboardEmpty; // can't do it

CString strMessage;

  strMessage = TFormat ("\n%s pastes to you: \n\n%s%s%s%s\n",
                            (LPCTSTR) m_strOurChatName,
                            (LPCTSTR) AnsiCode (ANSI_BOLD),  
                            (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                            (LPCTSTR) strContents,
                            (LPCTSTR) AnsiCode (ANSI_TEXT_RED)   
                             );

  pSocket->SendChatMessage (CHAT_TEXT_PERSONAL, strMessage);
  ChatNote (eChatOutgoingPersonal,
            TFormat ("You paste to %s: \n\n%s%s%s%s", 
                      (LPCTSTR) pSocket->m_strRemoteUserName, 
                      (LPCTSTR) AnsiCode (ANSI_BOLD),  
                      (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                      (LPCTSTR) strContents,
                      (LPCTSTR) AnsiCode (ANSI_TEXT_RED)));   
    return eOK;

}

long CMUSHclientDoc::ChatPasteEverybody() 
{
CString strContents; 

  if (!GetClipboardContents (strContents, m_bUTF_8, false))
    return eClipboardEmpty; // can't do it

CString strMessage;

  strMessage = TFormat ("\n%s pastes to everybody: \n\n%s%s%s%s\n",
                            (LPCTSTR) m_strOurChatName,
                            (LPCTSTR) AnsiCode (ANSI_BOLD),  
                            (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                            (LPCTSTR) strContents,
                            (LPCTSTR) AnsiCode (ANSI_TEXT_RED)   
                             );

  if (SendChatMessageToAll (CHAT_TEXT_EVERYBODY,
                                  strMessage,
                                  true,    // unless ignoring them
                                  false,
                                  false,
                                  0,    // don't ignore us
                                  "",   // no group
                                  0))   // use our stamp
  {
  ChatNote (eChatOutgoingEverybody,
            TFormat ("You paste to everybody: \n\n%s%s%s%s", 
                      (LPCTSTR) AnsiCode (ANSI_BOLD),  
                      (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                      (LPCTSTR) strContents,
                      (LPCTSTR) AnsiCode (ANSI_TEXT_RED)));   
  return eOK;
  }
else
  {
  ChatNote (eChatOutgoingEverybody, 
            "No (relevant) chat connections.");
  return eNoChatConnections;
  }


}


long CMUSHclientDoc::ChatSendFile(long ID, LPCTSTR FileName) 
{

  // quiet failure on ID zero - probably caused by a ChatGetID lookup failure
  if (ID == 0)
    return eChatIDNotFound;

  CChatSocket * pSocket = GetChatSocket (ID); // find them

  if (!pSocket)
    {
    ChatNote (eChatFile, TFormat ("Chat ID %i is not connected.", ID));
	  return eChatIDNotFound;
    }

  if (pSocket->m_bDoingFileTransfer)
    {
    if (pSocket->m_bSendFile)
      ChatNote (eChatFile, TFormat ("Already sending file %s",
                  (LPCTSTR) pSocket->m_strOurFileName));
    else
      ChatNote (eChatFile, TFormat ("Already receiving file %s",
                  (LPCTSTR) pSocket->m_strOurFileName));

    return eAlreadyTransferringFile;
    }

  CString strName = FileName;

  // if no file name, put up standard file dialog
  if (strName.IsEmpty ())
    {

	  CFileDialog	dlg(TRUE,						// TRUE for FileOpen
					  "",						// default extension
					  "",						// initial file name
					  OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
					  "All files (*.*)|*.*||");
	  
    dlg.m_ofn.lpstrTitle = "Select file to send";
	  
    ChangeToFileBrowsingDirectory ();
    int nResult = dlg.DoModal();
    ChangeToStartupDirectory ();

    if (nResult != IDOK)
		  return eFileNotFound;

    strName = dlg.GetPathName();

    }   // end of no file name supplied

  // remember file to open
  pSocket->m_strOurFileName = strName;

         
  try
    {
    pSocket->m_pFile = new CFile (strName, CFile::modeRead | CFile::shareDenyWrite); // open file
    pSocket->m_pFileBuffer = new unsigned char [pSocket->m_iFileBlockSize]; // get buffer    
    pSocket->m_iFileSize = pSocket->m_pFile->GetLength ();
    
    } // end of try block

  catch (CFileException * e)
    {
    ChatNote (eChatFile, TFormat ("File %s cannot be opened.", (LPCTSTR) strName));
    e->Delete ();
    // reset the two fields we changed so far
    pSocket->m_strOurFileName.Empty ();
    pSocket->m_iFileSize = 0;

    delete pSocket->m_pFile;    // in case it was set up
    delete [] pSocket->m_pFileBuffer;  // and get rid of buffer

    return eFileNotFound;
    } // end of catching a file exception

  // find last part of file name (ie. actual file name, not full path)
  pSocket->m_strSenderFileName = pSocket->m_pFile->GetFileName ();

  // ask them to receive the file
  pSocket->SendChatMessage (CHAT_FILE_START, 
      TFormat ("%s,%ld",
                (LPCTSTR) pSocket->m_strSenderFileName,
                pSocket->m_iFileSize));

  // get ready for transfer
  pSocket->m_tStartedFileTransfer = CTime::GetCurrentTime();  // when started
  pSocket->m_bSendFile = true;   // we are sending
  pSocket->m_bDoingFileTransfer = true;           
  pSocket->m_iBlocksTransferred = 0;   // no blocks yet
  // add block-size minus one to allow for final partial block
  pSocket->m_iFileBlocks = (pSocket->m_iFileSize + pSocket->m_iFileBlockSize - 1L) /
                  pSocket->m_iFileBlockSize;

  shsInit  (&pSocket->m_shsInfo);
  double K = pSocket->m_iFileSize / 1024.0;

  ChatNote (eChatFile, TFormat (
              "Initiated transfer of file %s, %ld bytes (%1.1f Kb).", 
                    (LPCTSTR) strName, 
                    pSocket->m_iFileSize,
                    K));
	return eOK;
}

// abort a file transfer in progress

long CMUSHclientDoc::ChatStopFileTransfer(long ID) 
{

  // quiet failure on ID zero - probably caused by a ChatGetID lookup failure
  if (ID == 0)
    return eChatIDNotFound;

  CChatSocket * pSocket = GetChatSocket (ID); // find them

  if (!pSocket)
    {
    ChatNote (eChatOutgoingPersonal, TFormat ("Chat ID %i is not connected.", ID));
	  return eChatIDNotFound;
    }

  if (!pSocket->m_bDoingFileTransfer)
    return eNotTransferringFile;

  pSocket->StopFileTransfer (true);

	return eOK;
}


VARIANT CMUSHclientDoc::GetHostAddress(LPCTSTR HostName) 
{
  COleSafeArray sa;   // for list

  if (strlen (HostName) > 0)
    {

    LPHOSTENT pHost = gethostbyname (HostName);
    long iCount;

    for (iCount = 0; pHost != NULL && pHost->h_addr_list [iCount] != NULL; iCount++ )
      {};   // end of counting them

    if (iCount) // cannot create empty array dimension
      {
      sa.CreateOneDim (VT_VARIANT, iCount);
  
    // put the addresses into the array
      for (long i = 0; pHost != NULL && pHost->h_addr_list [i] != NULL; i++ )
        {
        COleVariant v (inet_ntoa ( *((in_addr *)pHost->h_addr_list[i])));
        sa.PutElement (&i, &v);
        }      // end of looping through each IP address
      } // end of having at least one
    }   // end of non-blank name

	return sa.Detach ();
}

BSTR CMUSHclientDoc::GetHostName(LPCTSTR IPaddress) 
{
	CString strResult;

  if (strlen (IPaddress) > 0)
    {
    unsigned long ulAddress = inet_addr (IPaddress);
    if (ulAddress != INADDR_NONE)
      {
      LPHOSTENT lpHostEntry = gethostbyaddr ((const char *) &ulAddress, 
                                             sizeof ulAddress, 
                                             AF_INET);
      if (lpHostEntry != NULL)
        strResult =  lpHostEntry->h_name;
      } // end of valid IP address
    }   // end of non-blank name

	return strResult.AllocSysString();
}


BSTR CMUSHclientDoc::GetClipboard() 
{
CString strContents; 

 GetClipboardContents (strContents, m_bUTF_8, false);   // no warning

	return strContents.AllocSysString();
}

long CMUSHclientDoc::ImportXML(LPCTSTR XML) 
{
long iCount = 0;

  CMemFile f ((unsigned char *)  XML, strlen (XML));
  CArchive ar (&f, CArchive::load);

    try
    {

    if (IsArchiveXML (ar))
      {

      UINT iTriggers = 0;
      UINT iAliases = 0;
      UINT iTimers = 0;
      UINT iMacros = 0;
      UINT iVariables = 0;
      UINT iColours = 0;
      UINT iKeypad = 0;
      UINT iPrinting = 0;

      // do it
      Load_World_XML (ar, 
                      // don't load plugins or general world config here  (note, this sets XML_OVERWRITE)
                      (unsigned long) ~(XML_PLUGINS | XML_NO_PLUGINS | XML_GENERAL), 
                      0,          // load flags
                      &iTriggers,  
                      &iAliases,   
                      &iTimers,    
                      &iMacros,    
                      &iVariables, 
                      &iColours,   
                      &iKeypad,    
                      &iPrinting);  

      iCount =  iTriggers +  
                iAliases +   
                iTimers +    
                iMacros +    
                iVariables + 
                iColours +   
                iKeypad +    
                iPrinting;  

      }
    else
      iCount = -1;    // not in XML

     } // end of try block
  catch (CArchiveException* ) 
    {
    iCount = -1;    // error parsing XML
    }

	return iCount;
}


long CMUSHclientDoc::DoCommand(LPCTSTR Command) 
{

int nID = StringToCommandID (Command);

  if (nID == 0)
    return eNoSuchCommand;

  Frame.PostMessage(WM_COMMAND, nID, 0);

	return eOK;
}


VARIANT CMUSHclientDoc::GetInternalCommandsList() 
{

  COleSafeArray sa;   // for list

  long iCount;

  // count them
	for (iCount = 0; CommandIDs [iCount].iCommandID; iCount++)
    {};   // end of counting them

  sa.CreateOneDim (VT_VARIANT, iCount);

// put the addresses into the array
	for (long i = 0; CommandIDs [i].iCommandID; i++)
    {
    COleVariant v (CommandIDs [i].sCommandName);
    sa.PutElement (&i, &v);
    }      // end of looping through each command

	return sa.Detach ();
}


long CMUSHclientDoc::GetNotepadLength(LPCTSTR Title) 
{
CTextDocument * pTextDoc = FindNotepad (Title);

int iLength = 0;

  if (pTextDoc)
    {
    // find the view
    POSITION pos=pTextDoc->GetFirstViewPosition();

    if (pos)
      {
      CView* pView = pTextDoc->GetNextView(pos);
      if (pView->IsKindOf(RUNTIME_CLASS(CTextView)))
        {
        CTextView* pmyView = (CTextView*)pView;
        iLength = pmyView->GetWindowTextLength ();
        } // end of having the right type of view
      }   // end of having a view
    } // end of having an existing notepad document

	return iLength;
}

BSTR CMUSHclientDoc::GetNotepadText(LPCTSTR Title) 
{
CString strResult;
CTextDocument * pTextDoc = FindNotepad (Title);

  if (pTextDoc)
    {
    // find the view
    POSITION pos=pTextDoc->GetFirstViewPosition();

    if (pos)
      {
      CView* pView = pTextDoc->GetNextView(pos);
      if (pView->IsKindOf(RUNTIME_CLASS(CTextView)))
        {
        CTextView* pmyView = (CTextView*)pView;
        pmyView->GetWindowText (strResult);
        } // end of having the right type of view
      }   // end of having a view
    } // end of having an existing notepad document


	return strResult.AllocSysString();
}


/*
 Methods:

  0 = nothing
  1 = invert
  2 = lighter
  3 = darker
  4 = less saturation
  5 = more saturation
  other = nothing
*/

long CMUSHclientDoc::AdjustColour(long Colour, short Method) 
{
return ::AdjustColour (Colour, Method);
}

/*
Type:
  0 = trigger
  1 = alias
  2 = timer
  3 = macro
  4 = variable
  5 = keypad

  */

BSTR CMUSHclientDoc::ExportXML(short Type, LPCTSTR Name) 
{
	CString strResult;
  CString strName = Name;

  // trim spaces, force name to lower-case
  CheckObjectName (strName, false);

  char * p = NULL;

  try
    {
    CMemFile f;      // open memory file for writing
    CArchive ar(&f, CArchive::store);


    // see if trigger exists, if not return EMPTY

    switch (Type)
      {
      case 0:   // trigger
        {
        CTrigger * t;
        if (GetTriggerMap ().Lookup (strName, t))
          {
          Save_Header_XML (ar, "triggers", false);
          Save_One_Trigger_XML (ar, t);
          Save_Footer_XML (ar, "triggers");
          } // end of item existing
        }
        break;

      case 1:   // alias
        {
        CAlias * t;
        if (GetAliasMap ().Lookup (strName, t))
          {
          Save_Header_XML (ar, "aliases", false);
          Save_One_Alias_XML (ar, t);
          Save_Footer_XML (ar, "aliases");
          } // end of item existing
        }
        break;

      case 2:   // timer
        {
        CTimer * t;
        if (GetTimerMap ().Lookup (strName, t))
          {
          Save_Header_XML (ar, "timers", false);
          Save_One_Timer_XML (ar, t);
          Save_Footer_XML (ar, "timers");
          } // end of item existing
        }
        break;

      case 3:   // macro
        {
        for (int i = 0; i < NUMITEMS (strMacroDescriptions); i++)
          {
          if (strMacroDescriptions [i].CompareNoCase (strName) == 0)
            {
            Save_Header_XML (ar, "macros", false);
            Save_One_Macro_XML (ar, i);
            Save_Footer_XML (ar, "macros");
            } // end of item existing
          } // end of finding which one
        }
        break;

      case 4:   // variable
        {
        CVariable * t;
        if (GetVariableMap ().Lookup (strName, t))
          {
          Save_Header_XML (ar, "variables", false);
          Save_One_Variable_XML (ar, t);
          Save_Footer_XML (ar, "variables");
          } // end of item existing
        }
        break;

      case 5:   // keypad
        {
        for (int i = 0; i < NUMITEMS (strKeypadNames); i++)
          {
          if (strKeypadNames [i].CompareNoCase (strName) == 0)
            {
            Save_Header_XML (ar, "keypad", false);
            Save_One_Keypad_XML (ar, i);
            Save_Footer_XML (ar, "keypad");
            } // end of item existing
          } // end of finding which one

        }
        break;

      } // end of switch

    ar.Close();

    int nLength = f.GetLength ();
    p = (char *) f.Detach ();

    strResult = CString (p, nLength);

    free (p);   // remove memory allocated in CMemFile
    p = NULL;

    }   // end of try block

  catch (CException* e)
	  {
    if (p)
      free (p);   // remove memory allocated in CMemFile
	  e->Delete();
    strResult.Empty ();
	  }   // end of catch


	return strResult.AllocSysString();
}

void CMUSHclientDoc::Hyperlink(LPCTSTR Action, 
                               LPCTSTR Text, 
                               LPCTSTR Hint, 
                               LPCTSTR TextColour, 
                               LPCTSTR BackColour, 
                               BOOL URL) 
{
  // return if attempt to do tell (or note) before output buffer exists
  if (m_pCurrentLine == NULL)
    return;

  // don't muck around if empty message
  if (Action [0] == 0)
    return;

  // If current line is not a note line, force a line change (by displaying
  // an empty string), so that the style change is on the note line and not
  // the back of the previous line. This has various implications, including
  // the way that "bleed colour to edge" will work. Otherwise it bleeds the
  // forthcoming note colour to the edge of the previous (mud) line.

  if ((m_pCurrentLine->flags & NOTE_OR_COMMAND) != COMMENT)
    DisplayMsg ("", 0, COMMENT);

  COLORREF forecolour = m_iHyperlinkColour,
           backcolour;

  if (m_bNotesInRGB)
    backcolour = m_iNoteColourBack;
  else
    {
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      if (m_bCustom16isDefaultColour)
        backcolour =  m_customback [15];
      else
        backcolour =  m_normalcolour [BLACK];
      } // not same colour
    else
  	  backcolour =  m_customback [m_iNoteTextColour];
    }

  SetColour (TextColour, forecolour);
  SetColour (BackColour, backcolour);

  // change to underlined hyperlink
  AddStyle (COLOUR_RGB | 
            (URL ? ACTION_HYPERLINK : ACTION_SEND) | 
            UNDERLINE, 
            forecolour, 
            backcolour, 0, 
            GetAction (Action, 
                        Hint [0] == 0 ? Action : Hint, 
                        ""));

  // output the link text
  if (strlen (Text) > 0)
    AddToLine (Text, 0);
  else
    AddToLine (Action, 0);

  // add another style to finish the hyperlink

  if (m_bNotesInRGB)
    AddStyle (COLOUR_RGB, m_iNoteColourFore, m_iNoteColourBack, 0, NULL);
  else
    {
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      if (m_bCustom16isDefaultColour)
        AddStyle (COLOUR_CUSTOM, 15, BLACK, 0, NULL);
      else
        AddStyle (COLOUR_ANSI, WHITE, BLACK, 0, NULL);
      } // not same colour
    else
      AddStyle (COLOUR_CUSTOM, m_iNoteTextColour, BLACK, 0, NULL);
    } // not RGB

}

/*

  Notes text style:

  0 = normal
  1 = bold
  2 = underline
  4 = blink (italic)
  8 = inverse

  */

void CMUSHclientDoc::NoteStyle(short Style) 
{
  m_iNoteStyle = Style & TEXT_STYLE;    // only want those bits
}

short CMUSHclientDoc::GetNoteStyle() 
{
	return m_iNoteStyle & TEXT_STYLE;
}


long CMUSHclientDoc::GetFrame() 
{
	return (long) App.m_pMainWnd->m_hWnd;
}

// create a new array
long CMUSHclientDoc::ArrayCreate(LPCTSTR Name) 
{

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it != GetArrayMap ().end ())
    return eArrayAlreadyExists;

  tStringToStringMap * m = new tStringToStringMap;

  GetArrayMap ().insert (make_pair (Name, m));

  return eOK;

  } // end of CMUSHclientDoc::ArrayCreate

// delete an array by name
long CMUSHclientDoc::ArrayDelete(LPCTSTR Name) 
{

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return eArrayDoesNotExist;

  it->second->clear ();  // remove all entries

  delete it->second;     // delete the pointer

  GetArrayMap ().erase (it);  // erase from map of arrays

	return eOK;
  }  // end of CMUSHclientDoc::ArrayDelete

// list all known arrays into an array of variants

VARIANT CMUSHclientDoc::ArrayListAll() 
{
  COleSafeArray sa;   // for array list

  long iCount = 0;
  
  // put the arrays into the array
  if (!GetArrayMap ().empty ())    // cannot create empty dimension
    {
    sa.CreateOneDim (VT_VARIANT, GetArrayMap ().size ());

    for (tStringMapOfMaps::iterator it = GetArrayMap ().begin (); 
         it != GetArrayMap ().end ();
         it++)
           {
            // the array must be a bloody array of variants, or VBscript kicks up
            COleVariant v (it->first.c_str ());
            sa.PutElement (&iCount, &v);
            iCount++;
           }

    } // end of having at least one

	return sa.Detach ();
}   // end of CMUSHclientDoc::ArrayListAll

// list the keys for a particular array

VARIANT CMUSHclientDoc::ArrayListKeys(LPCTSTR Name) 
{
  COleSafeArray sa;   // for array list

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return sa.Detach ();   // empty array if array does not exist

  long iCount = 0;
  
  // put the keys into the array
  if (!it->second->empty ())    // cannot create empty dimension
    {
    sa.CreateOneDim (VT_VARIANT, it->second->size ());

    for (tStringToStringMap::iterator i = it->second->begin (); 
         i != it->second->end ();
         i++)
           {
            // the array must be a bloody array of variants, or VBscript kicks up
            COleVariant v (i->first.c_str ());
            sa.PutElement (&iCount, &v);
            iCount++;
           }

    } // end of having at least one

	return sa.Detach ();
} // end of CMUSHclientDoc::ArrayListKeys

// lists the values (not the keys) in a particular array

VARIANT CMUSHclientDoc::ArrayListValues(LPCTSTR Name) 
{
  COleSafeArray sa;   // for array list

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return sa.Detach ();   // empty array if array does not exist

  long iCount = 0;
  
  // put the keys into the array
  if (!it->second->empty ())    // cannot create empty dimension
    {
    sa.CreateOneDim (VT_VARIANT, it->second->size ());

    for (tStringToStringMap::iterator i = it->second->begin (); 
         i != it->second->end ();
         i++)
           {
            // the array must be a bloody array of variants, or VBscript kicks up
            COleVariant v (i->second.c_str ());
            sa.PutElement (&iCount, &v);
            iCount++;
           }

    } // end of having at least one

	return sa.Detach ();
}

// import from a delimited string into an array
// eg.  world.ArrayImport "myarray", "nick,blah,helen,aaaa", ","

// importing is tricky because of the blasted "escaped delimiter"
// so, what I am going to do is replace the escaped delimiter with something else
// then fix import using the ordinary delimiter, then fix up the mess afterwards

long CMUSHclientDoc::ArrayImport(LPCTSTR Name, LPCTSTR Values, LPCTSTR Delimiter) 
{
  // delimiter had better be a single character, other than backslash
  string sDelimiter (Delimiter);

  if (sDelimiter.size () != 1 || sDelimiter == "\\")
     return eBadDelimiter;

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return eArrayDoesNotExist;

  vector<string> v;
  int iDuplicates = 0;

  string sEscapedDelimiter = "\\" + sDelimiter;
  string sValues (Values);  
  string sTemp;

  if (sValues.find (sEscapedDelimiter) != string::npos)
    {
    int iTemp;

    // find a character not in the string
    for (iTemp = 1; iTemp <= 255; iTemp++)
      {
      sTemp = string (1, iTemp);
      if (sValues.find (sTemp) == string::npos)
        break;  // this character isn't used!
      }

    if (iTemp > 255)
      return eCannotImport;

    sValues = FindAndReplace (sValues, sEscapedDelimiter, sTemp);

    } // end of having at least one escaped delimiter in the string

  StringToVector (sValues, v, sDelimiter, false); // don't trim spaces

  if (v.size () & 1)
    return eArrayNotEvenNumberOfValues;

  // insert pairs (key, value) into designated map
  for (vector<string>::iterator i = v.begin (); i != v.end (); )
    {
    string sKey = *i++;
    string sValue = *i++;

    // fix up backslashes and delimiters
    sKey = FindAndReplace (sKey, "\\\\", "\\");
    if (!sTemp.empty ())
      sKey = FindAndReplace (sKey, sTemp, sDelimiter);
    sValue = FindAndReplace (sValue, "\\\\", "\\");
    if (!sTemp.empty ())
      sValue = FindAndReplace (sValue, sTemp, sDelimiter);

    pair<tStringToStringMap::iterator, bool> status = 
        it->second->insert (make_pair (sKey, sValue));

    if (!status.second)
      {
      status.first->second = sValue;
      iDuplicates++;
      }
    }

  if (iDuplicates)
    return eImportedWithDuplicates;

	return eOK;
} // end of CMUSHclientDoc::ArrayImport

// get an element from an array by key
VARIANT CMUSHclientDoc::ArrayGet(LPCTSTR Name, LPCTSTR Key) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return vaResult;     // empty

  tStringToStringMap::iterator i = it->second->find (Key);

  if (i == it->second->end ())
    return vaResult;     // empty

  SetUpVariantString (vaResult, i->second.c_str ());
  
	return vaResult;
  }  // end of CMUSHclientDoc::ArrayGet

// set an array item by key - overwriting is allowed but warned in the return value
long CMUSHclientDoc::ArraySet(LPCTSTR Name, LPCTSTR Key, LPCTSTR Value) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return eArrayDoesNotExist;   

  pair<tStringToStringMap::iterator, bool> status = 
      it->second->insert (make_pair (Key, Value));

  if (!status.second)
    {
    status.first->second = Value;
    return eSetReplacingExistingValue;
    }

	return eOK;
} // end of CMUSHclientDoc::ArraySet

// exports an entire array as a delimited string
// it is an error if the delimiter is in the array keys or values

VARIANT CMUSHclientDoc::ArrayExport(LPCTSTR Name, LPCTSTR Delimiter) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    {
    SetUpVariantLong (vaResult, eArrayDoesNotExist);
	  return vaResult;
    }

  // empty array gives empty string
  if (it->second->empty ())
    {
    SetUpVariantString (vaResult, "");
    return vaResult;
    }

  // delimiter had better be a single character, other than backslash
  string sDelimiter (Delimiter);

  if (sDelimiter.size () != 1 || sDelimiter == "\\")
    {
    SetUpVariantLong (vaResult, eBadDelimiter);
    return vaResult;
    }

  string sResult;
  int iCount = 0;

  for (tStringToStringMap::iterator i = it->second->begin ();
       i != it->second->end ();
       i++)
         {
         string sKey (i->first);
         string sData (i->second);

         // replace backslashes by two of them
         sKey = FindAndReplace (sKey, "\\", "\\\\");
         // replace delimiter by backslash, delimiter
         sKey = FindAndReplace (sKey, sDelimiter, "\\" + sDelimiter);

         // replace backslashes by two of them
         sData = FindAndReplace (sData, "\\", "\\\\");
         // replace delimiter by backslash, delimiter
         sData = FindAndReplace (sData, sDelimiter, "\\" + sDelimiter);

         sResult += sKey + sDelimiter + sData;
         if (++iCount < it->second->size ())
          sResult += sDelimiter;
         }  // end of doing each one


  SetUpVariantString (vaResult, sResult.c_str ());
	return vaResult;
} // end of CMUSHclientDoc::ArrayExport

// export the keys but not the values as a delimited string
VARIANT CMUSHclientDoc::ArrayExportKeys(LPCTSTR Name, LPCTSTR Delimiter) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    {
    SetUpVariantLong (vaResult, eArrayDoesNotExist);
	  return vaResult;
    }

  // empty array gives empty string
  if (it->second->empty ())
    {
    SetUpVariantString (vaResult, "");
    return vaResult;
    }

  // delimiter had better be a single character, other than backslash
  string sDelimiter (Delimiter);

  if (sDelimiter.size () != 1 || sDelimiter == "\\")
    {
    SetUpVariantLong (vaResult, eBadDelimiter);
    return vaResult;
    }

  string sResult;
  int iCount = 0;

  for (tStringToStringMap::iterator i = it->second->begin ();
       i != it->second->end ();
       i++)
     {
     string sKey (i->first);

     // replace backslashes by two of them
     sKey = FindAndReplace (sKey, "\\", "\\\\");
     // replace delimiter by backslash, delimiter
     sKey = FindAndReplace (sKey, sDelimiter, "\\" + sDelimiter);

     sResult += sKey;
     if (++iCount < it->second->size ())
      sResult += sDelimiter;
     }  // end of doing each one

  SetUpVariantString (vaResult, sResult.c_str ());
	return vaResult;
} // end of CMUSHclientDoc::ArrayExportKeys

// clear existing contents from an array
// eg. world.ArrayClear "myarray"

long CMUSHclientDoc::ArrayClear(LPCTSTR Name) 
{

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return eArrayDoesNotExist;

  it->second->clear ();  // remove all entries

	return eOK;
} // end of CMUSHclientDoc::ArrayClear

// delete a keyed item from an array
//  eg. world.ArrayDeleteKey "myarray", "spells"

long CMUSHclientDoc::ArrayDeleteKey(LPCTSTR Name, LPCTSTR Key) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return eArrayDoesNotExist;   

  tStringToStringMap::iterator i = it->second->find (Key);

  if (i == it->second->end ())
    return eKeyDoesNotExist;     

  it->second->erase (i);

  return eOK;
  
} // end of CMUSHclientDoc::ArrayDeleteKey

// quick test to see if an array exists
BOOL CMUSHclientDoc::ArrayExists(LPCTSTR Name) 
{

  return GetArrayMap ().find (Name) != GetArrayMap ().end ();

  } // end of CMUSHclientDoc::ArrayExists

// quick test to see if a key exists
BOOL CMUSHclientDoc::ArrayKeyExists(LPCTSTR Name, LPCTSTR Key) 
{

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return FALSE;   

  return it->second->find (Key) != it->second->end ();

} // end of CMUSHclientDoc::ArrayKeyExists

// return count of all arrays
long CMUSHclientDoc::ArrayCount() 
{
	return GetArrayMap ().size ();
} // end of CMUSHclientDoc::ArrayCount

// return count of elements in a particular array
long CMUSHclientDoc::ArraySize(LPCTSTR Name) 
{

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return 0;   

	return it->second->size ();
}   // end of CMUSHclientDoc::ArraySize

VARIANT CMUSHclientDoc::ArrayGetFirstKey(LPCTSTR Name) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return vaResult;     // empty

  tStringToStringMap::iterator i = it->second->begin ();

  if (i == it->second->end ())
    return vaResult;     // empty

  SetUpVariantString (vaResult, i->first.c_str ());
  
	return vaResult;
}

VARIANT CMUSHclientDoc::ArrayGetLastKey(LPCTSTR Name) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  tStringMapOfMaps::iterator it = GetArrayMap ().find (Name);

  if (it == GetArrayMap ().end ())
    return vaResult;     // empty

  tStringToStringMap::reverse_iterator i = it->second->rbegin ();

  if (i == it->second->rend ())
    return vaResult;     // empty

  SetUpVariantString (vaResult, i->first.c_str ());
  
	return vaResult;
}

// reset time on status bar
void CMUSHclientDoc::ResetStatusTime() 
{
  m_tStatusTime = CTime::GetCurrentTime(); 
}

// enable/disable auto-mapper
void CMUSHclientDoc::EnableMapping(BOOL Enabled) 
{
 m_bMapping =	Enabled != 0;
}

// restores the main frame and brings it to the front
void CMUSHclientDoc::ActivateClient() 
{
  if (Frame.IsIconic ())
    Frame.ShowWindow(SW_RESTORE);

  Frame.SetForegroundWindow();
  Frame.SetFocus();
}

// remove backtracks from a speedwalk string
BSTR CMUSHclientDoc::RemoveBacktracks(LPCTSTR Path) 
{
	CString strResult;

  // first convert to individual items separated by newlines

  CString strWalk = DoEvaluateSpeedwalk (Path);

  // if string empty, or has an error message, just return it
  if (strWalk.IsEmpty () || strWalk.Left (1) == "*")
	  return strWalk.AllocSysString();

  // convert speedwalk string into a nice vector we can walk
  vector<string> w;   // input

  StringToVector ((LPCTSTR) strWalk, w, ENDLINE);

  // empty? ah well, can't be many backtracks to remove
  if (w.empty ())
	  return strResult.AllocSysString();

  // we will push each item onto our queue (stack) and look for
  // backtracks on the top of it

  deque<string> q;    // output
  MapDirectionsIterator it;    // for looking up reverses

  for (vector<string>::const_iterator wi = w.begin ();
       wi != w.end ();
       wi++)
     {
     string sThisDirection = *wi;
     // convert back to a single character if possible
     it = MapDirectionsMap.find (tolower (sThisDirection));
     if (it != MapDirectionsMap.end ())
       sThisDirection = it->second.m_sDirectionToLog;

     // if output stack is empty we can hardly remove a backtrack
     if (q.empty ())
       q.push_back (sThisDirection); // so just add this one
     else
       {
       string sTop = q.back ();
       it = MapDirectionsMap.find (sTop);
       // if new entry is inverse of top of stack, discard both
       if (it != MapDirectionsMap.end () && 
           it->second.m_sReverseDirection == sThisDirection)
         q.pop_back ();
       else
         q.push_back (sThisDirection);
       }  // end of stack not empty

     }  // end of looking at each direction in the input string

  // empty? must have cleaned everything up
  if (q.empty ())
	  return strResult.AllocSysString();

  string sPrev;
  int iCount = 0;
  string sDirection;

  for (deque<string>::const_iterator di = q.begin ();
      di != q.end ();
      di++)
    {
    sDirection = trim (*di);

    if (sDirection.empty ())
      continue;

//  multiple length movements must be put in brackets
//   (eg. up, down, ne, nw)

    if (sDirection.size () > 1)
      sDirection = "(" + sDirection + ")";

    if (sDirection == sPrev && iCount < 99)
      iCount++;
    else
      {
      // output previous speedwalk
      if (!sPrev.empty ())
        {
        if (iCount > 1)
          strResult += CFormat ("%i%s", iCount, sPrev.c_str ());
        else
          strResult += sPrev.c_str ();
        strResult += " ";
        }
      sPrev = sDirection;
      iCount = 1;
      } // end of not multiple identical speedwalks

    } // end of pulling entries out of new queue

  // output final speedwalk
  if (!sPrev.empty ())
    {
    if (iCount > 1)
      strResult += CFormat ("%i%s", iCount, sPrev.c_str ());
    else
      strResult += sPrev.c_str ();
    strResult += " ";
    }

  return strResult.AllocSysString();

  }

/*

  arrays test:

arraycreate "a"
arrayimport "a", "nick,blah,helen,aaaa", ","
note arrayexport ("a", "|")
note arrayget ("a", "nick")
arrayset "a", "nick", "fish"
arrayset "a", "c", "c-value"
arrayset "a", "z", "oh, nothing"

arrayset "a", "1", "number 1"
arrayset "a", "11", "number 3"
arrayset "a", "2", "number 2"
  
arrayset "a", "nick,has a \\in his house", " with,fish,and\\,chips "
note arrayexport ("a", ",")
arraycreate "b"
arrayimport "b", arrayexport ("a", ","), "," 
world.debug "arrays"
      
Note ArrayGetFirstKey ("a")
Note ArrayGetLastKey ("a")


note arrayget ("a", "nick")

dim keyList

keyList = World.ArrayListKeys ("a")

If Not IsEmpty (keyList) Then
  
  For Each v In keyList 
    world.note "key = " & v
  Next

End If

arraydeletekey "a", "nick"
note arrayexport ("a", "|")

note "a exists = " & arrayexists ("a")
note "b exists = " & arrayexists ("b")
note "a/helen exists = " & arraykeyexists ("a", "helen")
note "a/jack exists = " & arraykeyexists ("a", "jack")

note arrayexportkeys ("a", ",")

note "count of arrays = " & arraycount
note "size of a = " & arraysize ("a")


ArrayCreate "spells"
ArrayCreate "onespell"

ArrayClear "spells"
ArraySet "spells", "dragonskin", "45"

ArrayClear "onespell"
ArraySet "onespell", "mana",  "45"
ArraySet "onespell", "wearoff", "Your flesh sheds its draconian aspects."
ArraySet "onespell", "hitvict", "Your flesh changes to emulate the scaly skin of a dragon."

ArraySet "spells", "dragonskin", ArrayExport ("onespell", "~")

ArrayClear "onespell"
ArraySet "onespell", "mana",  "75"
ArraySet "onespell", "wearoff", "The ethereal funnel about you ceases to exist."
ArraySet "onespell", "hitvict", "An aura surrounds you, channeling violent energies in your direction!"

ArraySet "spells", "ethereal funnel", ArrayExport ("onespell", "~")

Note ArrayExport ("spells", "=")

world.debug "arrays"

ArrayClear "onespell"
ArrayImport "onespell", ArrayGet ("spells",  "ethereal funnel"), "~"

Note "hitvict message for ethereal funnel is: " & _
   ArrayGet ("onespell", "hitvict")



arraycreate "a"
arrayclear "a"
arrayset "a", "nick,has a \in his house", " with,fish,and\,chips "
note "'" &  arrayexport ("a", ",") & "'"

x = arrayexport ("a", ",")
arraycreate "b"
arrayimport "b", x, ","
note "'" &  arrayexport ("b", ",") & "'"

  */


VARIANT CMUSHclientDoc::GetTriggerWildcard(LPCTSTR TriggerName, LPCTSTR WildcardName) 
{
CString strTriggerName = TriggerName;

CTrigger * trigger_item;

	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_NULL;

  // trim spaces from name, make lower-case
  CheckObjectName (strTriggerName, false);

  vaResult.vt = VT_EMPTY;

  // see if trigger exists, if not return EMPTY
  if (!GetTriggerMap ().Lookup (strTriggerName, trigger_item))
	  return vaResult;

  if (WildcardName [0] != 0 && trigger_item->regexp)
    {
    CString strResult = trigger_item->regexp->GetWildcard (WildcardName).c_str ();

    SetUpVariantString (vaResult, strResult);
    }
	return vaResult;
}

VARIANT CMUSHclientDoc::GetAliasWildcard(LPCTSTR AliasName, LPCTSTR WildcardName) 
{
CString strAliasName = AliasName;

CAlias * alias_item;

	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_NULL;

  // trim spaces from name, make lower-case
  CheckObjectName (strAliasName, false);

  vaResult.vt = VT_EMPTY;

  // see if alias exists, if not return EMPTY
  if (!GetAliasMap ().Lookup (strAliasName, alias_item))
	  return vaResult;

  if (WildcardName [0] != 0 && alias_item->regexp)
    {
    CString strResult = alias_item->regexp->GetWildcard (WildcardName).c_str ();

    SetUpVariantString (vaResult, strResult);
    }

	return vaResult;
  
}


BSTR CMUSHclientDoc::GetRecentLines(long Count) 
{
	CString strResult;

  // assemble multi-line match text
  int iPos = m_sRecentLines.size () - Count;
  if (iPos < 0)
    iPos = 0;

  string s;

  for (int iCount = 0; 
        iCount < Count &&
        iPos != m_sRecentLines.size ()
        ; iPos++, iCount++
      )
    {
    if (!s.empty ())
      s += '\n';
    s += m_sRecentLines [iPos];
    } // end of assembling text
  strResult = s.c_str ();

	return strResult.AllocSysString();
}


BSTR CMUSHclientDoc::TranslateGerman(LPCTSTR Text) 
{
	CString strResult = FixUpGerman (Text);

	return strResult.AllocSysString();
}


BSTR CMUSHclientDoc::GetWorldID() 
{
	return m_strWorldID.AllocSysString();
}


#ifdef PANE
#if 0

// helper function to find a named pane
CPaneView * CMUSHclientDoc::GetPane (LPCTSTR Name)
  {
  PaneMapIterator pit = m_PaneMap.find (Name);
  if (pit != m_PaneMap.end ())
    return pit->second;

  return NULL;
  }

// test:  /world.CreatePane "nick", "mytitle", 10, 20, 100, 200, 0, 0, 16777215

long CMUSHclientDoc::CreatePane(LPCTSTR Name, 
                                LPCTSTR Title, 
                                long Left, 
                                long Top, 
                                long Width, 
                                long Height, 
                                long Flags, 
                                long TextColour, 
                                long BackColour, 
                                LPCTSTR FontName, 
                                short FontSize, 
                                long MaxLines, 
                                long WrapColumn) 
{

  if (GetPane (Name))
    return ePaneAlreadyExists;

  CRect r (Left, Top, Left + Width, Top + Height);

  /*
  CWnd * pWnd = new CWnd;
  pWnd->Create (NULL,     // class name
                Name, 
                WS_CAPTION  | WS_VISIBLE,
                r,       // rectangle
                &Frame,     // parent
                0,        // ID
                NULL);    // create context

  */

  /*
	CMDIChildWnd* pActiveChild = MDIGetActive();
	if (pActiveChild == NULL)
	{
		TRACE0("Warning: No active document for WindowNew command.\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return eBadParameter;     // command failed
	}

  */

	// otherwise we have a new frame !
	CFrameWnd* pFrame = App.m_pPaneTemplate->CreateNewFrame(this, NULL);
	if (pFrame == NULL)
	{
		TRACE0("Warning: failed to create new frame.\n");
		return eBadParameter;     // command failed
	}


	App.m_pPaneTemplate->InitialUpdateFrame(pFrame, this);

  CPaneView * pPane = (CPaneView *) pFrame->GetActiveView ();

  // remember pane parameters

  pPane->m_sTitle = Title;   
  pPane->m_iLeft = Left;    
  pPane->m_iTop = Top;     
  pPane->m_iWidth = Width;   
  pPane->m_iHeight = Height;  
  pPane->m_iFlags = Flags;
  pPane->m_sName = Name;
  pPane->m_cText = TextColour;
  pPane->m_cBack = BackColour;
  pPane->m_strFont = FontName;
  pPane->m_nHeight = FontSize;
  pPane->m_iMaxLines = MaxLines;
  pPane->m_iMaxWidth = WrapColumn;

  // get some reasonable values here
  if (pPane->m_sTitle.empty ())
    pPane->m_sTitle = Name;     // title defaults to name

  if (pPane->m_strFont.IsEmpty ())
    pPane->m_strFont = MUSHCLIENT_DEFAULT_FONT;

  if (pPane->m_nHeight < 3)
    pPane->m_nHeight = 12;

  if (pPane->m_iMaxLines < 1)
    pPane->m_iMaxLines = 1;

  if (pPane->m_iMaxWidth < 10)
    pPane->m_iMaxWidth = 10;

  // set flags
  pPane->m_bShowBold      = (pPane->m_iFlags & PANE_SHOWBOLD) != 0;
  pPane->m_bShowItalic    = (pPane->m_iFlags & PANE_SHOWITALIC) != 0;
  pPane->m_bShowUnderline = (pPane->m_iFlags & PANE_SHOWUNDERLINE) != 0;

  m_PaneMap [Name] = pPane;

  pFrame->SetWindowText (Title);

  // if a rectangle specified, move window
  if (Left | Top | Width | Height) 
    pFrame->MoveWindow (r);



  /*
  // testing
  if (pView)
    {

    pView->NoteLine (" __  __        ____ _ _            _ ");
    pView->NoteLine ("|  \\/  |_   _ / ___| (_) ___ _ __ | |_ ");
    pView->NoteLine ("| |\\/| | | | | |   | | |/ _ \\ '_ \\| __|");
    pView->NoteLine ("| |  | | |_| | |___| | |  __/ | | | |_ ");
    pView->NoteLine ("|_|  |_|\\__,_|\\____|_|_|\\___|_| |_|\\__|");
    }

  */

	return eOK;
}

long CMUSHclientDoc::ClosePane(LPCTSTR Name) 
{

  CPaneView * pPane = GetPane (Name);
  if (!pPane)
    return ePaneDoesNotExist;

  pPane->GetParentFrame ()->PostMessage (WM_CLOSE);

	return eOK;
}

// adds text to a pane, no linefeed
long CMUSHclientDoc::PaneText(LPCTSTR Name, 
                              LPCTSTR Text, 
                              long TextColour, 
                              long BackColour, 
                              long Style) 
{
  CPaneView * pPane = GetPane (Name);
  if (!pPane)
    return ePaneDoesNotExist;

  pPane->AddText (Text, TextColour, BackColour, Style, false); 

	return eOK;

}

// adds text to a pane, marks end of line
long CMUSHclientDoc::PaneLine(LPCTSTR Name, 
                              LPCTSTR Text, 
                              long TextColour, 
                              long BackColour, 
                              long Style) 
{
  CPaneView * pPane = GetPane (Name);
  if (!pPane)
    return ePaneDoesNotExist;

  pPane->AddText (Text, TextColour, BackColour, Style, true); 

	return eOK;
}


long CMUSHclientDoc::ClearPane(LPCTSTR Name) 
{
  CPaneView * pPane = GetPane (Name);
  if (!pPane)
    return ePaneDoesNotExist;

  pPane->Clear ();

	return eOK;
}

#endif // 0
#endif // PANE


// accelerator finding functor

struct fFindAccelerator : binary_function<ACCEL, ACCEL, bool>
  {
  bool operator() (const ACCEL & a1, const ACCEL & a2) const
    { 
    return (a1.fVirt | FNOINVERT) == (a2.fVirt | FNOINVERT) &&
           a1.key == a2.key;
    };
  }; // end of fFindAccelerator


long CMUSHclientDoc::AcceleratorTo(LPCTSTR Key, LPCTSTR Send, short SendTo) 
{
BYTE fVirt;
WORD key;

  // check they are sending to somewhere valid

  if (SendTo < 0 || SendTo >= eSendToLast)
    return eOptionOutOfRange;

  try
    {
    StringToKeyCode (Key, fVirt, key);
    }
  catch (CException* e)
    {
    e->Delete ();
    return  eBadParameter;
    } // end of catch

       
  fVirt |= FNOINVERT;   // no inverting of menus

	POSITION pos = NULL;
  
  pos = GetFirstViewPosition();
	if (!pos)
		return eBadParameter;
  

  CMDIChildWnd * pMDIframe = NULL;

	// work out what our MDI frame is - that will have the accelerators in it

	CView* pView = GetNextView(pos);
	CWnd * pFrame =  pView->GetParentFrame ();
	pMDIframe = (CMDIChildWnd *) pFrame;

  vector <ACCEL> vAccel;
  HACCEL hAccel;

  hAccel = GetDefaultAccelerator ();

  // none for the document? Try the MDI frame
  if (hAccel == 0 && pMDIframe)
    hAccel = pMDIframe->GetDefaultAccelerator ();

  // try the multi-doc template
  if (hAccel == 0)
      hAccel = App.m_pWorldDocTemplate->m_hAccelTable;

	// find number of them
	int iCount = CopyAcceleratorTable (hAccel, NULL, 10000);
  vAccel.resize (iCount);   // make room, set "size" to iCount

	CopyAcceleratorTable (hAccel, &vAccel [0], iCount);

  ACCEL aWanted;            // new accelerator
  aWanted.fVirt = fVirt;
  aWanted.key = key;
  aWanted.cmd = 0;
  
  // if accelerator already exists, remove it
  vAccel.erase (remove_if (vAccel.begin (), vAccel.end (), bind2nd (fFindAccelerator (), aWanted)),
         vAccel.end ());

  // new in version 3.75 - if no Send string, simply delete accelerator
  if (Send [0]) 
    {
    // the command we will allocate (eg. 12000, 12001 etc.)
    WORD command;

    // make a flags/keystroke combination                                                      
    long virt_plus_key = ((long) fVirt) << 16 | key;

    // see if we have previously allocated that accelerator
    map<long, WORD>::const_iterator it = m_AcceleratorToCommandMap.find (virt_plus_key);

    if (it == m_AcceleratorToCommandMap.end ())
      {
      // check not too many ;)
      if (m_nextAcceleratorCommand >= (ACCELERATOR_FIRST_COMMAND + ACCELERATOR_COUNT))
        return eBadParameter;   // too many of them

      // allocate new one
      command = m_nextAcceleratorCommand++;
      }
    else
      command = it->second;  // re-use command

    aWanted.cmd = command;

    // put command into accelerator table
    vAccel.push_back (aWanted);

    // remember what the command was for that keystroke
    m_AcceleratorToCommandMap [virt_plus_key] = command;

    // remember what to send if they use that command
    m_CommandToMacroMap [command] = Send;

    // and where to send it
    m_CommandToSendToMap [command] = SendTo;

    }  // end of having something to do (ie. not deleting accelerator)

  // create new handle
  hAccel = CreateAcceleratorTable (&vAccel [0], vAccel.size ());

  // destroy old one, if we had one
  if (m_accelerator)
    DestroyAcceleratorTable (m_accelerator);

  // replace accelerator table in document
  m_accelerator = hAccel;

  // test test test

//  App.m_pWorldDocTemplate->m_hAccelTable = hAccel;

	return eOK;
}  // end of CMUSHclientDoc::AcceleratorTo

long CMUSHclientDoc::Accelerator(LPCTSTR Key, LPCTSTR Send) 
{
  return AcceleratorTo (Key, Send, eSendToExecute);
}




// /Accelerator ("F5", "sing")
// /Note (Accelerator ("Alt+Shift+Ctrl+V", "hello"))
// /Note (Accelerator ("Ctrl+F8", "hello"))
// /Note (Accelerator ("Shift+F1", "hello"))

/*

/Note (Accelerator ("Ctrl+Alt+Shift+K", "Ctrl+Alt+Shift+K"))
Note (Accelerator ("Ctrl+K", "Ctrl+K"))
Note (Accelerator ("Shift+Ctrl+K", "Shift+Ctrl+K"))
Note (Accelerator ("Shift+Alt+K", "Shift+Alt+K"))
Note (Accelerator ("Ctrl+Alt+K", "Ctrl+Alt+K"))

table.foreach (AcceleratorList (), print)
*/


VARIANT CMUSHclientDoc::AcceleratorList() 
{
  COleSafeArray sa;   // for wildcard list

  long iCount = m_AcceleratorToCommandMap.size ();

  if (iCount) // cannot create empty array dimension
    {
    sa.CreateOneDim (VT_VARIANT, iCount);
  
    map<long, WORD>::const_iterator it;

    for (iCount = 0, it = m_AcceleratorToCommandMap.begin (); 
         it != m_AcceleratorToCommandMap.end (); it++, iCount++)
      {
      CString key = KeyCodeToString (it->first >> 16, it->first);
      string command = m_CommandToMacroMap [it->second];

      CString strSendTo;
      strSendTo.Format ("\t[%i]", m_CommandToSendToMap [it->second]);

      // for backwards compatability, send to Execute is shown as empty
      if (m_CommandToSendToMap [it->second] == eSendToExecute)
        strSendTo = "";
      // the array must be a bloody array of variants, or VBscript kicks up
      CString s = CFormat ("%s = %s%s", (LPCTSTR) key, 
                          command.c_str (), 
                          (LPCTSTR) strSendTo);
      COleVariant v (s);
      sa.PutElement (&iCount, &v);
      }      // end of looping through each accelerator
    } // end of having at least one

	return sa.Detach ();
}


void CMUSHclientDoc::MapColour(long Original, long Replacement) 
{
m_ColourTranslationMap [Original] = Replacement;
}

// /MapColour (ColourNameToRGB ("blue"), ColourNameToRGB ("gold"))
// /MapColour (0, ColourNameToRGB ("red"))

long CMUSHclientDoc::GetMapColour(long Which) 
{
	return TranslateColour (Which);
}

VARIANT CMUSHclientDoc::MapColourList() 
{
  COleSafeArray sa;   // for wildcard list

  long iCount = m_ColourTranslationMap.size ();

  if (iCount) // cannot create empty array dimension
    {
    sa.CreateOneDim (VT_VARIANT, iCount);
  
    map<COLORREF, COLORREF>::const_iterator it;

    for (iCount = 0, it = m_ColourTranslationMap.begin (); 
         it != m_ColourTranslationMap.end (); it++, iCount++)
      {
      // the array must be a bloody array of variants, or VBscript kicks up
      CString s = CFormat ("%s = %s", 
                          (LPCTSTR) ColourToName (it->first), 
                          (LPCTSTR) ColourToName (it->second));
      COleVariant v (s);
      sa.PutElement (&iCount, &v);
      }      // end of looping through each colour
    } // end of having at least one

	return sa.Detach ();

}

VARIANT CMUSHclientDoc::SpellCheck(LPCTSTR Text) 
{

const char sFunction [] = "spellcheck_string";

	VARIANT vaResult;
	VariantInit(&vaResult);

  if (!App.m_bSpellCheckOK)
    return vaResult;

set<string> errors;

  if (App.m_SpellChecker_Lua)
    {

    lua_settop(App.m_SpellChecker_Lua, 0);   // clear stack

    lua_getglobal (App.m_SpellChecker_Lua, sFunction);  
    if (!lua_isfunction (App.m_SpellChecker_Lua, -1))
      return vaResult;  // cannot spell check string

    lua_pushstring (App.m_SpellChecker_Lua, Text);  // string to be checked

    int narg = lua_gettop(App.m_SpellChecker_Lua) - 1;  // all but the function
    int error = CallLuaWithTraceBack (App.m_SpellChecker_Lua, narg, 1);
  
    if (error)
      {
      LuaError (App.m_SpellChecker_Lua, "Run-time error", sFunction, "world.SpellCheck", "", this);
      return vaResult;  // cannot spell check string - syntax error
      }  

    if (lua_isnumber (App.m_SpellChecker_Lua, -1))
      {
      SetUpVariantLong (vaResult, (long) lua_tonumber (App.m_SpellChecker_Lua, -1));        // no errors
  	  return vaResult;
      }

    // must be table or else return bad result
    if (!lua_istable (App.m_SpellChecker_Lua, -1))
      return vaResult;  // cannot spell check string - syntax error

    // convert returned table into a set
    for (int i = 1; ; i++)
      {
      lua_rawgeti (App.m_SpellChecker_Lua, 1, i);   // get i'th item
      if (lua_isnil (App.m_SpellChecker_Lua, -1))
        break;    // first nil key, leave loop
      // to avoid crashes, ignore table items that are not strings
      if (lua_isstring (App.m_SpellChecker_Lua, -1))
         errors.insert (lua_tostring (App.m_SpellChecker_Lua, -1));
      lua_pop (App.m_SpellChecker_Lua, 1); // remove value
      } // end of looping through table

    // maybe didn't find any errors?
    if (errors.empty ())
      {
      SetUpVariantLong (vaResult, 0);        // no errors
  	  return vaResult;
      }

    // now make array of the errors
    COleSafeArray sa;   // for wildcard list

    sa.CreateOneDim (VT_VARIANT, errors.size ());

    long iCount = 0;

    for (set<string>::const_iterator it = errors.begin (); 
         it != errors.end (); it++, iCount++)
      {
      // the array must be a bloody array of variants, or VBscript kicks up
      COleVariant v (it->c_str ());
      sa.PutElement (&iCount, &v);
      }      // end of looping through each error

	  return sa.Detach ();
    }   // end custom spell check


return vaResult;

} // end of SpellCheck
  

long CMUSHclientDoc::SpellCheckCommand(long StartCol, long EndCol) 
{
  if (!App.m_bSpellCheckOK)
    return -1;

  CEdit * pEdit = NULL;
  CWnd * pWnd = NULL;

  // find command window

  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	  {
		  CSendView* pmyView = (CSendView*)pView;

      // what is the current selection?

      pWnd = pmyView;
      pEdit = & (pmyView->GetEditCtrl());
      break;
      
      }	  // end of being a CSendView
    }   // end of loop through views

  if (pEdit == NULL)
	  return -1;    // couldn't find it

  int nStartChar, 
      nEndChar;

  // get current selection
  pEdit->GetSel (nStartChar, nEndChar); 

  // make wanted selection 1-relative
  if (StartCol > 0)
    StartCol--;

  bool bHaveSelection = EndCol > StartCol &&
                         StartCol >= 0 &&
                         EndCol >= 0;

  // select what the scripter wanted
  if (bHaveSelection)
     pEdit->SetSel (StartCol, EndCol); 

  if (App.m_SpellChecker_Lua)
    {

    lua_settop(App.m_SpellChecker_Lua, 0);   // clear stack

    lua_getglobal (App.m_SpellChecker_Lua, SPELLCHECKFUNCTION);  
    if (!lua_isfunction (App.m_SpellChecker_Lua, -1))
      return true;  // assume ok, what can we do?

    CString strText;
    bool bAll = GetSelection (pEdit, strText);
    
    lua_pushstring (App.m_SpellChecker_Lua, strText);  // string to be checked
    lua_pushboolean (App.m_SpellChecker_Lua, bAll);    // doing all?

    int narg = lua_gettop(App.m_SpellChecker_Lua) - 1;  // all but the function
    int error = CallLuaWithTraceBack (App.m_SpellChecker_Lua, narg, 1);
    
    if (error)
      {
      LuaError (App.m_SpellChecker_Lua, "Run-time error", SPELLCHECKFUNCTION, "Command-line spell-check");
      lua_close (App.m_SpellChecker_Lua);
      App.m_SpellChecker_Lua = NULL;
      App.m_bSpellCheckOK = false;
      return -1;    
      }  

    if (lua_isstring (App.m_SpellChecker_Lua, -1))
      {
      const char * p = lua_tostring (App.m_SpellChecker_Lua, -1);
      if (bAll)
        pEdit->SetSel (0, -1, TRUE);
      pEdit->ReplaceSel (p, true);
      // put original selection back
      pEdit->SetSel (nStartChar, nEndChar); 
      return 1;   // spell checked ok
      }

    // put original selection back
    pEdit->SetSel (nStartChar, nEndChar); 
    return 0;      // they cancelled
    }

  return -1;

} // end of CMUSHclientDoc::SpellCheckCommand


VARIANT CMUSHclientDoc::SpellCheckDlg(LPCTSTR Text) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  if (!App.m_bSpellCheckOK)
    return vaResult;


  if (App.m_SpellChecker_Lua)
    {

    lua_settop(App.m_SpellChecker_Lua, 0);   // clear stack

    lua_getglobal (App.m_SpellChecker_Lua, SPELLCHECKFUNCTION);  
    if (!lua_isfunction (App.m_SpellChecker_Lua, -1))
      return vaResult;  // cannot spell check string

    lua_pushstring (App.m_SpellChecker_Lua, Text);  // string to be checked

    int narg = lua_gettop(App.m_SpellChecker_Lua) - 1;  // all but the function
    int error = CallLuaWithTraceBack (App.m_SpellChecker_Lua, narg, 1);
  
    if (error)
      {
      LuaError (App.m_SpellChecker_Lua, "Run-time error", SPELLCHECKFUNCTION, "world.SpellCheckDlg", "", this);
      return vaResult;  // cannot spell check string - syntax error or dialog cancelled
      }  

    // must be string or else return bad result
    if (!lua_isstring (App.m_SpellChecker_Lua, -1))
      return vaResult;  // shouldn't happen? maybe dialog cancelled

    SetUpVariantString (vaResult, lua_tostring (App.m_SpellChecker_Lua, -1)); 
    }


	return vaResult;
}


BSTR CMUSHclientDoc::MakeRegularExpression(LPCTSTR Text) 
{
	return ConvertToRegularExpression (Text).AllocSysString();
}   // end of CMUSHclientDoc::MakeRegularExpression


void CMUSHclientDoc::MoveMainWindow(long Left, long Top, long Width, long Height) 
{
  Frame.MoveWindow (Left, Top, Width, Height);
}   // end of CMUSHclientDoc::MoveMainWindow


void CMUSHclientDoc::MoveWorldWindow(long Left, long Top, long Width, long Height) 
{
MoveWorldWindowX (Left, Top, Width, Height, 1);   // move first window
}

long CMUSHclientDoc::MoveNotepadWindow(LPCTSTR Title, long Left, long Top, long Width, long Height) 
{
CTextDocument * pTextDoc = FindNotepad (Title);

  if (pTextDoc)
    {
    // get the view
    POSITION pos=pTextDoc->GetFirstViewPosition();

    if (pos)
      {
      CView* pView = pTextDoc->GetNextView(pos);

      if (pView->IsKindOf(RUNTIME_CLASS(CTextView)))
        {
        CTextView* pmyView = (CTextView*)pView;
        pmyView->GetParentFrame ()->MoveWindow (Left, Top, Width, Height);
        return true;            
        } // end of having the right type of view
      }   // end of having a view
    } // end of having an existing notepad document


return false;
}

/*
Colour types:

Scrollbar = 0
Background = 1
Active caption = 2
Inactive caption = 3
Menu = 4
Window = 5
Window frame = 6
Menu text = 7
Window text = 8
Caption text = 9
Active border = 10
Inactive border = 11
Application workspace = 12
Highlight = 13
Highlight text = 14
Button face = 15
Button shadow = 16
Gray text = 17
Button text = 18
Inactive caption text = 19
Button highlight = 20
3D dark shadow = 21
3D light = 22
Info text = 23
Info bk = 24

*/

long CMUSHclientDoc::GetSysColor(long Index) 
{
  return ::GetSysColor (Index);
}

/*
Info codes:

CX screen = 0
CY screen = 1
CX vscroll = 2
CY hscroll = 3
CY caption = 4
CX border = 5
CY border = 6
CX dialog frame = 7
CY dialog frame = 8
CY v thumb = 9
CX h thumb = 10
CX icon = 11
CY icon = 12
CX cursor = 13
CY cursor = 14
CY menu = 15
CX full screen = 16
CY full screen = 17
CY kanji window = 18
Mouse present = 19
CY vscroll = 20
CX hscroll = 21
Debug = 22
Swap button = 23
Reserved1 = 24
Reserved2 = 25
Reserved3 = 26
Reserved4 = 27
CX min = 28
CY min = 29
CX size = 30
CY size = 31
CX frame = 32
CY frame = 33
CX min track = 34
CY min track = 35
CX double click = 36
CY double click = 37
CX icon spacing = 38
CY icon spacing = 39
Menu drop alignment = 40
Pen windows = 41
DBCS enabled = 42
Cmouse buttons = 43
Secure = 44
CX edge = 45
CY edge = 46
CX minspacing = 47
CY minspacing = 48
CX smicon = 49
CY smicon = 50
CY smcaption = 51
CX smsize = 52
CY smsize = 53
CX menusize = 54
CY menusize = 55
Arrange = 56
CX minimized = 57
CY minimized = 58
CX maxtrack = 59
CY maxtrack = 60
CX maximized = 61
CY maximized = 62
Network = 63
Clean boot = 67
CX drag = 68
CY drag = 69
Show sounds = 70
CX menucheck = 71 
CY menucheck = 72
Slow machine = 73
Mid-east enabled = 74
Mouse wheel present = 75
X virtual screen = 76
Y virtual screen = 77
CX virtual screen = 78
CY virtual screen = 79
C monitors = 80
Same display format = 81

  */

long CMUSHclientDoc::GetSystemMetrics(long Index) 
{
  return ::GetSystemMetrics (Index);
}

static void windowPositionHelper (CString & str, const RECT & r)
  {
  str.Format ("%ld,%ld,%ld,%ld", 
              r.left, 
              r.top, 
              r.right - r.left,  // Width
              r.bottom - r.top);  // Height
  } // end of windowPositionHelper

// warning - different in Lua
BSTR CMUSHclientDoc::GetMainWindowPosition() 
{
	CString strResult;

  CWindowPlacement wp;
  Frame.GetWindowPlacement(&wp);  

	windowPositionHelper (strResult, wp.rcNormalPosition);
  return strResult.AllocSysString();
}

// warning - different in Lua
BSTR CMUSHclientDoc::GetNotepadWindowPosition(LPCTSTR Title) 
{
	CString strResult;

  CTextDocument * pTextDoc = FindNotepad (Title);

  if (pTextDoc)
    {
    CWindowPlacement wp;

    // get the view
    POSITION pos=pTextDoc->GetFirstViewPosition();

    if (pos)
      {
      CView* pView = pTextDoc->GetNextView(pos);

      if (pView->IsKindOf(RUNTIME_CLASS(CTextView)))
        {
        CTextView* pmyView = (CTextView*)pView;
        pmyView->GetParentFrame ()->GetWindowPlacement(&wp);
      	windowPositionHelper (strResult, wp.rcNormalPosition);
        } // end of having the right type of view
      }   // end of having a view
    } // end of having an existing notepad document

	return strResult.AllocSysString();
}

// warning - different in Lua
BSTR CMUSHclientDoc::GetWorldWindowPosition() 
{
  return GetWorldWindowPositionX (1); // first window
}



// /UdpSend ("10.0.0.3", 4111, "m,textcolour,5555")

long CMUSHclientDoc::UdpSend(LPCTSTR IP, short Port, LPCTSTR Text) 
{
// timer t ("UdpSend");

int s;  // socket
struct sockaddr_in sa; 	

  // work out address to send to
  memset (&sa, 0, sizeof(sa));
  sa.sin_port = htons (Port);
  sa.sin_family = AF_INET;

  if ((sa.sin_addr.s_addr = inet_addr (IP)) == INADDR_NONE)
    return -1;     // address no good

  // make socket
  s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s == INVALID_SOCKET)
    return WSAGetLastError ();    // cannot create socket

  // allow broadcasts
  int x = 1;
  setsockopt( s, SOL_SOCKET, SO_BROADCAST, (const char *) &x, sizeof x);

  // send it
  long result = sendto (s, 
                        Text, strlen (Text), 
                        0,     // flags
                        (struct sockaddr *) &sa, sizeof sa);

  // close it regardless
  closesocket (s);

  if (result == SOCKET_ERROR)
	  return WSAGetLastError ();    // return error reason

  return 0;
}


// /world.UdpListen "0.0.0.0", 4222, "OnUDP"

long CMUSHclientDoc::UdpListen(LPCTSTR IP, short Port, LPCTSTR Script) 
{

  // I think we'll insist plugins do this
  if (m_CurrentPlugin == NULL)
    return eNotAPlugin;

  // see if we already have one for this port
  map<int, UDPsocket *>::iterator it = m_UDPsocketMap.find (Port);

  // delete existing, so we can use an empty script to achieve this
  if (it != m_UDPsocketMap.end ())
    {
    // don't re-use another plugin's port
    if (it->second->m_strPluginID != m_CurrentPlugin->m_strID)
      return eBadParameter;

    delete it->second;  // delete existing listener
    m_UDPsocketMap.erase (it);
    }

  // no script, cannot do it 
  if (strlen (Script) == 0)
    return eNoNameSpecified;

  // make a new listener
  UDPsocket * pSocket = new UDPsocket (this);

	if (!pSocket->Create (Port, SOCK_DGRAM, FD_READ, IP))
	  {
		delete pSocket;
		return eBadParameter;
	  }     // end of can't create socket

  // remember script to call, and which plugin it is (should be?) in
  pSocket->m_strScript = Script;
  pSocket->m_strPluginID =  m_CurrentPlugin->m_strID;

  // keep a map of them
  m_UDPsocketMap [Port] = pSocket;
	return eOK;
}


long CMUSHclientDoc::NotepadColour (LPCTSTR Title, 
                                      LPCTSTR TextColour, 
                                      LPCTSTR BackgroundColour) 
{
CTextDocument * pTextDoc = FindNotepad (Title);

  if (!pTextDoc)
    return false;

  COLORREF iTextColour,
           iBackColour;

  if (SetColour (TextColour, iTextColour))
    return false;    // can't find colour
  if (SetColour (BackgroundColour, iBackColour))
    return false;    // can't find colour

  pTextDoc->m_textColour = iTextColour;
  pTextDoc->m_backColour = iBackColour;

  pTextDoc->UpdateAllViews (NULL);
  return true;  // did it!

}   // end of CMUSHclientDoc::NotepadColour



long CMUSHclientDoc::NotepadFont(LPCTSTR Title, 
                                 LPCTSTR FontName, 
                                 short Size, 
                                 short Style, 
                                 short Charset) 
{
CTextDocument * pTextDoc = FindNotepad (Title);

  if (!pTextDoc)
    return false;

  // font name provided?
  if (strlen (FontName) > 0)
   pTextDoc->m_strFontName = FontName;

  // size provided?
  if (Size > 0)
   pTextDoc->m_iFontSize = Size;

  if (Style & 1)   // bold
    pTextDoc->m_iFontWeight = FW_BOLD;
  else
    pTextDoc->m_iFontWeight = FW_NORMAL;

  pTextDoc->m_bFontItalic = (Style & 2) != 0;
  pTextDoc->m_bFontUnderline = (Style & 4) != 0;
  pTextDoc->m_bFontStrikeout = (Style & 8) != 0;

  pTextDoc->m_iFontCharset = Charset;

  pTextDoc->SetTheFont ();

  return true;    // done it OK
}

void CMUSHclientDoc::MtSrand(long Seed) 
{
	init_genrand (Seed);
}

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
}

void CMUSHclientDoc::SetChanged(BOOL ChangedFlag) 
{
SetModifiedFlag (ChangedFlag);
}


VARIANT CMUSHclientDoc::UdpPortList() 
{
  COleSafeArray sa;   // for wildcard list

  long iCount = m_UDPsocketMap.size ();

  if (iCount) // cannot create empty array dimension
    {
    sa.CreateOneDim (VT_VARIANT, iCount);
  
    map<int, UDPsocket *>::const_iterator it;

    for (iCount = 0, it = m_UDPsocketMap.begin (); 
         it != m_UDPsocketMap.end (); it++, iCount++)
      {
      // the array must be a bloody array of variants, or VBscript kicks up
      COleVariant v ((long) it->first);
      sa.PutElement (&iCount, &v);
      }      // end of looping through each colour
    } // end of having at least one

	return sa.Detach ();
}


void CMUSHclientDoc::NoteHr() 
{
  // wrap up previous line if necessary
  if (m_pCurrentLine->len > 0)
     StartNewLine (true, 0);

  // mark line as HR line
  m_pCurrentLine->flags = HORIZ_RULE;
  
  StartNewLine (true, 0); // now finish this line

  // refresh views

  for(POSITION pos=GetFirstViewPosition();pos!=NULL;)
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
  	  {
		  CMUSHView* pmyView = (CMUSHView*)pView;

		  pmyView->addedstuff();
	    }	
    }

}

void CMUSHclientDoc::MoveWorldWindowX(long Left, 
                                      long Top, 
                                      long Width, 
                                      long Height, 
                                      short Which) 
{
int i = 0;

  for(POSITION pos=GetFirstViewPosition();pos!=NULL;)
    {
    CView* pView = GetNextView(pos);

    if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
      {
      CSendView* pmyView = (CSendView*)pView;

      i++;

      if (i != Which)
        continue;   // wrong one

      pmyView->GetParentFrame ()->MoveWindow (Left, Top, Width, Height);

      break;

      }	
    }

}

// warning - different in Lua
BSTR CMUSHclientDoc::GetWorldWindowPositionX(short Which) 
{
CString strResult;
int i = 0;

  CWindowPlacement wp;

  for(POSITION pos=GetFirstViewPosition();pos!=NULL;)
    {
    CView* pView = GetNextView(pos);

    if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
      {
      CSendView* pmyView = (CSendView*)pView;

      i++;

      if (i != Which)
        continue;   // wrong one

      pmyView->GetParentFrame ()->GetWindowPlacement(&wp); 
    	windowPositionHelper (strResult, wp.rcNormalPosition);

      break;

      }	
    }

	return strResult.AllocSysString();
}


void CMUSHclientDoc::SetEntity(LPCTSTR Name, LPCTSTR Contents) 
{
  if (strlen (Contents) == 0)
    m_CustomEntityMap.RemoveKey (Name);
  else
    m_CustomEntityMap.SetAt (Name, Contents);
 
}


BSTR CMUSHclientDoc::GetXMLEntity(LPCTSTR Entity) 
{
	CString strResult;
  CString strEntity = Entity;
  CXMLparser parser;

  try 
    {
    strResult = parser.Get_XML_Entity (strEntity);
    }   // end of try
  catch(CException* e)
    {
    e->Delete ();
    } // end of catch

	return strResult.AllocSysString();
}



long CMUSHclientDoc::GetUdpPort(long First, long Last) 
{
  CMUSHclientDoc * pDoc;
  POSITION pos;

  if (First > Last || 
      First < 1 ||
      Last > 65535)
    return 0;   // bad range

  set<long> UsedPortsSet;
  set<long>::const_iterator it;

  map<int, UDPsocket *>::const_iterator i;

  // get all worlds
  for (pos = App.m_pWorldDocTemplate->GetFirstDocPosition(); pos != NULL; )
    {
    pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(pos);

    // remember all UDP ports used by this world
    for (i = pDoc->m_UDPsocketMap.begin ();
         i != pDoc->m_UDPsocketMap.end ();
         i++)
      UsedPortsSet.insert (i->first);

    }      // end of looping through each world

  // now UsedPortsSet contains an entry for every world - find the first free one

  long result;

  for (result = First; result < Last; result++)
    {
    it = UsedPortsSet.find (result);
    if (it == UsedPortsSet.end ())
      return result;    // this one wasn't used
    }

  return 0;   // couldn't find one
}


void CMUSHclientDoc::SetClipboard(LPCTSTR Text) 
{
putontoclipboard (Text);
}


long CMUSHclientDoc::BroadcastPlugin(long Message, LPCTSTR Text) 
{
  CPlugin * pSavedPlugin = m_CurrentPlugin;
  m_CurrentPlugin = NULL;
  long iCount = 0;

  CString strCurrentID;
  CString strCurrentName;

  if (pSavedPlugin)
    {
     strCurrentID = pSavedPlugin->m_strID;
     strCurrentName = pSavedPlugin->m_strName;
    }

  // tell a plugin the message
  for (POSITION pluginpos = m_PluginList.GetHeadPosition(); pluginpos; )
    {
    CPlugin * pPlugin = m_PluginList.GetNext (pluginpos);


    if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
      continue;

    // see what the plugin makes of this,
    pPlugin->ExecutePluginScript (ON_PLUGIN_BROADCAST,
                                  pPlugin->m_dispid_plugin_broadcast,
                                  Message, 
                                  (LPCTSTR) strCurrentID,
                                  (LPCTSTR) strCurrentName,
                                  Text); 

    if (pPlugin->m_dispid_plugin_broadcast != DISPID_UNKNOWN)
      iCount++;

    }   // end of doing each plugin

  m_CurrentPlugin = pSavedPlugin;

	return iCount;
}


long CMUSHclientDoc::PickColour(long Suggested) 
{

  CColourPickerDlg dlg;

  if (Suggested != -1)
    {
    dlg.m_iColour = Suggested;
    dlg.m_bPickColour = true;
    }


  if (dlg.DoModal () != IDOK)
    return -1;

  return dlg.GetColor ();
}


extern int_flags_pair error_descriptions [];

BSTR CMUSHclientDoc::ErrorDesc(long Code) 
{
	CString strResult;
  
  strResult.Format ("%ld", Code);
	
  const int_flags_pair *p;
  for(p = error_descriptions; p->val != NULL; p++) 
    {
    if (p->key == Code)
      {
      strResult = Translate (p->val);
      break;
      }
    } 

	return strResult.AllocSysString();
}



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
}


// simulates input from the MUD, for debugging scripts

void CMUSHclientDoc::Simulate(LPCTSTR Text) 
{
   DisplayMsg(Text, strlen (Text), 0);
}

// resets the IP address, forcing a name-lookup

void CMUSHclientDoc::ResetIP() 
{
  ZeroMemory (&m_sockAddr, sizeof m_sockAddr);
  ZeroMemory (&m_ProxyAddr, sizeof m_ProxyAddr);
}


long CMUSHclientDoc::AddSpellCheckWord(LPCTSTR OriginalWord, LPCTSTR ActionCode, LPCTSTR ReplacementWord) 
{

const char sFunction [] = "spellcheck_add_word";

  // make sure we enabled spell check
  if (!App.m_bSpellCheckOK)
    return eSpellCheckNotActive;

  // word to match on should be supplied
  if (strlen (OriginalWord) <= 0 || strlen (OriginalWord) > 63)
    return eBadParameter;

  // replacement word is optional
  if (strlen (ReplacementWord) > 63)
    return eBadParameter;

  // check action is single letter
  if (strlen (ActionCode) != 1)
    return eUnknownOption;

unsigned char cAction = ActionCode [0];

  // validate action code
  switch (cAction)
    {
    case 'a':
    case 'A':
    case 'c':
    case 'C':
    case 'e':
    case 'i':
      break;

    default:
      return eUnknownOption;
    }


  if (App.m_SpellChecker_Lua)
    {

    lua_settop(App.m_SpellChecker_Lua, 0);   // clear stack

    lua_getglobal (App.m_SpellChecker_Lua, sFunction);  
    if (!lua_isfunction (App.m_SpellChecker_Lua, -1))
      return eSpellCheckNotActive;  // cannot spell check string

    lua_pushstring (App.m_SpellChecker_Lua, OriginalWord);      // word to be added
    lua_pushstring (App.m_SpellChecker_Lua, ActionCode);        // action
    lua_pushstring (App.m_SpellChecker_Lua, ReplacementWord);   // replacement

    int narg = lua_gettop(App.m_SpellChecker_Lua) - 1;  // all but the function
    int error = CallLuaWithTraceBack (App.m_SpellChecker_Lua, narg, 1);
  
    if (error)
      {
      LuaError (App.m_SpellChecker_Lua, "Run-time error", sFunction, "world.AddSpellCheckWord", "", this);
      return eSpellCheckNotActive;  // cannot spell check string - syntax error
      }  

    // if it returns false, there was a problem
    if (lua_type (App.m_SpellChecker_Lua, -1) == LUA_TBOOLEAN)
      {
      int ok = lua_toboolean (App.m_SpellChecker_Lua, -1);
      if (!ok)
        return eBadParameter;
      }

    return eOK;
    }   // end custom spell check


  return eSpellCheckNotActive;

}

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
}

void CMUSHclientDoc::DeleteLines(long Count) 
{

POSITION pos;

/*

  I can't delete lines when in send-to-script, please don't try to make me. ;)

  The problem is that in ProcessPreviousLine we have established the start and end line
  of the paragraph we are processing, if that is deleted by a trigger in the middle, all 
  hell breaks loose.

  */

  if (m_bInSendToScript)
    return;   // can't do it

  if (Count <= 0) 
     return;        // nothing to do

  // if we have the empty line at the end of the buffer, delete that too
  if (m_pCurrentLine && m_pCurrentLine->len == 0)
    Count++;

// delete all lines in this set
  for (pos = m_LineList.GetTailPosition (); Count > 0 && pos; Count--)
   {
   // if this particular line was added to the line positions array, then make it null
    if (m_LineList.GetCount () % JUMP_SIZE == 1)
          m_pLinePositions [m_LineList.GetCount () / JUMP_SIZE] = NULL;

    delete m_LineList.GetTail (); // delete contents of tail iten -- version 3.85
    m_LineList.RemoveTail ();   // get rid of the line
    m_total_lines--;            // don't count as received

   if (m_LineList.IsEmpty ())  // give up if buffer is empty
     break;

   m_LineList.GetPrev (pos);
   }

  // try to allow world.tells to span omitted lines
  if (!m_LineList.IsEmpty ())
    {
    m_pCurrentLine = m_LineList.GetTail ();
    if (((m_pCurrentLine->flags & COMMENT) == 0) ||
        m_pCurrentLine->hard_return)
        m_pCurrentLine = NULL;
    }
  else
    m_pCurrentLine = NULL;

  if (!m_pCurrentLine)
    {
    // restart with a blank line at the end of the list
    m_pCurrentLine = new CLine (++m_total_lines, 
                                m_nWrapColumn,
                                m_iFlags,
                                m_iForeColour,
                                m_iBackColour,
                                m_bUTF_8);
    pos = m_LineList.AddTail (m_pCurrentLine);

    if (m_LineList.GetCount () % JUMP_SIZE == 1)
      m_pLinePositions [m_LineList.GetCount () / JUMP_SIZE] = pos;
    }


// notify view to invalidate this part

  for(pos = GetFirstViewPosition(); pos != NULL; )
    {
  	CView* pView = GetNextView(pos);
	
  	if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
  	  {
  		CMUSHView* pmyView = (CMUSHView*)pView;

  		pmyView->Invalidate ();
  		}	  // end of being an output view
  	}   // end of doing each view

}  // end of DeleteLines


BOOL CMUSHclientDoc::NotepadSaveMethod(LPCTSTR Title, short Method) 
{
CTextDocument * pTextDoc = FindNotepad (Title);

  if (!pTextDoc)
    return false;  // can't find it

  switch (Method)
    {
    case 0: pTextDoc->m_iSaveOnChange = eNotepadSaveDefault; break;
    case 1: pTextDoc->m_iSaveOnChange = eNotepadSaveAlways; break;
    case 2: pTextDoc->m_iSaveOnChange = eNotepadSaveNever; break;
    default: return false;  // bad selector
    }

	return true;
}   // end of NotepadSaveMethod

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
}    // end of ChangeDir

long CMUSHclientDoc::SendPkt(LPCTSTR Packet) 
{

  if (m_iConnectPhase != eConnectConnectedToMud)
    return eWorldClosed;             

  SendPacket (Packet, strlen (Packet));
	return eOK;
}


BSTR CMUSHclientDoc::Metaphone(LPCTSTR Word, short Length) 
{

// char buf [60];
//  metaphone (Word, buf, Length);

  MString m (Word, Length);

  CString str1, str2;

  m.DoubleMetaphone (str1, str2);

	CString strResult (str1);

  if (!str2.IsEmpty ())
    strResult += "," + str2;

	return strResult.AllocSysString();
}

long CMUSHclientDoc::EditDistance(LPCTSTR Source, LPCTSTR Target) 
{
  return ::EditDistance (Source, Target);
}

long CMUSHclientDoc::FlushLog() 
{
  
  if (m_logfile)
    {
    fflush (m_logfile);
    return eOK;
    }

	return eLogFileNotOpen;

}


BOOL CMUSHclientDoc::Transparency(long Key, short Amount) 
{
  if (Amount < MWT_MIN_FACTOR)
    Amount = MWT_MIN_FACTOR;
  else if (Amount > MWT_MAX_FACTOR)
    Amount = MWT_MAX_FACTOR;

  return MakeWindowTransparent (App.m_pMainWnd->m_hWnd, Key, (unsigned char) Amount);
}


void CMUSHclientDoc::SetWorldWindowStatus(short Parameter) 
{

CFrameWnd* pParent = NULL;

  for(POSITION pos=GetFirstViewPosition();pos!=NULL;)
    {
    CView* pView = GetNextView(pos);

    if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
      {
      CSendView* pmyView = (CSendView*)pView;

      pParent = pmyView->GetParentFrame ();

      break;

      }	
    }

  if (pParent)
    {
    switch (Parameter)
      {
      case 1: pParent->ShowWindow(SW_SHOWMAXIMIZED); break;
      case 2: pParent->ShowWindow(SW_MINIMIZE); break;
      case 3: pParent->ShowWindow(SW_RESTORE); break;
      case 4: pParent->ShowWindow(SW_SHOWNORMAL); break;
      } // end of switch
    } // have parent
}


BSTR CMUSHclientDoc::GetCustomColourName(short WhichColour) 
{
	CString strResult;
  if (WhichColour >= 1 && WhichColour <= MAX_CUSTOM)
    strResult = m_strCustomColourName [WhichColour - 1];

	return strResult.AllocSysString();
}

long CMUSHclientDoc::SetCustomColourName(short WhichColour, LPCTSTR Name) 
{
  if (WhichColour < 1 || WhichColour > MAX_CUSTOM)
    return eOptionOutOfRange;  

  if (strlen (Name) <= 0)
    return eNoNameSpecified;

  if (strlen (Name) > 30)
    return eInvalidObjectLabel;

  if (m_strCustomColourName [WhichColour - 1] != Name)
    SetModifiedFlag (TRUE);   // document has changed

  m_strCustomColourName [WhichColour - 1] = Name;

	return eOK;
}



VARIANT CMUSHclientDoc::GetNotepadList(BOOL All) 
{
  COleSafeArray sa;   // for list

  CTextDocument * pTextDoc;
  long iCount = 0;
  POSITION pos;

  // count number of notepads
  for (pos = App.m_pNormalDocTemplate->GetFirstDocPosition(); pos != NULL; )
    {
    pTextDoc = (CTextDocument *) App.m_pNormalDocTemplate->GetNextDoc(pos);

    if (All || (pTextDoc->m_pRelatedWorld == this &&
       pTextDoc->m_iUniqueDocumentNumber == m_iUniqueDocumentNumber))
      iCount++;

    }

  if (iCount) // cannot create empty array dimension
    {
    sa.CreateOneDim (VT_VARIANT, iCount);
  
    // put the worlds into the array
    for (iCount = 0, pos = App.m_pNormalDocTemplate->GetFirstDocPosition(); pos != NULL; )
      {
      pTextDoc = (CTextDocument *) App.m_pNormalDocTemplate->GetNextDoc(pos);

      // ignore unrelated worlds
      if (!All)
        if (pTextDoc->m_pRelatedWorld != this ||
           pTextDoc->m_iUniqueDocumentNumber != m_iUniqueDocumentNumber)
           continue;

      // the array must be a bloody array of variants, or VBscript kicks up
      COleVariant v (pTextDoc->m_strTitle);
      sa.PutElement (&iCount, &v);

      iCount++;
      }      // end of looping through each notepad
    } // end of having at least one

	return sa.Detach ();
}

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
}



VARIANT CMUSHclientDoc::GetGlobalOption(LPCTSTR Name) 
{
  return App.GetGlobalOption (Name);
}

VARIANT CMUSHclientDoc::GetGlobalOptionList() 
{
  return App.GetGlobalOptionList ();
}


// start playing sound FileName in buffer Buffer (or in memory in MemoryBuffer for length MemLen)

// If not a fully-qualified path, prepend mushclient.exe directory followed by "sounds\"
// Loop when finished if Loop is true
// Play at volume 'Volume' (percent) - 0 is full volume, -100 is -100 db  
//   (3 db each time halves the volume, so -3 would be half volume, and -6 would be quarter volume)
// Pan at Pan -100 to +100 percent where -100 is fully left and +100 is fully right (and zero is centered)
// If Buffer is nonzero, and FileName is "" then adjust the volume, pan and loop flag for an existing, playing, sound, if possible
// If Buffer zero then try to take an unused buffer. If that fails, take a non-playing buffer. If that fails, take buffer 1
// Buffer should be zero (for any free buffer) or 1 to MAX_SOUND_BUFFERS (currently 10)

// Sound files must be .WAV files, 16-bit, 22.05KHz, PCM, Mono or stereo

// test: /print (PlaySound (9, "/Program Files/Mushclient/sounds/battle/battle01.wav", true, 100))

long CMUSHclientDoc::PlaySoundHelper(short Buffer, LPCTSTR FileName, BOOL Loop, double Volume, double Pan, LPCTSTR MemoryBuffer, size_t MemLen) 
{
int i;

  // no sound object? can't do it
  if (!App.m_pDirectSoundObject || !App.m_pDirectSoundPrimaryBuffer)
    return eCannotPlaySound ;

  // first release sound buffers for sounds that are not playing or are lost
  for (i = 0; i < MAX_SOUND_BUFFERS; i++)
    if (m_pDirectSoundSecondaryBuffer [i])
      {
      DWORD iStatus;
      if (SUCCEEDED (m_pDirectSoundSecondaryBuffer [i]->GetStatus (&iStatus)) &&
          (((iStatus & (DSBSTATUS_PLAYING | DSBSTATUS_LOOPING)) == 0) ||
          (iStatus & DSBSTATUS_BUFFERLOST ) ) )
        {  
        m_pDirectSoundSecondaryBuffer [i]->Release ();
        m_pDirectSoundSecondaryBuffer [i] = NULL;
        }
      }


  // -------------------------- convert volume from percent into db * 100 ----------------------------

  // Force volume into range
  if (Volume > 0 || Volume < (-100.0))
    Volume = 0.0;

  long iVolume = - (Volume / 100) * DSBVOLUME_MIN;


  // -------------------------- convert pan from percent into +/- db * 100 ----------------------------

  // Force Pan into range
  if (Pan > 100.0 || Pan < (-100))
    Pan = 0;

  long iPan = (Pan / 100) * DSBPAN_RIGHT;


  // ---------------------- if no filename, attempt to adjust existing sound ---------------------

  if (MemoryBuffer == NULL)
    {
    if (Buffer >= 1 && Buffer <= MAX_SOUND_BUFFERS && strlen (FileName) == 0)
      {
      Buffer--;   // make zero-relative

      // can't if not there
      if (m_pDirectSoundSecondaryBuffer [Buffer] == NULL)
        return eBadParameter;
      
      // don't if not playing
      DWORD iStatus;
      if (FAILED (m_pDirectSoundSecondaryBuffer [Buffer]->GetStatus (&iStatus)) ||
          (iStatus & DSBSTATUS_PLAYING) == 0)
        return eCannotPlaySound;

        // set requested panning
      m_pDirectSoundSecondaryBuffer [Buffer]->SetPan (iPan);
      // set requested volume
      m_pDirectSoundSecondaryBuffer [Buffer]->SetVolume (iVolume);
      // set requested looping
      m_pDirectSoundSecondaryBuffer [Buffer]->Play( 0, 0, Loop ? DSBPLAY_LOOPING : 0);

      return eOK;

      }

    // no file name AND no buffer number? can't do much
    if  (strlen (FileName) < 2)  // later I test the 1st and 2nd characters
      return eBadParameter;

    } // have a file name (not memory file)

  // ------------------- we have filename, get on with it -----------------------------

  // ---------------- if Buffer is zero, first try to find a free one -----------------

  if (Buffer == 0)
    {
    for (i = 0; i < (MAX_SOUND_BUFFERS / 2); i++)
      if (m_pDirectSoundSecondaryBuffer [i] == NULL)
        {
        Buffer = i + 1;
        break;
        } // end of found an unused slot
    }  // end of no buffer specifed

  // no free buffers? re-use buffer #1
  if (Buffer == 0)
    Buffer = 1;  

  // Make buffer zero-relative
  Buffer--;
  
  // Buffer must be in range
  if (Buffer < 0 || Buffer >= MAX_SOUND_BUFFERS)
    return eBadParameter;


  // ------------------- we have a buffer now -------------------------------

  CString strName;

  if (MemoryBuffer == NULL)
    {

    // ---------- if not absolute path, use MUSHclient.exe / sounds / filename 

    strName = FileName;

    strName.Replace ("/", "\\");  // convert forward to backslash

    // without a full pathname, assume in sounds directory under MUSHclient.exe
    if (!(
        isalpha ((unsigned char) strName [0]) &&
        strName [1] == ':'               // eg. c: blah
        ) &&      
        strName [0] != '\\')          // eg. \mydir\blah or \\server\blah
      strName = ExtractDirectory (App.m_strMUSHclientFileName) + "sounds\\" + strName;

    if (strName.GetLength () > 127)
      return eBadParameter;

    } // have a file name (not memory file)



  // ---------- open multimedia file ----------------------------

  HMMIO          mmioWave ;
  MMCKINFO       mmckinfoParent, mmckinfoSubchunk ;
  WAVEFORMATEX   wfPCM ;
  MMIOINFO mmioInfo;
  memset(&mmioInfo, 0, sizeof(MMIOINFO));

  // Open the wave data using the mm functions
  if (MemoryBuffer == NULL)
    {
    char sFilename [128];
    strcpy (sFilename, strName);  // make not const

    mmioWave = mmioOpen (sFilename, NULL, MMIO_READ);   // open from disk
    }   // have a file name (not memory file)
  else
    {  // open from memory
    mmioInfo.pchBuffer = (char *) MemoryBuffer;
    mmioInfo.cchBuffer = MemLen;
    mmioInfo.fccIOProc = FOURCC_MEM;
    mmioWave = mmioOpen (NULL, &mmioInfo, MMIO_READ) ;
    }

  if ( !mmioWave )
   return eFileNotFound;

  // now that we know we have a file there, release the sound buffer if possible

  // buffer currently in use? release it
  if (m_pDirectSoundSecondaryBuffer [Buffer])
    {
    DWORD iStatus;
    if (SUCCEEDED (m_pDirectSoundSecondaryBuffer [Buffer]->GetStatus (&iStatus)) &&
        (iStatus & DSBSTATUS_PLAYING))
      m_pDirectSoundSecondaryBuffer [Buffer]->Stop ();

    m_pDirectSoundSecondaryBuffer [Buffer]->Release ();
    m_pDirectSoundSecondaryBuffer [Buffer] = NULL;
    }

  // Descend to find a "WAVE" block, if this fails then the data is not
  // WAV data.
  mmckinfoParent.fccType = mmioFOURCC( 'W', 'A', 'V', 'E' ) ;
  if ( mmioDescend( mmioWave, &mmckinfoParent, NULL, MMIO_FINDRIFF ) ) 
   {
    mmioClose( mmioWave, 0 ) ;
    return eCannotPlaySound ;
   }

  // Descend again to the "fmt " block and retrieve the format information
  mmckinfoSubchunk.ckid = mmioFOURCC( 'f', 'm', 't', ' ' ) ;
  if ( mmioDescend( mmioWave, &mmckinfoSubchunk, &mmckinfoParent, 
                   MMIO_FINDCHUNK ) )
   {
    mmioClose( mmioWave, 0 ) ;
    return eCannotPlaySound ;
   }

  // This line actually reads the data from the "fmt " chunk, this data
  // should be in the form of a WAVEFORMATEX structure
  if ( mmioRead( mmioWave, (char *) &wfPCM, mmckinfoSubchunk.cksize ) == -1 )
   {
    mmioClose( mmioWave, 0 ) ;
    return eCannotPlaySound ;
   }

  // Step out a layer... think of the mm functions as step in and out of
  // hierarchies of "chunks" of information
  mmioAscend( mmioWave, &mmckinfoSubchunk, 0 ) ;

  // Find the "data" subchunk
  mmckinfoSubchunk.ckid = mmioFOURCC( 'd', 'a', 't', 'a' ) ;
  if ( mmioDescend( mmioWave, &mmckinfoSubchunk, &mmckinfoParent, 
                    MMIO_FINDCHUNK ) ) 
   {
    mmioClose( mmioWave, 0 ) ;
    return eCannotPlaySound ;
   }

 HRESULT        hr ;
 LPBYTE         lpvAudio1 ;
 LPBYTE         lpvAudio2 ;
 DWORD          dwWriteBytes1 ;
 DWORD          dwWriteBytes2 ;
 DSBUFFERDESC   bd ;   
 
 // At this point we have succeeded in finding the data for the WAV file so
 // we need to create a DirectSoundBuffer 
  // Set up bd structure for a static secondary buffer.       
  bd.dwSize = sizeof( bd ) ;       
  bd.dwFlags = (DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY) | DSBCAPS_STATIC;  

  // want to hear in background?
  if (m_bPlaySoundsInBackground)
    bd.dwFlags |= DSBCAPS_GLOBALFOCUS;

  // Buffer size retrieved from the mmckinfo structure for the data 
  // portion of the wav
  bd.dwBufferBytes = mmckinfoSubchunk.cksize ; 
  bd.dwReserved = 0 ;
  bd.lpwfxFormat = &wfPCM ; 

  // Create buffer.    
  if ( FAILED( App.m_pDirectSoundObject->CreateSoundBuffer( &bd, 
               &m_pDirectSoundSecondaryBuffer [Buffer], NULL ) ) )
   {
    mmioClose( mmioWave, 0 ) ;
    return eCannotPlaySound ;
   }
 
#define WRAP_UP_FAILED_SOUND \
    m_pDirectSoundSecondaryBuffer [Buffer]->Release ();  \
    m_pDirectSoundSecondaryBuffer [Buffer] = NULL;  \
    mmioClose( mmioWave, 0 ) ;  \
    return eCannotPlaySound;

 // make copy to save a bit of typing
 LPDIRECTSOUNDBUFFER lpdsBuffer = m_pDirectSoundSecondaryBuffer [Buffer];

 // Lock the buffer for the DirectSoundBuffer object
 hr = lpdsBuffer->Lock( 0, 0, (void **) &lpvAudio1, &dwWriteBytes1, 
               (void **) &lpvAudio2, &dwWriteBytes2, DSBLOCK_ENTIREBUFFER ) ;
 if ( hr == DSERR_BUFFERLOST )
   {
    // If the buffer was lost try restoring it and lock again
    lpdsBuffer->Restore() ;
    hr = lpdsBuffer->Lock( 0, 0, (void **) &lpvAudio1, &dwWriteBytes1, 
               (void **) &lpvAudio2, &dwWriteBytes2, DSBLOCK_ENTIREBUFFER ) ;
   }

  if ( FAILED( hr ) )
    {
    WRAP_UP_FAILED_SOUND;  // lock failed
    }
    
  if ( dwWriteBytes1 != mmckinfoSubchunk.cksize )
    {
    lpdsBuffer->Unlock( (void *) lpvAudio1, dwWriteBytes1, 
                        (void *) lpvAudio2, dwWriteBytes2 );
    WRAP_UP_FAILED_SOUND;  // wrong number of bytes
    }

  // Read the data directly into the locked buffer
  if ( mmioRead( mmioWave, ( char* ) lpvAudio1, mmckinfoSubchunk.cksize ) 
      == -1 )
    {
    lpdsBuffer->Unlock( (void *) lpvAudio1, dwWriteBytes1, 
                        (void *) lpvAudio2, dwWriteBytes2 );
    WRAP_UP_FAILED_SOUND;  // couldn't read
    }

  // Unlock the buffer
  if ( FAILED( lpdsBuffer->Unlock( (void *) lpvAudio1, dwWriteBytes1, 
                                  (void *) lpvAudio2, dwWriteBytes2 ) ) )
    {
    WRAP_UP_FAILED_SOUND;   // couldn't unlock
    }
 
  // Close the multimedia object
  mmioClose( mmioWave, 0 ) ;

  // set requested panning
  lpdsBuffer->SetPan (iPan);
  // set requested volume
  lpdsBuffer->SetVolume (iVolume);

 if (FAILED (lpdsBuffer->Play( 0, 0, Loop ? DSBPLAY_LOOPING : 0) ))
   {
    m_pDirectSoundSecondaryBuffer [Buffer]->Release ();
    m_pDirectSoundSecondaryBuffer [Buffer] = NULL;
    return eCannotPlaySound;
   }

  return eOK;
} // end of CMUSHclientDoc::PlaySoundHelper

long CMUSHclientDoc::PlaySound(short Buffer, LPCTSTR FileName, BOOL Loop, double Volume, double Pan) 
  {
  return PlaySoundHelper (Buffer, FileName, Loop, Volume, Pan, NULL, 0);
  }  // end of  CMUSHclientDoc::PlaySound


// stop the sound in the numbered Buffer from playing
// if Buffer is zero, stop all sounds
long CMUSHclientDoc::StopSound(short Buffer) 
{
  // buffer of zero means all
  if (Buffer == 0)
    {
    // stop sounds playing, release sound buffers
    for (int i = 0; i < MAX_SOUND_BUFFERS; i++)
      if (m_pDirectSoundSecondaryBuffer [i])
        {
        DWORD iStatus;
        if (SUCCEEDED (m_pDirectSoundSecondaryBuffer [i]->GetStatus (&iStatus)) &&
            (iStatus & DSBSTATUS_PLAYING))
          m_pDirectSoundSecondaryBuffer [i]->Stop ();

        m_pDirectSoundSecondaryBuffer [i]->Release ();
        m_pDirectSoundSecondaryBuffer [i] = NULL;

        }
      return eOK;
    }

  // Make buffer zero-relative
  Buffer--;
  
  // Buffer must be in range
  if (Buffer < 0 || Buffer >= MAX_SOUND_BUFFERS)
    return eBadParameter;

  // buffer currently in use? release it
  if (m_pDirectSoundSecondaryBuffer [Buffer])
    {
    DWORD iStatus;
    if (SUCCEEDED (m_pDirectSoundSecondaryBuffer [Buffer]->GetStatus (&iStatus)) &&
        (iStatus & DSBSTATUS_PLAYING))
      m_pDirectSoundSecondaryBuffer [Buffer]->Stop ();

    m_pDirectSoundSecondaryBuffer [Buffer]->Release ();
    m_pDirectSoundSecondaryBuffer [Buffer] = NULL;
    }

	return eOK;
}

// set the status of the sound buffer
// -1 means buffer number out of range (1 to MAX_SOUND_BUFFERS)
// -2 means that the buffer is free
// -3 means sound system is not initialized (eg. Windows NT)
//  0 means the sound is NOT playing
//  1 means the sound is playing but not looping
//  2 means the sound is playing AND looping

long CMUSHclientDoc::GetSoundStatus(short Buffer) 
{

  // no sound object? can't do it
  if (!App.m_pDirectSoundObject || !App.m_pDirectSoundPrimaryBuffer)
    return -3;

  // Make buffer zero-relative
  Buffer--;
  
  // Buffer must be in range
  if (Buffer < 0 || Buffer >= MAX_SOUND_BUFFERS)
    return -1;

  if (!m_pDirectSoundSecondaryBuffer [Buffer])
    return -2;

  DWORD iStatus = 0;

  if (FAILED (m_pDirectSoundSecondaryBuffer [Buffer]->GetStatus (&iStatus)))
    return 0;  // can't get status, presume no sound

  if (iStatus & DSBSTATUS_PLAYING)
    return 1;

  if (iStatus & DSBSTATUS_LOOPING)
    return 2;

	return 0;  // not playing
}

CChildFrame * CMUSHclientDoc::GetChildFrame (void)
  {
  CChildFrame * pFrame = NULL;

  // find the child frame
  for(POSITION pos=GetFirstViewPosition();pos!=NULL;)
    {
    CView* pView = GetNextView(pos);

    if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
      {
      CSendView* pmyView = (CSendView*)pView;

      pFrame = pmyView->m_owner_frame;
      break;
      }	  // end of being a CSendView
    }
  
  return pFrame;

  } // end of CMUSHclientDoc::GetChildFrame

// set the height of the command window
long CMUSHclientDoc::SetCommandWindowHeight(short Height) 
{
	
  if (Height < 0)
    return eBadParameter;

  CChildFrame * pFrame = GetChildFrame ();

  // fail if we can't find one
  if (pFrame == NULL)
    return eBadParameter;

	CRect rectInside;
	pFrame->m_wndSplitter.GetClientRect(rectInside);
	rectInside.InflateRect(-9, -9); // allow for borders and splitter bar

  // we have to root around like this, because recalclayout lays out the top
  // view first, and allocates the rest (possibly nothing) to the bottom view.
  // I don't really want this, the important thing is that you can see where
  // you are going to type. Thus I work out how much the maxmimum top view
  // can be.

  int iRoom = rectInside.bottom - rectInside.top - Height 
      + 7;    // 7 pixels for the splitter bar

  int cyCurTop = iRoom;

  if (cyCurTop < 20)
    return eBadParameter;  // too small, want to see a line at least

  // set the info for the top view
  pFrame->m_wndSplitter.SetRowInfo (OUTPUT_PANE, cyCurTop, 20);

  // set the info for the bottom view
  pFrame->m_wndSplitter.SetRowInfo (COMMAND_PANE, Height, 9);

  // recalculate it all
  pFrame->m_wndSplitter.RecalcLayout ();


	return eOK;
}   // end of CMUSHclientDoc::SetCommandWindowHeight

// move a toolbar
long CMUSHclientDoc::SetToolBarPosition(short Which, BOOL Float, short Side, long Top, long Left) 
{
CMyToolBar * pToolBar = NULL;

  Frame.RecalcLayout(TRUE);

  switch (Which)
    {
    case 1: pToolBar = &Frame.m_wndToolBar; break;  // main toolbar
    case 2: pToolBar = &Frame.m_wndGameToolBar; break;  // world toolbar
    case 3: pToolBar = &Frame.m_wndActivityToolBar; break;  // activity toolbar
    case 4: pToolBar = (CMyToolBar *) &Frame.m_wndInfoBar; break;  // info toolbar
    default: return eBadParameter;
    } // end of switch

  if (!pToolBar || !pToolBar->m_hWnd)
      return eBadParameter;  // oops - no window

	CRect rectBar;
	pToolBar->GetWindowRect(&rectBar);

  if (Float)
    {
    DWORD dwStyle;
    CPoint point (Left, Top);
    Frame.ClientToScreen (&point);
    switch (Side)
      {
      case 1: dwStyle = CBRS_ALIGN_TOP ; break;
      case 3: dwStyle = CBRS_ALIGN_LEFT; break;
      default: return eBadParameter;
      }
    Frame.FloatControlBar (pToolBar, point, dwStyle);
    }
  else
    {
    UINT nDockBarID = AFX_IDW_DOCKBAR_TOP;
    CRect rect (Left, Top, Left + rectBar.right - rectBar.left, Top + rectBar.bottom - rectBar.top);
    Frame.ClientToScreen (rect);
    switch (Side)
      {
      case 0: break;   // zero is OK
      case 1: nDockBarID = AFX_IDW_DOCKBAR_TOP; break;
      case 2: nDockBarID = AFX_IDW_DOCKBAR_BOTTOM; break;
      case 3: nDockBarID = AFX_IDW_DOCKBAR_LEFT; break;
      case 4: nDockBarID = AFX_IDW_DOCKBAR_RIGHT; break;
      default: return eBadParameter;
      }
    Frame.DockControlBar (pToolBar, nDockBarID, rect);
    }

	return eOK;
}    // end of CMUSHclientDoc::SetToolBarPosition


// add an item for shift+tab completion
long CMUSHclientDoc::ShiftTabCompleteItem(LPCTSTR Item) 
{
  if (strlen (Item) <= 0 || strlen (Item) > 30)
    return eBadParameter;  // need a string

  if (strcmp (Item, "<clear>") == 0)
    m_ExtraShiftTabCompleteItems.clear ();
  else if (strcmp (Item, "<functions>") == 0)
    m_bTabCompleteFunctions = true;
  else if (strcmp (Item, "<nofunctions>") == 0)
    m_bTabCompleteFunctions = false;
  else
    {
    if (!IsValidName (Item))    // must be alphanumeric, dot, hyphen, underscore
      return eBadParameter;  // need a string
    m_ExtraShiftTabCompleteItems.insert (Item);
    }

	return eOK;
}    // end of CMUSHclientDoc::ShiftTabCompleteItem

// make a notepad read-only
long CMUSHclientDoc::NotepadReadOnly(LPCTSTR Title, BOOL ReadOnly) 
{
CTextDocument * pTextDoc = FindNotepad (Title);

  if (!pTextDoc)
    return false;

  pTextDoc->SetReadOnly (ReadOnly);

	return true;
}   // end of CMUSHclientDoc::NotepadReadOnly


// add a  new font from a file
long CMUSHclientDoc::AddFont(LPCTSTR PathName) 
{

  if (strlen (PathName) <= 0)
    return eBadParameter;  // need a string

  return AddSpecialFont (PathName);

}  // end of  CMUSHclientDoc::AddFont

/* modes: 

  0 = strech to output view size 
  1 = stretch with aspect ratio

  -- next two will not be affected by command window size changes:

  2 = strech to owner size 
  3 = stretch with aspect ratio
  
  -- going clockwise here:

  -- top
  4 = top left
  5 = center left-right at top
  6 = top right

  -- rh side
  7 = on right, center top-bottom
  8 = on right, at bottom

  -- bottom
  9 = center left-right at bottom

  -- lh side
  10 = on left, at bottom  
  11 = on left, center top-bottom 

  -- middle
  12 = center all

  13 = tile


  */


// set main window background image
long CMUSHclientDoc::SetBackgroundImage(LPCTSTR FileName, short Mode) 
{

  if (Mode < 0 || Mode > 13)
    return eBadParameter; 

  if ((HBITMAP) m_BackgroundBitmap)
    m_BackgroundBitmap.DeleteObject ();

  m_strBackgroundImageName.Empty ();

  UpdateAllViews (NULL);

  CString strFileName = FileName;

  strFileName.TrimLeft ();
  strFileName.TrimRight ();

  // no file name means get rid of image
  if (strFileName.IsEmpty ())
    return eOK;

  // have to be long enough to have x.bmp
  if (strFileName.GetLength () < 5)
    return eBadParameter;

  HBITMAP hBmp;

  // handle PNG files separately
  if (strFileName.Right (4).CompareNoCase (".png") == 0)
    {
    long result =  LoadPng (FileName, hBmp);

    if (result != eOK) 
      return result;

    }    // png file
  else
    {

    // must be .bmp or .png file
    if (strFileName.Right (4).CompareNoCase (".bmp") != 0)
      return eBadParameter;

    hBmp = (HBITMAP)::LoadImage(
                    NULL,
                    FileName,
                    IMAGE_BITMAP,
                    0,
                    0,
                    LR_LOADFROMFILE|LR_CREATEDIBSECTION
                    );
    }  // bmp file

   if (hBmp) 
     {
      m_BackgroundBitmap.Attach (hBmp);
      m_iBackgroundMode = Mode;
      m_strBackgroundImageName = FileName;
    	return eOK;
     }  // end of having a bitmap loaded


   if (GetLastError () == 2)
     return eFileNotFound;

   return eCouldNotOpenFile;
  
  } // end of CMUSHclientDoc::SetBackgroundImage

// set main window foreground image
long CMUSHclientDoc::SetForegroundImage(LPCTSTR FileName, short Mode) 
{
  if (Mode < 0 || Mode > 13)
    return eBadParameter;  

  if ((HBITMAP) m_ForegroundBitmap)
    m_ForegroundBitmap.DeleteObject ();

  m_strForegroundImageName.Empty ();

  UpdateAllViews (NULL);

  CString strFileName = FileName;

  strFileName.TrimLeft ();
  strFileName.TrimRight ();

  // no file name means get rid of image
  if (strFileName.IsEmpty ())
    return eOK;

  // have to be long enough to have x.bmp
  if (strFileName.GetLength () < 5)
    return eBadParameter;

  HBITMAP hBmp;

  // handle PNG files separately
  if (strFileName.Right (4).CompareNoCase (".png") == 0)
    {
    long result =  LoadPng (FileName, hBmp);

    if (result != eOK) 
      return result;

    }    // png file
  else
    {

    // must be .bmp or .png file
    if (strFileName.Right (4).CompareNoCase (".bmp") != 0)
      return eBadParameter;

    hBmp = (HBITMAP)::LoadImage(
                    NULL,
                    FileName,
                    IMAGE_BITMAP,
                    0,
                    0,
                    LR_LOADFROMFILE|LR_CREATEDIBSECTION
                    );
    }  // bmp file

   if (hBmp) 
     {
      m_ForegroundBitmap.Attach (hBmp);
      m_iForegroundMode = Mode;
      m_strForegroundImageName = FileName;
    	return eOK;
     }  // end of having a bitmap loaded


   if (GetLastError () == 2)
     return eFileNotFound;

   return eCouldNotOpenFile;
  
}   // end of CMUSHclientDoc::SetForegroundImage

// set main window background colour
long CMUSHclientDoc::SetBackgroundColour(long Colour) 
{
  long oldColour = m_iBackgroundColour;

  m_iBackgroundColour = Colour;

  UpdateAllViews (NULL);

	return oldColour;
}   // end of CMUSHclientDoc::SetBackgroundColour

/* positions: 

  0 = strech to output view size 
  1 = stretch with aspect ratio

  2 = strech to owner size 
  3 = stretch with aspect ratio
  
  -- going clockwise here:

  -- top
  4 = top left
  5 = center left-right at top
  6 = top right

  -- rh side
  7 = on right, center top-bottom
  8 = on right, at bottom

  -- bottom
  9 = center left-right at bottom

  -- lh side
  10 = on left, at bottom  
  11 = on left, center top-bottom 

  -- middle
  12 = center all

  13 = tile


  */

// create (or re-create) a mini-window
long CMUSHclientDoc::WindowCreate(LPCTSTR Name, 
                                  long Left, long Top, 
                                  long Width, long Height, 
                                  short Position, 
                                  long Flags, 
                                  long BackgroundColour) 
  {

  if (strlen (Name) == 0) 
    return eNoNameSpecified;    // empty name throws out various things

  if (Width < 0 || Height < 0)
    return eBadParameter;

  MiniWindowMapIterator it = m_MiniWindows.find (Name);

  CMiniWindow * pMiniWindow = NULL;

  if (it == m_MiniWindows.end ())
    {
    pMiniWindow = new CMiniWindow ();
    m_MiniWindows [Name] = pMiniWindow;
    }
  else
    pMiniWindow = it->second;

  pMiniWindow->Create (Left, Top, Width, Height,
                       Position, Flags, 
                       BackgroundColour);

  UpdateAllViews (NULL);

	return eOK;
}     // end of CMUSHclientDoc::WindowCreate

// set/clear the show flag so the window becomes visible
long CMUSHclientDoc::WindowShow(LPCTSTR Name, BOOL Show) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  it->second->Show (Show);

  UpdateAllViews (NULL);

	return eOK;
}     // end of CMUSHclientDoc::WindowShow

/*

------------------------------------------------------------------------------------
See: http://www.functionx.com/win32/Lesson11.htm

  for some nice examples of what they look like
------------------------------------------------------------------------------------

  */

// various rectangle operations
long CMUSHclientDoc::WindowRectOp(LPCTSTR Name, short Action, 
                                  long Left, long Top, long Right, long Bottom, 
                                  long Colour1, long Colour2) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->RectOp (Action, 
                             Left, Top, Right, Bottom, 
                             Colour1, Colour2);

}   // end of CMUSHclientDoc::WindowRectOp


// various circle/ellipse/pie operations
long CMUSHclientDoc::WindowCircleOp(LPCTSTR Name, short Action, 
                                    long Left, long Top, long Right, long Bottom, 
                                    long PenColour, long PenStyle, long PenWidth, 
                                    long BrushColour, long BrushStyle, 
                                    long Extra1, long Extra2, long Extra3, long Extra4) 
{
  if (strlen (Name) == 0) 
    return eNoNameSpecified;    // empty name throws out various things

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->CircleOp (Action, 
                               Left, Top, Right, Bottom, 
                               PenColour, PenStyle, PenWidth, 
                               BrushColour, BrushStyle,
                               Extra1, Extra2, Extra3, Extra4);

}     // end of CMUSHclientDoc::WindowCircleOp 


// add a font to our map of fonts by name (eg. "inventory")
long CMUSHclientDoc::WindowFont(LPCTSTR Name,        // which window
                                LPCTSTR FontId,      // eg. Inventory
                                LPCTSTR FontName,    // eg. Courier New
                                double Size,         // eg. 9 pt
                                BOOL Bold, BOOL Italic, BOOL Underline, BOOL Strikeout, 
                                short Charset, 
                                short PitchAndFamily) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Font (FontId, FontName,
                           Size, 
                           Bold, Italic, Underline, Strikeout, 
                           Charset, PitchAndFamily);
}    // end of CMUSHclientDoc::WindowFont

// output text, ordinary or UTF8 - returns length of text
long CMUSHclientDoc::WindowText(LPCTSTR Name,      // which window
                                LPCTSTR FontId,    // eg. Inventory
                                LPCTSTR Text,      // what to say
                                long Left, long Top, long Right, long Bottom,   // where to put it
                                long Colour,       // colour to show it in
                                BOOL Unicode)      // true if UTF8
{
  if (strlen (Name) == 0) 
    return -1;    // empty name throws out various things

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return -1;

  return it->second->Text (FontId, Text,  Left,  Top,  Right,  Bottom, Colour, Unicode );
}    // end of CMUSHclientDoc::WindowText 

// measure text, ordinary or UTF8
long CMUSHclientDoc::WindowTextWidth(LPCTSTR Name, LPCTSTR FontId, LPCTSTR Text, BOOL Unicode) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return -1;

  return it->second->TextWidth (FontId, Text, Unicode );
}     // end of CMUSHclientDoc::WindowTextWidth

// return info about the select font
VARIANT CMUSHclientDoc::WindowFontInfo(LPCTSTR Name, LPCTSTR FontId, long InfoType) 
{

	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;  

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return vaResult;

  it->second->FontInfo (FontId, InfoType, vaResult);
	return vaResult;
}    // end of CMUSHclientDoc::WindowFontInfo

// draws a straight line
long CMUSHclientDoc::WindowLine(LPCTSTR Name, 
                                long x1, long y1, long x2, long y2, 
                                long PenColour, long PenStyle, long PenWidth) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Line (x1, y1, x2, y2, PenColour, PenStyle, PenWidth);
}


// draws an arc
long CMUSHclientDoc::WindowArc(LPCTSTR Name, 
                               long Left, long Top, long Right, long Bottom, 
                               long x1, long y1, 
                               long x2, long y2, 
                               long PenColour, long PenStyle, long PenWidth) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Arc (Left, Top, Right, Bottom, x1, y1, x2, y2, PenColour, PenStyle, PenWidth);
}      // end of CMUSHclientDoc::WindowArc


// return list of windows we have made
VARIANT CMUSHclientDoc::WindowList() 
{
  COleSafeArray sa;   // for array list

  long iCount = 0;
  
  // put the arrays into the array
  if (!m_MiniWindows.empty ())    // cannot create empty dimension
    {
    sa.CreateOneDim (VT_VARIANT, m_MiniWindows.size ());

    for (MiniWindowMapIterator it = m_MiniWindows.begin (); 
         it != m_MiniWindows.end ();
         it++)
           {
            // the array must be a bloody array of variants, or VBscript kicks up
            COleVariant v (it->first.c_str ());
            sa.PutElement (&iCount, &v);
            iCount++;
           }

    } // end of having at least one

	return sa.Detach ();
}    // end of CMUSHclientDoc::WindowList

// return list of fonts created for this window
VARIANT CMUSHclientDoc::WindowFontList(LPCTSTR Name) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;  

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return vaResult;

  it->second->FontList (vaResult);
	return vaResult;
}    // end of CMUSHclientDoc::WindowFontList

// information about a window
VARIANT CMUSHclientDoc::WindowInfo(LPCTSTR Name, long InfoType) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;  

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return vaResult;

  it->second->Info (InfoType, vaResult);
	return vaResult;
}   // end of CMUSHclientDoc::WindowInfo

// load an image from disk
long CMUSHclientDoc::WindowLoadImage(LPCTSTR Name, 
                                     LPCTSTR ImageId, 
                                     LPCTSTR FileName) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->LoadImage (ImageId, FileName);
}   // end of  CMUSHclientDoc::WindowLoadImage

// load an image from memory
long CMUSHclientDoc::WindowLoadImageMemory(LPCTSTR Name, 
                                           LPCTSTR ImageId, 
                                           unsigned char * Buffer, 
                                           const size_t Length,
                                           const bool bAlpha) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->LoadImageMemory (ImageId, Buffer, Length, bAlpha);
}   // end of  CMUSHclientDoc::WindowLoadImageMemory


long CMUSHclientDoc::WindowWrite(LPCTSTR Name, LPCTSTR FileName) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Write (FileName);
}   // CMUSHclientDoc::WindowWrite


// draw an image
long CMUSHclientDoc::WindowDrawImage(LPCTSTR Name, 
                                     LPCTSTR ImageId, 
                                     long Left, long Top, long Right, long Bottom, 
                                     short Mode,
                                     long SrcLeft, long SrcTop, long SrcRight, long SrcBottom) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->DrawImage (ImageId, Left, Top, Right, Bottom, Mode, 
                                SrcLeft,   SrcTop,   SrcRight,   SrcBottom);
}     // end of CMUSHclientDoc::WindowDrawImage

// list loaded images
VARIANT CMUSHclientDoc::WindowImageList(LPCTSTR Name) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;  

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return vaResult;

  it->second->ImageList (vaResult);
	return vaResult;
}   // end of CMUSHclientDoc::WindowImageList


// info about an image
VARIANT CMUSHclientDoc::WindowImageInfo(LPCTSTR Name, LPCTSTR ImageId, long InfoType) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;  

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return vaResult;

  it->second->ImageInfo (ImageId, InfoType, vaResult);
	return vaResult;
}  // end of CMUSHclientDoc::WindowImageInfo

/*
 Device Parameters for GetDeviceCaps() 

    DRIVERVERSION 0       Device driver version                    
    TECHNOLOGY    2       Device classification                    
    HORZSIZE      4       Horizontal size in millimeters           
    VERTSIZE      6       Vertical size in millimeters             
    HORZRES       8       Horizontal width in pixels               
    VERTRES       10      Vertical height in pixels                
    BITSPIXEL     12      Number of bits per pixel                 
    PLANES        14      Number of planes                         
    NUMBRUSHES    16      Number of brushes the device has         
    NUMPENS       18      Number of pens the device has            
    NUMMARKERS    20      Number of markers the device has         
    NUMFONTS      22      Number of fonts the device has           
    NUMCOLORS     24      Number of colors the device supports     
    PDEVICESIZE   26      Size required for device descriptor      
    CURVECAPS     28      Curve capabilities                       
    LINECAPS      30      Line capabilities                        
    POLYGONALCAPS 32      Polygonal capabilities                   
    TEXTCAPS      34      Text capabilities                        
    CLIPCAPS      36      Clipping capabilities                    
    RASTERCAPS    38      Bitblt capabilities                      
    ASPECTX       40      Length of the X leg                      
    ASPECTY       42      Length of the Y leg                      
    ASPECTXY      44      Length of the hypotenuse                 
                          
    SHADEBLENDCAPS 45     Shading and blending caps                
                         
    LOGPIXELSX    88      Logical pixels/inch in X                 
    LOGPIXELSY    90      Logical pixels/inch in Y                 
                         
    SIZEPALETTE  104      Number of entries in physical palette    
    NUMRESERVED  106      Number of reserved entries in palette    
    COLORRES     108      Actual color resolution                  

    // Display driver specific

    VREFRESH        116   Current vertical refresh rate of the    
                          display device (for displays only) in Hz
    DESKTOPVERTRES  117   Horizontal width of entire desktop in   
                          pixels                                  
    DESKTOPHORZRES  118   Vertical height of entire desktop in    
                         pixels                                  
    BLTALIGNMENT    119    Preferred blt alignment                 

*/

// get device capabilites
long CMUSHclientDoc::GetDeviceCaps(long Index) 
{
CDC dc;
  dc.CreateCompatibleDC (NULL);
  return dc.GetDeviceCaps (Index);
}   // end of CMUSHclientDoc::GetDeviceCaps

// draw a bezier curve
long CMUSHclientDoc::WindowBezier(LPCTSTR Name, 
                                  LPCTSTR Points, 
                                  long PenColour, long PenStyle, long PenWidth) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Bezier (Points, PenColour, PenStyle, PenWidth);
}  // end of CMUSHclientDoc::WindowBezier


// draw an open or closed polygon
long CMUSHclientDoc::WindowPolygon(LPCTSTR Name, 
                                   LPCTSTR Points, 
                                   long PenColour, long PenStyle, long PenWidth, 
                                   long BrushColour, long BrushStyle,
                                   BOOL Close,       // close the polygon
                                   BOOL Winding)     // ALTERNATE or WINDING fill
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Polygon (Points, PenColour, PenStyle, PenWidth, BrushColour, BrushStyle, Close, Winding);
}  // end of CMUSHclientDoc::WindowPolygon


// move an existing window
long CMUSHclientDoc::WindowPosition(LPCTSTR Name, 
                                    long Left, long Top, 
                                    short Position, 
                                    long Flags) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  long status = it->second->Position (Left, Top, Position, Flags);

  if (status == eOK)
    UpdateAllViews (NULL);

  return status;

}    // end of CMUSHclientDoc::WindowPosition


/*

  Cursor values:

  0:  arrow                                
  1:  hand                                 
  2:  I-beam                               
  3:  + symbol                           
  4:  wait (hour-glass)                    
  5:  up arrow                             
  6:  arrow nw-se                          
  7:  arrow ne-sw                          
  8:  arrow e-w                            
  9:  arrow n-s                            
 10:  arrow - all ways                     
 11:  (X) no, no, I won't do that, but ... 
 12:  help  (? symbol)                     

  */


// add a hotspot for handling mouse-over, mouse up/down events
long CMUSHclientDoc::WindowAddHotspot(LPCTSTR Name, 
                                     LPCTSTR HotspotId, 
                                     long Left, long Top, long Right, long Bottom, 
                                     LPCTSTR MouseOver, 
                                     LPCTSTR CancelMouseOver, 
                                     LPCTSTR MouseDown, 
                                     LPCTSTR CancelMouseDown, 
                                     LPCTSTR MouseUp, 
                                     LPCTSTR TooltipText, 
                                     long Cursor, 
                                     long Flags) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  string sPluginID;

  if (m_CurrentPlugin)                            
    sPluginID = m_CurrentPlugin->m_strID;

  return it->second->AddHotspot (this, 
                               HotspotId, 
                               sPluginID,
                               Left, Top, Right, Bottom, 
                               MouseOver, 
                               CancelMouseOver, 
                               MouseDown, 
                               CancelMouseDown, 
                               MouseUp,
                               TooltipText,
                               Cursor,
                               Flags);
}  // end of CMUSHclientDoc::WindowHotspot

// remove a previously-installed hotspot
long CMUSHclientDoc::WindowDeleteHotspot(LPCTSTR Name, LPCTSTR HotspotId) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->DeleteHotspot (HotspotId);
}   // end of CMUSHclientDoc::WindowDeleteHotspot

// return list of all hotspots in this miniwindow
VARIANT CMUSHclientDoc::WindowHotspotList(LPCTSTR Name) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;  

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return vaResult;

  it->second->HotspotList (vaResult);
	return vaResult;
}    // end of CMUSHclientDoc::WindowHotspotList

// delete all hotspots
long CMUSHclientDoc::WindowDeleteAllHotspots(LPCTSTR Name) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->DeleteAllHotspots ();
}    // end of CMUSHclientDoc::WindowDeleteAllHotspots


// get information about a hotspot
VARIANT CMUSHclientDoc::WindowHotspotInfo(LPCTSTR Name, LPCTSTR HotspotId, long InfoType) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;  

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return vaResult;

  it->second->HotspotInfo (HotspotId, InfoType, vaResult);
	return vaResult;
}      // end of CMUSHclientDoc::WindowHotspotInfo


long CMUSHclientDoc::WindowImageOp(LPCTSTR Name, short Action, 
                                   long Left, long Top, long Right, long Bottom, 
                                   long PenColour, long PenStyle, long PenWidth,
                                   long BrushColour,  // 0-bits are drawn in this colour
                                   LPCTSTR ImageId, 
                                   long EllipseWidth, long EllipseHeight) 
  {
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->ImageOp (Action, Left, Top, Right, Bottom, 
                              PenColour, PenStyle, PenWidth, 
                              BrushColour, ImageId, 
                              EllipseWidth, EllipseHeight);
}    // end of CMUSHclientDoc::WindowImageOp

long CMUSHclientDoc::WindowCreateImage(LPCTSTR Name, LPCTSTR ImageId, 
                                       long Row1, long Row2, long Row3, long Row4, long Row5, long Row6, long Row7, long Row8) 
{

  if (strlen (Name) == 0) 
    return eNoNameSpecified;    // empty name throws out various things

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->CreateImage (ImageId, (WORD) Row1, (WORD) Row2, (WORD) Row3,
	                   (WORD) Row4, (WORD) Row5, (WORD) Row6, (WORD) Row7, (WORD) Row8);
}     // end of CMUSHclientDoc::WindowCreateImage


long CMUSHclientDoc::WindowBlendImage(LPCTSTR Name, LPCTSTR ImageId, 
                                      long Left, long Top, long Right, long Bottom, 
                                      short Mode, double Opacity, 
                                      long SrcLeft, long SrcTop, long SrcRight, long SrcBottom) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->BlendImage (ImageId, Left, Top, Right, Bottom, Mode, Opacity, SrcLeft, SrcTop, SrcRight, SrcBottom);
}   // end of CMUSHclientDoc::WindowBlendImage


long CMUSHclientDoc::WindowImageFromWindow(LPCTSTR Name, LPCTSTR ImageId, LPCTSTR SourceWindow) 
  {
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  CMiniWindow * pDestWindow = it->second;

  it = m_MiniWindows.find (SourceWindow);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  CMiniWindow * pSrcWindow = it->second;

  return pDestWindow->ImageFromWindow (ImageId, pSrcWindow);
}  // end of CMUSHclientDoc::WindowImageFromWindow


void CMUSHclientDoc::Repaint() 
{
  UpdateAllViews (NULL);

  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
  	  {
		  CMUSHView* pmyView = (CMUSHView*)pView;
      pmyView->UpdateWindow ();
	    }	  // end of being a CMUSHView
    }   // end of loop through views

}    // end of CMUSHclientDoc::Repaint



long CMUSHclientDoc::TextRectangle(long Left, long Top, long Right, long Bottom, 
                                   long BorderOffset, 
                                   long BorderColour, long BorderWidth, 
                                   long OutsideFillColour, long OutsideFillStyle) 
{


  CBrush br;

  if (ValidateBrushStyle (OutsideFillStyle, BorderColour, OutsideFillColour, br) != eOK)
    return eBrushStyleNotValid;

  m_TextRectangle = CRect (Left, Top, Right, Bottom);

  m_TextRectangleBorderOffset =       BorderOffset;      
  m_TextRectangleBorderColour =       BorderColour;        
  m_TextRectangleBorderWidth =        BorderWidth;         
  m_TextRectangleOutsideFillColour =  OutsideFillColour;
  m_TextRectangleOutsideFillStyle =   OutsideFillStyle;    

  // refresh views  - get scroll bars right

  for(POSITION pos=GetFirstViewPosition();pos!=NULL;)
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
  	  {
		  CMUSHView* pmyView = (CMUSHView*)pView;

		  pmyView->addedstuff();
	    }	
    }

  SendWindowSizes (m_nWrapColumn);  // notify of different window height
  Redraw ();
	return eOK;
}   // end of CMUSHclientDoc::TextRectangle
  


long CMUSHclientDoc::WindowGradient(LPCTSTR Name, 
                                    long Left, long Top, long Right, long Bottom, 
                                    long StartColour, long EndColour, 
                                    short Mode) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Gradient (Left, Top, Right, Bottom, 
                                    StartColour, EndColour, 
                                    Mode);
}    // end of CMUSHclientDoc::WindowGradient

  
long CMUSHclientDoc::WindowFilter(LPCTSTR Name, long Left, long Top, long Right, long Bottom, 
                                  short Operation, double Options) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Filter (Left, Top, Right, Bottom, 
                                    Operation, Options);
}    // CMUSHclientDoc::WindowFilter
                                       

long CMUSHclientDoc::WindowSetPixel(LPCTSTR Name, long x, long y, long Colour) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->SetPixel (x, y, Colour);
}   // end of CMUSHclientDoc::WindowSetPixel


long CMUSHclientDoc::WindowGetPixel(LPCTSTR Name, long x, long y) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return -2;

  return it->second->GetPixel (x, y);
}     // end of CMUSHclientDoc::WindowGetPixel


// see also: CMiniWindow::Filter

long CMUSHclientDoc::FilterPixel(long Pixel, short Operation, double Options) 
{
long r = GetRValue (Pixel),
     g = GetGValue (Pixel),
     b = GetBValue (Pixel);

   switch (Operation)
     {
     case 1:        // Noise
       {
       double threshold = Options / 100.0;
       r += (128 - genrand () * 256) * threshold;
       g += (128 - genrand () * 256) * threshold;
       b += (128 - genrand () * 256) * threshold;
       break;
       }

     case 2:    // MonoNoise
       {
       double threshold = Options / 100.0;
       long j = (128 - genrand () * 256) * threshold;
       r += j;
       g += j;
       b += j;
       break;
       }


     case  7: // Brightness     
        {
        r += Options;
        g += Options;
        b += Options;
        break;
        }


     case  8: // Contrast        
        {
        double c;

        c = r - 128;   // center on zero
        c *= Options;  // multiply by contrast
        r = c + 128;   // put back

        c = g - 128;   // center on zero
        c *= Options;  // multiply by contrast
        g = c + 128;   // put back

        c = b - 128;   // center on zero
        c *= Options;  // multiply by contrast
        b = c + 128;   // put back

        break;
        }


     case  9: // Gamma         
        {
        double c;

        if (Options < 0.0)
          Options = 0.0;

        c = ( (double) r) / 255.0;  // normalize it
        c = pow (c, Options);
        r = c * 255.0;

        c = ( (double) g) / 255.0;  // normalize it
        c = pow (c, Options);
        g = c * 255.0;

        c = ( (double) b) / 255.0;  // normalize it
        c = pow (c, Options);
        b = c * 255.0;

        break;
        }

     case  10: // ColourBrightness - red     
        {
        r += Options;
        break;
        }

     case  11: // ColourContrast - red        
        {
        double c;

        c = r - 128;   // center on zero
        c *= Options;  // multiply by contrast
        r = c + 128;   // put back

        break;
        }

     case  12: // ColourGamma - red         
        {
        double c;

        if (Options < 0.0)
          Options = 0.0;

        c = ( (double) r) / 255.0;  // normalize it
        c = pow (c, Options);
        r = c * 255.0;

        break;
        }

     case  13: // ColourBrightness - green     
        {
        g += Options;
        break;
        }

     case  14: // ColourContrast - green        
        {
        double c;

        c = g - 128;   // center on zero
        c *= Options;  // multiply by contrast
        g = c + 128;   // put back

        break;
        }

     case  15: // ColourGamma - green         
        {
        double c;

        if (Options < 0.0)
          Options = 0.0;

        c = ( (double) g) / 255.0;  // normalize it
        c = pow (c, Options);
        g = c * 255.0;

        break;
        }

     case  16: // ColourBrightness - blue     
        {
        b += Options;
        break;
        }

     case  17: // ColourContrast - blue        
        {
        double c;

        c = b - 128;   // center on zero
        c *= Options;  // multiply by contrast
        b = c + 128;   // put back

        break;
        }

     case  18: // ColourGamma - blue         
        {
        double c;

        if (Options < 0.0)
          Options = 0.0;

        c = ( (double) b) / 255.0;  // normalize it
        c = pow (c, Options);
        b = c * 255.0;

        break;
        }

     case 19: // MakeGreyscale  - linear
       {
        double c;
        c =  r + g + b;
        c /= 3;
        r = c;
        g = c;
        b = c;
        break;
       }

     case 20: // MakeGreyscale  - perceptual
       {
        double c;
        c = b * 0.11 + // blue   (perceptual)
            g * 0.59 + // green
            r * 0.30;  // red
        c /= 3;
        r = c;
        g = c;
        b = c;
        break;
       }

     case  21: // Brightness - multiplicative    
        {
        r *= Options;
        g *= Options;
        b *= Options;
        break;
        }

     case  22: // Brightness - multiplicative - red    
        {
        r *= Options;
        break;
        }

     case  23: // Brightness - multiplicative - green    
        {
        g *= Options;
        break;
        }

     case  24: // Brightness - multiplicative - blue    
        {
        b *= Options;
        break;
        }


     case 27:  // Average (of 1 pixel is itself)
       break;

     default: return -1;

     } // end of switch


	return RGB (CLAMP (r), CLAMP (g), CLAMP (b));
}   // end of CMUSHclientDoc::FilterPixel

#define Blend_It(Op) \
   do \
      if (Opacity < 1.0) \
        {  \
          r =  Blend_Opacity (rA, rB, Op, Opacity);\
          g =  Blend_Opacity (gA, gB, Op, Opacity);\
          b =  Blend_Opacity (bA, bB, Op, Opacity);\
        }  \
      else\
        {  \
          r = Op (rA, rB);\
          g = Op (gA, gB);\
          b = Op (bA, bB);\
        }  \
   while (false)


#define Colour_Op(fR,fG,fB) \
   do \
     {  \
      if (Opacity < 1.0) \
          {  \
          r = Simple_Opacity (rB, fR, Opacity); \
          g = Simple_Opacity (gB, fG, Opacity); \
          b = Simple_Opacity (bB, fB, Opacity); \
          }\
      else\
          {\
          r = fR;  \
          g = fG;  \
          b = fB;  \
          }\
       }\
   while (false)


// see also: CMiniWindow::BlendImage

long CMUSHclientDoc::BlendPixel(long Blend, long Base, short Mode, double Opacity) 
{
long rA = GetRValue (Blend),
     gA = GetGValue (Blend),
     bA = GetBValue (Blend);
long rB = GetRValue (Base),
     gB = GetGValue (Base),
     bB = GetBValue (Base);

long r, g, b;

  if (Opacity < 0.0 || Opacity > 1.0)
    return -2;

  // precompute cos table
  static unsigned char cos_table [256];
  static bool bTableComputed = false;

  if (!bTableComputed)
    {
    double pi_div255 = 3.1415926535898 / 255.0;
    long i;
    for (i = 0; i < 256; i++)
      {
      double a = 64.0 - cos ((double) i * pi_div255) * 64.0;
      cos_table [i] = (uint8) (a + 0.5);  // round
      }
    bTableComputed = true;
    } // table needs computing

  switch (Mode)
    {

    // normal modes
    case  1: Blend_It (Blend_Normal);     break;
    case  2: Blend_It (Blend_Average);    break;
    case  3: Blend_It (Blend_Interpolate);  break;

    case  4:  // dissolve - randomly choose pixels based on opacity
      {
       double rnd = genrand ();

        r = (rnd < Opacity) ? rA : rB;
        g = (rnd < Opacity) ? gA : gB;
        b = (rnd < Opacity) ? bA : bB;
      }
      break;


    // darkening modes
    case  5: Blend_It (Blend_Darken);     break;
    case  6: Blend_It (Blend_Multiply);   break;
    case  7: Blend_It (Blend_ColorBurn);  break;
    case  8: Blend_It (Blend_LinearBurn); break;
    case  9: Blend_It (Blend_InverseColorBurn);  break;
    case 10: Blend_It (Blend_Subtract);   break;
    
    // lightening modes
    case 11: Blend_It (Blend_Lighten);    break;
    case 12: Blend_It (Blend_Screen);     break;
    case 13: Blend_It (Blend_ColorDodge); break;
    case 14: Blend_It (Blend_LinearDodge);break;
    case 15: Blend_It (Blend_InverseColorDodge);  break;
    case 16: Blend_It (Blend_Add);        break;
      
    // soft/hard light etc.
    case 17: Blend_It (Blend_Overlay);    break;
    case 18: Blend_It (Blend_SoftLight);  break;
    case 19: Blend_It (Blend_HardLight);  break;
    case 20: Blend_It (Blend_VividLight); break;
    case 21: Blend_It (Blend_LinearLight);break;
    case 22: Blend_It (Blend_PinLight);   break;
    case 23: Blend_It (Blend_HardMix);    break;

    // difference modes
    case 24: Blend_It (Blend_Difference); break;
    case 25: Blend_It (Blend_Exclusion);  break;

    // glow modes
    case 26: Blend_It (Blend_Reflect);    break;
    case 27: Blend_It (Blend_Glow);       break;
    case 28: Blend_It (Blend_Freeze);     break;
    case 29: Blend_It (Blend_Heat);       break;
        
    case 30: Blend_It (Blend_Negation);   break;
    case 31: Blend_It (Blend_Phoenix);    break;
    case 32: Blend_It (Blend_Stamp);      break;

    // logical modes
    case 33: Blend_It (Blend_Xor);      break;
    case 34: Blend_It (Blend_And);      break;
    case 35: Blend_It (Blend_Or);       break;

    // the follow modes take one colour from the blend and retain 2 from the base
    case 36: Colour_Op (rA, gB, bB); break;  // red  
    case 37: Colour_Op (rB, gA, bB); break;  // green
    case 38: Colour_Op (rB, gB, bA); break;  // blue 

    // the follow modes take two colours from the blend and retain 1 from the base
    case 39: Colour_Op (rA, gA, bB); break;  // yellow 
    case 40: Colour_Op (rB, gA, bA); break;  // cyan    
    case 41: Colour_Op (rA, gB, bA); break;  // magenta    

    // limit green
    case 42: Colour_Op (rA, (gA > rA) ? rA : gA, bA); break;  // green limited by red    
    case 43: Colour_Op (rA, (gA > bA) ? bA : gA, bA); break;  // green limited by blue    
    case 44: Colour_Op (rA, (gA > ((rA + bA) / 2)) ? ((rA + bA) / 2) : gA, bA); break;  // green limited by average of red and blue    

    // limit blue
    case 45: Colour_Op (rA, gA, (bA > rA) ? rA : bA); break;  // blue limited by red    
    case 46: Colour_Op (rA, gA, (bA > gA) ? gA : bA); break;  // blue limited by green    
    case 47: Colour_Op (rA, gA, (bA > ((rA + gA) / 2)) ? ((rA + gA) / 2) : bA); break; // blue limited by average of red and green    

    // limit red
    case 48: Colour_Op ((rA > gA) ? gA : rA, gA, bA); break;  // red limited by green    
    case 49: Colour_Op ((rA > bA) ? bA : rA, gA, bA); break;  // red limited by blue    
    case 50: Colour_Op ((rA > ((gA + bA) / 2)) ? ((gA + bA) / 2) : rA, gA, bA); break;  // red limited by average of green and blue    

    // select single colour
    case 51: Colour_Op (rA, 0, 0); break;  // red only       (looks red)
    case 52: Colour_Op (0, gA, 0); break;  // green only     (looks green)
    case 53: Colour_Op (0, 0, bA); break;  // blue only      (looks blue)

    // discard single colour
    case 54: Colour_Op (0, gA, bA); break;  // discard red    (looks cyan)
    case 55: Colour_Op (rA, 0, bA); break;  // discard green  (looks magenta)
    case 56: Colour_Op (rA, gA, 0); break;  // discard blue   (looks yellow)

    // one colour to all channels  (ie. it looks grey)
    case 57: Colour_Op (rA, rA, rA); break;  // all red   
    case 58: Colour_Op (gA, gA, gA); break;  // all green   
    case 59: Colour_Op (bA, bA, bA); break;  // all blue   

    case 60:           // Hue mode
      {
          CColor cA, cB;    // A = blend, B = base

          cA.SetRGB (rA, gA, bA);
          cB.SetRGB (rB, gB, bB);

          cB.SetHue (cA.GetHue ());  // hue of blend image, others from base image
          r = Simple_Opacity (rB, cB.GetRed (), Opacity); 
          g = Simple_Opacity (gB, cB.GetGreen (), Opacity); 
          b = Simple_Opacity (bB, cB.GetBlue (), Opacity); 
       }
      break;

    case 61:           // Saturation mode
        {
          CColor cA, cB;    // A = blend, B = base

          cA.SetRGB (rA, gA, bA);
          cB.SetRGB (rB, gB, bB);

          cB.SetSaturation (cA.GetSaturation ());  // saturation of blend image, others from base image
          r = Simple_Opacity (rB, cB.GetRed (), Opacity); 
          g = Simple_Opacity (gB, cB.GetGreen (), Opacity); 
          b = Simple_Opacity (bB, cB.GetBlue (), Opacity); 
       }
      break;

    case 62:           // Colour mode
      {
          CColor cA, cB;    // A = blend, B = base

          cA.SetRGB (rA, gA, bA);
          cB.SetRGB (rB, gB, bB);

          cB.SetHue (cA.GetHue ());  // hue of blend image, 
          cB.SetSaturation (cA.GetSaturation ());  // saturation of blend image, luminance from base image
          r = Simple_Opacity (rB, cB.GetRed (), Opacity); 
          g = Simple_Opacity (gB, cB.GetGreen (), Opacity); 
          b = Simple_Opacity (bB, cB.GetBlue (), Opacity); 
       }
      break;


    case 63:           // Luminance mode
      {
          CColor cA, cB;    // A = blend, B = base

          cA.SetRGB (rA, gA, bA);
          cB.SetRGB (rB, gB, bB);

          cB.SetLuminance (cA.GetLuminance ());  // luminance from blend image, others from base image

          r = Simple_Opacity (rB, cB.GetRed (), Opacity); 
          g = Simple_Opacity (gB, cB.GetGreen (), Opacity); 
          b = Simple_Opacity (bB, cB.GetBlue (), Opacity); 

       }
      break;

    case 64:           // HSL (hue to red, saturation to green, luminance to blue)
        {
          CColor cA, cB;    // A = blend, B = base
          cA.SetRGB (rA, gA, bA);

          r = Simple_Opacity (rB, cB.GetRed (), Opacity); 
          g = Simple_Opacity (gB, cB.GetGreen (), Opacity); 
          b = Simple_Opacity (bB, cB.GetBlue (), Opacity); 

         }
      break;

     default: return -1;

     } // end of switch


	return RGB (CLAMP (r), CLAMP (g), CLAMP (b));
} // end of CMUSHclientDoc::BlendPixel


long CMUSHclientDoc::WindowMergeImageAlpha(LPCTSTR Name, 
                                           LPCTSTR ImageId, LPCTSTR MaskId, 
                                           long Left, long Top, long Right, long Bottom, 
                                           short Mode, 
                                           double Opacity, 
                                           long SrcLeft, long SrcTop, long SrcRight, long SrcBottom) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->MergeImageAlpha (ImageId, MaskId, Left, Top, Right, Bottom, Mode, Opacity, SrcLeft, SrcTop, SrcRight, SrcBottom);
}    // end of CMUSHclientDoc::WindowMergeImageAlpha


long CMUSHclientDoc::WindowDelete(LPCTSTR Name) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  delete it->second;

  m_MiniWindows.erase (it);

  UpdateAllViews (NULL);

	return eOK;
}    // end of CMUSHclientDoc::WindowDelete


BSTR CMUSHclientDoc::WindowMenu(LPCTSTR Name, long Left, long Top, LPCTSTR Items) 
{

  CString strResult;

  MiniWindowMapIterator it = m_MiniWindows.find (Name);

	CView* pView = NULL;
  
  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  pView = GetNextView(pos);

	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
      break;

    }

  if (pView && it != m_MiniWindows.end ())
    strResult = it->second->Menu (Left, Top, Items, (CMUSHView *) pView);

	return strResult.AllocSysString();

}   // end of CMUSHclientDoc::WindowMenu


long CMUSHclientDoc::SetCommandSelection(long First, long Last) 
{
  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	  {
		  CSendView* pmyView = (CSendView*)pView;
      pmyView->GetEditCtrl().SetSel(First - 1, Last, FALSE);

	    }	  // end of being a CSendView
    }   // end of loop through views

	return eOK;

}  // end of CMUSHclientDoc::SetCommandSelection

long CMUSHclientDoc::WindowDragHandler(LPCTSTR Name, LPCTSTR HotspotId, LPCTSTR MoveCallback, LPCTSTR ReleaseCallback, long Flags) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  string sPluginID;

  if (m_CurrentPlugin)                            
    sPluginID = m_CurrentPlugin->m_strID;

  return it->second->DragHandler (this, HotspotId, sPluginID, MoveCallback, ReleaseCallback, Flags);
}   // end of CMUSHclientDoc::WindowDragHandler


long CMUSHclientDoc::WindowScrollwheelHandler(LPCTSTR Name, LPCTSTR HotspotId, LPCTSTR MoveCallback) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  string sPluginID;

  if (m_CurrentPlugin)                            
    sPluginID = m_CurrentPlugin->m_strID;

  return it->second->ScrollwheelHandler (this, HotspotId, sPluginID, MoveCallback);
}   // end of CMUSHclientDoc::WindowScrollwheelHandler


long CMUSHclientDoc::SetCursor(long Cursor) 
{
  switch (Cursor)
    {
    case -1:  ::SetCursor (NULL);                              break;    // no cursor
    case  0:  ::SetCursor (::LoadCursor (NULL, IDC_ARROW));    break;    // arrow
    case  1:  ::SetCursor (CStaticLink::g_hCursorLink);        break;    // hand
    case  2:  ::SetCursor (App.g_hCursorIbeam);                break;    // I-beam
    case  3:  ::SetCursor (::LoadCursor (NULL, IDC_CROSS));    break;    // + (cross)
    case  4:  ::SetCursor (::LoadCursor (NULL, IDC_WAIT));     break;    // wait (hour-glass)
    case  5:  ::SetCursor (::LoadCursor (NULL, IDC_UPARROW));  break;    // up arrow
    case  6:  ::SetCursor (::LoadCursor (NULL, IDC_SIZENWSE)); break;    // arrow nw-se
    case  7:  ::SetCursor (::LoadCursor (NULL, IDC_SIZENESW)); break;    // arrow ne-sw
    case  8:  ::SetCursor (::LoadCursor (NULL, IDC_SIZEWE));   break;    // arrow e-w
    case  9:  ::SetCursor (::LoadCursor (NULL, IDC_SIZENS));   break;    // arrow n-s
    case 10:  ::SetCursor (::LoadCursor (NULL, IDC_SIZEALL));  break;    // arrow - all ways
    case 11:  ::SetCursor (::LoadCursor (NULL, IDC_NO));       break;    // (X) no, no, I won't do that, but ...
    case 12:  ::SetCursor (::LoadCursor (NULL, IDC_HELP));     break;    // help  (? symbol)
    default:  return eBadParameter;
    } // end of switch

  return eOK;
}   // end of CMUSHclientDoc::SetCursor


//  Flags For File Open Operations
//  
//  #define SQLITE_OPEN_READONLY         0x00000001
//  #define SQLITE_OPEN_READWRITE        0x00000002
//  #define SQLITE_OPEN_CREATE           0x00000004
//  #define SQLITE_OPEN_DELETEONCLOSE    0x00000008
//  #define SQLITE_OPEN_EXCLUSIVE        0x00000010
//  #define SQLITE_OPEN_MAIN_DB          0x00000100
//  #define SQLITE_OPEN_TEMP_DB          0x00000200
//  #define SQLITE_OPEN_TRANSIENT_DB     0x00000400
//  #define SQLITE_OPEN_MAIN_JOURNAL     0x00000800
//  #define SQLITE_OPEN_TEMP_JOURNAL     0x00001000
//  #define SQLITE_OPEN_SUBJOURNAL       0x00002000
//  #define SQLITE_OPEN_MASTER_JOURNAL   0x00004000
//  #define SQLITE_OPEN_NOMUTEX          0x00008000
//  #define SQLITE_OPEN_FULLMUTEX        0x00010000


// Note: filename ":memory:" opens an in-memory database

long CMUSHclientDoc::DatabaseOpen(LPCTSTR Name, LPCTSTR Filename, long Flags) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);

  tDatabase * pDatabase = NULL;

  if (it == m_Databases.end ())
    {
    pDatabase = new tDatabase;

    pDatabase->db = NULL;
    pDatabase->pStmt = NULL;
    pDatabase->bValidRow = false;
    pDatabase->db_name = Filename;
    pDatabase->iColumns = 0;

    }
  else 
    {
    if (it->second->db_name == Filename)
      return SQLITE_OK;      // OK to re-use same database
    else
      return -6;   // database already exists under this id but a different disk name
    }

	int rc = sqlite3_open_v2 (Filename, &pDatabase->db, Flags, NULL);

  // add to map if opened OK
  if (rc == SQLITE_OK && pDatabase->db != NULL)
    m_Databases [Name] = pDatabase;
  else
    delete pDatabase;

  return rc;
}   // end of CMUSHclientDoc::DatabaseOpen


long CMUSHclientDoc::DatabaseClose(LPCTSTR Name) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return -1;              // database not found

  if  (it->second->db == NULL)
    return -2;              // database not open

  if (it->second->pStmt)        // finalize any outstanding statement
    sqlite3_finalize(it->second->pStmt);

  int rc = sqlite3_close(it->second->db);

  delete it->second;       // free up memory used by structure

  m_Databases.erase (it);  // remove from map

  return rc;               // return code from close
}   // end of CMUSHclientDoc::DatabaseClose

long CMUSHclientDoc::DatabasePrepare(LPCTSTR Name, LPCTSTR Sql) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return -1;                  // database not found

  if  (it->second->db == NULL)
    return -2;                  // database not open

  if  (it->second->pStmt != NULL)
    return -3;                  // already have prepared statement

  const char *pzTail;

  it->second->bValidRow = false;  // no valid row yet
  it->second->iColumns = 0;

  int rc = sqlite3_prepare_v2 (it->second->db, Sql, -1, &it->second->pStmt, &pzTail);

  // for future validation that columns are in range
  if (rc == SQLITE_OK)
     it->second->iColumns = sqlite3_column_count(it->second->pStmt);

  return rc;

}   // end of CMUSHclientDoc::DatabasePrepare

long CMUSHclientDoc::DatabaseFinalize(LPCTSTR Name) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return -1;                // database not found

  if  (it->second->db == NULL)
    return -2;               // database not open

  if  (it->second->pStmt == NULL)
    return -4;               // do not have prepared statement

  int rc = sqlite3_finalize(it->second->pStmt);  // finished with statement

  it->second->pStmt = NULL;     // show not in use
  it->second->bValidRow = false;  // no valid row
  it->second->iColumns = 0;   // no columns


	return rc;
}    // end of CMUSHclientDoc::DatabaseFinalize


// note: don't need to step first, the prepare gives us the count
long CMUSHclientDoc::DatabaseColumns(LPCTSTR Name) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return -1;                // database not found

  if  (it->second->db == NULL)
    return -2;               // database not open

  if  (it->second->pStmt == NULL)
    return -4;               // do not have prepared statement

  return sqlite3_column_count(it->second->pStmt);  // column count

}   // end of CMUSHclientDoc::DatabaseColumns


long CMUSHclientDoc::DatabaseStep(LPCTSTR Name) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return -1;                // database not found

  if  (it->second->db == NULL)
    return -2;               // database not open

  if  (it->second->pStmt == NULL)
    return -4;               // do not have prepared statement

  int rc = sqlite3_step (it->second->pStmt);  // step into next row

  it->second->bValidRow = rc == SQLITE_ROW;   // show if we have a row or not

  return rc;
}    // end of CMUSHclientDoc::DatabaseStep

BSTR CMUSHclientDoc::DatabaseError(LPCTSTR Name) 
{
	CString strResult;

  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    strResult = "database id not found";           // database not found
  else if (it->second->db == NULL)
    strResult = "database not open";       // database not open
  else
    {
    switch (sqlite3_errcode (it->second->db))
      {
      case SQLITE_ROW: strResult = "row ready"; break;
      case SQLITE_DONE: strResult = "finished"; break;
      default:
          strResult = sqlite3_errmsg (it->second->db);
          break;
      } // end of switch
    }

	return strResult.AllocSysString();
}   // end of CMUSHclientDoc::DatabaseError


// note: don't need to step first, the prepare gives us the names
BSTR CMUSHclientDoc::DatabaseColumnName(LPCTSTR Name, long Column) 
{
	CString strResult;

  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it != m_Databases.end () &&    // database exists
      it->second->db != NULL &&      // and is open
      it->second->pStmt != NULL &&   // and we have a prepared statement
      Column >= 1 &&
      Column <= it->second->iColumns)
    {
    const char * p = sqlite3_column_name (it->second->pStmt, Column - 1);

    if (p)
      strResult = p;
    }

	return strResult.AllocSysString();
}  // end of CMUSHclientDoc::DatabaseColumnName


BSTR CMUSHclientDoc::DatabaseColumnText(LPCTSTR Name, long Column) 
{
	CString strResult;

  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it != m_Databases.end () &&    // database exists
      it->second->db != NULL &&      // and is open
      it->second->pStmt != NULL &&   // and we have a prepared statement
      it->second->bValidRow &&       // and we stepped to a valid row
      Column >= 1 &&
      Column <= it->second->iColumns)
    {
    const unsigned char * p = sqlite3_column_text (it->second->pStmt, Column - 1);

    if (p)
      strResult = p;
    }

	return strResult.AllocSysString();
}   // end of CMUSHclientDoc::DatabaseColumnText

VARIANT CMUSHclientDoc::DatabaseColumnValue(LPCTSTR Name, long Column) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);
  vaResult.vt = VT_NULL;

  tDatabaseMapIterator it = m_Databases.find (Name);

  if (it != m_Databases.end () &&    // database exists
      it->second->db != NULL &&      // and is open
      it->second->pStmt != NULL &&   // and we have a prepared statement
      it->second->bValidRow &&       // and we stepped to a valid row
      Column >= 1 &&
      Column <= it->second->iColumns)
    {
    int iType = sqlite3_column_type(it->second->pStmt, Column - 1);

    switch (iType)
      {
      case SQLITE3_TEXT:
      case SQLITE_BLOB:
      default:
        {
        CString strResult;
        const unsigned char * p = sqlite3_column_text (it->second->pStmt, Column - 1);
        if (p)
          strResult = p;
        SetUpVariantString (vaResult, strResult); 
        }
        break;

      case SQLITE_NULL:
        vaResult.vt = VT_EMPTY;
        break;

      case SQLITE_INTEGER:
        {
        int iResult =  sqlite3_column_int  (it->second->pStmt, Column - 1);
        SetUpVariantLong (vaResult, iResult); 
        }
        break;

      case SQLITE_FLOAT:
        {
        double fResult =  sqlite3_column_double (it->second->pStmt, Column - 1);
        SetUpVariantDouble (vaResult, fResult); 
        }
        break;

      }  // end of switch

    }  // end of something valid



	return vaResult;
}     // end of CMUSHclientDoc::DatabaseColumnValue



// #define SQLITE_INTEGER  1
// #define SQLITE_FLOAT    2
// #define SQLITE3_TEXT    3
// #define SQLITE_BLOB     4
// #define SQLITE_NULL     5


long CMUSHclientDoc::DatabaseColumnType(LPCTSTR Name, long Column) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return -1;                // database not found

  if  (it->second->db == NULL)
    return -2;               // database not open

  if  (it->second->pStmt == NULL)
    return -4;               // do not have prepared statement

  if  (!it->second->bValidRow)
    return -5;               // do not have a valid row

  if  (Column < 1 || Column > it->second->iColumns)
    return -7;               // column count out of valid range

  return sqlite3_column_type(it->second->pStmt, Column - 1);  // column type
}     // end of CMUSHclientDoc::DatabaseColumnType


long CMUSHclientDoc::DatabaseTotalChanges(LPCTSTR Name) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return -1;                // database not found

  if  (it->second->db == NULL)
    return -2;               // database not open

  return sqlite3_total_changes(it->second->db); 

}    // end of CMUSHclientDoc::DatabaseTotalChanges


long CMUSHclientDoc::DatabaseChanges(LPCTSTR Name) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return -1;                // database not found

  if  (it->second->db == NULL)
    return -2;               // database not open

  return sqlite3_changes(it->second->db); 
}   // end of CMUSHclientDoc::DatabaseChanges


BSTR CMUSHclientDoc::DatabaseLastInsertRowid(LPCTSTR Name) 
{
	CString strResult;

  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it != m_Databases.end () &&    // database exists
      it->second->db != NULL)
    {
    strResult.Format("%I64", sqlite3_last_insert_rowid(it->second->db));
    }

	return strResult.AllocSysString();
}    // end of CMUSHclientDoc::DatabaseLastInsertRowid

VARIANT CMUSHclientDoc::DatabaseList() 
{
  COleSafeArray sa;   // for array list

  long iCount = 0;
  
  // put the databases into the array
  if (!m_Databases.empty ())    // cannot create empty dimension
    {
    sa.CreateOneDim (VT_VARIANT, m_Databases.size ());

    for (tDatabaseMapIterator it = m_Databases.begin (); 
         it != m_Databases.end ();
         it++)
           {
            // the array must be a bloody array of variants, or VBscript kicks up
            COleVariant v (it->first.c_str ());
            sa.PutElement (&iCount, &v);
            iCount++;
           }

    } // end of having at least one

	return sa.Detach ();
}    // end of CMUSHclientDoc::DatabaseList


VARIANT CMUSHclientDoc::DatabaseInfo(LPCTSTR Name, long InfoType) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;  

  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
  	return vaResult;

  tDatabase * pDatabase = it->second;

  switch (InfoType)
    {
    case 1:  SetUpVariantString  (vaResult, pDatabase->db_name.c_str ()); break; // disk name
    case 2:  SetUpVariantBool    (vaResult, pDatabase->pStmt != NULL);   break;  // valid prepared statement
    case 3:  SetUpVariantBool    (vaResult, pDatabase->bValidRow);       break;  // valid row returned from last step
    case 4:  SetUpVariantLong    (vaResult, pDatabase->iColumns);        break;  // number of columns 

    default:
      vaResult.vt = VT_NULL;
      break;

    } // end of switch


	return vaResult;
}   // end of CMUSHclientDoc::DatabaseInfo


long CMUSHclientDoc::DatabaseExec(LPCTSTR Name, LPCTSTR Sql) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return -1;                  // database not found

  if  (it->second->db == NULL)
    return -2;                  // database not open

  if  (it->second->pStmt != NULL)
    return -3;                  // already have prepared statement

  it->second->bValidRow = false;  // no valid row yet
  it->second->iColumns = 0;

  return sqlite3_exec (it->second->db, Sql, NULL, NULL, NULL);
}   // end of CMUSHclientDoc::DatabaseExec

// returns an array of column names
VARIANT CMUSHclientDoc::DatabaseColumnNames(LPCTSTR Name) 
{
  COleSafeArray sa;   // for array list

 
  tDatabaseMapIterator it = m_Databases.find (Name);

  if (it != m_Databases.end () &&    // database exists
      it->second->db != NULL &&      // and is open
      it->second->pStmt != NULL)  // and we have a prepared statement
    {
    // put the column names into the array
    if (it->second->iColumns > 0)    // cannot create empty dimension
      {
      sa.CreateOneDim (VT_VARIANT, it->second->iColumns);

      for (long i = 0; i < it->second->iColumns; i++)
         {

         CString strResult;

         const char * p = sqlite3_column_name (it->second->pStmt, i);

         if (p)
           strResult = p;

         COleVariant v (strResult);
         sa.PutElement (&i, &v);
         }

      } // end of having at least one

    }

	return sa.Detach ();
}  // end of CMUSHclientDoc::DatabaseColumnNames

// returns an array of column values
VARIANT CMUSHclientDoc::DatabaseColumnValues(LPCTSTR Name) 
{
  COleSafeArray sa;   // for array list

 
  tDatabaseMapIterator it = m_Databases.find (Name);

  if (it != m_Databases.end () &&    // database exists
      it->second->db != NULL &&      // and is open
      it->second->pStmt != NULL &&   // and we have a prepared statement
      it->second->bValidRow)         // and have a valid row
    {
    // put the column values into the array
    if (it->second->iColumns > 0)    // cannot create empty dimension
      {
      sa.CreateOneDim (VT_VARIANT, it->second->iColumns);

      for (long i = 0; i < it->second->iColumns; i++)
         {

        int iType = sqlite3_column_type(it->second->pStmt, i);
        COleVariant v ;

        switch (iType)
          {
          case SQLITE3_TEXT:
          case SQLITE_BLOB:
          default:
            {
            CString strResult;
            const unsigned char * p = sqlite3_column_text (it->second->pStmt, i);
            if (p)
              strResult = p;
            v = strResult; 
            }
            break;

          case SQLITE_NULL:
            v.ChangeType (VT_NULL);
            break;

          case SQLITE_INTEGER:
            {
            long iResult =  sqlite3_column_int  (it->second->pStmt, i);
            v = iResult; 
            }
            break;

          case SQLITE_FLOAT:
            {
            double fResult =  sqlite3_column_double (it->second->pStmt, i);
            v = fResult; 
            }
            break;

          }  // end of switch
         sa.PutElement (&i, &v);

         }

      } // end of having at least one

    }

	return sa.Detach ();
}  // end of  CMUSHclientDoc::DatabaseColumnNames


long CMUSHclientDoc::DatabaseReset(LPCTSTR Name) 
{
  tDatabaseMapIterator it = m_Databases.find (Name);
    
  if (it == m_Databases.end ())
    return -1;                // database not found

  if  (it->second->db == NULL)
    return -2;               // database not open

  if  (it->second->pStmt == NULL)
    return -4;               // do not have prepared statement

  return sqlite3_reset(it->second->pStmt);  // reset statement

}  // end of  CMUSHclientDoc::DatabaseReset


void CMUSHclientDoc::FlashIcon() 
{
Frame.FlashWindow (TRUE);
}    // end of CMUSHclientDoc::FlashIcon


long CMUSHclientDoc::WindowHotspotTooltip(LPCTSTR Name, LPCTSTR HotspotId, LPCTSTR TooltipText) 
{

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return eNoSuchWindow;

  return it->second->HotspotTooltip (HotspotId, TooltipText);
}    // end of CMUSHclientDoc::WindowHotspotTooltip

long CMUSHclientDoc::WindowDrawImageAlpha(LPCTSTR Name, LPCTSTR ImageId, long Left, long Top, long Right, long Bottom, double Opacity, long SrcLeft, long SrcTop) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->DrawImageAlpha (ImageId, Left, Top, Right, Bottom, Opacity, 
                                    SrcLeft,   SrcTop);

}  // end of CMUSHclientDoc::WindowDrawImageAlpha

long CMUSHclientDoc::WindowGetImageAlpha(LPCTSTR Name, LPCTSTR ImageId, long Left, long Top, long Right, long Bottom, long SrcLeft, long SrcTop) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->GetImageAlpha (ImageId, Left, Top, Right, Bottom, 
                                    SrcLeft,   SrcTop);

}   // end of CMUSHclientDoc::WindowGetImageAlpha



long CMUSHclientDoc::WindowResize(LPCTSTR Name, long Width, long Height, long BackgroundColour) 
{

  if (Width < 0 || Height < 0)
    return eBadParameter;

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Resize (Width, Height, BackgroundColour);
}  // end of CMUSHclientDoc::WindowResize
     

long CMUSHclientDoc::WindowMoveHotspot(LPCTSTR Name, LPCTSTR HotspotId, long Left, long Top, long Right, long Bottom) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return eNoSuchWindow;

  return it->second->MoveHotspot (HotspotId, Left, Top, Right, Bottom);
}   // end of CMUSHclientDoc::WindowMoveHotspot


long CMUSHclientDoc::WindowTransformImage(LPCTSTR Name, LPCTSTR ImageId, float Left, float Top, short Mode, float Mxx, float Mxy, float Myx, float Myy) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->TransformImage (ImageId, Left, Top, Mode, Mxx, Mxy, Myx, Myy);
}



long CMUSHclientDoc::SetScroll(long Position, BOOL Visible) 
{

CPoint pt (0, 0);
int lastline = GetLastLine ();
m_bScrollBarWanted = Visible;

  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
  	  {
		  CMUSHView* pmyView = (CMUSHView*)pView;

      int highest = (lastline * m_FontHeight) - pmyView->GetOutputWindowHeight ();

      // -1 goes to the end
      if (Position == -1)
        pt.y = highest; 
      else
        pt.y = Position;

      if (pt.y < 0)
        pt.y = 0;
      if (pt.y > highest)
        pt.y = highest;

      pmyView->EnableScrollBarCtrl (SB_VERT, Visible);
      if (Position != -2)      // if -2, do not change position
        pmyView->ScrollToPosition (pt, false);
      pmyView->Invalidate ();

	    }	  // end of being a CMUSHView
    }   // end of loop through views


	return eOK;
}


/*

  ======================================================================

  When adding a new script function, remember to do this as well:

  1. Add the Lua "glue" routine to lua_methods.cpp.
  
  2. Add the name of the function to the table in  functionlist.cpp 

  3. Add to the function documentation

  4. Add to release notes.

  5. Add to the help "contents" file: mushclient.cnt


  (please leave this comment at the bottom where it will hopefully be seen).

  ======================================================================

*/


