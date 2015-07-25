// methods_triggers.cpp

// Related to triggers

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\..\MUSHview.h"
#include "..\..\mainfrm.h"
#include "..\..\flags.h"
#include "..\errors.h"

// Implements:

//    AddTrigger
//    AddTriggerEx
//    DeleteTemporaryTriggers
//    DeleteTrigger
//    DeleteTriggerGroup
//    EnableTrigger
//    EnableTriggerGroup
//    GetTrigger
//    GetTriggerInfo
//    GetTriggerList
//    GetTriggerOption
//    GetTriggerWildcard
//    IsTrigger
//    SetTriggerOption
//    StopEvaluatingTriggers


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
}    // end of CMUSHclientDoc::DeleteTrigger

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
}     // end of CMUSHclientDoc::AddTriggerEx

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
}  // end of CMUSHclientDoc::AddTrigger

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
}     // end of CMUSHclientDoc::EnableTrigger

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
}   // end of CMUSHclientDoc::GetTrigger

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
}   // end of CMUSHclientDoc::IsTrigger

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
}    // end of CMUSHclientDoc::GetTriggerList


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
}   // end of CMUSHclientDoc::GetTriggerInfo


long CMUSHclientDoc::DeleteTemporaryTriggers() 
{
long iCount = 0;
POSITION pos;
CString strTriggerName;
CTrigger * trigger_item;

  for (pos = GetTriggerMap ().GetStartPosition(); pos; )
    {
    GetTriggerMap ().GetNextAssoc (pos, strTriggerName, trigger_item);
    if (trigger_item->bTemporary && !trigger_item->bExecutingScript)
      {
      delete trigger_item;
      GetTriggerMap ().RemoveKey (strTriggerName);
      iCount++;
      }
    }   // end of deleting triggers

  SortTriggers ();

	return iCount;
}   // end of CMUSHclientDoc::DeleteTemporaryTriggers


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
}  // end of CMUSHclientDoc::GetTriggerWildcard

void CMUSHclientDoc::StopEvaluatingTriggers(BOOL AllPlugins) 
  {
  m_iStopTriggerEvaluation = AllPlugins ? eStopEvaluatingTriggersInAllPlugins : eStopEvaluatingTriggers;
  }   // end of CMUSHclientDoc::StopEvaluatingTriggers

