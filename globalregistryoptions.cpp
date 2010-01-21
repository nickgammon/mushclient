#include "stdafx.h"
#include "MUSHclient.h"
#include "doc.h"


static CMUSHclientApp * pApp = NULL;  // purely for #define below


#define GLB_OPT(arg) offsetof (CMUSHclientApp, arg)

// ints - WARNING - we assume these are ints, the type-casting will defeat
// warnings if they are not.

static tGlobalConfigurationNumericOption GlobalOptionsTable [] = {

// option in MUSHclient.h                       name in registry          default
{ GLB_OPT (m_bAllTypingToCommandWindow      ), "AllTypingToCommandWindow",  1 },
{ GLB_OPT (m_bAlwaysOnTop                   ), "AlwaysOnTop", 0 },
{ GLB_OPT (m_bAppendToLogFiles              ), "AppendToLogFiles",              0 },
{ GLB_OPT (m_bAutoConnectWorlds             ), "AutoConnectWorlds",             1 },
{ GLB_OPT (m_bAutoExpand                    ), "AutoExpandConfig",  1 },
{ GLB_OPT (m_bFlatToolbars                  ), "FlatToolbars",  1 },
{ GLB_OPT (m_bAutoLogWorld                  ), "AutoLogWorld",                  0 },
{ GLB_OPT (m_bBleedBackground               ), "BleedBackground",  0 },
{ GLB_OPT (m_bColourGradient                ), "ColourGradientConfig",  1 },
{ GLB_OPT (m_bConfirmBeforeClosingMXPdebug  ), "ConfirmBeforeClosingMXPdebug",  0 },
{ GLB_OPT (m_bConfirmBeforeClosingMushclient), "ConfirmBeforeClosingMushclient",1 },
{ GLB_OPT (m_bConfirmBeforeClosingWorld     ), "ConfirmBeforeClosingWorld",     1 },
{ GLB_OPT (m_bConfirmBeforeSavingVariables  ), "ConfirmBeforeSavingVariables",  1 },
{ GLB_OPT (m_bConfirmLogFileClose           ), "ConfirmLogFileClose",           1 },
{ GLB_OPT (m_bEnableSpellCheck              ), "EnableSpellCheck", 1 },
{ GLB_OPT (m_bEnablePackageLibrary          ), "AllowLoadingDlls", 0 },
{ GLB_OPT (m_bF1macro                       ), "F1macro",  0 },
{ GLB_OPT (m_bFixedFontForEditing           ), "FixedFontForEditing",  1 },
{ GLB_OPT (m_bNotepadWordWrap               ), "NotepadWordWrap", 1 }, 
{ GLB_OPT (m_bNotifyIfCannotConnect         ), "NotifyIfCannotConnect",         1 },
{ GLB_OPT (m_bErrorNotificationToOutputWindow), "ErrorNotificationToOutputWindow",         0 },
{ GLB_OPT (m_bNotifyOnDisconnect            ), "NotifyOnDisconnect",            1 },
{ GLB_OPT (m_bOpenActivityWindow            ), "OpenActivityWindow",            0 },
{ GLB_OPT (m_bOpenWorldsMaximised           ), "OpenWorldsMaximised",           0 },
{ GLB_OPT (m_iWindowTabsStyle               ), "WindowTabsStyle",           0 },
{ GLB_OPT (m_bReconnectOnLinkFailure        ), "ReconnectOnLinkFailure",        0 },
{ GLB_OPT (m_bRegexpMatchEmpty              ), "RegexpMatchEmpty",  1 },
{ GLB_OPT (m_bShowGridLinesInListViews      ), "ShowGridLinesInListViews",  1 },
{ GLB_OPT (m_bSmoothScrolling               ), "SmoothScrolling",  0 },
{ GLB_OPT (m_bSmootherScrolling             ), "SmootherScrolling",  0 },
{ GLB_OPT (m_bDisableKeyboardMenuActivation ), "DisableKeyboardMenuActivation",  0 },
{ GLB_OPT (m_bTriggerRemoveCheck            ), "TriggerRemoveCheck",  1 },
{ GLB_OPT (m_cNotepadBackColour             ), "NotepadBackColour", 0 }, 
{ GLB_OPT (m_cNotepadTextColour             ), "NotepadTextColour", 0 }, 
{ GLB_OPT (m_iActivityButtonBarStyle        ), "ActivityButtonBarStyle", 0 },                              
{ GLB_OPT (m_iAsciiArtLayout                ), "AsciiArtLayout", 0 },                              
{ GLB_OPT (m_iDefaultInputFontHeight        ), "DefaultInputFontHeight",            9 },            
{ GLB_OPT (m_iDefaultInputFontItalic        ), "DefaultInputFontItalic ", 0 },     
{ GLB_OPT (m_iDefaultInputFontWeight        ), "DefaultInputFontWeight", FW_NORMAL },           
{ GLB_OPT (m_iDefaultOutputFontHeight       ), "DefaultOutputFontHeight", 9 },                              
{ GLB_OPT (m_iIconPlacement                 ), "Icon Placement", ICON_PLACEMENT_TASKBAR },                              
{ GLB_OPT (m_iTrayIcon                      ), "Tray Icon", 0 },  // normal icon                            
{ GLB_OPT (m_nActivityWindowRefreshInterval ), "ActivityWindowRefreshInterval", 15 },
{ GLB_OPT (m_nActivityWindowRefreshType     ), "ActivityWindowRefreshType", CMUSHclientApp::eRefreshBoth },
{ GLB_OPT (m_nParenMatchFlags               ), "ParenMatchFlags",                         PAREN_NEST_BRACES | PAREN_BACKSLASH_ESCAPES | PAREN_PERCENT_ESCAPES }, 
{ GLB_OPT (m_nPrinterFontSize               ), "PrinterFontSize",               10 },
{ GLB_OPT (m_nPrinterLeftMargin             ), "PrinterLeftMargin",             15 },
{ GLB_OPT (m_nPrinterLinesPerPage           ), "PrinterLinesPerPage",           60 },
{ GLB_OPT (m_nPrinterTopMargin              ), "PrinterTopMargin",              15 },
{ GLB_OPT (m_nTimerInterval                 ), "TimerInterval",                 0 },
{ GLB_OPT (m_iFixedPitchFontSize            ), "FixedPitchFontSize",            9 },

{NULL}   // end of table marker            

  };    // end of table


#define GLB_ALPHA_OPT(arg) offsetof (CMUSHclientApp, arg)

// strings

static tGlobalConfigurationAlphaOption AlphaGlobalOptionsTable  [] = {

// option in MUSHclient.h                          name in registry          default

{ GLB_ALPHA_OPT (m_strAsciiArtFont             ), "AsciiArtFont",     "fonts\\standard.flf" },                    
{ GLB_ALPHA_OPT (m_strDefaultAliasesFile       ), "DefaultAliasesFile",   "" },             
{ GLB_ALPHA_OPT (m_strDefaultColoursFile       ), "DefaultColoursFile",   "" },      
{ GLB_ALPHA_OPT (m_strDefaultInputFont         ), "DefaultInputFont",     "FixedSys" },                   
{ GLB_ALPHA_OPT (m_strDefaultLogFileDirectory  ), "DefaultLogFileDirectory",    ".\\logs\\" },
{ GLB_ALPHA_OPT (m_strDefaultMacrosFile        ), "DefaultMacrosFile",    "" },      
{ GLB_ALPHA_OPT (m_strDefaultOutputFont        ), "DefaultOutputFont ",   "FixedSys" },              
{ GLB_ALPHA_OPT (m_strDefaultTimersFile        ), "DefaultTimersFile ",   "" },          
{ GLB_ALPHA_OPT (m_strDefaultTriggersFile      ), "DefaultTriggersFile",  "" },          
{ GLB_ALPHA_OPT (m_strDefaultWorldFileDirectory), "DefaultWorldFileDirectory",    ".\\worlds\\" },
{ GLB_ALPHA_OPT (m_strNotepadQuoteString       ), "NotepadQuoteString",     "> " },                    
{ GLB_ALPHA_OPT (m_strPluginList               ), "PluginList",                  "" },
{ GLB_ALPHA_OPT (m_strPluginsDirectory         ), "PluginsDirectory", ".\\worlds\\plugins\\" },
{ GLB_ALPHA_OPT (m_strPrinterFont              ), "PrinterFont",                "Courier" },
{ GLB_ALPHA_OPT (m_strTrayIconFileName         ), "TrayIconFileName",  "" },                    
{ GLB_ALPHA_OPT (m_strWordDelimiters           ), "WordDelimiters",         ".,()[]\"\'" },
{ GLB_ALPHA_OPT (m_strWordDelimitersDblClick   ), "WordDelimitersDblClick", ".,()[]\"\'" },
{ GLB_ALPHA_OPT (m_strWorldList                ), "WorldList",                  "" },
{ GLB_ALPHA_OPT (m_strLuaScript                ), "LuaScript",                  "" },
{ GLB_ALPHA_OPT (m_strLocale                   ), "Locale",                  "EN" },
{ GLB_ALPHA_OPT (m_strFixedPitchFont           ), "FixedPitchFont",          "FixedSys" },

{NULL}   // end of table marker            

};    // end of table


// copy the registry prefs into the SQLite database into table 'prefs'
int CMUSHclientApp::PopulateDatabase (void)
  {

  SetRegistryKey ("Gammon Software Solutions");

  int i;
  int db_rc = SQLITE_OK;

  for (i = 0; GlobalOptionsTable [i].pName; i++)
    {
    const char * p = (const char *) this +  GlobalOptionsTable [i].iOffset;
    const int Value = GetProfileInt ("Global prefs",  
                                GlobalOptionsTable [i].pName, 
                                GlobalOptionsTable [i].iDefault);

    db_rc = db_execute ((LPCTSTR) CFormat ("INSERT INTO prefs (name, value) VALUES ('%s', %i)",
                        GlobalOptionsTable [i].pName, Value), true);

    if (db_rc != SQLITE_OK)
      return db_rc;
    };

  for (i = 0; AlphaGlobalOptionsTable [i].pName; i++)
    {
    const char * p = (const char *) this +  AlphaGlobalOptionsTable [i].iOffset;

    // fix up the fixed-pitch font
    if (strcmp (AlphaGlobalOptionsTable [i].pName, "DefaultInputFont") == 0 ||
        strcmp (AlphaGlobalOptionsTable [i].pName, "DefaultOutputFont") == 0 ||
        strcmp (AlphaGlobalOptionsTable [i].pName, "FixedPitchFont") == 0)
       AlphaGlobalOptionsTable [i].sDefault = (LPCTSTR) m_strFixedPitchFont;
    
    CString strValue = GetProfileString ("Global prefs",  
                                AlphaGlobalOptionsTable [i].pName, 
                                AlphaGlobalOptionsTable [i].sDefault);

    strValue.Replace ("'", "''");  // fix up quotes

    db_rc = db_execute ((LPCTSTR) CFormat ("INSERT INTO prefs (name, value) VALUES ('%s', '%s')",
                        AlphaGlobalOptionsTable [i].pName, (LPCTSTR) strValue), true);

    if (db_rc != SQLITE_OK)
      return db_rc;

    };

   return SQLITE_OK;

  }  // end of CMUSHclientApp::PopulateDatabase 


void CMUSHclientApp::LoadGlobalsFromDatabase (void)
  {
  int i;
  string db_value;

  for (i = 0; GlobalOptionsTable [i].pName; i++)
    {
    const char * p = (const char *) this +  GlobalOptionsTable [i].iOffset;

    db_simple_query ((LPCTSTR) CFormat (
          "SELECT value FROM prefs WHERE name = '%s'", (LPCTSTR) GlobalOptionsTable [i].pName), 
          db_value, 
          true,
          (LPCTSTR) CFormat ("%i", GlobalOptionsTable [i].iDefault));

    * (long *) p = atoi (db_value.c_str());

    };

  for (i = 0; AlphaGlobalOptionsTable [i].pName; i++)
    {
    const char * p = (const char *) this +  AlphaGlobalOptionsTable [i].iOffset;

    db_simple_query ((LPCTSTR) CFormat (
          "SELECT value FROM prefs WHERE name = '%s'", (LPCTSTR)  AlphaGlobalOptionsTable [i].pName), 
          db_value, 
          true,
          AlphaGlobalOptionsTable [i].sDefault);

    * (CString *) p = db_value.c_str ();

    };


  // get Lua initialisation if necessary

  if (m_strLuaScript.IsEmpty ())
    {

    HRSRC hRsrc;
    HGLOBAL hRsrc_text = NULL;
    const char * p = NULL;
    DWORD iLength = 0;

  // Load the text of the Lua sandbox

    hRsrc = FindResource (App.m_hInstDLL, MAKEINTRESOURCE (IDR_LUA_SCRIPT), "TEXT");

    if (hRsrc)
      {
      hRsrc_text = LoadResource (App.m_hInstDLL, hRsrc);
      iLength = SizeofResource (App.m_hInstDLL, hRsrc);
      }

    if (hRsrc_text)
      p = (const char *) LockResource (hRsrc_text);

    if (p)
      m_strLuaScript = CString (p, iLength);
    else
      m_strLuaScript = "-- Lua initialization could not be loaded";
    }

  } // end of CMUSHclientApp::LoadGlobalsFromDatabase


void CMUSHclientApp::SaveGlobalsToDatabase (void)
  {

  // close and re-open database, in case they somehow lost connection to it

  if (db)
    sqlite3_close(db);

  int db_rc = sqlite3_open(m_PreferencesDatabaseName.c_str (), &db);

  if( db_rc )
    {
    ::AfxMessageBox ((LPCTSTR) CFormat ("Can't open global preferences database at: %s"
         "\r\n(Error was: \"%s\")"
         "\r\nCheck you have write-access to that file.", 
        m_PreferencesDatabaseName.c_str (), 
        sqlite3_errmsg(db)));
    sqlite3_close(db);
    db = NULL;
		return;
    }

  db_rc = db_execute ("BEGIN TRANSACTION", true);

  int i;

  for (i = 0; GlobalOptionsTable [i].pName; i++)
    {
    const char * p = (const char *) this +  GlobalOptionsTable [i].iOffset;
    const int Value = * (const long *) p;


    db_rc = db_execute ((LPCTSTR) CFormat ("UPDATE prefs SET value = %i WHERE name = '%s'",
                        Value, GlobalOptionsTable [i].pName), true);

    if (db_rc != SQLITE_OK)
      break;

    };

  if (db_rc == SQLITE_OK)
    for (i = 0; AlphaGlobalOptionsTable [i].pName; i++)
      {
      const char * p = (const char *) this +  AlphaGlobalOptionsTable [i].iOffset;
      CString strValue = * (CString *) p;
    
      strValue.Replace ("'", "''");  // fix up quotes

      db_rc = db_execute ((LPCTSTR) CFormat ("UPDATE prefs SET value = '%s' WHERE name = '%s'",
                          (LPCTSTR) strValue, AlphaGlobalOptionsTable [i].pName), true);

      if (db_rc != SQLITE_OK)
        break;

      };

  db_execute ("COMMIT", true);

  } // end of CMUSHclientApp::SaveGlobalsToDatabase

void CMUSHclientApp::ShowGlobalOptions (CMUSHclientDoc * pDoc)
  {

  int i;

  for (i = 0; GlobalOptionsTable [i].pName; i++)
    {
    const char * p = (const char *) this +  GlobalOptionsTable [i].iOffset;
    const int Value = * (const long *) p;
    pDoc->Note (CFormat ("%35s = %ld",
                   (LPCTSTR) GlobalOptionsTable [i].pName, 
                   Value));

    };

  for (i = 0; AlphaGlobalOptionsTable [i].pName; i++)
    {
    const char * p = (const char *) this +  AlphaGlobalOptionsTable [i].iOffset;
    CString strValue = * (CString *) p;
    
    pDoc->Note (CFormat ("%35s = %s",
                   (LPCTSTR) AlphaGlobalOptionsTable [i].pName, 
                   (LPCTSTR) strValue));

    };



  } // end of CMUSHclientApp::ShowGlobalOptions 


VARIANT CMUSHclientApp::GetGlobalOption(LPCTSTR Name) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  int i;

  for (i = 0; GlobalOptionsTable [i].pName; i++)
    {
    if (_stricmp (Name, GlobalOptionsTable [i].pName) == 0)
      {
      const char * p = (const char *) this +  GlobalOptionsTable [i].iOffset;
      const int Value = * (const long *) p;
      vaResult.vt = VT_I4;
      vaResult.lVal = Value; 
  	  return vaResult;
      }

    };

  for (i = 0; AlphaGlobalOptionsTable [i].pName; i++)
    {
    if (_stricmp (Name, AlphaGlobalOptionsTable [i].pName) == 0)
      {
      const char * p = (const char *) this +  AlphaGlobalOptionsTable [i].iOffset;
      CString strValue = * (CString *) p;
      vaResult.vt = VT_BSTR;
      vaResult.bstrVal = strValue.AllocSysString (); 
  	  return vaResult;
      }
    };

	return vaResult;   // not found (empty)
}


VARIANT CMUSHclientApp::GetGlobalOptionList() 
{
  COleSafeArray sa;   // for list
  long i, count = 0;

  // count them
  for (i = 0; GlobalOptionsTable [i].pName; i++)
    count++;

  for (i = 0; AlphaGlobalOptionsTable [i].pName; i++)
    count++;

  sa.CreateOneDim (VT_VARIANT, count);
  count = 0;

  // put the numeric option names into the array
  for (i = 0; GlobalOptionsTable [i].pName; i++)
    {
    // the array must be a bloody array of variants, or VBscript kicks up
    COleVariant v ((LPCTSTR) GlobalOptionsTable [i].pName);
    sa.PutElement (&count, &v);
    count++;
    }      // end of looping through each option

  // put the alpha option names into the array
  for (i = 0; AlphaGlobalOptionsTable [i].pName; i++)
    {
    // the array must be a bloody array of variants, or VBscript kicks up
    COleVariant v ((LPCTSTR) AlphaGlobalOptionsTable [i].pName);
    sa.PutElement (&count, &v);
    count++;
    }      // end of looping through each option

	return sa.Detach ();
}

