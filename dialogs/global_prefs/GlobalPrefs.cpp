// CGlobalPrefs.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "GlobalPrefsSheet.h"
#include "GlobalPrefs.h"
#include "..\..\mainfrm.h"
#include "..\..\childfrm.h"
#include "..\..\doc.h"
#include "..\ColourPickerDlg.h"
#include "..\EditMultiLine.h"

#include <shlobj.h>
#include <cderr.h>

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern int ActivityToolBarResourceNames [6];

// The field below is needed to initialise the browse directory dialog 
// with the initial directory

CString strStartingDirectory;

// function prototypes needed for folder browsing

int __stdcall BrowseCallbackProc(
    HWND hwnd, 	
    UINT uMsg, 	
    LPARAM lParam, 	
    LPARAM lpData	
   );

void RemoveTrailingBackslash (CString& str);

// working directory at login time

extern char working_dir [_MAX_PATH];


IMPLEMENT_DYNCREATE(CGlobalPrefsP1, CPropertyPage)
IMPLEMENT_DYNCREATE(CGlobalPrefsP2, CPropertyPage)
IMPLEMENT_DYNCREATE(CGlobalPrefsP3, CPropertyPage)
IMPLEMENT_DYNCREATE(CGlobalPrefsP4, CPropertyPage)
IMPLEMENT_DYNCREATE(CGlobalPrefsP5, CPropertyPage)
IMPLEMENT_DYNCREATE(CGlobalPrefsP6, CPropertyPage)
IMPLEMENT_DYNCREATE(CGlobalPrefsP7, CPropertyPage)
IMPLEMENT_DYNCREATE(CGlobalPrefsP9, CPropertyPage)
IMPLEMENT_DYNCREATE(CGlobalPrefsP10, CPropertyPage)
IMPLEMENT_DYNCREATE(CGlobalPrefsP11, CPropertyPage)
IMPLEMENT_DYNCREATE(CGlobalPrefsP12, CPropertyPage)
IMPLEMENT_DYNCREATE(CGlobalPrefsP13, CPropertyPage)


/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP1 property page

CGlobalPrefsP1::CGlobalPrefsP1() : CPropertyPage(CGlobalPrefsP1::IDD)
{
	//{{AFX_DATA_INIT(CGlobalPrefsP1)
	m_strDefaultWorldFileDirectory = _T("");
	//}}AFX_DATA_INIT
}

CGlobalPrefsP1::~CGlobalPrefsP1()
{
}

void CGlobalPrefsP1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalPrefsP1)
	DDX_Control(pDX, IDC_SELECTED_WORLD, m_ctlSelectedWorld);
	DDX_Text(pDX, IDC_DEFAULT_DIRECTORY_NAME, m_strDefaultWorldFileDirectory);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGlobalPrefsP1, CPropertyPage)
	//{{AFX_MSG_MAP(CGlobalPrefsP1)
	ON_BN_CLICKED(IDC_NEW_STARTUP_WORLD, OnNewStartupWorld)
	ON_BN_CLICKED(IDC_REMOVE_STARTUP_WORLD, OnRemoveStartupWorld)
	ON_BN_CLICKED(IDC_MOVE_UP, OnMoveUp)
	ON_BN_CLICKED(IDC_MOVE_DOWN, OnMoveDown)
	ON_BN_CLICKED(IDC_DEFAULT_DIRECTORY, OnDefaultDirectory)
	ON_BN_CLICKED(IDC_ADD_CURRENT_WORLD, OnAddCurrentWorld)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_REMOVE_STARTUP_WORLD, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_MOVE_UP, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_MOVE_DOWN, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_ADD_CURRENT_WORLD, OnUpdateNeedCurrentWorld)
  ON_UPDATE_COMMAND_UI(IDC_SELECTED_WORLD, OnUpdateSelectedWorld)
END_MESSAGE_MAP()

void CGlobalPrefsP1::OnNewStartupWorld() 
{
	CFileDialog	dlg(TRUE,
		"mcl",
		NULL,
		OFN_ALLOWMULTISELECT|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,
		"MUSHclient worlds (*.mcl)|*.mcl||",
		this);
	dlg.m_ofn.lpstrTitle = "Add Startup Worlds";
	char	szFileBuffer[4096];
	szFileBuffer[0] = 0;
	dlg.m_ofn.lpstrFile = szFileBuffer;
	dlg.m_ofn.nMaxFile = sizeof(szFileBuffer);
  // use default world file directory
  CString strStartingDirectory;
	GetDlgItemText(IDC_DEFAULT_DIRECTORY_NAME, strStartingDirectory);
  dlg.m_ofn.lpstrInitialDir = strStartingDirectory;

  ChangeToFileBrowsingDirectory ();
	int nResult = dlg.DoModal();
  ChangeToStartupDirectory ();

	if (nResult == IDOK)
	{
		CListBox*	pList = (CListBox*) GetDlgItem(IDC_WORLD_LIST);
		ASSERT_VALID(pList);

		for (POSITION pos = dlg.GetStartPosition(); pos; )
		{
			CString	strFile = dlg.GetNextPathName(pos);

			// If file not already in list
			if (pList->FindStringExact(-1, strFile) == LB_ERR)
				pList->AddString(strFile);
		}

    CString strWorldCount;
    strWorldCount.Format ("%i world%s", 
                          pList->GetCount(),
                          pList->GetCount() == 1 ? "" : "s");

		SetDlgItemText(IDC_WORLD_COUNT, strWorldCount);
	}
	else if (CommDlgExtendedError() == FNERR_BUFFERTOOSMALL)
		// To many files - buffer not large enough
		TMessageBox("You have selected too many worlds to add.  Please try again with fewer worlds.");

	UpdateDialogControls(this, TRUE);
}

void CGlobalPrefsP1::OnRemoveStartupWorld() 
{
	CListBox*	pList = (CListBox*) GetDlgItem(IDC_WORLD_LIST);
	ASSERT_VALID(pList);
	
	int	nIndex = pList->GetCurSel();
	pList->DeleteString(nIndex);

	if (nIndex >= pList->GetCount())
		nIndex = pList->GetCount() - 1;
	pList->SetCurSel(nIndex);

  CString strWorldCount;
  strWorldCount.Format ("%i world%s", 
                        pList->GetCount(),
                        pList->GetCount() == 1 ? "" : "s");

	SetDlgItemText(IDC_WORLD_COUNT, strWorldCount);
	UpdateDialogControls(this, TRUE);
}

LRESULT CGlobalPrefsP1::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CPrefsP1::OnKickIdle

void CGlobalPrefsP1::OnUpdateSelectedWorld(CCmdUI* pCmdUI)
{
	CListBox*	pList = (CListBox*) GetDlgItem(IDC_WORLD_LIST);
  int nIndex = pList->GetCurSel();

  if (nIndex == LB_ERR)
    pCmdUI->SetText ("");
  else
    {
    CString strItem; 
    pList->GetText (nIndex, strItem);
  	pCmdUI->SetText(strItem);
    }
}

void CGlobalPrefsP1::OnUpdateNeedSelection(CCmdUI* pCmdUI)
{
	CListBox*	pList = (CListBox*) GetDlgItem(IDC_WORLD_LIST);
	pCmdUI->Enable(pList && pList->GetCurSel() != LB_ERR);
}

BOOL CGlobalPrefsP1::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	CListBox*	pList = (CListBox*) GetDlgItem(IDC_WORLD_LIST);
	
	CRect	rect(0, 0, _MAX_PATH, 1);
	MapDialogRect(rect);
	pList->SetHorizontalExtent(rect.Width());

// add to list here
  
  while (!m_strWorldList.IsEmpty ())
    {
    int i = m_strWorldList.Find ('*');
    if (i == -1)
      {
      pList->AddString (m_strWorldList);
      break;
      }
    else
      {
      pList->AddString (m_strWorldList.Left (i));
      m_strWorldList = m_strWorldList.Mid (i + 1);
      }
    }

  CString strWorldCount;
  strWorldCount.Format ("%i world%s", 
                        pList->GetCount(),
                        pList->GetCount() == 1 ? "" : "s");

	SetDlgItemText(IDC_WORLD_COUNT, strWorldCount);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGlobalPrefsP1::OnOK() 
{

	CListBox*	pList = (CListBox*) GetDlgItem(IDC_WORLD_LIST);
	
  m_strWorldList.Empty ();
  CString strItem;

// make a long string containing all world names, separated by asterisks

  for (int i = 0; i < pList->GetCount (); i++)
    {
    pList->GetText (i, strItem);
    if (!m_strWorldList.IsEmpty ())
      m_strWorldList += '*';
    m_strWorldList += strItem;
    }

	CPropertyPage::OnOK();
}

void CGlobalPrefsP1::OnMoveUp() 
{
	CListBox*	pList = (CListBox*) GetDlgItem(IDC_WORLD_LIST);
	
  CString strItem;

	int	nIndex = pList->GetCurSel();

  // if no selection, or at top, give up
  if (nIndex == LB_ERR || nIndex == 0)
    return;

  // get item
  pList->GetText (nIndex, strItem);

  // now get rid of it
	pList->DeleteString(nIndex);

  // re-insert it one up

  nIndex = pList->InsertString (nIndex - 1, strItem);

  // re-select it
  pList->SetCurSel (nIndex);
  
}

void CGlobalPrefsP1::OnMoveDown() 
{
	CListBox*	pList = (CListBox*) GetDlgItem(IDC_WORLD_LIST);
	
  CString strItem;

	int	nIndex = pList->GetCurSel();

  // if no selection, or at bottom, give up
  if (nIndex == LB_ERR || nIndex >= (pList->GetCount () - 1))
    return;

  // get item
  pList->GetText (nIndex, strItem);

  // now get rid of it
	pList->DeleteString(nIndex);

  // re-insert it one down
  nIndex = pList->InsertString (nIndex + 1, strItem);

  // re-select it
  pList->SetCurSel (nIndex);
  
}

void CGlobalPrefsP1::OnDefaultDirectory() 
{
	// Gets the Shell's default allocator
	LPMALLOC pMalloc;
	if (::SHGetMalloc(&pMalloc) == NOERROR)
	{
		char	pszBuffer[MAX_PATH];
		BROWSEINFO		bi;
		LPITEMIDLIST	pidl;

        // Get help on BROWSEINFO struct - it's got all the bit settings.
		bi.hwndOwner = GetSafeHwnd();
		bi.pidlRoot = NULL;
		bi.pszDisplayName = pszBuffer;
		bi.lpszTitle = "World files folder";
		bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
    // if possible, let them create one
    if (!bWine)  
	  	bi.ulFlags |= BIF_NEWDIALOGSTYLE | BIF_EDITBOX;     // requires CoInitialize
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = 0;
  	GetDlgItemText(IDC_DEFAULT_DIRECTORY_NAME, strStartingDirectory);

		// This next call issues the dialog box.
		if ((pidl = ::SHBrowseForFolder(&bi)) != NULL)
		{
			if (::SHGetPathFromIDList(pidl, pszBuffer))
				SetDlgItemText(IDC_DEFAULT_DIRECTORY_NAME, pszBuffer);

			// Free the PIDL allocated by SHBrowseForFolder.
			pMalloc->Free(pidl);
		}
		// Release the shell's allocator.
		pMalloc->Release();
    }
	
}


void CGlobalPrefsP1::OnUpdateNeedCurrentWorld(CCmdUI* pCmdUI)
{
// we must have a current document, and it must have a file name
	pCmdUI->Enable (m_pCurrentDoc != NULL &&
                  !m_pCurrentDoc->GetPathName ().IsEmpty ());
}


void CGlobalPrefsP1::OnAddCurrentWorld() 
{
if (!m_pCurrentDoc)
  return;

CString strFile = m_pCurrentDoc->GetPathName ();

if (strFile.IsEmpty ())
  return;

CListBox*	pList = (CListBox*) GetDlgItem(IDC_WORLD_LIST);
ASSERT_VALID(pList);

// If file not already in list
if (pList->FindStringExact(-1, strFile) == LB_ERR)
	pList->AddString(strFile);

CString strWorldCount;
strWorldCount.Format ("%i world%s", 
                      pList->GetCount(),
                      pList->GetCount() == 1 ? "" : "s");

SetDlgItemText(IDC_WORLD_COUNT, strWorldCount);

UpdateDialogControls(this, TRUE);
	
}


/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP2 property page

CGlobalPrefsP2::CGlobalPrefsP2() : CPropertyPage(CGlobalPrefsP2::IDD)
{
	//{{AFX_DATA_INIT(CGlobalPrefsP2)
	m_bAutoConnectWorlds = FALSE;
	m_bReconnectOnLinkFailure = FALSE;
	m_bOpenWorldsMaximised = FALSE;
	m_bNotifyOnDisconnect = FALSE;
	m_bEnableSpellCheck = FALSE;
	m_strWordDelimiters = _T("");
	m_strWordDelimitersDblClick = _T("");
	m_bAutoExpand = FALSE;
	m_bColourGradient = FALSE;
	m_bBleedBackground = FALSE;
	m_bFixedFontForEditing = FALSE;
	m_bSmootherScrolling = FALSE;
	m_bAllTypingToCommandWindow = FALSE;
	m_bDisableKeyboardMenuActivation = FALSE;
	m_bSmoothScrolling = FALSE;
	m_bRegexpMatchEmpty = FALSE;
	m_bNotifyIfCannotConnect = FALSE;
	m_bShowGridLinesInListViews = FALSE;
	m_bF1macro = FALSE;
	m_bTriggerRemoveCheck = FALSE;
	m_iWindowTabsStyle = -1;
	m_bFlatToolbars = FALSE;
	m_bErrorNotificationToOutputWindow = FALSE;
	//}}AFX_DATA_INIT
}

CGlobalPrefsP2::~CGlobalPrefsP2()
{
}

void CGlobalPrefsP2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalPrefsP2)
	DDX_Check(pDX, IDC_AUTO_CONNECT, m_bAutoConnectWorlds);
	DDX_Check(pDX, IDC_RECONNECT, m_bReconnectOnLinkFailure);
	DDX_Check(pDX, IDC_OPEN_WORLDS_MAXIMISED, m_bOpenWorldsMaximised);
	DDX_Check(pDX, IDC_NOTIFY_DISCONNECT, m_bNotifyOnDisconnect);
	DDX_Check(pDX, IDC_ENABLE_SPELL_CHECK, m_bEnableSpellCheck);
	DDX_Text(pDX, IDC_WORD_DELIMITERS, m_strWordDelimiters);
	DDX_Text(pDX, IDC_WORD_DELIMITERS2, m_strWordDelimitersDblClick);
	DDX_Check(pDX, IDC_AUTO_EXPAND, m_bAutoExpand);
	DDX_Check(pDX, IDC_COLOUR_GRADIENT, m_bColourGradient);
	DDX_Check(pDX, IDC_BLEED_BACKGROUND, m_bBleedBackground);
	DDX_Check(pDX, IDC_FIXED_FONT, m_bFixedFontForEditing);
	DDX_Check(pDX, IDC_SMOOTHER_SCROLLING, m_bSmootherScrolling);
	DDX_Check(pDX, IDC_ALL_TYPING_TO_COMMAND, m_bAllTypingToCommandWindow);
	DDX_Check(pDX, IDC_DISABLE_KEYBOARD_MENU_ACTIVATION, m_bDisableKeyboardMenuActivation);
	DDX_Check(pDX, IDC_SMOOTH_SCROLLING, m_bSmoothScrolling);
	DDX_Check(pDX, IDC_REGEXP_MATCH_EMPTY, m_bRegexpMatchEmpty);
	DDX_Check(pDX, IDC_NOTIFY_IF_CANNOT_CONNECT, m_bNotifyIfCannotConnect);
	DDX_Check(pDX, IDC_SHOW_GRID_LINES, m_bShowGridLinesInListViews);
	DDX_Check(pDX, IDC_F1_MACRO, m_bF1macro);
	DDX_Check(pDX, IDC_REMOVE_CHECK, m_bTriggerRemoveCheck);
	DDX_CBIndex(pDX, IDC_TABS, m_iWindowTabsStyle);
	DDX_Check(pDX, IDC_FLAT_TOOLBARS, m_bFlatToolbars);
	DDX_Check(pDX, IDC_NOTIFY_TO_OUTPUT, m_bErrorNotificationToOutputWindow);
	DDX_MinMaxString(pDX, IDC_LOCALE, m_strLocale);
	DDV_MinMaxString(pDX, m_strLocale, 2, 3);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGlobalPrefsP2, CPropertyPage)
	//{{AFX_MSG_MAP(CGlobalPrefsP2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP3 property page

CGlobalPrefsP3::CGlobalPrefsP3() : CPropertyPage(CGlobalPrefsP3::IDD)
{
	//{{AFX_DATA_INIT(CGlobalPrefsP3)
	m_bConfirmBeforeClosingMushclient = FALSE;
	m_bConfirmBeforeClosingWorld = FALSE;
	m_bConfirmBeforeClosingMXPdebug = FALSE;
	m_bConfirmBeforeSavingVariables = FALSE;
	//}}AFX_DATA_INIT
}

CGlobalPrefsP3::~CGlobalPrefsP3()
{
}

void CGlobalPrefsP3::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalPrefsP3)
	DDX_Check(pDX, IDC_CONFIRM_BEFORE_CLOSING_MUSHCLIENT, m_bConfirmBeforeClosingMushclient);
	DDX_Check(pDX, IDC_CONFIRM_BEFORE_CLOSING_WORLD, m_bConfirmBeforeClosingWorld);
	DDX_Check(pDX, IDC_CONFIRM_BEFORE_CLOSING_MXP_DEBUG, m_bConfirmBeforeClosingMXPdebug);
	DDX_Check(pDX, IDC_CONFIRM_BEFORE_SAVING_VARIABLES, m_bConfirmBeforeSavingVariables);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGlobalPrefsP3, CPropertyPage)
	//{{AFX_MSG_MAP(CGlobalPrefsP3)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP4 property page

CGlobalPrefsP4::CGlobalPrefsP4() : CPropertyPage(CGlobalPrefsP4::IDD)
{
	//{{AFX_DATA_INIT(CGlobalPrefsP4)
	m_strPrinterFont = _T("");
	m_nPrinterLeftMargin = 0;
	m_nPrinterTopMargin = 0;
	m_nPrinterLinesPerPage = 0;
	//}}AFX_DATA_INIT
}

CGlobalPrefsP4::~CGlobalPrefsP4()
{
}

void CGlobalPrefsP4::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalPrefsP4)
	DDX_Control(pDX, IDC_FONT_NAME, m_ctlFontName);
	DDX_Text(pDX, IDC_FONT_NAME, m_strPrinterFont);
	DDX_Text(pDX, IDC_LEFT_MARGIN, m_nPrinterLeftMargin);
	DDV_MinMaxUInt(pDX, m_nPrinterLeftMargin, 0, 100);
	DDX_Text(pDX, IDC_TOP_MARGIN, m_nPrinterTopMargin);
	DDV_MinMaxUInt(pDX, m_nPrinterTopMargin, 0, 100);
	DDX_Text(pDX, IDC_LINES_PER_PAGE, m_nPrinterLinesPerPage);
	DDV_MinMaxUInt(pDX, m_nPrinterLinesPerPage, 10, 500);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGlobalPrefsP4, CPropertyPage)
	//{{AFX_MSG_MAP(CGlobalPrefsP4)
	ON_BN_CLICKED(IDC_FONT, OnFont)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CGlobalPrefsP4::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	((CSpinButtonCtrl*) GetDlgItem(IDC_SPIN_TOP_MARGIN))->SetRange(0, 100);
	((CSpinButtonCtrl*) GetDlgItem(IDC_SPIN_LEFT_MARGIN))->SetRange(0, 100);
	((CSpinButtonCtrl*) GetDlgItem(IDC_SPIN_LINES_PER_PAGE))->SetRange(10, 500);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGlobalPrefsP4::OnFont() 
{
CDC dc;

dc.CreateCompatibleDC (NULL);

LOGFONT lf;

ZeroMemory (&lf, sizeof lf);

lf.lfHeight = -MulDiv(m_nPrinterFontSize, dc.GetDeviceCaps(LOGPIXELSY), 72);
strncpy (lf.lfFaceName, App.m_strPrinterFont, sizeof (lf.lfFaceName) - 1);
lf.lfFaceName [sizeof (lf.lfFaceName) - 1] = 0;

CFontDialog dlg (&lf,
                 CF_BOTH,
                 NULL,
                 GetOwner ());       // owner view

if (dlg.DoModal () != IDOK)
  return;

m_nPrinterFontSize = dlg.GetSize () / 10;

m_ctlFontName.SetWindowText(dlg.GetFaceName ());
	
}

/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP5 property page

CGlobalPrefsP5::CGlobalPrefsP5() : CPropertyPage(CGlobalPrefsP5::IDD)
{
	//{{AFX_DATA_INIT(CGlobalPrefsP5)
	m_bAutoLogWorld = FALSE;
	m_bAppendToLogFiles = FALSE;
	m_strDefaultLogFileDirectory = _T("");
	m_bConfirmLogFileClose = FALSE;
	//}}AFX_DATA_INIT
}

CGlobalPrefsP5::~CGlobalPrefsP5()
{
}

void CGlobalPrefsP5::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalPrefsP5)
	DDX_Check(pDX, IDC_AUTO_LOG, m_bAutoLogWorld);
	DDX_Check(pDX, IDC_AUTO_APPEND, m_bAppendToLogFiles);
	DDX_Text(pDX, IDC_DEFAULT_DIRECTORY_NAME, m_strDefaultLogFileDirectory);
	DDX_Check(pDX, IDC_CONFIRM_LOG_FILE_CLOSE, m_bConfirmLogFileClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGlobalPrefsP5, CPropertyPage)
	//{{AFX_MSG_MAP(CGlobalPrefsP5)
	ON_BN_CLICKED(IDC_DEFAULT_DIRECTORY, OnDefaultDirectory)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CGlobalPrefsP5::OnDefaultDirectory() 
{
	// Gets the Shell's default allocator
	LPMALLOC pMalloc;
	if (::SHGetMalloc(&pMalloc) == NOERROR)
	{
		char	pszBuffer[MAX_PATH];
		BROWSEINFO		bi;
		LPITEMIDLIST	pidl;

        // Get help on BROWSEINFO struct - it's got all the bit settings.
		bi.hwndOwner = GetSafeHwnd();
		bi.pidlRoot = NULL;
		bi.pszDisplayName = pszBuffer;
		bi.lpszTitle = "Log files folder";
		bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
    // if possible, let them create one
    if (!bWine)  
	  	bi.ulFlags |= BIF_NEWDIALOGSTYLE | BIF_EDITBOX;     // requires CoInitialize
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = 0;
  	GetDlgItemText(IDC_DEFAULT_DIRECTORY_NAME, strStartingDirectory);

		// This next call issues the dialog box.
		if ((pidl = ::SHBrowseForFolder(&bi)) != NULL)
		{
			if (::SHGetPathFromIDList(pidl, pszBuffer))
				SetDlgItemText(IDC_DEFAULT_DIRECTORY_NAME, pszBuffer);

			// Free the PIDL allocated by SHBrowseForFolder.
			pMalloc->Free(pidl);
		}
		// Release the shell's allocator.
		pMalloc->Release();
    }
	
}

/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP6 property page

CGlobalPrefsP6::CGlobalPrefsP6() : CPropertyPage(CGlobalPrefsP6::IDD)
{
	//{{AFX_DATA_INIT(CGlobalPrefsP6)
	m_nTimerInterval = 0;
	//}}AFX_DATA_INIT
}

CGlobalPrefsP6::~CGlobalPrefsP6()
{
}

void CGlobalPrefsP6::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalPrefsP6)
	DDX_Text(pDX, IDC_TIMER_INTERVAL, m_nTimerInterval);
	DDV_MinMaxUInt(pDX, m_nTimerInterval, 0, 120);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGlobalPrefsP6, CPropertyPage)
	//{{AFX_MSG_MAP(CGlobalPrefsP6)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CGlobalPrefsP6::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	((CSpinButtonCtrl*) GetDlgItem(IDC_SPIN_TIMER))->SetRange(0, 120);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//*****************************************************************************

// Callback for function BrowseFolder

int __stdcall BrowseCallbackProc(
    HWND hwnd, 	
    UINT uMsg, 	
    LPARAM lParam, 	
    LPARAM lpData	
   )
  {

  // initialise browse box if directory name is of the style:
  //      C:  (ie. a colon in column 2)
  //  or  \\servername\blah

  if (uMsg == BFFM_INITIALIZED)
    {

    RemoveTrailingBackslash (strStartingDirectory);

    CString tempstr = strStartingDirectory;

// no backslash? Prepend our startup directory to it
    
    if (strStartingDirectory.Find ('\\') == -1)
      {
      strStartingDirectory = working_dir;
      strStartingDirectory += tempstr;
      }
    else

// see if they said ".\". If so, replace with working directory
      
      if (strStartingDirectory.Left (2) == ".\\")
        {
        strStartingDirectory = working_dir;
        strStartingDirectory += tempstr.Mid (2);
        }

// working_dir may have added a backslash - get rid of it

    RemoveTrailingBackslash (strStartingDirectory);

    if (strStartingDirectory.GetLength () > 2 &&
          (strStartingDirectory [1] == ':'  || 
          strStartingDirectory.Left (2) == "\\\\")
         )
      SendMessage (hwnd, BFFM_SETSELECTION, TRUE, (LPARAM) (const char *) strStartingDirectory);
    }
  return 0;
  } // end of browse callback

 
void RemoveTrailingBackslash (CString& str)
  {

// don't get confused by leading or trailing spaces

  str.TrimLeft ();
  str.TrimRight ();

// empty string cannot have a backslash

  if (str.IsEmpty ())
    return;

// last character not backslash? If so, return

  if (str.GetAt (str.GetLength () - 1) != '\\')
    return;

// Remove one character from the string

  str = str.Left (str.GetLength () - 1);

  }

/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP7 property page


CGlobalPrefsP7::CGlobalPrefsP7() : CPropertyPage(CGlobalPrefsP7::IDD)
{
	//{{AFX_DATA_INIT(CGlobalPrefsP7)
	m_bOpenActivityWindow = FALSE;
	m_nActivityWindowRefreshInterval = 0;
	m_nActivityWindowRefreshType = -1;
	m_iActivityButtonBarStyle = -1;
	//}}AFX_DATA_INIT
}

CGlobalPrefsP7::~CGlobalPrefsP7()
{
}

void CGlobalPrefsP7::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalPrefsP7)
	DDX_Check(pDX, IDC_OPEN_ACTIVITY, m_bOpenActivityWindow);
	DDX_Text(pDX, IDC_ACTIVITY_INTERVAL, m_nActivityWindowRefreshInterval);
	DDV_MinMaxUInt(pDX, m_nActivityWindowRefreshInterval, 1, 300);
	DDX_Radio(pDX, IDC_UPDATE_ACTIVITY, m_nActivityWindowRefreshType);
	DDX_CBIndex(pDX, IDC_ACTIVITY_BUTTONS_STYLE, m_iActivityButtonBarStyle);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGlobalPrefsP7, CPropertyPage)
	//{{AFX_MSG_MAP(CGlobalPrefsP7)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP7 message handlers

BOOL CGlobalPrefsP7::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	((CSpinButtonCtrl*) GetDlgItem(IDC_PERIOD_SPIN))->SetRange(1, 300);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP9 property page


CGlobalPrefsP9::CGlobalPrefsP9() : CPropertyPage(CGlobalPrefsP9::IDD)
{
	//{{AFX_DATA_INIT(CGlobalPrefsP9)
	m_strDefaultColoursFile = _T("");
	m_strDefaultTriggersFile = _T("");
	m_strDefaultAliasesFile = _T("");
	m_strDefaultMacrosFile = _T("");
	m_strDefaultTimersFile = _T("");
	m_strDefaultOutputFont = _T("");
	m_strDefaultInputFont = _T("");
	m_strInputStyle = _T("");
	m_strOutputStyle = _T("");
	//}}AFX_DATA_INIT
}

CGlobalPrefsP9::~CGlobalPrefsP9()
{
}

void CGlobalPrefsP9::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalPrefsP9)
	DDX_Control(pDX, IDC_INPUT_STYLE, m_ctlInputStyle);
	DDX_Control(pDX, IDC_OUTPUT_STYLE, m_ctlOutputStyle);
	DDX_Control(pDX, IDC_DEFAULT_INPUT_FONT, m_ctlDefaultInputFont);
	DDX_Control(pDX, IDC_DEFAULT_OUTPUT_FONT, m_ctlDefaultOutputFont);
	DDX_Control(pDX, IDC_DEFAULT_TIMERS, m_ctlDefaultTimersFile);
	DDX_Control(pDX, IDC_DEFAULT_MACROS, m_ctlDefaultMacrosFile);
	DDX_Control(pDX, IDC_DEFAULT_ALIASES, m_ctlDefaultAliasesFile);
	DDX_Control(pDX, IDC_DEFAULT_TRIGGERS, m_ctlDefaultTriggersFile);
	DDX_Control(pDX, IDC_DEFAULT_COLOURS, m_ctlDefaultColoursFile);
	DDX_Text(pDX, IDC_DEFAULT_COLOURS, m_strDefaultColoursFile);
	DDX_Text(pDX, IDC_DEFAULT_TRIGGERS, m_strDefaultTriggersFile);
	DDX_Text(pDX, IDC_DEFAULT_ALIASES, m_strDefaultAliasesFile);
	DDX_Text(pDX, IDC_DEFAULT_MACROS, m_strDefaultMacrosFile);
	DDX_Text(pDX, IDC_DEFAULT_TIMERS, m_strDefaultTimersFile);
	DDX_Text(pDX, IDC_DEFAULT_OUTPUT_FONT, m_strDefaultOutputFont);
	DDX_Text(pDX, IDC_DEFAULT_INPUT_FONT, m_strDefaultInputFont);
	DDX_Text(pDX, IDC_INPUT_STYLE, m_strInputStyle);
	DDX_Text(pDX, IDC_OUTPUT_STYLE, m_strOutputStyle);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGlobalPrefsP9, CPropertyPage)
	//{{AFX_MSG_MAP(CGlobalPrefsP9)
	ON_BN_CLICKED(IDC_BROWSE_COLOURS, OnBrowseColours)
	ON_BN_CLICKED(IDC_BROWSE_TRIGGERS, OnBrowseTriggers)
	ON_BN_CLICKED(IDC_BROWSE_ALIASES, OnBrowseAliases)
	ON_BN_CLICKED(IDC_BROWSE_MACROS, OnBrowseMacros)
	ON_BN_CLICKED(IDC_BROWSE_TIMERS, OnBrowseTimers)
	ON_BN_CLICKED(IDC_SELECT_OUTPUT_FONT, OnSelectOutputFont)
	ON_BN_CLICKED(IDC_SELECT_INPUT_FONT, OnSelectInputFont)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP9 message handlers

bool CGlobalPrefsP9::GeneralBrowse (const int set_type,
                                    CString & strPathName)
  {
CString suggested_name = "Default",
        filter,
        title,
        suggested_extension;

  if (Set_Up_Set_Strings (set_type, 
                          suggested_name,
                          filter,
                          title,
                          suggested_extension))
      return true;    // bad set_type
  
 CString filename;

  CFileDialog filedlg (TRUE,   // loading the file
                       suggested_extension,    // default extension
                       "",  // suggested name
                       OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
                       filter,    // filter 
                       this);  // parent window

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
    return true;    // cancelled dialog

  strPathName = filedlg.GetPathName ();

  return false;
  }


void CGlobalPrefsP9::OnBrowseColours() 
{
  if (GeneralBrowse (COLOUR, m_strDefaultColoursFile)) 	
    return;   // failed

  m_ctlDefaultColoursFile.SetWindowText (m_strDefaultColoursFile);

}

void CGlobalPrefsP9::OnBrowseTriggers() 
{
  if (GeneralBrowse (TRIGGER, m_strDefaultTriggersFile)) 	
    return;   // failed

  m_ctlDefaultTriggersFile.SetWindowText (m_strDefaultTriggersFile);
	
}

void CGlobalPrefsP9::OnBrowseAliases() 
{
  if (GeneralBrowse (ALIAS, m_strDefaultAliasesFile)) 	
    return;   // failed

  m_ctlDefaultAliasesFile.SetWindowText (m_strDefaultAliasesFile);
	
}

void CGlobalPrefsP9::OnBrowseMacros() 
{
  if (GeneralBrowse (MACRO, m_strDefaultMacrosFile)) 	
    return;   // failed

  m_ctlDefaultMacrosFile.SetWindowText (m_strDefaultMacrosFile);
	
}

void CGlobalPrefsP9::OnBrowseTimers() 
{
  if (GeneralBrowse (TIMER, m_strDefaultTimersFile)) 	
    return;   // failed

  m_ctlDefaultTimersFile.SetWindowText (m_strDefaultTimersFile);
	
}

void CGlobalPrefsP9::OnSelectOutputFont() 
{
CDC dc;

dc.CreateCompatibleDC (NULL);

LOGFONT lf;

ZeroMemory (&lf, sizeof lf);

lf.lfHeight = -MulDiv(m_iDefaultOutputFontHeight, dc.GetDeviceCaps(LOGPIXELSY), 72);
strncpy (lf.lfFaceName, m_strDefaultOutputFont, sizeof (lf.lfFaceName) - 1);
lf.lfFaceName [sizeof (lf.lfFaceName) - 1] = 0;
lf.lfWeight = FW_NORMAL;
lf.lfItalic = FALSE;
lf.lfCharSet = m_iDefaultOutputFontCharset;

CFontDialog dlg (&lf,
                 CF_SCREENFONTS,
                 NULL,
                 GetOwner ());       // owner view

if (dlg.DoModal () != IDOK)
  return;

m_iDefaultOutputFontHeight = dlg.GetSize () / 10;
m_strDefaultOutputFont   = dlg.GetFaceName (); 
m_iDefaultOutputFontCharset = lf.lfCharSet;

m_ctlDefaultOutputFont.SetWindowText(m_strDefaultOutputFont);
m_ctlOutputStyle.SetWindowText(CFormat ("%i pt.",m_iDefaultOutputFontHeight));
	
	
}

void CGlobalPrefsP9::OnSelectInputFont() 
{

CDC dc;

dc.CreateCompatibleDC (NULL);

LOGFONT lf;

ZeroMemory (&lf, sizeof lf);

lf.lfHeight = -MulDiv(m_iDefaultInputFontHeight, dc.GetDeviceCaps(LOGPIXELSY), 72);
strncpy (lf.lfFaceName, m_strDefaultInputFont, sizeof (lf.lfFaceName) - 1);
lf.lfFaceName [sizeof (lf.lfFaceName) - 1] = 0;
lf.lfWeight = m_iDefaultInputFontWeight;
lf.lfItalic = m_iDefaultInputFontItalic;
lf.lfCharSet = m_iDefaultInputFontCharset;

CFontDialog dlg (&lf,
                 CF_SCREENFONTS,
                 NULL,
                 GetOwner ());       // owner view

if (dlg.DoModal () != IDOK)
  return;

m_iDefaultInputFontHeight = dlg.GetSize () / 10;
m_strDefaultInputFont   = dlg.GetFaceName (); 
m_iDefaultInputFontWeight = dlg.GetWeight (); 
m_iDefaultInputFontItalic = dlg.IsItalic ();
m_iDefaultInputFontCharset = lf.lfCharSet;

m_ctlDefaultInputFont.SetWindowText(m_strDefaultInputFont);

CString strStyle = CFormat ("%i pt.", m_iDefaultInputFontHeight);
if (m_iDefaultInputFontWeight == FW_BOLD)
  strStyle += " bold";
if (m_iDefaultInputFontItalic)
  strStyle += " italic";
                    
m_ctlInputStyle.SetWindowText(strStyle);

}

/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP10 property page


CGlobalPrefsP10::CGlobalPrefsP10() : CPropertyPage(CGlobalPrefsP10::IDD)
{
	//{{AFX_DATA_INIT(CGlobalPrefsP10)
	m_strNotepadFont = _T("");
	m_strNotepadStyle = _T("");
	m_bNotepadWordWrap = FALSE;
	m_strNotepadQuoteString = _T("");
	//}}AFX_DATA_INIT
}

CGlobalPrefsP10::~CGlobalPrefsP10()
{
}

void CGlobalPrefsP10::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalPrefsP10)
	DDX_Control(pDX, IDC_BACKGROUND_SWATCH, m_ctlBackgroundSwatch);
	DDX_Control(pDX, IDC_TEXT_SWATCH, m_ctlTextSwatch);
	DDX_Control(pDX, IDC_ESCAPE_DOUBLE_QUOTES, m_ctlEscapeDoubleQuotes);
	DDX_Control(pDX, IDC_ESCAPE_SINGLE_QUOTES, m_ctlEscapeSingleQuotes);
	DDX_Control(pDX, IDC_DOUBLE_QUOTES, m_ctlDoubleQuotes);
	DDX_Control(pDX, IDC_SINGLE_QUOTES, m_ctlSingleQuotes);
	DDX_Control(pDX, IDC_PERCENT_ESCAPES, m_ctlPercentEscapes);
	DDX_Control(pDX, IDC_BACKSLASH_ESCAPES, m_ctlBackslashEscapes);
	DDX_Control(pDX, IDC_BRACES_NEST, m_ctlNestBraces);
	DDX_Control(pDX, IDC_NOTEPAD_STYLE, m_ctlNotepadStyle);
	DDX_Control(pDX, IDC_DEFAULT_NOTEPAD_FONT, m_ctlNotepadFont);
	DDX_Text(pDX, IDC_DEFAULT_NOTEPAD_FONT, m_strNotepadFont);
	DDX_Text(pDX, IDC_NOTEPAD_STYLE, m_strNotepadStyle);
	DDX_Check(pDX, IDC_WORD_WRAP, m_bNotepadWordWrap);
	DDX_Text(pDX, IDC_QUOTE_STRING, m_strNotepadQuoteString);
	//}}AFX_DATA_MAP

  if (pDX->m_bSaveAndValidate )
    {
    m_nParenMatchFlags = 0;
    if (m_ctlNestBraces.GetCheck ()) m_nParenMatchFlags |= PAREN_NEST_BRACES;
    if (m_ctlBackslashEscapes.GetCheck ()) m_nParenMatchFlags |= PAREN_BACKSLASH_ESCAPES;
    if (m_ctlPercentEscapes.GetCheck ()) m_nParenMatchFlags |= PAREN_PERCENT_ESCAPES;
    if (m_ctlSingleQuotes.GetCheck ()) m_nParenMatchFlags |= PAREN_SINGLE_QUOTES;
    if (m_ctlDoubleQuotes.GetCheck ()) m_nParenMatchFlags |= PAREN_DOUBLE_QUOTES;
    if (m_ctlEscapeSingleQuotes.GetCheck ()) m_nParenMatchFlags |= PAREN_ESCAPE_SINGLE_QUOTES;
    if (m_ctlEscapeDoubleQuotes.GetCheck ()) m_nParenMatchFlags |= PAREN_ESCAPE_DOUBLE_QUOTES;
    }  // end of saving and validating
  else
    {
    m_ctlNestBraces.SetCheck ((m_nParenMatchFlags & PAREN_NEST_BRACES) != 0);
    m_ctlBackslashEscapes.SetCheck ((m_nParenMatchFlags & PAREN_BACKSLASH_ESCAPES) != 0);
    m_ctlPercentEscapes.SetCheck ((m_nParenMatchFlags & PAREN_PERCENT_ESCAPES) != 0);
    m_ctlSingleQuotes.SetCheck ((m_nParenMatchFlags & PAREN_SINGLE_QUOTES) != 0);
    m_ctlDoubleQuotes.SetCheck ((m_nParenMatchFlags & PAREN_DOUBLE_QUOTES) != 0);
    m_ctlEscapeSingleQuotes.SetCheck ((m_nParenMatchFlags & PAREN_ESCAPE_SINGLE_QUOTES) != 0);
    m_ctlEscapeDoubleQuotes.SetCheck ((m_nParenMatchFlags & PAREN_ESCAPE_DOUBLE_QUOTES) != 0);
    }   // end of not saving and validating
}


BEGIN_MESSAGE_MAP(CGlobalPrefsP10, CPropertyPage)
	//{{AFX_MSG_MAP(CGlobalPrefsP10)
	ON_BN_CLICKED(IDC_TEXT_SWATCH, OnTextSwatch)
	ON_BN_CLICKED(IDC_BACKGROUND_SWATCH, OnBackgroundSwatch)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_ESCAPE_SINGLE_QUOTES, OnUpdateNeedSingleQuotes)
  ON_UPDATE_COMMAND_UI(IDC_ESCAPE_DOUBLE_QUOTES, OnUpdateNeedDoubleQuotes)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP10 message handlers


BOOL CGlobalPrefsP10::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
  m_ctlTextSwatch.m_colour = m_cNotepadTextColour;
  m_ctlBackgroundSwatch.m_colour = m_cNotepadBackColour;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGlobalPrefsP10::OnOK() 
{
// remember colour of text
  
  m_cNotepadTextColour = m_ctlTextSwatch.m_colour;
  m_cNotepadBackColour = m_ctlBackgroundSwatch.m_colour;
	
	CPropertyPage::OnOK();
}

LRESULT CGlobalPrefsP10::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CGlobalPrefsP10::OnKickIdle

void CGlobalPrefsP10::OnUpdateNeedSingleQuotes(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_ctlSingleQuotes.GetCheck () &&
                 (m_ctlPercentEscapes.GetCheck () || 
                  m_ctlBackslashEscapes.GetCheck ())
                );
}   // end of  CGlobalPrefsP10::OnUpdateNeedSingleQuotes

void CGlobalPrefsP10::OnUpdateNeedDoubleQuotes(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_ctlDoubleQuotes.GetCheck () &&
                 (m_ctlPercentEscapes.GetCheck () || 
                  m_ctlBackslashEscapes.GetCheck ())
                );
}   // end of  CGlobalPrefsP10::OnUpdateNeedDoubleQuotes


void CGlobalPrefsP10::OnTextSwatch() 
{
  CColourPickerDlg dlg;
  dlg.m_iColour = m_ctlTextSwatch.m_colour;
  dlg.m_bPickColour = true;


  if (dlg.DoModal () != IDOK)
    return;

  m_ctlTextSwatch.m_colour = dlg.GetColor ();
  m_ctlTextSwatch.RedrawWindow();
	
}

void CGlobalPrefsP10::OnBackgroundSwatch() 
{
  CColourPickerDlg dlg;
  dlg.m_iColour = m_ctlBackgroundSwatch.m_colour;
  dlg.m_bPickColour = true;

  if (dlg.DoModal () != IDOK)
    return;

  m_ctlBackgroundSwatch.m_colour = dlg.GetColor ();
  m_ctlBackgroundSwatch.RedrawWindow();
	
}


void DoGlobalPrefs (CMUSHclientDoc * pCurrentDoc)
  {

	CGlobalPrefsSheet propSheet;
  propSheet.m_Page1.m_pCurrentDoc = pCurrentDoc;

	CMUSHclientApp*	pApp = &App;

  // *** also see table in globalregistryoptions.cpp

  propSheet.m_Page2.m_bAutoConnectWorlds              =  pApp->m_bAutoConnectWorlds                ;
  propSheet.m_Page2.m_bReconnectOnLinkFailure         =  pApp->m_bReconnectOnLinkFailure           ;
  propSheet.m_Page2.m_bOpenWorldsMaximised            =  pApp->m_bOpenWorldsMaximised              ;
  propSheet.m_Page2.m_iWindowTabsStyle                =  pApp->m_iWindowTabsStyle              ;
  propSheet.m_Page2.m_bNotifyOnDisconnect             =  pApp->m_bNotifyOnDisconnect               ;
  propSheet.m_Page2.m_bNotifyIfCannotConnect          =  pApp->m_bNotifyIfCannotConnect            ;
  propSheet.m_Page2.m_bErrorNotificationToOutputWindow=  pApp->m_bErrorNotificationToOutputWindow            ;
  propSheet.m_Page2.m_bEnableSpellCheck               =  pApp->m_bEnableSpellCheck                 ;
  propSheet.m_Page2.m_strWordDelimiters               =  pApp->m_strWordDelimiters;
  propSheet.m_Page2.m_strWordDelimitersDblClick       =  pApp->m_strWordDelimitersDblClick;
  propSheet.m_Page2.m_bAutoExpand                     =  pApp->m_bAutoExpand;
  propSheet.m_Page2.m_bFlatToolbars                   =  pApp->m_bFlatToolbars;
  propSheet.m_Page2.m_bColourGradient                 =  pApp->m_bColourGradient;
  propSheet.m_Page2.m_bBleedBackground                =  pApp->m_bBleedBackground;
  propSheet.m_Page2.m_bFixedFontForEditing            =  pApp->m_bFixedFontForEditing;
  propSheet.m_Page2.m_bRegexpMatchEmpty               =  pApp->m_bRegexpMatchEmpty;
  propSheet.m_Page2.m_bSmootherScrolling              =  pApp->m_bSmootherScrolling;
  propSheet.m_Page2.m_bSmoothScrolling                =  pApp->m_bSmoothScrolling;
  propSheet.m_Page2.m_bShowGridLinesInListViews       =  pApp->m_bShowGridLinesInListViews;
  propSheet.m_Page2.m_bTriggerRemoveCheck             =  pApp->m_bTriggerRemoveCheck;
  propSheet.m_Page2.m_bF1macro                        =  pApp->m_bF1macro;
  propSheet.m_Page2.m_bAllTypingToCommandWindow       =  pApp->m_bAllTypingToCommandWindow;
  propSheet.m_Page2.m_bDisableKeyboardMenuActivation  =  pApp->m_bDisableKeyboardMenuActivation;
  propSheet.m_Page2.m_strLocale                       =  pApp->m_strLocale;
  propSheet.m_Page3.m_bConfirmBeforeClosingWorld      =  pApp->m_bConfirmBeforeClosingWorld        ;
  propSheet.m_Page3.m_bConfirmBeforeClosingMushclient =  pApp->m_bConfirmBeforeClosingMushclient   ;
  propSheet.m_Page3.m_bConfirmBeforeClosingMXPdebug   =  pApp->m_bConfirmBeforeClosingMXPdebug   ;
  propSheet.m_Page3.m_bConfirmBeforeSavingVariables   =  pApp->m_bConfirmBeforeSavingVariables   ;
  propSheet.m_Page4.m_nPrinterFontSize                =  pApp->m_nPrinterFontSize                  ;
  propSheet.m_Page4.m_nPrinterLeftMargin              =  pApp->m_nPrinterLeftMargin                ;
  propSheet.m_Page4.m_nPrinterTopMargin               =  pApp->m_nPrinterTopMargin                 ;
  propSheet.m_Page4.m_nPrinterLinesPerPage            =  pApp->m_nPrinterLinesPerPage              ;
  propSheet.m_Page5.m_bAutoLogWorld                   =  pApp->m_bAutoLogWorld                     ;
  propSheet.m_Page5.m_bAppendToLogFiles               =  pApp->m_bAppendToLogFiles                 ;
  propSheet.m_Page5.m_bConfirmLogFileClose            =  pApp->m_bConfirmLogFileClose                 ;
  propSheet.m_Page6.m_nTimerInterval                  =  pApp->m_nTimerInterval                    ;
  propSheet.m_Page4.m_strPrinterFont                  =  pApp->m_strPrinterFont                    ;
  propSheet.m_Page5.m_strDefaultLogFileDirectory      =  pApp->m_strDefaultLogFileDirectory        ;
  propSheet.m_Page1.m_strWorldList                    =  pApp->m_strWorldList                      ;
  propSheet.m_Page1.m_strDefaultWorldFileDirectory    =  pApp->m_strDefaultWorldFileDirectory;
  propSheet.m_Page7.m_bOpenActivityWindow             =  pApp->m_bOpenActivityWindow               ;
  propSheet.m_Page7.m_nActivityWindowRefreshInterval  =  pApp->m_nActivityWindowRefreshInterval    ;
  propSheet.m_Page7.m_nActivityWindowRefreshType      =  pApp->m_nActivityWindowRefreshType        ;
  propSheet.m_Page7.m_iActivityButtonBarStyle         =  pApp->m_iActivityButtonBarStyle        ;
  propSheet.m_Page12.m_strPluginsDirectory            =  pApp->m_strPluginsDirectory;
  propSheet.m_Page12.m_strPluginList                  =  pApp->m_strPluginList                      ;
  propSheet.m_Page13.m_strLuaScript                   =  pApp->m_strLuaScript                      ;
  propSheet.m_Page13.m_bEnablePackageLibrary          =  pApp->m_bEnablePackageLibrary             ;

  // defaults
  propSheet.m_Page9.m_strDefaultColoursFile        =   pApp->m_strDefaultColoursFile   ;
  propSheet.m_Page9.m_strDefaultTriggersFile       =   pApp->m_strDefaultTriggersFile  ;
  propSheet.m_Page9.m_strDefaultAliasesFile        =   pApp->m_strDefaultAliasesFile   ;
  propSheet.m_Page9.m_strDefaultMacrosFile         =   pApp->m_strDefaultMacrosFile    ;
  propSheet.m_Page9.m_strDefaultTimersFile         =   pApp->m_strDefaultTimersFile    ;
  propSheet.m_Page9.m_strDefaultOutputFont         =   pApp->m_strDefaultOutputFont    ;
  propSheet.m_Page9.m_strDefaultInputFont          =   pApp->m_strDefaultInputFont     ;      
  propSheet.m_Page9.m_iDefaultInputFontHeight      =   pApp->m_iDefaultInputFontHeight ;  
  propSheet.m_Page9.m_iDefaultInputFontWeight      =   pApp->m_iDefaultInputFontWeight ;
  propSheet.m_Page9.m_iDefaultInputFontCharset     =   pApp->m_iDefaultInputFontCharset ;
  propSheet.m_Page9.m_iDefaultInputFontItalic      =   pApp->m_iDefaultInputFontItalic ;
  propSheet.m_Page9.m_iDefaultOutputFontHeight     =   pApp->m_iDefaultOutputFontHeight;          
  propSheet.m_Page9.m_iDefaultOutputFontCharset    =   pApp->m_iDefaultOutputFontCharset;          

  // notepad
  propSheet.m_Page10.m_bNotepadWordWrap             =  pApp->m_bNotepadWordWrap;
  propSheet.m_Page10.m_cNotepadTextColour           =  pApp->m_cNotepadTextColour;
  propSheet.m_Page10.m_cNotepadBackColour           =  pApp->m_cNotepadBackColour;
  propSheet.m_Page10.m_nParenMatchFlags             =  pApp->m_nParenMatchFlags;
  propSheet.m_Page10.m_strNotepadQuoteString        =  pApp->m_strNotepadQuoteString;

  CString strStyle = CFormat ("%i pt.", pApp->m_iDefaultInputFontHeight);
  if (pApp->m_iDefaultInputFontWeight == FW_BOLD)
    strStyle += " bold";
  if (pApp->m_iDefaultInputFontItalic)
    strStyle += " italic";
 
  propSheet.m_Page9.m_strInputStyle   =   strStyle   ;      
  propSheet.m_Page9.m_strOutputStyle  =   CFormat ("%i pt.", pApp->m_iDefaultOutputFontHeight)   ;      

  // tray

  propSheet.m_Page11.m_iShowIconPlace                 =  pApp->m_iIconPlacement;
  propSheet.m_Page11.m_iIconType                      =  pApp->m_iTrayIcon;
  propSheet.m_Page11.m_strTrayIconFileName            =  pApp->m_strTrayIconFileName;
                                                      
	if (propSheet.DoModal() != IDOK)
    return;

// if they changed the timer interval, kill the old timer and restart it with the new interval

  if (pApp->m_nTimerInterval != propSheet.m_Page6.m_nTimerInterval)
    {
    if (Frame.m_timer)
        Frame.KillTimer (Frame.m_timer);
    if (propSheet.m_Page6.m_nTimerInterval > 0)
      Frame.m_timer = Frame.SetTimer(ACTIVITY_TIMER_ID, propSheet.m_Page6.m_nTimerInterval * 1000, NULL );
    else
      Frame.m_timer = Frame.SetTimer(ACTIVITY_TIMER_ID, MIN_TIMER_INTERVAL, NULL );  // 1/10 of a second ;)
    }   // end of timer interval changing


  if (propSheet.m_Page1.m_strDefaultWorldFileDirectory.Right (1) != '\\')
    propSheet.m_Page1.m_strDefaultWorldFileDirectory += '\\';
  if (propSheet.m_Page12.m_strPluginsDirectory.Right (1) != '\\')
    propSheet.m_Page12.m_strPluginsDirectory += '\\';
  if (propSheet.m_Page5.m_strDefaultLogFileDirectory.Right (1) != '\\')
    propSheet.m_Page5.m_strDefaultLogFileDirectory += '\\';

  // *** also see table in globalregistryoptions.cpp

  pApp->m_bAutoConnectWorlds              =  propSheet.m_Page2.m_bAutoConnectWorlds                ;
  pApp->m_bReconnectOnLinkFailure         =  propSheet.m_Page2.m_bReconnectOnLinkFailure           ;
  pApp->m_bOpenWorldsMaximised            =  propSheet.m_Page2.m_bOpenWorldsMaximised              ;
  pApp->m_iWindowTabsStyle                =  propSheet.m_Page2.m_iWindowTabsStyle              ;
  pApp->m_bNotifyOnDisconnect             =  propSheet.m_Page2.m_bNotifyOnDisconnect               ;
  pApp->m_bNotifyIfCannotConnect          =  propSheet.m_Page2.m_bNotifyIfCannotConnect            ;
  pApp->m_bErrorNotificationToOutputWindow=  propSheet.m_Page2.m_bErrorNotificationToOutputWindow            ;
  pApp->m_bEnableSpellCheck               =  propSheet.m_Page2.m_bEnableSpellCheck                 ;
  pApp->m_strWordDelimiters               =  propSheet.m_Page2.m_strWordDelimiters                 ;
  pApp->m_strWordDelimitersDblClick       =  propSheet.m_Page2.m_strWordDelimitersDblClick         ;
  pApp->m_bAutoExpand                     =  propSheet.m_Page2.m_bAutoExpand         ;
  pApp->m_bFlatToolbars                   =  propSheet.m_Page2.m_bFlatToolbars         ;
  pApp->m_bColourGradient                 =  propSheet.m_Page2.m_bColourGradient;
  pApp->m_bBleedBackground                =  propSheet.m_Page2.m_bBleedBackground;
  pApp->m_bFixedFontForEditing            =  propSheet.m_Page2.m_bFixedFontForEditing;
  pApp->m_bRegexpMatchEmpty               =  propSheet.m_Page2.m_bRegexpMatchEmpty;
  pApp->m_bSmootherScrolling              =  propSheet.m_Page2.m_bSmootherScrolling;
  pApp->m_bSmoothScrolling                =  propSheet.m_Page2.m_bSmoothScrolling;
  pApp->m_bShowGridLinesInListViews       =  propSheet.m_Page2.m_bShowGridLinesInListViews;
  pApp->m_bTriggerRemoveCheck             =  propSheet.m_Page2.m_bTriggerRemoveCheck;
  pApp->m_bF1macro                        =  propSheet.m_Page2.m_bF1macro;
  pApp->m_bAllTypingToCommandWindow       =  propSheet.m_Page2.m_bAllTypingToCommandWindow;
  pApp->m_bDisableKeyboardMenuActivation  =  propSheet.m_Page2.m_bDisableKeyboardMenuActivation;
  pApp->m_strLocale                       =  propSheet.m_Page2.m_strLocale;
  pApp->m_bConfirmBeforeClosingWorld      =  propSheet.m_Page3.m_bConfirmBeforeClosingWorld        ;
  pApp->m_bConfirmBeforeClosingMushclient =  propSheet.m_Page3.m_bConfirmBeforeClosingMushclient   ;
  pApp->m_bConfirmBeforeClosingMXPdebug   =  propSheet.m_Page3.m_bConfirmBeforeClosingMXPdebug   ;
  pApp->m_bConfirmBeforeSavingVariables   =  propSheet.m_Page3.m_bConfirmBeforeSavingVariables   ;
  pApp->m_nPrinterFontSize                =  propSheet.m_Page4.m_nPrinterFontSize                  ;
  pApp->m_nPrinterLeftMargin              =  propSheet.m_Page4.m_nPrinterLeftMargin                ;
  pApp->m_nPrinterTopMargin               =  propSheet.m_Page4.m_nPrinterTopMargin                 ;
  pApp->m_nPrinterLinesPerPage            =  propSheet.m_Page4.m_nPrinterLinesPerPage              ;
  pApp->m_bAutoLogWorld                   =  propSheet.m_Page5.m_bAutoLogWorld                     ;
  pApp->m_bAppendToLogFiles               =  propSheet.m_Page5.m_bAppendToLogFiles                 ;
  pApp->m_bConfirmLogFileClose            =  propSheet.m_Page5.m_bConfirmLogFileClose                 ;
  pApp->m_nTimerInterval                  =  propSheet.m_Page6.m_nTimerInterval                    ;
  pApp->m_strPrinterFont                  =  propSheet.m_Page4.m_strPrinterFont                    ;
  pApp->m_strDefaultLogFileDirectory      =  propSheet.m_Page5.m_strDefaultLogFileDirectory        ;
  pApp->m_strDefaultWorldFileDirectory    =  propSheet.m_Page1.m_strDefaultWorldFileDirectory        ;
  pApp->m_strWorldList                    =  propSheet.m_Page1.m_strWorldList                      ;
  pApp->m_bOpenActivityWindow             =  propSheet.m_Page7.m_bOpenActivityWindow               ;
  pApp->m_nActivityWindowRefreshInterval  =  propSheet.m_Page7.m_nActivityWindowRefreshInterval    ;
  pApp->m_nActivityWindowRefreshType      =  propSheet.m_Page7.m_nActivityWindowRefreshType        ;
  pApp->m_iActivityButtonBarStyle         =  propSheet.m_Page7.m_iActivityButtonBarStyle        ;
  pApp->m_strPluginsDirectory             =  propSheet.m_Page12.m_strPluginsDirectory        ;
  pApp->m_strPluginList                   =  propSheet.m_Page12.m_strPluginList                      ;
  pApp->m_strLuaScript                    =  propSheet.m_Page13.m_strLuaScript                      ;
  pApp->m_bEnablePackageLibrary           =  propSheet.m_Page13.m_bEnablePackageLibrary          ;

  // defaults
  pApp->m_strDefaultColoursFile        =  propSheet.m_Page9.m_strDefaultColoursFile   ;
  pApp->m_strDefaultTriggersFile       =  propSheet.m_Page9.m_strDefaultTriggersFile  ;
  pApp->m_strDefaultAliasesFile        =  propSheet.m_Page9.m_strDefaultAliasesFile   ;
  pApp->m_strDefaultMacrosFile         =  propSheet.m_Page9.m_strDefaultMacrosFile    ;
  pApp->m_strDefaultTimersFile         =  propSheet.m_Page9.m_strDefaultTimersFile    ;
  pApp->m_strDefaultOutputFont         =  propSheet.m_Page9.m_strDefaultOutputFont    ;
  pApp->m_strDefaultInputFont          =  propSheet.m_Page9.m_strDefaultInputFont     ;      
  pApp->m_iDefaultInputFontHeight      =  propSheet.m_Page9.m_iDefaultInputFontHeight ;  
  pApp->m_iDefaultInputFontWeight      =  propSheet.m_Page9.m_iDefaultInputFontWeight ;
  pApp->m_iDefaultInputFontCharset     =  propSheet.m_Page9.m_iDefaultInputFontCharset ;
  pApp->m_iDefaultInputFontItalic      =  propSheet.m_Page9.m_iDefaultInputFontItalic ;
  pApp->m_iDefaultOutputFontHeight     =  propSheet.m_Page9.m_iDefaultOutputFontHeight;          
  pApp->m_iDefaultOutputFontCharset    =  propSheet.m_Page9.m_iDefaultOutputFontCharset;          

  // notepad

  pApp->m_bNotepadWordWrap             =  propSheet.m_Page10.m_bNotepadWordWrap;
  pApp->m_nParenMatchFlags             =  propSheet.m_Page10.m_nParenMatchFlags;
  pApp->m_cNotepadTextColour           =  propSheet.m_Page10.m_cNotepadTextColour;
  pApp->m_cNotepadBackColour           =  propSheet.m_Page10.m_cNotepadBackColour;
  pApp->m_strNotepadQuoteString        =  propSheet.m_Page10.m_strNotepadQuoteString;

  // tray

  pApp->m_iIconPlacement               =     propSheet.m_Page11.m_iShowIconPlace;
  pApp->m_iTrayIcon                    =     propSheet.m_Page11.m_iIconType;               
  pApp->m_strTrayIconFileName          =     propSheet.m_Page11.m_strTrayIconFileName;               

  // save back to database
  App.SaveGlobalsToDatabase ();

  // remove old tray icon
  if (Frame.m_niData.uID)
    Shell_NotifyIcon (NIM_DELETE, &Frame.m_niData);

  // add new one if required
  if (pApp->m_iIconPlacement == ICON_PLACEMENT_TRAY ||
      pApp->m_iIconPlacement == ICON_PLACEMENT_BOTH)
    Frame.AddTrayIcon ();       


  // enable spell checker, if wanted in prefs
  if (App.m_bEnableSpellCheck)
    App.InitSpellCheck ();      // re-init, in case it was off before
  else
    App.m_bSpellCheckOK = false;    // the user doesn't want the spell checker

  // load appropriate bitmap for activity buttons
  
  Frame.m_wndActivityToolBar.LoadBitmap (ActivityToolBarResourceNames [App.m_iActivityButtonBarStyle]);
  Frame.m_wndActivityToolBar.Invalidate ();  // redraw it

   // now update all views in all documents in case they changed the bleed characteristic
  for (POSITION docPos = App.m_pWorldDocTemplate->GetFirstDocPosition();
      docPos != NULL; )
    {
    CMUSHclientDoc * pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(docPos);
    pDoc->UpdateAllViews (NULL);
    } // end of doing each document
} // end of DoGlobalPrefs




/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP11 property page

CGlobalPrefsP11::CGlobalPrefsP11() : CPropertyPage(CGlobalPrefsP11::IDD)
{
	//{{AFX_DATA_INIT(CGlobalPrefsP11)
	m_iShowIconPlace = -1;
	m_strTrayIconFileName = _T("");
	m_iIconType = -1;
	//}}AFX_DATA_INIT
}

CGlobalPrefsP11::~CGlobalPrefsP11()
{
}

void CGlobalPrefsP11::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalPrefsP11)
	DDX_Control(pDX, IDC_ICON_FILE_NAME, m_ctlFileName);
	DDX_CBIndex(pDX, IDC_MINIMISE_TO, m_iShowIconPlace);
	DDX_Text(pDX, IDC_ICON_FILE_NAME, m_strTrayIconFileName);
	DDX_Radio(pDX, IDC_MUSHCLIENT_ICON, m_iIconType);
	//}}AFX_DATA_MAP

  if (pDX->m_bSaveAndValidate)
    m_ctlFileName.SetWindowText (m_strTrayIconFileName);
  else
    m_strTrayIconFileName = GetText (m_ctlFileName);

}


BEGIN_MESSAGE_MAP(CGlobalPrefsP11, CPropertyPage)
	//{{AFX_MSG_MAP(CGlobalPrefsP11)
	ON_BN_CLICKED(IDC_CHOOSE_ICON_FILE, OnChooseIconFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP11 message handlers


void CGlobalPrefsP11::OnChooseIconFile() 
{
  CFileDialog filedlg (TRUE,   // loading the file
                       "ico",    // default extension
                       App.m_strTrayIconFileName,  // suggested name
                       OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
                       "Icon files (*.ico)|*.ico|"
                       "|",    // filter 
                       this);  // parent window

  filedlg.m_ofn.lpstrTitle = "Tray icon";

  ChangeToFileBrowsingDirectory ();
	int nResult = filedlg.DoModal();
  ChangeToStartupDirectory ();

  if (nResult != IDOK)
    return;    // cancelled dialog
	
  m_strTrayIconFileName = filedlg.GetPathName ();

  m_ctlFileName.SetWindowText (m_strTrayIconFileName);

  CheckRadioButton (IDC_MUSHCLIENT_ICON, IDC_CUSTOM_ICON, IDC_CUSTOM_ICON);

}
/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP12 property page


CGlobalPrefsP12::CGlobalPrefsP12() : CPropertyPage(CGlobalPrefsP12::IDD)
{
	//{{AFX_DATA_INIT(CGlobalPrefsP12)
	m_strPluginsDirectory = _T("");
	//}}AFX_DATA_INIT
}

CGlobalPrefsP12::~CGlobalPrefsP12()
{
}

void CGlobalPrefsP12::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalPrefsP12)
	DDX_Control(pDX, IDC_SELECTED_PLUGIN, m_ctlSelectedPlugin);
	DDX_Text(pDX, IDC_PLUGINS_DIRECTORY_NAME, m_strPluginsDirectory);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGlobalPrefsP12, CPropertyPage)
	//{{AFX_MSG_MAP(CGlobalPrefsP12)
	ON_BN_CLICKED(IDC_PLUGINS_DIRECTORY, OnPluginsDirectory)
	ON_BN_CLICKED(IDC_NEW_PLUGIN, OnNewPlugin)
	ON_BN_CLICKED(IDC_REMOVE_PLUGIN, OnRemovePlugin)
	ON_BN_CLICKED(IDC_MOVE_UP, OnMoveUp)
	ON_BN_CLICKED(IDC_MOVE_DOWN, OnMoveDown)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_MOVE_UP, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_MOVE_DOWN, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_REMOVE_PLUGIN, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_SELECTED_PLUGIN, OnUpdateSelectedPlugin)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP12 message handlers

void CGlobalPrefsP12::OnPluginsDirectory() 
{
	// Gets the Shell's default allocator
	LPMALLOC pMalloc;
	if (::SHGetMalloc(&pMalloc) == NOERROR)
	{
		char	pszBuffer[MAX_PATH];
		BROWSEINFO		bi;
		LPITEMIDLIST	pidl;

        // Get help on BROWSEINFO struct - it's got all the bit settings.
		bi.hwndOwner = GetSafeHwnd();
		bi.pidlRoot = NULL;
		bi.pszDisplayName = pszBuffer;
		bi.lpszTitle = "Plugins folder";
		bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
    // if possible, let them create one
    if (!bWine)  
	  	bi.ulFlags |= BIF_NEWDIALOGSTYLE | BIF_EDITBOX;     // requires CoInitialize
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = 0;
  	GetDlgItemText(IDC_PLUGINS_DIRECTORY_NAME, strStartingDirectory);

		// This next call issues the dialog box.
		if ((pidl = ::SHBrowseForFolder(&bi)) != NULL)
		{
			if (::SHGetPathFromIDList(pidl, pszBuffer))
				SetDlgItemText(IDC_PLUGINS_DIRECTORY_NAME, pszBuffer);

			// Free the PIDL allocated by SHBrowseForFolder.
			pMalloc->Free(pidl);
		}
		// Release the shell's allocator.
		pMalloc->Release();
    }
		
}

void CGlobalPrefsP12::OnNewPlugin() 
{
	CFileDialog	dlg(TRUE,
		"mcl",
		NULL,
		OFN_ALLOWMULTISELECT|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,
		"MUSHclient Plugins (*.xml)|*.xml||",
		this);
	dlg.m_ofn.lpstrTitle = "Add Global Plugins";
	char	szFileBuffer[4096];
	szFileBuffer[0] = 0;
	dlg.m_ofn.lpstrFile = szFileBuffer;
	dlg.m_ofn.nMaxFile = sizeof(szFileBuffer);
  // use default Plugin file directory
  CString strStartingDirectory;
	GetDlgItemText(IDC_PLUGINS_DIRECTORY_NAME, strStartingDirectory);
  dlg.m_ofn.lpstrInitialDir = strStartingDirectory;

  ChangeToFileBrowsingDirectory ();
	int nResult = dlg.DoModal();
  ChangeToStartupDirectory ();

	if (nResult == IDOK)
	{
		CListBox*	pList = (CListBox*) GetDlgItem(IDC_PLUGIN_LIST);
		ASSERT_VALID(pList);

		for (POSITION pos = dlg.GetStartPosition(); pos; )
		{
			CString	strFile = dlg.GetNextPathName(pos);

			// If file not already in list
			if (pList->FindStringExact(-1, strFile) == LB_ERR)
				pList->AddString(strFile);
		}

    CString strPluginCount;
    strPluginCount.Format ("%i plugin%s", 
                          pList->GetCount(),
                          pList->GetCount() == 1 ? "" : "s");

		SetDlgItemText(IDC_PLUGIN_COUNT, strPluginCount);
	}
	else if (CommDlgExtendedError() == FNERR_BUFFERTOOSMALL)
		// To many files - buffer not large enough
		TMessageBox("You have selected too many plugins to add.  Please try again with fewer Plugins.");

	UpdateDialogControls(this, TRUE);
	
}

void CGlobalPrefsP12::OnRemovePlugin() 
{
	CListBox*	pList = (CListBox*) GetDlgItem(IDC_PLUGIN_LIST);
	ASSERT_VALID(pList);
	
	int	nIndex = pList->GetCurSel();
	pList->DeleteString(nIndex);

	if (nIndex >= pList->GetCount())
		nIndex = pList->GetCount() - 1;
	pList->SetCurSel(nIndex);

  CString strPluginCount;
  strPluginCount.Format ("%i plugin%s", 
                        pList->GetCount(),
                        pList->GetCount() == 1 ? "" : "s");

	SetDlgItemText(IDC_PLUGIN_COUNT, strPluginCount);
	UpdateDialogControls(this, TRUE);
	
}

LRESULT CGlobalPrefsP12::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CPrefsP12::OnKickIdle


void CGlobalPrefsP12::OnUpdateNeedSelection(CCmdUI* pCmdUI)
{
	CListBox*	pList = (CListBox*) GetDlgItem(IDC_PLUGIN_LIST);
	pCmdUI->Enable(pList && pList->GetCurSel() != LB_ERR);
}   // end of CGlobalPrefsP12::OnUpdateNeedSelection


void CGlobalPrefsP12::OnUpdateSelectedPlugin(CCmdUI* pCmdUI)
{
	CListBox*	pList = (CListBox*) GetDlgItem(IDC_PLUGIN_LIST);
  int nIndex = pList->GetCurSel();

  if (nIndex == LB_ERR)
    pCmdUI->SetText ("");
  else
    {
    CString strItem; 
    pList->GetText (nIndex, strItem);
  	pCmdUI->SetText(strItem);
    }
}  // end of CGlobalPrefsP12::OnUpdateSelectedPlugin

BOOL CGlobalPrefsP12::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	CListBox*	pList = (CListBox*) GetDlgItem(IDC_PLUGIN_LIST);
	
	CRect	rect(0, 0, _MAX_PATH, 1);
	MapDialogRect(rect);
	pList->SetHorizontalExtent(rect.Width());

// add to list here
  
  while (!m_strPluginList.IsEmpty ())
    {
    int i = m_strPluginList.Find ('*');
    if (i == -1)
      {
      pList->AddString (m_strPluginList);
      break;
      }
    else
      {
      pList->AddString (m_strPluginList.Left (i));
      m_strPluginList = m_strPluginList.Mid (i + 1);
      }
    }

  CString strPluginCount;
  strPluginCount.Format ("%i plugin%s", 
                        pList->GetCount(),
                        pList->GetCount() == 1 ? "" : "s");

	SetDlgItemText(IDC_PLUGIN_COUNT, strPluginCount);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGlobalPrefsP12::OnOK() 
{
	CListBox*	pList = (CListBox*) GetDlgItem(IDC_PLUGIN_LIST);
	
  m_strPluginList.Empty ();
  CString strItem;

// make a long string containing all plugin names, separated by asterisks

  for (int i = 0; i < pList->GetCount (); i++)
    {
    pList->GetText (i, strItem);
    if (!m_strPluginList.IsEmpty ())
      m_strPluginList += '*';
    m_strPluginList += strItem;
    }

}

void CGlobalPrefsP12::OnMoveUp() 
{
	CListBox*	pList = (CListBox*) GetDlgItem(IDC_PLUGIN_LIST);
	
  CString strItem;

	int	nIndex = pList->GetCurSel();

  // if no selection, or at top, give up
  if (nIndex == LB_ERR || nIndex == 0)
    return;

  // get item
  pList->GetText (nIndex, strItem);

  // now get rid of it
	pList->DeleteString(nIndex);

  // re-insert it one up

  nIndex = pList->InsertString (nIndex - 1, strItem);

  // re-select it
  pList->SetCurSel (nIndex);
  
	
}

void CGlobalPrefsP12::OnMoveDown() 
{
	CListBox*	pList = (CListBox*) GetDlgItem(IDC_PLUGIN_LIST);
	
  CString strItem;

	int	nIndex = pList->GetCurSel();

  // if no selection, or at bottom, give up
  if (nIndex == LB_ERR || nIndex >= (pList->GetCount () - 1))
    return;

  // get item
  pList->GetText (nIndex, strItem);

  // now get rid of it
	pList->DeleteString(nIndex);

  // re-insert it one down
  nIndex = pList->InsertString (nIndex + 1, strItem);

  // re-select it
  pList->SetCurSel (nIndex);
	
}
/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP13 property page

CGlobalPrefsP13::CGlobalPrefsP13() : CPropertyPage(CGlobalPrefsP13::IDD)
{
	//{{AFX_DATA_INIT(CGlobalPrefsP13)
	m_strLuaScript = _T("");
	m_bEnablePackageLibrary = FALSE;
	//}}AFX_DATA_INIT
}

CGlobalPrefsP13::~CGlobalPrefsP13()
{
}

void CGlobalPrefsP13::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalPrefsP13)
	DDX_Control(pDX, IDC_LUA_SCRIPT, m_ctlScript);
	DDX_Text(pDX, IDC_LUA_SCRIPT, m_strLuaScript);
	DDX_Check(pDX, IDC_ENABLE_PACKAGE_LIBRARY, m_bEnablePackageLibrary);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGlobalPrefsP13, CPropertyPage)
	//{{AFX_MSG_MAP(CGlobalPrefsP13)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP13 message handlers

void CGlobalPrefsP13::OnEdit() 
{
CEditMultiLine dlg;

  dlg.m_strText = GetText (m_ctlScript);

  dlg.m_strTitle = "Edit Lua initial code";
  
  dlg.m_bScript = true;
  dlg.m_bLua = true;

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlScript.SetWindowText (dlg.m_strText);
	
}

LRESULT CGlobalPrefsP13::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CGlobalPrefsP13::OnKickIdle
