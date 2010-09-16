// MUSHclient.cpp : Defines the class behaviors for the application.
//

// xgettext  -kTMessageBox -kTranslate -o mushclient_static.po *.cpp
// xgettext  -kTranslateFormat -o mushclient_formatted.po *.cpp

#include "stdafx.h"
#include "MUSHclient.h"
#include "doc.h"
#include "ActivityDoc.h"
#include "TextDocument.h"

#include "mainfrm.h"
#include "childfrm.h"
#include "activitychildfrm.h"
#include "textchildfrm.h"
#include "MUSHview.h"
#include "ActivityView.h"
#include "TextView.h"

#include "winplace.h"
#include "StatLink.h"

#include "dialogs\welcome.h"
#include "dialogs\welcome1.h"

#include "dialogs\TipDlg.h"
#include "dialogs\CreditsDlg.h"
#include "dialogs\ColourPickerDlg.h"

#include "dialogs\Splash.h"
#include "direct.h"

COLORREF xterm_256_colours [256];

// Lua 5.1
#pragma comment( lib, "lua5.1.lib" )

// library needed for timers
#pragma comment( lib, "winmm.lib")

// Winsock library
#pragma comment( lib, "ws2_32.lib ")

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static TCHAR BASED_CODE szCtrlBars[] = _T("CtrlBars");

// working directory at login time

char working_dir [_MAX_PATH];
char file_browsing_dir [_MAX_PATH];

bool bWinNT;
bool bWin95;
bool bWin98;
bool bWine;

// memory state tracking
#ifdef _DEBUG
    CMemoryState oldMemState, newMemState, diffMemState;
#endif

extern tConfigurationNumericOption OptionsTable [];
extern tConfigurationAlphaOption AlphaOptionsTable [];

void LoadMapDirections (void);

void Generate256colours (void);

extern const struct luaL_reg *ptr_xmllib;

extern "C"
  {
  LUALIB_API int luaopen_rex(lua_State *L);
  LUALIB_API int luaopen_bits(lua_State *L);
  LUALIB_API int luaopen_compress(lua_State *L);
  LUALIB_API int luaopen_bc(lua_State *L);
  LUALIB_API int luaopen_lsqlite3(lua_State *L);
  LUALIB_API int luaopen_lpeg (lua_State *L);
  }

/////////////////////////////////////////////////////////////////////////////
// CMUSHclientApp

//IMPLEMENT_DYNCREATE(CMUSHclientApp, CWinApp)

BEGIN_MESSAGE_MAP(CMUSHclientApp, CWinApp)
	ON_COMMAND(CG_IDS_TIPOFTHEDAY, ShowTipOfTheDay)
	//{{AFX_MSG_MAP(CMUSHclientApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_GAME_MINIMISEPROGRAM, OnGameMinimiseprogram)
	ON_COMMAND(ID_CONNECTION_QUICK_CONNECT, OnConnectionQuickConnect)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_HELP_GETTINGSTARTED, OnHelpGettingstarted)
	ON_COMMAND(ID_EDIT_COLOURPICKER, OnEditColourpicker)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/*

  See: http://www.codeproject.com/com/mfc_autom.asp

BEGIN_DISPATCH_MAP(CMUSHclientApp, CWinApp)
  DISP_FUNCTION_ID(CMUSHclientApp, "Test", dispidTest, Test, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()

static const IID IID_IMUSHclient =
  { 0xE594883F, 0x0CC4, 0x491a, { 0xA2, 0x8C, 0xB5,0x07, 0x1E, 0x53, 0xAE, 0x2C } };


BEGIN_INTERFACE_MAP(CMUSHclientApp, CWinApp)
//  INTERFACE_PART(CMUSHclientApp, IID_IMUSHclient, LocalClass)
  INTERFACE_PART(CMUSHclientApp, IID_IMUSHclient, Dispatch)
END_INTERFACE_MAP()

  */

/*
BEGIN_DISPATCH_MAP(CMUSHclientApp, CWinApp)
   //{{AFX_DISPATCH_MAP(CMUSHclientApp)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_IMUSHclient to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {11DFC5E8-AD6F-11D0-8EAE-00A0247B3BFD}
static const IID IID_IMUSHclient =
{ 0x11dfc5e8, 0xad6f, 0x11d0, { 0x8e, 0xae, 0x0, 0xa0, 0x24, 0x7b, 0x3b, 0xfd } };

BEGIN_INTERFACE_MAP(CMUSHclientApp, CWinApp)
	INTERFACE_PART(CMUSHclientApp, IID_IMUSHclient, Dispatch)
END_INTERFACE_MAP()

*/

/*
BEGIN_INTERFACE_PART(LocalClass, IMUSHclient)
    STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo);
    STDMETHOD(GetTypeInfo)(
        UINT itinfo,
        LCID lcid,
        ITypeInfo FAR* FAR* pptinfo);
    STDMETHOD(GetIDsOfNames)(
        REFIID riid,
        OLECHAR FAR* FAR* rgszNames,
        UINT cNames,
        LCID lcid,
        DISPID FAR* rgdispid);
    STDMETHOD(Invoke)(
        DISPID dispidMember,
        REFIID riid,
        LCID lcid,
        WORD wFlags,
        DISPPARAMS FAR* pdispparams,
        VARIANT FAR* pvarResult,
        EXCEPINFO FAR* pexcepinfo,
        UINT FAR* puArgErr);
    STDMETHOD(Test)(THIS);
END_INTERFACE_PART(LocalClass)


IMPLEMENT_OLECREATE(CMUSHclientApp, "MUSHclient.Application", 
                    0x14FE63AB, 0x691A, 0x11DB, 0x99, 0x8B, 0x00, 0x00, 0x8C, 0x01, 0x27, 0x85 )

  */

//#define new DEBUG_NEW 

/////////////////////////////////////////////////////////////////////////////
// CMUSHclientApp construction

CMUSHclientApp::CMUSHclientApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

EnableAutomation();
   
//::AfxOleLockApp();

}

CMUSHclientApp::~CMUSHclientApp()
{
//::AfxOleUnlockApp();

}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMUSHclientApp object

CMUSHclientApp theApp;

// This identifier was generated to be statistically unique for your app.
// You may change it if you prefer to choose a specific identifier.

// {11DFC5E6-AD6F-11D0-8EAE-00A0247B3BFD}
static const CLSID clsid =
{ 0x11dfc5e6, 0xad6f, 0x11d0, { 0x8e, 0xae, 0x0, 0xa0, 0x24, 0x7b, 0x3b, 0xfd } };



/////////////////////////////////////////////////////////////////////////////
// CMUSHclientApp initialization

BOOL CMUSHclientApp::InitInstance()
{

  m_whenClientStarted = CTime::GetCurrentTime();

  char fullfilename [MAX_PATH];

  if (GetModuleFileName (NULL, fullfilename, sizeof (fullfilename)))
    m_strMUSHclientFileName = ExtractDirectory (CString (fullfilename));
  else
    m_strMUSHclientFileName = ".\\MUSHclient.exe";

  // stupid cursor disappears under Parallels
  g_hCursorIbeam = CopyCursor(AfxGetApp()->LoadCursor (IDC_MY_IBEAM));  

// find the working directory at startup time

  _getdcwd (0, working_dir, sizeof (working_dir) - 1);

// make sure directory name ends in a slash

  working_dir [sizeof (working_dir) - 2] = 0;

  if (working_dir [strlen (working_dir) - 1] != '\\')
    strcat (working_dir, "\\");  

  // where we do file browsing from
  strcpy (file_browsing_dir, working_dir);


  // First free the string that was allocated by MFC in the startup
   // of CWinApp. The string is allocated before InitInstance is
   // called.
   free((void*)m_pszProfileName);

   // Change the name of the .INI file--CWinApp destructor will free
   // the memory.

   CString strIniFile (working_dir);
   strIniFile += "MUSHclient.ini";

   m_pszProfileName=_tcsdup(strIniFile);


  WorkOutFixedFont ();  // FixedSys or whatever

  // open SQLite database for preferences

  int rc;
  CFileStatus	status;

  // initially look in MUSHclient working directory for database
  m_PreferencesDatabaseName = working_dir;
  m_PreferencesDatabaseName += PREFERENCES_DATABASE_FILE;

  // if not there, try application directory
  if (!CFile::GetStatus(m_PreferencesDatabaseName.c_str (), status))
    {
    CString strTemp;
    strTemp = ExtractDirectory (App.m_strMUSHclientFileName);
    strTemp += PREFERENCES_DATABASE_FILE;

    // if actually in the application directory, switch to using that
    if (CFile::GetStatus(strTemp, status))
      m_PreferencesDatabaseName = strTemp;

    // if not, leave in the working directory
    }


  db = NULL;

  rc = sqlite3_open(m_PreferencesDatabaseName.c_str (), &db);

  if( rc )
    {
    ::AfxMessageBox ((LPCTSTR) CFormat ("Can't open global preferences database at: %s"
         "\r\n(Error was: \"%s\")"
         "\r\nCheck you have write-access to that file.", 
        m_PreferencesDatabaseName.c_str (), 
        sqlite3_errmsg(db)));
    sqlite3_close(db);
		return FALSE;
    }


#define CURRENT_DB_VERSION 1

  string db_version;
  int db_rc;

  db_rc = db_simple_query ("SELECT value FROM control WHERE name = 'database_version'", db_version, false);

  // no version or out of date, make database
  if (db_version.empty () || atoi (db_version.c_str ()) < CURRENT_DB_VERSION)
    {

    db_execute ("BEGIN TRANSACTION", true);

    db_rc = db_execute (
      // general control information
      "DROP TABLE IF EXISTS control;"
      "CREATE TABLE control (name VARCHAR(10) NOT NULL PRIMARY KEY, value INT NOT NULL );",
      true);

    if (db_rc != SQLITE_OK)
      {
      db_execute ("ROLLBACK", true);
      return FALSE;        // SQL error
      }

    db_write_int ("control", "database_version", CURRENT_DB_VERSION);

    db_rc = db_execute (

      // global preferences
      "DROP TABLE IF EXISTS prefs;"
      "CREATE TABLE prefs (name VARCHAR(50) NOT NULL PRIMARY KEY, value TEXT NOT NULL ); "

      // world window positions
      "DROP TABLE IF EXISTS worlds;"
      "CREATE TABLE worlds (name VARCHAR(50) NOT NULL PRIMARY KEY, value TEXT NOT NULL ); "
      
      ,true
      );

    if (db_rc != SQLITE_OK)
      {
      db_execute ("ROLLBACK", true);
      return FALSE;        // SQL error
      }

    // copy from registry to database for legacy support
    if (PopulateDatabase () != SQLITE_OK)
      {
      db_execute ("ROLLBACK", true);
      return FALSE;        // SQL error
      }

    db_execute ("COMMIT", true);

    }   // end database empty

  // i18n setup  -------------------------------

  if (!I18N_Setup ())
    return FALSE;        // no resources, or Lua won't start up

// as at version 3.13
// int i = sizeof (CLine);     // 56 bytes
// int j = sizeof (CStyle);    // 20 bytes
// int k = sizeof (CAction);   // 28 bytes 

// memory state tracking
#ifdef _DEBUG
    oldMemState.Checkpoint();
#endif

   if (strstr (m_lpCmdLine, "/wine"))
     bWine = true;
   else
     bWine = false;

   LoadMapDirections ();
   Generate256colours ();

 //  UMessageBox ("\xC9\xB3\xC9\xA8\xC9\x95\xC9\xAE");

   // set the current locale
   setlocale (LC_ALL, "" );

// Set the debug-heap flag so that freed blocks are kept on the
   // linked list, to catch any inadvertent use of freed memory
   SET_CRT_DEBUG_FIELD( _CRTDBG_DELAY_FREE_MEM_DF );
   SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );
//   SET_CRT_DEBUG_FIELD( _CRTDBG_CHECK_ALWAYS_DF );
// speed warning: see:   _ASSERTE( _CrtCheckMemory( ) );
// in ProcessPreviousLine.cpp

  m_SpellChecker_Lua = NULL;

  m_bShowInitialDelay = true;
  m_TypeOfNewDocument = eNormalNewDocument;
  m_bSpellCheckOK = false;
  m_bEnableSpellCheck = true;
  m_bEchoSendToAll = true;
  m_nUniqueNumber = 0;

  // for string.gsub dialog
  m_bEachLine = true;             
  m_bEscapeSequences = false;      
  m_bCallFunction = false;         

  m_pDirectSoundObject = NULL;
  m_pDirectSoundPrimaryBuffer = NULL;

	// Initialize OLE libraries
  if (!bWine)
    if (!AfxOleInit())        // not needed?
	  {
		  TMessageBox("OLE initialization failed", MB_ICONSTOP); 
		  return FALSE;
	  }

  AfxInitRichEdit ();

  LARGE_INTEGER large_int_frequency;
  if (QueryPerformanceFrequency (&large_int_frequency))
    m_iCounterFrequency = large_int_frequency.QuadPart;
  else
    m_iCounterFrequency = 0;    // no performance counter

	// CG: The following block was added by the Splash Screen component.

  CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	CSplashWnd::EnableSplashScreen(true);

  TRACE ("MUSHclient starting up ...\n");

	if (!AfxSocketInit(&m_wsadata))
	{
		::AfxMessageBox(IDP_SOCKETS_INIT_FAILED, MB_ICONSTOP);
		return FALSE;
	}

	// Standard initialization

	Enable3dControls();

  //Make sure this is here so you can use XP Styles
  //InitCommonControls();

	LoadStdProfileSettings(10);  // Load standard INI file options (including MRU)

  if (!bWine)
    AfxEnableControlContainer ();   // not needed?

  m_pActivityDoc = NULL;
  m_pActivityView = NULL;
  m_bUpdateActivity = FALSE;


  // seed the random number generator
  time_t timer;
  time (&timer);
  srand (timer);  

  // Marsenne Twister generator
  init_genrand (timer);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

  // put this first so unknown extensions default to text (eg. mush)
  // the normal document (*.txt)

	m_pNormalDocTemplate = new CMultiDocTemplate(
		IDR_NORMALTYPE,
		RUNTIME_CLASS(CTextDocument),
		RUNTIME_CLASS(CTextChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CTextView));
	AddDocTemplate(m_pNormalDocTemplate);

  // normal worlds

	m_pWorldDocTemplate = new CMultiDocTemplate(
		IDR_MUSHCLTYPE,
		RUNTIME_CLASS(CMUSHclientDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CMUSHView));
	AddDocTemplate(m_pWorldDocTemplate);

#ifdef PANE

  // normal worlds - extra pane windows

	m_pPaneTemplate = new CMultiDocTemplate(
		IDR_PANETYPE,
		RUNTIME_CLASS(CMUSHclientDoc),
		RUNTIME_CLASS(CPaneChildWnd), // custom MDI child frame
		RUNTIME_CLASS(CPaneView));
	AddDocTemplate(m_pPaneTemplate);

#endif

  // the activity window

	m_pActivityDocTemplate = new CMultiDocTemplate(
		IDR_ACTIVITYTYPE,
		RUNTIME_CLASS(CActivityDoc),
		RUNTIME_CLASS(CActivityChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CActivityView));
	AddDocTemplate(m_pActivityDocTemplate);

  if (!bWine)
    {
	  // Connect the COleTemplateServer to the document template.
	  //  The COleTemplateServer creates new documents on behalf
	  //  of requesting OLE containers by using information
	  //  specified in the document template.
	  m_server.ConnectTemplate(clsid, m_pWorldDocTemplate, FALSE);

	  // Register all OLE server factories as running.  This enables the
	  //  OLE libraries to create objects from other applications.
	  COleTemplateServer::RegisterAll();
		  // Note: MDI applications register all server objects without regard
		  //  to the /Embedding or /Automation on the command line.
    }


  // read global prefs from the database

  LoadGlobalsFromDatabase ();

// check for configuration name collisions

#ifdef _DEBUG
  int i;
  for (i = 0; AlphaOptionsTable [i].pName; i++)
    for (int j = 0;  OptionsTable [j].pName; j++)
      if (strcmp (AlphaOptionsTable [i].pName, OptionsTable [j].pName) == 0)
        ::UMessageBox (TFormat ("Internal MUSHclient error, config name collision: %s",
                         (LPCTSTR) OptionsTable [j].pName), MB_ICONEXCLAMATION);
#endif

  CWnd * extraWnd = NULL;

  // if *only* tray wanted, hide icon from task bar
  if (m_iIconPlacement == ICON_PLACEMENT_TRAY)
    {
    extraWnd = new CWnd;

    // MUSHclient icon for hidden window, so Alt+Tab will look OK
    HICON hIcon =
        (HICON)LoadImage( AfxGetResourceHandle(),
            MAKEINTRESOURCE(IDR_MUSHCLTYPE),
            IMAGE_ICON,
            GetSystemMetrics(SM_CXICON),
            GetSystemMetrics(SM_CYICON),
            LR_DEFAULTCOLOR);

    // for hiding main window from taskbar
    VERIFY(extraWnd->CreateEx 
        ( 0, AfxRegisterWndClass(CS_CLASSDC | CS_GLOBALCLASS, 0, 0, hIcon), 
        _T("MUSHclient"), WS_OVERLAPPEDWINDOW | WS_EX_TOOLWINDOW,
        0, 0, 0, 0, NULL, NULL ));

    }   // end of tray wanted

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME,
    WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE  , 
      extraWnd)) // owner is hidden window
    {
    TMessageBox ("Unable to load main frame window", MB_ICONSTOP);
		return FALSE;
    }
	m_pMainWnd = pMainFrame;

  if (m_iIconPlacement == ICON_PLACEMENT_TRAY ||
      m_iIconPlacement == ICON_PLACEMENT_BOTH)
    Frame.AddTrayIcon ();       

  // Enable DDE Execute open 

  if (strstr (m_lpCmdLine, "/noregister") == NULL)
    {
	  EnableShellOpen();
	  RegisterShellFileTypes();
    }

// initialise COM

  if (!bWine)
    CoInitialize (NULL);

  // check direct sound available
  FARPROC pDirectSoundCreate = NULL;

  if (strstr (m_lpCmdLine, "/nodirectsound") == NULL)
    {
	  HMODULE hDLL = LoadLibrary ("dsound");
    if (hDLL)
		  pDirectSoundCreate = GetProcAddress(hDLL, "DirectSoundCreate");

    if (pDirectSoundCreate)
      {
      // try to set up for DirectSound
      if (FAILED (DirectSoundCreate (NULL, &m_pDirectSoundObject, NULL))) 
        m_pDirectSoundObject = NULL;

      // set sound cooperation level
      if (m_pDirectSoundObject)
        if (FAILED (m_pDirectSoundObject->SetCooperativeLevel (pMainFrame->m_hWnd, DSSCL_NORMAL)))
          m_pDirectSoundObject = NULL;    // no DirectSound
      }
    }   // if DirectSound wanted

  if (m_pDirectSoundObject)
    {
    DSBUFFERDESC bd;

    memset (&bd, 0, sizeof (DSBUFFERDESC));
    bd.dwSize = sizeof (DSBUFFERDESC);
    bd.dwFlags = DSBCAPS_PRIMARYBUFFER;
    bd.dwBufferBytes = 0;     //must be 0 for primary buffer
    bd.lpwfxFormat = NULL;    //must be null for primary buffer


    if (FAILED (m_pDirectSoundObject->CreateSoundBuffer (&bd, &m_pDirectSoundPrimaryBuffer, NULL)))
      m_pDirectSoundObject = NULL;  // no DirectSound
    }


  // see which OS we are using
  OSVERSIONINFO ver;
  memset(&ver, 0, sizeof(ver));
  ver.dwOSVersionInfoSize = sizeof(ver);
  VERIFY(::GetVersionEx(&ver));
  bWinNT = (ver.dwPlatformId == VER_PLATFORM_WIN32_NT);
  bWin95 = (ver.dwPlatformId == 
     VER_PLATFORM_WIN32_WINDOWS) && (ver.dwMinorVersion == 0);
  bWin98 = (ver.dwPlatformId == 
     VER_PLATFORM_WIN32_WINDOWS) && (ver.dwMinorVersion > 0);

  // MXP initialisation

  MXP_LoadElements ();  // elements like <b>
  MXP_LoadEntities ();  // entities like &gt;
  MXP_LoadColours ();   // colours like blue

// show the main window

 	pMainFrame->ShowWindow(m_nCmdShow);

// get main window position from last time

  CWindowPlacement wp;
  pMainFrame->GetWindowPlacement(&wp);   // default if no registry entry

  wp.Restore ("Main window", pMainFrame, true);

  pMainFrame->LoadBarState(szCtrlBars);

  if (m_bAlwaysOnTop)
    pMainFrame->SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);

// get finger cursor

  CStaticLink::g_hCursorLink = AfxGetApp()->LoadCursor (ID_FINGER_CURSOR);

  bool bAutoOpen = true; 

	// simple command line parsing
	if (m_lpCmdLine[0] == '\0')
	{
		// create a new (empty) document
	//	OnFileNew();    // do nothing
	}
	else
	  {
    CString strTemp = m_lpCmdLine;
    strTemp.MakeLower ();

    strTemp.TrimLeft ();

    // look for /noauto command-line option
    if (strTemp == "/noauto")
      bAutoOpen = false;
    else if (strTemp == "/wine")
      { }  // do nothing else, checked further up
    else if (strTemp == "/noregister")
      { }  // do nothing else, checked further up
    else if (strTemp == "/nodirectsound")
      { }  // do nothing else, checked further up
    else if (strstr (strTemp, ".mcl"))
		// open an existing document
		  OpenDocumentFile(m_lpCmdLine);
    else
      {
      // switch to "telnet from netscape" mode
      m_TypeOfNewDocument = eTelnetFromNetscape;
      App.m_pWorldDocTemplate->OpenDocumentFile(NULL);
      // back to normal
      m_TypeOfNewDocument = eNormalNewDocument;
      bAutoOpen = false;    // and cancel auto-open
      } // end of world and port supplied
	}

  // enable spell checker, if wanted in prefs
  if (m_bEnableSpellCheck)
    InitSpellCheck ();

// open all worlds specified in global preferences if no shift key is down

  if ((GetKeyState (VK_LSHIFT) & 0x8000) == 0 &&
      (GetKeyState (VK_RSHIFT) & 0x8000) == 0 && 
      bAutoOpen)
    {
    vector<string> v;

    StringToVector ((const char *) m_strWorldList, v, "*");

    for (vector<string>::const_iterator i = v.begin (); i != v.end (); i++)
      m_pWorldDocTemplate->OpenDocumentFile (i->c_str ());

    }

	// The main window has been initialized, so update it.
	pMainFrame->UpdateWindow();

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// When a server application is launched stand-alone, it is a good idea
	//  to update the system registry in case it has been damaged.

  /*

  Ah, bollocks. It just raises an error on Windows XP guest accounts.
  // NJG - version 4.03

  if (!bWine)
    {  
    m_server.UpdateRegistry(OAT_DISPATCH_OBJECT);
	  COleObjectFactory::UpdateRegistryAll();
    }

  */

// Show first welcome

  BOOL firsttime = db_get_int ("control", "First time", 1);	
  UINT version = 0;

  if (firsttime)
    {

    // fix up tool bars
    Frame.RecalcLayout(TRUE);
	  CRect rectBar;
    CMyToolBar * pToolBar;
    
    // main toolbar
    pToolBar = &Frame.m_wndToolBar;
    if (pToolBar)   // assuming we can find it
      {
	    pToolBar->GetWindowRect(&rectBar);
      CRect rect (0, 0, rectBar.right - rectBar.left, rectBar.bottom - rectBar.top);
      Frame.ClientToScreen (rect);
      Frame.DockControlBar (pToolBar, AFX_IDW_DOCKBAR_TOP, rect);

      // put game toolbar next to it
      Frame.ScreenToClient (rect);
      OffsetRect (rect, 265, 0);

      pToolBar = &Frame.m_wndGameToolBar;

      if (pToolBar)   // assuming we can find it
        {
	      pToolBar->GetWindowRect(&rectBar);
        Frame.ClientToScreen (rect);
        Frame.DockControlBar (pToolBar, AFX_IDW_DOCKBAR_TOP, rect);
        }  // toolbar found

      // float the activity bar
      pToolBar = &Frame.m_wndActivityToolBar;
      if (pToolBar)   // assuming we can find it
        {
        pToolBar->GetWindowRect(&rectBar);
        CPoint point (500, 70);
        Frame.ClientToScreen (&point);
        Frame.FloatControlBar (pToolBar, point, CBRS_ALIGN_LEFT);
        } // toolbar found
      } // toolbar found

    CWelcomeDlg dlg;

    dlg.m_strMessage.Format (Translate ("I notice that this is the first time you have used"
                         " MUSHclient on this PC."));

    dlg.DoModal ();

    db_write_int ("control", "First time", 0);	

    }   // end of first time

  else   // not the first time they have used this program
    
    {

    version = db_get_int ("control", "Version", 0);	

    if (version < THISVERSION)  // THISVERSION is defined at start of this module
      {

      CWelcome1Dlg dlg;         // Welcome to this version dialog

      dlg.m_strMessage1 = TFormat ("Welcome to MUSHclient, version %s", MUSHCLIENT_VERSION);
      dlg.m_strMessage2 = TFormat ("Thank you for upgrading MUSHclient to version %s",
                                MUSHCLIENT_VERSION);
  
      dlg.DoModal ();

      } // end of having lower version than THISVERSION
    
    } // end of not first time


// Write out the new version number if necessary

  if (version != THISVERSION)
    db_write_int ("control", "Version", THISVERSION);	

// Find which version of Windows we are using.

OSVERSIONINFO VersionInformation;

  VersionInformation.dwOSVersionInfoSize = sizeof (VersionInformation);

  GetVersionEx(&VersionInformation);

  platform = VersionInformation.dwPlatformId;

// open activity window if wanted

  if (m_bOpenActivityWindow)
    App.m_pActivityDocTemplate->OpenDocumentFile(NULL);

// activate first world, if any (so activity world doesn't have the focus)

  pMainFrame->SendMessage (WM_COMMAND, ID_WORLDS_WORLD1, 0);

  if (firsttime && !bWine)
    App.WinHelp(ID_GETTING_STARTED + HID_BASE_COMMAND);	

  App.ShowTipAtStartup();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
  protected:

 	// static controls with hyperlinks
	CStaticLink	m_EmailLink;
	CStaticLink	m_WebLink;
	CStaticLink	m_ChangesLink;
	CStaticLink	m_RegcodeLink;

public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_strVersion;
	CString	m_strEmail;
	CString	m_strWebAddress;
	CString	m_strChangeHistoryAddress;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCredits();
	afx_msg void OnLicense();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_strVersion = _T("");
	m_strEmail = _T("");
	m_strWebAddress = _T("");
	m_strChangeHistoryAddress = _T("");
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_VERSION_LABEL, m_strVersion);
	DDX_Text(pDX, IDC_EMAIL_ADDRESS, m_strEmail);
	DDX_Text(pDX, IDC_WEB_ADDRESS, m_strWebAddress);
	DDX_Text(pDX, IDC_CHANGES_ADDRESS, m_strChangeHistoryAddress);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_BN_CLICKED(IDC_CREDITS, OnCredits)
	ON_BN_CLICKED(IDC_LICENSE, OnLicense)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CMUSHclientApp::OnAppAbout()
{
	static CAboutDlg aboutDlg;

  aboutDlg.m_strVersion = "Version ";
  aboutDlg.m_strVersion += MUSHCLIENT_VERSION;

  aboutDlg.m_strWebAddress = MY_WEB_PAGE;
  aboutDlg.m_strChangeHistoryAddress = CHANGES_WEB_PAGE;

  aboutDlg.DoModal();
}


BOOL CAboutDlg::OnInitDialog() 
{
	
	// subclass static controls.
	m_WebLink.SubclassDlgItem(IDC_WEB_ADDRESS, this);
	m_ChangesLink.SubclassDlgItem(IDC_CHANGES_ADDRESS,  this);
	m_RegcodeLink.SubclassDlgItem(IDC_REGCODE,  this);
	

	return CDialog::OnInitDialog();

 }

/////////////////////////////////////////////////////////////////////////////
// CMUSHclientApp commands

extern int gdoccount;

BOOL CMUSHclientApp::SaveAllModified() 
{

// warn them, if they have sessions open.
  
  if (gdoccount > 0 && App.m_bConfirmBeforeClosingMushclient)
    if (::TMessageBox ("This will end your MUSHclient session.", 
          MB_OKCANCEL | MB_ICONINFORMATION)
          == IDCANCEL)
      return 0;
	
	return CWinApp::SaveAllModified();
}

BOOL CMUSHclientApp::OnIdle(LONG lCount) 
{
	
	if (CWinApp::OnIdle(lCount))
    return 1;

CWnd* wnd = Frame.GetForegroundWindow( );

  if (!wnd)
    return 0;

// update activity window if required

  if (m_bUpdateActivity)
    {
    if (Frame.m_wndMDITabs.InUse ())
      Frame.m_wndMDITabs.Update ();

    if (m_pActivityDoc && 
        m_nActivityWindowRefreshType != eRefreshPeriodically)
          m_pActivityDoc->UpdateAllViews (NULL);
    else
      App.UpdateWorldCounts ();
    }

// See if the front window is our main frame

	if (wnd->IsKindOf(RUNTIME_CLASS(CMainFrame)))
    {
    CMainFrame * frame = (CMainFrame *) wnd;

// find the active view

    CFrameWnd*  pFrame = frame->GetActiveFrame( );

// see if the active view is a MUSHview (output window)

    if (!pFrame)
      return 0;
                
  	if (pFrame->IsKindOf(RUNTIME_CLASS(CChildFrame)))
    	{

      CChildFrame * pmyFrame = (CChildFrame *) pFrame;

  		CMUSHView * pmyView = pmyFrame->m_topview;

      if (!pmyView)
        return 0;

// if so, see if mouse is still down

  		return pmyView->mouse_still_down ();
  	  }	

    }

  
  return 0;   // no more idle processing

}

void CMUSHclientApp::OnGameMinimiseprogram() 
{
  ASSERT(AfxGetMainWnd () != NULL);
  AfxGetMainWnd ()->ShowWindow(SW_MINIMIZE);		
}

BOOL CMUSHclientApp::PreTranslateMessage(MSG* pMsg)
{
	// CG: The following lines were added by the Splash Screen component.
	if (CSplashWnd::PreTranslateAppMessage(pMsg))
		return TRUE;

	return CWinApp::PreTranslateMessage(pMsg);
}

void CMUSHclientApp::ShowTipAtStartup(void)
{
	// CG: This function added by 'Tip of the Day' component.

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (cmdInfo.m_bShowSplash)
	{
		CTipDlg dlg;
		if (dlg.m_bStartup)
			dlg.DoModal();
	}

}

void CMUSHclientApp::ShowTipOfTheDay(void)
{
	// CG: This function added by 'Tip of the Day' component.

	CTipDlg dlg;
	dlg.DoModal();

}


// saves column width and location for a generic list control

void CMUSHclientApp::SaveColumnConfiguration (LPCTSTR strName, 
                                              const int iColCount,
                                              CListCtrl& ctlList,
                                              const int iLastColumn,
                                              const BOOL bReverse)
  {

int * iColOrder;

  iColOrder = new int [iColCount];

  if (!iColOrder)
    return;

// set up default order in case our SendMessage doesn't work

  for (int i = 0; i < iColCount; i++)
    iColOrder [i] = i;

// find column sequence

  ctlList.SendMessage (LVM_GETCOLUMNORDERARRAY, iColCount, (DWORD) iColOrder);

// save column sequence and column width

  CString strTitle;

  db_execute ("BEGIN TRANSACTION", true);

  for (int nCol = 0; nCol < iColCount; nCol++)
    {
    strTitle.Format ("%s Col %i Width", strName, nCol);
    App.db_write_int ("control", strTitle, 
                          ctlList.GetColumnWidth (nCol));	

    strTitle.Format ("%s Col %i Order", strName, nCol);
    App.db_write_int ("control", strTitle, 
                        iColOrder [nCol]);	
    } // end of doing each column


  delete [] iColOrder;

  // what column they sorted on
  strTitle.Format ("%s Sort Sequence", strName);
  App.db_write_int ("control", strTitle, iLastColumn);	

  // was it in reverse?
  strTitle.Format ("%s Sort Reverse", strName);
  App.db_write_int ("control", strTitle, bReverse);	

  db_execute ("COMMIT", true);

  } // end of SaveColumnConfiguration

void CMUSHclientApp::RestoreColumnConfiguration (LPCTSTR strName, 
                                                 const int iColCount,
                                                 CListCtrl& ctlList,
                                                 int iColOrder [],
                                                 int iColWidth [],
                                                 int & iLastColumn,
                                                 BOOL & bReverse)
  {

// set list control to display whole row when selected and to allow column drag/drop


  ctlList.SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, 0, 
                        ctlList.SendMessage (LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) 
                        | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP 
                        | (App.m_bShowGridLinesInListViews ? LVS_EX_GRIDLINES : 0) );

// Restore widths and positions of each column

  CString strTitle;

// restore column sequence and column width

  for (int nCol = 0; nCol < iColCount; nCol++)
    {
    strTitle.Format ("%s Col %i Width", strName, nCol);
    iColWidth [nCol] = App.db_get_int ("control", strTitle, iColWidth [nCol]);
    strTitle.Format ("%s Col %i Order", strName, nCol);
    iColOrder [nCol] = App.db_get_int ("control", strTitle, iColOrder [nCol]);

    if (iColWidth [nCol] == 0)
      iColWidth [nCol] = 10;   // just in case

    ctlList.SetColumnWidth (nCol, iColWidth [nCol]);
    } // end of doing each column

  // what column they sorted on
  strTitle.Format ("%s Sort Sequence", strName);
  iLastColumn = App.db_get_int ("control", strTitle, iLastColumn);

  // was it in reverse?
  strTitle.Format ("%s Sort Reverse", strName);
  bReverse = App.db_get_int ("control", strTitle, bReverse);	

  } // end of RestoreColumnConfiguration

int CMUSHclientApp::ExitInstance() 
{
  if (!bWine)
    CoUninitialize ();
	

// delete Element map

  DELETE_MAP (m_ElementMap, CAtomicElement); 


// delete Colours map

  DELETE_MAP (m_ColoursMap, CColours); 


  // close spell checker Lua state
  if (m_SpellChecker_Lua)
    lua_close (m_SpellChecker_Lua);
  m_SpellChecker_Lua = NULL;

  // close i18n Lua state
  if (m_Translator_Lua)
    lua_close (m_Translator_Lua);
  m_Translator_Lua = NULL;

  // release sound buffer if allocated
  if (m_pDirectSoundPrimaryBuffer)
    m_pDirectSoundPrimaryBuffer->Release ();

  // close SQLite database
  if (db)
    sqlite3_close(db);

  // free the resources DLL
  FreeLibrary (m_hInstDLL);
	return CWinApp::ExitInstance();
}


void CMUSHclientApp::UpdateWorldCounts(void) 
  {
POSITION pos;
int nDocCount;


  App.m_bUpdateActivity = FALSE;

  App.m_timeLastActivityUpdate = CTime::GetCurrentTime();

// first, count worlds

 	pos = App.m_pWorldDocTemplate->GetFirstDocPosition();
  for (nDocCount = 0; pos; nDocCount++)
    {
    CMUSHclientDoc* pDoc = (CMUSHclientDoc*) App.m_pWorldDocTemplate->GetNextDoc(pos);
    pDoc->m_view_number = nDocCount + 1;    // so we can use Ctrl+1 etc.
    }

  }   // end of UpdateWorldCounts


void CAboutDlg::OnCredits() 
{
CCreditsDlg dlg;

  dlg.m_iResourceID = IDR_CREDITS;
  dlg.m_strTitle = "Credits";

  dlg.DoModal ();	
	
}

void CMUSHclientApp::OnConnectionQuickConnect() 
{
  // switch to quick connect
  m_TypeOfNewDocument = eQuickConnect;
  App.m_pWorldDocTemplate->OpenDocumentFile(NULL);
  // back to normal
  m_TypeOfNewDocument = eNormalNewDocument;
}

void CMUSHclientApp::OnFileNew() 
{
  m_TypeOfNewDocument = eNormalNewDocument;
  App.m_pWorldDocTemplate->OpenDocumentFile(NULL);
}

void CMUSHclientApp::OnHelpGettingstarted() 
{
App.WinHelp(ID_GETTING_STARTED + HID_BASE_COMMAND);	
}

bool GetSelection (CEdit * pEdit, CString & strSelection)
  {
int nStartChar,
    nEndChar;

  // find the selection range
  pEdit->GetSel(nStartChar, nEndChar);
  // get window text
  pEdit->GetWindowText (strSelection);

  // get selection unless no selection in which case take all of it
  if (nEndChar > nStartChar)
    {
    strSelection = strSelection.Mid (nStartChar, nEndChar - nStartChar);
    return false;
    }

  return true;    // all was selected

  } // end of GetSelection


bool CMUSHclientApp::SpellCheck (CWnd * pWnd, CEdit * pEdit)
  {
  if (!m_bSpellCheckOK)
    return true;

  if (m_SpellChecker_Lua)
    {

    lua_settop(m_SpellChecker_Lua, 0);   // clear stack

    lua_getglobal (m_SpellChecker_Lua, SPELLCHECKFUNCTION);  
    if (!lua_isfunction (m_SpellChecker_Lua, -1))
      return true;  // assume ok, what can we do?

    CString strText;
    bool bAll = GetSelection (pEdit, strText);

    lua_pushstring (m_SpellChecker_Lua, strText);  // string to be checked
    lua_pushboolean (m_SpellChecker_Lua, bAll);    // doing all?

    int narg = lua_gettop(m_SpellChecker_Lua) - 1;  // all but the function
    int error = CallLuaWithTraceBack (m_SpellChecker_Lua, narg, 1);
    
    if (error)
      {
      LuaError (m_SpellChecker_Lua, "Run-time error", SPELLCHECKFUNCTION, "Command-line spell-check");
      lua_close (m_SpellChecker_Lua);
      m_SpellChecker_Lua = NULL;
      m_bSpellCheckOK = false;
      return true;    
      }  

    if (lua_isstring (m_SpellChecker_Lua, -1))
      {
      const char * p = lua_tostring (m_SpellChecker_Lua, -1);
      if (bAll)
        pEdit->SetSel (0, -1, TRUE);
      pEdit->ReplaceSel (p, true);
      return true;
      }

    return false;
    }



  return true;

     
  }


LUALIB_API int luaopen_progress_dialog(lua_State *L);

void CMUSHclientApp::OpenLuaDelayed ()
  {
  if (m_SpellChecker_Lua)
    lua_close (m_SpellChecker_Lua);   // close previous one

  m_SpellChecker_Lua = MakeLuaState();   /* opens Lua */
  if (!m_SpellChecker_Lua)
    return;         // can't open Lua

  luaL_openlibs (m_SpellChecker_Lua);    // new way of opening all libraries
  luaopen_rex (m_SpellChecker_Lua);      // regular expression library
  luaopen_bits (m_SpellChecker_Lua);     // bit manipulation library
  luaopen_compress (m_SpellChecker_Lua); // compression (utils) library
  luaopen_progress_dialog (m_SpellChecker_Lua);    // progress dialog
  luaopen_bc (m_SpellChecker_Lua);   // open bc library 
  luaopen_lsqlite3 (m_SpellChecker_Lua);   // open sqlite library
  lua_pushcfunction(m_SpellChecker_Lua, luaopen_lpeg);   // open lpeg library
  lua_call(m_SpellChecker_Lua, 0, 0);

  // add xml reader to utils lib
  luaL_register (m_SpellChecker_Lua, "utils", ptr_xmllib);

  lua_settop(m_SpellChecker_Lua, 0);   // clear stack

  // unless they explicitly enable it, remove ability to load DLLs
  DisableDLLs (m_SpellChecker_Lua);

  lua_settop(m_SpellChecker_Lua, 0);   // clear stack

   // spell checker folder pathname
  CString luaspellcheckerpath =  ExtractDirectory (m_strMUSHclientFileName);
  luaspellcheckerpath += "spellchecker.lua";

  int error = luaL_loadfile(m_SpellChecker_Lua, luaspellcheckerpath) ||
        lua_pcall (m_SpellChecker_Lua, 0, 0, 0);
  if (error)
      {
      LuaError (m_SpellChecker_Lua);
      lua_close (m_SpellChecker_Lua);
      m_SpellChecker_Lua = NULL;
      return;   // don't press on,  m_SpellChecker_Lua is NULL now.
      }  

  lua_settop(m_SpellChecker_Lua, 0);   // clear stack

  lua_getglobal (m_SpellChecker_Lua, SPELLCHECKFUNCTION);  
  if (!lua_isfunction (m_SpellChecker_Lua, -1))
    {
    ::UMessageBox (TFormat ("Function '%s' not in spellchecker.lua file",
                     SPELLCHECKFUNCTION),  MB_OK  | MB_ICONSTOP);
    lua_close (m_SpellChecker_Lua);
    m_SpellChecker_Lua = NULL;
    }

  } // end of OpenLuaDelayed



void CMUSHclientApp::InitSpellCheck (void)
  {


   OpenLuaDelayed ();
   if (m_SpellChecker_Lua)
     m_bSpellCheckOK = true;
   else
     m_bSpellCheckOK = false; 




  }   // end of InitSpellCheck


// have to put error message here because of __try block
void zLibError (const int izError, z_stream & m_zCompress)
  {

  if (m_zCompress.msg)
    ::UMessageBox (TFormat ("Could not initialise zlib decompression engine: %s",
                      m_zCompress.msg), MB_ICONEXCLAMATION);
  else
    ::UMessageBox (TFormat ("Could not initialise zlib decompression engine: %i",
                      izError), MB_ICONEXCLAMATION);

  }  // end of zLibError


// initialise zlib - allow for delay-load DLL not loading
int InitZlib (z_stream & m_zCompress)
  {
bool bCompressInitOK = false; 

  int izError;
  if ((izError = inflateInit (&m_zCompress)) == Z_OK)
    bCompressInitOK = true; 
  else
    zLibError (izError, m_zCompress);

  return  bCompressInitOK;
 }   // end of InitZlib


CString CMUSHclientApp::GetUniqueString (void)
  {
  CString strNumber;
  strNumber.Format ("%I64d", GetUniqueNumber ());
  return strNumber;
  }   // end of CMUSHclientApp::GetUniqueString


void CAboutDlg::OnLicense() 
{
CCreditsDlg dlg;

  dlg.m_iResourceID = IDR_LICENSE;
  dlg.m_strTitle = "License Agreement";

  dlg.DoModal ();	
	
}

void CMUSHclientApp::OnEditColourpicker() 
{
CColourPickerDlg dlg;

  if (dlg.DoModal () != IDOK)
    return;

//  putontoclipboard (dlg.m_strMXPname);
  

}

        /*
STDMETHODIMP_(ULONG) CMUSHclientApp::XLocalClass::AddRef()
{
  METHOD_PROLOGUE(CMUSHclientApp, LocalClass)
  return pThis->ExternalAddRef();
}
STDMETHODIMP_(ULONG) CMUSHclientApp::XLocalClass::Release()
{
  METHOD_PROLOGUE(CMUSHclientApp, LocalClass)
  return pThis->ExternalRelease();
}
STDMETHODIMP CMUSHclientApp::XLocalClass::QueryInterface(
  REFIID iid, LPVOID* ppvObj)
{
  METHOD_PROLOGUE(CMUSHclientApp, LocalClass)
  return pThis->ExternalQueryInterface(&iid, ppvObj);
}
STDMETHODIMP CMUSHclientApp::XLocalClass::GetTypeInfoCount(
    UINT FAR* pctinfo)
{
  METHOD_PROLOGUE(CMUSHclientApp, LocalClass)
  LPDISPATCH lpDispatch = pThis->GetIDispatch(FALSE);
  ASSERT(lpDispatch != NULL);
  return lpDispatch->GetTypeInfoCount(pctinfo);
}
STDMETHODIMP CMUSHclientApp::XLocalClass::GetTypeInfo(
  UINT itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
  METHOD_PROLOGUE(CMUSHclientApp, LocalClass)
  LPDISPATCH lpDispatch = pThis->GetIDispatch(FALSE);
  ASSERT(lpDispatch != NULL);
  return lpDispatch->GetTypeInfo(itinfo, lcid, pptinfo);
}
STDMETHODIMP CMUSHclientApp::XLocalClass::GetIDsOfNames(
  REFIID riid, OLECHAR FAR* FAR* rgszNames, UINT cNames,
  LCID lcid, DISPID FAR* rgdispid) 
{
  METHOD_PROLOGUE(CMUSHclientApp, LocalClass)
  LPDISPATCH lpDispatch = pThis->GetIDispatch(FALSE);
  ASSERT(lpDispatch != NULL);
  return lpDispatch->GetIDsOfNames(riid, rgszNames, cNames, 
    lcid, rgdispid);
}
STDMETHODIMP CMUSHclientApp::XLocalClass::Invoke(
  DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
  DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult,
  EXCEPINFO FAR* pexcepinfo, UINT FAR* puArgErr)
{
  METHOD_PROLOGUE(CMUSHclientApp, LocalClass)
  LPDISPATCH lpDispatch = pThis->GetIDispatch(FALSE);
  ASSERT(lpDispatch != NULL);
  return lpDispatch->Invoke(dispidMember, riid, lcid,
    wFlags, pdispparams, pvarResult,
    pexcepinfo, puArgErr);
}

STDMETHODIMP CMUSHclientApp::XLocalClass::ShowWindow()
{
    METHOD_PROLOGUE(CMUSHclientApp, LocalClass)
    pThis->ShowWindow();
    return TRUE;
}

  */


void Generate256colours (void)
  {
  int red, green, blue;
//  const BYTE values [6] = { 0x00, 0x5F, 0x87, 0xAF, 0xD7, 0xFF };

  xterm_256_colours   [0] = RGB (0, 0, 0);         // (black)       
  xterm_256_colours   [1] = RGB (128, 0, 0);       // (maroon)      
  xterm_256_colours   [2] = RGB (0, 128, 0);       // (green)       
  xterm_256_colours   [3] = RGB (128, 128, 0);     // (olive)       
  xterm_256_colours   [4] = RGB (0, 0, 128);       // (navy)        
  xterm_256_colours   [5] = RGB (128, 0, 128);     // (purple)      
  xterm_256_colours   [6] = RGB (0, 128, 128);     // (teal)        
  xterm_256_colours   [7] = RGB (192, 192, 192);   // (silver)      
                       
  xterm_256_colours   [8]  = RGB (128, 128, 128);  // (gray)                 
  xterm_256_colours   [9]  = RGB (255, 0, 0);      // (red)               
  xterm_256_colours   [10] = RGB (0, 255, 0);      // (lime)              
  xterm_256_colours   [11] = RGB (255, 255, 0);    // (yellow)            
  xterm_256_colours   [12] = RGB (0, 0, 255);      // (blue)              
  xterm_256_colours   [13] = RGB (255, 0, 255);    // (magenta)  
  xterm_256_colours   [14] = RGB (0, 255, 255);    // (cyan)        
  xterm_256_colours   [15] = RGB (255, 255, 255);  // (white)             

  /*
  for (red = 0; red < 6; red++)
    for (green = 0; green < 6; green++)
      for (blue = 0; blue < 6; blue++)
         xterm_256_colours [16 + (red * 36) + (blue * 6) + green] =
            RGB (values [red], values [blue], values [green]);
  */

// alternate algorithm - gives even 6x6x6 colour cube
//  there are 6 colours in the cube, but only 5 gaps:
//   Thus the colours will be: 0x00 / 0x33 / 0x66 / 0x99 / 0xCC / 0xFF

const BYTE colour_increment = 255 / 5;     // that is, 51 (0x33)

  for (red = 0; red < 6; red++)
      for (green = 0; green < 6; green++)
        for (blue = 0; blue < 6; blue++)
           xterm_256_colours [16 + (red * 36) + (blue * 6) + green] =
              RGB (red   * colour_increment, 
                   blue  * colour_increment, 
                   green * colour_increment);
  

  int grey;
  for (grey = 0; grey < 24; grey++)
    {
    BYTE value = 8 + (grey * 10);
    xterm_256_colours [232 + grey] = RGB (value, value, value);
    }

  } // end of Generate256colours


// file names might be: (MUSHclient executable)\locale\EN.dll  - resources
//                      (MUSHclient executable)\locale\EN.lua  - localization strings

BOOL CMUSHclientApp::I18N_Setup (void)  // internationalization
  {
long iScreenX = GetSystemMetrics (SM_CXSCREEN);
long iScreenY = GetSystemMetrics (SM_CYSCREEN);
bool bSmallScreen = (iScreenX < 1024) || (iScreenY < 768);

  m_Translator_Lua = NULL;

  m_hInstDLL = NULL;

  char localeBuf [10];

  // find 2-character country ID - default
  GetLocaleInfo (GetUserDefaultLCID (), LOCALE_SABBREVLANGNAME, localeBuf, sizeof (localeBuf));
  localeBuf [2] = 0;  // truncate to 2 characters

  // see if different in registry
  m_strLocale  = db_get_string  ("prefs", "Locale", localeBuf);

  // executable directory
  m_strTranslatorFile = ExtractDirectory (App.m_strMUSHclientFileName);

  // locale subdirectory
  m_strTranslatorFile += "locale\\";

  // english resource file
  CString m_strEnglishResourceFile = m_strTranslatorFile + "EN.dll";

  if (bSmallScreen)
    m_strEnglishResourceFile = m_strTranslatorFile + "EN_small.dll";

  // locale-specific file
  m_strTranslatorFile += m_strLocale;     // eg. EN

  // resource file has same prefix
  CString m_strResourceFile = m_strTranslatorFile;

  // translator file is Lua
  m_strTranslatorFile += ".lua";

  if (bSmallScreen)
    m_strResourceFile += "_small";

  // resource file is DLL
  m_strResourceFile += ".dll";

  // try non-English one, if not EN locale
  if (m_strEnglishResourceFile.CompareNoCase (m_strResourceFile) != 0)
    {
    m_hInstDLL = ::LoadLibrary(m_strResourceFile);

    if (m_hInstDLL == NULL)
      {
      CString strMessage = "Failed to load resources file: ";
      strMessage += m_strResourceFile;
      strMessage += " - trying English file";

      ::MessageBox (NULL, strMessage, "MUSHclient", MB_ICONINFORMATION); 
      }
    }

  // not found? try English
  if (m_hInstDLL == NULL)
    {
    m_hInstDLL = ::LoadLibrary(m_strEnglishResourceFile);

    // better default to English next time
    if (m_hInstDLL != NULL) 
      db_write_string  ("prefs", "Locale", "EN");

    }

  // still no good? error message and give up
  if (m_hInstDLL == NULL) 
    { 

    CString strMessage = "Failed to load resources file: ";
    strMessage += m_strEnglishResourceFile;

    ::MessageBox (NULL, strMessage, "MUSHclient", MB_ICONSTOP); 
    return FALSE; // failed to load the localized resources

    } 

  // use these resources
  AfxSetResourceHandle(m_hInstDLL); // get resources from the DLL

  // see if translator file present - if so, load it
  CFileStatus	status;
  if (CFile::GetStatus(m_strTranslatorFile, status))
    {

    m_Translator_Lua = MakeLuaState();   /* opens Lua */
    if (!m_Translator_Lua)
      {
      ::MessageBox (NULL, "Lua (i18n) initialization failed", "MUSHclient", MB_ICONSTOP); // DON'T TRANSLATE
      return FALSE;                     // can't open Lua
      }

    luaL_openlibs (m_Translator_Lua);    // new way of opening all libraries

    luaopen_rex (m_Translator_Lua);      // regular expression library
    luaopen_bits (m_Translator_Lua);     // bit manipulation library
    luaopen_compress (m_Translator_Lua); // compression (utils) library
    luaopen_bc (m_Translator_Lua);   // open bc library   
    luaopen_lsqlite3 (m_Translator_Lua);   // open sqlite library
    lua_pushcfunction(m_Translator_Lua, luaopen_lpeg);   // open lpeg library
    lua_call(m_Translator_Lua, 0, 0);

    // add xml reader (and other stuff) to utils lib
    luaL_register (m_Translator_Lua, "utils", ptr_xmllib);
  
    m_bEnablePackageLibrary = false;  // force to false for the sandboxing

    DisableDLLs (m_Translator_Lua);

    lua_settop(m_Translator_Lua, 0);   // clear stack

    const char * lua_sandbox = 
      // only allow safe os functions
      " os = { "
      "   date = os.date, "
      "   time = os.time, "
      "   setlocale = os.setlocale, "
      "   clock = os.clock, "
      "   difftime = os.difftime, "
      "   exit = os.exit,  "   // not really implemented but we have nice error message 
      "  }  "
      // no io calls
      " io = nil "
      "";

    // sandbox it
    if (luaL_loadbuffer (m_Translator_Lua, lua_sandbox, strlen (lua_sandbox), "sandbox") ||
        lua_pcall (m_Translator_Lua, 0, 0, 0) ||

    // .. this is still part of the 'if' statement ...

    // process translator file into Lua space
    // sample file name: (application directory)\locale\EN.lua

    // now load localizing file
        luaL_loadfile(m_Translator_Lua, m_strTranslatorFile) ||
        lua_pcall (m_Translator_Lua, 0, 0, 0))
          {
          LuaError (m_Translator_Lua, "Localization initialization");
          lua_close (m_Translator_Lua);
          m_Translator_Lua = NULL;
          }

    }   // end of localization file exists

  return TRUE;
  } // end of void CMUSHclientApp::I18N_Setup

/* 

  Colour cube printer in Lua:

for i = 0, 255 do
  if i == 16 then
    print ""
  end -- if

  Tell (string.format (" %3i: ", i))
  ColourTell ("black", RGBColourToName (extended_colours [i]), "    ")

  if i < 16 then
    if (i % 8) == 7 then
      print ""
    end -- if 
  else
    if ((i - 16) % 6) == 5 then
      print ""
    end -- if 
  end -- if

  if i > 16 then

    if ((i - 16) % 36) == 35 then
      print ""
    end -- if
  end -- if > 16

end -- for

*/


bool CMUSHclientApp::AppendToTheNotepad (const CString strTitle,
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
                      NULL,   // document
                      0,      // document number
                      App.m_strDefaultInputFont,
                      App.m_iDefaultInputFontHeight,
                      App.m_iDefaultInputFontWeight,
                      App.m_iDefaultOutputFontCharset,
                      RGB (0, 0, 0),
                      RGB (255, 255, 255),
                      "",     // search string
                      "",       // line preamble
                      false,
                      false,
                      false,
                      false,  
                      false,
                      false,
                      iNotepadType
                      );

  return bOK;
  }   // end of CMUSHclientApp::AppendToTheNotepad


CTextDocument * CMUSHclientApp::FindNotepad (const CString strTitle)
  {
CTextDocument * pTextDoc = NULL;

  for (POSITION docPos = App.m_pNormalDocTemplate->GetFirstDocPosition();
      docPos != NULL; )
    {
    pTextDoc = (CTextDocument *) App.m_pWorldDocTemplate->GetNextDoc(docPos);

    // ignore related worlds
    if (pTextDoc->m_pRelatedWorld == NULL &&
       pTextDoc->m_strTitle.CompareNoCase (strTitle) == 0)
      return pTextDoc;      // right title, not attached to a world

    } // end of doing each document


  return NULL;    // not found
  }  // end of CMUSHclientApp::FindNotepad

BOOL CMUSHclientApp::ActivateNotepad(LPCTSTR Title) 
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
}   // end of CMUSHclientApp::ActivateNotepad


// simply query which returns a single column from the first row
// -- useful for stuff like getting a single preference value
int CMUSHclientApp::db_simple_query (const char * sql, 
                                     string & result, 
                                     const bool bShowError, 
                                     const char * default_value)
  {
  if (!db)
    {
    result = default_value;
    return SQLITE_CANTOPEN;
    }

  sqlite3_stmt *pStmt;
  const char *pzTail;

  int rc;

  // ensure results empty if nothing there
  result.erase ();

  // prepare the SQL
  rc = sqlite3_prepare_v2 (db, sql, -1, &pStmt, &pzTail);

  if (rc != SQLITE_OK)
    {
    if (bShowError)
      db_show_error (sql);
    return rc;
    }

  // step into first row
  rc = sqlite3_step (pStmt);

  // should be SQLITE_DONE (no rows), or SQLITE_ROW (one or more rows)

  if (rc == SQLITE_DONE)        // if not row, take the default value
    result = default_value;
  else 
    {

    // error if not a row
    if (rc != SQLITE_ROW)
      {
      if (bShowError)
        db_show_error (sql);
      sqlite3_finalize(pStmt);  // finished with statement
      return rc;
      }

    const unsigned char *p = sqlite3_column_text(pStmt, 0);
                     
    result = (const char *) p;;
    } // we have a row

  // done with prepared statement
  sqlite3_finalize(pStmt);  // finished with statement
  return SQLITE_OK;
  }   // end of CMUSHclientApp::db_simple_query

// replaces: GetProfileString 
CString CMUSHclientApp::db_get_string (LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault)
  {

  string db_value;

  CString strEntry = lpszEntry;

  strEntry.Replace ("'", "''");  // fix up quotes

  db_simple_query ((LPCTSTR) CFormat (
        "SELECT value FROM %s WHERE name = '%s'", 
         lpszSection,
         (LPCTSTR) strEntry), 
        db_value, 
        true,
        lpszDefault);


  return db_value.c_str ();
  }   // end of CMUSHclientApp::db_get_string 

// replaces: GetProfileInt 
int CMUSHclientApp::db_get_int (LPCTSTR lpszSection, LPCTSTR lpszEntry, int iDefault)
  {
  return atoi (db_get_string (lpszSection, lpszEntry, (LPCTSTR) CFormat ("%i", iDefault)));
  }

  // execute an SQL statement against the database
int CMUSHclientApp::db_execute (const char * sql, const bool bShowError)
  {

  if (!db)
    return SQLITE_CANTOPEN;

  sqlite3_stmt *pStmt;
  const char *pzHead = sql;
  const char *pzTail;

  int rc;

  // loop until we run out of SQL statements (separated by semicolons)
  while (*pzHead) 
    {
    // prepare the SQL
    rc = sqlite3_prepare_v2 (db, pzHead, -1, &pStmt, &pzTail);

    if (rc != SQLITE_OK)
      {
      if (bShowError)
        db_show_error (sql);
      return rc;
      }

    // execute the SQL
    rc = sqlite3_step (pStmt);

    // error if not done
    if (rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW)
      {
      if (bShowError)
        db_show_error (sql);
      sqlite3_finalize(pStmt);  // finished with statement
      return rc;
      }

    // done with the statement
    sqlite3_finalize(pStmt);  // finished with statement

    pzHead = pzTail;

    while (*pzHead == ' ')
      pzHead++;  // skip spaces

    }

  return SQLITE_OK;
  }   // end of CMUSHclientApp::db_execute


// replaces: WriteProfileString
int CMUSHclientApp::db_write_string (LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
  {

  if (!db)
    return SQLITE_ERROR;

  CString strEntry = lpszEntry;
  CString strValue = lpszValue;


  strEntry.Replace ("'", "''");  // fix up quotes
  strValue.Replace ("'", "''");  // fix up quotes

  int db_rc;

  db_rc = db_execute ((LPCTSTR) CFormat ("UPDATE %s SET value = '%s' WHERE name = '%s'",
                        lpszSection, 
                        (LPCTSTR) strValue, 
                        (LPCTSTR) strEntry), 
              true);


  if (db_rc != SQLITE_OK)
    return db_rc;

  int count = sqlite3_changes(db);

  // if count is zero, row does not exist, so we will add it

  if (count == 0)
    db_rc = db_execute ((LPCTSTR) CFormat ("INSERT INTO %s (name, value) VALUES ('%s', '%s')",
                lpszSection,
                (LPCTSTR) strEntry, 
                (LPCTSTR) strValue), 
                true);

  return db_rc;

  }  // end of CMUSHclientApp::db_write_string 


// replaces: WriteProfileInt 
int CMUSHclientApp::db_write_int (LPCTSTR lpszSection, LPCTSTR lpszEntry, int iValue)
  {
  return db_write_string (lpszSection, lpszEntry, (LPCTSTR) CFormat ("%i", iValue));
  }



void CMUSHclientApp::db_show_error (const char * sql)
  {
  if (!db)
   return;

  CString  strTitle = "SQL errors in global preferences";

  AppendToTheNotepad (strTitle, 
                      CFormat ("SQL error on statement:\r\n\"%s\"\r\n%s\r\n", sql, sqlite3_errmsg(db)),             
                      false,   // append
                      eNotepadWorldLoadError);

  // make sure they see it
  ActivateNotepad (strTitle);

  }  // end of CMUSHclientApp::db_show_error


static int __stdcall EnumFontFamExProc (CONST LOGFONTA * lpelfe,   // pointer to logical-font data
                                 CONST TEXTMETRICA * lpntme,// pointer to physical-font data
                                 DWORD FontType,            // type of font
                                 LPARAM pUser)            // application-defined data
                                                            
  {

  string sFont = lpelfe->lfFaceName;
  CString strFont = sFont.c_str ();
  strFont.MakeLower ();

  // Use either of these for preference
  if (strFont == "bitstream vera sans mono" || strFont == "fixedsys")
    {
    App.m_strFixedPitchFont = sFont.c_str ();
    return 0;       // stop evaluating, we are happy now
    }

  // otherwise choose some fixed-pitch font and keep looking
  App.m_strFixedPitchFont = sFont.c_str ();

  return 1;   // keep enumerating
  } // end of EnumFontFamExProc


// try to pick a default font that actually exists on their system
void CMUSHclientApp::WorkOutFixedFont ()
  {

  // in case of error dialogs, etc.
  m_strFixedPitchFont = "FixedSys";
  m_iFixedPitchFontSize = 9;

  CDC dc;

  dc.CreateCompatibleDC (NULL);

  LOGFONT lf;

  ZeroMemory (&lf, sizeof lf);

  lf.lfCharSet = DEFAULT_CHARSET;
  lf.lfFaceName [0] = 0;
  lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;

  EnumFontFamiliesEx (dc.m_hDC,           // handle to device context
                      &lf,                // pointer to logical font information
                      EnumFontFamExProc,  // pointer to callback function
                      0,                  // application-supplied data
                      0);                 // reserved; must be zero


  } // end of CMUSHclientApp::WorkOutFixedFont
