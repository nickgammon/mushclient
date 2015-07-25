// methods_timers.cpp

// Related to timers (and DoAfter, stuff like that)

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\..\flags.h"
#include "..\errors.h"

// Implements:

//    AddTimer
//    DeleteTemporaryTimers
//    DeleteTimer
//    DeleteTimerGroup
//    DoAfter
//    DoAfterNote
//    DoAfterSpecial
//    DoAfterSpeedWalk
//    EnableTimer
//    EnableTimerGroup
//    GetTimer
//    GetTimerInfo
//    GetTimerList
//    GetTimerOption
//    IsTimer
//    ResetTimer
//    ResetTimers
//    SetTimerOption


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
}  // end of CMUSHclientDoc::EnableTimer

// world.ResetTimers - resets all timers

void CMUSHclientDoc::ResetTimers() 
{
  ResetAllTimers (GetTimerMap ());
}   // end of CMUSHclientDoc::ResetTimers

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
}  // end of CMUSHclientDoc::ResetTimers



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
}  // end of CMUSHclientDoc::AddTimer

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
}   // end of CMUSHclientDoc::DeleteTimer

long CMUSHclientDoc::IsTimer(LPCTSTR TimerName) 
{
CString strTimerName = TimerName;
CTimer * timer_item;

  // trim spaces from name, make lower-case
  CheckObjectName (strTimerName, false);

  if (!GetTimerMap ().Lookup (strTimerName, timer_item))
    return eTimerNotFound;

	return eOK;
}   // end of CMUSHclientDoc::IsTimer

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
}  // end of CMUSHclientDoc::GetTimerList

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
}   // end of CMUSHclientDoc::GetTimer

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
}     // end of CMUSHclientDoc::GetTimerInfo


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
}   // end of CMUSHclientDoc::DoAfterSpecial



// world.DoAfter - does a command after n seconds
//   eg. world.DoAfter 10, "go west"

long CMUSHclientDoc::DoAfter(double Seconds, LPCTSTR SendText) 
{
  return DoAfterSpecial (Seconds, SendText, eSendToWorld);
}    // end of CMUSHclientDoc::DoAfter

// world.DoAfterSpeedWalk - does a speedwalk after n seconds
//   eg. world.DoAfterSpeedWalk 10, "4n 5w"

long CMUSHclientDoc::DoAfterSpeedWalk(double Seconds, LPCTSTR SendText) 
{
  return DoAfterSpecial (Seconds, SendText, eSendToSpeedwalk);
}   // end of CMUSHclientDoc::DoAfterSpeedWalk


// world.DoAfterNote - does a note after n seconds
//   eg. world.DoAfterNote 10, "tick"

long CMUSHclientDoc::DoAfterNote(double Seconds, LPCTSTR NoteText) 
{
  return DoAfterSpecial (Seconds, NoteText, eSendToOutput);
}    // end of CMUSHclientDoc::DoAfterNote


long CMUSHclientDoc::DeleteTemporaryTimers() 
{
long iCount = 0;
POSITION pos;
CString strTimerName;
CTimer * timer_item;

  for (pos = GetTimerMap ().GetStartPosition(); pos; )
    {
    GetTimerMap ().GetNextAssoc (pos, strTimerName, timer_item);
    if (timer_item->bTemporary && !timer_item->bExecutingScript)
      {
      delete timer_item;
      GetTimerMap ().RemoveKey (strTimerName);
      iCount++;
      }
    }   // end of deleting timers

  if (iCount)
    SortTimers ();

	return iCount;
}  // end of CMUSHclientDoc::DeleteTemporaryTimers


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

