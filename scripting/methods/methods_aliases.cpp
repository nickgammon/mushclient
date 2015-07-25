// methods_aliases.cpp  
// 
// Aliases processing

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\..\MUSHview.h"
#include "..\..\mainfrm.h"
#include "..\..\flags.h"
#include "..\errors.h"


// Implements:

//    AddAlias
//    DeleteAlias
//    DeleteAliasGroup
//    DeleteTemporaryAliases
//    EnableAlias
//    EnableAliasGroup
//    GetAlias
//    GetAliasInfo
//    GetAliasList
//    GetAliasOption
//    GetAliasWildcard
//    IsAlias
//    SetAliasOption


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
}   // end of CMUSHclientDoc::DeleteAlias

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
  alias_item->bKeepEvaluating  = (Flags & eKeepEvaluating) != 0;
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
}   // end of CMUSHclientDoc::AddAlias

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
}    // end of CMUSHclientDoc::EnableAlias

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
}   // end of CMUSHclientDoc::GetAlias

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
}       // end of CMUSHclientDoc::GetAliasList

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
}   // end of CMUSHclientDoc::IsAlias


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
}  // end of  CMUSHclientDoc::GetAliasInfo


long CMUSHclientDoc::DeleteTemporaryAliases() 
{
long iCount = 0;
POSITION pos;
CString strAliasName;
CAlias * alias_item;

  for (pos = GetAliasMap ().GetStartPosition(); pos; )
    {
    GetAliasMap ().GetNextAssoc (pos, strAliasName, alias_item);
    if (alias_item->bTemporary && !alias_item->bExecutingScript)
      {
      delete alias_item;
      GetAliasMap ().RemoveKey (strAliasName);
      iCount++;
      }
    }   // end of deleting aliases

  SortAliases ();

	return iCount;
}


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
  
}  // end of CMUSHclientDoc::GetAliasWildcard
