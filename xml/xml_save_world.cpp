// xml_save_world.cpp : XML world serialization (saving)
//

#include "stdafx.h"
#include "..\MUSHclient.h"
#include "..\doc.h"
#include "..\MUSHview.h"
#include "..\genprint.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern tConfigurationNumericOption OptionsTable [];
extern tConfigurationAlphaOption AlphaOptionsTable [];

void SortAliases (CAliasMap & AliasMap,
                  CAliasArray & AliasArray);

void SortVariables (CVariableMap & VariableMap,
                    CVariableArray & VariableArray);

#define NL "\r\n"

void CMUSHclientDoc::Save_Version_And_Date_XML (CArchive& ar)
  {

  Save_XML_string (ar, "muclient_version", MUSHCLIENT_VERSION);
  Save_XML_number (ar, "world_file_version", VERSION);
  Save_XML_date   (ar, "date_saved", CTime::GetCurrentTime());

  } // end of CMUSHclientDoc::Save_Version_And_Date_XML

void CMUSHclientDoc::Save_World_XML (CArchive& ar, 
                                     const unsigned long iMask,
                                     CString strComment)
  {

  // xml prolog
  ar.WriteString ("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" NL);
  // document type is world
  ar.WriteString ("<!DOCTYPE muclient>" NL);

  // when did we save it?
  ar.WriteString (CFormat ("<!-- Saved on %s -->" NL, 
        FixHTMLString (CTime::GetCurrentTime().Format 
        (TranslateTime ("%A, %B %d, %Y, %#I:%M %p")))));

  // which version was it?
  ar.WriteString (CFormat 
        ("<!-- MuClient version %s -->" NL, MUSHCLIENT_VERSION));

  ar.WriteString ("<!-- Written by Nick Gammon -->" NL);
  ar.WriteString ("<!-- Home Page: http://www.mushclient.com/ -->" NL);

  // comment (eg. save-state for world X )
  if (!strComment.IsEmpty ())
    {

    // cannot have "--" in comments
    while (strComment.Find ("--") != -1)
      strComment.Replace ("--", "-");

    ar.WriteString (CFormat (NL "<!-- %s -->" NL NL, (LPCTSTR) strComment));
    }

  // ok - ready to go
  ar.WriteString ("<muclient>" NL);
  
  // write out any included files                 
  if (iMask & XML_INCLUDES)
    Save_Includes_XML (ar);

  // general options
  if (iMask & XML_GENERAL)
    Save_General_XML (ar);

  // triggers 
  if (iMask & XML_TRIGGERS)
    Save_Triggers_XML (ar);

  // aliases 
  if (iMask & XML_ALIASES)
    Save_Aliases_XML (ar);

  // timers 
  if (iMask & XML_TIMERS)
    Save_Timers_XML (ar);

  // macros
  if (iMask & XML_MACROS)
    Save_Macros_XML (ar);

  // variables 
  if (iMask & XML_VARIABLES)
    Save_Variables_XML (ar);

  // colours
  if (iMask & XML_COLOURS)
    Save_Colours_XML (ar);

  // numeric keypad
  if (iMask & XML_KEYPAD)
    Save_Keypad_XML (ar);

  // printing stuff
  if (iMask & XML_PRINTING)
    Save_Printing_XML (ar);

  // write out any plugin files                 
  if (iMask & XML_PLUGINS)
    Save_Plugins_XML (ar);

  // last line in XML file
  ar.WriteString ("</muclient>" NL);
  }  // end of CMUSHclientDoc::Save_World_XML

void Save_XML_number (CArchive& ar, 
                      const char * sName, 
                      const long iNumber,
                      const bool sameline)
  {
  // default of zero does not need to be written
  if (iNumber)
    ar.WriteString (CFormat ("%s%s=\"%ld\"%s",
                    sameline ? "" : "   ",
                    sName, 
                    iNumber,
                    sameline ? " " : NL));
  } // end of Save_XML_number

void Save_XML_boolean (CArchive& ar, 
                       const char * sName, 
                       const long iNumber,
                       const bool sameline)
  {
  // default of false does not need to be written
  if (iNumber)
    ar.WriteString (CFormat ("%s%s=\"%s\"%s", 
                    sameline ? "" : "   ",
                    sName, 
                    iNumber != 0 ? "y" : "n",
                    sameline ? " " : NL));
  } // end of Save_XML_boolean

void Save_XML_string (CArchive& ar, 
                      const char * sName, 
                      const CString & str,
                      const bool sameline)
  {
  // shouldn't have newlines, get rid of them if there are any
  if (!str.IsEmpty ())
    ar.WriteString (CFormat ("%s%s=\"%s\"%s",
                   sameline ? "" : "   ",
                   sName, 
                   (LPCTSTR) FixHTMLString (Replace (str, ENDLINE, " ")),
                   sameline ? " " : NL));

  } // end of Save_XML_string

void Save_XML_date    (CArchive& ar, 
                       const char * sName, 
                       const CTime & date,
                       const bool sameline)
  {
  if (date.GetTime ())
    Save_XML_string (ar, 
                     sName, 
                     FixHTMLString (date.Format ("%Y-%m-%d %H:%M:%S")), 
                     sameline);
  } // end of Save_XML_date

void Save_XML_double    (CArchive& ar, 
                       const char * sName, 
                       const double & fNumber,
                       const bool sameline)
  {

  /*
  --- THIS DIDN'T WORK, see: http://www.gammon.com.au/forum/?id=7953&page=2

        // see: http://www.gammon.com.au/forum/?id=8077
      char old_locale[256];
      strncpy(old_locale, setlocale(LC_NUMERIC, NULL), sizeof old_locale); // store the locale
      old_locale [255] = 0;
      // want numbers formatted correctly
      setlocale (LC_NUMERIC, "C" );
      Save_XML_string (ar, 
                       sName, 
                       FixHTMLString (CFormat ("%.2f", fNumber)), 
                       sameline);
      setlocale(LC_NUMERIC, old_locale);         // restore the old locale

      */

  CString strNumber = CFormat ("%.2f", fNumber);
  strNumber.Replace (",", ".");    // don't want any blasted commas!

  Save_XML_string (ar, 
                   sName, 
                   FixHTMLString (strNumber), 
                   sameline);

  } // end of Save_XML_double

void Save_XML_colour  (CArchive& ar, 
                       const char * sName, 
                       const COLORREF & colour,
                       const bool sameline)
  {
  Save_XML_string (ar, sName, ColourToName (colour), sameline);
  } // end of Save_XML_colour


void Save_XML_multi (CArchive& ar, 
                     const char * sName, 
                     const CString & str)
  {
  if (!str.IsEmpty ())
    {

    // note - no spaces or newlines inside the tag, as they are preserved when
    // reading back in, and we don't want any extra ones

    // option name  - opening tag (eg. <notes> )
    ar.WriteString (CFormat ("  <%s>%s</%s>" NL, 
                    sName,
                    (LPCTSTR) FixHTMLMultilineString (str),
                    sName));

    } // end of not empty option

  } // end of Save_XML_multi

void CMUSHclientDoc::Save_World_Numeric_Options_XML (CArchive& ar)
  {
  for (long i = 0; OptionsTable [i].pName; i++)
    {
    long iValue = GetOptionItem (i);

    // skip included values that have not been changed
    if (m_NumericConfiguration [i]->bInclude &&
        iValue == m_NumericConfiguration [i]->iValue)
        continue;          

    // do booleans differently so they come out as "y" or "n"
    if (OptionsTable [i].iMinimum == 0 &&
        OptionsTable [i].iMaximum == 0)
      Save_XML_boolean (ar, OptionsTable [i].pName, iValue);
    else
      // RGB colours are different, also
      if (OptionsTable [i].iFlags & OPT_RGB_COLOUR)
        Save_XML_colour (ar, OptionsTable [i].pName, iValue);
      else
        Save_XML_number (ar, OptionsTable [i].pName, iValue);

    } // end of looping through each option

  } // end of CMUSHclientDoc::Save_World_Numeric_Options_XML

void CMUSHclientDoc::Save_World_Single_Line_Alpha_Options_XML (CArchive& ar)
  {

  for (int i = 0; AlphaOptionsTable [i].pName; i++)
    if (!(AlphaOptionsTable [i].iFlags & OPT_MULTLINE))
      {
      CString strValue =  GetAlphaOptionItem (i);

      // skip included values that have not been changed
      if (m_AlphaConfiguration [i]->bInclude &&
          strValue == m_AlphaConfiguration [i]->sValue)
          continue;          
      
      if ((AlphaOptionsTable [i].iFlags & OPT_PASSWORD) && !strValue.IsEmpty ())
        {
        strValue = encodeBase64 (strValue, false);  // base 64 encoding
        Save_XML_boolean (ar, 
                          CFormat ("%s_base64", (LPCTSTR) AlphaOptionsTable [i].pName), 
                          true);
        }
      Save_XML_string (ar, AlphaOptionsTable [i].pName, strValue);
      }    // end of looping through each option

  } // end of CMUSHclientDoc::Save_World_Single_Line_Alpha_Options_XML 

void CMUSHclientDoc::Save_World_Multi_Line_Alpha_Options_XML (CArchive& ar)
  {
  for (int i = 0; AlphaOptionsTable [i].pName; i++)
    if (AlphaOptionsTable [i].iFlags & OPT_MULTLINE)
      {
      CString strValue =  GetAlphaOptionItem (i);

      // skip included values that have not been changed
      if (m_AlphaConfiguration [i]->bInclude &&
          strValue == m_AlphaConfiguration [i]->sValue)
          continue;          

      Save_XML_multi (ar, AlphaOptionsTable [i].pName, strValue);
      }   // end of looping through each option

  } // end of CMUSHclientDoc::Save_World_Multi_Line_Alpha_Options_XML

void CMUSHclientDoc::Save_General_XML (CArchive& ar)
  {
  ar.WriteString ("<world" NL);

// identifying stuff to help us later on

  Save_Version_And_Date_XML (ar);

  // blank line to separate above stuff from actual world data
  ar.WriteString (NL);

  // first do the fixed attributes for the whole world

  // single-line alpha
  Save_World_Single_Line_Alpha_Options_XML (ar);

  // blank line to separate alpha from booleans :)
  ar.WriteString (NL);

  // numbers, booleans
  Save_World_Numeric_Options_XML (ar);

  ar.WriteString ("   > <!-- end of general world attributes -->" NL);    // end of world attributes

  // multi-line alpha
  Save_World_Multi_Line_Alpha_Options_XML (ar);

  ar.WriteString (NL "</world>" NL);

  } // end of CMUSHclientDoc::Save_General_XML

void CMUSHclientDoc::Save_Header_XML (CArchive& ar, 
                                      const char * sName,
                                      const bool bShowVersion)
  {

  if (bShowVersion)
    {
    ar.WriteString (CFormat (NL "<!-- %s -->" NL, sName));
    ar.WriteString (CFormat (NL "<%s" NL, sName));
    Save_Version_And_Date_XML (ar);
    ar.WriteString ("  >" NL);
    }
  else
    ar.WriteString (CFormat ("<%s>" NL, sName));

  } // end of CMUSHclientDoc::Save_Header_XML
 
void CMUSHclientDoc::Save_Footer_XML (CArchive& ar, const char * sName)
  {
  ar.WriteString (CFormat ("</%s>" NL, sName));

  } // end of CMUSHclientDoc::Save_Footer_XML

void CMUSHclientDoc::Save_Triggers_XML (CArchive& ar)
  {
  Save_Header_XML (ar, "triggers");

  // make absolutely sure we have all of them in the array
  SortTriggers ();

  // save them in trigger sequence order
  for (int i = 0; i < m_TriggerMap.GetCount (); i++)
     {
     CTrigger * t = m_TriggerArray [i];

     if (t->bTemporary || t->bIncluded)   // don't save temporary triggers
        continue;

     Save_One_Trigger_XML (ar, t);
     }    // end of loop

  Save_Footer_XML (ar, "triggers");

} // end of CMUSHclientDoc::Save_Triggers_XML

void CMUSHclientDoc::Save_One_Trigger_XML (CArchive& ar, CTrigger * t)
  {
  ar.WriteString ("  <trigger" NL);
  Save_XML_number  (ar, "back_colour",       (t->iMatch >> 8) & 0x0F);
  Save_XML_boolean (ar, "bold",              t->iMatch & HILITE);
  Save_XML_number  (ar, "clipboard_arg",     t->iClipboardArg);
  if (t->colour != SAMECOLOUR)   // don't write 65535 to world file
   Save_XML_number  (ar, "custom_colour",     t->colour + 1); // + 1, to match custom colours
  Save_XML_number  (ar, "colour_change_type",t->iColourChangeType);
  Save_XML_boolean (ar, "enabled",           t->bEnabled);
  Save_XML_boolean (ar, "expand_variables",  t->bExpandVariables);
  Save_XML_string  (ar, "group",             t->strGroup);
  Save_XML_boolean (ar, "ignore_case",       t->ignore_case);
  Save_XML_boolean (ar, "inverse",           t->iMatch & INVERSE);
  Save_XML_boolean (ar, "italic",            t->iMatch & BLINK);
  Save_XML_number  (ar, "lines_to_match",    t->iLinesToMatch);
  Save_XML_boolean (ar, "keep_evaluating",   t->bKeepEvaluating);
  Save_XML_boolean (ar, "make_bold",         t->iStyle & HILITE);
  Save_XML_boolean (ar, "make_italic",       t->iStyle & BLINK);
  Save_XML_boolean (ar, "make_underline",    t->iStyle & UNDERLINE);
  Save_XML_string  (ar, "match",             t->trigger);
  Save_XML_boolean (ar, "match_back_colour", t->iMatch & TRIGGER_MATCH_BACK);
  Save_XML_boolean (ar, "match_bold",        t->iMatch & TRIGGER_MATCH_HILITE);
  Save_XML_boolean (ar, "match_inverse",     t->iMatch & TRIGGER_MATCH_INVERSE);
  Save_XML_boolean (ar, "match_italic",      t->iMatch & TRIGGER_MATCH_BLINK);
  Save_XML_boolean (ar, "match_text_colour", t->iMatch & TRIGGER_MATCH_TEXT);
  Save_XML_boolean (ar, "match_underline",   t->iMatch & TRIGGER_MATCH_UNDERLINE);
  Save_XML_boolean (ar, "multi_line",        t->bMultiLine);
  Save_XML_string  (ar, "name",              t->strLabel);
  Save_XML_boolean (ar, "one_shot",          t->bOneShot);
  Save_XML_boolean (ar, "omit_from_log",     t->omit_from_log);
  Save_XML_boolean (ar, "omit_from_output",  t->bOmitFromOutput);
#ifdef PANE
  Save_XML_string  (ar, "pane",              t->strPane);
#endif // PANE
  Save_XML_boolean (ar, "regexp",            t->bRegexp);
  Save_XML_boolean (ar, "repeat",            t->bRepeat);
  Save_XML_string  (ar, "script",            t->strProcedure);
  Save_XML_number  (ar, "send_to",           t->iSendTo);
  Save_XML_number  (ar, "sequence",          t->iSequence);
  Save_XML_string  (ar, "sound",             t->sound_to_play);
  Save_XML_boolean (ar, "sound_if_inactive", t->bSoundIfInactive);
  Save_XML_boolean (ar, "lowercase_wildcard", t->bLowercaseWildcard);
  Save_XML_boolean (ar, "temporary",          t->bTemporary);
  Save_XML_number  (ar, "text_colour",       (t->iMatch >> 4) & 0x0F);
  Save_XML_number  (ar, "user",              t->iUserOption);
  Save_XML_string  (ar, "variable",          t->strVariable);

  // don't save black, that will be the default anyway
  if (t->iOtherForeground)
    Save_XML_colour  (ar, "other_text_colour", t->iOtherForeground);

  if (t->iOtherBackground)
    Save_XML_colour  (ar, "other_back_colour", t->iOtherBackground);

  ar.WriteString ("  >" NL);
  Save_XML_multi (ar, "send", t->contents);

  ar.WriteString ("  </trigger>" NL);

  } // end of CMUSHclientDoc::Save_One_Trigger_XML 

void CMUSHclientDoc::Save_Aliases_XML (CArchive& ar)
  {

  Save_Header_XML (ar, "aliases");

  // make absolutely sure we have all of them in the array
  SortAliases ();

  for (int i = 0; i < m_AliasMap.GetCount (); i++)
    {
    CAlias * a = m_AliasArray [i];

    if (a->bTemporary || a->bIncluded)   // don't save temporary aliases
      continue;

    Save_One_Alias_XML (ar, a);
    }

  Save_Footer_XML (ar, "aliases");

} // end of CMUSHclientDoc::Save_Aliases_XML

void CMUSHclientDoc::Save_One_Alias_XML (CArchive& ar, CAlias * a)
  {
  ar.WriteString ("  <alias" NL);

  Save_XML_string  (ar, "name",              a->strLabel);
  Save_XML_string  (ar, "script",            a->strProcedure);
  Save_XML_string  (ar, "match",             a->name);
  Save_XML_boolean (ar, "enabled",           a->bEnabled);

  Save_XML_boolean (ar, "echo_alias",        a->bEchoAlias);
  Save_XML_boolean (ar, "expand_variables",  a->bExpandVariables);
  Save_XML_string  (ar, "group",             a->strGroup);
  Save_XML_string  (ar, "variable",          a->strVariable);
  Save_XML_boolean (ar, "omit_from_command_history", a->bOmitFromCommandHistory);
  Save_XML_boolean (ar, "omit_from_log",     a->bOmitFromLog);
  Save_XML_boolean (ar, "regexp",            a->bRegexp);
  Save_XML_number  (ar, "send_to",           a->iSendTo);
  Save_XML_boolean (ar, "omit_from_output",  a->bOmitFromOutput);
  Save_XML_boolean (ar, "one_shot",          a->bOneShot);
  Save_XML_boolean (ar, "menu",              a->bMenu);
  Save_XML_boolean (ar, "ignore_case",       a->bIgnoreCase);
  Save_XML_boolean (ar, "keep_evaluating",   a->bKeepEvaluating);
  Save_XML_number  (ar, "sequence",          a->iSequence);
  Save_XML_boolean (ar, "temporary",         a->bTemporary);
  Save_XML_number  (ar, "user",              a->iUserOption);
  //     Save_XML_boolean (ar, "delayed",           t->bDelayed);

  ar.WriteString ("  >" NL);
  Save_XML_multi (ar, "send", a->contents);

  ar.WriteString ("  </alias>" NL);

  } // end of CMUSHclientDoc::Save_One_Alias_XML


void CMUSHclientDoc::Save_Variables_XML (CArchive& ar)
  {
  Save_Header_XML (ar, "variables");

  CVariableArray VariableArray; // temporary array, to get them in order

  // put all of them into the array, and sort them
  SortVariables (GetVariableMap (), VariableArray);

    // save them
  for (int i = 0; i < GetVariableMap ().GetCount (); i++)
    Save_One_Variable_XML (ar, VariableArray [i]);

  ar.WriteString ("</variables>" NL);

  } // end of CMUSHclientDoc::Save_Variables_XML

void CMUSHclientDoc::Save_One_Variable_XML (CArchive& ar, CVariable * v)
  {
  ar.WriteString (CFormat ("  <variable name=\"%s\">%s</variable>" NL,
                   (LPCTSTR) v->strLabel,
                   (LPCTSTR) FixHTMLMultilineString (v->strContents)));

  } // end of CMUSHclientDoc::Save_One_Variable_XML 

void CMUSHclientDoc::Save_Timers_XML (CArchive& ar)
  {
  Save_Header_XML (ar, "timers");

  for (POSITION pos = m_TimerMap.GetStartPosition(); pos; )
    {
    CString strTimerName;
    CTimer * t;

    m_TimerMap.GetNextAssoc (pos, strTimerName, t);

    if (t->bTemporary || t->bIncluded)   // don't save temporary timers
      continue;

    Save_One_Timer_XML (ar, t);

    }      // end of looping through each timer

  Save_Footer_XML (ar, "timers");

  } // end of CMUSHclientDoc::Save_Timers_XML


void CMUSHclientDoc::Save_One_Timer_XML (CArchive& ar, CTimer * t)
  {
  ar.WriteString ("  <timer ");

  Save_XML_string  (ar, "name",          t->strLabel, true);
  Save_XML_string  (ar, "script",        t->strProcedure, true);
  Save_XML_boolean (ar, "enabled",       t->bEnabled, true);

  Save_XML_number  (ar, "hour",    (t->iType == CTimer::eAtTime ? t->iAtHour   : t->iEveryHour), true);
  Save_XML_number  (ar, "minute",  (t->iType == CTimer::eAtTime ? t->iAtMinute : t->iEveryMinute), true);
  Save_XML_double  (ar, "second",  (t->iType == CTimer::eAtTime ? t->fAtSecond : t->fEverySecond), true);

  Save_XML_number  (ar, "offset_hour",       t->iOffsetHour, true);
  Save_XML_number  (ar, "offset_minute",     t->iOffsetMinute, true);
  Save_XML_double  (ar, "offset_second",     t->fOffsetSecond, true);
  Save_XML_number  (ar, "send_to",           t->iSendTo);
  Save_XML_boolean (ar, "temporary",         t->bTemporary);
  Save_XML_number  (ar, "user",              t->iUserOption);

  Save_XML_boolean (ar, "at_time",       t->iType, true);
  Save_XML_string  (ar, "group",         t->strGroup, true);
  Save_XML_string  (ar, "variable",      t->strVariable);
  Save_XML_boolean (ar, "one_shot",      t->bOneShot, true);
  Save_XML_boolean (ar, "omit_from_output",  t->bOmitFromOutput, true);
  Save_XML_boolean (ar, "omit_from_log",  t->bOmitFromLog, true);
  Save_XML_boolean (ar, "active_closed", t->bActiveWhenClosed, true);

  ar.WriteString (">" NL);
  Save_XML_multi (ar, "send", t->strContents);

  ar.WriteString (NL"  </timer>" NL);

  } // end of CMUSHclientDoc::Save_One_Timer_XML 

extern CString strMacroDescriptions [MACRO_COUNT];

void CMUSHclientDoc::Save_Macros_XML (CArchive& ar)
  {
  Save_Header_XML (ar, "macros");

  for (int i = 0; i < NUMITEMS (strMacroDescriptions); i++)
    {
    // ignore ones that don't send anything
    if (m_macros [i].IsEmpty ())
      continue;

    Save_One_Macro_XML (ar, i);
    }   // end of macros loop


  Save_Footer_XML (ar, "macros");

  } // end of CMUSHclientDoc::Save_Macros_XML


void CMUSHclientDoc::Save_One_Macro_XML (CArchive& ar, const int iIndex)
  {

  CString strType;

  switch (m_macro_type [iIndex])
    {
    case REPLACE_COMMAND: strType = "replace";
                          break;
    case SEND_NOW:        strType = "send_now";
                          break;
    case ADD_TO_COMMAND:  strType = "insert";
                          break;
    default:              strType = "unknown";
                          break;
    } // end of switch

   ar.WriteString (NL "  <macro ");

   Save_XML_string  (ar, "name",  strMacroDescriptions [iIndex], true);
   Save_XML_string  (ar, "type",  strType, true);
   ar.WriteString (">" NL);
   Save_XML_multi (ar, "send", m_macros [iIndex]);

   ar.WriteString (NL"  </macro>" NL);

  } // end of Save_One_Macro_XML 


void CMUSHclientDoc::Save_Colours_XML (CArchive& ar)
  {
int i;

  Save_Header_XML (ar, "colours");

// warning - we add 1 to sequences so they are 1-relative

  ar.WriteString (NL "<ansi>" NL);

  ar.WriteString (NL " <normal>" NL);
  for (i = 0; i < NUMITEMS (m_normalcolour); i++)
    {
    ar.WriteString ("   <colour ");
    Save_XML_number (ar, "seq", i + 1, true);
    Save_XML_colour (ar, "rgb", m_normalcolour [i], true);
    ar.WriteString ("/>" NL);
    }
  ar.WriteString (NL " </normal>" NL);

  ar.WriteString (NL " <bold>" NL);
  for (i = 0; i < NUMITEMS (m_boldcolour); i++)
    {
    ar.WriteString ("   <colour ");
    Save_XML_number (ar, "seq", i + 1, true);
    Save_XML_colour (ar, "rgb", m_boldcolour [i], true);
    ar.WriteString ("/>" NL);
    }
  ar.WriteString (NL " </bold>" NL);

  ar.WriteString (NL "</ansi>" NL);

  ar.WriteString (NL "<custom>" NL);
  for (i = 0; i < NUMITEMS (m_customtext); i++)
    {

    ar.WriteString ("  <colour ");
    Save_XML_number (ar, "seq", i + 1, true);
    Save_XML_string (ar, "name", m_strCustomColourName [i], true);
    Save_XML_colour (ar, "text", m_customtext [i], true);
    Save_XML_colour (ar, "back", m_customback [i], true);
    ar.WriteString ("/>" NL);

    }

  ar.WriteString (NL "</custom>" NL);

  Save_Footer_XML (ar, "colours");

  } // end of  CMUSHclientDoc::Save_Colours_XML 


extern CString strKeypadNames [eKeypad_Max_Items];

void CMUSHclientDoc::Save_One_Keypad_XML (CArchive& ar, const int iIndex)
  {
  ar.WriteString (NL "  <key ");
  Save_XML_string (ar, "name", strKeypadNames [iIndex], true);
  ar.WriteString (">" NL);
  Save_XML_multi (ar, "send", m_keypad [iIndex]);
  ar.WriteString ("  </key>" NL);

  }   // end of CMUSHclientDoc::Save_One_Keypad_XML

void CMUSHclientDoc::Save_Keypad_XML (CArchive& ar)
  {

  Save_Header_XML (ar, "keypad");

  for (int i = 0; i < eKeypad_Max_Items; i++)
      Save_One_Keypad_XML (ar, i);
  
  Save_Footer_XML (ar, "keypad");

  } // end of  CMUSHclientDoc::Save_Keypad_XML


void CMUSHclientDoc::Save_Printing_XML (CArchive& ar)
  {
int i;

  Save_Header_XML (ar, "printing");

// warning - we add 1 to sequences so they are 1-relative

  ar.WriteString (NL "<ansi>" NL);

  ar.WriteString (NL " <normal>" NL);
  for (i = 0; i < NUMITEMS (m_nNormalPrintStyle); i++)
    {

    // ignore ones that are normal
    if (m_nNormalPrintStyle [i] == FONT_NORMAL)
      continue;

    ar.WriteString ("   <style ");
    Save_XML_number (ar, "seq", i + 1, true);
    Save_XML_boolean (ar, "bold",  m_nNormalPrintStyle [i] & FONT_BOLD, true);
    Save_XML_boolean (ar, "italic",  m_nNormalPrintStyle [i] & FONT_ITALIC, true);
    Save_XML_boolean (ar, "underline",  m_nNormalPrintStyle [i] & FONT_UNDERLINE, true);
    ar.WriteString ("/>" NL);
    }
  ar.WriteString (NL " </normal>" NL);

  ar.WriteString (NL " <bold>" NL);
  for (i = 0; i < NUMITEMS (m_nBoldPrintStyle); i++)
    {

    // ignore ones that are normal
    if (m_nBoldPrintStyle [i] == FONT_NORMAL)
      continue;

    ar.WriteString ("   <style ");
    Save_XML_number (ar, "seq", i + 1, true);
    Save_XML_boolean (ar, "bold",  m_nBoldPrintStyle [i] & FONT_BOLD, true);
    Save_XML_boolean (ar, "italic",  m_nBoldPrintStyle [i] & FONT_ITALIC, true);
    Save_XML_boolean (ar, "underline",  m_nBoldPrintStyle [i] & FONT_UNDERLINE, true);
    ar.WriteString ("/>" NL);
    }
  ar.WriteString (NL " </bold>" NL);

  ar.WriteString (NL "</ansi>" NL);

  Save_Footer_XML (ar, "printing");

  } // end of  CMUSHclientDoc::Save_Printing_XML 


void CMUSHclientDoc::Save_Includes_XML (CArchive& ar)
  {
CString strFileName;

  for (POSITION pos = m_strIncludeFileList.GetHeadPosition (); pos; )
    {
    strFileName = m_strIncludeFileList.GetNext (pos);
    ar.WriteString   ("<include ");
    Save_XML_string  (ar, "name", strFileName, true);
    ar.WriteString   ("/>" NL);
    }

  } // end of CMUSHclientDoc::Save_Includes_XML

void CMUSHclientDoc::Save_Plugins_XML (CArchive& ar)
  {

  if (m_PluginList.GetCount () > 0)
    ar.WriteString   (NL "<!-- plugins -->" NL);

  for (POSITION pos = m_PluginList.GetHeadPosition (); pos; )
    {
    CPlugin * p = m_PluginList.GetNext (pos);

    if (!p->m_bGlobal)
      {
      CString strSource = p->m_strSource;

      CString strPluginsPath = Make_Absolute_Path (App.m_strPluginsDirectory);

      // strip absolute path if possible
      if (!strPluginsPath.IsEmpty ())
        if (strSource.Left (strPluginsPath.GetLength ()) ==
            strPluginsPath)
           strSource = strSource.Mid (strPluginsPath.GetLength ());

      ar.WriteString   ("<include ");
      Save_XML_string  (ar, "name", strSource, true);
      Save_XML_boolean (ar, "plugin", true, true);
      ar.WriteString   ("/>" NL);
      }  // end of local plugin
    }

  } // end of CMUSHclientDoc::Save_Plugins_XML


