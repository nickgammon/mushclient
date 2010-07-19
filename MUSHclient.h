// MUSHclient.h : main header file for the MUSHCLIENT application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "hostsite.h"
#include "scripting\scripting.h"
#include "othertypes.h"

#define DIRECTSOUND_VERSION 5

#include <mmsystem.h>
#include <dsound.h>

/////////////////////////////////////////////////////////////////////////////
// CMUSHclientApp:
// See MUSHclient.cpp for the implementation of this class
//

#define MAX_SPELL_ERRORS 10

// how many sounds we can play at once
#define MAX_SOUND_BUFFERS 10

// flags for m_nParenMatchFlags
#define PAREN_NEST_BRACES    0x0001      // do braces nest? eg. ( [)] )
#define PAREN_SINGLE_QUOTES  0x0002      // handle single quotes eg. 'nick'
#define PAREN_DOUBLE_QUOTES  0x0004      // handle double quotes eg. "nick"
#define PAREN_ESCAPE_SINGLE_QUOTES  0x0008      // escape inside single quotes, eg. 'nick\'s dog'
#define PAREN_ESCAPE_DOUBLE_QUOTES  0x0010      // escape inside double quotes, eg. "Press \"enter\" "
#define PAREN_BACKSLASH_ESCAPES  0x0020   // \ escapes next character, eg. (  \)  )
#define PAREN_PERCENT_ESCAPES  0x0040   // % escapes next character, eg. (  %)  )

#define ICON_PLACEMENT_TASKBAR 0
#define ICON_PLACEMENT_TRAY 1
#define ICON_PLACEMENT_BOTH 2

class CActivityDoc;
class CActivityView;
class CTextDocument;

extern COLORREF xterm_256_colours [256];

class CMUSHclientApp : public CWinApp
{
public:

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CMUSHclientApp();
	~CMUSHclientApp();

  CActivityDoc * m_pActivityDoc;
  CActivityView * m_pActivityView;
  CTime m_timeLastActivityUpdate;

  DWORD platform;

	CMultiDocTemplate* m_pWorldDocTemplate;
#ifdef PANE
  CMultiDocTemplate* m_pPaneTemplate;         // for extra panes
#endif // PANE
	CMultiDocTemplate* m_pActivityDocTemplate;
	CMultiDocTemplate* m_pNormalDocTemplate;    // text document

  CAtomicElementMap  m_ElementMap;   // MXP elements we know of (eg. <b> )
  CMapStringToString m_EntityMap;    // MXP entities we know of (eg. &lt; )
  CColoursMap        m_ColoursMap;   // MXP colours we know of (eg. blue)

  BOOL m_bUpdateActivity;

  bool m_bShowInitialDelay;

  enum {
        eNormalNewDocument,
        eQuickConnect,
        eTelnetFromNetscape,
    };

  int m_TypeOfNewDocument;

	COleTemplateServer m_server;		// Server object for document creation

  enum {
        eRefreshOnActivity,
        eRefreshPeriodically,
        eRefreshBoth,
    };    // activity world refresh types

  bool m_bSpellCheckOK;

  bool m_bEchoSendToAll;    // does send to all get echoed?

  __int64 m_nUniqueNumber;

  lua_State           * m_SpellChecker_Lua;     // Lua state - spellchecker
  lua_State           * m_Translator_Lua;       // Lua state - translation (i18n)

  CString m_strTranslatorFile;    // eg. (MUSHclient executable)\locale\EN.lua

  HINSTANCE m_hInstDLL;  // resources file - all dialogs etc., loaded in application startup

  LPDIRECTSOUND m_pDirectSoundObject;   // for DirectSound output
  LPDIRECTSOUNDBUFFER m_pDirectSoundPrimaryBuffer;

  // SQLite database handle for global preferences

  sqlite3 *db;
  string m_PreferencesDatabaseName;

// global preferences

  int m_iDefaultInputFontHeight;
  int m_iDefaultInputFontWeight;
  int m_iDefaultInputFontCharset;
  int m_iDefaultOutputFontHeight;
  int m_iDefaultOutputFontCharset;
  unsigned int m_bAllTypingToCommandWindow;
  unsigned int m_bAlwaysOnTop;
  unsigned int m_bAppendToLogFiles;
  unsigned int m_bAutoConnectWorlds;
  unsigned int m_bAutoExpand;
  unsigned int m_bFlatToolbars;
  unsigned int m_bAutoLogWorld;
  unsigned int m_bBleedBackground; 
  unsigned int m_bColourGradient;
  unsigned int m_bConfirmBeforeClosingMXPdebug;
  unsigned int m_bConfirmBeforeClosingMushclient;
  unsigned int m_bConfirmBeforeClosingWorld;
  unsigned int m_bConfirmBeforeSavingVariables;
  unsigned int m_bConfirmLogFileClose;
  unsigned int m_bEnableSpellCheck;  // is spell checker enabled?
  unsigned int m_bF1macro;
  unsigned int m_bFixedFontForEditing;
  unsigned int m_bNotepadWordWrap;
  unsigned int m_bNotifyIfCannotConnect;
  unsigned int m_bErrorNotificationToOutputWindow;
  unsigned int m_bNotifyOnDisconnect;
  unsigned int m_bOpenActivityWindow;
  unsigned int m_bOpenWorldsMaximised;
  unsigned int m_iWindowTabsStyle;
  unsigned int m_bReconnectOnLinkFailure;
  unsigned int m_bRegexpMatchEmpty;
  unsigned int m_bShowGridLinesInListViews;
  unsigned int m_bSmoothScrolling;
  unsigned int m_bSmootherScrolling;
  unsigned int m_bDisableKeyboardMenuActivation;
  unsigned int m_bTriggerRemoveCheck;
  unsigned int m_iActivityButtonBarStyle;
  unsigned int m_iAsciiArtLayout;
  unsigned int m_iDefaultInputFontItalic;
  unsigned int m_iIconPlacement;  // 0 = task bar, 1 = tray, 2 = both
  unsigned int m_iTrayIcon;       // 0 = default, otherwise non-standard (10 = choose file)
  unsigned int m_nActivityWindowRefreshInterval;
  unsigned int m_nActivityWindowRefreshType;
  unsigned int m_nParenMatchFlags;
  unsigned int m_nPrinterFontSize;
  unsigned int m_nPrinterLeftMargin;
  unsigned int m_nPrinterLinesPerPage;
  unsigned int m_nPrinterTopMargin;
  unsigned int m_nTimerInterval;
  unsigned int m_bEnablePackageLibrary;
  unsigned int m_iFixedPitchFontSize;


  COLORREF m_cNotepadBackColour;
  COLORREF m_cNotepadTextColour;

  CString	m_strAsciiArtFont;
  CString	m_strDefaultAliasesFile;
  CString	m_strDefaultColoursFile;
  CString	m_strDefaultInputFont;
  CString	m_strDefaultMacrosFile;
  CString	m_strDefaultOutputFont;
  CString	m_strDefaultTimersFile;
  CString	m_strDefaultTriggersFile;
  CString m_strDefaultLogFileDirectory;
  CString m_strDefaultWorldFileDirectory;
  CString m_strDefaultStateFilesDirectory;
  CString m_strNotepadQuoteString;
  CString m_strPluginList;
  CString m_strPluginsDirectory;
  CString m_strPrinterFont;
  CString m_strTrayIconFileName;
  CString m_strWordDelimiters;
  CString m_strWordDelimitersDblClick;
  CString m_strWorldList;
  CString m_strLuaScript;
  CString m_strLocale;
  CString m_strFixedPitchFont;

// other things

  LONGLONG m_iCounterFrequency;
  WSADATA m_wsadata;

	CString	m_strAsciiArtText;
  CString m_strMUSHclientFileName;

  CString m_strLastDebugCommand;

// for string.gsub find box


  CString m_strFind;
  CString m_strReplace;
  CString m_strFunctionText;
  bool    m_bEachLine;
  bool    m_bEscapeSequences;
  bool    m_bCallFunction;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMUSHclientApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL SaveAllModified();
	virtual BOOL OnIdle(LONG lCount);
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

  void RestoreColumnConfiguration (LPCTSTR strName, 
                                   const int iColCount,
                                   CListCtrl& ctlList,
                                   int iColOrder [],
                                   int iColWidth [],
                                   int & iLastColumn,
                                   BOOL & bReverse);

  void SaveColumnConfiguration (LPCTSTR strName, 
                                const int iColCount,
                                CListCtrl& ctlList,
                                const int iLastColumn,
                                const BOOL bReverse);

  void UpdateWorldCounts (void);

  // spell check stuff
  void InitSpellCheck (void);
  bool SpellCheck (CWnd * pWnd, CEdit * pEdit);

  __int64 GetUniqueNumber (void) { return m_nUniqueNumber++; };
  CString GetUniqueString (void);

  void MXP_LoadEntities (void);
  void MXP_LoadElements (void);
  void MXP_LoadColours (void);

  void LoadGlobalsFromDatabase (void);
  void SaveGlobalsToDatabase (void);
  int PopulateDatabase (void);
  void ShowGlobalOptions (CMUSHclientDoc * pDoc);
  VARIANT GetGlobalOption(LPCTSTR Name);
  VARIANT GetGlobalOptionList();

  void OpenLuaDelayed ();  // for spell checker

  BOOL I18N_Setup (void);  // internationalization

  bool AppendToTheNotepad (const CString strTitle,
                          const CString strText,
                          const bool bReplace,
                          const int  iNotepadType);

  CTextDocument * FindNotepad (const CString strTitle);
  BOOL ActivateNotepad(LPCTSTR Title);

  HCURSOR	 g_hCursorIbeam;

  int db_simple_query (const char * sql, string & result, const bool bShowError = false, const char * default_value = "");
  int db_execute (const char * sql, const bool bShowError = false);
  void db_show_error (const char * sql);
  CString db_get_string (LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = "" );
  int db_write_string (LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
  int db_get_int (LPCTSTR lpszSection, LPCTSTR lpszEntry, int iDefault = 0 );
  int db_write_int (LPCTSTR lpszSection, LPCTSTR lpszEntry, int iValue);


  void WorkOutFixedFont ();

  /*
  virtual void OnFinalRelease()
    {
      CWinApp::OnFinalRelease();
    }

  HRESULT Test()
    {
    ::AfxMessageBox ("Test");
      return TRUE;
    }

  */

	//{{AFX_MSG(CMUSHclientApp)
	afx_msg void OnAppAbout();
	afx_msg void OnGameMinimiseprogram();
	afx_msg void OnConnectionQuickConnect();
	afx_msg void OnFileNew();
	afx_msg void OnHelpGettingstarted();
	afx_msg void OnEditColourpicker();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    // Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CMUSHclientApp)
	//}}AFX_DISPATCH
//	DECLARE_DISPATCH_MAP()
//	DECLARE_INTERFACE_MAP()

/*
DECLARE_DYNCREATE(CMUSHclientApp)
DECLARE_OLECREATE(CMUSHclientApp)
DECLARE_DISPATCH_MAP()
DECLARE_INTERFACE_MAP()

  */

public:
	void ShowTipAtStartup(void);
private:
	void ShowTipOfTheDay(void);
};


/////////////////////////////////////////////////////////////////////////////

// for global numeric options
typedef struct
  {
  int     iOffset;    // offset in CMUSHclientApp
  char *  pName;      // name, eg. "ActivityButtonBarStyle"
  int     iDefault;   // original (default) value
  }  tGlobalConfigurationNumericOption;

// for global alpha options
typedef struct
  {
  int    iOffset;    // offset in CMUSHclientApp
  char * pName;      // name, eg. "AsciiArtFont"
  const char * sDefault;   // original (default) value
  }  tGlobalConfigurationAlphaOption;


