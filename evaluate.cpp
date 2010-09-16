#include "stdafx.h"
#include "MUSHclient.h"
#include "doc.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


BOOL CMUSHclientDoc::EvaluateCommand (const CString & full_input, 
                                            const bool bCountThem,
                                            bool & bOmitFromLog,
                                            const bool bTest)
  {
CString str;
POSITION pos;
CString input = full_input;
CAliasList AliasList;
  
// get rid of any carriage returns

  input.Replace ("\r", "");
      
// ignore blank lines ?? is this wise?
        
  if (input.IsEmpty ())
    return false;

// ------------------------- SPEED WALKING ------------------------------

// see if they are doing speed walking

  if (m_enable_speed_walk && 
      input.Left (m_speed_walk_prefix.GetLength ()) == m_speed_walk_prefix)

    {
    CString strEvaluatedSpeedwalk = DoEvaluateSpeedwalk (input.Mid (m_speed_walk_prefix.GetLength ()));
    if (!strEvaluatedSpeedwalk.IsEmpty ())
      {
      if (strEvaluatedSpeedwalk [0] == '*')    // error in speedwalk string?
        {
        ::UMessageBox (strEvaluatedSpeedwalk.Mid (1));
        return true;
        }
      // let them know if they are foolishly trying to send to a closed connection
      if (CheckConnected ())
        return true;
      SendMsg (strEvaluatedSpeedwalk, m_display_my_input, 
              true,                // queue it
              LoggingInput ());    
      }
    return false;
    }
       // end of having a speed-walk string

// here if not speed walking


// --------------------------- ALIASES ------------------------------


  bool bEchoAlias = m_display_my_input;
  OneShotItemMap mapOneShotItems;

  if (m_enable_aliases)
    {
    m_CurrentPlugin = NULL;
    if (ProcessOneAliasSequence (input,
                             bCountThem,
                             bOmitFromLog,
                             bEchoAlias,
                             AliasList,
                             mapOneShotItems))
       return true;
    // do plugins
    for (pos = m_PluginList.GetHeadPosition (); pos; )
      {
      m_CurrentPlugin = m_PluginList.GetNext (pos);
      if (m_CurrentPlugin->m_bEnabled)
        if (ProcessOneAliasSequence (input,
                                 bCountThem,
                                 bOmitFromLog,
                                 bEchoAlias,
                                 AliasList,
                                 mapOneShotItems))
          {
          m_CurrentPlugin = NULL;
          return true;
          }
      } // end of doing each plugin

    m_CurrentPlugin = NULL; // not in a plugin any more

    } // end of aliases enabled


  // if no alias matched at all, just send the raw command

  if (AliasList.IsEmpty ())
    {
    // let them know if they are foolishly trying to send to a closed connection
    if (CheckConnected ())
      return true;

    // don't reconnect on a deliberate QUIT
    if (input.CompareNoCase (m_macros [MAC_QUIT]) == 0)
      m_bDisconnectOK = true;     // don't want reconnect on quit

    SendMsg (input, m_display_my_input, false, LoggingInput ());    // send now

    return FALSE;
    }

// execute any scripts associated with aliases we found

  bool bFoundIt;
  CAlias * existing_alias_item;
  CAlias * alias_item;
  CString strAliasName;

  for (pos = AliasList.GetHeadPosition (); pos; )
    {
    alias_item = AliasList.GetNext (pos);
    bFoundIt = false;

  // check that alias still exists, in case a script deleted it - and also
  // to work out which plugin it is in
    
    m_CurrentPlugin = NULL;

    // main aliases - if main scripting active
    for (POSITION pos = GetAliasMap ().GetStartPosition (); !bFoundIt && pos; )
      {
      GetAliasMap ().GetNextAssoc (pos, strAliasName, existing_alias_item);
      if (existing_alias_item == alias_item)
        {
        bFoundIt = true;
        // execute Alias script
        ExecuteAliasScript (alias_item, input);
        }

     }  // end of scanning main aliases

    // do plugins
    for (POSITION plugin_pos = m_PluginList.GetHeadPosition (); !bFoundIt && plugin_pos; )
      {
      m_CurrentPlugin = m_PluginList.GetNext (plugin_pos);

      if (m_CurrentPlugin->m_bEnabled)
        for (POSITION pos = GetAliasMap ().GetStartPosition (); !bFoundIt && pos; )
          {
          GetAliasMap ().GetNextAssoc (pos, strAliasName, existing_alias_item);
          if (existing_alias_item == alias_item)
            {
            bFoundIt = true;
            // execute Alias script
            ExecuteAliasScript (alias_item, input);
            }

          }  // end of scanning plugin aliases
      } // end of doing plugins list
    }       // end of list of aliass that fired



// now that we have run all scripts etc., delete one-shot aliases      

  int iDeletedCount = 0;
  int iDeletedNonTemporaryCount = 0;

  for (OneShotItemMap::const_iterator one_shot_it = mapOneShotItems.begin ();
       one_shot_it != mapOneShotItems.end ();
       one_shot_it++)
   {
    CAlias * alias_item;
    CString strAliasName = one_shot_it->sItemKey.c_str ();

    m_CurrentPlugin = one_shot_it->pWhichPlugin;   // set back to correct plugin

    if (!GetAliasMap ().Lookup (strAliasName, alias_item))
      continue;

    // can't if executing a script
    if (alias_item->bExecutingScript)
      continue;

    if (!m_CurrentPlugin && !alias_item->bTemporary)
      iDeletedNonTemporaryCount++;

    iDeletedCount++;

    // the alias seems to exist - delete its pointer
    delete alias_item;

    // now delete its entry
    GetAliasMap ().RemoveKey (strAliasName);

   }  // end of deleting one-shot items

   if (iDeletedCount > 0)
     {
     SortAliases ();

     if (iDeletedNonTemporaryCount > 0) // plugin mods don't really count
       SetModifiedFlag (TRUE);   // document has changed
     }

  m_CurrentPlugin = NULL;


  return FALSE;

  } // end of CMUSHclientDoc::EvaluateCommand 

// wildcard fixer

string FixWildcard (const string sWildcard,       // the wildcard
                     const bool bMakeLowerCase,   // true to make lower case
                     const int iSendTo,           // where it is going to
                     const CString strLanguage)   // what script language

  {
  string sResult = sWildcard;

  // force to lower-case if that is what they want
  if (bMakeLowerCase)
     sResult = tolower (sResult);

  // escape out strings if we are sending to script
  if (iSendTo == eSendToScript || iSendTo == eSendToScriptAfterOmit)
    {
    if (strLanguage.CompareNoCase ("vbscript") == 0)
      // " becomes ""
      sResult = FindAndReplace (sResult, "\"", "\"\"");
    else
      {  // not VBscript
      // escape out backslashes first (ie. \ becomes \\ )
      sResult = FindAndReplace (sResult, "\\", "\\\\");
      // now turn " to \"
      sResult = FindAndReplace (sResult, "\"", "\\\"");
      // finally better escape out the $ signs
      if (strLanguage.CompareNoCase ("perlscript") == 0)
        sResult = FindAndReplace (sResult, "$", "\\$");
      } // end of not VBscript
    } // end of sending to script

  return sResult;
  } // end of FixWildcard


CTrigger * CMUSHclientDoc::EvaluateTrigger (const CString & input, 
                                            CString & output,
                                            int & iItem,  // which one to start with
                                            int & iStartCol,
                                            int & iEndCol)
  {          
//  timer t ("EvaluateTrigger");

bool matched = false;
CTrigger * trigger_item = NULL;
int iCount = GetTriggerArray ().GetSize ();    // how many there are

  output.Empty ();

  // matching start/end col defaults to whole line
  iStartCol = 0;
  iEndCol = input.GetLength ();

// if triggers not enabled, return empty response

  if (!m_enable_triggers)
    return NULL;    // error return

  for ( ; iItem < iCount; iItem++)
    {
    trigger_item = GetTriggerArray () [iItem];

    if (!trigger_item->bEnabled)
      continue;   // ignore non-enabled triggers

    m_iTriggersEvaluatedCount++;  // count evaluations

    // do regular expression, if available
    if (trigger_item->regexp)
      {
      CString strTarget;
      
      if (trigger_item->bMultiLine)
        {
//        timer t ("Assembling text");
        string s;

        // can't do it if not enough lines received (hmm, maybe not)
//        if (m_sRecentLines.size () < trigger_item->iLinesToMatch)
//          continue;

        // assemble multi-line match text
        int iPos = m_sRecentLines.size () - trigger_item->iLinesToMatch;
        if (iPos < 0)
          iPos = 0;

        for (int iCount = 0; 
              iCount < trigger_item->iLinesToMatch &&
              iPos != m_sRecentLines.size ()
              ; iPos++, iCount++
            )
          {
          s += m_sRecentLines [iPos];
          s += '\n';  // multi-line triggers always end in newlines (new in version 3.50)
          } // end of assembling text
        strTarget = s.c_str ();
        }
      else
        strTarget = input;

  /*
  New feature in 3.18 - trigger match strings can incorporate variables in 
  the "trigger" portion. Do a quick scan to see if this is the case, and if
  so, recompile the regexp with substituted variables.

  Note, non-existent and empty variables will be silently dropped.

  */

      if (trigger_item->bExpandVariables &&
          trigger_item->trigger.Find ('@') != -1)
        {
        CString strOutput = FixSendText (trigger_item->trigger, 
                                        trigger_item->iSendTo,
                                        NULL,     // regexp
                                        GetLanguage (),
                                        false,    // lower-case wildcards
                                        true,     // expand variables
                                        false,    // expand wildcards
                                        true,     // convert regexps
                                        trigger_item->bRegexp,  // is it regexp or normal?
                                        false,         // don't throw exceptions
                                        NULL);    // no name substitution in match text


        LONGLONG iOldTimeTaken = 0;

        // remember time taken to execute them

        if (trigger_item->regexp)
          iOldTimeTaken = trigger_item->regexp->iTimeTaken;

        delete trigger_item->regexp;    // get rid of earlier regular expression
        trigger_item->regexp = NULL;

      // all triggers are now regular expressions

        CString strRegexp; 

        if (trigger_item->bRegexp)
          strRegexp = strOutput;
        else
          strRegexp = ConvertToRegularExpression (strOutput);

        try
          {
          trigger_item->regexp = regcomp (strRegexp,
                                          (trigger_item->ignore_case  ? PCRE_CASELESS : 0) |
                                          (trigger_item->bMultiLine  ? PCRE_MULTILINE : 0) |
                                          (m_bUTF_8 ? PCRE_UTF8 : 0)
                                         );
          } // end of try
    	  catch(CException* e)
          {
          e->ReportError ();
          e->Delete ();
          continue;
          }   // end of catch

        // add back execution time
        if (trigger_item->regexp)
          trigger_item->regexp->iTimeTaken += iOldTimeTaken;

        } // end of variable substitution

      try
        {
//        timer t ("Evaluating regular expression");
        if (!regexec (trigger_item->regexp, strTarget))
          continue;
        } // end of try
    	catch(CException* e)
        {
        e->ReportError ();
        e->Delete ();
        continue;
        }   // end of catch

      iStartCol = trigger_item->regexp->m_vOffsets [0];
      iEndCol   = trigger_item->regexp->m_vOffsets [1];

      trigger_item->wildcards.clear ();

      for (int iWildcard = 0; 
           iWildcard < MAX_WILDCARDS; 
           iWildcard++)
        trigger_item->wildcards.push_back 
                        (
                        FixWildcard (trigger_item->regexp->GetWildcard (iWildcard),
                                     trigger_item->bLowercaseWildcard,
                                     trigger_item->iSendTo,
                                     m_strLanguage)
                        );
          
      }
    else
      continue;   // no regexp, ignore trigger

    matched = true;

    trigger_item->tWhenMatched = CTime::GetCurrentTime(); // when it matched        

// copy contents to output area, replacing %1, %2 etc. with appropriate contents

    // get unlabelled trigger's internal name
    const char * pLabel = trigger_item->strLabel;
    if (pLabel [0] == 0)
       pLabel = GetTriggerRevMap () [trigger_item].c_str ();

    output += FixSendText (::FixupEscapeSequences (trigger_item->contents), 
                            trigger_item->iSendTo,    // where it is going
                            trigger_item->regexp,     // regexp
                            GetLanguage (),           // eg. vbscript
                            trigger_item->bLowercaseWildcard,    // lower-case wildcards
                            trigger_item->bExpandVariables,     // expand variables
                            true,      // expand wildcards
                            false,     // convert regexps
                            false,     // is it regexp or normal?
                            false,         // don't throw exceptions
                            pLabel);   

    break;   // break out of loop, we have a trigger match

    } // end of search each trigger item

  if (!matched)
    return NULL;

  return trigger_item;
  } // end of CMUSHclientDoc::EvaluateTrigger 


BOOL Set_Up_Set_Strings (const int set_type, 
                         CString & suggested_name,
                         CString & filter,
                         CString & title,
                         CString & suggested_extension)
  {

  switch (set_type)
    {
    case TRIGGER: suggested_extension = "mct"; 
                  filter = "MUSHclient triggers (*.mct)|*.mct||";
                  title = "Trigger file name";
                  suggested_name += " triggers";
                  break;   
    case ALIAS:   suggested_extension = "mca"; 
                  filter = "MUSHclient aliases (*.mca)|*.mca||";
                  title = "Alias file name";
                  suggested_name += " aliases";
                  break;   
    case COLOUR:  suggested_extension = "mcc"; 
                  filter = "MUSHclient colours (*.mcc)|*.mcc||";
                  title = "Colour file name";
                  suggested_name += " colours";
                  break;   
    case MACRO:   suggested_extension = "mcm"; 
                  filter = "MUSHclient macros (*.mcm)|*.mcm||";
                  title = "Macro file name";
                  suggested_name += " macros";
                  break;   
    case STRING:  suggested_extension = "mcs"; 
                  filter = "MUSHclient strings (*.mcs)|*.mcs||";
                  title = "Strings file name";
                  suggested_name += " strings";
                  break;   

    case TIMER:  suggested_extension = "mci"; 
                  filter = "MUSHclient timers (*.mci)|*.mci||";
                  title = "Timers file name";
                  suggested_name += " timers";
                  break;   

    default: return TRUE;   // error return
    } // end of switch

  return FALSE;   // OK return
                              
  } // end of CMUSHclientDoc::Set_Up_Set_Strings 

BOOL CMUSHclientDoc::Load_Set (const int set_type, 
                               CString strFileName,
                               CWnd * parent_window)
  {
BOOL replace = TRUE;

if (strFileName.IsEmpty ())
  {
  CString suggested_name = m_mush_name,
          filter,
          title,
          suggested_extension;

  CString filename;

    if (Set_Up_Set_Strings (set_type, 
                            suggested_name,
                            filter,
                            title,
                            suggested_extension))
        return TRUE;    // bad set_type
  

    CFileDialog filedlg (TRUE,   // loading the file
                         suggested_extension,    // default extension
                         "",  // suggested name
                         OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
                         filter,    // filter 
                         parent_window);  // parent window

    filedlg.m_ofn.lpstrTitle = title;
    filedlg.m_ofn.lpstrFile = filename.GetBuffer (_MAX_PATH); // needed!! (for Win32s)  
    if (App.platform == VER_PLATFORM_WIN32s)
      strcpy (filedlg.m_ofn.lpstrFile, "");
    else
      strcpy (filedlg.m_ofn.lpstrFile, suggested_name);

    ChangeToFileBrowsingDirectory ();
    int nResult = filedlg.DoModal();
    ChangeToStartupDirectory ();

    if (nResult!= IDOK)
      return TRUE;    // cancelled dialog

  // since they can have any number of triggers, aliases and timers, ask them
  // whether they want to add this file to an existing list (if any)

    if (set_type == TRIGGER && !m_TriggerMap.IsEmpty ())
      {
        if (::TMessageBox ("Replace existing triggers?\n"
                            "If you reply \"No\", then triggers from the file"
                            " will be added to existing triggers",
                            MB_YESNO | MB_ICONQUESTION) == IDNO)
          replace = FALSE;
      }
    else
    if (set_type == ALIAS && !m_AliasMap.IsEmpty ())
      {
        if (::TMessageBox ("Replace existing aliases?\n"
                            "If you reply \"No\", then aliases from the file"
                            " will be added to existing aliases",
                            MB_YESNO | MB_ICONQUESTION) == IDNO)
          replace = FALSE;
      }
    else
    if (set_type == TIMER && !m_TimerMap.IsEmpty ())
      {
        if (::TMessageBox ("Replace existing timers?\n"
                            "If you reply \"No\", then timers from the file"
                            " will be added to existing timers",
                            MB_YESNO | MB_ICONQUESTION) == IDNO)
          replace = FALSE;
      }

    strFileName = filedlg.GetPathName ();
  }   // end of no filename suppliedl

CFile * f = NULL;
CArchive * ar = NULL;

  try
    {
    f = new CFile (strFileName, CFile::modeRead | CFile::shareDenyWrite);

    ar = new CArchive(f, CArchive::load);

    if (IsArchiveXML (*ar))
      {

      switch (set_type)
        {
        case TRIGGER: 
          if (replace)
            DELETE_MAP (m_TriggerMap, CTrigger);
          Load_World_XML (*ar, XML_TRIGGERS | XML_NO_PLUGINS);  
          break;  

        case ALIAS:   
          if (replace)
            DELETE_MAP (m_AliasMap, CAlias);
          Load_World_XML (*ar, XML_ALIASES | XML_NO_PLUGINS);  
          break;  

        case COLOUR:  
          Load_World_XML (*ar, XML_COLOURS | XML_NO_PLUGINS);  
          break;  
        
        case MACRO:   
          Load_World_XML (*ar, XML_MACROS | XML_NO_PLUGINS);  
          break;   

        case TIMER:   
          if (replace)
            DELETE_MAP (m_TimerMap, CTimer);
          Load_World_XML (*ar, XML_TIMERS | XML_NO_PLUGINS);  
          break;  

        } // end of switch

      }  // end of XML load
    else
      {
      ::TMessageBox ("File does not have a valid MUSHclient XML signature.",
                         MB_ICONSTOP);
        AfxThrowArchiveException (CArchiveException::badSchema);
      } // end of not XML

    } // end of try block

  // even on an exception we will return a "good" status, because the triggers etc.
  // may well have been deleted by now, so we need to redraw the lists

  catch (CFileException * e)
    {
    ::UMessageBox (TFormat ("Unable to open or read %s",
                      (LPCTSTR) strFileName), MB_ICONEXCLAMATION);
    e->Delete ();
    } // end of catching a file exception

  catch (CMemoryException * e)
    {
    ::TMessageBox ("Insufficient memory to do this operation", MB_ICONEXCLAMATION);
    e->Delete ();
    } // end of catching a memory exception

  catch (CArchiveException * e)
    {
    ::UMessageBox (TFormat ("The file %s is not in the correct format", 
                      (LPCTSTR) strFileName), MB_ICONEXCLAMATION);
    e->Delete ();
    } // end of catching an archive exception

  delete ar;      // delete archive
  delete f;       // delete file

  SetModifiedFlag (TRUE);   // document has now changed
  return false;   // OK return

  } // end of CMUSHclientDoc::load_set
    
BOOL CMUSHclientDoc::Save_Set (const int set_type, 
                               CWnd * parent_window)
  {
CString suggested_name = m_mush_name,
        filter,
        title,
        suggested_extension;

CFile * f = NULL;
CArchive * ar = NULL;
BOOL error = TRUE;

CString sig;
CString filename;

  if (Set_Up_Set_Strings (set_type, 
                          suggested_name,
                          filter,
                          title,
                          suggested_extension))
      return TRUE;    // bad set_type

  CFileDialog filedlg (FALSE,   // saving the file
                       suggested_extension,    // default extension
                       "",  // suggested name
                       OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                       filter,    // filter 
                       parent_window);  // parent window


  // fix up name to remove characters that are invalid

  int i;
  while ((i = suggested_name.FindOneOf ("<>\"|?:#%;/\\")) != -1)
    suggested_name = suggested_name.Left (i) + suggested_name.Mid (i + 1);

  filedlg.m_ofn.lpstrTitle = title;
  filedlg.m_ofn.lpstrFile = filename.GetBuffer (_MAX_PATH); // needed!! (for Win32s)  
  if (App.platform == VER_PLATFORM_WIN32s)
    strcpy (filedlg.m_ofn.lpstrFile, "");
  else
    strcpy (filedlg.m_ofn.lpstrFile, suggested_name);

  ChangeToFileBrowsingDirectory ();
  int nResult = filedlg.DoModal();
  ChangeToStartupDirectory ();

  if (nResult != IDOK)
    return TRUE;    // cancelled dialog

  CPlugin * pSavedPlugin = m_CurrentPlugin;
  m_CurrentPlugin = NULL;   // make sure we save main triggers etc.

  try
    {
    f = new CFile (filedlg.GetPathName (), 
                    CFile::modeCreate | CFile::modeReadWrite);

    ar = new CArchive(f, CArchive::store);


    switch (set_type)
      {
      case TRIGGER:   Save_World_XML (*ar, XML_TRIGGERS); break;
      case ALIAS:     Save_World_XML (*ar, XML_ALIASES); break;
      case COLOUR:    Save_World_XML (*ar, XML_COLOURS); break;
      case MACRO:     Save_World_XML (*ar, XML_MACROS); break;
      case TIMER:     Save_World_XML (*ar, XML_TIMERS); break;

      } // end of switch
    
    error = FALSE;
    } // end of try block

  catch (CFileException * e)
    {
    ::TMessageBox ("Unable to create the requested file", MB_ICONEXCLAMATION);
    e->Delete ();
    } // end of catching a file exception

  catch (CMemoryException * e)
    {
    ::TMessageBox ("Insufficient memory to do this operation", MB_ICONEXCLAMATION);
    e->Delete ();
    } // end of catching a memory exception

  catch (CArchiveException * e)
    {
    ::TMessageBox ("There was a problem in the data format", MB_ICONEXCLAMATION);
    e->Delete ();
    } // end of catching an archive exception

  m_CurrentPlugin = pSavedPlugin;

  delete ar;      // delete archive
  delete f;       // delete file
  return error;   // OK return

  } // end of CMUSHclientDoc::save_set



bool CMUSHclientDoc::ProcessOneAliasSequence (const CString strCurrentLine,
                            const bool bCountThem,
                            bool & bOmitFromLog,
                            bool & bEchoAlias,
                            CAliasList & AliasList,
                            OneShotItemMap & mapOneShotItems)
  {

  for (int iAlias = 0; iAlias < GetAliasArray ().GetSize (); iAlias++)
    {
    CAlias * alias_item = GetAliasArray () [iAlias];

  // ignore non-enabled aliases

    if (!alias_item->bEnabled)
      continue;

    m_iAliasesEvaluatedCount++;

    BOOL bMatched;

    // empty wildcards now
    for (int i = 0; i < MAX_WILDCARDS; i++)
      alias_item->wildcards [i] = "";

    CString strTarget = strCurrentLine;
  
    try
      {
      bMatched = regexec (alias_item->regexp, strTarget);
      }
    catch(CException* e)
      {
      e->ReportError ();
      e->Delete ();
      bMatched = false;
      }

    if (!bMatched) // no match, try next one
      continue;   


    m_iAliasesMatchedCount++;
    m_iAliasesMatchedThisSessionCount++;

    if (alias_item->bOneShot)
      mapOneShotItems.push_back (
          OneShotItem (m_CurrentPlugin, 
                      (const char *) alias_item->strInternalName));

    // if alias wants it, omit entire typed line from command history
    if (alias_item->bOmitFromCommandHistory)
      m_bOmitFromCommandHistory = true;

    alias_item->wildcards.clear ();

    for (int iWildcard = 0; 
         iWildcard < MAX_WILDCARDS; 
         iWildcard++)
      alias_item->wildcards.push_back 
                      (
                      FixWildcard (alias_item->regexp->GetWildcard (iWildcard),
                                   false,
                                   alias_item->iSendTo,
                                   m_strLanguage)
                      );

  // echo the alias they typed, unless command echo off, or previously displayed
      // (if wanted - v3.38)

    if (bEchoAlias &&     // not already done
        alias_item->bEchoAlias)  // alias wants to be echoed
      {
      DisplayMsg (strCurrentLine + ENDLINE, 
                  strCurrentLine.GetLength () + strlen (ENDLINE), 
                  USER_INPUT | (LoggingInput () ? LOG_LINE : 0));
      bEchoAlias = false;   // don't echo the same line twice
      // and log the command the actually typed
      if (LoggingInput ())
        LogCommand (strCurrentLine);
      }

    if (bCountThem)
      alias_item->nMatched++;   // count alias matches
    
    bOmitFromLog = alias_item->bOmitFromLog;

    alias_item->tWhenMatched = CTime::GetCurrentTime(); // when it matched        

    if (alias_item->strLabel.IsEmpty ())
      Trace ("Matched alias \"%s\"", (LPCTSTR) alias_item->name);
    else
      Trace ("Matched alias %s", (LPCTSTR) alias_item->strLabel);
  
    // get unlabelled alias's internal name
    const char * pLabel = alias_item->strLabel;
    if (pLabel [0] == 0)
       pLabel = GetAliasRevMap () [alias_item].c_str ();

    // if we have to do parameter substitution on the alias, do it now

    CString strSendText;

    // copy contents to strSendText area, replacing %1, %2 etc. with appropriate contents

    try
      {
      strSendText = FixSendText (::FixupEscapeSequences (alias_item->contents), 
                              alias_item->iSendTo,    // where it is going
                              alias_item->regexp,     // regexp
                              GetLanguage (),           // eg. vbscript
                              false,    // lower-case wildcards
                              alias_item->bExpandVariables,     // expand variables
                              true,      // expand wildcards
                              false,     // convert regexps
                              false,     // is it regexp or normal?
                              true,         // throw exceptions
                              pLabel);   
      }
	  catch (CException* e)
	    {
		  e->ReportError();
		  e->Delete();
      return true;
	    }	

    AliasList.AddTail (alias_item);   // add to list of aliases

    CString strExtraOutput;

    // let them know if they are foolishly trying to send to a closed connection
    // - only applies to commands that actually send to the world
    if (!strSendText.IsEmpty ())
      switch (alias_item->iSendTo)
        {
        case eSendToWorld:
        case eSendToCommandQueue:
        case eSendToSpeedwalk:
        case eSendImmediate:
          if (CheckConnected ())
            return true;
          break;
        }

    alias_item->bExecutingScript = true;     // cannot be deleted now
    SendTo (alias_item->iSendTo, 
            strSendText, 
            alias_item->bOmitFromOutput,
            alias_item->bOmitFromLog,
            TFormat ("Alias: %s", (LPCTSTR) alias_item->strLabel),
            alias_item->strVariable,
            strExtraOutput);
    alias_item->bExecutingScript = false;     // can be deleted now

    // display any stuff sent to output window

    if (!strExtraOutput.IsEmpty ())
       DisplayMsg (strExtraOutput, strExtraOutput.GetLength (), COMMENT);

   // only re-match if they want multiple matches

   if (!alias_item->bKeepEvaluating)
     break;
   } // end of looping, checking each alias

  return FALSE;
  } // end of CMUSHclientDoc::ProcessOneAliasSequence 


bool CMUSHclientDoc::ExecuteAliasScript (CAlias * alias_item,
                             const CString strCurrentLine)
  {

  if (CheckScriptingAvailable ("Alias", alias_item->dispid, alias_item->strProcedure))
     return false;

  if (alias_item->dispid != DISPID_UNKNOWN)        // if we have a dispatch id
    {

    CString strType = "alias";
    CString strReason =  TFormat ("processing alias \"%s\"", 
                                  (LPCTSTR) alias_item->strLabel);

    // get unlabelled alias's internal name
    const char * pLabel = alias_item->strLabel;
    if (pLabel [0] == 0)
       pLabel = GetAliasRevMap () [alias_item].c_str ();

    if (GetScriptEngine () && GetScriptEngine ()->IsLua ())
      {
      list<double> nparams;
      list<string> sparams;
      sparams.push_back (pLabel);
      sparams.push_back ((LPCTSTR) strCurrentLine);
      alias_item->bExecutingScript = true;     // cannot be deleted now
      GetScriptEngine ()->ExecuteLua (alias_item->dispid, 
                                     alias_item->strProcedure, 
                                     eDontChangeAction,
                                     strType, 
                                     strReason, 
                                     nparams,
                                     sparams, 
                                     alias_item->nInvocationCount,
                                     alias_item->regexp); 
      alias_item->bExecutingScript = false;     // can be deleted now
      return true;
      }   // end of Lua

    // prepare for the arguments, so far, 3 which are: 
    //  1. alias name, 
    //  2. expanded line 
    //  3. replacement string

    // WARNING - arguments should appear in REVERSE order to what the sub expects them!

    enum
      {
      eWildcards,
      eInputLine,
      eAliasName,
      eArgCount,     // this MUST be last
      };    

    COleSafeArray sa;   // for wildcard list
    COleVariant args [eArgCount];
    DISPPARAMS params = { args, NULL, eArgCount, 0 };

    args [eAliasName] = pLabel;
    args [eInputLine] = strCurrentLine;

    // --------------- set up wildcards array ---------------------------
    sa.Clear ();
    // nb - to be consistent with %1, %2 etc. we will make array 1-relative
    sa.CreateOneDim (VT_VARIANT, MAX_WILDCARDS, NULL, 1);
    long i;
    for (i = 1; i < MAX_WILDCARDS; i++)
      {
      COleVariant v (alias_item->wildcards [i].c_str ());
      sa.PutElement (&i, &v);
      }
    // i should be MAX_WILDCARDS (10) now ;)
    COleVariant v (alias_item->wildcards [0].c_str ()); // the whole matching line
    sa.PutElement (&i, &v);
    args [eWildcards] = sa;
    
    alias_item->bExecutingScript = true;     // cannot be deleted now
    ExecuteScript (alias_item->dispid,  
                   alias_item->strProcedure,
                   eDontChangeAction,     // don't change current action
                   strType, 
                   strReason,
                   params, 
                   alias_item->nInvocationCount); 
    alias_item->bExecutingScript = false;     // can be deleted now

    return true;
    }     // end of having a dispatch ID

  return false;
  } // end of CMUSHclientDoc::ExecuteAliasScript

// ===============================================================================
// New in version 3.51 - generalised "send box" fixer-upper
//  does variables, wildcards (if applicable)
//  -- also does the trigger match text for triggers (set bFixRegexps) by doing
//     variable substitution

// for triggers, timers, aliases - replace a string with imbedded variables

// eg.  @bar :   contents of variable bar
//      @!bar :  contents of variable bar with no fixups of imbedded '|' symbols etc.
//      %1 :     contents of wildcard 1 
//      %<foo> : contents of named wildcard "foo"
//      %<22> :  contents of wildcard 22
//      %% :     %
//      %C :     contents of (text) clipboard
//      %N :     name of trigger / timer / alias

// If bThrowExceptions is set certain conditions (such as variable not existing)
// cause an exception to be thrown (eg. for alias handling), 
// ===============================================================================


CString CMUSHclientDoc::FixSendText (const CString strSource, 
                                     const int iSendTo,
                                     const t_regexp * regexp,    // regular expression (for triggers, aliases)
                                     const char * sLanguage,     // language for send-to-script
                                     const bool bMakeWildcardsLower,
                                     const bool bExpandVariables,   // convert @foo
                                     const bool bExpandWildcards,   // convert %x
                                     const bool bFixRegexps, // convert \ to \\ for instance
                                     const bool bIsRegexp,   // true = regexp trigger
                                     const bool bThrowExceptions,   // throw exception on error
                                     const char * sName)            // the name of the trigger/timer/alias (for %N)
  {
CString strOutput;   // result of expansion

const char * pText,
           * pStartOfGroup;

  pText = pStartOfGroup = strSource;

  while (*pText)
    {
    switch (*pText)
      {

/* -------------------------------------------------------------------- *
 *  Variable expansion - @foo becomes <contents of foo>                 *
 * -------------------------------------------------------------------- */

      case '@':
        {
        if (!bExpandVariables)
          {
          pText++;      // just copy the @
          break;
          }


       // copy up to the @ sign

        strOutput += CString (pStartOfGroup, pText - pStartOfGroup);
      
        pText++;    // skip the @

        // @@ becomes @
        if (*pText == '@')
          {
          pStartOfGroup = ++pText;
          strOutput += "@";
          continue;
          }

        const char * pName;
        bool bEscape = bFixRegexps;

        // syntax @!variable defeats the escaping

        if (*pText == '!')
          {
          pText++;
          bEscape = false;
          }

        pName = pText;

        // find end of variable name
        while (*pText)
          if (*pText == '_' || isalnum ((unsigned char) *pText))
            pText++;
          else
            break;

/* -------------------------------------------------------------------- *
 *  We have a variable - look it up and do internal replacements        *
 * -------------------------------------------------------------------- */
          
        CString strVariableName (pName, pText - pName);

        if (strVariableName.IsEmpty ())
          {
          if (bThrowExceptions)
            ThrowErrorException ("@ must be followed by a variable name");
          } // end of no variable name
        else
          {
          CVariable * variable_item;

          strVariableName.MakeLower ();

          if (GetVariableMap ().Lookup (strVariableName, variable_item))
            {
            // fix up so regexps don't get confused with [ etc. inside variable
            CString strVariableContents;
            if (bEscape)
              {
              const char * pi;
              // allow for doubling in size, plus terminating null
              char * po = strVariableContents.GetBuffer ((variable_item->strContents.GetLength () * 2) + 1);
              for (pi = variable_item->strContents;
                  *pi;
                  pi++)
                {
                if (((unsigned char) *pi) < ' ')
                  continue;   // drop non-printables
                if (bIsRegexp)
                  if (!isalnum ((unsigned char) *pi) && *pi != ' ')
                    {
                    *po++ = '\\'; // escape it
                    *po++ = *pi;
                    }
                  else
                    *po++ = *pi;    // just copy it
                else          // not regexp
                  if (*pi != '*')
                    *po++ = *pi;    // copy all except asterisks
                }

              *po = 0;  // terminating null
              strVariableContents.ReleaseBuffer (-1); 
              }  // end of escaping wanted
            else
              strVariableContents = variable_item->strContents;

            // in the "send" box may need to convert script expansions etc.
            if (!bFixRegexps)
               strVariableContents = FixWildcard ((const char *) strVariableContents,
                                                 false, // not force variables to lowercase
                                                 iSendTo,
                                                 sLanguage).c_str ();

            // fix up HTML sequences if we are sending it to a log file
            if (m_bLogHTML && iSendTo == eSendToLogFile)
              strVariableContents = FixHTMLString (strVariableContents);

            strOutput += strVariableContents;
            }   // end of name existing in variable map
          else
            {
            if (bThrowExceptions)
              ThrowErrorException ("Variable '%s' is not defined.", 
                                  (LPCTSTR) strVariableName);
            }  // end of variable does not exist

          } // end of not empty name

          // get ready for next batch from beyond the variable
          pStartOfGroup = pText;
         }
        break;    // end of '@'

/* -------------------------------------------------------------------- *
 *  Wildcard substitution - %1 becomes <contents of wildcard 1>         *
 *                        - %<foo> becomes <contents of wildcard "foo"  *
 *                        - %% becomes %                                *
 * -------------------------------------------------------------------- */

      case '%':

           if (!bExpandWildcards)
            {
            pText++;      // just copy the %
            break;
            }

        // see what comes after the % symbol
        switch (pText [1])
           {

/* -------------------------------------------------------------------- *
 *  %%                                                                  *
 * -------------------------------------------------------------------- */

           case '%':

          // copy up to - and including - the percent sign

            strOutput += CString (pStartOfGroup, pText - pStartOfGroup + 1);

            // get ready for next batch from beyond the %%

            pText += 2;    // don't reprocess the %%
            pStartOfGroup = pText;  
            break;   // end of %%

/* -------------------------------------------------------------------- *
 *  %0 to %9                                                            *
 * -------------------------------------------------------------------- */
          
           case '0':       // a digit?
           case '1':
           case '2':
           case '3':
           case '4':
           case '5':
           case '6':
           case '7':
           case '8':
           case '9':
             {

            // copy up to the percent sign

              strOutput += CString (pStartOfGroup, pText - pStartOfGroup);
                    
            // output the appropriate replacement text

            if (regexp)
              {
              CString strWildcard = FixWildcard (regexp->GetWildcard (string (1, *++pText)),
                                                 bMakeWildcardsLower,
                                                 iSendTo,
                                                 sLanguage).c_str ();

              // fix up HTML sequences if we are sending it to a log file
              if (m_bLogHTML && iSendTo == eSendToLogFile)
                strWildcard = FixHTMLString (strWildcard);

              strOutput += strWildcard;
              }

            // get ready for next batch from beyond the digit

             pStartOfGroup = ++pText;


             } 
             break;   // end of %(digit) (eg. %2)

/* -------------------------------------------------------------------- *
 *  %<name>                                                             *
 * -------------------------------------------------------------------- */

           case '<':
             {
            // copy up to the % sign

              strOutput += CString (pStartOfGroup, pText - pStartOfGroup);
    
              pText +=2;    // skip the %<

              const char * pName = pText;

              // find end of wildcard name
              while (*pText)
                if (*pText != '>')
                  pText++;
                else
                  break;
  
              string sWildcardName (pName, pText - pName);

              if (!sWildcardName.empty () && regexp)
                {
                CString strWildcard = FixWildcard (regexp->GetWildcard (sWildcardName),
                                                   bMakeWildcardsLower,
                                                   iSendTo,
                                                   sLanguage).c_str ();

                // fix up HTML sequences if we are sending it to a log file
                if (m_bLogHTML && iSendTo == eSendToLogFile)
                  strWildcard = FixHTMLString (strWildcard);

                strOutput += strWildcard;

                }
              // get ready for next batch from beyond the name

              if (*pText == '>')
                pText++;
              pStartOfGroup = pText;
             }
             break;   // end of %<foo>

/* -------------------------------------------------------------------- *
 *  %C - clipboard contents                                             *
 * -------------------------------------------------------------------- */

           case 'C':
           case 'c':
             {
            // copy up to the percent sign
              strOutput += CString (pStartOfGroup, pText - pStartOfGroup);

              CString strClipboard;

              if (GetClipboardContents (strClipboard, m_bUTF_8))
                strOutput += strClipboard;
              else
                if (bThrowExceptions)
                  ThrowErrorException ("No text on the Clipboard");

        // get ready for next batch from beyond the 'c'

              pText += 2;
              pStartOfGroup = pText;
             }
             break; // end of %c

/* -------------------------------------------------------------------- *
 *  %N - name of the thing                                              *
 * -------------------------------------------------------------------- */

           case 'N':
           case 'n':
             {
            // copy up to the percent sign
              strOutput += CString (pStartOfGroup, pText - pStartOfGroup);

              if (sName)
                strOutput += sName;

        // get ready for next batch from beyond the 'n'

              pText += 2;
              pStartOfGroup = pText;
             }
             break; // end of %n

/* -------------------------------------------------------------------- *
 *  %(something else)                                                   *
 * -------------------------------------------------------------------- */

           default:
              pText++;
              break;

           }  // end of switch on character after '%'

         break;    // end of '%(something)'

/* -------------------------------------------------------------------- *
 *  All other characters - just increment pointer so we can copy later  *
 * -------------------------------------------------------------------- */

      default:
        pText++;
        break;

      } // end of switch on *pText

    }   // end of not end of string yet

// copy last group

  strOutput += pStartOfGroup;

  return strOutput;
  } // end of  CMUSHclientDoc::FixSendText


#ifdef PANE

void CMUSHclientDoc::SendToPane (const CString strSource,      // what to send (eg. %1 = %2)
                                 const CPaneLine & StyledLine, // the styled text to be sent
                                 const string sPaneName,       // which pane
                                 const t_regexp * regexp,      // regular expression (for triggers, aliases)
                                 const bool bMakeWildcardsLower, // force wildcards to lower-case?
                                 const bool bExpandVariables)  // expand variables (eg. @foo)
  {



  } // end of CMUSHclientDoc::SendToPane

#endif
