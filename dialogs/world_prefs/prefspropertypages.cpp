// prefsp1.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\mainfrm.h"
#include "..\..\doc.h"
#include "..\..\TextDocument.h"
#include "..\..\MUSHview.h"
#include "..\..\sendvw.h"
#include "..\..\scripting\errors.h"
#include "..\..\childfrm.h"

#include "editmac.h"
#include "aliasdlg.h"
#include "triggdlg.h"
#include "TimerDlg.h"
#include "EditVariable.h"
#include "..\FindDlg.h"
#include "TabDefaultsDlg.h"
#include "TreePropertySheet.h"
#include "..\Splash.h"
#include "CommandOptionsDlg.h"
#include "..\MXPscriptRoutinesDlg.h"
#include "..\ColourPickerDlg.h"
#include "..\EditMultiLine.h"
#include "..\CreditsDlg.h"
#include "..\ProxyServerPasswordDlg.h"
#include "..\ProgDlg.h"

//#define new DEBUG_NEW 

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable : 4800)  // forcing value to bool 'true' or 'false' (performance warning)
bool NotFound (CFindInfo & FindInfo);

#define NL "\r\n"

static inline short get_style (int style)
  {

  return style & 0x0F;

  } // end of get_style

 // - see copy in mushview.cpp

static inline short get_foreground (int style)
  {

  return (style >> 4) & 0x07;

  } // end of get_foreground

static inline short get_background (int style)
  {

  return (style >> 8) & 0x07;

  } // end of get_background

// The field below is needed to initialise the browse directory dialog 
// with the initial directory

extern CString strStartingDirectory;

// function prototypes needed for folder browsing

int __stdcall BrowseCallbackProc(
    HWND hwnd, 	
    UINT uMsg, 	
    LPARAM lParam, 	
    LPARAM lpData	
   );
void RemoveTrailingBackslash (CString& str);

/////////////////////////////////////////////////////////////////////////////
// CPrefsP1 property page

IMPLEMENT_DYNCREATE(CPrefsP1, CPropertyPage)

CPrefsP1::CPrefsP1() : CPropertyPage(CPrefsP1::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP1)
	m_port = 0;
	m_mush_name = _T("");
	m_server = _T("");
	m_bSaveWorldAutomatically = FALSE;
	m_strMudListAddress = _T("");
	m_strBugReportAddress = _T("");
	m_iSocksProcessing = -1;
	m_strProxyServerName = _T("");
	m_iProxyServerPort = 0;
	//}}AFX_DATA_INIT
  m_page_number = 1;
  m_doc = NULL;
}

CPrefsP1::~CPrefsP1()
{
}

void CPrefsP1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP1)
	DDX_Text(pDX, IDC_CHANNEL, m_port);
	DDV_MinMaxLong(pDX, m_port, 0, 65535);
	DDX_Text(pDX, IDC_MUSH_NAME, m_mush_name);
	DDX_Text(pDX, IDC_SERVER, m_server);
	DDX_Check(pDX, IDC_SAVE_AUTOMATICALLY, m_bSaveWorldAutomatically);
	DDX_Text(pDX, IDC_MUD_LISTS, m_strMudListAddress);
	DDX_Text(pDX, IDC_BUG_REPORT, m_strBugReportAddress);
	DDX_CBIndex(pDX, IDC_PROXY_TYPE, m_iSocksProcessing);
	DDX_Text(pDX, IDC_PROXY_SERVER_NAME, m_strProxyServerName);
	DDX_Text(pDX, IDC_PROXY_PORT, m_iProxyServerPort);
	DDV_MinMaxLong(pDX, m_iProxyServerPort, 0, 65535);
	//}}AFX_DATA_MAP


   if(pDX->m_bSaveAndValidate)
   {

      m_mush_name.TrimLeft (); // don't let them get away with a single space
      m_mush_name.TrimRight ();

      if(m_mush_name.IsEmpty ())
          {
          ::TMessageBox("Your world name cannot be blank.\n\n"
                          "You must fill in your world name, TCP/IP address and "
                          "port number before tabbing to other configuration screens");
          DDX_Text(pDX, IDC_MUSH_NAME, m_mush_name);
          pDX->Fail();
          }     // end of MUSH name being blank

      m_server.TrimLeft (); // don't let them get away with a single space
      m_server.TrimRight ();

      if(m_server.IsEmpty ())
          {
          ::TMessageBox("The world IP address cannot be blank.");
          DDX_Text(pDX, IDC_SERVER, m_server);
          pDX->Fail();
          }     // end of server name being blank

      if (m_port == 0)
          {
          ::TMessageBox("The world port number must be specified.");
          DDX_Text(pDX, IDC_CHANNEL, m_port);
          pDX->Fail();
          }     // end of port being zero

      m_strProxyServerName.TrimLeft ();
      m_strProxyServerName.TrimRight ();

      switch (m_iSocksProcessing)
        {
        case eProxyServerNone:
          break;    // nothing needed here

        case eProxyServerSocks4:     // socks 4
        case eProxyServerSocks5:     // socks 5
          if (m_strProxyServerName.IsEmpty ())
              {
              ::TMessageBox("The proxy server address cannot be blank.");
              DDX_Text(pDX, IDC_PROXY_SERVER_NAME, m_strProxyServerName);
              pDX->Fail();
              }     // end of proxy server name being blank

          if (m_iProxyServerPort == 0)
              {
              ::TMessageBox("The proxy server port must be specified.");
              DDX_Text(pDX, IDC_PROXY_PORT, m_iProxyServerPort);
              pDX->Fail();
              }     // end of proxy server port being zero
          break;

        default:
              ::TMessageBox("Unknown proxy server type.");
              pDX->Fail();
              break;

        } // end of switch

    }   // end of saving and validating

}

BOOL CPrefsP1::OnInitDialog() 
{

	// subclass static controls.
	m_MudListLink.SubclassDlgItem(IDC_MUD_LISTS,  this);
  m_strMudListAddress = MUD_LIST;
	
	m_BugReportLink.SubclassDlgItem(IDC_BUG_REPORT,  this);
  m_strBugReportAddress = BUG_REPORT_PAGE;

  return CPropertyPage::OnInitDialog();
	
}


BEGIN_MESSAGE_MAP(CPrefsP1, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP1)
	ON_BN_CLICKED(IDC_PROXY_SERVER_PASSWORD, OnProxyServerPassword)
	ON_BN_CLICKED(IDC_CLEAR_CACHED, OnClearCached)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_UPDATE_COMMAND_UI(IDC_CLEAR_CACHED, OnUpdateCanClearCache)
END_MESSAGE_MAP()


void CPrefsP1::OnProxyServerPassword() 
{
CProxyServerPasswordDlg dlg;

  dlg.m_strProxyUserName = m_strProxyUserName;	
  dlg.m_strProxyPassword = m_strProxyPassword;

  if (dlg.DoModal () != IDOK)
    return;

  m_strProxyUserName = dlg.m_strProxyUserName;	
  m_strProxyPassword = dlg.m_strProxyPassword;
  
}


void CPrefsP1::OnClearCached() 
{
m_doc->ResetIP ();	
}

LRESULT CPrefsP1::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CPrefsP1::OnKickIdle

void CPrefsP1::OnUpdateCanClearCache(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_doc->m_sockAddr.sin_addr.s_addr != 0 ||
                  m_doc->m_ProxyAddr.sin_addr.s_addr != 0);

  } // end of CPrefsP1::OnUpdateCanClearCache



/////////////////////////////////////////////////////////////////////////////
// CPrefsP1 message handlers


/////////////////////////////////////////////////////////////////////////////
// CPrefsP2 property page

IMPLEMENT_DYNCREATE(CPrefsP2, CPropertyPage)

CPrefsP2::CPrefsP2() : CPropertyPage(CPrefsP2::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP2)
	m_sound_pathname = _T("");
	//}}AFX_DATA_INIT
  m_page_number = 1;
  m_doc = NULL;
}

CPrefsP2::~CPrefsP2()
{
}

void CPrefsP2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP2)
	DDX_Text(pDX, IDC_SOUND_PATH, m_sound_pathname);
	//}}AFX_DATA_MAP

}


BEGIN_MESSAGE_MAP(CPrefsP2, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP2)
	ON_BN_CLICKED(IDC_BROWSE_SOUNDS, OnBrowseSounds)
	ON_BN_CLICKED(IDC_TEST_SOUND, OnTestSound)
	ON_BN_CLICKED(IDC_NO_SOUND, OnNoSound)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPrefsP2 message handlers

void CPrefsP2::OnBrowseSounds() 
{
}

void CPrefsP2::OnTestSound() 
{
}

void CPrefsP2::OnNoSound() 
{
}


LRESULT CPrefsP2::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CPrefsP2::OnKickIdle

void CPrefsP2::OnUpdateNeedSound(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (!m_sound_pathname.IsEmpty () && m_sound_pathname != NOSOUNDLIT);
  } // end of CPrefsP2::OnUpdateNeedSound

/////////////////////////////////////////////////////////////////////////////
// CPrefsP3 property page

IMPLEMENT_DYNCREATE(CPrefsP3, CPropertyPage)

CPrefsP3::CPrefsP3() : CPropertyPage(CPrefsP3::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP3)
	//}}AFX_DATA_INIT
  m_page_number = 1;

  m_RH_click_count = 0;
  m_doc = NULL;
  m_prefsp5 = NULL;

}

CPrefsP3::~CPrefsP3()
{
}

void CPrefsP3::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP3)
	DDX_Control(pDX, IDC_CUSTOM_NAME9, m_ctlCustomName9);
	DDX_Control(pDX, IDC_CUSTOM_NAME8, m_ctlCustomName8);
	DDX_Control(pDX, IDC_CUSTOM_NAME7, m_ctlCustomName7);
	DDX_Control(pDX, IDC_CUSTOM_NAME6, m_ctlCustomName6);
	DDX_Control(pDX, IDC_CUSTOM_NAME5, m_ctlCustomName5);
	DDX_Control(pDX, IDC_CUSTOM_NAME4, m_ctlCustomName4);
	DDX_Control(pDX, IDC_CUSTOM_NAME3, m_ctlCustomName3);
	DDX_Control(pDX, IDC_CUSTOM_NAME2, m_ctlCustomName2);
	DDX_Control(pDX, IDC_CUSTOM_NAME16, m_ctlCustomName16);
	DDX_Control(pDX, IDC_CUSTOM_NAME15, m_ctlCustomName15);
	DDX_Control(pDX, IDC_CUSTOM_NAME14, m_ctlCustomName14);
	DDX_Control(pDX, IDC_CUSTOM_NAME13, m_ctlCustomName13);
	DDX_Control(pDX, IDC_CUSTOM_NAME12, m_ctlCustomName12);
	DDX_Control(pDX, IDC_CUSTOM_NAME11, m_ctlCustomName11);
	DDX_Control(pDX, IDC_CUSTOM_NAME10, m_ctlCustomName10);
	DDX_Control(pDX, IDC_CUSTOM_NAME1, m_ctlCustomName1);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM9, m_ctlCustom9);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM8, m_ctlCustom8);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM7, m_ctlCustom7);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM6, m_ctlCustom6);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM5, m_ctlCustom5);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM4, m_ctlCustom4);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM32, m_ctlCustom32);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM31, m_ctlCustom31);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM30, m_ctlCustom30);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM3, m_ctlCustom3);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM29, m_ctlCustom29);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM28, m_ctlCustom28);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM27, m_ctlCustom27);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM26, m_ctlCustom26);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM25, m_ctlCustom25);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM24, m_ctlCustom24);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM23, m_ctlCustom23);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM22, m_ctlCustom22);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM21, m_ctlCustom21);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM20, m_ctlCustom20);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM2, m_ctlCustom2);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM19, m_ctlCustom19);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM18, m_ctlCustom18);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM17, m_ctlCustom17);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM16, m_ctlCustom16);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM15, m_ctlCustom15);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM14, m_ctlCustom14);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM13, m_ctlCustom13);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM12, m_ctlCustom12);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM11, m_ctlCustom11);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM10, m_ctlCustom10);
	DDX_Control(pDX, IDC_SWATCH_CUSTOM1, m_ctlCustom1);
	DDX_Control(pDX, IDC_YELLOW_SWATCH_BOLD, m_ctlYellowSwatchBold);
	DDX_Control(pDX, IDC_YELLOW_SWATCH, m_ctlYellowSwatch);
	DDX_Control(pDX, IDC_WHITE_SWATCH_BOLD, m_ctlWhiteSwatchBold);
	DDX_Control(pDX, IDC_WHITE_SWATCH, m_ctlWhiteSwatch);
	DDX_Control(pDX, IDC_RED_SWATCH_BOLD, m_ctlRedSwatchBold);
	DDX_Control(pDX, IDC_RED_SWATCH, m_ctlRedSwatch);
	DDX_Control(pDX, IDC_MAGENTA_SWATCH_BOLD, m_ctlMagentaSwatchBold);
	DDX_Control(pDX, IDC_MAGENTA_SWATCH, m_ctlMagentaSwatch);
	DDX_Control(pDX, IDC_GREEN_SWATCH_BOLD, m_ctlGreenSwatchBold);
	DDX_Control(pDX, IDC_GREEN_SWATCH, m_ctlGreenSwatch);
	DDX_Control(pDX, IDC_CYAN_SWATCH_BOLD, m_ctlCyanSwatchBold);
	DDX_Control(pDX, IDC_CYAN_SWATCH, m_ctlCyanSwatch);
	DDX_Control(pDX, IDC_BLUE_SWATCH_BOLD, m_ctlBlueSwatchBold);
	DDX_Control(pDX, IDC_BLUE_SWATCH, m_ctlBlueSwatch);
	DDX_Control(pDX, IDC_BLACK_SWATCH_BOLD, m_ctlBlackSwatchBold);
	DDX_Control(pDX, IDC_BLACK_SWATCH, m_ctlBlackSwatch);
	//}}AFX_DATA_MAP

	DDX_MinMaxString(pDX, IDC_CUSTOM_NAME1, m_strCustomName [0]);
	DDV_MinMaxString(pDX, m_strCustomName [0], 1, 30);
	DDX_MinMaxString(pDX, IDC_CUSTOM_NAME2, m_strCustomName [1]);
	DDV_MinMaxString(pDX, m_strCustomName [1], 1, 30);
	DDX_MinMaxString(pDX, IDC_CUSTOM_NAME3, m_strCustomName [2]);
	DDV_MinMaxString(pDX, m_strCustomName [2], 1, 30);
	DDX_MinMaxString(pDX, IDC_CUSTOM_NAME4, m_strCustomName [3]);
	DDV_MinMaxString(pDX, m_strCustomName [3], 1, 30);
	DDX_MinMaxString(pDX, IDC_CUSTOM_NAME5, m_strCustomName [4]);
	DDV_MinMaxString(pDX, m_strCustomName [4], 1, 30);
	DDX_MinMaxString(pDX, IDC_CUSTOM_NAME6, m_strCustomName [5]);
	DDV_MinMaxString(pDX, m_strCustomName [5], 1, 30);
	DDX_MinMaxString(pDX, IDC_CUSTOM_NAME7, m_strCustomName [6]);
	DDV_MinMaxString(pDX, m_strCustomName [6], 1, 30);
	DDX_MinMaxString(pDX, IDC_CUSTOM_NAME8, m_strCustomName [7]);
	DDV_MinMaxString(pDX, m_strCustomName [7], 1, 30);
	DDX_MinMaxString(pDX, IDC_CUSTOM_NAME9, m_strCustomName [8]);
	DDV_MinMaxString(pDX, m_strCustomName [8], 1, 30);
	DDX_MinMaxString(pDX, IDC_CUSTOM_NAME10, m_strCustomName [9]);
	DDV_MinMaxString(pDX, m_strCustomName [9], 1, 30);
	DDX_MinMaxString(pDX, IDC_CUSTOM_NAME11, m_strCustomName [10]);
	DDV_MinMaxString(pDX, m_strCustomName [10], 1, 30);
	DDX_MinMaxString(pDX, IDC_CUSTOM_NAME12, m_strCustomName [11]);
	DDV_MinMaxString(pDX, m_strCustomName [11], 1, 30);
	DDX_MinMaxString(pDX, IDC_CUSTOM_NAME13, m_strCustomName [12]);
	DDV_MinMaxString(pDX, m_strCustomName [12], 1, 30);
	DDX_MinMaxString(pDX, IDC_CUSTOM_NAME14, m_strCustomName [13]);
	DDV_MinMaxString(pDX, m_strCustomName [13], 1, 30);
	DDX_MinMaxString(pDX, IDC_CUSTOM_NAME15, m_strCustomName [14]);
	DDV_MinMaxString(pDX, m_strCustomName [14], 1, 30);
	DDX_MinMaxString(pDX, IDC_CUSTOM_NAME16, m_strCustomName [15]);
	DDV_MinMaxString(pDX, m_strCustomName [15], 1, 30);

}


BEGIN_MESSAGE_MAP(CPrefsP3, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP3)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM1, OnSwatchCustom1)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM2, OnSwatchCustom2)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM10, OnSwatchCustom10)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM11, OnSwatchCustom11)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM12, OnSwatchCustom12)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM13, OnSwatchCustom13)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM14, OnSwatchCustom14)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM15, OnSwatchCustom15)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM16, OnSwatchCustom16)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM17, OnSwatchCustom17)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM18, OnSwatchCustom18)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM19, OnSwatchCustom19)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM20, OnSwatchCustom20)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM21, OnSwatchCustom21)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM22, OnSwatchCustom22)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM23, OnSwatchCustom23)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM24, OnSwatchCustom24)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM25, OnSwatchCustom25)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM26, OnSwatchCustom26)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM27, OnSwatchCustom27)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM28, OnSwatchCustom28)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM29, OnSwatchCustom29)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM3, OnSwatchCustom3)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM30, OnSwatchCustom30)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM31, OnSwatchCustom31)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM32, OnSwatchCustom32)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM4, OnSwatchCustom4)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM5, OnSwatchCustom5)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM6, OnSwatchCustom6)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM7, OnSwatchCustom7)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM8, OnSwatchCustom8)
	ON_BN_CLICKED(IDC_SWATCH_CUSTOM9, OnSwatchCustom9)
	ON_WM_RBUTTONDOWN()
	ON_BN_CLICKED(IDC_INVERT, OnInvert)
	ON_BN_CLICKED(IDC_ALL_LIGHTER, OnAllLighter)
	ON_BN_CLICKED(IDC_ALL_DARKER, OnAllDarker)
	ON_BN_CLICKED(IDC_ALL_MORE_COLOUR, OnAllMoreColour)
	ON_BN_CLICKED(IDC_ALL_LESS_COLOUR, OnAllLessColour)
	ON_BN_CLICKED(IDC_DEFAULTS, OnDefaults)
	ON_BN_CLICKED(IDC_SWAP, OnSwap)
	ON_BN_CLICKED(IDC_RANDOM, OnRandom)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM1, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM2, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM10, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM11, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM12, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM13, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM14, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM15, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM16, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM17, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM18, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM19, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM20, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM21, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM22, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM23, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM24, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM25, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM26, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM27, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM28, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM29, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM3, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM30, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM31, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM32, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM4, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM5, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM6, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM7, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM8, OnUpdateNotUsingDefaults)
	ON_UPDATE_COMMAND_UI(IDC_SWATCH_CUSTOM9, OnUpdateNotUsingDefaults)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPrefsP3 message handlers


void CPrefsP3::amendcolour (CColourButton & whichcolour)
  {

  CColourPickerDlg dlg;
  dlg.m_iColour = whichcolour.m_colour;
  dlg.m_bPickColour = true;

  /*
CColorDialog dlg (whichcolour.m_colour, 
                  CC_RGBINIT | CC_FULLOPEN, 
                  GetOwner());
  
  */

  if (dlg.DoModal () != IDOK)
    return;

  whichcolour.m_colour = dlg.GetColor ();
  whichcolour.RedrawWindow();

  } // end of amendcolour



BOOL CPrefsP3::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
  LoadSwatches ();

  bool bEnable = !m_prefsp5->m_bUseDefaultColours ||
                  App.m_strDefaultColoursFile.IsEmpty ();

  m_ctlCustom1.EnableWindow (bEnable);   
  m_ctlCustom2.EnableWindow (bEnable);   
  m_ctlCustom3.EnableWindow (bEnable);   
  m_ctlCustom4.EnableWindow (bEnable);   
  m_ctlCustom5.EnableWindow (bEnable);   
  m_ctlCustom6.EnableWindow (bEnable);   
  m_ctlCustom7.EnableWindow (bEnable);   
  m_ctlCustom8.EnableWindow (bEnable);   
  m_ctlCustom9.EnableWindow (bEnable);   
  m_ctlCustom10.EnableWindow (bEnable);   
  m_ctlCustom11.EnableWindow (bEnable);   
  m_ctlCustom12.EnableWindow (bEnable);   
  m_ctlCustom13.EnableWindow (bEnable);   
  m_ctlCustom14.EnableWindow (bEnable);   
  m_ctlCustom15.EnableWindow (bEnable);   
  m_ctlCustom16.EnableWindow (bEnable);   
  m_ctlCustom17.EnableWindow (bEnable);   
  m_ctlCustom18.EnableWindow (bEnable);   
  m_ctlCustom19.EnableWindow (bEnable);   
  m_ctlCustom20.EnableWindow (bEnable);   
  m_ctlCustom21.EnableWindow (bEnable);   
  m_ctlCustom22.EnableWindow (bEnable);   
  m_ctlCustom23.EnableWindow (bEnable);   
  m_ctlCustom24.EnableWindow (bEnable);   
  m_ctlCustom25.EnableWindow (bEnable);   
  m_ctlCustom26.EnableWindow (bEnable);   
  m_ctlCustom27.EnableWindow (bEnable);   
  m_ctlCustom28.EnableWindow (bEnable);   
  m_ctlCustom29.EnableWindow (bEnable);   
  m_ctlCustom30.EnableWindow (bEnable);   
  m_ctlCustom31.EnableWindow (bEnable);   
  m_ctlCustom32.EnableWindow (bEnable);   

  m_RH_last_click = CTime::GetCurrentTime ();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CPrefsP3::OnOK( )
  {

    SaveSwatches ();

    CPropertyPage::OnOK ();
    
  }

// save colours when our window becomes inactive, so they show up in other windows

BOOL CPrefsP3::OnKillActive() 
{
  SaveSwatches ();
	
	return CPropertyPage::OnKillActive();
}

void CPrefsP3::SaveSwatches(void)
  {
    
    // custom text colours
    m_customtext [0] = m_ctlCustom1.m_colour;
    m_customtext [1] = m_ctlCustom2.m_colour;
    m_customtext [2] = m_ctlCustom3.m_colour;
    m_customtext [3] = m_ctlCustom4.m_colour;
    m_customtext [4] = m_ctlCustom5.m_colour;
    m_customtext [5] = m_ctlCustom6.m_colour;
    m_customtext [6] = m_ctlCustom7.m_colour;
    m_customtext [7] = m_ctlCustom8.m_colour;
    m_customtext [8] = m_ctlCustom9.m_colour;
    m_customtext [9] = m_ctlCustom10.m_colour;
    m_customtext [10] = m_ctlCustom11.m_colour;
    m_customtext [11] = m_ctlCustom12.m_colour;
    m_customtext [12] = m_ctlCustom13.m_colour;
    m_customtext [13] = m_ctlCustom14.m_colour;
    m_customtext [14] = m_ctlCustom15.m_colour;
    m_customtext [15] = m_ctlCustom16.m_colour;

    // custom background colours
    m_customback [0] = m_ctlCustom17.m_colour;
    m_customback [1] = m_ctlCustom18.m_colour;
    m_customback [2] = m_ctlCustom19.m_colour;
    m_customback [3] = m_ctlCustom20.m_colour;
    m_customback [4] = m_ctlCustom21.m_colour;
    m_customback [5] = m_ctlCustom22.m_colour;
    m_customback [6] = m_ctlCustom23.m_colour;
    m_customback [7] = m_ctlCustom24.m_colour;
    m_customback [8] = m_ctlCustom25.m_colour;
    m_customback [9] = m_ctlCustom26.m_colour;
    m_customback [10] = m_ctlCustom27.m_colour;
    m_customback [11] = m_ctlCustom28.m_colour;
    m_customback [12] = m_ctlCustom29.m_colour;
    m_customback [13] = m_ctlCustom30.m_colour;
    m_customback [14] = m_ctlCustom31.m_colour;
    m_customback [15] = m_ctlCustom32.m_colour;

    m_strCustomName [0]  = GetText (m_ctlCustomName1);
    m_strCustomName [1]  = GetText (m_ctlCustomName2);
    m_strCustomName [2]  = GetText (m_ctlCustomName3);
    m_strCustomName [3]  = GetText (m_ctlCustomName4);
    m_strCustomName [4]  = GetText (m_ctlCustomName5);
    m_strCustomName [5]  = GetText (m_ctlCustomName6);
    m_strCustomName [6]  = GetText (m_ctlCustomName7);
    m_strCustomName [7]  = GetText (m_ctlCustomName8);
    m_strCustomName [8]  = GetText (m_ctlCustomName9);
    m_strCustomName [9]  = GetText (m_ctlCustomName10);
    m_strCustomName [10] = GetText (m_ctlCustomName11);
    m_strCustomName [11] = GetText (m_ctlCustomName12);
    m_strCustomName [12] = GetText (m_ctlCustomName13);
    m_strCustomName [13] = GetText (m_ctlCustomName14);
    m_strCustomName [14] = GetText (m_ctlCustomName15);
    m_strCustomName [15] = GetText (m_ctlCustomName16);

                  
  }


void CPrefsP3::LoadSwatches(void)
  {

    // custom text colours
  m_ctlCustom1.m_colour =  m_customtext [0]; 
  m_ctlCustom2.m_colour =  m_customtext [1]; 
  m_ctlCustom3.m_colour =  m_customtext [2]; 
  m_ctlCustom4.m_colour =  m_customtext [3]; 
  m_ctlCustom5.m_colour =  m_customtext [4]; 
  m_ctlCustom6.m_colour =  m_customtext [5]; 
  m_ctlCustom7.m_colour =  m_customtext [6]; 
  m_ctlCustom8.m_colour =  m_customtext [7]; 
  m_ctlCustom9.m_colour =  m_customtext [8]; 
  m_ctlCustom10.m_colour = m_customtext [9]; 
  m_ctlCustom11.m_colour = m_customtext [10];
  m_ctlCustom12.m_colour = m_customtext [11];
  m_ctlCustom13.m_colour = m_customtext [12];
  m_ctlCustom14.m_colour = m_customtext [13];
  m_ctlCustom15.m_colour = m_customtext [14];
  m_ctlCustom16.m_colour = m_customtext [15];

    // custom background colours
  m_ctlCustom17.m_colour = m_customback [0]; 
  m_ctlCustom18.m_colour = m_customback [1]; 
  m_ctlCustom19.m_colour = m_customback [2]; 
  m_ctlCustom20.m_colour = m_customback [3]; 
  m_ctlCustom21.m_colour = m_customback [4]; 
  m_ctlCustom22.m_colour = m_customback [5]; 
  m_ctlCustom23.m_colour = m_customback [6]; 
  m_ctlCustom24.m_colour = m_customback [7]; 
  m_ctlCustom25.m_colour = m_customback [8]; 
  m_ctlCustom26.m_colour = m_customback [9]; 
  m_ctlCustom27.m_colour = m_customback [10];
  m_ctlCustom28.m_colour = m_customback [11];
  m_ctlCustom29.m_colour = m_customback [12];
  m_ctlCustom30.m_colour = m_customback [13];
  m_ctlCustom31.m_colour = m_customback [14];
  m_ctlCustom32.m_colour = m_customback [15];


  m_ctlCustomName1.SetWindowText  (m_strCustomName [0]); 
  m_ctlCustomName2.SetWindowText  (m_strCustomName [1]); 
  m_ctlCustomName3.SetWindowText  (m_strCustomName [2]); 
  m_ctlCustomName4.SetWindowText  (m_strCustomName [3]); 
  m_ctlCustomName5.SetWindowText  (m_strCustomName [4]); 
  m_ctlCustomName6.SetWindowText  (m_strCustomName [5]); 
  m_ctlCustomName7.SetWindowText  (m_strCustomName [6]); 
  m_ctlCustomName8.SetWindowText  (m_strCustomName [7]); 
  m_ctlCustomName9.SetWindowText  (m_strCustomName [8]); 
  m_ctlCustomName10.SetWindowText (m_strCustomName [9]); 
  m_ctlCustomName11.SetWindowText (m_strCustomName [10]);
  m_ctlCustomName12.SetWindowText (m_strCustomName [11]);
  m_ctlCustomName13.SetWindowText (m_strCustomName [12]);
  m_ctlCustomName14.SetWindowText (m_strCustomName [13]);
  m_ctlCustomName15.SetWindowText (m_strCustomName [14]);
  m_ctlCustomName16.SetWindowText (m_strCustomName [15]);

  
  // ansi colours

  m_ctlBlackSwatch.m_colour       = m_prefsp5->m_normalcolour [BLACK];      
  m_ctlBlueSwatch.m_colour        = m_prefsp5->m_normalcolour [BLUE];       
  m_ctlCyanSwatch.m_colour        = m_prefsp5->m_normalcolour [CYAN];       
  m_ctlGreenSwatch.m_colour       = m_prefsp5->m_normalcolour [GREEN];      
  m_ctlMagentaSwatch.m_colour     = m_prefsp5->m_normalcolour [MAGENTA];    
  m_ctlRedSwatch.m_colour         = m_prefsp5->m_normalcolour [RED];        
  m_ctlWhiteSwatch.m_colour       = m_prefsp5->m_normalcolour [WHITE];      
  m_ctlYellowSwatch.m_colour      = m_prefsp5->m_normalcolour [YELLOW];     
                                  
  m_ctlBlackSwatchBold.m_colour   = m_prefsp5->m_boldcolour [BLACK];      
  m_ctlBlueSwatchBold.m_colour    = m_prefsp5->m_boldcolour [BLUE];       
  m_ctlCyanSwatchBold.m_colour    = m_prefsp5->m_boldcolour [CYAN];       
  m_ctlGreenSwatchBold.m_colour   = m_prefsp5->m_boldcolour [GREEN];      
  m_ctlMagentaSwatchBold.m_colour = m_prefsp5->m_boldcolour [MAGENTA];    
  m_ctlRedSwatchBold.m_colour     = m_prefsp5->m_boldcolour [RED];        
  m_ctlWhiteSwatchBold.m_colour   = m_prefsp5->m_boldcolour [WHITE];      
  m_ctlYellowSwatchBold.m_colour  = m_prefsp5->m_boldcolour [YELLOW];     

  }



void CPrefsP3::OnSwatchCustom1() 
{
  amendcolour (m_ctlCustom1);	
}

void CPrefsP3::OnSwatchCustom2() 
{
  amendcolour (m_ctlCustom2);
}

void CPrefsP3::OnSwatchCustom10() 
{
  amendcolour (m_ctlCustom10);
}

void CPrefsP3::OnSwatchCustom11() 
{
  amendcolour (m_ctlCustom11);
}

void CPrefsP3::OnSwatchCustom12() 
{
  amendcolour (m_ctlCustom12);
}

void CPrefsP3::OnSwatchCustom13() 
{
  amendcolour (m_ctlCustom13);
}

void CPrefsP3::OnSwatchCustom14() 
{
  amendcolour (m_ctlCustom14);
}

void CPrefsP3::OnSwatchCustom15() 
{
  amendcolour (m_ctlCustom15);
}

void CPrefsP3::OnSwatchCustom16() 
{
  amendcolour (m_ctlCustom16);
}

void CPrefsP3::OnSwatchCustom17() 
{
  amendcolour (m_ctlCustom17);
}

void CPrefsP3::OnSwatchCustom18() 
{
  amendcolour (m_ctlCustom18);
}

void CPrefsP3::OnSwatchCustom19() 
{
  amendcolour (m_ctlCustom19);
}

void CPrefsP3::OnSwatchCustom20() 
{
  amendcolour (m_ctlCustom20);
}

void CPrefsP3::OnSwatchCustom21() 
{
  amendcolour (m_ctlCustom21);
}

void CPrefsP3::OnSwatchCustom22() 
{
  amendcolour (m_ctlCustom22);
}

void CPrefsP3::OnSwatchCustom23() 
{
  amendcolour (m_ctlCustom23);
}

void CPrefsP3::OnSwatchCustom24() 
{
  amendcolour (m_ctlCustom24);
}

void CPrefsP3::OnSwatchCustom25() 
{
  amendcolour (m_ctlCustom25);
}

void CPrefsP3::OnSwatchCustom26() 
{
  amendcolour (m_ctlCustom26);
}

void CPrefsP3::OnSwatchCustom27() 
{
  amendcolour (m_ctlCustom27);
}

void CPrefsP3::OnSwatchCustom28() 
{
  amendcolour (m_ctlCustom28);
}

void CPrefsP3::OnSwatchCustom29() 
{
  amendcolour (m_ctlCustom29);
}

void CPrefsP3::OnSwatchCustom3() 
{
  amendcolour (m_ctlCustom3);
}

void CPrefsP3::OnSwatchCustom30() 
{
  amendcolour (m_ctlCustom30);
}

void CPrefsP3::OnSwatchCustom31() 
{
  amendcolour (m_ctlCustom31);
}

void CPrefsP3::OnSwatchCustom32() 
{
  amendcolour (m_ctlCustom32);
}

void CPrefsP3::OnSwatchCustom4() 
{
  amendcolour (m_ctlCustom4);
}

void CPrefsP3::OnSwatchCustom5() 
{
  amendcolour (m_ctlCustom5);
}

void CPrefsP3::OnSwatchCustom6() 
{
  amendcolour (m_ctlCustom6);
}

void CPrefsP3::OnSwatchCustom7() 
{
  amendcolour (m_ctlCustom7);
}

void CPrefsP3::OnSwatchCustom8() 
{
  amendcolour (m_ctlCustom8);
}

void CPrefsP3::OnSwatchCustom9() 
{
  amendcolour (m_ctlCustom9);
}

// Easter egg - click slowly or it will count as a double-click

// Instructions: *slowly* control-RH-click anywhere on *custom* colours page *except* a button
// or other control. On every third click you will see the message.

void CPrefsP3::OnRButtonDown(UINT nFlags, CPoint point) 
{
  if (nFlags & MK_CONTROL)
    {
    CTime now = CTime::GetCurrentTime ();

    // make sure each click occurs within 5 seconds
    if (now.GetTime () > (m_RH_last_click.GetTime () + 5))
      m_RH_click_count = 0;
    m_RH_last_click = now;

    if ((++m_RH_click_count % 3) == 0)
      {
//      ::TMessageBox ("But does it get goat's blood out?", MB_YESNO |MB_ICONQUESTION);
//      CEasterEggDlg dlg;
//      dlg.DoModal ();
    	CSplashWnd::ShowSplashScreen(&Frame, IDB_NICK);

      }
	   }  // end of ctrl+RH-click
	CPropertyPage::OnRButtonDown(nFlags, point);
}


LRESULT CPrefsP3::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CPrefsP3::OnKickIdle

void CPrefsP3::OnUpdateNotUsingDefaults(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_prefsp5->m_ctlUseDefaultColours.GetCheck () == 0 ||
                  App.m_strDefaultColoursFile.IsEmpty ());
  } // end of CPrefsP3::OnUpdateNotUsingDefaults

void CPrefsP3::OnInvert() 
{
int i;

  SaveSwatches ();

  for (i = 0; i < MAX_CUSTOM; i++)
    {
    m_customtext [i] = ::AdjustColour (m_customtext [i], ADJUST_COLOUR_INVERT);
    m_customback [i] = ::AdjustColour (m_customback [i], ADJUST_COLOUR_INVERT);
    }


  LoadSwatches ();
  Invalidate (false);
	
}

void CPrefsP3::OnAllLighter() 
{
int i;

  SaveSwatches ();

  for (i = 0; i < MAX_CUSTOM; i++)
    {
    m_customtext [i] = ::AdjustColour (m_customtext [i], ADJUST_COLOUR_LIGHTER);
    m_customback [i] = ::AdjustColour (m_customback [i], ADJUST_COLOUR_LIGHTER);
    }
	
  LoadSwatches ();
  Invalidate (false);

}

void CPrefsP3::OnAllDarker() 
{
int i;

  SaveSwatches ();

  for (i = 0; i < MAX_CUSTOM; i++)
    {
    m_customtext [i] = ::AdjustColour (m_customtext [i], ADJUST_COLOUR_DARKER);
    m_customback [i] = ::AdjustColour (m_customback [i], ADJUST_COLOUR_DARKER);
    }
	
  LoadSwatches ();
  Invalidate (false);
	
}

void CPrefsP3::OnAllMoreColour() 
{
int i;

  SaveSwatches ();

  for (i = 0; i < MAX_CUSTOM; i++)
    {
    m_customtext [i] = ::AdjustColour (m_customtext [i], ADJUST_COLOUR_MORE_COLOUR);
    m_customback [i] = ::AdjustColour (m_customback [i], ADJUST_COLOUR_MORE_COLOUR);
    }
	
  LoadSwatches ();
  Invalidate (false);
	
}

void CPrefsP3::OnAllLessColour() 
{
int i;

  SaveSwatches ();

  for (i = 0; i < MAX_CUSTOM; i++)
    {
    m_customtext [i] = ::AdjustColour (m_customtext [i], ADJUST_COLOUR_LESS_COLOUR);
    m_customback [i] = ::AdjustColour (m_customback [i], ADJUST_COLOUR_LESS_COLOUR);
    }
	
  LoadSwatches ();
  Invalidate (false);
	
}

void CPrefsP3::OnDefaults() 
{
  if (::TMessageBox ("Reset all custom colours to MUSHclient defaults?", 
          MB_YESNO | MB_ICONQUESTION) != IDYES)
    return;

  SetDefaultCustomColours (m_customtext, m_customback);

  LoadSwatches ();
  Invalidate (false);
	
}

void CPrefsP3::OnSwap() 
{
  SaveSwatches ();
	
  for (int i = 0; i < MAX_CUSTOM; i++)
    {
    COLORREF temp;
    temp = m_customtext [i];
    m_customtext [i] = m_customback [i];
    m_customback [i] = temp;
    }

  LoadSwatches ();
  Invalidate (false);

}

void CPrefsP3::OnRandom() 
{
  if (::TMessageBox ("Make all colours random?", 
          MB_YESNO | MB_ICONQUESTION) != IDYES)
  return;

  for (int i = 0; i < MAX_CUSTOM; i++)
    {
    m_customtext [i]  = genrand () * (double) 0x1000000;
    m_customback [i]  = genrand () * (double) 0x1000000;
    }


  LoadSwatches ();
  Invalidate (false);
	
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsP4 property page

IMPLEMENT_DYNCREATE(CPrefsP4, CPropertyPage)

CPrefsP4::CPrefsP4() : CPropertyPage(CPrefsP4::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP4)
	m_log_input = FALSE;
	m_bLogNotes = FALSE;
	m_strLogFilePreamble = _T("");
	m_strAutoLogFileName = _T("");
	m_bWriteWorldNameToLog = FALSE;
	m_strLogLinePreambleOutput = _T("");
	m_strLogLinePreambleInput = _T("");
	m_strLogLinePreambleNotes = _T("");
	m_strLogFilePostamble = _T("");
	m_strLogLinePostambleOutput = _T("");
	m_strLogLinePostambleInput = _T("");
	m_strLogLinePostambleNotes = _T("");
	m_bLogOutput = FALSE;
	m_bLogHTML = FALSE;
	m_bLogInColour = FALSE;
	m_bLogRaw = FALSE;
	//}}AFX_DATA_INIT
  m_page_number = 1;
  m_doc = NULL;
}

CPrefsP4::~CPrefsP4()
{
}

void CPrefsP4::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP4)
	DDX_Control(pDX, IDC_LOG_FILE_POSTAMBLE, m_ctlPostamble);
	DDX_Control(pDX, IDC_LOG_FILE_PREAMBLE, m_ctlPreamble);
	DDX_Control(pDX, IDC_LOG_HTML, m_ctlLogHTML);
	DDX_Control(pDX, IDC_LOG_FILE, m_ctlLogFileName);
	DDX_Check(pDX, IDC_LOG_INPUT, m_log_input);
	DDX_Check(pDX, IDC_LOG_NOTES, m_bLogNotes);
	DDX_Text(pDX, IDC_LOG_FILE_PREAMBLE, m_strLogFilePreamble);
	DDX_Text(pDX, IDC_LOG_FILE, m_strAutoLogFileName);
	DDX_Check(pDX, IDC_WRITE_WORLD_NAME, m_bWriteWorldNameToLog);
	DDX_Text(pDX, IDC_LINE_PREAMBLE_OUTPUT, m_strLogLinePreambleOutput);
	DDX_Text(pDX, IDC_LINE_PREAMBLE_INPUT, m_strLogLinePreambleInput);
	DDX_Text(pDX, IDC_LINE_PREAMBLE_NOTES, m_strLogLinePreambleNotes);
	DDX_Text(pDX, IDC_LOG_FILE_POSTAMBLE, m_strLogFilePostamble);
	DDX_Text(pDX, IDC_LINE_POSTAMBLE_OUTPUT, m_strLogLinePostambleOutput);
	DDX_Text(pDX, IDC_LINE_POSTAMBLE_INPUT, m_strLogLinePostambleInput);
	DDX_Text(pDX, IDC_LINE_POSTAMBLE_NOTES, m_strLogLinePostambleNotes);
	DDX_Check(pDX, IDC_LOG_OUTPUT, m_bLogOutput);
	DDX_Check(pDX, IDC_LOG_HTML, m_bLogHTML);
	DDX_Check(pDX, IDC_LOG_COLOUR, m_bLogInColour);
	DDX_Check(pDX, IDC_RAW_LOG, m_bLogRaw);
	//}}AFX_DATA_MAP

  if (pDX->m_bSaveAndValidate)
    {
    // check if output is not logged, sounds strange.
    if (!m_bLogOutput && !m_bLogRaw)
      if (::TMessageBox ("You are not logging output from the MUD - is this intentional?", 
              MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) 
          != IDYES)
         pDX->Fail();  

    } // end of save and validate

}



BEGIN_MESSAGE_MAP(CPrefsP4, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP4)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_STANDARD_PREAMBLE, OnStandardPreamble)
	ON_BN_CLICKED(IDC_EDIT_PREAMBLE, OnEditPreamble)
	ON_BN_CLICKED(IDC_EDIT_POSTAMBLE, OnEditPostamble)
	ON_BN_CLICKED(IDC_SUBSITUTION_HELP, OnSubsitutionHelp)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_LOG_COLOUR, OnUpdateNeedHTML)
  ON_UPDATE_COMMAND_UI(IDC_STANDARD_PREAMBLE, OnUpdateNeedHTML)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPrefsP4 message handlers

void CPrefsP4::OnBrowse() 
{
	CFileDialog	dlg(FALSE,						// FALSE for FileSave
					"txt",						// default extension
					"",
					OFN_HIDEREADONLY|OFN_PATHMUSTEXIST,
					"Text files (*.txt)|*.txt||",
					this);
	dlg.m_ofn.lpstrTitle = "Log file name";

  // use default log file directory
  dlg.m_ofn.lpstrInitialDir = Make_Absolute_Path (App.m_strDefaultLogFileDirectory);

  ChangeToFileBrowsingDirectory ();
  int nResult = dlg.DoModal();
  ChangeToStartupDirectory ();

	if (nResult != IDOK)
		return;
	
  m_ctlLogFileName.SetWindowText (dlg.GetPathName());

}

LRESULT CPrefsP4::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CPrefsP4::OnKickIdle

void CPrefsP4::OnUpdateNeedHTML(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlLogHTML.GetCheck () != FALSE);
  } // end of CPrefsP4::OnUpdateNeedHTML


void CPrefsP4::OnStandardPreamble() 
{
  m_ctlPreamble.SetWindowText (
    "<html>" NL 
    " <head>" NL  
    " <title>Log of %N session</title>" NL  
    " </head>" NL  
    " <body>" NL  
    "   <table border=0 cellpadding=5 bgcolor=\"#000000\">" NL   
    "   <tr><td>" NL   
    "   <pre><code><font size=2 face=\"FixedSys, Lucida Console, Courier New, Courier\">" NL  
    );

  m_ctlPostamble.SetWindowText (
    "</font></code></pre>" NL 
    "</td></tr></table>" NL 
    "</body>" NL 
    "</html>" NL 
          );
  
}

void CPrefsP4::OnEditPreamble() 
{
CEditMultiLine dlg;

  dlg.m_strText = GetText (m_ctlPreamble);

  dlg.m_strTitle = "Edit log file preamble";

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlPreamble.SetWindowText (dlg.m_strText);
}

void CPrefsP4::OnEditPostamble() 
{
CEditMultiLine dlg;

  dlg.m_strText = GetText (m_ctlPostamble);

  dlg.m_strTitle = "Edit log file postamble";

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlPostamble.SetWindowText (dlg.m_strText);
}

void CPrefsP4::OnSubsitutionHelp() 
{
CCreditsDlg dlg;

  dlg.m_iResourceID = IDR_SUBSTITUTION;
  dlg.m_strTitle = "Special characters";

  dlg.DoModal ();	
	
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsP5 property page

IMPLEMENT_DYNCREATE(CPrefsP5, CPropertyPage)

CPrefsP5::CPrefsP5() : CPropertyPage(CPrefsP5::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP5)
	m_bUseDefaultColours = FALSE;
	m_bCustom16isDefaultColour = FALSE;
	//}}AFX_DATA_INIT

  m_page_number = 1;
  m_doc = NULL;
  m_prefsp3 = NULL;

}

CPrefsP5::~CPrefsP5()
{
}


void CPrefsP5::DoDataExchange(CDataExchange* pDX)
{

  // check they really want to use defaults
  if (pDX->m_bSaveAndValidate)
    if (m_ctlUseDefaultColours.GetCheck ())
      if (m_ctlUseDefaultColours.GetCheck () != m_bUseDefaultColours)
         if (::TMessageBox ("By checking the option \"Override with default colours\" "
             " your existing colours will be PERMANENTLY discarded next time you open this world.\n\n"
             "Are you SURE you want to do this?",
             MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 ) != IDYES)
             pDX->Fail ();


	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP5)
	DDX_Control(pDX, IDC_USE_DEFAULT_COLOURS, m_ctlUseDefaultColours);
	DDX_Control(pDX, IDC_YELLOW_SWATCH_BOLD, m_ctlYellowSwatchBold);
	DDX_Control(pDX, IDC_YELLOW_SWATCH, m_ctlYellowSwatch);
	DDX_Control(pDX, IDC_WHITE_SWATCH_BOLD, m_ctlWhiteSwatchBold);
	DDX_Control(pDX, IDC_WHITE_SWATCH, m_ctlWhiteSwatch);
	DDX_Control(pDX, IDC_RED_SWATCH_BOLD, m_ctlRedSwatchBold);
	DDX_Control(pDX, IDC_RED_SWATCH, m_ctlRedSwatch);
	DDX_Control(pDX, IDC_MAGENTA_SWATCH_BOLD, m_ctlMagentaSwatchBold);
	DDX_Control(pDX, IDC_MAGENTA_SWATCH, m_ctlMagentaSwatch);
	DDX_Control(pDX, IDC_GREEN_SWATCH_BOLD, m_ctlGreenSwatchBold);
	DDX_Control(pDX, IDC_GREEN_SWATCH, m_ctlGreenSwatch);
	DDX_Control(pDX, IDC_CYAN_SWATCH_BOLD, m_ctlCyanSwatchBold);
	DDX_Control(pDX, IDC_CYAN_SWATCH, m_ctlCyanSwatch);
	DDX_Control(pDX, IDC_BLUE_SWATCH_BOLD, m_ctlBlueSwatchBold);
	DDX_Control(pDX, IDC_BLUE_SWATCH, m_ctlBlueSwatch);
	DDX_Control(pDX, IDC_BLACK_SWATCH_BOLD, m_ctlBlackSwatchBold);
	DDX_Control(pDX, IDC_BLACK_SWATCH, m_ctlBlackSwatch);
	DDX_Check(pDX, IDC_USE_DEFAULT_COLOURS, m_bUseDefaultColours);
	DDX_Check(pDX, IDC_USE_CUSTOM_COLOUR_16, m_bCustom16isDefaultColour);
	//}}AFX_DATA_MAP

}


BEGIN_MESSAGE_MAP(CPrefsP5, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP5)
	ON_BN_CLICKED(IDC_SAVE_COLOURS, OnSaveColours)
	ON_BN_CLICKED(IDC_LOAD_COLOURS, OnLoadColours)
	ON_BN_CLICKED(IDC_ANSI_COLOURS, OnAnsiColours)
	ON_BN_CLICKED(IDC_BLACK_SWATCH, OnBlackSwatch)
	ON_BN_CLICKED(IDC_RED_SWATCH, OnRedSwatch)
	ON_BN_CLICKED(IDC_GREEN_SWATCH, OnGreenSwatch)
	ON_BN_CLICKED(IDC_YELLOW_SWATCH, OnYellowSwatch)
	ON_BN_CLICKED(IDC_BLUE_SWATCH, OnBlueSwatch)
	ON_BN_CLICKED(IDC_MAGENTA_SWATCH, OnMagentaSwatch)
	ON_BN_CLICKED(IDC_CYAN_SWATCH, OnCyanSwatch)
	ON_BN_CLICKED(IDC_WHITE_SWATCH, OnWhiteSwatch)
	ON_BN_CLICKED(IDC_BLACK_SWATCH_BOLD, OnBlackSwatchBold)
	ON_BN_CLICKED(IDC_RED_SWATCH_BOLD, OnRedSwatchBold)
	ON_BN_CLICKED(IDC_GREEN_SWATCH_BOLD, OnGreenSwatchBold)
	ON_BN_CLICKED(IDC_YELLOW_SWATCH_BOLD, OnYellowSwatchBold)
	ON_BN_CLICKED(IDC_BLUE_SWATCH_BOLD, OnBlueSwatchBold)
	ON_BN_CLICKED(IDC_MAGENTA_SWATCH_BOLD, OnMagentaSwatchBold)
	ON_BN_CLICKED(IDC_CYAN_SWATCH_BOLD, OnCyanSwatchBold)
	ON_BN_CLICKED(IDC_WHITE_SWATCH_BOLD, OnWhiteSwatchBold)
	ON_BN_CLICKED(IDC_SWAP, OnSwap)
	ON_BN_CLICKED(IDC_NORMAL_LIGHTER, OnNormalLighter)
	ON_BN_CLICKED(IDC_NORMAL_DARKER, OnNormalDarker)
	ON_BN_CLICKED(IDC_BOLD_LIGHTER, OnBoldLighter)
	ON_BN_CLICKED(IDC_BOLD_DARKER, OnBoldDarker)
	ON_BN_CLICKED(IDC_ALL_LIGHTER, OnAllLighter)
	ON_BN_CLICKED(IDC_ALL_DARKER, OnAllDarker)
	ON_BN_CLICKED(IDC_ALL_MORE_COLOUR, OnAllMoreColour)
	ON_BN_CLICKED(IDC_ALL_LESS_COLOUR, OnAllLessColour)
	ON_BN_CLICKED(IDC_COPY_TO_CUSTOM, OnCopyToCustom)
	ON_BN_CLICKED(IDC_INVERT, OnInvert)
	ON_BN_CLICKED(IDC_RANDOM, OnRandom)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_SWAP, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_LOAD_COLOURS, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_ANSI_COLOURS, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_BLACK_SWATCH, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_RED_SWATCH, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_GREEN_SWATCH, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_YELLOW_SWATCH, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_BLUE_SWATCH, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_MAGENTA_SWATCH, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_CYAN_SWATCH, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_WHITE_SWATCH, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_BLACK_SWATCH_BOLD, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_RED_SWATCH_BOLD, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_GREEN_SWATCH_BOLD, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_YELLOW_SWATCH_BOLD, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_BLUE_SWATCH_BOLD, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_MAGENTA_SWATCH_BOLD, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_CYAN_SWATCH_BOLD, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_WHITE_SWATCH_BOLD, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_USE_DEFAULT_COLOURS, OnUpdateHaveDefaults)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPrefsP5 message handlers

BOOL CPrefsP5::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
  LoadSwatches ();

  bool bEnable = !m_bUseDefaultColours ||
                  App.m_strDefaultColoursFile.IsEmpty ();

  m_ctlYellowSwatchBold.EnableWindow (bEnable);   
  m_ctlYellowSwatch.EnableWindow (bEnable);       
  m_ctlWhiteSwatchBold.EnableWindow (bEnable);    
  m_ctlWhiteSwatch.EnableWindow (bEnable);        
  m_ctlRedSwatchBold.EnableWindow (bEnable);      
  m_ctlRedSwatch.EnableWindow (bEnable);          
  m_ctlMagentaSwatchBold.EnableWindow (bEnable);  
  m_ctlMagentaSwatch.EnableWindow (bEnable);      
  m_ctlGreenSwatchBold.EnableWindow (bEnable);    
  m_ctlGreenSwatch.EnableWindow (bEnable);        
  m_ctlCyanSwatchBold.EnableWindow (bEnable);     
  m_ctlCyanSwatch.EnableWindow (bEnable);         
  m_ctlBlueSwatchBold.EnableWindow (bEnable);     
  m_ctlBlueSwatch.EnableWindow (bEnable);         
  m_ctlBlackSwatchBold.EnableWindow (bEnable);    
  m_ctlBlackSwatch.EnableWindow (bEnable);        
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPrefsP5::OnOK() 
{
  SaveSwatches ();
	
	CPropertyPage::OnOK();
}

BOOL CPrefsP5::OnKillActive() 
{
  SaveSwatches ();
	
	return CPropertyPage::OnKillActive();
}

void CPrefsP5::amendcolour (CColourButton & whichcolour)
  {

int i;

  CColourPickerDlg dlg;
  dlg.m_iColour = whichcolour.m_colour;
  dlg.m_bPickColour = true;

/*
CColorDialog dlg (whichcolour.m_colour, 
                  CC_RGBINIT | CC_FULLOPEN, 
                  GetOwner());

  */

  for (i = BLACK; i <= WHITE; i++)
    {
    dlg.m_iCustColour [i] = m_normalcolour [i];
    dlg.m_iCustColour [i + 8] = m_boldcolour [i];
    }

  
  if (dlg.DoModal () != IDOK)
    return;

  whichcolour.m_colour = dlg.GetColor ();
  whichcolour.RedrawWindow();

  } // end of amendcolour


void CPrefsP5::OnAnsiColours() 
{
  if (::TMessageBox ("Reset all colours to the ANSI defaults?", 
          MB_YESNO | MB_ICONQUESTION) != IDYES)
    return;

  SetDefaultAnsiColours (m_normalcolour, m_boldcolour);

  LoadSwatches ();
  Invalidate (false);
}


void CPrefsP5::OnSaveColours() 
{

// To save the colours we must copy the swatches back to our array

  SaveSwatches ();

// and must copy the array to the document, as that is what "Save_Set" uses

  // ansi colours
  memcpy (m_doc->m_normalcolour, m_normalcolour, sizeof (m_doc->m_normalcolour));
  memcpy (m_doc->m_boldcolour, m_boldcolour, sizeof (m_doc->m_boldcolour));

  // custom colours
  memcpy (m_doc->m_customtext, m_prefsp3->m_customtext, sizeof (m_doc->m_customtext));
  memcpy (m_doc->m_customback, m_prefsp3->m_customback, sizeof (m_doc->m_customback));

// As we have now changed the document, we may as well redraw all views
// immediately

  m_doc->UpdateAllViews (NULL);

// They can no longer cancel the propery sheet, the document has changed

  CancelToClose ();

// save the colours

  if (m_doc->Save_Set (COLOUR, this))
    return;

}

void CPrefsP5::OnLoadColours() 
{

  if (m_doc->Load_Set (COLOUR, "", this))
    return;

  memcpy (m_normalcolour, m_doc->m_normalcolour, sizeof (m_normalcolour));
  memcpy (m_boldcolour,   m_doc->m_boldcolour, sizeof (m_boldcolour));

  memcpy (m_prefsp3->m_customtext, m_doc->m_customtext, sizeof (m_prefsp3->m_customtext));
  memcpy (m_prefsp3->m_customback, m_doc->m_customback, sizeof (m_prefsp3->m_customback));

  LoadSwatches ();
  Invalidate ();    // redraw swatches

  UpdateData (FALSE);

// As we have now changed the document, we may as well redraw all views
// immediately

  m_doc->UpdateAllViews (NULL);

// They can no longer cancel the propery sheet, the document has changed

  CancelToClose ();

}

void CPrefsP5::OnBlackSwatch() 
{
  amendcolour (m_ctlBlackSwatch);
}

void CPrefsP5::OnRedSwatch() 
{
  amendcolour (m_ctlRedSwatch);	
}

void CPrefsP5::OnGreenSwatch() 
{
  amendcolour (m_ctlGreenSwatch);
}

void CPrefsP5::OnYellowSwatch() 
{
  amendcolour (m_ctlYellowSwatch);
}

void CPrefsP5::OnBlueSwatch() 
{
  amendcolour (m_ctlBlueSwatch);
}

void CPrefsP5::OnMagentaSwatch() 
{
  amendcolour (m_ctlMagentaSwatch);
}

void CPrefsP5::OnCyanSwatch() 
{
  amendcolour (m_ctlCyanSwatch);
}

void CPrefsP5::OnWhiteSwatch() 
{
  amendcolour (m_ctlWhiteSwatch);
}

void CPrefsP5::OnBlackSwatchBold() 
{
  amendcolour (m_ctlBlackSwatchBold);
}

void CPrefsP5::OnRedSwatchBold() 
{
  amendcolour (m_ctlRedSwatchBold);	
}

void CPrefsP5::OnGreenSwatchBold() 
{
  amendcolour (m_ctlGreenSwatchBold);
}

void CPrefsP5::OnYellowSwatchBold() 
{
  amendcolour (m_ctlYellowSwatchBold);
}

void CPrefsP5::OnBlueSwatchBold() 
{
  amendcolour (m_ctlBlueSwatchBold);
}

void CPrefsP5::OnMagentaSwatchBold() 
{
  amendcolour (m_ctlMagentaSwatchBold);
}

void CPrefsP5::OnCyanSwatchBold() 
{
  amendcolour (m_ctlCyanSwatchBold);
}

void CPrefsP5::OnWhiteSwatchBold() 
{
  amendcolour (m_ctlWhiteSwatchBold);
}


void CPrefsP5::OnSwap() 
{
COLORREF oldColours [8];

// To swap the colours we must copy the swatches back to our array

  SaveSwatches ();
 
  // save normal ones
  oldColours [BLACK]  = m_normalcolour [BLACK]   ;
  oldColours [BLUE]   = m_normalcolour [BLUE]    ;
  oldColours [CYAN]   = m_normalcolour [CYAN]    ;
  oldColours [GREEN]  = m_normalcolour [GREEN]   ;
  oldColours [MAGENTA]= m_normalcolour [MAGENTA] ;
  oldColours [RED]    = m_normalcolour [RED]     ;
  oldColours [WHITE]  = m_normalcolour [WHITE]   ;
  oldColours [YELLOW] = m_normalcolour [YELLOW]  ;

  // make normal bold
  m_normalcolour [BLACK]   = m_boldcolour  [BLACK];    
  m_normalcolour [BLUE]    = m_boldcolour  [BLUE] ;    
  m_normalcolour [CYAN]    = m_boldcolour  [CYAN] ;    
  m_normalcolour [GREEN]   = m_boldcolour  [GREEN];    
  m_normalcolour [MAGENTA] = m_boldcolour  [MAGENTA];  
  m_normalcolour [RED]     = m_boldcolour  [RED]    ;  
  m_normalcolour [WHITE]   = m_boldcolour  [WHITE]  ;  
  m_normalcolour [YELLOW]  = m_boldcolour  [YELLOW] ;  
                   
  // and make bold normal      
  m_boldcolour  [BLACK]    = oldColours [BLACK];             
  m_boldcolour  [BLUE]     = oldColours [BLUE] ;       
  m_boldcolour  [CYAN]     = oldColours [CYAN] ;       
  m_boldcolour  [GREEN]    = oldColours [GREEN];       
  m_boldcolour  [MAGENTA]  = oldColours [MAGENTA];     
  m_boldcolour  [RED]      = oldColours [RED]    ;     
  m_boldcolour  [WHITE]    = oldColours [WHITE]  ;     
  m_boldcolour  [YELLOW]   = oldColours [YELLOW] ;     

  LoadSwatches ();
  Invalidate (false);
	
}

void CPrefsP5::SaveSwatches(void)
  {

  // ansi colours
    m_normalcolour [BLACK]     =  m_ctlBlackSwatch.m_colour     ; 	
    m_normalcolour [BLUE]      =  m_ctlBlueSwatch.m_colour      ; 	
    m_normalcolour [CYAN]      =  m_ctlCyanSwatch.m_colour      ; 	
    m_normalcolour [GREEN]     =  m_ctlGreenSwatch.m_colour     ; 	
    m_normalcolour [MAGENTA]   =  m_ctlMagentaSwatch.m_colour   ; 	
    m_normalcolour [RED]       =  m_ctlRedSwatch.m_colour       ; 	
    m_normalcolour [WHITE]     =  m_ctlWhiteSwatch.m_colour     ; 	
    m_normalcolour [YELLOW]    =  m_ctlYellowSwatch.m_colour    ; 	
                              
    m_boldcolour  [BLACK]      =  m_ctlBlackSwatchBold.m_colour  ; 	
    m_boldcolour  [BLUE]       =  m_ctlBlueSwatchBold.m_colour   ; 	
    m_boldcolour  [CYAN]       =  m_ctlCyanSwatchBold.m_colour   ; 	
    m_boldcolour  [GREEN]      =  m_ctlGreenSwatchBold.m_colour  ; 	
    m_boldcolour  [MAGENTA]    =  m_ctlMagentaSwatchBold.m_colour; 	
    m_boldcolour  [RED]        =  m_ctlRedSwatchBold.m_colour    ; 	
    m_boldcolour  [WHITE]      =  m_ctlWhiteSwatchBold.m_colour  ; 	
    m_boldcolour  [YELLOW]     =  m_ctlYellowSwatchBold.m_colour ; 	
                     
    m_bUseDefaultColours = m_ctlUseDefaultColours.GetCheck ();

  }


void CPrefsP5::LoadSwatches(void)
  {

  // ansi colours

  m_ctlBlackSwatch.m_colour       = 	m_normalcolour [BLACK];      
  m_ctlBlueSwatch.m_colour        = 	m_normalcolour [BLUE];       
  m_ctlCyanSwatch.m_colour        = 	m_normalcolour [CYAN];       
  m_ctlGreenSwatch.m_colour       = 	m_normalcolour [GREEN];      
  m_ctlMagentaSwatch.m_colour     = 	m_normalcolour [MAGENTA];    
  m_ctlRedSwatch.m_colour         = 	m_normalcolour [RED];        
  m_ctlWhiteSwatch.m_colour       = 	m_normalcolour [WHITE];      
  m_ctlYellowSwatch.m_colour      = 	m_normalcolour [YELLOW];     
                                  
  m_ctlBlackSwatchBold.m_colour   = 	m_boldcolour [BLACK];      
  m_ctlBlueSwatchBold.m_colour    = 	m_boldcolour [BLUE];       
  m_ctlCyanSwatchBold.m_colour    = 	m_boldcolour [CYAN];       
  m_ctlGreenSwatchBold.m_colour   = 	m_boldcolour [GREEN];      
  m_ctlMagentaSwatchBold.m_colour = 	m_boldcolour [MAGENTA];    
  m_ctlRedSwatchBold.m_colour     = 	m_boldcolour [RED];        
  m_ctlWhiteSwatchBold.m_colour   = 	m_boldcolour [WHITE];      
  m_ctlYellowSwatchBold.m_colour  = 	m_boldcolour [YELLOW];     
  
  }


void CPrefsP5::OnNormalLighter() 
{

  SaveSwatches ();

  for (int i = BLACK; i <= WHITE; i++)
    m_normalcolour [i] = ::AdjustColour (m_normalcolour [i], ADJUST_COLOUR_LIGHTER);

  LoadSwatches ();
  Invalidate (false);
	
}

void CPrefsP5::OnNormalDarker() 
{

  SaveSwatches ();

  for (int i = BLACK; i <= WHITE; i++)
    m_normalcolour [i] = ::AdjustColour (m_normalcolour [i], ADJUST_COLOUR_DARKER);

  LoadSwatches ();
  Invalidate (false);
	
}

void CPrefsP5::OnBoldLighter() 
{

  SaveSwatches ();

  for (int i = BLACK; i <= WHITE; i++)
    m_boldcolour [i] = ::AdjustColour (m_boldcolour [i], ADJUST_COLOUR_LIGHTER);


  LoadSwatches ();
  Invalidate (false);
	
}

void CPrefsP5::OnBoldDarker() 
{

  SaveSwatches ();

  for (int i = BLACK; i <= WHITE; i++)
    m_boldcolour [i] = ::AdjustColour (m_boldcolour [i], ADJUST_COLOUR_DARKER);

  LoadSwatches ();
  Invalidate (false);
}

void CPrefsP5::OnAllLighter() 
{
  OnNormalLighter ();
  OnBoldLighter ();	
}

void CPrefsP5::OnAllDarker() 
{
  OnNormalDarker ();
  OnBoldDarker ();	
}

void CPrefsP5::OnAllMoreColour() 
{

  SaveSwatches ();

  for (int i = BLACK; i <= WHITE; i++)
    {
    m_normalcolour [i] = ::AdjustColour (m_normalcolour [i], ADJUST_COLOUR_MORE_COLOUR);
    m_boldcolour [i] = ::AdjustColour (m_boldcolour [i], ADJUST_COLOUR_MORE_COLOUR);
    }


  LoadSwatches ();
  Invalidate (false);

}

void CPrefsP5::OnAllLessColour() 
{

 SaveSwatches ();

  for (int i = BLACK; i <= WHITE; i++)
    {
    m_normalcolour [i] = ::AdjustColour (m_normalcolour [i], ADJUST_COLOUR_LESS_COLOUR);
    m_boldcolour [i] = ::AdjustColour (m_boldcolour [i], ADJUST_COLOUR_LESS_COLOUR);
    }


  LoadSwatches ();
  Invalidate (false);
	
}

 LRESULT CPrefsP5::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CPrefsP5::OnKickIdle

void CPrefsP5::OnUpdateNotUsingDefaults(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlUseDefaultColours.GetCheck () == 0 ||
                  App.m_strDefaultColoursFile.IsEmpty ());
  } // end of CPrefsP5::OnUpdateNotUsingDefaults

void CPrefsP5::OnUpdateHaveDefaults(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (!App.m_strDefaultColoursFile.IsEmpty ());
  } // end of CPrefsP5::OnUpdateHaveDefaults

void CPrefsP5::OnCopyToCustom() 
{
if (::TMessageBox ("Copy all 16 colours to the custom colours?", 
        MB_YESNO | MB_ICONQUESTION) != IDYES)
return;

SaveSwatches ();

m_prefsp3->m_customtext [0]  = m_normalcolour [0];
m_prefsp3->m_customtext [1]  = m_normalcolour [1];
m_prefsp3->m_customtext [2]  = m_normalcolour [2];
m_prefsp3->m_customtext [3]  = m_normalcolour [3];
m_prefsp3->m_customtext [4]  = m_normalcolour [4];
m_prefsp3->m_customtext [5]  = m_normalcolour [5];
m_prefsp3->m_customtext [6]  = m_normalcolour [6];
m_prefsp3->m_customtext [7]  = m_normalcolour [7];
	                
m_prefsp3->m_customtext [8]  = m_boldcolour [0];
m_prefsp3->m_customtext [9]  = m_boldcolour [1];
m_prefsp3->m_customtext [10] = m_boldcolour [2];
m_prefsp3->m_customtext [11] = m_boldcolour [3];
m_prefsp3->m_customtext [12] = m_boldcolour [4];
m_prefsp3->m_customtext [13] = m_boldcolour [5];
m_prefsp3->m_customtext [14] = m_boldcolour [6];
m_prefsp3->m_customtext [15] = m_boldcolour [7];
	
for (int i = 0; i < 16; i++)
  m_prefsp3->m_customback [i] = RGB (0, 0, 0);   // black backgrounds

m_prefsp3->LoadSwatches ();
  
}


void CPrefsP5::OnInvert() 
{

  SaveSwatches ();

  for (int i = BLACK; i <= WHITE; i++)
    {
    m_normalcolour [i] = ::AdjustColour (m_normalcolour [i], ADJUST_COLOUR_INVERT);
    m_boldcolour [i] = ::AdjustColour (m_boldcolour [i], ADJUST_COLOUR_INVERT);
    }


  LoadSwatches ();
  Invalidate (false);
	
}


void CPrefsP5::OnRandom() 
{
  if (::TMessageBox ("Make all colours random?", 
          MB_YESNO | MB_ICONQUESTION) != IDYES)
  return;

  for (int i = BLACK; i <= WHITE; i++)
    {
    m_normalcolour [i]  = genrand () * (double) 0x1000000;
    m_boldcolour [i]    = genrand () * (double) 0x1000000;
    }


  LoadSwatches ();
  Invalidate (false);

	
}

CString strMacroDescriptions [MACRO_COUNT] = {
     "up",
     "down",
     "north",
     "south",
     "east",
     "west",
     "examine",
     "look",
     "page",
     "say",
     "whisper",
     "doing",
     "who",
     "drop",
     "take",
     "F2",
     "F3",
     "F4",
     "F5",
     "F7",
     "F8",
     "F9",
     "F10",
     "F11",
     "F12",
     "F2+Shift",
     "F3+Shift",
     "F4+Shift",
     "F5+Shift",
     "F6+Shift",
     "F7+Shift",
     "F8+Shift",
     "F9+Shift",
     "F10+Shift",
     "F11+Shift",
     "F12+Shift",
     "F2+Ctrl",
     "F3+Ctrl",
     "F5+Ctrl",
     "F7+Ctrl",
     "F8+Ctrl",
     "F9+Ctrl",
     "F10+Ctrl",
     "F11+Ctrl",
     "F12+Ctrl",
     "logout",
     "quit",

     // new ones in v 3.42
     "Alt+A",
     "Alt+B",
     "Alt+J",
     "Alt+K",
     "Alt+L",
     "Alt+M",
     "Alt+N",
     "Alt+O",
     "Alt+P",
     "Alt+Q",
     "Alt+R",
     "Alt+S",
     "Alt+T",
     "Alt+U",
     "Alt+X",
     "Alt+Y",
     "Alt+Z",

     // new ones in v 3.44

     "F1",
     "F1+Ctrl",
     "F1+Shift",
     "F6",
     "F6+Ctrl"

     };


CString strKeypadNames [eKeypad_Max_Items] =
  {
  // normal
  "0", 
  "1", 
  "2", 
  "3", 
  "4", 
  "5", 
  "6", 
  "7", 
  "8", 
  "9", 
  ".", 
  "/", 
  "*", 
  "-", 
  "+",
  // ctrl
  "Ctrl+0", 
  "Ctrl+1", 
  "Ctrl+2", 
  "Ctrl+3", 
  "Ctrl+4", 
  "Ctrl+5", 
  "Ctrl+6", 
  "Ctrl+7", 
  "Ctrl+8", 
  "Ctrl+9", 
  "Ctrl+.", 
  "Ctrl+/", 
  "Ctrl+*", 
  "Ctrl+-", 
  "Ctrl++"
  };

/////////////////////////////////////////////////////////////////////////////
// CPrefsP6 property page

IMPLEMENT_DYNCREATE(CPrefsP6, CPropertyPage)

CPrefsP6::CPrefsP6() : CPropertyPage(CPrefsP6::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP6)
	m_bUseDefaultMacros = FALSE;
	//}}AFX_DATA_INIT

  m_page_number = 1;
  m_pMacrosFindInfo = NULL;
  m_doc = NULL;

// default to sorting in name order

  m_last_col = eColumnName;
  m_reverse = FALSE;

}

CPrefsP6::~CPrefsP6()
{
}

void CPrefsP6::DoDataExchange(CDataExchange* pDX)
{

  // check they really want to use defaults
  if (pDX->m_bSaveAndValidate)
    if (m_ctlUseDefaultMacros.GetCheck ())
      if (m_ctlUseDefaultMacros.GetCheck () != m_bUseDefaultMacros)
         if (::TMessageBox ("By checking the option \"Override with default macros\" "
             " your existing macros will be PERMANENTLY discarded next time you open this world.\n\n"
             "Are you SURE you want to do this?",
             MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 ) != IDYES)
             pDX->Fail ();

	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP6)
	DDX_Control(pDX, IDC_USE_DEFAULT_MACROS, m_ctlUseDefaultMacros);
	DDX_Control(pDX, IDC_MACROS_LIST, m_ctlMacroList);
	DDX_Check(pDX, IDC_USE_DEFAULT_MACROS, m_bUseDefaultMacros);
	//}}AFX_DATA_MAP

}


BEGIN_MESSAGE_MAP(CPrefsP6, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP6)
	ON_BN_CLICKED(IDC_EDIT_MACRO, OnEditMacro)
	ON_NOTIFY(NM_DBLCLK, IDC_MACROS_LIST, OnDblclkMacrosList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_MACROS_LIST, OnColumnclickMacrosList)
	ON_BN_CLICKED(IDC_LOAD_MACRO, OnLoadMacro)
	ON_BN_CLICKED(IDC_SAVE_MACRO, OnSaveMacro)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_FIND, OnFind)
	ON_BN_CLICKED(IDC_FIND_NEXT, OnFindNext)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_EDIT_MACRO, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_SAVE_MACRO, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_LOAD_MACRO, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_USE_DEFAULT_MACROS, OnUpdateHaveDefaults)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsP6 message handlers

void CPrefsP6::OnEditMacro() 
{

// don't do it if they are using defaults
if (m_ctlUseDefaultMacros.GetCheck () != 0 &&
                  !App.m_strDefaultMacrosFile.IsEmpty ())
  return;

for (int nItem = -1;
      (nItem = m_ctlMacroList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
        {
  int subscript = m_ctlMacroList.GetItemData (nItem);

  CEditMacro dlg;

  dlg.m_macro_desc = strMacroDescriptions [subscript];
  dlg.m_macro = m_macros [subscript];
  dlg.m_macro_type = m_macro_type [subscript];
  dlg.m_strTitle = "Edit macro " + strMacroDescriptions [subscript]; 

  if (dlg.DoModal () != IDOK)
    continue;

  m_macros [subscript] = dlg.m_macro;
  m_macro_type [subscript] = dlg.m_macro_type;

  switch (m_macro_type [subscript])
    {
    case REPLACE_COMMAND: m_macro_type_alpha [subscript] = "Replace";
                          break;
    case SEND_NOW:        m_macro_type_alpha [subscript]  = "Send now";
                          break;
    case ADD_TO_COMMAND:  m_macro_type_alpha [subscript]  = "Insert";
                          break;
    default:              m_macro_type_alpha [subscript]  = "Unknown";
                          break;
    } // end of switch

// re-setup list with amended details

	m_ctlMacroList.SetItemText(nItem, eColumnText, 
              Replace (m_macros [subscript], ENDLINE, "\\n", true));
	m_ctlMacroList.SetItemText(nItem, eColumnAction, m_macro_type_alpha [subscript]);

  m_ctlMacroList.RedrawItems (nItem, nItem);

  }
}

void CPrefsP6::OnDblclkMacrosList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnEditMacro ();
	*pResult = 0;
}

typedef struct
  {
  CPrefsP6 * dlg;
  int        sortkey;
  int        reverse;
  }   t_p6_sort_param;

int CALLBACK CPrefsP6::CompareFunc ( LPARAM lParam1, 
                                     LPARAM lParam2,
                                     LPARAM lParamSort)
  { 

t_p6_sort_param * psort_param = (t_p6_sort_param *) lParamSort;

int iResult;

  switch (psort_param->sortkey)   // which sort key
    {
    case eColumnName: iResult = strMacroDescriptions [lParam1].CompareNoCase 
                          (strMacroDescriptions [lParam2]); break;
    case eColumnText: iResult = psort_param->dlg->m_macros [lParam1].CompareNoCase 
                          (psort_param->dlg->m_macros [lParam2]); break;
    case eColumnAction: iResult = psort_param->dlg->m_macro_type_alpha [lParam1].CompareNoCase 
                          (psort_param->dlg->m_macro_type_alpha [lParam2]); break;

    default: return 0;
    } // end of switch

// if reverse sort wanted, reverse sense of result

  if (psort_param->reverse)
    iResult *= -1;

  return iResult;

  } // end of CompareFunc


void CPrefsP6::OnColumnclickMacrosList(NMHDR* pNMHDR, LRESULT* pResult) 
{

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

int col = pNMListView->iSubItem;

  if (col == m_last_col)
    m_reverse = !m_reverse;
  else
    m_reverse = FALSE;

  m_last_col = col;

 t_p6_sort_param sort_param;

  sort_param.dlg = this;
  sort_param.sortkey = m_last_col;
  sort_param.reverse = m_reverse;

  m_ctlMacroList.SortItems (CompareFunc, (LPARAM) &sort_param); 
	
	*pResult = 0;
}

void CPrefsP6::OnLoadMacro() 
{
int i;

  if (m_doc->Load_Set (MACRO,  "", this))
    return;

  for (i = 0; i < NUMITEMS (m_doc->m_macros); i++)
    {
    m_macros [i] = m_doc->m_macros [i];
    m_macro_type [i] = m_doc->m_macro_type [i];
    switch (m_doc->m_macro_type [i])
      {
      case REPLACE_COMMAND: m_macro_type_alpha [i] = "Replace";
                            break;
      case SEND_NOW:        m_macro_type_alpha [i]  = "Send now";
                            break;
      case ADD_TO_COMMAND:  m_macro_type_alpha [i]  = "Insert";
                            break;
      default:              m_macro_type_alpha [i]  = "Unknown";
                            break;
      } // end of switch
    }

  load_list ();

  CancelToClose ();   // this change is irrevocable

}


void CPrefsP6::OnSaveMacro() 
{

  if (m_doc->Save_Set (MACRO, this))
    return;

}

void CPrefsP6::load_list (void)
  {

   m_ctlMacroList.DeleteAllItems ();

 
  BYTE fVirt;
  WORD key;
   
  // Item data

  int nInsertItem = 0;

  for (int nItem = 0; nItem < NUMITEMS (strMacroDescriptions); nItem++)
    {

    // extra code for version 3.53 - don't show in list if it is an accelerator
    try
      {
      const char * sKey = strMacroDescriptions [nItem];

      // all lower-case words (like north, quit etc.) won't be keycodes
      bool bLower = true;
      for (const char * p = sKey; *p; p++)
        if (!islower (*p))
           bLower = false;

      if (bLower)
        {
        fVirt = 0;    // won't be in list
        key = 0;
        }
      else
        StringToKeyCode (sKey, fVirt, key);
      }
    catch (CException* e)
      {
      e->Delete ();
      fVirt = 0;    // won't be in list
      key = 0;
      } // end of catch

       
    fVirt |= FNOINVERT;   // no inverting of menus

    // make a flags/keystroke combination                                                      
    long virt_plus_key = ((long) fVirt) << 16 | key;

    // see if we have allocated that accelerator
    map<long, WORD>::const_iterator it = m_doc->m_AcceleratorToCommandMap.find (virt_plus_key);

    if (it != m_doc->m_AcceleratorToCommandMap.end ())
      continue;

  	m_ctlMacroList.InsertItem (nInsertItem, strMacroDescriptions [nItem]);  // eColumnName
	  m_ctlMacroList.SetItemText(nInsertItem, eColumnText, 
            Replace (m_macros [nItem], ENDLINE, "\\n", true));
	  m_ctlMacroList.SetItemText(nInsertItem, eColumnAction, m_macro_type_alpha [nItem]);

    m_ctlMacroList.SetItemData(nInsertItem, nItem);
    
    nInsertItem++;

    } // end of inserting items

 t_p6_sort_param sort_param;

  sort_param.dlg = this;
  sort_param.sortkey = m_last_col;
  sort_param.reverse = m_reverse;

  m_ctlMacroList.SortItems (CompareFunc, (LPARAM) &sort_param); 

// set the 1st item to be selected - we do this here because sorting the
// list means our first item is not necessarily the 1st item in the list

 m_ctlMacroList.SetItemState (0, LVIS_FOCUSED | LVIS_SELECTED, 
                                 LVIS_FOCUSED | LVIS_SELECTED);

  } // end of load_list

BOOL CPrefsP6::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

int iColOrder [eColumnCount] = {0, 1, 2},
    iColWidth [eColumnCount] = {100, 180, 0};

  RECT rect;

  m_ctlMacroList.GetClientRect (&rect);

  iColWidth [eColumnAction] = rect.right - rect.left -            // get width of control
                              GetSystemMetrics (SM_CXVSCROLL) -   // minus vertical scroll bar
                              iColWidth [eColumnName] -
                              iColWidth [eColumnText] ;           // minus width of other items

  App.RestoreColumnConfiguration ("Macro List", eColumnCount, m_ctlMacroList,
                                  iColOrder, iColWidth, m_last_col, m_reverse);
  
	m_ctlMacroList.InsertColumn(0, TranslateHeading ("Macro name"), LVCFMT_LEFT, iColWidth [eColumnName]);
  m_ctlMacroList.InsertColumn(1, TranslateHeading ("Text"), LVCFMT_LEFT, iColWidth [eColumnText]);
  m_ctlMacroList.InsertColumn(2, TranslateHeading ("Action"), LVCFMT_LEFT, iColWidth [eColumnAction]);

// recover column sequence

  m_ctlMacroList.SendMessage (LVM_SETCOLUMNORDERARRAY, eColumnCount, (DWORD) iColOrder);

  load_list ();
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPrefsP6::OnDestroy() 
{

	App.SaveColumnConfiguration ("Macro List", eColumnCount, m_ctlMacroList, m_last_col, m_reverse);

  CPropertyPage::OnDestroy();
	
}


LRESULT CPrefsP6::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CPrefsP6::OnKickIdle

void CPrefsP6::OnUpdateNeedSelection(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlMacroList.GetSelectedCount () != 0 &&
                  (m_ctlUseDefaultMacros.GetCheck () == 0 ||
                  App.m_strDefaultMacrosFile.IsEmpty ()));
  } // end of CPrefsP6::OnUpdateNeedSelection

void CPrefsP6::OnUpdateNeedEntries(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlMacroList.GetItemCount () != 0);
  } // end of CPrefsP6::OnUpdateNeedEntries

void CPrefsP6::OnUpdateNotUsingDefaults(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlUseDefaultMacros.GetCheck () == 0 ||
                  App.m_strDefaultMacrosFile.IsEmpty ());
  } // end of CPrefsP6::OnUpdateNotUsingDefaults

void CPrefsP6::OnUpdateHaveDefaults(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (!App.m_strDefaultMacrosFile.IsEmpty ());
  } // end of CPrefsP6::OnUpdateHaveDefaults

void CPrefsP6::OnFind() 
{
  DoFind (false);
}

void CPrefsP6::OnFindNext() 
{
  DoFind (true);
}

void CPrefsP6::DoFind (bool bAgain)
  {

m_pMacrosFindInfo->m_bAgain = bAgain;
m_pMacrosFindInfo->m_nTotalLines = m_ctlMacroList.GetItemCount ();

bool found = FindRoutine (&m_ctlMacroList,       // passed back to callback routines
                          *m_pMacrosFindInfo,   // finding structure
                          InitiateSearch,        // how to re-initiate a find
                          GetNextLine);          // get the next line

// unselect everything first

for (int i = 0; i < m_ctlMacroList.GetItemCount (); i++)
  m_ctlMacroList.SetItemState (i, 0, LVIS_FOCUSED | LVIS_SELECTED);

if (found)
  {
   m_ctlMacroList.SetItemState (m_pMacrosFindInfo->m_nCurrentLine, 
                                   LVIS_FOCUSED | LVIS_SELECTED, 
                                   LVIS_FOCUSED | LVIS_SELECTED);
   m_ctlMacroList.EnsureVisible (m_pMacrosFindInfo->m_nCurrentLine, false);
   m_ctlMacroList.RedrawItems (m_pMacrosFindInfo->m_nCurrentLine, 
                               m_pMacrosFindInfo->m_nCurrentLine);

  }

  } // end of CPrefsP6::DoFind



void CPrefsP6::InitiateSearch (const CObject * pObject,
                                  CFindInfo & FindInfo)
  {
CListCtrl* pList = (CListCtrl*) pObject;

  if (FindInfo.m_bAgain)
    FindInfo.m_pFindPosition = (POSITION) FindInfo.m_nCurrentLine;
  else
    if (FindInfo.m_bForwards)
      FindInfo.m_pFindPosition = 0;
    else
      FindInfo.m_pFindPosition = (POSITION) pList->GetItemCount () - 1;

  } // end of CPrefsP6::InitiateSearch

bool CPrefsP6::GetNextLine (const CObject * pObject,
                             CFindInfo & FindInfo, 
                             CString & strLine)
  {
CListCtrl* pList = (CListCtrl*) pObject;

  if ((long) FindInfo.m_pFindPosition < 0 ||
      (long) FindInfo.m_pFindPosition >= pList->GetItemCount ())
    return true;

 
  strLine = pList->GetItemText((long) FindInfo.m_pFindPosition, eColumnName);
  strLine += '\t';
  strLine += pList->GetItemText((long) FindInfo.m_pFindPosition, eColumnText);
  strLine += '\t';
  strLine += pList->GetItemText((long) FindInfo.m_pFindPosition, eColumnAction);

  if (FindInfo.m_bForwards)
    FindInfo.m_pFindPosition++;
  else
    FindInfo.m_pFindPosition--;

  return false;
  } // end of CPrefsP6::GetNextLine


/////////////////////////////////////////////////////////////////////////////
// CPrefsP7 property page

IMPLEMENT_DYNCREATE(CPrefsP7, CGenPropertyPage)

CPrefsP7::CPrefsP7() : CGenPropertyPage(CPrefsP7::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP7)
	m_enable_aliases = FALSE;
	m_bUseDefaultAliases = FALSE;
	//}}AFX_DATA_INIT
  m_doc = NULL;
  m_last_col = eColumnSequence;

}

CPrefsP7::~CPrefsP7()
{
}

void CPrefsP7::DoDataExchange(CDataExchange* pDX)
{

  // check they really want to use defaults
  if (pDX->m_bSaveAndValidate)
    if (m_ctlUseDefaultAliases.GetCheck ())
      if (m_ctlUseDefaultAliases.GetCheck () != m_bUseDefaultAliases)
        if (m_doc->m_AliasMap.GetCount () > 0)
         if (::UMessageBox (TFormat ("By checking the option \"Override with default aliases\" "
             " your existing %i aliase(s) will be PERMANENTLY discarded next time you open this world.\n\n"
             "Are you SURE you want to do this?", m_doc->m_AliasMap.GetCount ()),
             MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 ) != IDYES)
             pDX->Fail ();

	CGenPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP7)
	DDX_Control(pDX, IDC_COUNT, m_ctlSummary);
	DDX_Control(pDX, IDC_FILTER, m_ctlFilter);
	DDX_Control(pDX, IDC_USE_DEFAULT_ALIASES, m_ctlUseDefaultAliases);
	DDX_Control(pDX, IDC_ALIASES_LIST, m_ctlAliasList);
	DDX_Check(pDX, IDC_ENABLE_ALIASES, m_enable_aliases);
	DDX_Check(pDX, IDC_USE_DEFAULT_ALIASES, m_bUseDefaultAliases);
	//}}AFX_DATA_MAP


}


BEGIN_MESSAGE_MAP(CPrefsP7, CGenPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP7)
	ON_BN_CLICKED(IDC_ADD_ALIAS, OnAddAlias)
	ON_BN_CLICKED(IDC_CHANGE_ALIAS, OnChangeAlias)
	ON_BN_CLICKED(IDC_DELETE_ALIAS, OnDeleteAlias)
	ON_NOTIFY(NM_DBLCLK, IDC_ALIASES_LIST, OnDblclkAliasesList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_ALIASES_LIST, OnColumnclickAliasesList)
	ON_BN_CLICKED(IDC_LOAD_ALIAS, OnLoadAlias)
	ON_BN_CLICKED(IDC_SAVE_ALIAS, OnSaveAlias)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	ON_BN_CLICKED(IDC_PASTE, OnPaste)
	ON_BN_CLICKED(IDC_MOVE_UP, OnMoveUp)
	ON_BN_CLICKED(IDC_MOVE_DOWN, OnMoveDown)
	ON_BN_CLICKED(IDC_FILTER, OnFilter)
	ON_BN_CLICKED(IDC_EDIT_FILTER, OnEditFilter)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_FIND, OnFind)
	ON_BN_CLICKED(IDC_FIND_NEXT, OnFindNext)
  ON_UPDATE_COMMAND_UI(IDC_COPY, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_PASTE, OnUpdateNeedXMLClipboard)
  ON_UPDATE_COMMAND_UI(IDC_CHANGE_ALIAS, OnUpdateNeedOneSelection)
  ON_UPDATE_COMMAND_UI(IDC_DELETE_ALIAS, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_SAVE_ALIAS, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_FIND, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_FIND_NEXT, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_ADD_ALIAS, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_LOAD_ALIAS, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_USE_DEFAULT_ALIASES, OnUpdateHaveDefaults)
  ON_UPDATE_COMMAND_UI(IDC_MOVE_UP, OnUpdateCanSequence)
  ON_UPDATE_COMMAND_UI(IDC_MOVE_DOWN, OnUpdateCanSequence)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsP7 message handlers


void CPrefsP7::InitDialog (CDialog * pDlg)
  {
  CAliasDlg * dlg = (CAliasDlg *) pDlg;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CAliasDlg ) ) );

  dlg->m_pDoc = m_doc;

  dlg->m_bIgnoreCase = m_doc->m_bDefaultAliasIgnoreCase;
  dlg->m_bEnabled = TRUE;
  dlg->m_bExpandVariables = m_doc->m_bDefaultAliasExpandVariables; 
  dlg->m_bOmitFromLogFile = FALSE; 
  dlg->m_bRegexp = m_doc->m_bDefaultAliasRegexp;
  dlg->m_iSequence = m_doc->m_iDefaultAliasSequence; 
  dlg->m_bKeepEvaluating = m_doc->m_bDefaultAliasKeepEvaluating;
  dlg->m_bOneShot = FALSE;
  dlg->m_bEchoAlias = FALSE;
  dlg->m_bOmitFromCommandHistory = FALSE;
  dlg->m_bTemporary = FALSE; 
//  dlg->m_bDelayed = FALSE; 
  dlg->m_iSendTo = m_doc->m_iDefaultAliasSendTo;

  dlg->m_pAliasMap = &m_doc->m_AliasMap;
  dlg->m_current_alias = NULL;

  }  // end of CPrefsP7::InitDialog

void CPrefsP7::LoadDialog (CDialog * pDlg, CObject * pItem)
  {
  CAliasDlg * dlg = (CAliasDlg *) pDlg;
  CAlias * alias_item = (CAlias *) pItem;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CAliasDlg ) ) );
  ASSERT_VALID (alias_item);
  ASSERT( alias_item->IsKindOf( RUNTIME_CLASS( CAlias ) ) );

  dlg->m_pDoc = m_doc;

  dlg->m_name = alias_item->name;
  dlg->m_contents = alias_item->contents;
  dlg->m_bIgnoreCase = alias_item->bIgnoreCase;
//  dlg->m_bDelayed = alias_item->bDelayed;
  dlg->m_strLabel = alias_item->strLabel;
  dlg->m_strProcedure = alias_item->strProcedure;
  dlg->m_bEnabled = alias_item->bEnabled;
  dlg->m_bExpandVariables = alias_item->bExpandVariables;
  dlg->m_bOmitFromLogFile = alias_item->bOmitFromLog;
  dlg->m_bRegexp = alias_item->bRegexp;
  dlg->m_bOmitFromOutput = alias_item->bOmitFromOutput;
  dlg->m_bMenu = alias_item->bMenu;
  dlg->m_bTemporary = alias_item->bTemporary;
  dlg->m_strGroup = alias_item->strGroup;
  dlg->m_strVariable = alias_item->strVariable;
  dlg->m_iSendTo = alias_item->iSendTo;
  dlg->m_iSequence       = alias_item->iSequence; 
  dlg->m_bKeepEvaluating       = alias_item->bKeepEvaluating; 
  dlg->m_bOneShot              = alias_item->bOneShot;
  dlg->m_bEchoAlias       = alias_item->bEchoAlias; 
  dlg->m_bOmitFromCommandHistory       = alias_item->bOmitFromCommandHistory; 

  dlg->m_pAliasMap = &m_doc->m_AliasMap;
  dlg->m_current_alias = alias_item;

  if (alias_item->regexp && 
      alias_item->regexp->m_program == NULL && 
      alias_item->regexp->m_iExecutionError < PCRE_ERROR_NOMATCH)
    dlg->m_strRegexpError = 
      TFormat ("Error: %s ", Convert_PCRE_Runtime_Error (alias_item->regexp->m_iExecutionError));

  // NB - also see MapDlg.cpp for alias processing

  }   // end of CPrefsP7::LoadDialog


void CPrefsP7::UnloadDialog (CDialog * pDlg, CObject * pItem)
  {
  CAliasDlg * dlg = (CAliasDlg *) pDlg;
  CAlias * alias_item = (CAlias *) pItem;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CAliasDlg ) ) );
  ASSERT_VALID (alias_item);
  ASSERT( alias_item->IsKindOf( RUNTIME_CLASS( CAlias ) ) );

  alias_item->name            = dlg->m_name;
  alias_item->contents        = dlg->m_contents;
  alias_item->bIgnoreCase     = dlg->m_bIgnoreCase;
//  alias_item->bDelayed        = dlg->m_bDelayed;
  alias_item->strLabel        = dlg->m_strLabel;
  alias_item->strProcedure    = dlg->m_strProcedure;
  alias_item->bEnabled        = dlg->m_bEnabled;
  alias_item->bExpandVariables = dlg->m_bExpandVariables;
  alias_item->bOmitFromLog    = dlg->m_bOmitFromLogFile;
  alias_item->bRegexp         = dlg->m_bRegexp;
  alias_item->bOmitFromOutput = dlg->m_bOmitFromOutput;
  alias_item->iSendTo         = dlg->m_iSendTo;
  alias_item->bMenu           = dlg->m_bMenu;
  alias_item->bTemporary      = dlg->m_bTemporary;
  alias_item->strGroup        = dlg->m_strGroup;
  alias_item->strVariable     = dlg->m_strVariable;
  alias_item->iSequence       = dlg->m_iSequence;
  alias_item->bKeepEvaluating = dlg->m_bKeepEvaluating;
  alias_item->bOneShot        = dlg->m_bOneShot;
  alias_item->bEchoAlias      = dlg->m_bEchoAlias;
  alias_item->bOmitFromCommandHistory      = dlg->m_bOmitFromCommandHistory;

  delete alias_item->regexp;    // get rid of earlier regular expression
  alias_item->regexp = NULL;

// all aliass are now regular expressions

  CString strRegexp; 

  if (alias_item->bRegexp)
    strRegexp = alias_item->name;
  else
    strRegexp = ConvertToRegularExpression (alias_item->name);

  alias_item->regexp = regcomp (strRegexp,
                                  (alias_item->bIgnoreCase  ? PCRE_CASELESS : 0)
#if ALIASES_USE_UTF8
                                  | (m_pDoc->m_bUTF_8 ? PCRE_UTF8 : 0)
#endif // ALIASES_USE_UTF8
                                  );

// NB - also see MapDlg.cpp for alias processing

  }   // end of CPrefsP7::UnloadDialog

CString CPrefsP7::GetObjectName (CDialog * pDlg) const
  {
  CAliasDlg * dlg = (CAliasDlg *) pDlg;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CAliasDlg ) ) );
        
  return dlg->m_strLabel;
  }    // end of CPrefsP7::GetObjectName

CString CPrefsP7::GetScriptName (CObject * pItem) const
  {
  CAlias * alias_item = (CAlias *) pItem;

  ASSERT_VALID (alias_item);
  ASSERT( alias_item->IsKindOf( RUNTIME_CLASS( CAlias ) ) );

  return alias_item->strProcedure;

  } // end of CPrefsP7::GetScriptName 
 
CString CPrefsP7::GetLabel (CObject * pItem) const
  {
  CAlias * alias_item = (CAlias *) pItem;

  ASSERT_VALID (alias_item);
  ASSERT( alias_item->IsKindOf( RUNTIME_CLASS( CAlias ) ) );

  return alias_item->strLabel;

  } // end of CPrefsP7::GetLabel 

void CPrefsP7::SetDispatchID (CObject * pItem, const DISPID dispid)
  {
  CAlias * alias_item = (CAlias *) pItem;

  ASSERT_VALID (alias_item);
  ASSERT( alias_item->IsKindOf( RUNTIME_CLASS( CAlias ) ) );

  alias_item->dispid = dispid;

  } // end of CPrefsP7::SetDispatchID

void CPrefsP7::SetInternalName (CObject * pItem, const CString strName)
  {
  CAlias * alias_item = (CAlias *) pItem;

  ASSERT_VALID (alias_item);
  ASSERT( alias_item->IsKindOf( RUNTIME_CLASS( CAlias ) ) );

  alias_item->strInternalName = strName;

  } // end of CPrefsP7::SetInternalName

CObject * CPrefsP7::MakeNewObject (void)
  {
  return (CObject *) new CAlias;
  }    // end of CPrefsP7::MakeNewObject

__int64 CPrefsP7::GetModificationNumber (CObject * pItem) const
  {
  CAlias * alias_item = (CAlias *) pItem;

  ASSERT_VALID (alias_item);
  ASSERT( alias_item->IsKindOf( RUNTIME_CLASS( CAlias ) ) );

  return alias_item->nUpdateNumber;

  }   // end of CPrefsP7::GetModificationNumber

void CPrefsP7::SetModificationNumber (CObject * pItem, __int64 nUpdateNumber)
  {
  CAlias * alias_item = (CAlias *) pItem;

  ASSERT_VALID (alias_item);
  ASSERT( alias_item->IsKindOf( RUNTIME_CLASS( CAlias ) ) );
  
  alias_item->nUpdateNumber = nUpdateNumber;
  }    // end of CPrefsP7::SetModificationNumber


bool CPrefsP7::CheckIfChanged (CDialog * pDlg, CObject * pItem) const
  {
  CAliasDlg * dlg = (CAliasDlg *) pDlg;
  CAlias * alias_item = (CAlias *) pItem;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CAliasDlg ) ) );
  ASSERT_VALID (alias_item);
  ASSERT( alias_item->IsKindOf( RUNTIME_CLASS( CAlias ) ) );

  if (alias_item->name          == dlg->m_name &&
      alias_item->contents      == dlg->m_contents &&
      alias_item->bIgnoreCase   == dlg->m_bIgnoreCase &&
//      alias_item->bDelayed      == dlg->m_bDelayed &&
      alias_item->bOmitFromLog  == dlg->m_bOmitFromLogFile &&
      alias_item->strLabel      == dlg->m_strLabel && 
      alias_item->strProcedure  == dlg->m_strProcedure && 
      alias_item->bEnabled      == dlg->m_bEnabled &&
      alias_item->bExpandVariables  == dlg->m_bExpandVariables &&
      alias_item->bRegexp       == dlg->m_bRegexp &&
      alias_item->bOmitFromOutput  == dlg->m_bOmitFromOutput &&
      alias_item->iSendTo       ==   dlg->m_iSendTo &&
      alias_item->bMenu         ==   dlg->m_bMenu  &&
      alias_item->bTemporary    == (bool) dlg->m_bTemporary &&
      alias_item->strGroup      == dlg->m_strGroup  &&
      alias_item->strVariable   == dlg->m_strVariable &&
      alias_item->iSequence     == dlg->m_iSequence &&
      alias_item->bKeepEvaluating  == dlg->m_bKeepEvaluating &&
      alias_item->bOneShot        == dlg->m_bOneShot &&
      alias_item->bEchoAlias       == dlg->m_bEchoAlias  &&
      alias_item->bOmitFromCommandHistory       == dlg->m_bOmitFromCommandHistory

     ) return false;    // no need to set modified flag if nothing changed

  return true;
  }    // end of CPrefsP7::CheckIfChanged


void CPrefsP7::OnAddAlias() 
{
  CAliasDlg dlg;
  dlg.m_pDoc = m_doc;

  OnAddItem (dlg);

  m_doc->SortAliases ();

}   // end of CPrefsP7::OnAddAlias

void CPrefsP7::OnChangeAlias() 
{

// don't do it if they are using defaults
if (m_ctlUseDefaultAliases.GetCheck () != 0 &&
                  !App.m_strDefaultAliasesFile.IsEmpty ())
  return;

  CAliasDlg dlg;
  dlg.m_pDoc = m_doc;

  OnChangeItem (dlg);

  m_doc->SortAliases ();
}    // end of CPrefsP7::OnChangeAlias

void CPrefsP7::OnDeleteAlias() 
{
  OnDeleteItem ();

  m_doc->SortAliases ();
}    // end of CPrefsP7::OnDeleteAlias

void CPrefsP7::OnDblclkAliasesList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  
  OnChangeAlias ();	
	*pResult = 0;
}   // end of CPrefsP7::OnDblclkAliasesList

int CPrefsP7::CompareObjects (const int iColumn, const CObject * item1, const CObject * item2)
  {
CAlias * alias1 = (CAlias *) item1,
         * alias2 = (CAlias *) item2;

  ASSERT_VALID (alias1);
  ASSERT( alias1->IsKindOf( RUNTIME_CLASS( CAlias ) ) );
  ASSERT_VALID (alias2);
  ASSERT( alias2->IsKindOf( RUNTIME_CLASS( CAlias ) ) );

int iResult;

  switch (iColumn)   // which sort key
    {
    case eColumnSequence:
          if (alias1->iSequence < alias2->iSequence)
            iResult = -1;
          else if (alias1->iSequence > alias2->iSequence)
            iResult = 1;
          else 
            // if same sequence, sort in alias match order
            iResult = alias1->name.CompareNoCase (alias2->name);
          break;
    case eColumnGroup:    iResult = alias1->strGroup.CompareNoCase (alias2->strGroup);
                          if (iResult)
                            break;
    case eColumnLabel:    iResult = alias1->strLabel.CompareNoCase (alias2->strLabel); 
                          if (iResult)
                            break;
    case eColumnAlias:    iResult = alias1->name.CompareNoCase (alias2->name); 
                          break;
    case eColumnContents: iResult = alias1->contents.CompareNoCase (alias2->contents); break;
    default: iResult = 0;
    } // end of switch

  return iResult;
  }    // end of CPrefsP7::CompareObjects

void CPrefsP7::OnColumnclickAliasesList(NMHDR* pNMHDR, LRESULT* pResult) 
  {
   OnColumnclickItemList (pNMHDR, pResult);
  }   // end of CPrefsP7::OnColumnclickAliasesList


void CPrefsP7::OnLoadAlias() 
{

  if (m_doc->Load_Set (ALIAS,  "", this))
    return;

  LoadList ();

// They can no longer cancel the propery sheet, the document has changed

  CancelToClose ();
  m_doc->SetModifiedFlag (TRUE);

  // resolve all entry points for triggers, aliases etc.

  // if scripting active, find entry points
  if (m_doc->m_ScriptEngine)
    m_doc->FindAllEntryPoints ();

}    // end of CPrefsP7::OnLoadAlias

void CPrefsP7::OnSaveAlias() 
{

  if (m_doc->Save_Set (ALIAS, this))
    return;
}    // end of CPrefsP7::OnSaveAlias


int CPrefsP7::AddItem (CObject * pItem, 
                         const int item_number,
                         const BOOL insert)
  {
CAlias * alias_item = (CAlias * ) pItem;

ASSERT_VALID (alias_item);
ASSERT( alias_item->IsKindOf( RUNTIME_CLASS( CAlias ) ) );

int nItem;

  if (insert)
    {
    nItem = m_ctlAliasList.GetItemCount ();
  	m_ctlAliasList.InsertItem (nItem, alias_item->name);   // eColumnAlias
    }
  else
    {
    nItem = item_number;
  	m_ctlAliasList.SetItemText(nItem, eColumnAlias, alias_item->name);
    }

	m_ctlAliasList.SetItemText (nItem, eColumnSequence, CFormat ("%i", alias_item->iSequence));
	m_ctlAliasList.SetItemText (nItem, eColumnContents, 
                        Replace (alias_item->contents, ENDLINE, "\\n", true));
	m_ctlAliasList.SetItemText (nItem, eColumnLabel, alias_item->strLabel);
	m_ctlAliasList.SetItemText (nItem, eColumnGroup, alias_item->strGroup);

  return nItem;

  } // end of CPrefsP7::add_item


BOOL CPrefsP7::OnInitDialog() 
{
	CGenPropertyPage::OnInitDialog();

  // fix up last column to exactly fit the list view
  RECT rect;

  m_ctlAliasList.GetClientRect (&rect);

  m_iColWidth [eColumnContents] = rect.right - rect.left -            // get width of control
                                GetSystemMetrics (SM_CXVSCROLL) -   // minus vertical scroll bar
                                m_iColWidth [eColumnAlias];           // minus width of other items
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}   // end of CPrefsP7::OnInitDialog

    
void CPrefsP7::OnMoveUp() 
{
CString strMsg;

// iterate through list in case we implement multiple selection one day
for (int nItem = -1;
      (nItem = m_ctlList->GetNextItem(nItem, LVNI_SELECTED)) != -1;)
  {

  // get the lower-case name of this item's object
  CString * pstrObjectName = (CString *) m_ctlList->GetItemData (nItem);

  ASSERT (pstrObjectName != NULL);

  CObject * pItem;

  // check object is still there (it might have gone while we looked at the list box)
  if (!m_ObjectMap->Lookup (*pstrObjectName, pItem))
    {
    m_ctlList->DeleteItem (nItem);    // it's gone, so delete it from the list view
    m_ctlList->RedrawItems (0, m_ctlList->GetItemCount () - 1);    // redraw the list

    strMsg = TFormat ("The %s named \"%s\" is no longer in the %s list",
                  (LPCTSTR) m_strObjectType,
                  (LPCTSTR) *pstrObjectName,
                  (LPCTSTR) m_strObjectType);

    ::UMessageBox (strMsg);

    delete pstrObjectName;                 // and get rid of its name string
    continue;
    }

  ASSERT_VALID (pItem);
  ASSERT( pItem->IsKindOf( RUNTIME_CLASS( CAlias ) ) );

  CAlias * alias = (CAlias *) pItem;
  int iSeq = alias->iSequence;

  if (iSeq == 0)
    ::TMessageBox ("Cannot move up - already has a sequence of zero");
  else
    {
  
    int iLower = -1;
    int bSame = false;

    for (int iItem = 0; iItem < m_doc->m_AliasMap.GetCount (); iItem++)
      if (m_doc->m_AliasArray [iItem] != alias)
        if (m_doc->m_AliasArray [iItem]->iSequence < alias->iSequence)
          iLower = m_doc->m_AliasArray [iItem]->iSequence;
        else if (m_doc->m_AliasArray [iItem]->iSequence == alias->iSequence)
          bSame = true;

    if (bSame)
      alias->iSequence--;   // make a lower sequence (drop out of group)
    else
      if (iLower == -1)
        ::TMessageBox ("Cannot move up - already at top of list");
      else
        alias->iSequence = iLower; // move into lower group

    m_doc->SortAliases ();


    // They can no longer cancel the propery sheet, the document has changed
    CancelToClose ();
    m_doc->SetModifiedFlag (TRUE);

    // re-setup list with amended details
    add_item (pItem, pstrObjectName, nItem, FALSE);

    }

  }   // end of dealing with each selected item

  // resort the list
  t_gen_sort_param sort_param (m_ObjectMap, m_last_col, m_reverse, m_CompareObjects);
  m_ctlList->SortItems (CompareFunc, (LPARAM) &sort_param); 
  m_ctlList->RedrawItems (0, m_ctlList->GetItemCount () - 1);    // redraw the list
	
}

void CPrefsP7::OnMoveDown() 
{
CString strMsg;

// iterate through list in case we implement multiple selection one day
for (int nItem = -1;
      (nItem = m_ctlList->GetNextItem(nItem, LVNI_SELECTED)) != -1;)
  {

  // get the lower-case name of this item's object
  CString * pstrObjectName = (CString *) m_ctlList->GetItemData (nItem);

  ASSERT (pstrObjectName != NULL);

  CObject * pItem;

  // check object is still there (it might have gone while we looked at the list box)
  if (!m_ObjectMap->Lookup (*pstrObjectName, pItem))
    {
    m_ctlList->DeleteItem (nItem);    // it's gone, so delete it from the list view
    m_ctlList->RedrawItems (0, m_ctlList->GetItemCount () - 1);    // redraw the list

    strMsg = TFormat ("The %s named \"%s\" is no longer in the %s list",
                  (LPCTSTR) m_strObjectType,
                  (LPCTSTR) *pstrObjectName,
                  (LPCTSTR) m_strObjectType);

    ::UMessageBox (strMsg);

    delete pstrObjectName;                 // and get rid of its name string
    continue;
    }

  ASSERT_VALID (pItem);
  ASSERT( pItem->IsKindOf( RUNTIME_CLASS( CAlias ) ) );

  CAlias * alias = (CAlias *) pItem;
  int iSeq = alias->iSequence;

  if (iSeq == 10000)
    ::TMessageBox ("Cannot move down - already has a sequence of 10000");
  else
    {
  
    int iHigher = -1;
    int bSame = false;

    for (int iItem = 0; iItem < m_doc->m_AliasMap.GetCount (); iItem++)
      if (m_doc->m_AliasArray [iItem] != alias)
        if (m_doc->m_AliasArray [iItem]->iSequence > alias->iSequence &&
            iHigher == -1)
          iHigher = m_doc->m_AliasArray [iItem]->iSequence;
        else if (m_doc->m_AliasArray [iItem]->iSequence == alias->iSequence)
          bSame = true;

    if (bSame)
      alias->iSequence++;   // make a higher sequence (jump out of group)
    else
      if (iHigher == -1)
        ::TMessageBox ("Cannot move down - already at bottom of list");
      else
        alias->iSequence = iHigher; // move into Higher group

    m_doc->SortAliases ();


    // They can no longer cancel the propery sheet, the document has changed
    CancelToClose ();
    m_doc->SetModifiedFlag (TRUE);

    // re-setup list with amended details
    add_item (pItem, pstrObjectName, nItem, FALSE);

    }

  }   // end of dealing with each selected item

  // resort the list
  t_gen_sort_param sort_param (m_ObjectMap, m_last_col, m_reverse, m_CompareObjects);
  m_ctlList->SortItems (CompareFunc, (LPARAM) &sort_param); 
  m_ctlList->RedrawItems (0, m_ctlList->GetItemCount () - 1);    // redraw the list
	
	
}

void CPrefsP7::OnUpdateNotUsingDefaults(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlUseDefaultAliases.GetCheck () == 0 ||
                  App.m_strDefaultAliasesFile.IsEmpty ());
  } // end of CPrefsP7::OnUpdateNotUsingDefaults

void CPrefsP7::OnUpdateHaveDefaults(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (!App.m_strDefaultAliasesFile.IsEmpty ());
  } // end of CPrefsP7::OnUpdateHaveDefaults


void CPrefsP7::OnUpdateNeedSelection(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlList->GetSelectedCount () != 0   &&
                  (m_ctlUseDefaultAliases.GetCheck () == 0 ||
                  App.m_strDefaultAliasesFile.IsEmpty ()));
  } // end of CPrefsP7::OnUpdateNeedSelection

void CPrefsP7::OnUpdateNeedOneSelection(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlList->GetSelectedCount () == 1   &&
                  (m_ctlUseDefaultAliases.GetCheck () == 0 ||
                  App.m_strDefaultAliasesFile.IsEmpty ()));
  } // end of CPrefsP7::OnUpdateNeedOneSelection

void CPrefsP7::OnUpdateNeedXMLClipboard(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (IsClipboardXML ());
  } // end of CPrefsP7::OnUpdateNeedXMLClipboard

void CPrefsP7::OnCopy() 
{
OnCopyItem ();	
}

void CPrefsP7::OnPaste() 
{
OnPasteItem ();	
}

bool CPrefsP7::CheckIfIncluded (CObject * pItem)
  {
  CAlias * alias_item = (CAlias *) pItem;

  ASSERT_VALID (alias_item);
  ASSERT( alias_item->IsKindOf( RUNTIME_CLASS( CAlias ) ) );
  
  return alias_item->bIncluded;
  }    // end of CPrefsP7::CheckIfIncluded

bool CPrefsP7::CheckIfTemporary (CObject * pItem)
  {
  CAlias * alias_item = (CAlias *) pItem;

  ASSERT_VALID (alias_item);
  ASSERT( alias_item->IsKindOf( RUNTIME_CLASS( CAlias ) ) );
  
  return alias_item->bTemporary;
  }    // end of CPrefsP7::CheckIfTemporary


bool CPrefsP7::CheckIfExecuting (CObject * pItem)
  {
  CAlias * alias_item = (CAlias *) pItem;

  ASSERT_VALID (alias_item);
  ASSERT( alias_item->IsKindOf( RUNTIME_CLASS( CAlias ) ) );
  
  return alias_item->bExecutingScript;
  }    // end of CPrefsP7::CheckIfExecuting

void CPrefsP7::OnUpdateCanSequence(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_last_col == eColumnSequence && !m_reverse  &&
                  m_ctlList->GetSelectedCount () == 1  &&
                  (m_ctlUseDefaultAliases.GetCheck () == 0 ||
                  App.m_strDefaultAliasesFile.IsEmpty ()));
  } // end of CPrefsP7::OnUpdateCanSequence

bool CPrefsP7::GetFilterFlag ()            // is filtering enabled?
  {
  return m_ctlFilter.GetCheck ();
  }

CString CPrefsP7::GetFilterScript ()       // get the filter script
  {
  return m_doc->m_strAliasesFilter;
  }

void CPrefsP7::OnFilter() 
{
  LoadList ();  // need to reload the list now	
}

void CPrefsP7::OnEditFilter() 
{
if (EditFilterText (m_doc->m_strAliasesFilter))
  LoadList ();  // need to reload the list now	
}

void CPrefsP7::GetFilterInfo (CObject * pItem, lua_State * L)
  {
  CAlias * alias = (CAlias *) pItem;

  ASSERT_VALID (alias);
  ASSERT( alias->IsKindOf( RUNTIME_CLASS( CAlias ) ) );

  // the table of stuff
  lua_newtable(L);                                                            

  MakeTableItemBool (L, "echo_alias",                 alias->bEchoAlias);
  MakeTableItemBool (L, "enabled",                    alias->bEnabled);
  MakeTableItemBool (L, "expand_variables",           alias->bExpandVariables);
  MakeTableItem     (L, "group",                      alias->strGroup);
  MakeTableItemBool (L, "ignore_case",                alias->bIgnoreCase);
  MakeTableItemBool (L, "keep_evaluating",            alias->bKeepEvaluating);
  MakeTableItem     (L, "match",                      alias->name);
  MakeTableItemBool (L, "menu",                       alias->bMenu);
  MakeTableItem     (L, "name",                       alias->strLabel);
  MakeTableItemBool (L, "omit_from_command_history",  alias->bOmitFromCommandHistory);
  MakeTableItemBool (L, "omit_from_log",              alias->bOmitFromLog);
  MakeTableItemBool (L, "omit_from_output",           alias->bOmitFromOutput);
  MakeTableItemBool (L, "one_shot",                   alias->bOneShot);
  MakeTableItemBool (L, "regexp",                     alias->bRegexp);
  MakeTableItem     (L, "script",                     alias->strProcedure);
  MakeTableItem     (L, "send",                       alias->contents);
  MakeTableItem     (L, "send_to",                    alias->iSendTo);
  MakeTableItem     (L, "sequence",                   alias->iSequence);
  MakeTableItem     (L, "user",                       alias->iUserOption);
  MakeTableItem     (L, "variable",                   alias->strVariable);

  // stuff below is not part of the world file but calculated at runtime

  //  GetAliasInfo  (10)
  MakeTableItem (L, "invocation_count", alias->nInvocationCount);
  
  //  GetAliasInfo  (11)
  MakeTableItem (L, "times_matched",    alias->nMatched);

  //  GetAliasInfo  (13)
  if (alias->tWhenMatched.GetTime ())  
    MakeTableItem   (L, "when_matched", COleDateTime (alias->tWhenMatched.GetTime ())); 

  //  GetAliasInfo  (14)
  MakeTableItemBool (L, "temporary",    alias->bTemporary);

  //  GetAliasInfo  (15)
  MakeTableItemBool (L, "included",     alias->bIncluded);

  //  GetAliasInfo  (24)
  if (alias->regexp)      
    MakeTableItem   (L, "match_count",  alias->regexp->m_iCount);

  //  GetAliasInfo  (25)
  if (alias->regexp)      
    MakeTableItem   (L, "last_match",   alias->regexp->m_sTarget);

  //  GetAliasInfo  (27)
  MakeTableItemBool (L, "script_valid", alias->dispid != DISPID_UNKNOWN);

  //  GetAliasInfo  (28)
  if (alias->regexp && alias->regexp->m_program == NULL)      
    MakeTableItem   (L, "execution_error", alias->regexp->m_iExecutionError);

  //  GetAliasInfo  (30)
  if (alias->regexp && App.m_iCounterFrequency)
    MakeTableItem (L, "execution_time", ((double) alias->regexp->iTimeTaken) / 
                                        ((double) App.m_iCounterFrequency));

  //  GetAliasInfo  (31)
  if (alias->regexp)
    MakeTableItem   (L, "match_attempts", alias->regexp->m_iMatchAttempts);

  }   // end of CPrefsP7::GetFilterInfo


/////////////////////////////////////////////////////////////////////////////
// CPrefsP8 property page

IMPLEMENT_DYNCREATE(CPrefsP8, CGenPropertyPage)

CPrefsP8::CPrefsP8() : CGenPropertyPage(CPrefsP8::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP8)
	m_enable_triggers = FALSE;
	m_enable_trigger_sounds = FALSE;
	m_bUseDefaultTriggers = FALSE;
	m_bFilter = FALSE;
	//}}AFX_DATA_INIT

  m_last_col = eColumnSequence;
  m_doc = NULL;
  m_prefsp3 = NULL;
  m_prefsp14 = NULL;

}

CPrefsP8::~CPrefsP8()
{
}

void CPrefsP8::DoDataExchange(CDataExchange* pDX)
{

  // check they really want to use defaults
  if (pDX->m_bSaveAndValidate)
    if (m_ctlUseDefaultTriggers.GetCheck ())
      if (m_ctlUseDefaultTriggers.GetCheck () != m_bUseDefaultTriggers)
        if (m_doc->m_TriggerMap.GetCount () > 0)
         if (::UMessageBox (TFormat ("By checking the option \"Override with default triggers\" "
             " your existing %i trigger%s will be PERMANENTLY discarded next time you open this world.\n\n"
             "Are you SURE you want to do this?", PLURAL (m_doc->m_TriggerMap.GetCount ())),
             MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 ) != IDYES)
             pDX->Fail ();

	CGenPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP8)
	DDX_Control(pDX, IDC_COUNT, m_ctlSummary);
	DDX_Control(pDX, IDC_FILTER, m_ctlFilter);
	DDX_Control(pDX, IDC_USE_DEFAULT_TRIGGERS, m_ctlUseDefaultTriggers);
	DDX_Control(pDX, IDC_TRIGGERS_LIST, m_ctlTriggerList);
	DDX_Check(pDX, IDC_ENABLE_TRIGGERS, m_enable_triggers);
	DDX_Check(pDX, IDC_ENABLE_TRIGGERS_SOUNDS, m_enable_trigger_sounds);
	DDX_Check(pDX, IDC_USE_DEFAULT_TRIGGERS, m_bUseDefaultTriggers);
	DDX_Check(pDX, IDC_FILTER, m_bFilter);
	//}}AFX_DATA_MAP

}


BEGIN_MESSAGE_MAP(CPrefsP8, CGenPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP8)
	ON_BN_CLICKED(IDC_ADD_TRIGGER, OnAddTrigger)
	ON_BN_CLICKED(IDC_CHANGE_TRIGGER, OnChangeTrigger)
	ON_BN_CLICKED(IDC_DELETE_TRIGGER, OnDeleteTrigger)
	ON_NOTIFY(NM_DBLCLK, IDC_TRIGGERS_LIST, OnDblclkTriggersList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_TRIGGERS_LIST, OnColumnclickTriggersList)
	ON_BN_CLICKED(IDC_LOAD_TRIGGER, OnLoadTrigger)
	ON_BN_CLICKED(IDC_SAVE_TRIGGER, OnSaveTrigger)
	ON_BN_CLICKED(IDC_MOVE_UP, OnMoveUp)
	ON_BN_CLICKED(IDC_MOVE_DOWN, OnMoveDown)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	ON_BN_CLICKED(IDC_PASTE, OnPaste)
	ON_BN_CLICKED(IDC_EDIT_FILTER, OnEditFilter)
	ON_BN_CLICKED(IDC_FILTER, OnFilter)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_FIND, OnFind)
	ON_BN_CLICKED(IDC_FIND_NEXT, OnFindNext)
  ON_UPDATE_COMMAND_UI(IDC_CHANGE_TRIGGER, OnUpdateNeedOneSelection)
  ON_UPDATE_COMMAND_UI(IDC_DELETE_TRIGGER, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_COPY, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_PASTE, OnUpdateNeedXMLClipboard)
  ON_UPDATE_COMMAND_UI(IDC_SAVE_TRIGGER, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_FIND, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_FIND_NEXT, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_MOVE_UP, OnUpdateCanSequence)
  ON_UPDATE_COMMAND_UI(IDC_MOVE_DOWN, OnUpdateCanSequence)
  ON_UPDATE_COMMAND_UI(IDC_ADD_TRIGGER, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_LOAD_TRIGGER, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_USE_DEFAULT_TRIGGERS, OnUpdateHaveDefaults)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsP8 message handlers

void CPrefsP8::InitDialog (CDialog * pDlg)
  {
  CTriggerDlg * dlg = (CTriggerDlg *) pDlg;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CTriggerDlg ) ) );

  dlg->m_pDoc = m_doc;

  dlg->m_bUTF_8 = m_doc->m_bUTF_8;
  dlg->m_ignore_case = m_doc->m_bDefaultTriggerIgnoreCase;
  dlg->m_colour = 0;    // no change
  dlg->m_sound_pathname = NOSOUNDLIT;
  dlg->m_omit_from_log = FALSE;
  dlg->m_pTriggerMap = &m_doc->m_TriggerMap;
  dlg->m_current_trigger = NULL;
  dlg->m_bOmitFromOutput = FALSE;
  dlg->m_bKeepEvaluating = m_doc->m_bDefaultTriggerKeepEvaluating;
  dlg->m_bOneShot = FALSE;
  dlg->m_bEnabled = TRUE;
  dlg->m_iClipboardArg = 0;
  dlg->m_iSendTo = m_doc->m_iDefaultTriggerSendTo;
  dlg->m_bRegexp = m_doc->m_bDefaultTriggerRegexp;
  dlg->m_bRepeat = FALSE; 
  dlg->m_bExpandVariables = m_doc->m_bDefaultTriggerExpandVariables; 
  dlg->m_iSequence = m_doc->m_iDefaultTriggerSequence; 
  dlg->m_iMatchTextColour = 0;  // any
  dlg->m_iMatchBackColour = 0;  // any
  dlg->m_iMatchBold = 2;        // any
  dlg->m_iMatchItalic = 2;      // any
  dlg->m_iMatchInverse = 2;      // any
  dlg->m_bShowBold = m_prefsp14->m_bShowBold;
  dlg->m_bShowItalic = m_prefsp14->m_bShowItalic;
  dlg->m_bShowUnderline = m_prefsp14->m_bShowUnderline;
  dlg->m_bTemporary = FALSE; 
  dlg->m_iOtherForeground = 0;
  dlg->m_iOtherBackground = 0;
  dlg->m_bMultiLine = FALSE;
  dlg->m_iLinesToMatch = 0;
  dlg->m_iColourChangeType = TRIGGER_COLOUR_CHANGE_BOTH;


//  m_imglIcons.Create (IDR_TRIGGERIMAGES, 16, 1, RGB (255, 0, 255));
//  m_ctlTriggerList.SetImageList (&m_imglIcons, TVSIL_NORMAL);

  }    // end of CPrefsP8::InitDialog

void CPrefsP8::LoadDialog (CDialog * pDlg, CObject * pItem)
  {
  CTriggerDlg * dlg = (CTriggerDlg *) pDlg;
  CTrigger * trigger_item = (CTrigger *) pItem;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CTriggerDlg ) ) );
  ASSERT_VALID (trigger_item);
  ASSERT( trigger_item->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );

  dlg->m_pDoc = m_doc;

  dlg->m_bUTF_8 = m_doc->m_bUTF_8;
  dlg->m_trigger = trigger_item->trigger;
  dlg->m_contents = trigger_item->contents;
  dlg->m_ignore_case = trigger_item->ignore_case;
  dlg->m_colour = trigger_item->colour == SAMECOLOUR ? 0 : trigger_item->colour + 1;
  if (trigger_item->sound_to_play.IsEmpty ())
    dlg->m_sound_pathname = NOSOUNDLIT;
  else
    dlg->m_sound_pathname = trigger_item->sound_to_play;
  dlg->m_omit_from_log = trigger_item->omit_from_log;
  dlg->m_bOmitFromOutput = trigger_item->bOmitFromOutput;
  dlg->m_bKeepEvaluating = trigger_item->bKeepEvaluating;
  dlg->m_bOneShot        = trigger_item->bOneShot;
  dlg->m_bEnabled        = trigger_item->bEnabled;
  dlg->m_strProcedure    = trigger_item->strProcedure;
  dlg->m_strLabel        = trigger_item->strLabel;
  dlg->m_iClipboardArg   = trigger_item->iClipboardArg;
  dlg->m_iSendTo         = trigger_item->iSendTo;
  dlg->m_bRegexp         = trigger_item->bRegexp;
  dlg->m_bRepeat         = trigger_item->bRepeat; 
  dlg->m_bExpandVariables = trigger_item->bExpandVariables;
  dlg->m_iSequence       = trigger_item->iSequence; 
  dlg->m_bSoundIfInactive      = trigger_item->bSoundIfInactive; 
  dlg->m_bLowercaseWildcard    = trigger_item->bLowercaseWildcard; 

  dlg->m_bBold            = (trigger_item->iStyle & HILITE) != 0; 
  dlg->m_bItalic          = (trigger_item->iStyle & BLINK) != 0; 
  dlg->m_bUnderline       = (trigger_item->iStyle & UNDERLINE) != 0; 

  dlg->m_pTriggerMap = &m_doc->m_TriggerMap;
  dlg->m_current_trigger = trigger_item;

  dlg->m_bShowBold = m_prefsp14->m_bShowBold;
  dlg->m_bShowItalic = m_prefsp14->m_bShowItalic;
  dlg->m_bShowUnderline = m_prefsp14->m_bShowUnderline;
  dlg->m_bTemporary = trigger_item->bTemporary;
  dlg->m_strGroup = trigger_item->strGroup;
  switch (trigger_item->iSendTo)
    {
    case eSendToVariable: dlg->m_strVariable = trigger_item->strVariable; break;
#ifdef PANE
    case eSendToPane:     dlg->m_strVariable = trigger_item->strPane; break;
#endif // PANE
    default:  dlg->m_strVariable = "";  break;
    } // end of switch
   
  // set up matching colours
  if (trigger_item->iMatch & TRIGGER_MATCH_TEXT)
    dlg->m_iMatchTextColour = get_foreground (trigger_item->iMatch) + 1;
  else
    dlg->m_iMatchTextColour = 0;  // any
  if (trigger_item->iMatch & TRIGGER_MATCH_BACK)
    dlg->m_iMatchBackColour = get_background (trigger_item->iMatch) + 1;
  else
    dlg->m_iMatchBackColour = 0;  // any
  if (trigger_item->iMatch & TRIGGER_MATCH_HILITE)
    dlg->m_iMatchBold = (get_style (trigger_item->iMatch) & HILITE) != 0;
  else
    dlg->m_iMatchBold = 2;        // any
  if (trigger_item->iMatch & TRIGGER_MATCH_BLINK)
    dlg->m_iMatchItalic = (get_style (trigger_item->iMatch) & BLINK) != 0;
  else
    dlg->m_iMatchItalic = 2;        // any
  if (trigger_item->iMatch & TRIGGER_MATCH_INVERSE)
    dlg->m_iMatchInverse = (get_style (trigger_item->iMatch) & INVERSE) != 0;
  else
    dlg->m_iMatchInverse = 2;        // any

  dlg->m_iOtherForeground = trigger_item->iOtherForeground;
  dlg->m_iOtherBackground = trigger_item->iOtherBackground;

  dlg->m_bMultiLine = trigger_item->bMultiLine;
  dlg->m_iLinesToMatch = trigger_item->iLinesToMatch;
  dlg->m_iColourChangeType = trigger_item->iColourChangeType;

  if (trigger_item->regexp && 
      trigger_item->regexp->m_program == NULL && 
      trigger_item->regexp->m_iExecutionError < PCRE_ERROR_NOMATCH)
    dlg->m_strRegexpError = 
      TFormat ("Error: %s ", Convert_PCRE_Runtime_Error (trigger_item->regexp->m_iExecutionError));

  }   // end of CPrefsP8::LoadDialog

void CPrefsP8::UnloadDialog (CDialog * pDlg, CObject * pItem)
  {
  CTriggerDlg * dlg = (CTriggerDlg *) pDlg;
  CTrigger * trigger_item = (CTrigger *) pItem;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CTriggerDlg ) ) );
  ASSERT_VALID (trigger_item);
  ASSERT( trigger_item->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );

  trigger_item->trigger = dlg->m_trigger;
  trigger_item->contents = dlg->m_contents;
  trigger_item->ignore_case = dlg->m_ignore_case;
  trigger_item->colour = dlg->m_colour - 1;
  trigger_item->bOmitFromOutput = dlg->m_bOmitFromOutput;
  trigger_item->bKeepEvaluating = dlg->m_bKeepEvaluating;
  trigger_item->bOneShot        = dlg->m_bOneShot;
  trigger_item->bEnabled        = dlg->m_bEnabled;
  trigger_item->strProcedure    = dlg->m_strProcedure;
  trigger_item->strLabel        = dlg->m_strLabel;
  trigger_item->iClipboardArg   = dlg->m_iClipboardArg;
  trigger_item->iSendTo       = dlg->m_iSendTo;
  trigger_item->bRegexp          = dlg->m_bRegexp;
  trigger_item->bRepeat          = dlg->m_bRepeat;
  trigger_item->bExpandVariables = dlg->m_bExpandVariables;
  trigger_item->iSequence        = dlg->m_iSequence; 
  trigger_item->bSoundIfInactive        = dlg->m_bSoundIfInactive; 
  trigger_item->bLowercaseWildcard      = dlg->m_bLowercaseWildcard; 
  trigger_item->strGroup      = dlg->m_strGroup; 

  switch (trigger_item->iSendTo)
    {
    case eSendToVariable: trigger_item->strVariable = dlg->m_strVariable; break;
#ifdef PANE
    case eSendToPane:     trigger_item->strPane     = dlg->m_strVariable;break;
#endif // PANE
    default:  dlg->m_strVariable = "";  break;
    } // end of switch
 

  trigger_item->iOtherForeground = dlg->m_iOtherForeground;
  trigger_item->iOtherBackground = dlg->m_iOtherBackground;

  trigger_item->bMultiLine      =   dlg->m_bMultiLine;
  trigger_item->iLinesToMatch =   dlg->m_iLinesToMatch;
  trigger_item->iColourChangeType = dlg->m_iColourChangeType;

  trigger_item->iStyle = 0;
  if (dlg->m_bBold)
     trigger_item->iStyle |= HILITE;
  if (dlg->m_bItalic)
     trigger_item->iStyle |= BLINK;
  if (dlg->m_bUnderline)
     trigger_item->iStyle |= UNDERLINE;
  trigger_item->bTemporary = dlg->m_bTemporary;

  // do matching colours

  trigger_item->iMatch = 0;   // no matches yet
  if (dlg->m_iMatchTextColour > 0)   // colour selected
    {
    trigger_item->iMatch |= TRIGGER_MATCH_TEXT;
    trigger_item->iMatch |= ((dlg->m_iMatchTextColour - 1) & 7) << 4;
    }

  if (dlg->m_iMatchBackColour > 0)   // colour selected
    {
    trigger_item->iMatch |= TRIGGER_MATCH_BACK;
    trigger_item->iMatch |= ((dlg->m_iMatchBackColour - 1) & 7) << 8;
    }

  if (dlg->m_iMatchBold != 2)
    {
    trigger_item->iMatch |= TRIGGER_MATCH_HILITE;
    trigger_item->iMatch |= dlg->m_iMatchBold ? HILITE : 0;
    }

  if (dlg->m_iMatchItalic != 2)
    {
    trigger_item->iMatch |= TRIGGER_MATCH_BLINK;
    trigger_item->iMatch |= dlg->m_iMatchItalic  ? BLINK : 0;
    }

  if (dlg->m_iMatchInverse != 2)
    {
    trigger_item->iMatch |= TRIGGER_MATCH_INVERSE;
    trigger_item->iMatch |= dlg->m_iMatchInverse  ? INVERSE : 0;
    }


  if (dlg->m_sound_pathname == NOSOUNDLIT)
    trigger_item->sound_to_play.Empty ();
  else
    trigger_item->sound_to_play = dlg->m_sound_pathname;
  trigger_item->omit_from_log = dlg->m_omit_from_log;

  LONGLONG iOldTimeTaken = 0;

  // remember time taken to execute them

  if (trigger_item->regexp)
    iOldTimeTaken = trigger_item->regexp->iTimeTaken;

  delete trigger_item->regexp;    // get rid of earlier regular expression
  trigger_item->regexp = NULL;

// all triggers are now regular expressions

  CString strRegexp; 

  if (trigger_item->bRegexp)
    strRegexp = trigger_item->trigger;
  else
    strRegexp = ConvertToRegularExpression (trigger_item->trigger);

  trigger_item->regexp = regcomp (strRegexp,
                                  (trigger_item->ignore_case  ? PCRE_CASELESS : 0) |
                                  (trigger_item->bMultiLine  ? PCRE_MULTILINE : 0) |
                                  (m_doc->m_bUTF_8 ? PCRE_UTF8 : 0));

  // add back execution time
  if (trigger_item->regexp)
    trigger_item->regexp->iTimeTaken += iOldTimeTaken;
  }    // end of  CPrefsP8::UnloadDialog

CString CPrefsP8::GetObjectName (CDialog * pDlg) const
  {
  CTriggerDlg * dlg = (CTriggerDlg *) pDlg;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CTriggerDlg ) ) );
        
  return dlg->m_strLabel;
  }    // end of CPrefsP8::GetObjectName

CString CPrefsP8::GetScriptName (CObject * pItem) const
  {
  CTrigger * trigger_item = (CTrigger *) pItem;

  ASSERT_VALID (trigger_item);
  ASSERT( trigger_item->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );

  return trigger_item->strProcedure;

  } // end of CPrefsP8::GetScriptName 
 
CString CPrefsP8::GetLabel (CObject * pItem) const
  {
  CTrigger * trigger_item = (CTrigger *) pItem;

  ASSERT_VALID (trigger_item);
  ASSERT( trigger_item->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );

  return trigger_item->strLabel;

  } // end of CPrefsP8::GetLabel 

void CPrefsP8::SetDispatchID (CObject * pItem, const DISPID dispid)
  {
  CTrigger * trigger_item = (CTrigger *) pItem;

  ASSERT_VALID (trigger_item);
  ASSERT( trigger_item->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );

  trigger_item->dispid = dispid;

  } // end of CPrefsP8::SetDispatchID

void CPrefsP8::SetInternalName (CObject * pItem, const CString strName)
  {
  CTrigger * trigger_item = (CTrigger *) pItem;

  ASSERT_VALID (trigger_item);
  ASSERT( trigger_item->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );

  trigger_item->strInternalName = strName;

  } // end of CPrefsP8::SetInternalName

CObject * CPrefsP8::MakeNewObject (void)
  {
  return (CObject *) new CTrigger;
  }     // end of CPrefsP8::MakeNewObject

__int64 CPrefsP8::GetModificationNumber (CObject * pItem) const
  {
  CTrigger * trigger_item = (CTrigger *) pItem;

  ASSERT_VALID (trigger_item);
  ASSERT( trigger_item->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );

  return trigger_item->nUpdateNumber;

  }   // end of CPrefsP8::GetModificationNumber 

void CPrefsP8::SetModificationNumber (CObject * pItem, __int64 nUpdateNumber)
  {
  CTrigger * trigger_item = (CTrigger *) pItem;

  ASSERT_VALID (trigger_item);
  ASSERT( trigger_item->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );
  
  trigger_item->nUpdateNumber = nUpdateNumber;
  }   // end of CPrefsP8::SetModificationNumber

bool CPrefsP8::CheckIfChanged (CDialog * pDlg, CObject * pItem) const
  {
  CTriggerDlg * dlg = (CTriggerDlg *) pDlg;
  CTrigger * trigger_item = (CTrigger *) pItem;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CTriggerDlg ) ) );
  ASSERT_VALID (trigger_item);
  ASSERT( trigger_item->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );

  // do matching colours

  unsigned short iMatch = 0;   // no matches yet
  if (dlg->m_iMatchTextColour > 0)   // colour selected
    {
    iMatch |= TRIGGER_MATCH_TEXT;
    iMatch |= ((dlg->m_iMatchTextColour - 1) & 7) << 4;
    }

  if (dlg->m_iMatchBackColour > 0)   // colour selected
    {
    iMatch |= TRIGGER_MATCH_BACK;
    iMatch |= ((dlg->m_iMatchBackColour - 1) & 7) << 8;
    }

  if (dlg->m_iMatchBold != 2)
    {
    iMatch |= TRIGGER_MATCH_HILITE;
    iMatch |= dlg->m_iMatchBold ? HILITE : 0;
    }

  if (dlg->m_iMatchItalic != 2)
    {
    iMatch |= TRIGGER_MATCH_BLINK;
    iMatch |= dlg->m_iMatchItalic  ? BLINK : 0;
    }

  if (dlg->m_iMatchInverse != 2)
    {
    iMatch |= TRIGGER_MATCH_INVERSE;
    iMatch |= dlg->m_iMatchInverse  ? INVERSE : 0;
    }

  bool bChanged = false;

  switch (trigger_item->iSendTo)
    {
    case eSendToVariable: 
      if (trigger_item->strVariable != dlg->m_strVariable)
        bChanged = true;
#ifdef PANE
    case eSendToPane:    
      if (trigger_item->strPane != dlg->m_strVariable)
        bChanged = true;
#endif // PANE
    default:    break;
    } // end of switch

  if (!bChanged &&
      trigger_item->trigger == dlg->m_trigger &&
      trigger_item->contents == dlg->m_contents &&
      trigger_item->ignore_case == dlg->m_ignore_case &&
      trigger_item->omit_from_log == dlg->m_omit_from_log &&
      trigger_item->colour == (dlg->m_colour - 1) &&
      trigger_item->bOmitFromOutput == dlg->m_bOmitFromOutput &&
      trigger_item->bKeepEvaluating == dlg->m_bKeepEvaluating &&
      trigger_item->bOneShot == dlg->m_bOneShot &&
      trigger_item->bEnabled == dlg->m_bEnabled &&
      trigger_item->strProcedure == dlg->m_strProcedure &&
      trigger_item->strLabel == dlg->m_strLabel &&
      trigger_item->iClipboardArg == dlg->m_iClipboardArg &&
      trigger_item->iSendTo == dlg->m_iSendTo &&
      trigger_item->bRegexp == dlg->m_bRegexp &&
      trigger_item->bRepeat == dlg->m_bRepeat &&
      trigger_item->bExpandVariables  == dlg->m_bExpandVariables &&
      trigger_item->iSequence == dlg->m_iSequence &&
      trigger_item->bSoundIfInactive == dlg->m_bSoundIfInactive &&
      trigger_item->bLowercaseWildcard == (bool) dlg->m_bLowercaseWildcard &&
      trigger_item->iMatch == iMatch &&   // colour matching
      ((trigger_item->iStyle & HILITE) != 0) ==  dlg->m_bBold  &&
      ((trigger_item->iStyle & BLINK) != 0)  ==    dlg->m_bItalic  &&
      ((trigger_item->iStyle & UNDERLINE) != 0)  == dlg->m_bUnderline  &&
      ((trigger_item->sound_to_play == dlg->m_sound_pathname) ||
       (trigger_item->sound_to_play.IsEmpty () &&
        (dlg->m_sound_pathname == NOSOUNDLIT))) &&
      trigger_item->bTemporary  == (bool) dlg->m_bTemporary &&
      trigger_item->strGroup  == dlg->m_strGroup &&
      trigger_item->strVariable  == dlg->m_strVariable &&
      trigger_item->iOtherForeground == dlg->m_iOtherForeground &&
      trigger_item->iOtherBackground == dlg->m_iOtherBackground &&
      trigger_item->iLinesToMatch == dlg->m_iLinesToMatch &&
      trigger_item->iColourChangeType == dlg->m_iColourChangeType
     ) return false;    // no need to set modified flag if nothing changed

  return true;
  }   // end of CPrefsP8::CheckIfChanged

void CPrefsP8::OnAddTrigger() 
{
  CTriggerDlg dlg;

  for (int i = 0; i < MAX_CUSTOM; i++)
    {
    dlg.m_customtext [i] = m_prefsp3->m_customtext [i];
    dlg.m_customback [i] = m_prefsp3->m_customback [i];
    dlg.m_strCustomName [i] = m_prefsp3->m_strCustomName [i];
    }

  dlg.m_bUTF_8 = m_doc->m_bUTF_8;
  OnAddItem (dlg);

  m_doc->SortTriggers ();

}   // end of CPrefsP8::OnAddTrigger

void CPrefsP8::OnChangeTrigger() 
{         

// don't do it if they are using defaults
if (m_ctlUseDefaultTriggers.GetCheck () != 0 &&
                  !App.m_strDefaultTriggersFile.IsEmpty ())
  return;

  CTriggerDlg dlg;

  for (int i = 0; i < MAX_CUSTOM; i++)
    {
    dlg.m_customtext [i] = m_prefsp3->m_customtext [i];
    dlg.m_customback [i] = m_prefsp3->m_customback [i];
    dlg.m_strCustomName [i] = m_prefsp3->m_strCustomName [i];
    }

  dlg.m_bUTF_8 = m_doc->m_bUTF_8;
  OnChangeItem (dlg);

  m_doc->SortTriggers ();

}    // end of CPrefsP8::OnChangeTrigger

void CPrefsP8::OnDeleteTrigger() 
{
  OnDeleteItem ();
  m_doc->SortTriggers ();

}    // end of CPrefsP8::OnDeleteTrigger

void CPrefsP8::OnDblclkTriggersList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  
  OnChangeTrigger ();	
	*pResult = 0;
}   // end of CPrefsP8::OnDblclkTriggersList

int CPrefsP8::CompareObjects (const int iColumn, const CObject * item1, const CObject * item2)
  {
CTrigger * trigger1 = (CTrigger *) item1,
         * trigger2 = (CTrigger *) item2;

  ASSERT_VALID (trigger1);
  ASSERT( trigger1->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );
  ASSERT_VALID (trigger2);
  ASSERT( trigger2->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );

int iResult;

  switch (iColumn)   // which sort key
    {
    case eColumnSequence:
          if (trigger1->iSequence < trigger2->iSequence)
            iResult = -1;
          else if (trigger1->iSequence > trigger2->iSequence)
            iResult = 1;
          else 
            // if same sequence, sort in trigger match order
            iResult = trigger1->trigger.CompareNoCase (trigger2->trigger);
          break;
    case eColumnGroup:    iResult = trigger1->strGroup.CompareNoCase (trigger2->strGroup); 
                          if (iResult)
                            break;
    case eColumnLabel:    iResult = trigger1->strLabel.CompareNoCase (trigger2->strLabel); 
                          if (iResult)
                            break;
    case eColumnTrigger:  iResult = trigger1->trigger.CompareNoCase (trigger2->trigger); 
                          if (iResult)
                            break;
    case eColumnContents: iResult = trigger1->contents.CompareNoCase (trigger2->contents); 
                          break;
    default: iResult = 0;
    } // end of switch

  return iResult;
  }    // end of CPrefsP8::CompareObjects

void CPrefsP8::OnColumnclickTriggersList(NMHDR* pNMHDR, LRESULT* pResult) 
  {
   OnColumnclickItemList (pNMHDR, pResult);
  }   // end of CPrefsP8::OnColumnclickTriggersList

void CPrefsP8::OnLoadTrigger() 
{

  if (m_doc->Load_Set (TRIGGER,  "", this))
    return;

  LoadList ();

// They can no longer cancel the propery sheet, the document has changed

  CancelToClose ();
  m_doc->SetModifiedFlag (TRUE);

  // resolve all entry points for triggers, aliases etc.

  // if scripting active, find entry points
  if (m_doc->m_ScriptEngine)
    m_doc->FindAllEntryPoints ();


}   // end of CPrefsP8::OnLoadTrigger

void CPrefsP8::OnSaveTrigger() 
{
  if (m_doc->Save_Set (TRIGGER, this))
    return;
}   // end of CPrefsP8::OnSaveTrigger

int CPrefsP8::AddItem (CObject * pItem, 
                         const int item_number,
                         const BOOL insert)
  {
CTrigger * trigger_item = (CTrigger * ) pItem;

ASSERT_VALID (trigger_item);
ASSERT( trigger_item->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );

int nItem;

  if (insert)
    {
    nItem = m_ctlTriggerList.GetItemCount ();
   	m_ctlTriggerList.InsertItem (nItem, trigger_item->trigger);    // eColumnTrigger
    }
  else
    {
    nItem = item_number;
  	m_ctlTriggerList.SetItemText (nItem, eColumnTrigger, trigger_item->trigger);
    }

	m_ctlTriggerList.SetItemText (nItem, eColumnSequence, CFormat ("%i", trigger_item->iSequence));
	m_ctlTriggerList.SetItemText (nItem, eColumnContents, 
                                Replace (trigger_item->contents, ENDLINE, "\\n", true));
	m_ctlTriggerList.SetItemText (nItem, eColumnLabel, trigger_item->strLabel);
	m_ctlTriggerList.SetItemText (nItem, eColumnGroup, trigger_item->strGroup);

  return nItem;

  } // end of CPrefsP8::add_item

void CPrefsP8::OnMoveUp() 
{
CString strMsg;

// iterate through list in case we implement multiple selection one day
for (int nItem = -1;
      (nItem = m_ctlList->GetNextItem(nItem, LVNI_SELECTED)) != -1;)
  {

  // get the lower-case name of this item's object
  CString * pstrObjectName = (CString *) m_ctlList->GetItemData (nItem);

  ASSERT (pstrObjectName != NULL);

  CObject * pItem;

  // check object is still there (it might have gone while we looked at the list box)
  if (!m_ObjectMap->Lookup (*pstrObjectName, pItem))
    {
    m_ctlList->DeleteItem (nItem);    // it's gone, so delete it from the list view
    m_ctlList->RedrawItems (0, m_ctlList->GetItemCount () - 1);    // redraw the list

    strMsg = TFormat ("The %s named \"%s\" is no longer in the %s list",
                  (LPCTSTR) m_strObjectType,
                  (LPCTSTR) *pstrObjectName,
                  (LPCTSTR) m_strObjectType);

    ::UMessageBox (strMsg);

    delete pstrObjectName;                 // and get rid of its name string
    continue;
    }

  ASSERT_VALID (pItem);
  ASSERT( pItem->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );

  CTrigger * trigger = (CTrigger *) pItem;
  int iSeq = trigger->iSequence;

  if (iSeq == 0)
    ::TMessageBox ("Cannot move up - already has a sequence of zero");
  else
    {
  
    int iLower = -1;
    int bSame = false;

    for (int iItem = 0; iItem < m_doc->m_TriggerMap.GetCount (); iItem++)
      if (m_doc->m_TriggerArray [iItem] != trigger)
        if (m_doc->m_TriggerArray [iItem]->iSequence < trigger->iSequence)
          iLower = m_doc->m_TriggerArray [iItem]->iSequence;
        else if (m_doc->m_TriggerArray [iItem]->iSequence == trigger->iSequence)
          bSame = true;

    if (bSame)
      trigger->iSequence--;   // make a lower sequence (drop out of group)
    else
      if (iLower == -1)
        ::TMessageBox ("Cannot move up - already at top of list");
      else
        trigger->iSequence = iLower; // move into lower group

    m_doc->SortTriggers ();


    // They can no longer cancel the propery sheet, the document has changed
    CancelToClose ();
    m_doc->SetModifiedFlag (TRUE);

    // re-setup list with amended details
    add_item (pItem, pstrObjectName, nItem, FALSE);

    }

  }   // end of dealing with each selected item

  // resort the list
  t_gen_sort_param sort_param (m_ObjectMap, m_last_col, m_reverse, m_CompareObjects);
  m_ctlList->SortItems (CompareFunc, (LPARAM) &sort_param); 
  m_ctlList->RedrawItems (0, m_ctlList->GetItemCount () - 1);    // redraw the list
	
}

void CPrefsP8::OnMoveDown() 
{
CString strMsg;

// iterate through list in case we implement multiple selection one day
for (int nItem = -1;
      (nItem = m_ctlList->GetNextItem(nItem, LVNI_SELECTED)) != -1;)
  {

  // get the lower-case name of this item's object
  CString * pstrObjectName = (CString *) m_ctlList->GetItemData (nItem);

  ASSERT (pstrObjectName != NULL);

  CObject * pItem;

  // check object is still there (it might have gone while we looked at the list box)
  if (!m_ObjectMap->Lookup (*pstrObjectName, pItem))
    {
    m_ctlList->DeleteItem (nItem);    // it's gone, so delete it from the list view
    m_ctlList->RedrawItems (0, m_ctlList->GetItemCount () - 1);    // redraw the list

    strMsg = TFormat ("The %s named \"%s\" is no longer in the %s list",
                  (LPCTSTR) m_strObjectType,
                  (LPCTSTR) *pstrObjectName,
                  (LPCTSTR) m_strObjectType);

    ::UMessageBox (strMsg);

    delete pstrObjectName;                 // and get rid of its name string
    continue;
    }

  ASSERT_VALID (pItem);
  ASSERT( pItem->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );

  CTrigger * trigger = (CTrigger *) pItem;
  int iSeq = trigger->iSequence;

  if (iSeq == 10000)
    ::TMessageBox ("Cannot move down - already has a sequence of 10000");
  else
    {
  
    int iHigher = -1;
    int bSame = false;

    for (int iItem = 0; iItem < m_doc->m_TriggerMap.GetCount (); iItem++)
      if (m_doc->m_TriggerArray [iItem] != trigger)
        if (m_doc->m_TriggerArray [iItem]->iSequence > trigger->iSequence &&
            iHigher == -1)
          iHigher = m_doc->m_TriggerArray [iItem]->iSequence;
        else if (m_doc->m_TriggerArray [iItem]->iSequence == trigger->iSequence)
          bSame = true;

    if (bSame)
      trigger->iSequence++;   // make a higher sequence (jump out of group)
    else
      if (iHigher == -1)
        ::TMessageBox ("Cannot move down - already at bottom of list");
      else
        trigger->iSequence = iHigher; // move into Higher group

    m_doc->SortTriggers ();


    // They can no longer cancel the propery sheet, the document has changed
    CancelToClose ();
    m_doc->SetModifiedFlag (TRUE);

    // re-setup list with amended details
    add_item (pItem, pstrObjectName, nItem, FALSE);

    }

  }   // end of dealing with each selected item

  // resort the list
  t_gen_sort_param sort_param (m_ObjectMap, m_last_col, m_reverse, m_CompareObjects);
  m_ctlList->SortItems (CompareFunc, (LPARAM) &sort_param); 
  m_ctlList->RedrawItems (0, m_ctlList->GetItemCount () - 1);    // redraw the list
	
	
}

void CPrefsP8::OnUpdateCanSequence(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_last_col == eColumnSequence && !m_reverse  &&
                  m_ctlList->GetSelectedCount () == 1  &&
                  (m_ctlUseDefaultTriggers.GetCheck () == 0 ||
                  App.m_strDefaultTriggersFile.IsEmpty ()));
  } // end of CPrefsP8::OnUpdateCanSequence


void CPrefsP8::OnUpdateNotUsingDefaults(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlUseDefaultTriggers.GetCheck () == 0 ||
                  App.m_strDefaultTriggersFile.IsEmpty ());
  } // end of CPrefsP5::OnUpdateNotUsingDefaults

void CPrefsP8::OnUpdateHaveDefaults(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (!App.m_strDefaultTriggersFile.IsEmpty ());
  } // end of CPrefsP5::OnUpdateHaveDefaults


void CPrefsP8::OnUpdateNeedSelection(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlList->GetSelectedCount () != 0   &&
                  (m_ctlUseDefaultTriggers.GetCheck () == 0 ||
                  App.m_strDefaultTriggersFile.IsEmpty ()));
  } // end of CGenPropertyPage::OnUpdateNeedSelection

void CPrefsP8::OnUpdateNeedOneSelection(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlList->GetSelectedCount () == 1  &&
                  (m_ctlUseDefaultTriggers.GetCheck () == 0 ||
                  App.m_strDefaultTriggersFile.IsEmpty ()));
  } // end of CGenPropertyPage::OnUpdateNeedOneSelection

void CPrefsP8::OnUpdateNeedXMLClipboard(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (IsClipboardXML ());
  } // end of CPrefsP8::OnUpdateNeedXMLClipboard

void CPrefsP8::OnCopy() 
{
OnCopyItem ();	
}

void CPrefsP8::OnPaste() 
{
OnPasteItem ();	
}

bool CPrefsP8::CheckIfIncluded (CObject * pItem)
  {
CTrigger * trigger_item = (CTrigger * ) pItem;

ASSERT_VALID (trigger_item);
ASSERT( trigger_item->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );
  
  return trigger_item->bIncluded;
  }    // end of CPrefsP8::CheckIfIncluded

bool CPrefsP8::CheckIfTemporary (CObject * pItem)
  {
CTrigger * trigger_item = (CTrigger * ) pItem;

ASSERT_VALID (trigger_item);
ASSERT( trigger_item->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );
  
  return trigger_item->bTemporary;
  }    // end of CPrefsP8::CheckIfTemporary

bool CPrefsP8::CheckIfExecuting (CObject * pItem)
  {
CTrigger * trigger_item = (CTrigger * ) pItem;

ASSERT_VALID (trigger_item);
ASSERT( trigger_item->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );
  
  return trigger_item->bExecutingScript;
  }    // end of CPrefsP8::CheckIfExecuting

void CPrefsP8::OnEditFilter() 
{
if (EditFilterText (m_doc->m_strTriggersFilter))
  LoadList ();  // need to reload the list now	
}

void CPrefsP8::OnFilter() 
{
  LoadList ();  // need to reload the list now	
}

bool CPrefsP8::GetFilterFlag ()            // is filtering enabled?
  {
  return m_ctlFilter.GetCheck ();
  }

CString CPrefsP8::GetFilterScript ()       // get the filter script
  {
  return m_doc->m_strTriggersFilter;
  }

void CPrefsP8::GetFilterInfo (CObject * pItem, lua_State * L)
  {
CTrigger * trigger = (CTrigger * ) pItem;

ASSERT_VALID (trigger);
ASSERT( trigger->IsKindOf( RUNTIME_CLASS( CTrigger ) ) );


  // the table of stuff
  lua_newtable(L);                                                            

  MakeTableItem     (L, "back_colour",       (trigger->iMatch >> 8) & 0x0F);
  MakeTableItemBool (L, "bold",              trigger->iMatch & HILITE);
  MakeTableItem     (L, "clipboard_arg",     trigger->iClipboardArg);
  if (trigger->colour == SAMECOLOUR)   // 
   MakeTableItem    (L, "custom_colour",     0); 
  else
   MakeTableItem    (L, "custom_colour",     trigger->colour + 1); // + 1, to match custom colours
  MakeTableItem     (L, "colour_change_type",trigger->iColourChangeType);
  MakeTableItemBool (L, "enabled",           trigger->bEnabled);
  MakeTableItemBool (L, "expand_variables",  trigger->bExpandVariables);
  MakeTableItem     (L, "group",             trigger->strGroup);
  MakeTableItemBool (L, "ignore_case",       trigger->ignore_case);
  MakeTableItemBool (L, "inverse",           trigger->iMatch & INVERSE);
  MakeTableItemBool (L, "italic",            trigger->iMatch & BLINK);
  MakeTableItem     (L, "lines_to_match",    trigger->iLinesToMatch);
  MakeTableItemBool (L, "keep_evaluating",   trigger->bKeepEvaluating);
  MakeTableItemBool (L, "make_bold",         trigger->iStyle & HILITE);
  MakeTableItemBool (L, "make_italic",       trigger->iStyle & BLINK);
  MakeTableItemBool (L, "make_underline",    trigger->iStyle & UNDERLINE);
  MakeTableItem     (L, "match",             trigger->trigger);
  MakeTableItemBool (L, "match_back_colour", trigger->iMatch & TRIGGER_MATCH_BACK);
  MakeTableItemBool (L, "match_bold",        trigger->iMatch & TRIGGER_MATCH_HILITE);
  MakeTableItemBool (L, "match_inverse",     trigger->iMatch & TRIGGER_MATCH_INVERSE);
  MakeTableItemBool (L, "match_italic",      trigger->iMatch & TRIGGER_MATCH_BLINK);
  MakeTableItemBool (L, "match_text_colour", trigger->iMatch & TRIGGER_MATCH_TEXT);
  MakeTableItemBool (L, "match_underline",   trigger->iMatch & TRIGGER_MATCH_UNDERLINE);
  MakeTableItemBool (L, "multi_line",        trigger->bMultiLine);
  MakeTableItem     (L, "name",              trigger->strLabel);
  MakeTableItemBool (L, "one_shot",          trigger->bOneShot);
  MakeTableItemBool (L, "omit_from_log",     trigger->omit_from_log);
  MakeTableItemBool (L, "omit_from_output",  trigger->bOmitFromOutput);
  MakeTableItem     (L, "other_text_colour", trigger->iOtherForeground);
  MakeTableItem     (L, "other_back_colour", trigger->iOtherBackground);
  MakeTableItemBool (L, "regexp",            trigger->bRegexp);
  MakeTableItemBool (L, "repeat",            trigger->bRepeat);
  MakeTableItem     (L, "script",            trigger->strProcedure);
  MakeTableItem     (L, "send", trigger->contents);
  MakeTableItem     (L, "send_to",           trigger->iSendTo);
  MakeTableItem     (L, "sequence",          trigger->iSequence);
  MakeTableItem     (L, "sound",             trigger->sound_to_play);
  MakeTableItemBool (L, "sound_if_inactive", trigger->bSoundIfInactive);
  MakeTableItemBool (L, "lowercase_wildcard",trigger->bLowercaseWildcard);
  MakeTableItemBool (L, "temporary",         trigger->bTemporary);
  MakeTableItem     (L, "text_colour",       (trigger->iMatch >> 4) & 0x0F);
  MakeTableItem     (L, "user",              trigger->iUserOption);
  MakeTableItem     (L, "variable",          trigger->strVariable);


  // stuff below is not part of the world file but calculated at runtime

  //  GetTriggerInfo  (20)
  MakeTableItem (L, "invocation_count", trigger->nInvocationCount);
  
  //  GetTriggerInfo  (21)
  MakeTableItem (L, "times_matched",    trigger->nMatched);

  //  GetTriggerInfo  (22)
  if (trigger->tWhenMatched.GetTime ())  
    MakeTableItem   (L, "when_matched", COleDateTime (trigger->tWhenMatched.GetTime ())); 

  //  GetTriggerInfo  (23)
  MakeTableItemBool (L, "temporary",    trigger->bTemporary);

  //  GetTriggerInfo  (24)
  MakeTableItemBool (L, "included",     trigger->bIncluded);

  //  GetTriggerInfo  (31)
  if (trigger->regexp)      
    MakeTableItem   (L, "match_count",  trigger->regexp->m_iCount);

  //  GetTriggerInfo  (32)
  if (trigger->regexp)      
    MakeTableItem   (L, "last_match",   trigger->regexp->m_sTarget);

  //  GetTriggerInfo  (34)
  MakeTableItemBool (L, "script_valid", trigger->dispid != DISPID_UNKNOWN);

  //  GetTriggerInfo  (35)
  if (trigger->regexp && trigger->regexp->m_program == NULL)      
    MakeTableItem   (L, "execution_error", trigger->regexp->m_iExecutionError);

  //  GetTriggerInfo  (37)
  if (trigger->regexp && App.m_iCounterFrequency)
    MakeTableItem (L, "execution_time", ((double) trigger->regexp->iTimeTaken) / 
                                        ((double) App.m_iCounterFrequency));

  //  GetTriggerInfo  (38)
  if (trigger->regexp)
    MakeTableItem   (L, "match_attempts", trigger->regexp->m_iMatchAttempts);

  }   // end of CPrefsP8::GetFilterInfo

/////////////////////////////////////////////////////////////////////////////
// CPrefsP9 property page

IMPLEMENT_DYNCREATE(CPrefsP9, CPropertyPage)

CPrefsP9::CPrefsP9() : CPropertyPage(CPrefsP9::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP9)
	m_command_stack_character = _T("");
	m_enable_command_stacking = FALSE;
	m_enable_speed_walk = FALSE;
	m_speed_walk_prefix = _T("");
	m_echo_input = FALSE;
	m_input_font_name = _T("");
	m_nHistoryLines = 0;
	m_bAutoRepeat = FALSE;
	m_bTranslateGerman = FALSE;
	m_bSpellCheckOnSend = FALSE;
	m_strSpeedWalkFiller = _T("");
	m_bLowerCaseTabCompletion = FALSE;
	m_echo_colour = -1;
	m_bUseDefaultInputFont = FALSE;
	m_strInputStyle = _T("");
	m_bTranslateBackslashSequences = FALSE;
	m_bKeepCommandsOnSameLine = FALSE;
	m_iSpeedWalkDelay = 0;
	m_bNoEchoOff = FALSE;
	m_bEnableSpamPrevention = FALSE;
	m_iSpamLineCount = 0;
	m_bConfirmBeforeReplacingTyping = FALSE;
	m_bEscapeDeletesInput = FALSE;
	m_bArrowsChangeHistory = FALSE;
	m_bAltArrowRecallsPartial = FALSE;
	m_bDoubleClickInserts = FALSE;
	m_bArrowKeysWrap = FALSE;
	m_bDoubleClickSends = FALSE;
	m_bSaveDeletedCommand = FALSE;
	m_bArrowRecallsPartial = FALSE;
	m_strSpamMessage = _T("");
	//}}AFX_DATA_INIT
  m_page_number = 1;
  m_doc = NULL;
  m_prefsp3 = NULL;

}

CPrefsP9::~CPrefsP9()
{
}

void CPrefsP9::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CPrefsP9)
	DDX_Control(pDX, IDC_INPUT_STYLE, m_ctlInputStyle);
	DDX_Control(pDX, IDC_USE_DEFAULT_INPUT_FONT, m_ctlUseDefaultInputFont);
	DDX_Control(pDX, IDC_ECHO_COLOUR, m_ctlInputColour);
	DDX_Control(pDX, IDC_INPUT_SWATCH2, m_ctlInputSwatch2);
	DDX_Control(pDX, IDC_FONT_NAME, m_ctlFontName);
	DDX_Control(pDX, IDC_BACKGROUND_SWATCH, m_ctlBackgroundSwatch);
	DDX_Control(pDX, IDC_TEXT_SWATCH, m_ctlTextSwatch);
	DDX_Control(pDX, IDC_INPUT_SWATCH, m_ctlInputSwatch);
	DDX_Text(pDX, IDC_COMMAND_STACK_CHARACTER, m_command_stack_character);
	DDV_MaxChars(pDX, m_command_stack_character, 1);
	DDX_Check(pDX, IDC_ENABLE_COMMAND_STACKING, m_enable_command_stacking);
	DDX_Check(pDX, IDC_ENABLE_SPEED_WALK, m_enable_speed_walk);
	DDX_Text(pDX, IDC_SPEED_WALK_PREFIX, m_speed_walk_prefix);
	DDX_Check(pDX, IDC_ECHO_INPUT, m_echo_input);
	DDX_Text(pDX, IDC_FONT_NAME, m_input_font_name);
	DDX_Text(pDX, IDC_HISTORY_LINES, m_nHistoryLines);
	DDV_MinMaxInt(pDX, m_nHistoryLines, 20, 5000);
	DDX_Check(pDX, IDC_AUTO_REPEAT, m_bAutoRepeat);
	DDX_Check(pDX, IDC_TRANSLATE_GERMAN, m_bTranslateGerman);
	DDX_Check(pDX, IDC_SPELL_CHECK_ON_SEND, m_bSpellCheckOnSend);
	DDX_Text(pDX, IDC_SPEED_WALK_FILLER, m_strSpeedWalkFiller);
	DDX_Check(pDX, IDC_LOWER_CASE_TAB_COMPLETION, m_bLowerCaseTabCompletion);
	DDX_CBIndex(pDX, IDC_ECHO_COLOUR, m_echo_colour);
	DDX_Check(pDX, IDC_USE_DEFAULT_INPUT_FONT, m_bUseDefaultInputFont);
	DDX_Text(pDX, IDC_INPUT_STYLE, m_strInputStyle);
	DDX_Check(pDX, IDC_TRANSLATE_BACKSLASH, m_bTranslateBackslashSequences);
	DDX_Check(pDX, IDC_TRANSLATE_BACKSLASH2, m_bKeepCommandsOnSameLine);
	DDX_Text(pDX, IDC_SPEED_WALK_DELAY, m_iSpeedWalkDelay);
	DDV_MinMaxInt(pDX, m_iSpeedWalkDelay, 0, 30000);
	DDX_Check(pDX, IDC_NO_ECHO_OFF, m_bNoEchoOff);
	DDX_Check(pDX, IDC_ENABLE_SPAM_PREVENTION, m_bEnableSpamPrevention);
	DDX_Text(pDX, IDC_SPAM_LINE_COUNT, m_iSpamLineCount);
	DDV_MinMaxInt(pDX, m_iSpamLineCount, 5, 500);
	DDX_Text(pDX, IDC_SPAM_FILLER, m_strSpamMessage);
	//}}AFX_DATA_MAP

 if(pDX->m_bSaveAndValidate)
   {


    if(m_enable_speed_walk && m_speed_walk_prefix.IsEmpty ())
        {
        ::TMessageBox("You must supply a speed-walk prefix.");
        DDX_Text(pDX, IDC_SPEED_WALK_PREFIX, m_speed_walk_prefix);
        pDX->Fail();
        }     // end of contents being blank


    if(m_enable_command_stacking && m_command_stack_character.IsEmpty ())
        {
        ::TMessageBox("You must supply a command stack character.");
        DDX_Text(pDX, IDC_COMMAND_STACK_CHARACTER, m_command_stack_character);
        pDX->Fail();
        }     // end of command stack character being blank

    if(m_enable_command_stacking && !isprint (m_command_stack_character [0]))
        {
        ::TMessageBox("The command stack character is invalid.");
        DDX_Text(pDX, IDC_COMMAND_STACK_CHARACTER, m_command_stack_character);
        pDX->Fail();
        }     // end of command stack character being blank


// if command stacking not enabled, just put a space in as the character

    if(!m_enable_command_stacking && m_command_stack_character.IsEmpty ())
      m_command_stack_character = " ";

  } // end of saving and validating

}


BEGIN_MESSAGE_MAP(CPrefsP9, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP9)
	ON_CBN_SELCHANGE(IDC_ECHO_COLOUR, OnSelchangeEchoColour)
	ON_BN_CLICKED(IDC_INPUT_FONT, OnInputFont)
	ON_BN_CLICKED(IDC_TAB_COMPLETION_DEFAULTS, OnTabCompletionDefaults)
	ON_BN_CLICKED(IDC_TEXT_SWATCH, OnTextSwatch)
	ON_BN_CLICKED(IDC_BACKGROUND_SWATCH, OnBackgroundSwatch)
	ON_BN_CLICKED(IDC_KEYBOARD_PREFS, OnKeyboardPrefs)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_SPELL_CHECK_ON_SEND, OnUpdateNeedSpellCheck)
  ON_UPDATE_COMMAND_UI(IDC_SPELL_CHECK_OPTIONS, OnUpdateNeedSpellCheck)
  ON_UPDATE_COMMAND_UI(IDC_INPUT_FONT, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_USE_DEFAULT_INPUT_FONT, OnUpdateHaveDefaults)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPrefsP9 message handlers


void CPrefsP9::FixSwatch (CColourButton & m_ctlSwatch, 
                          CColourButton & m_ctlSwatch2,
                          const int iNewColour)
  {
  if (iNewColour == 0)
    {
     m_ctlSwatch.ShowWindow (SW_HIDE);
     m_ctlSwatch2.ShowWindow (SW_HIDE);
    }
  else
    {
    m_ctlSwatch.ShowWindow (SW_SHOW | SW_SHOWNOACTIVATE);
    m_ctlSwatch2.ShowWindow (SW_SHOW | SW_SHOWNOACTIVATE);
    m_ctlSwatch.m_colour = m_prefsp3->m_customtext [iNewColour - 1];
    m_ctlSwatch2.m_colour = m_prefsp3->m_customback [iNewColour - 1];
    m_ctlSwatch.RedrawWindow ();
    m_ctlSwatch2.RedrawWindow ();
    }
  } // end of FixSwatch


BOOL CPrefsP9::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
 	((CSpinButtonCtrl*) GetDlgItem(IDC_SPIN_HISTORY_LINES))->SetRange(20, 5000);
   
  m_ctlTextSwatch.m_colour = m_input_text_colour;
  m_ctlBackgroundSwatch.m_colour = m_input_background_colour;

  // load combo box now
  m_ctlInputColour.ResetContent ();
  m_ctlInputColour.AddString ("(no change)");
  for (int i = 0; i < MAX_CUSTOM; i++)
    m_ctlInputColour.AddString (m_prefsp3->m_strCustomName [i]);

  // put the selection back
  m_ctlInputColour.SetCurSel (m_echo_colour);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPrefsP9::OnSelchangeEchoColour() 
{
  if (m_ctlInputColour.GetCurSel () != CB_ERR)
     FixSwatch (m_ctlInputSwatch, 
                m_ctlInputSwatch2,
                m_ctlInputColour.GetCurSel ());
}

void CPrefsP9::OnTextSwatch() 
{

  CColourPickerDlg dlg;
  dlg.m_iColour = m_ctlTextSwatch.m_colour;
  dlg.m_bPickColour = true;

  /*
CColorDialog dlg (m_ctlTextSwatch.m_colour, 
                  CC_RGBINIT | CC_FULLOPEN, 
                  GetOwner());
  
    */

  if (dlg.DoModal () != IDOK)
    return;

  m_ctlTextSwatch.m_colour = dlg.GetColor ();
  m_ctlTextSwatch.RedrawWindow();
	
}

void CPrefsP9::OnBackgroundSwatch() 
{

  CColourPickerDlg dlg;
  dlg.m_iColour = m_ctlBackgroundSwatch.m_colour;
  dlg.m_bPickColour = true;

  /*
CColorDialog dlg (m_ctlBackgroundSwatch.m_colour, 
                  CC_RGBINIT | CC_FULLOPEN, 
                  GetOwner());
  
    */

  if (dlg.DoModal () != IDOK)
    return;

  m_ctlBackgroundSwatch.m_colour = dlg.GetColor ();
  m_ctlBackgroundSwatch.RedrawWindow();
}

void CPrefsP9::OnOK( )
  {

// remember colour of text
  
  m_input_text_colour = m_ctlTextSwatch.m_colour;
  m_input_background_colour = m_ctlBackgroundSwatch.m_colour;

  CPropertyPage::OnOK ();
    
  }

void CPrefsP9::OnInputFont() 
{
CDC dc;

dc.CreateCompatibleDC (NULL);

LOGFONT lf;

ZeroMemory (&lf, sizeof lf);

lf.lfHeight = -MulDiv(m_input_font_height, dc.GetDeviceCaps(LOGPIXELSY), 72);
strncpy (lf.lfFaceName, m_input_font_name, sizeof (lf.lfFaceName) - 1);
lf.lfFaceName [sizeof (lf.lfFaceName) - 1] = 0;
lf.lfWeight = m_input_font_weight;
lf.lfItalic = m_input_font_italic;
lf.lfCharSet = m_input_font_charset;

CFontDialog dlg (&lf,
                 CF_SCREENFONTS,
                 NULL,
                 GetOwner ());       // owner view

if (dlg.DoModal () != IDOK)
  return;

m_input_font_height = dlg.GetSize () / 10;
m_input_font_name   = dlg.GetFaceName (); 
m_input_font_weight = dlg.GetWeight (); 
m_input_font_italic = dlg.IsItalic ();
m_input_font_charset = lf.lfCharSet;

m_ctlFontName.SetWindowText(m_input_font_name);
	
CString strStyle = CFormat ("%i pt.", m_input_font_height);
if (m_input_font_weight == FW_BOLD)
  strStyle += " bold";
if (m_input_font_italic)
  strStyle += " italic";
                    
m_ctlInputStyle.SetWindowText(strStyle);

}

void CPrefsP9::OnTabCompletionDefaults() 
{
CTabDefaultsDlg dlg;

  dlg.m_strWordList = m_strTabCompletionDefaults;
  dlg.m_iTabCompletionLines = m_iTabCompletionLines;
  dlg.m_bTabCompletionSpace = m_bTabCompletionSpace;
	
  if (dlg.DoModal () != IDOK)
    return;;

  m_strTabCompletionDefaults = dlg.m_strWordList;
  m_iTabCompletionLines = dlg.m_iTabCompletionLines;
  m_bTabCompletionSpace = dlg.m_bTabCompletionSpace;

}


LRESULT CPrefsP9::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CPrefsP9::OnKickIdle

void CPrefsP9::OnUpdateNeedSpellCheck(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (App.m_bSpellCheckOK);
  } // end of CPrefsP9::OnUpdateNeedSpellCheck


void CPrefsP9::OnUpdateNotUsingDefaults(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlUseDefaultInputFont.GetCheck () == 0 ||
                  App.m_strDefaultInputFont.IsEmpty ());
  } // end of CPrefsP9::OnUpdateNotUsingDefaults

void CPrefsP9::OnUpdateHaveDefaults(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (!App.m_strDefaultInputFont.IsEmpty ());
  } // end of CPrefsP9::OnUpdateHaveDefaults


void CPrefsP9::OnKeyboardPrefs() 
{
CCommandOptionsDlg dlg;

  dlg.m_bConfirmBeforeReplacingTyping = m_bConfirmBeforeReplacingTyping;
  dlg.m_bEscapeDeletesInput           = m_bEscapeDeletesInput;  
  dlg.m_bArrowsChangeHistory          = m_bArrowsChangeHistory; 
  dlg.m_bAltArrowRecallsPartial       = m_bAltArrowRecallsPartial;
  dlg.m_bArrowRecallsPartial          = m_bArrowRecallsPartial;
  dlg.m_bDoubleClickInserts           = m_bDoubleClickInserts;
  dlg.m_bDoubleClickSends             = m_bDoubleClickSends;
  dlg.m_bSaveDeletedCommand           = m_bSaveDeletedCommand;
  dlg.m_bArrowKeysWrap                = m_bArrowKeysWrap;
  dlg.m_bCtrlZGoesToEndOfBuffer       = m_bCtrlZGoesToEndOfBuffer;
  dlg.m_bCtrlPGoesToPreviousCommand   = m_bCtrlPGoesToPreviousCommand;
  dlg.m_bCtrlNGoesToNextCommand       = m_bCtrlNGoesToNextCommand;
	
  if (dlg.DoModal () != IDOK)
    return;

  m_bConfirmBeforeReplacingTyping = dlg.m_bConfirmBeforeReplacingTyping;
  m_bEscapeDeletesInput           = dlg.m_bEscapeDeletesInput;  
  m_bArrowsChangeHistory          = dlg.m_bArrowsChangeHistory; 
  m_bAltArrowRecallsPartial       = dlg.m_bAltArrowRecallsPartial;
  m_bArrowRecallsPartial          = dlg.m_bArrowRecallsPartial;
  m_bDoubleClickInserts           = dlg.m_bDoubleClickInserts;
  m_bDoubleClickSends             = dlg.m_bDoubleClickSends;
  m_bSaveDeletedCommand           = dlg.m_bSaveDeletedCommand;
  m_bArrowKeysWrap                = dlg.m_bArrowKeysWrap;
  m_bCtrlZGoesToEndOfBuffer       = dlg.m_bCtrlZGoesToEndOfBuffer;
  m_bCtrlPGoesToPreviousCommand   = dlg.m_bCtrlPGoesToPreviousCommand;
  m_bCtrlNGoesToNextCommand       = dlg.m_bCtrlNGoesToNextCommand;


}

/////////////////////////////////////////////////////////////////////////////
// CPrefsP10 property page

IMPLEMENT_DYNCREATE(CPrefsP10, CPropertyPage)

CPrefsP10::CPrefsP10() : CPropertyPage(CPrefsP10::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP10)
	m_file_postamble = _T("");
	m_file_preamble = _T("");
	m_line_postamble = _T("");
	m_line_preamble = _T("");
	m_bCommentedSoftcode = FALSE;
	m_iLineDelay = 0;
	m_bConfirmOnSend = FALSE;
	m_bEcho = FALSE;
	m_nFileDelayPerLines = 0;
	//}}AFX_DATA_INIT
  m_page_number = 1;
  m_doc = NULL;

}

CPrefsP10::~CPrefsP10()
{
}

void CPrefsP10::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP10)
	DDX_Text(pDX, IDC_FILE_POSTAMBLE, m_file_postamble);
	DDX_Text(pDX, IDC_FILE_PREAMBLE, m_file_preamble);
	DDX_Text(pDX, IDC_LINE_POSTAMBLE, m_line_postamble);
	DDX_Text(pDX, IDC_LINE_PREAMBLE, m_line_preamble);
	DDX_Check(pDX, IDC_COMMENTED_SOFTCODE, m_bCommentedSoftcode);
	DDX_Text(pDX, IDC_LINE_DELAY, m_iLineDelay);
	DDV_MinMaxLong(pDX, m_iLineDelay, 0, 10000);
	DDX_Check(pDX, IDC_CONFIRM_ON_SEND, m_bConfirmOnSend);
	DDX_Check(pDX, IDC_ECHO, m_bEcho);
	DDX_Text(pDX, IDC_LINE_DELAY_PER_LINES, m_nFileDelayPerLines);
	DDV_MinMaxLong(pDX, m_nFileDelayPerLines, 1, 100000);
	//}}AFX_DATA_MAP


}


BEGIN_MESSAGE_MAP(CPrefsP10, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP10)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPrefsP10 message handlers

/////////////////////////////////////////////////////////////////////////////
// CPrefsP11 property page

IMPLEMENT_DYNCREATE(CPrefsP11, CPropertyPage)

CPrefsP11::CPrefsP11() : CPropertyPage(CPrefsP11::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP11)
	m_notes = _T("");
	//}}AFX_DATA_INIT
  m_page_number = 1;
  m_doc = NULL;
  m_pNotesFindInfo = NULL;
}

CPrefsP11::~CPrefsP11()
{
}

void CPrefsP11::DoDataExchange(CDataExchange* pDX)
{

	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP11)
	DDX_Control(pDX, IDC_NOTES, m_ctlNotes);
	DDX_Text(pDX, IDC_NOTES, m_notes);
	DDV_MaxChars(pDX, m_notes, 32000);
	//}}AFX_DATA_MAP

}


BEGIN_MESSAGE_MAP(CPrefsP11, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP11)
	ON_BN_CLICKED(IDC_LOAD_NOTES, OnLoadNotes)
	ON_BN_CLICKED(IDC_SAVE_NOTES, OnSaveNotes)
	ON_EN_SETFOCUS(IDC_NOTES, OnSetfocusNotes)
	ON_BN_CLICKED(IDC_FIND, OnFind)
	ON_BN_CLICKED(IDC_FIND_NEXT, OnFindNext)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_SAVE_NOTES, OnUpdateNeedText)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPrefsP11 message handlers

void CPrefsP11::OnLoadNotes() 
{
CFile * f = NULL;
CString filename;
DWORD length;

  CFileDialog filedlg (TRUE,   // loading the file
                       "txt",    // default extension
                       "",  // suggested name
                       OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
                       "Text files (*.txt)|*.txt||",    // filter 
                       NULL);  // parent window

  filedlg.m_ofn.lpstrTitle = "File to load notes from";
  filedlg.m_ofn.lpstrFile = filename.GetBuffer (_MAX_PATH); // needed!! (for Win32s)  
  strcpy (filedlg.m_ofn.lpstrFile, "");

  ChangeToFileBrowsingDirectory ();
  int nResult = filedlg.DoModal();
  ChangeToStartupDirectory ();

  if (nResult != IDOK)
    return;    // cancelled dialog

  try
    {
    f = new CFile (filedlg.GetPathName (), CFile::modeRead | CFile::shareDenyWrite);
    
    length = f->GetLength ();

    if (length > 32000)
      ::TMessageBox ("File exceeds 32000 bytes in length, cannot be loaded", 
                       MB_ICONSTOP);
    else
      if (length <= 0)
      ::TMessageBox ("File is empty", MB_ICONSTOP);
    else
      {
      f->Read (m_notes.GetBuffer (length), length);
      m_notes.ReleaseBuffer (length);
      UpdateData (FALSE);   // reload and redraw dialog box
      }
    
    } // end of try block

  catch (CFileException * e)
    {
    ::TMessageBox ("Unable to open or read the requested file", MB_ICONEXCLAMATION);
    e->Delete ();
    } // end of catching a file exception

   catch (CMemoryException * e)
    {
    ::TMessageBox ("Insufficient memory to do this operation", MB_ICONEXCLAMATION);
    e->Delete ();
    } // end of catching a memory exception


  delete f;       // delete file

}

void CPrefsP11::OnSaveNotes() 
{
CFile * f = NULL;
CString filename;
CString suggested_name;

CEdit * pEdit = (CEdit *) GetDlgItem (IDC_NOTES);
ASSERT (pEdit);

CString str;

  suggested_name = m_doc->m_mush_name;
  suggested_name += " notes";

  CFileDialog filedlg (TRUE,   // loading the file
                       "txt",    // default extension
                       "",  // suggested name
                       OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                       "Text files (*.txt)|*.txt||",    // filter 
                       NULL);  // parent window

  filedlg.m_ofn.lpstrTitle = "File to save notes into";
  filedlg.m_ofn.lpstrFile = filename.GetBuffer (_MAX_PATH); // needed!! (for Win32s)  
  if (App.platform == VER_PLATFORM_WIN32s)
    strcpy (filedlg.m_ofn.lpstrFile, "");
  else
    strcpy (filedlg.m_ofn.lpstrFile, suggested_name);

  ChangeToFileBrowsingDirectory ();
  int nResult = filedlg.DoModal();
  ChangeToStartupDirectory ();

  if (nResult != IDOK)
    return;    // cancelled dialog

  try
    {
    f = new CFile (filedlg.GetPathName (), CFile::modeWrite | CFile::modeCreate);

		pEdit->GetWindowText(str);

    f->Write (str, str.GetLength ());
    } // end of try block

  catch (CFileException * e)
    {
    ::TMessageBox ("Unable to open or write the requested file", MB_ICONEXCLAMATION);
    e->Delete ();
    } // end of catching a file exception

   catch (CMemoryException * e)
    {
    ::TMessageBox ("Insufficient memory to do this operation", MB_ICONEXCLAMATION);
    e->Delete ();
    } // end of catching a memory exception


  delete f;       // delete file

}


void CPrefsP11::OnSetfocusNotes() 
{
CEdit * pEdit = (CEdit *) GetDlgItem (IDC_NOTES);
ASSERT (pEdit);

int nStartChar, 
    nEndChar;


  pEdit->GetSel (nStartChar, nEndChar);
	nEndChar = nStartChar;
  pEdit->SetSel (nStartChar, nEndChar);

}


void CPrefsP11::OnFind() 
{
  DoFind (false);
}

void CPrefsP11::OnFindNext() 
{
  DoFind (true);
}

void CPrefsP11::DoFind (bool bAgain)
  {

CEdit * pEdit = (CEdit *) GetDlgItem (IDC_NOTES);
ASSERT (pEdit);
int nStartChar,
    nEndChar;
 
pEdit->GetSel (nStartChar, nEndChar);

CString strText;

CFindInfo & FindInfo = *m_pNotesFindInfo;

  pEdit->GetWindowText (strText);

  FindInfo.m_bAgain = bAgain;
  FindInfo.m_nTotalLines = strText.GetLength ();

  // have to do this myself because:

  // a) Finding text in notes is character-oriented, not line oriented
  // b) pEdit->LineLength returns the wrong value (seems to always be the
  //    line the cursor is in)

  //  m_nCurrentLine  <------ this will be the current *character* in this case

CFindDlg dlg (FindInfo.m_strFindStringList);

  if (!FindInfo.m_bAgain || FindInfo.m_strFindStringList.IsEmpty ())
    {
    if (!FindInfo.m_strFindStringList.IsEmpty ())
      dlg.m_strFindText = FindInfo.m_strFindStringList.GetHead ();

    dlg.m_bMatchCase  = FindInfo.m_bMatchCase;
    dlg.m_bForwards   = FindInfo.m_bForwards;
    dlg.m_strTitle    = FindInfo.m_strTitle;
    dlg.m_bRegexp     = FindInfo.m_bRegexp;

    if (dlg.DoModal () != IDOK)
      return;

    // add find string to head of list, provided it is not empty, and not the same as before
    if (!dlg.m_strFindText.IsEmpty () &&
        (FindInfo.m_strFindStringList.IsEmpty () ||
        FindInfo.m_strFindStringList.GetHead () != dlg.m_strFindText))
      FindInfo.m_strFindStringList.AddHead (dlg.m_strFindText);

    FindInfo.m_bMatchCase    = dlg.m_bMatchCase;
    FindInfo.m_bForwards     = dlg.m_bForwards;
//    FindInfo.m_bUTF8         = m_bUTF_8;

    if (dlg.m_bRegexp)
      {
      ::TMessageBox ("Regular expressions not supported here.",
          MB_ICONEXCLAMATION);
      return;
      }

//    FindInfo.m_bRegexp       = dlg.m_bRegexp;

    if (FindInfo.m_bForwards)
       FindInfo.m_nCurrentLine = 0;
    else
      FindInfo.m_nCurrentLine = strText.GetLength () -
                                FindInfo.m_strFindStringList.GetHead ().GetLength ();

    FindInfo.m_bAgain = false;

    }   // end of not repeating the last find
  else

// doing a "find again" - step past the line we were on

    if (FindInfo.m_bForwards)
      FindInfo.m_nCurrentLine++;
    else
      FindInfo.m_nCurrentLine--;

CString strFindString = FindInfo.m_strFindStringList.GetHead ();

// if case-insensitive search wanted, force "text to find",
// and "text we are searching" to lower case

  if (!FindInfo.m_bMatchCase)
    {
    strText.MakeLower ();
    strFindString.MakeLower ();
    }

  int iFindLength = strFindString.GetLength ();

  while (FindInfo.m_nCurrentLine >= 0 && 
        (FindInfo.m_nCurrentLine + iFindLength) <= FindInfo.m_nTotalLines)
    {

// if text found on this line, then we have done it!
  
    if (strText.Mid (FindInfo.m_nCurrentLine, iFindLength) ==
        strFindString)
        break;

// keep track of line count

    if (FindInfo.m_bForwards)
      FindInfo.m_nCurrentLine++;
    else
      FindInfo.m_nCurrentLine--;

    }   // end of looping through each line

  // if we have shuffled off the end of the buffer, say "not found"
  if (FindInfo.m_nCurrentLine < 0 || 
      (FindInfo.m_nCurrentLine + iFindLength) > FindInfo.m_nTotalLines)
    {
    // reset line pointer to a reasonable figure
    if (FindInfo.m_bForwards)
       FindInfo.m_nCurrentLine = 0;
    else
      FindInfo.m_nCurrentLine = FindInfo.m_nTotalLines - iFindLength - 1;
    NotFound (FindInfo);
    pEdit->SetSel (nStartChar, nStartChar);
    return;
    }

  pEdit->SetSel (FindInfo.m_nCurrentLine, 
                 FindInfo.m_nCurrentLine +
                 strFindString.GetLength ());

  } // end of CPrefsP11::DoFind



LRESULT CPrefsP11::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CPrefsP11::OnKickIdle

void CPrefsP11::OnUpdateNeedText(CCmdUI* pCmdUI)
  {
CEdit * pEdit = (CEdit *) GetDlgItem (IDC_NOTES);
ASSERT (pEdit);

  pCmdUI->Enable (pEdit->GetWindowTextLength () != 0);
  } // end of CPrefsP11::OnUpdateNeedText

void CPrefsP11::OnEdit() 
{
CEditMultiLine dlg;

  dlg.m_strText = GetText (m_ctlNotes);

  dlg.m_strTitle = "Edit world notes";

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlNotes.SetWindowText (dlg.m_strText);
	
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsP12 property page

IMPLEMENT_DYNCREATE(CPrefsP12, CPropertyPage)

CPrefsP12::CPrefsP12() : CPropertyPage(CPrefsP12::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP12)
	m_keypad_enable = FALSE;
	m_keypad_0 = _T("");
	m_keypad_1 = _T("");
	m_keypad_2 = _T("");
	m_keypad_3 = _T("");
	m_keypad_4 = _T("");
	m_keypad_5 = _T("");
	m_keypad_6 = _T("");
	m_keypad_7 = _T("");
	m_keypad_8 = _T("");
	m_keypad_9 = _T("");
	m_keypad_dash = _T("");
	m_keypad_dot = _T("");
	m_keypad_plus = _T("");
	m_keypad_slash = _T("");
	m_keypad_star = _T("");
	m_bControl = FALSE;
	//}}AFX_DATA_INIT
  m_page_number = 1;
  m_doc = NULL;
  m_bFirstTime = true;

}

CPrefsP12::~CPrefsP12()
{
}

void CPrefsP12::DoDataExchange(CDataExchange* pDX)
{

  // start with normal keys
  if(!pDX->m_bSaveAndValidate)
    {
    m_keypad_0     = m_keypad [eKeypad_0    ];   
    m_keypad_1     = m_keypad [eKeypad_1    ]; 
    m_keypad_2     = m_keypad [eKeypad_2    ];    
    m_keypad_3     = m_keypad [eKeypad_3    ]; 
    m_keypad_4     = m_keypad [eKeypad_4    ];   
    m_keypad_5     = m_keypad [eKeypad_5    ];  
    m_keypad_6     = m_keypad [eKeypad_6    ];   
    m_keypad_7     = m_keypad [eKeypad_7    ]; 
    m_keypad_8     = m_keypad [eKeypad_8    ];    
    m_keypad_9     = m_keypad [eKeypad_9    ]; 
    m_keypad_dot   = m_keypad [eKeypad_Dot  ]; 
    m_keypad_slash = m_keypad [eKeypad_Slash];    
    m_keypad_star  = m_keypad [eKeypad_Star ]; 
    m_keypad_dash  = m_keypad [eKeypad_Dash ]; 
    m_keypad_plus  = m_keypad [eKeypad_Plus ]; 
    }

	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP12)
	DDX_Control(pDX, IDC_CONTROL_KEY, m_ctlControl);
	DDX_Check(pDX, IDC_ENABLE_KEYPAD, m_keypad_enable);
	DDX_Text(pDX, IDC_KEYPAD_0, m_keypad_0);
	DDX_Text(pDX, IDC_KEYPAD_1, m_keypad_1);
	DDX_Text(pDX, IDC_KEYPAD_2, m_keypad_2);
	DDX_Text(pDX, IDC_KEYPAD_3, m_keypad_3);
	DDX_Text(pDX, IDC_KEYPAD_4, m_keypad_4);
	DDX_Text(pDX, IDC_KEYPAD_5, m_keypad_5);
	DDX_Text(pDX, IDC_KEYPAD_6, m_keypad_6);
	DDX_Text(pDX, IDC_KEYPAD_7, m_keypad_7);
	DDX_Text(pDX, IDC_KEYPAD_8, m_keypad_8);
	DDX_Text(pDX, IDC_KEYPAD_9, m_keypad_9);
	DDX_Text(pDX, IDC_KEYPAD_DASH, m_keypad_dash);
	DDX_Text(pDX, IDC_KEYPAD_DOT, m_keypad_dot);
	DDX_Text(pDX, IDC_KEYPAD_PLUS, m_keypad_plus);
	DDX_Text(pDX, IDC_KEYPAD_SLASH, m_keypad_slash);
	DDX_Text(pDX, IDC_KEYPAD_STAR, m_keypad_star);
	DDX_Check(pDX, IDC_CONTROL_KEY, m_bControl);
	//}}AFX_DATA_MAP

  // get them back
  if(pDX->m_bSaveAndValidate)
    {
    if (m_bControl)
      {   // control keys
      m_keypad [eCtrl_Keypad_0    ]  = m_keypad_0    ;   
      m_keypad [eCtrl_Keypad_1    ]  = m_keypad_1    ; 
      m_keypad [eCtrl_Keypad_2    ]  = m_keypad_2    ;    
      m_keypad [eCtrl_Keypad_3    ]  = m_keypad_3    ; 
      m_keypad [eCtrl_Keypad_4    ]  = m_keypad_4    ;   
      m_keypad [eCtrl_Keypad_5    ]  = m_keypad_5    ;  
      m_keypad [eCtrl_Keypad_6    ]  = m_keypad_6    ;   
      m_keypad [eCtrl_Keypad_7    ]  = m_keypad_7    ; 
      m_keypad [eCtrl_Keypad_8    ]  = m_keypad_8    ;    
      m_keypad [eCtrl_Keypad_9    ]  = m_keypad_9    ; 
      m_keypad [eCtrl_Keypad_Dot  ]  = m_keypad_dot  ; 
      m_keypad [eCtrl_Keypad_Slash]  = m_keypad_slash;    
      m_keypad [eCtrl_Keypad_Star ]  = m_keypad_star ; 
      m_keypad [eCtrl_Keypad_Dash ]  = m_keypad_dash ; 
      m_keypad [eCtrl_Keypad_Plus ]  = m_keypad_plus ;
      }
    else
      {   // normal keys
      m_keypad [eKeypad_0    ]  = m_keypad_0    ;   
      m_keypad [eKeypad_1    ]  = m_keypad_1    ; 
      m_keypad [eKeypad_2    ]  = m_keypad_2    ;    
      m_keypad [eKeypad_3    ]  = m_keypad_3    ; 
      m_keypad [eKeypad_4    ]  = m_keypad_4    ;   
      m_keypad [eKeypad_5    ]  = m_keypad_5    ;  
      m_keypad [eKeypad_6    ]  = m_keypad_6    ;   
      m_keypad [eKeypad_7    ]  = m_keypad_7    ; 
      m_keypad [eKeypad_8    ]  = m_keypad_8    ;    
      m_keypad [eKeypad_9    ]  = m_keypad_9    ; 
      m_keypad [eKeypad_Dot  ]  = m_keypad_dot  ; 
      m_keypad [eKeypad_Slash]  = m_keypad_slash;    
      m_keypad [eKeypad_Star ]  = m_keypad_star ; 
      m_keypad [eKeypad_Dash ]  = m_keypad_dash ; 
      m_keypad [eKeypad_Plus ]  = m_keypad_plus ;
      }
    }                         

}


BEGIN_MESSAGE_MAP(CPrefsP12, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP12)
	ON_BN_CLICKED(IDC_CONTROL_KEY, OnControlKey)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CPrefsP12::OnControlKey() 
{

 if (m_ctlControl.GetCheck () == 0)
   {
   // control is not checked, so it must have been before
   if (!m_bFirstTime)
     {
     // save old values
     GetDlgItem (IDC_KEYPAD_0     )->GetWindowText (m_keypad [eCtrl_Keypad_0    ]);
     GetDlgItem (IDC_KEYPAD_1     )->GetWindowText (m_keypad [eCtrl_Keypad_1    ]);
     GetDlgItem (IDC_KEYPAD_2     )->GetWindowText (m_keypad [eCtrl_Keypad_2    ]);
     GetDlgItem (IDC_KEYPAD_3     )->GetWindowText (m_keypad [eCtrl_Keypad_3    ]);
     GetDlgItem (IDC_KEYPAD_4     )->GetWindowText (m_keypad [eCtrl_Keypad_4    ]);
     GetDlgItem (IDC_KEYPAD_5     )->GetWindowText (m_keypad [eCtrl_Keypad_5    ]);
     GetDlgItem (IDC_KEYPAD_6     )->GetWindowText (m_keypad [eCtrl_Keypad_6    ]);
     GetDlgItem (IDC_KEYPAD_7     )->GetWindowText (m_keypad [eCtrl_Keypad_7    ]);
     GetDlgItem (IDC_KEYPAD_8     )->GetWindowText (m_keypad [eCtrl_Keypad_8    ]);
     GetDlgItem (IDC_KEYPAD_9     )->GetWindowText (m_keypad [eCtrl_Keypad_9    ]);
     GetDlgItem (IDC_KEYPAD_DASH  )->GetWindowText (m_keypad [eCtrl_Keypad_Dash ]);
     GetDlgItem (IDC_KEYPAD_DOT   )->GetWindowText (m_keypad [eCtrl_Keypad_Dot  ]);
     GetDlgItem (IDC_KEYPAD_PLUS  )->GetWindowText (m_keypad [eCtrl_Keypad_Plus ]);
     GetDlgItem (IDC_KEYPAD_SLASH )->GetWindowText (m_keypad [eCtrl_Keypad_Slash]);
     GetDlgItem (IDC_KEYPAD_STAR  )->GetWindowText (m_keypad [eCtrl_Keypad_Star ]);
     }    // end of not first time
   GetDlgItem (IDC_KEYPAD_0     )->SetWindowText (m_keypad [eKeypad_0    ]);
   GetDlgItem (IDC_KEYPAD_1     )->SetWindowText (m_keypad [eKeypad_1    ]);
   GetDlgItem (IDC_KEYPAD_2     )->SetWindowText (m_keypad [eKeypad_2    ]);
   GetDlgItem (IDC_KEYPAD_3     )->SetWindowText (m_keypad [eKeypad_3    ]);
   GetDlgItem (IDC_KEYPAD_4     )->SetWindowText (m_keypad [eKeypad_4    ]);
   GetDlgItem (IDC_KEYPAD_5     )->SetWindowText (m_keypad [eKeypad_5    ]);
   GetDlgItem (IDC_KEYPAD_6     )->SetWindowText (m_keypad [eKeypad_6    ]);
   GetDlgItem (IDC_KEYPAD_7     )->SetWindowText (m_keypad [eKeypad_7    ]);
   GetDlgItem (IDC_KEYPAD_8     )->SetWindowText (m_keypad [eKeypad_8    ]);
   GetDlgItem (IDC_KEYPAD_9     )->SetWindowText (m_keypad [eKeypad_9    ]);
   GetDlgItem (IDC_KEYPAD_DASH  )->SetWindowText (m_keypad [eKeypad_Dash ]);
   GetDlgItem (IDC_KEYPAD_DOT   )->SetWindowText (m_keypad [eKeypad_Dot  ]);
   GetDlgItem (IDC_KEYPAD_PLUS  )->SetWindowText (m_keypad [eKeypad_Plus ]);
   GetDlgItem (IDC_KEYPAD_SLASH )->SetWindowText (m_keypad [eKeypad_Slash]);
   GetDlgItem (IDC_KEYPAD_STAR  )->SetWindowText (m_keypad [eKeypad_Star ]);

   }   // end of control not checked
 else
   {
   // control is checked, so it must not have been before
   if (!m_bFirstTime)
     {
     // save old values
     GetDlgItem (IDC_KEYPAD_0     )->GetWindowText (m_keypad [eKeypad_0    ]);
     GetDlgItem (IDC_KEYPAD_1     )->GetWindowText (m_keypad [eKeypad_1    ]);
     GetDlgItem (IDC_KEYPAD_2     )->GetWindowText (m_keypad [eKeypad_2    ]);
     GetDlgItem (IDC_KEYPAD_3     )->GetWindowText (m_keypad [eKeypad_3    ]);
     GetDlgItem (IDC_KEYPAD_4     )->GetWindowText (m_keypad [eKeypad_4    ]);
     GetDlgItem (IDC_KEYPAD_5     )->GetWindowText (m_keypad [eKeypad_5    ]);
     GetDlgItem (IDC_KEYPAD_6     )->GetWindowText (m_keypad [eKeypad_6    ]);
     GetDlgItem (IDC_KEYPAD_7     )->GetWindowText (m_keypad [eKeypad_7    ]);
     GetDlgItem (IDC_KEYPAD_8     )->GetWindowText (m_keypad [eKeypad_8    ]);
     GetDlgItem (IDC_KEYPAD_9     )->GetWindowText (m_keypad [eKeypad_9    ]);
     GetDlgItem (IDC_KEYPAD_DASH  )->GetWindowText (m_keypad [eKeypad_Dash ]);
     GetDlgItem (IDC_KEYPAD_DOT   )->GetWindowText (m_keypad [eKeypad_Dot  ]);
     GetDlgItem (IDC_KEYPAD_PLUS  )->GetWindowText (m_keypad [eKeypad_Plus ]);
     GetDlgItem (IDC_KEYPAD_SLASH )->GetWindowText (m_keypad [eKeypad_Slash]);
     GetDlgItem (IDC_KEYPAD_STAR  )->GetWindowText (m_keypad [eKeypad_Star ]);
     }    // end of not first time
   GetDlgItem (IDC_KEYPAD_0     )->SetWindowText (m_keypad [eCtrl_Keypad_0    ]);
   GetDlgItem (IDC_KEYPAD_1     )->SetWindowText (m_keypad [eCtrl_Keypad_1    ]);
   GetDlgItem (IDC_KEYPAD_2     )->SetWindowText (m_keypad [eCtrl_Keypad_2    ]);
   GetDlgItem (IDC_KEYPAD_3     )->SetWindowText (m_keypad [eCtrl_Keypad_3    ]);
   GetDlgItem (IDC_KEYPAD_4     )->SetWindowText (m_keypad [eCtrl_Keypad_4    ]);
   GetDlgItem (IDC_KEYPAD_5     )->SetWindowText (m_keypad [eCtrl_Keypad_5    ]);
   GetDlgItem (IDC_KEYPAD_6     )->SetWindowText (m_keypad [eCtrl_Keypad_6    ]);
   GetDlgItem (IDC_KEYPAD_7     )->SetWindowText (m_keypad [eCtrl_Keypad_7    ]);
   GetDlgItem (IDC_KEYPAD_8     )->SetWindowText (m_keypad [eCtrl_Keypad_8    ]);
   GetDlgItem (IDC_KEYPAD_9     )->SetWindowText (m_keypad [eCtrl_Keypad_9    ]);
   GetDlgItem (IDC_KEYPAD_DASH  )->SetWindowText (m_keypad [eCtrl_Keypad_Dash ]);
   GetDlgItem (IDC_KEYPAD_DOT   )->SetWindowText (m_keypad [eCtrl_Keypad_Dot  ]);
   GetDlgItem (IDC_KEYPAD_PLUS  )->SetWindowText (m_keypad [eCtrl_Keypad_Plus ]);
   GetDlgItem (IDC_KEYPAD_SLASH )->SetWindowText (m_keypad [eCtrl_Keypad_Slash]);
   GetDlgItem (IDC_KEYPAD_STAR  )->SetWindowText (m_keypad [eCtrl_Keypad_Star ]);

   }

 m_bFirstTime = false;

}


/////////////////////////////////////////////////////////////////////////////
// CPrefsP12 message handlers


/////////////////////////////////////////////////////////////////////////////
// CPrefsP13 property page

IMPLEMENT_DYNCREATE(CPrefsP13, CPropertyPage)

CPrefsP13::CPrefsP13() : CPropertyPage(CPrefsP13::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP13)
	m_paste_postamble = _T("");
	m_paste_preamble = _T("");
	m_pasteline_postamble = _T("");
	m_pasteline_preamble = _T("");
	m_bConfirmOnPaste = FALSE;
	m_bCommentedSoftcode = FALSE;
	m_iLineDelay = 0;
	m_bEcho = FALSE;
	m_nPasteDelayPerLines = 0;
	//}}AFX_DATA_INIT
  m_page_number = 1;
  m_doc = NULL;

}

CPrefsP13::~CPrefsP13()
{
}

void CPrefsP13::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP13)
	DDX_Text(pDX, IDC_PASTE_POSTAMBLE, m_paste_postamble);
	DDX_Text(pDX, IDC_PASTE_PREAMBLE, m_paste_preamble);
	DDX_Text(pDX, IDC_PASTELINE_POSTAMBLE, m_pasteline_postamble);
	DDX_Text(pDX, IDC_PASTELINE_PREAMBLE, m_pasteline_preamble);
	DDX_Check(pDX, IDC_CONFIRM_ON_PASTE, m_bConfirmOnPaste);
	DDX_Check(pDX, IDC_COMMENTED_SOFTCODE, m_bCommentedSoftcode);
	DDX_Text(pDX, IDC_LINE_DELAY, m_iLineDelay);
	DDV_MinMaxLong(pDX, m_iLineDelay, 0, 10000);
	DDX_Check(pDX, IDC_ECHO, m_bEcho);
	DDX_Text(pDX, IDC_LINE_DELAY_PER_LINES, m_nPasteDelayPerLines);
	DDV_MinMaxLong(pDX, m_nPasteDelayPerLines, 1, 100000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsP13, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP13)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsP13 message handlers


/////////////////////////////////////////////////////////////////////////////
// CPrefsP14 property page

IMPLEMENT_DYNCREATE(CPrefsP14, CPropertyPage)

CPrefsP14::CPrefsP14() : CPropertyPage(CPrefsP14::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP14)
	m_enable_beeps = FALSE;
	m_font_name = _T("");
	m_indent_paras = FALSE;
	m_wrap_output = FALSE;
	m_nLines = 0;
	m_nWrapColumn = 0;
	m_bLineInformation = FALSE;
	m_bStartPaused = FALSE;
	m_bShowBold = FALSE;
	m_iPixelOffset = 0;
	m_bAutoFreeze = FALSE;
	m_bDisableCompression = FALSE;
	m_bFlashIcon = FALSE;
	m_bShowItalic = FALSE;
	m_bShowUnderline = FALSE;
	m_bUseDefaultOutputFont = FALSE;
	m_strOutputStyle = _T("");
	m_bUnpauseOnSend = FALSE;
	m_bAlternativeInverse = FALSE;
	m_bAutoWrapWindowWidth = FALSE;
	m_strBeepSound = _T("");
	m_bNAWS = FALSE;
	m_strTerminalIdentification = _T("");
	m_bShowConnectDisconnect = FALSE;
	m_bCopySelectionToClipboard = FALSE;
	m_bAutoCopyInHTML = FALSE;
	m_iLineSpacing = 0;
	m_bUTF_8 = FALSE;
	m_bCarriageReturnClearsLine = FALSE;
	m_sound_pathname = _T("");
	m_bConvertGAtoNewline = FALSE;
	//}}AFX_DATA_INIT
  m_page_number = 1;
  m_doc = NULL;

}

CPrefsP14::~CPrefsP14()
{
}

void CPrefsP14::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP14)
	DDX_Control(pDX, IDC_COPY_SELECTION_TO_CLIPBOARD, m_ctlCopySelectionToClipboard);
	DDX_Control(pDX, IDC_OUTPUT_STYLE, m_ctlOutputStyle);
	DDX_Control(pDX, IDC_USE_DEFAULT_OUTPUT_FONT, m_ctlUseDefaultOutputFont);
	DDX_Control(pDX, IDC_WRAP_COLUMN, m_ctlWrapColumn);
	DDX_Control(pDX, IDC_FONT_NAME, m_ctlFontName);
	DDX_Check(pDX, IDC_ENABLE_BEEPS, m_enable_beeps);
	DDX_Text(pDX, IDC_FONT_NAME, m_font_name);
	DDX_Check(pDX, IDC_INDENT_PARAS, m_indent_paras);
	DDX_Check(pDX, IDC_WRAP_OUTPUT, m_wrap_output);
	DDX_Text(pDX, IDC_LINES, m_nLines);
	DDV_MinMaxLong(pDX, m_nLines, 200, 500000);
	DDX_Text(pDX, IDC_WRAP_COLUMN, m_nWrapColumn);
	DDV_MinMaxUInt(pDX, m_nWrapColumn, 20, MAX_LINE_WIDTH);
	DDX_Check(pDX, IDC_LINE_INFORMATION, m_bLineInformation);
	DDX_Check(pDX, IDC_START_PAUSED, m_bStartPaused);
	DDX_Check(pDX, IDC_SHOW_BOLD, m_bShowBold);
	DDX_Text(pDX, IDC_PIXEL_OFFSET, m_iPixelOffset);
	DDV_MinMaxInt(pDX, m_iPixelOffset, 0, 20);
	DDX_Check(pDX, IDC_AUTOFREEZE, m_bAutoFreeze);
	DDX_Check(pDX, IDC_DISABLE_COMPRESSION, m_bDisableCompression);
	DDX_Check(pDX, IDC_FLASH_ICON, m_bFlashIcon);
	DDX_Check(pDX, IDC_SHOW_ITALIC, m_bShowItalic);
	DDX_Check(pDX, IDC_SHOW_UNDERLINE, m_bShowUnderline);
	DDX_Check(pDX, IDC_USE_DEFAULT_OUTPUT_FONT, m_bUseDefaultOutputFont);
	DDX_Text(pDX, IDC_OUTPUT_STYLE, m_strOutputStyle);
	DDX_Check(pDX, IDC_UNPAUSEONSEND, m_bUnpauseOnSend);
	DDX_Check(pDX, IDC_ALTERNATIVE_INVERSE, m_bAlternativeInverse);
	DDX_Check(pDX, IDC_AUTO_WRAP_WINDOW, m_bAutoWrapWindowWidth);
	DDX_Text(pDX, IDC_SOUND_PATH, m_strBeepSound);
	DDX_Check(pDX, IDC_NAWS, m_bNAWS);
	DDX_Text(pDX, IDC_TERMINAL_TYPE, m_strTerminalIdentification);
	DDV_MaxChars(pDX, m_strTerminalIdentification, 20);
	DDX_Check(pDX, IDC_SHOW_CONNECT_DISCONNECT, m_bShowConnectDisconnect);
	DDX_Check(pDX, IDC_COPY_SELECTION_TO_CLIPBOARD, m_bCopySelectionToClipboard);
	DDX_Check(pDX, IDC_AUTO_COPY_HTML, m_bAutoCopyInHTML);
	DDX_Text(pDX, IDC_LINE_SPACING, m_iLineSpacing);
	DDV_MinMaxInt(pDX, m_iLineSpacing, 0, 100);
	DDX_Check(pDX, IDC_UTF_8, m_bUTF_8);
	DDX_Check(pDX, IDC_CARRIAGE_RETURN_CLEARS_LINE, m_bCarriageReturnClearsLine);
	DDX_Text(pDX, IDC_SOUND_PATH2, m_sound_pathname);
	DDX_Check(pDX, IDC_CONVERT_GA_TO_NEWLINE, m_bConvertGAtoNewline);
	//}}AFX_DATA_MAP

   if(pDX->m_bSaveAndValidate)
    {

    // warn them if they are allocating too many lines for their physical memory
    if (m_nOldLines != m_nLines && m_nLines > 1000)
      {
	    // Find available memory
	    MEMORYSTATUS MemStat;
	    MemStat.dwLength = sizeof(MEMORYSTATUS);
	    GlobalMemoryStatus(&MemStat);

      DWORD nBytesNeeded = 16L * 1024L * 1024L;    // start with 16 Mb for OS
      nBytesNeeded += m_nLines * 60L;             // plus 60 bytes per line

      if (nBytesNeeded > MemStat.dwTotalPhys)
        {
        CString strMsg;

        strMsg = TFormat ("You are allocating %ld lines for your output buffer, but have "
          "only %ld Mb of physical RAM. This is not recommended. Do you wish to "
          "continue anyway?", m_nLines, MemStat.dwTotalPhys / 1024L / 1024L); 

        if (::UMessageBox (strMsg, MB_ICONQUESTION | MB_YESNO) == IDNO)
          pDX->Fail ();

        } // end of not having enough memory

      }   // end of changing the amount of memory

     }  // end of saving and validating
}


BEGIN_MESSAGE_MAP(CPrefsP14, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP14)
	ON_BN_CLICKED(IDC_CHOOSE_FONT, OnChooseFont)
	ON_BN_CLICKED(IDC_ADJUST_WIDTH, OnAdjustWidth)
	ON_BN_CLICKED(IDC_BROWSE_SOUNDS, OnBrowseSounds)
	ON_BN_CLICKED(IDC_TEST_SOUND, OnTestSound)
	ON_BN_CLICKED(IDC_ADJUST_TO_WIDTH, OnAdjustToWidth)
	ON_BN_CLICKED(IDC_BROWSE_SOUNDS2, OnBrowseSounds2)
	ON_BN_CLICKED(IDC_TEST_SOUND2, OnTestSound2)
	ON_BN_CLICKED(IDC_NO_SOUND, OnNoSound)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_CHOOSE_FONT, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_USE_DEFAULT_OUTPUT_FONT, OnUpdateHaveDefaults)
  ON_UPDATE_COMMAND_UI(IDC_TEST_SOUND, OnUpdateNeedSound)
  ON_UPDATE_COMMAND_UI(IDC_AUTO_COPY_HTML, OnUpdateNeedCopyToClipboard)
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_TEST_SOUND2, OnUpdateNeedActivitySound)
  ON_UPDATE_COMMAND_UI(IDC_NO_SOUND, OnUpdateNeedActivitySound)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsP14 message handlers

BOOL CPrefsP14::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	((CSpinButtonCtrl*) GetDlgItem(IDC_SPIN_WRAP_COLUMN))->SetRange(20, MAX_LINE_WIDTH);
	
	((CSpinButtonCtrl*) GetDlgItem(IDC_SPIN_OFFSET))->SetRange(0, 20);
	((CSpinButtonCtrl*) GetDlgItem(IDC_SPIN_LINE_SPACING))->SetRange(0, 100);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPrefsP14::OnChooseFont() 
{
CDC dc;

dc.CreateCompatibleDC (NULL);

LOGFONT lf;

ZeroMemory (&lf, sizeof lf);

lf.lfHeight = -MulDiv(m_font_height, dc.GetDeviceCaps(LOGPIXELSY), 72);
strncpy (lf.lfFaceName, m_font_name, sizeof (lf.lfFaceName) - 1);
lf.lfFaceName [sizeof (lf.lfFaceName) - 1] = 0;
lf.lfWeight = m_font_weight;
lf.lfCharSet = m_font_charset;
lf.lfItalic = FALSE;

CFontDialog dlg (&lf,
                 CF_SCREENFONTS,
                 NULL,
                 GetOwner ());       // owner view

if (dlg.DoModal () != IDOK)
  return;

m_font_height = dlg.GetSize () / 10;
m_font_name   = dlg.GetFaceName (); 
m_font_weight = dlg.GetWeight (); 
m_font_charset = lf.lfCharSet;

m_ctlFontName.SetWindowText(m_font_name);

m_ctlOutputStyle.SetWindowText(CFormat ("%i pt.",m_font_height));

}


void CPrefsP14::OnAdjustWidth() 
{

// this is for the guy that wants to fit the max text he can in his window,
// after resizing it

if (m_doc->m_pActiveCommandView || m_doc->m_pActiveOutputView)
  {

  CDC dc;

  dc.CreateCompatibleDC (NULL);

  int lfHeight = -MulDiv(m_font_height, dc.GetDeviceCaps(LOGPIXELSY), 72);
  CFont font;

   font.CreateFont(lfHeight, // int nHeight, 
				  0, // int nWidth, 
				  0, // int nEscapement, 
				  0, // int nOrientation, 
				  m_font_weight, // int nWeight, 
				  0, // BYTE bItalic, 
				  0, // BYTE bUnderline, 
          0, // BYTE cStrikeOut, 
          m_font_charset, // BYTE nCharSet, 
          0, // BYTE nOutPrecision, 
          0, // BYTE nClipPrecision, 
          0, // BYTE nQuality, 
          MUSHCLIENT_FONT_FAMILY, // BYTE nPitchAndFamily,     // was  FF_DONTCARE
          m_font_name);// LPCTSTR lpszFacename );

    // Get the metrics of the font.

    dc.SelectObject(font);

    TEXTMETRIC tm;
    dc.GetTextMetrics(&tm);

  RECT rect;

  if (m_doc->m_pActiveCommandView)
    m_doc->m_pActiveCommandView->GetClientRect (&rect);
  else
    m_doc->m_pActiveOutputView->GetClientRect (&rect);

  int iWidth = (rect.right - rect.left - m_iPixelOffset) / tm.tmAveCharWidth;

  // ensure in range that we allow
  if (iWidth < 20)
    iWidth = 20;
  if (iWidth > MAX_LINE_WIDTH)
    iWidth = MAX_LINE_WIDTH;

  // put in the new figure
  m_ctlWrapColumn.SetWindowText (CFormat ("%i", iWidth));

  }
}

LRESULT CPrefsP14::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CPrefsP14::OnKickIdle

void CPrefsP14::OnUpdateNotUsingDefaults(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlUseDefaultOutputFont.GetCheck () == 0 ||
                  App.m_strDefaultOutputFont.IsEmpty ());
  } // end of CPrefsP14::OnUpdateNotUsingDefaults

void CPrefsP14::OnUpdateHaveDefaults(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (!App.m_strDefaultOutputFont.IsEmpty ());
  } // end of CPrefsP14::OnUpdateHaveDefaults

void CPrefsP14::OnUpdateNeedCopyToClipboard(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlCopySelectionToClipboard.GetCheck () != 0 );
  } // end of CPrefsP14::OnUpdateNeedCopyToClipboard


void CPrefsP14::OnBrowseSounds() 
{
CString filename;

  CFileDialog filedlg (TRUE,   // loading the file
                       ".wav",    // default extension
                       m_strBeepSound,  // suggested name
                       OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
                       "Waveaudio files (*.wav)|*.wav|"
                       "MIDI files (*.mid)|*.mid|"
                       "Sequencer files (*.rmi)|*.rmi|"
                       "|",    // filter 
                       this);  // parent window

  filedlg.m_ofn.lpstrTitle = "Select sound to play";
  filedlg.m_ofn.lpstrFile = filename.GetBuffer (_MAX_PATH); // needed!! (for Win32s)  

  strcpy (filedlg.m_ofn.lpstrFile, m_strBeepSound);
    
  ChangeToFileBrowsingDirectory ();
  int nResult = filedlg.DoModal();
  ChangeToStartupDirectory ();

  if (nResult != IDOK)
    return;    // cancelled dialog

  m_strBeepSound = filedlg.GetPathName ();

  GetDlgItem (IDC_SOUND_PATH)->SetWindowText(m_strBeepSound);

  GetDlgItem (IDC_SOUND_PATH)->EnableWindow (TRUE);
	
}

void CPrefsP14::OnTestSound() 
{
  if (!m_strBeepSound.IsEmpty ())
    m_doc->PlaySoundFile (m_strBeepSound);
	
}

void CPrefsP14::OnUpdateNeedSound(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (!m_strBeepSound.IsEmpty ());
  } // end of CPrefsP14::OnUpdateNeedSound

void CPrefsP14::OnUpdateNeedActivitySound(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (!m_sound_pathname.IsEmpty () && m_sound_pathname != NOSOUNDLIT);
  } // end of CPrefsP14::OnUpdateNeedActivitySound

/////////////////////////////////////////////////////////////////////////////
// CPrefsP15 property page

IMPLEMENT_DYNCREATE(CPrefsP15, CPropertyPage)

CPrefsP15::CPrefsP15() : CPropertyPage(CPrefsP15::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP15)
	m_strBufferLines = _T("");
	m_strConnectionDuration = _T("");
	m_strConnectionTime = _T("");
	m_strAliases = _T("");
	m_strTriggers = _T("");
	m_strCompressionRatio = _T("");
	m_strBytesSent = _T("");
	m_strBytesReceived = _T("");
	m_strTimers = _T("");
	m_strTriggerTimeTaken = _T("");
	m_strIpAddress = _T("");
	m_strMXPinbuiltElements = _T("");
	m_strMXPinbuiltEntities = _T("");
	m_strMXPentitiesReceived = _T("");
	m_strMXPerrors = _T("");
	m_strMXPmudElements = _T("");
	m_strMXPmudEntities = _T("");
	m_strMXPtagsReceived = _T("");
	m_strMXPunclosedTags = _T("");
	m_strCompressedIn = _T("");
	m_strCompressedOut = _T("");
	m_strTimeTakenCompressing = _T("");
	m_strMXPactionsCached = _T("");
	m_strMXPreferenceCount = _T("");
	//}}AFX_DATA_INIT
  m_page_number = 1;
  m_doc = NULL;

}

CPrefsP15::~CPrefsP15()
{
}

void CPrefsP15::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP15)
	DDX_Text(pDX, IDC_BUFFER_LINES, m_strBufferLines);
	DDX_Text(pDX, IDC_CONNECTION_DURATION, m_strConnectionDuration);
	DDX_Text(pDX, IDC_CONNECTION_TIME, m_strConnectionTime);
	DDX_Text(pDX, IDC_ALIASES, m_strAliases);
	DDX_Text(pDX, IDC_TRIGGERS, m_strTriggers);
	DDX_Text(pDX, IDC_COMPRESSION_RATIO, m_strCompressionRatio);
	DDX_Text(pDX, IDC_BYTES_SENT, m_strBytesSent);
	DDX_Text(pDX, IDC_BYTES_RECEIVED, m_strBytesReceived);
	DDX_Text(pDX, IDC_TIMERS, m_strTimers);
	DDX_Text(pDX, IDC_TIME_TAKEN, m_strTriggerTimeTaken);
	DDX_Text(pDX, IDC_IP_ADDRESS, m_strIpAddress);
	DDX_Text(pDX, IDC_MXP_BUILTIN_ELEMENTS, m_strMXPinbuiltElements);
	DDX_Text(pDX, IDC_MXP_BUILTIN_ENTITIES, m_strMXPinbuiltEntities);
	DDX_Text(pDX, IDC_MXP_ENTITIES_RECEIVED, m_strMXPentitiesReceived);
	DDX_Text(pDX, IDC_MXP_ERRORS, m_strMXPerrors);
	DDX_Text(pDX, IDC_MXP_MUD_ELEMENTS, m_strMXPmudElements);
	DDX_Text(pDX, IDC_MXP_MUD_ENTITIES, m_strMXPmudEntities);
	DDX_Text(pDX, IDC_MXP_TAGS_RECEIVED, m_strMXPtagsReceived);
	DDX_Text(pDX, IDC_MXP_UNCLOSED_TAGS, m_strMXPunclosedTags);
	DDX_Text(pDX, IDC_COMPRESSED_IN, m_strCompressedIn);
	DDX_Text(pDX, IDC_COMPRESSED_OUT, m_strCompressedOut);
	DDX_Text(pDX, IDC_TIME_TAKEN_COMPRESSING, m_strTimeTakenCompressing);
	DDX_Text(pDX, IDC_MXP_ACTIONS_CACHED, m_strMXPactionsCached);
	DDX_Text(pDX, IDC_MXP_REFERENCE_COUNT, m_strMXPreferenceCount);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsP15, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP15)
	ON_BN_CLICKED(IDC_CALCULATE_MEMORY, OnCalculateMemory)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsP15 message handlers

BOOL CPrefsP15::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
  // do it now if quick
  if (m_doc->m_LineList.GetCount () <= 1000)
    CalculateMemoryUsage ();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


/////////////////////////////////////////////////////////////////////////////
// CPrefsP16 property page

IMPLEMENT_DYNCREATE(CPrefsP16, CGenPropertyPage)

CPrefsP16::CPrefsP16() : CGenPropertyPage(CPrefsP16::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP16)
	m_bEnableTimers = FALSE;
	m_bUseDefaultTimers = FALSE;
	//}}AFX_DATA_INIT
  m_doc = NULL;

  }


CPrefsP16::~CPrefsP16()
{
}

void CPrefsP16::DoDataExchange(CDataExchange* pDX)
{

  // check they really want to use defaults
  if (pDX->m_bSaveAndValidate)
    if (m_ctlUseDefaultTimers.GetCheck ())
      if (m_ctlUseDefaultTimers.GetCheck () != m_bUseDefaultTimers)
        if (m_doc->m_TimerMap.GetCount () > 0)
         if (::UMessageBox (TFormat ("By checking the option \"Override with default timers\" "
             " your existing %i timer%s will be PERMANENTLY discarded next time you open this world.\n\n"
             "Are you SURE you want to do this?", PLURAL (m_doc->m_TimerMap.GetCount ())),
             MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 ) != IDYES)
             pDX->Fail ();

	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP16)
	DDX_Control(pDX, IDC_COUNT, m_ctlSummary);
	DDX_Control(pDX, IDC_FILTER, m_ctlFilter);
	DDX_Control(pDX, IDC_USE_DEFAULT_TIMERS, m_ctlUseDefaultTimers);
	DDX_Control(pDX, IDC_TIMERS_LIST, m_ctlTimerList);
	DDX_Check(pDX, IDC_ENABLE_TIMERS, m_bEnableTimers);
	DDX_Check(pDX, IDC_USE_DEFAULT_TIMERS, m_bUseDefaultTimers);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsP16, CGenPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP16)
	ON_BN_CLICKED(IDC_ADD_TIMER, OnAddTimer)
	ON_BN_CLICKED(IDC_CHANGE_TIMER, OnChangeTimer)
	ON_BN_CLICKED(IDC_DELETE_TIMER, OnDeleteTimer)
	ON_BN_CLICKED(IDC_LOAD_TIMERS, OnLoadTimers)
	ON_BN_CLICKED(IDC_RESET_TIMERS, OnResetTimers)
	ON_BN_CLICKED(IDC_SAVE_TIMERS, OnSaveTimers)
	ON_NOTIFY(NM_DBLCLK, IDC_TIMERS_LIST, OnDblclkTimersList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_TIMERS_LIST, OnColumnclickTimersList)
	ON_BN_CLICKED(IDC_ENABLE_TIMERS, OnEnableTimers)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	ON_BN_CLICKED(IDC_PASTE, OnPaste)
	ON_BN_CLICKED(IDC_FILTER, OnFilter)
	ON_BN_CLICKED(IDC_EDIT_FILTER, OnEditFilter)
	//}}AFX_MSG_MAP
  ON_BN_CLICKED(IDC_FIND, OnFind)
	ON_BN_CLICKED(IDC_FIND_NEXT, OnFindNext)
  ON_UPDATE_COMMAND_UI(IDC_COPY, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_PASTE, OnUpdateNeedXMLClipboard)
  ON_UPDATE_COMMAND_UI(IDC_CHANGE_TIMER, OnUpdateNeedOneSelection)
  ON_UPDATE_COMMAND_UI(IDC_DELETE_TIMER, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_SAVE_TIMERS, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_RESET_TIMERS, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_FIND, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_FIND_NEXT, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_LOAD_TIMERS, OnUpdateNotUsingDefaults)
  ON_UPDATE_COMMAND_UI(IDC_USE_DEFAULT_TIMERS, OnUpdateHaveDefaults)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsP16 message handlers

void CPrefsP16::InitDialog (CDialog * pDlg)
  {
  CTimerDlg * dlg = (CTimerDlg *) pDlg;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CTimerDlg ) ) );

// default timer stuff here

  dlg->m_iType = CTimer::eInterval;
  dlg->m_bEnabled = TRUE;

  dlg->m_pTimerMap = &m_doc->m_TimerMap;
  dlg->m_current_timer = NULL;
  dlg->m_iSendTo = m_doc->m_iDefaultTimerSendTo;

  }  // end of CPrefsP16::InitDialog

void CPrefsP16::LoadDialog (CDialog * pDlg, CObject * pItem)
  {
  CTimerDlg * dlg = (CTimerDlg *) pDlg;
  CTimer * timer_item = (CTimer *) pItem;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CTimerDlg ) ) );
  ASSERT_VALID (timer_item);
  ASSERT( timer_item->IsKindOf( RUNTIME_CLASS( CTimer ) ) );

  dlg->m_strLabel        = timer_item->strLabel; 
  dlg->m_strProcedure    = timer_item->strProcedure; 
  dlg->m_bEnabled        = timer_item->bEnabled; 
  dlg->m_bOneShot        = timer_item->bOneShot; 
  dlg->m_bOmitFromOutput = timer_item->bOmitFromOutput; 
  dlg->m_bOmitFromLog    = timer_item->bOmitFromLog; 
  dlg->m_iSendTo         = timer_item->iSendTo; 
  dlg->m_bActiveWhenClosed = timer_item->bActiveWhenClosed; 
  dlg->m_iType           = timer_item->iType; 
  dlg->m_strContents     = timer_item->strContents; 

  dlg->m_iAtHour         = timer_item->iAtHour; 
  dlg->m_iAtMinute       = timer_item->iAtMinute; 
  dlg->m_fAtSecond       = timer_item->fAtSecond; 
                        
  dlg->m_iEveryHour      = timer_item->iEveryHour; 
  dlg->m_iEveryMinute    = timer_item->iEveryMinute; 
  dlg->m_fEverySecond    = timer_item->fEverySecond; 
  
  dlg->m_iOffsetHour     = timer_item->iOffsetHour; 
  dlg->m_iOffsetMinute   = timer_item->iOffsetMinute; 
  dlg->m_fOffsetSecond   = timer_item->fOffsetSecond; 

  dlg->m_bTemporary      = timer_item->bTemporary; 
  dlg->m_strGroup        = timer_item->strGroup; 
  dlg->m_strVariable     = timer_item->strVariable; 

  dlg->m_pTimerMap = &m_doc->m_TimerMap;
  dlg->m_current_timer = timer_item;

  }   // end of CPrefsP16::LoadDialog


void CPrefsP16::UnloadDialog (CDialog * pDlg, CObject * pItem)
  {
  CTimerDlg * dlg = (CTimerDlg *) pDlg;
  CTimer * timer_item = (CTimer *) pItem;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CTimerDlg ) ) );
  ASSERT_VALID (timer_item);
  ASSERT( timer_item->IsKindOf( RUNTIME_CLASS( CTimer ) ) );

// move from dialog to timer_item

  timer_item->strLabel        = dlg->m_strLabel;
  timer_item->strProcedure    = dlg->m_strProcedure;
  timer_item->bEnabled        = dlg->m_bEnabled;
  timer_item->bOneShot        = dlg->m_bOneShot; 
  timer_item->bOmitFromOutput = dlg->m_bOmitFromOutput; 
  timer_item->bOmitFromLog    = dlg->m_bOmitFromLog; 
  timer_item->iSendTo           = dlg->m_iSendTo; 
  timer_item->bActiveWhenClosed = dlg->m_bActiveWhenClosed; 
  timer_item->iType           = dlg->m_iType;
  timer_item->strContents     = dlg->m_strContents;

  timer_item->iAtHour         = dlg->m_iAtHour;
  timer_item->iAtMinute       = dlg->m_iAtMinute;
  timer_item->fAtSecond       = dlg->m_fAtSecond;
  
  timer_item->iEveryHour      = dlg->m_iEveryHour;
  timer_item->iEveryMinute    = dlg->m_iEveryMinute;
  timer_item->fEverySecond    = dlg->m_fEverySecond;
  
  timer_item->iOffsetHour     = dlg->m_iOffsetHour;
  timer_item->iOffsetMinute   = dlg->m_iOffsetMinute;
  timer_item->fOffsetSecond   = dlg->m_fOffsetSecond;

  timer_item->bTemporary      = dlg->m_bTemporary; 
  timer_item->strGroup        = dlg->m_strGroup; 
  timer_item->strVariable     = dlg->m_strVariable; 
  
  m_doc->ResetOneTimer (timer_item);

  }   // end of CPrefsP16::UnloadDialog


CString CPrefsP16::GetObjectName (CDialog * pDlg) const
  {
  CTimerDlg * dlg = (CTimerDlg *) pDlg;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CTimerDlg ) ) );
        
  return dlg->m_strLabel;
  }    // end of CPrefsP16::GetObjectName

CString CPrefsP16::GetScriptName (CObject * pItem) const
  {
  CTimer * timer_item = (CTimer *) pItem;

  ASSERT_VALID (timer_item);
  ASSERT( timer_item->IsKindOf( RUNTIME_CLASS( CTimer ) ) );

  return timer_item->strProcedure;

  } // end of CPrefsP16::GetScriptName 


CString CPrefsP16::GetLabel (CObject * pItem) const
  {
  CTimer * timer_item = (CTimer *) pItem;

  ASSERT_VALID (timer_item);
  ASSERT( timer_item->IsKindOf( RUNTIME_CLASS( CTimer ) ) );

  return timer_item->strLabel;

  } // end of CPrefsP16::GetLabel 
 
void CPrefsP16::SetDispatchID (CObject * pItem, const DISPID dispid)
  {
  CTimer * timer_item = (CTimer *) pItem;

  ASSERT_VALID (timer_item);
  ASSERT( timer_item->IsKindOf( RUNTIME_CLASS( CTimer ) ) );

  timer_item->dispid = dispid;

  } // end of CPrefsP16::SetDispatchID


void CPrefsP16::SetInternalName (CObject * pItem, const CString strName)
  {
  // timers don't have internal names
  } // end of CPrefsP16::SetInternalName

CObject * CPrefsP16::MakeNewObject (void)
  {
  return (CObject *) new CTimer;
  }    // end of CPrefsP16::MakeNewObject

__int64 CPrefsP16::GetModificationNumber (CObject * pItem) const
  {
  CTimer * timer_item = (CTimer *) pItem;

  ASSERT_VALID (timer_item);
  ASSERT( timer_item->IsKindOf( RUNTIME_CLASS( CTimer ) ) );

  return timer_item->nUpdateNumber;

  }   // end of CPrefsP16::GetModificationNumber

void CPrefsP16::SetModificationNumber (CObject * pItem, __int64 nUpdateNumber)
  {
  CTimer * timer_item = (CTimer *) pItem;

  ASSERT_VALID (timer_item);
  ASSERT( timer_item->IsKindOf( RUNTIME_CLASS( CTimer ) ) );
  
  timer_item->nUpdateNumber = nUpdateNumber;
  }    // end of CPrefsP16::SetModificationNumber


bool CPrefsP16::CheckIfChanged (CDialog * pDlg, CObject * pItem) const
  {
  CTimerDlg * dlg = (CTimerDlg *) pDlg;
  CTimer * timer_item = (CTimer *) pItem;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CTimerDlg ) ) );
  ASSERT_VALID (timer_item);
  ASSERT( timer_item->IsKindOf( RUNTIME_CLASS( CTimer ) ) );

  if (timer_item->bEnabled        == dlg->m_bEnabled &&
      timer_item->bOneShot        == dlg->m_bOneShot &&
      timer_item->bOmitFromOutput == dlg->m_bOmitFromOutput &&
      timer_item->bOmitFromLog    == dlg->m_bOmitFromLog &&
      timer_item->iSendTo         == dlg->m_iSendTo &&
      timer_item->bActiveWhenClosed == dlg->m_bActiveWhenClosed &&
      timer_item->strLabel        == dlg->m_strLabel &&
      timer_item->strProcedure    == dlg->m_strProcedure &&
      timer_item->iType           == dlg->m_iType &&
      timer_item->strContents     == dlg->m_strContents &&
      timer_item->iAtHour         == dlg->m_iAtHour &&
      timer_item->iAtMinute       == dlg->m_iAtMinute &&
      timer_item->fAtSecond       == dlg->m_fAtSecond &&
      timer_item->iEveryHour      == dlg->m_iEveryHour &&
      timer_item->iEveryMinute    == dlg->m_iEveryMinute &&
      timer_item->fEverySecond    == dlg->m_fEverySecond &&
      timer_item->iOffsetHour     == dlg->m_iOffsetHour &&
      timer_item->iOffsetMinute   == dlg->m_iOffsetMinute &&
      timer_item->fOffsetSecond   == dlg->m_fOffsetSecond &&
      timer_item->bTemporary      == (bool) dlg->m_bTemporary &&
      timer_item->strGroup        == dlg->m_strGroup &&
      timer_item->strVariable     == dlg->m_strVariable 

      ) return false;    // no need to set modified flag if nothing changed

  return true;
  }    // end of CPrefsP16::CheckIfChanged

void CPrefsP16::OnAddTimer() 
{
  CTimerDlg dlg;
  dlg.m_pDoc = m_doc;

  OnAddItem (dlg);
  m_doc->SortTimers ();

}   // end of CPrefsP16::OnAddTimer

void CPrefsP16::OnChangeTimer() 
{

  // don't do it if they are using defaults
  if (m_ctlUseDefaultTimers.GetCheck () != 0 &&
                    !App.m_strDefaultTimersFile.IsEmpty ())
    return;

  CTimerDlg dlg;
  dlg.m_pDoc = m_doc;

  OnChangeItem (dlg);
  m_doc->SortTimers ();

}    // end of CPrefsP16::OnChangeTimer

void CPrefsP16::OnDeleteTimer() 
{
  OnDeleteItem (); 
  m_doc->SortTimers ();

}    // end of CPrefsP16::OnDeleteTimer

void CPrefsP16::OnDblclkTimersList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  
  OnChangeTimer ();	
	*pResult = 0;
}   // end of CPrefsP16::OnDblclkTimersList

int CPrefsP16::CompareObjects (const int iColumn, const CObject * item1, const CObject * item2)
  {
CTimer * timer1 = (CTimer *) item1,
       * timer2 = (CTimer *) item2;

  ASSERT_VALID (timer1);
  ASSERT( timer1->IsKindOf( RUNTIME_CLASS( CTimer ) ) );
  ASSERT_VALID (timer2);
  ASSERT( timer2->IsKindOf( RUNTIME_CLASS( CTimer ) ) );

int iResult = 0;
CmcDateTimeSpan ts1,
                ts2;

  switch (iColumn)   // which sort key
    {
    case eColumnType:
                if (timer1->iType < timer2->iType)
                  iResult = -1;
                else
                if (timer1->iType > timer2->iType)
                  iResult = 1;
                break;
    case eColumnWhen:
                if (timer1->iType == CTimer::eAtTime)
                  ts1 = CmcDateTimeSpan (0, 
                                  timer1->iAtHour, 
                                  timer1->iAtMinute, 
                                  timer1->fAtSecond);
                else
                  ts1 = CmcDateTimeSpan (0, 
                                  timer1->iEveryHour, 
                                  timer1->iEveryMinute, 
                                  timer1->fEverySecond);

                if (timer2->iType == CTimer::eAtTime)
                  ts2 = CmcDateTimeSpan (0, 
                                  timer2->iAtHour, 
                                  timer2->iAtMinute, 
                                  timer2->fAtSecond);
                else
                  ts2 = CmcDateTimeSpan (0, 
                                  timer2->iEveryHour, 
                                  timer2->iEveryMinute, 
                                  timer2->fEverySecond);
                if (ts1 < ts2)
                  iResult = -1;
                else
                if (ts1 > ts2)
                  iResult = 1;
                break;
    case eColumnGroup:    iResult = timer1->strGroup.CompareNoCase (timer2->strGroup); 
                          if (iResult)
                            break;
    case eColumnLabel:    iResult = timer1->strLabel.CompareNoCase (timer2->strLabel); break;
    case eColumnNext:    
                if (timer1->tFireTime < timer2->tFireTime)
                  iResult = -1;
                else
                if (timer1->tFireTime > timer2->tFireTime)
                  iResult = 1;
                break;
    default: iResult = 0;
    } // end of switch

  return iResult;
  }    // end of CPrefsP16::CompareObjects

void CPrefsP16::OnColumnclickTimersList(NMHDR* pNMHDR, LRESULT* pResult) 
  {
   OnColumnclickItemList (pNMHDR, pResult);
  }   // end of CPrefsP16::OnColumnclickTimersList


void CPrefsP16::OnResetTimers() 
{
  m_doc->ResetAllTimers (m_doc->m_TimerMap);
}

void CPrefsP16::OnLoadTimers() 
{
CAliasMap dummy_alias;
CTriggerMap dummy_trigger;

  if (m_doc->Load_Set (TIMER,  "", this))
    return;

  LoadList ();

// They can no longer cancel the propery sheet, the document has changed

  CancelToClose ();
  m_doc->SetModifiedFlag (TRUE);

  // resolve all entry points for triggers, aliases etc.

  // if scripting active, find entry points
  if (m_doc->m_ScriptEngine)
    m_doc->FindAllEntryPoints ();

}

void CPrefsP16::OnSaveTimers() 
{

  if (m_doc->Save_Set (TIMER, this))
    return;
}


void CPrefsP16::OnEnableTimers() 
{
  
  m_doc->m_bConfigEnableTimers = IsDlgButtonChecked(IDC_ENABLE_TIMERS);
	  
}


int CPrefsP16::AddItem (CObject * pItem, 
                         const int item_number,
                         const BOOL insert)
  {
CTimer * timer_item = (CTimer * ) pItem;

ASSERT_VALID (timer_item);
ASSERT( timer_item->IsKindOf( RUNTIME_CLASS( CTimer ) ) );

int nItem;
CString strType;
CString strWhen;

  switch (timer_item->iType)
    {
    case CTimer::eAtTime:
                          strType = "At"; 
                          strWhen.Format ("%02i:%02i:%04.2f",
                                          timer_item->iAtHour, 
                                          timer_item->iAtMinute, 
                                          timer_item->fAtSecond);
                          break;

    case CTimer::eInterval: 
                          strType = "Every"; 
                          if (timer_item->iOffsetHour || 
                              timer_item->iOffsetMinute ||
                              timer_item->fOffsetSecond != 0.0)
                            strWhen.Format ("%02i:%02i:%04.2f offset %02i:%02i:%04.2f",
                                            timer_item->iEveryHour, 
                                            timer_item->iEveryMinute, 
                                            timer_item->fEverySecond,
                                            timer_item->iOffsetHour, 
                                            timer_item->iOffsetMinute, 
                                            timer_item->fOffsetSecond);
                          else
                            strWhen.Format ("%02i:%02i:%04.2f",
                                            timer_item->iEveryHour, 
                                            timer_item->iEveryMinute, 
                                            timer_item->fEverySecond);

                          break;

    default:              strType = "Unknown"; 
                          strWhen = "00:00:00.00";
                          break;
    }

  if (insert)
    {
    nItem = m_ctlTimerList.GetItemCount ();
  	m_ctlTimerList.InsertItem (nItem, strType);  // eColumnType
    }
  else
    {
    nItem = item_number;
  	m_ctlTimerList.SetItemText(nItem, eColumnType, strType);
    }

   
  m_ctlTimerList.SetItemText(nItem, eColumnWhen, strWhen);
  m_ctlTimerList.SetItemText(nItem, eColumnContents, 
                  Replace (timer_item->strContents, ENDLINE, "\\n", true));
	m_ctlTimerList.SetItemText (nItem, eColumnLabel, timer_item->strLabel);
	m_ctlTimerList.SetItemText (nItem, eColumnGroup, timer_item->strGroup);


  CString strDuration;
  CmcDateTime tDue = CmcDateTime (timer_item->tFireTime.GetTime ());
  CmcDateTime tNow = CmcDateTime::GetTimeNow ();
  CmcDateTimeSpan ts = tDue - tNow;

  if (tDue < tNow)
    strDuration = "-";
  else
    {
    if (floor (ts.GetTotalDays ()) > 0)
      strDuration = CFormat ("%.f d", floor (ts.GetTotalDays ()));
    else
      if (floor (ts.GetTotalHours ()) > 0)
        strDuration = CFormat ("%.f h", floor (ts.GetTotalHours ()));
      else
        if (floor (ts.GetTotalMinutes ()) > 0)
          strDuration = CFormat ("%.f m", floor (ts.GetTotalMinutes ()));
        else
          strDuration = CFormat ("%.f s", floor (ts.GetTotalSeconds ()));
    } // not time passed

	m_ctlTimerList.SetItemText (nItem, eColumnNext, strDuration);

  return nItem;

  } // end of CPrefsP16::add_item


BOOL CPrefsP16::OnInitDialog() 
{
	CGenPropertyPage::OnInitDialog();

  // fix up last column to exactly fit the list view

  RECT rect;

  m_ctlTimerList.GetClientRect (&rect);

  m_iColWidth [eColumnContents] = rect.right - rect.left -            // get width of control
                                GetSystemMetrics (SM_CXVSCROLL) -   // minus vertical scroll bar
                                m_iColWidth [eColumnType] -           // minus existing column
                                m_iColWidth [eColumnWhen];            // minus existing column
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}   // end of CPrefsP16::OnInitDialog

     

void CPrefsP16::OnUpdateNotUsingDefaults(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlUseDefaultTimers.GetCheck () == 0 ||
                  App.m_strDefaultTimersFile.IsEmpty ());
  } // end of CPrefsP16::OnUpdateNotUsingDefaults

void CPrefsP16::OnUpdateHaveDefaults(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (!App.m_strDefaultTimersFile.IsEmpty ());
  } // end of CPrefsP16::OnUpdateHaveDefaults


void CPrefsP16::OnUpdateNeedSelection(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlList->GetSelectedCount () != 0   &&
                  (m_ctlUseDefaultTimers.GetCheck () == 0 ||
                  App.m_strDefaultTimersFile.IsEmpty ()));
  } // end of CPrefsP16::OnUpdateNeedSelection

void CPrefsP16::OnUpdateNeedOneSelection(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlList->GetSelectedCount () == 1   &&
                  (m_ctlUseDefaultTimers.GetCheck () == 0 ||
                  App.m_strDefaultTimersFile.IsEmpty ()));
  } // end of CPrefsP16::OnUpdateNeedOneSelection

void CPrefsP16::OnUpdateNeedXMLClipboard(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (IsClipboardXML ());
  } // end of CPrefsP16::OnUpdateNeedXMLClipboard

void CPrefsP16::OnCopy() 
{
OnCopyItem ();	
}

void CPrefsP16::OnPaste() 
{
OnPasteItem ();	
}

bool CPrefsP16::CheckIfIncluded (CObject * pItem)
  {
  CTimer * timer_item = (CTimer *) pItem;

  ASSERT_VALID (timer_item);
  ASSERT( timer_item->IsKindOf( RUNTIME_CLASS( CTimer ) ) );
  
  return timer_item->bIncluded;
  }    // end of CPrefsP16::CheckIfIncluded

bool CPrefsP16::CheckIfTemporary (CObject * pItem)
  {
  CTimer * timer_item = (CTimer *) pItem;

  ASSERT_VALID (timer_item);
  ASSERT( timer_item->IsKindOf( RUNTIME_CLASS( CTimer ) ) );
  
  return timer_item->bTemporary;
  }    // end of CPrefsP16::CheckIfTemporary

bool CPrefsP16::CheckIfExecuting (CObject * pItem)
  {
  CTimer * timer_item = (CTimer *) pItem;

  ASSERT_VALID (timer_item);
  ASSERT( timer_item->IsKindOf( RUNTIME_CLASS( CTimer ) ) );
  
  return timer_item->bExecutingScript;
  }    // end of CPrefsP16::CheckIfExecuting

bool CPrefsP16::GetFilterFlag ()            // is filtering enabled?
  {
  return m_ctlFilter.GetCheck ();
  }

CString CPrefsP16::GetFilterScript ()       // get the filter script
  {
  return m_doc->m_strTimersFilter;
  }

void CPrefsP16::OnFilter() 
{
  LoadList ();  // need to reload the list now	
}

void CPrefsP16::OnEditFilter() 
{
if (EditFilterText (m_doc->m_strTimersFilter))
  LoadList ();  // need to reload the list now	
}

void CPrefsP16::GetFilterInfo (CObject * pItem, lua_State * L)
  {
  CTimer * timer = (CTimer *) pItem;

  ASSERT_VALID (timer);
  ASSERT( timer->IsKindOf( RUNTIME_CLASS( CTimer ) ) );

  // the table of stuff
  lua_newtable(L);                                                            

  MakeTableItemBool (L, "active_closed",    timer->bActiveWhenClosed);
  MakeTableItemBool (L, "at_time",          timer->iType);
  MakeTableItemBool (L, "enabled",          timer->bEnabled);
  MakeTableItem     (L, "group",            timer->strGroup);
  MakeTableItem     (L, "hour",             (timer->iType == CTimer::eAtTime ? timer->iAtHour   : timer->iEveryHour));
  MakeTableItem     (L, "minute",           (timer->iType == CTimer::eAtTime ? timer->iAtMinute : timer->iEveryMinute));
  MakeTableItem     (L, "name",             timer->strLabel);
  MakeTableItem     (L, "offset_hour",      timer->iOffsetHour);
  MakeTableItem     (L, "offset_minute",    timer->iOffsetMinute);
  MakeTableItem     (L, "offset_second",    timer->fOffsetSecond);
  MakeTableItemBool (L, "omit_from_log",    timer->bOmitFromLog);
  MakeTableItemBool (L, "omit_from_output", timer->bOmitFromOutput);
  MakeTableItemBool (L, "one_shot",         timer->bOneShot);
  MakeTableItem     (L, "script",           timer->strProcedure);
  MakeTableItem     (L, "second",           (timer->iType == CTimer::eAtTime ? timer->fAtSecond : timer->fEverySecond));
  MakeTableItem     (L, "send", timer->strContents);
  MakeTableItem     (L, "send_to",          timer->iSendTo);
  MakeTableItemBool (L, "temporary",        timer->bTemporary);
  MakeTableItem     (L, "user",             timer->iUserOption);
  MakeTableItem     (L, "variable",         timer->strVariable);

  // stuff below is not part of the world file but calculated at runtime

  //  GetTimerInfo  (9)
  MakeTableItem (L, "invocation_count", timer->nInvocationCount);
  
  //  GetTimerInfo  (10)
  MakeTableItem (L, "times_fired",      timer->nMatched);

  //  GetTimerInfo  (11)
  if (timer->tWhenFired.GetTime ())  
    MakeTableItem   (L, "when_fired",   COleDateTime (timer->tWhenFired.GetTime ())); 

  //  GetTimerInfo  (12)
  if (timer->tFireTime.GetTime ())  
    MakeTableItem   (L,"fire_time",     COleDateTime (timer->tFireTime.GetTime ())); 

  //  GetTimerInfo  (14)
  MakeTableItemBool (L, "temporary",    timer->bTemporary);

  //  GetTimerInfo  (18)
  MakeTableItemBool (L, "included",     timer->bIncluded);

  //  GetTimerInfo  (26)
  MakeTableItemBool (L, "script_valid", timer->dispid != DISPID_UNKNOWN);

  }   // end of CPrefsP16::GetFilterInfo


/////////////////////////////////////////////////////////////////////////////
// CPrefsP17 property page

IMPLEMENT_DYNCREATE(CPrefsP17, CPropertyPage)

CPrefsP17::CPrefsP17() : CPropertyPage(CPrefsP17::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP17)
	m_strWorldClose = _T("");
	m_strWorldConnect = _T("");
	m_strWorldDisconnect = _T("");
	m_strScriptFilename = _T("");
	m_bEnableScripts = FALSE;
	m_iNoteTextColour = -1;
	m_strScriptPrefix = _T("");
	m_strWorldOpen = _T("");
	m_strScriptEditor = _T("");
	m_iReloadOption = -1;
	m_strWorldGetFocus = _T("");
	m_strWorldLoseFocus = _T("");
	m_bEditScriptWithNotepad = FALSE;
	m_strExecutionTime = _T("");
	m_strWorldSave = _T("");
	m_bWarnIfScriptingInactive = FALSE;
	m_strIsActive = _T("");
	m_bScriptErrorsToOutputWindow = FALSE;
	//}}AFX_DATA_INIT
  m_page_number = 1;
  m_prefsp3 = NULL;
  m_doc = NULL;

}

CPrefsP17::~CPrefsP17()
{
}

void CPrefsP17::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP17)
	DDX_Control(pDX, IDC_REGISTER, m_ctlRegister);
	DDX_Control(pDX, IDC_USE_NOTEPAD, m_ctlEditScriptWithNotepad);
	DDX_Control(pDX, IDC_CHOOSE_EDITOR, m_ctlChooseEditor);
	DDX_Control(pDX, IDC_TEXT_COLOUR, m_ctlTextColour);
	DDX_Control(pDX, IDC_SCRIPT_TEXT_SWATCH2, m_ctlTextSwatch2);
	DDX_Control(pDX, IDC_SCRIPT_TEXT_SWATCH, m_ctlTextSwatch);
	DDX_Control(pDX, IDC_LANGUAGE, m_ctlLanguage);
	DDX_Text(pDX, IDC_ON_WORLD_CLOSE, m_strWorldClose);
	DDX_Text(pDX, IDC_ON_WORLD_CONNECT, m_strWorldConnect);
	DDX_Text(pDX, IDC_ON_WORLD_DISCONNECT, m_strWorldDisconnect);
	DDX_Text(pDX, IDC_SCRIPT_FILENAME, m_strScriptFilename);
	DDX_Check(pDX, IDC_ENABLE, m_bEnableScripts);
	DDX_CBIndex(pDX, IDC_TEXT_COLOUR, m_iNoteTextColour);
	DDX_Text(pDX, IDC_SCRIPT_PREFIX, m_strScriptPrefix);
	DDX_Text(pDX, IDC_ON_WORLD_OPEN, m_strWorldOpen);
	DDX_Text(pDX, IDC_SCRIPT_EDITOR, m_strScriptEditor);
	DDX_CBIndex(pDX, IDC_RELOAD_OPTION, m_iReloadOption);
	DDX_Text(pDX, IDC_ON_WORLD_GET_FOCUS, m_strWorldGetFocus);
	DDX_Text(pDX, IDC_ON_WORLD_LOSE_FOCUS, m_strWorldLoseFocus);
	DDX_Check(pDX, IDC_USE_NOTEPAD, m_bEditScriptWithNotepad);
	DDX_Text(pDX, IDC_EXECUTION_TIME, m_strExecutionTime);
	DDX_Text(pDX, IDC_ON_WORLD_SAVE, m_strWorldSave);
	DDX_Check(pDX, IDC_WARN_IF_INACTIVE, m_bWarnIfScriptingInactive);
	DDX_Text(pDX, IDC_IS_ACTIVE, m_strIsActive);
	DDX_Check(pDX, IDC_ERRORS_AS_NOTES, m_bScriptErrorsToOutputWindow);
	//}}AFX_DATA_MAP

  if(pDX->m_bSaveAndValidate)
    {

    // don't let them foolishly slip in spaces :)
		m_strWorldClose.TrimLeft ();
		m_strWorldSave.TrimLeft ();
		m_strWorldConnect.TrimLeft ();
		m_strWorldDisconnect.TrimLeft ();
		m_strWorldOpen.TrimLeft ();
		m_strWorldGetFocus.TrimLeft ();
		m_strWorldLoseFocus.TrimLeft ();
	  m_strOnMXP_Start.TrimLeft ();
	  m_strOnMXP_Stop.TrimLeft ();
	  m_strOnMXP_OpenTag.TrimLeft ();
	  m_strOnMXP_CloseTag.TrimLeft ();
	  m_strOnMXP_SetVariable.TrimLeft ();
	  m_strOnMXP_Error.TrimLeft ();

		m_strWorldClose.TrimRight ();
		m_strWorldSave.TrimRight ();
		m_strWorldConnect.TrimRight ();
		m_strWorldDisconnect.TrimRight ();
		m_strWorldOpen.TrimRight ();
		m_strWorldGetFocus.TrimRight ();
		m_strWorldLoseFocus.TrimRight ();
	  m_strOnMXP_Start.TrimRight ();
	  m_strOnMXP_Stop.TrimRight ();
	  m_strOnMXP_OpenTag.TrimRight ();
	  m_strOnMXP_CloseTag.TrimRight ();
	  m_strOnMXP_SetVariable.TrimRight ();
	  m_strOnMXP_Error.TrimRight ();

    }   // end of saving and validating
  else
    {
    m_ctlChooseEditor.EnableWindow (!m_bEditScriptWithNotepad);
    } // loading
}


BEGIN_MESSAGE_MAP(CPrefsP17, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP17)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_CBN_SELCHANGE(IDC_TEXT_COLOUR, OnSelchangeTextColour)
	ON_BN_CLICKED(IDC_REGISTER, OnRegister)
	ON_BN_CLICKED(IDC_EDIT_SCRIPT, OnEditScript)
	ON_BN_CLICKED(IDC_CHOOSE_EDITOR, OnChooseEditor)
	ON_BN_CLICKED(IDC_USE_NOTEPAD, OnUseNotepad)
	ON_BN_CLICKED(IDC_MXP_SCRIPTS, OnMxpScripts)
	ON_BN_CLICKED(IDC_NEW_SCRIPT, OnNewScript)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_EDIT_SCRIPT, OnUpdateNeedScriptFile)
  ON_UPDATE_COMMAND_UI(IDC_REGISTER, OnUpdateEnableRegisterButton)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsP17 message handlers

BOOL CPrefsP17::ScriptBrowser (BOOL bLoad)
  {
CString strSubscript,
        strPrompt;

int nItem = m_ctlLanguage.GetCurSel ();

  switch (nItem)
    {
    case SCRIPT_VBSCRIPT:   strSubscript = "vbs"; 
              strPrompt = "VBscript source files (*.vbs)|*.vbs";
              break;
    case SCRIPT_JSCRIPT:   strSubscript = "js"; 
              strPrompt = "JScript source files (*.jvs;*.js)|*.jvs;*.js";
              break;
    case SCRIPT_PERL:   strSubscript = "pl"; 
              strPrompt = "PerlScript source files (*.pl)|*.pl";
              break;
#ifdef USE_PYTHON
    case SCRIPT_PYTHON:   strSubscript = "pys"; 
              strPrompt = "Python source files (*.pys;*.py)|*.pys;*.py";
              break;
#endif
#ifdef USE_TCL
    case SCRIPT_TCL:   strSubscript = "tcl"; 
              strPrompt = "Tcl source files (*.tcl)|*.tcl";
              break;
#endif
    case SCRIPT_LUA:   strSubscript = "lua"; 
              strPrompt = "Lua source files (*.lua)|*.lua";
              break;
#ifdef USE_PHP
    case SCRIPT_PHP:   strSubscript = "phpscript"; 
              strPrompt = "PHP source files (*.php)|*.php";
              break;
#endif
#ifdef USE_RUBY
    case SCRIPT_RUBY:   strSubscript = "RubyScript"; 
              strPrompt = "Ruby source files (*.rb)|*.rb";
              break;
#endif
    default:  return FALSE;
    }   // end of switch

  int iFlags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;

  if (!bLoad)
     iFlags = OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST;

  strPrompt += "|All files (*.*)|*.*||";

  CString strFilename = m_strScriptFilename;

  if (!bLoad)
     strFilename.Empty ();

	CFileDialog	dlg(bLoad,
		              strSubscript,
		              strFilename,
		              iFlags,
		              strPrompt,
		              this);
	
	dlg.m_ofn.lpstrTitle = "Script source file";

  ChangeToFileBrowsingDirectory ();
  int nResult = dlg.DoModal();
  ChangeToStartupDirectory ();

	if (nResult != IDOK)
    return FALSE;

  m_strScriptFilename = dlg.GetPathName ();
  SetDlgItemText(IDC_SCRIPT_FILENAME, m_strScriptFilename);

  return TRUE;

  } // end of CPrefsP17::ScriptBrowser

void CPrefsP17::OnBrowse() 
  {
   ScriptBrowser (TRUE);
  }  // end of CPrefsP17::OnBrowse() 

void CPrefsP17::OnNewScript() 
{
 if (!ScriptBrowser (FALSE))
   return;

  try
    {

    CFile (m_strScriptFilename, 
           CFile::modeCreate | CFile::modeReadWrite);
	
    } // end of try block

  catch (CException * e)
    {
    e->ReportError ();
    e->Delete ();
    }

}   // end of CPrefsP17::OnNewScript() 


void CPrefsP17::FixSwatch (CColourButton & m_ctlSwatch,
                           CColourButton & m_ctlSwatch2,
                           int iNewColour)
  {
  if (iNewColour == 0)
    {
     m_ctlSwatch.ShowWindow (SW_HIDE);
     m_ctlSwatch2.ShowWindow (SW_HIDE);
    }
  else
    {
    m_ctlSwatch.ShowWindow (SW_SHOW | SW_SHOWNOACTIVATE);
    m_ctlSwatch2.ShowWindow (SW_SHOW | SW_SHOWNOACTIVATE);
    m_ctlSwatch.m_colour = m_prefsp3->m_customtext [iNewColour - 1];
    m_ctlSwatch2.m_colour = m_prefsp3->m_customback [iNewColour - 1];
    m_ctlSwatch.RedrawWindow ();
    m_ctlSwatch2.RedrawWindow ();
    }
  } // end of FixSwatch

BOOL CPrefsP17::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
int nItem = -1;

  if (m_strLanguage.CompareNoCase ("vbscript") == 0)
    nItem = SCRIPT_VBSCRIPT;
  else
  if (m_strLanguage.CompareNoCase ("jscript") == 0)
    nItem = SCRIPT_JSCRIPT;
  else
  if (m_strLanguage.CompareNoCase ("perlscript") == 0)
    nItem = SCRIPT_PERL;
  else
  if (m_strLanguage.CompareNoCase ("python") == 0)
    nItem = SCRIPT_PYTHON;
  else
  if (m_strLanguage.CompareNoCase ("tclscript") == 0)
    nItem = SCRIPT_TCL;
  else
  if (m_strLanguage.CompareNoCase ("lua") == 0)
    nItem = SCRIPT_LUA;
  else
  if (m_strLanguage.CompareNoCase ("phpscript") == 0)
    nItem = SCRIPT_PHP;
  else
  if (m_strLanguage.CompareNoCase ("rubyscript") == 0)
    nItem = SCRIPT_RUBY;

  m_ctlLanguage.SetCurSel(nItem);
	
  // load combo box now
  m_ctlTextColour.ResetContent ();
  m_ctlTextColour.AddString ("(default)");
  for (int i = 0; i < MAX_CUSTOM; i++)
    m_ctlTextColour.AddString (m_prefsp3->m_strCustomName [i]);

  // put the selection back
  m_ctlTextColour.SetCurSel (m_iNoteTextColour);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CPrefsP17::OnSelchangeTextColour() 
{
  if (m_ctlTextColour.GetCurSel () != CB_ERR)
     FixSwatch (m_ctlTextSwatch, 
                 m_ctlTextSwatch2,
                 m_ctlTextColour.GetCurSel ());
}

void CPrefsP17::OnOK() 
{

int nItem = m_ctlLanguage.GetCurSel ();

  switch (nItem)
    {
    case SCRIPT_VBSCRIPT:   m_strLanguage = "VBscript"; break;
    case SCRIPT_JSCRIPT:    m_strLanguage = "JScript"; break;
    case SCRIPT_PERL:       m_strLanguage = "PerlScript"; break;
    case SCRIPT_PYTHON:     m_strLanguage = "Python"; break;
    case SCRIPT_TCL:        m_strLanguage = "TclScript"; break;
    case SCRIPT_LUA:        m_strLanguage = "Lua"; break;
    case SCRIPT_PHP:        m_strLanguage = "PhpScript"; break;
    case SCRIPT_RUBY:       m_strLanguage = "RubyScript"; break;
    default:  m_strLanguage = ""; break;
    }   // end of switch
  
 	CPropertyPage::OnOK();
}

// register our VBSCRIPT or JSCRIPT dll - see REGSVR code

void CPrefsP17::OnRegister() 
{
CString strDLLname = "VBscript.dll";

int nItem = m_ctlLanguage.GetCurSel ();

  switch (nItem)
    {
    case SCRIPT_VBSCRIPT:   strDLLname = "VBscript.dll";
              break;
    case SCRIPT_JSCRIPT:    strDLLname = "JScript.dll";
              break;
    case SCRIPT_PHP:        strDLLname = "php5activescript.dll";
              break;
    default:  return;
    }   // end of switch

HRESULT (FAR STDAPICALLTYPE * lpDllEntryPoint)(void);
CHECKHRESULT_EXCEPTION hr;
HINSTANCE hLib = NULL;
  
  try
    {

	  // Load the library.	
	  hLib = LoadLibrary(strDLLname);
	  if (!hLib)
      ThrowSystemException ();

	  // Find the entry point.		
	  (FARPROC&)lpDllEntryPoint = GetProcAddress(hLib, "DllRegisterServer");
	  if (!lpDllEntryPoint)
      ThrowSystemException ();

	  // Call the entry point.
    hr = (*lpDllEntryPoint) ();

   ::UMessageBox (TFormat ("Successfully registered %s", (LPCTSTR) strDLLname),
                          MB_ICONINFORMATION);

    } // end of try block

  catch (CException * e)
    {
    e->ReportError ();
    e->Delete ();
    }

  if (hLib)
 		FreeLibrary(hLib);

}


void CPrefsP17::OnEditScript() 
{
  if (m_ctlEditScriptWithNotepad.GetCheck ())
    {
    CTextDocument * pNewDoc =
      (CTextDocument *) App.OpenDocumentFile (m_strScriptFilename);

    if (pNewDoc)
      {
      pNewDoc->SetTheFont ();
      pNewDoc->m_pRelatedWorld = m_doc;
      pNewDoc->m_iUniqueDocumentNumber = m_doc->m_iUniqueDocumentNumber;
      }
    else
      ::TMessageBox("Unable to edit the script file.", 
                      MB_ICONEXCLAMATION);
    return;
    }   // end of using inbuilt notepad

  m_doc->EditFileWithEditor (m_strScriptFilename);

}


void CPrefsP17::OnChooseEditor() 
{
	CFileDialog	dlg(TRUE,
		"exe",
		m_strScriptEditor,
		OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,
		"Applications (*.exe)|*.exe||",
		this);
	
	dlg.m_ofn.lpstrTitle = "Script editor";

  ChangeToFileBrowsingDirectory ();
  int nResult = dlg.DoModal();
  ChangeToStartupDirectory ();

	if (nResult != IDOK)
    return;

  m_strScriptEditor = dlg.GetPathName ();
  SetDlgItemText(IDC_SCRIPT_EDITOR, m_strScriptEditor);
}


LRESULT CPrefsP17::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CPrefsP17::OnKickIdle

void CPrefsP17::OnUpdateNeedScriptFile(CCmdUI* pCmdUI)
  {

CString strScriptFilename;

  GetDlgItemText (IDC_SCRIPT_FILENAME, strScriptFilename);

  pCmdUI->Enable (!strScriptFilename.IsEmpty ());

  } // end of CPrefsP17::OnUpdateNeedScriptFile

void CPrefsP17::OnUpdateEnableRegisterButton(CCmdUI* pCmdUI)
  {

BOOL bEnable = FALSE;

int nItem = m_ctlLanguage.GetCurSel ();

  switch (nItem)
    {
    case SCRIPT_VBSCRIPT:  
    case SCRIPT_JSCRIPT:   
    case SCRIPT_PHP:       
            bEnable = true;
            break;
    }   // end of switch

  pCmdUI->Enable (bEnable);

  } // end of CPrefsP17::OnUpdateEnableRegisterButton


void CPrefsP17::OnUseNotepad() 
{
 m_ctlChooseEditor.EnableWindow (!m_ctlEditScriptWithNotepad.GetCheck ());

}

void CPrefsP17::OnMxpScripts() 
{    
CMXPscriptRoutinesDlg dlg;


  dlg.m_strOnMXP_Start =        m_strOnMXP_Start;       
  dlg.m_strOnMXP_Stop =         m_strOnMXP_Stop;        
  dlg.m_strOnMXP_OpenTag =      m_strOnMXP_OpenTag;     
  dlg.m_strOnMXP_CloseTag =     m_strOnMXP_CloseTag;    
  dlg.m_strOnMXP_SetVariable =  m_strOnMXP_SetVariable; 
  dlg.m_strOnMXP_Error =        m_strOnMXP_Error;       

  if (dlg.DoModal () != IDOK)
    return;
	
  m_strOnMXP_Start =        dlg.m_strOnMXP_Start;       
  m_strOnMXP_Stop =         dlg.m_strOnMXP_Stop;        
  m_strOnMXP_OpenTag =      dlg.m_strOnMXP_OpenTag;     
  m_strOnMXP_CloseTag =     dlg.m_strOnMXP_CloseTag;    
  m_strOnMXP_SetVariable =  dlg.m_strOnMXP_SetVariable; 
  m_strOnMXP_Error =        dlg.m_strOnMXP_Error;       

}

/////////////////////////////////////////////////////////////////////////////
// CPrefsP18 property page

IMPLEMENT_DYNCREATE(CPrefsP18, CGenPropertyPage)

CPrefsP18::CPrefsP18() : CGenPropertyPage(CPrefsP18::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP18)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
  m_doc = NULL;

}

CPrefsP18::~CPrefsP18()
{
}

void CPrefsP18::DoDataExchange(CDataExchange* pDX)
{
	CGenPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP18)
	DDX_Control(pDX, IDC_COUNT, m_ctlSummary);
	DDX_Control(pDX, IDC_FILTER, m_ctlFilter);
	DDX_Control(pDX, IDC_VARIABLES_LIST, m_ctlVariableList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsP18, CGenPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP18)
	ON_BN_CLICKED(IDC_ADD_VARIABLE, OnAddVariable)
	ON_BN_CLICKED(IDC_CHANGE_VARIABLE, OnChangeVariable)
	ON_BN_CLICKED(IDC_DELETE_VARIABLE, OnDeleteVariable)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_VARIABLES_LIST, OnColumnclickVariablesList)
	ON_BN_CLICKED(IDC_FIND, OnFind)
	ON_BN_CLICKED(IDC_FIND_NEXT, OnFindNext)
	ON_NOTIFY(NM_DBLCLK, IDC_VARIABLES_LIST, OnDblclkVariablesList)
	ON_BN_CLICKED(IDC_LOAD_VARIABLES, OnLoadVariables)
	ON_BN_CLICKED(IDC_SAVE_VARIABLES, OnSaveVariables)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	ON_BN_CLICKED(IDC_PASTE, OnPaste)
	ON_BN_CLICKED(IDC_FILTER, OnFilter)
	ON_BN_CLICKED(IDC_EDIT_FILTER, OnEditFilter)
	//}}AFX_MSG_MAP
  ON_UPDATE_COMMAND_UI(IDC_CHANGE_VARIABLE, OnUpdateNeedOneSelection)
  ON_UPDATE_COMMAND_UI(IDC_DELETE_VARIABLE, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_COPY, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_PASTE, OnUpdateNeedXMLClipboard)
  ON_UPDATE_COMMAND_UI(IDC_CLEAR_VARIABLES, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_FIND, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_FIND_NEXT, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_SAVE_VARIABLES, OnUpdateNeedEntries)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsP18 message handlers

void CPrefsP18::InitDialog (CDialog * pDlg)
  {
  CEditVariable * dlg = (CEditVariable *) pDlg;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CEditVariable ) ) );

  dlg->m_pVariableMap = &m_doc->m_VariableMap;
  dlg->m_bDoingChange = false;

  }  // end of CPrefsP18::InitDialog

void CPrefsP18::LoadDialog (CDialog * pDlg, CObject * pItem)
  {
  CEditVariable * dlg = (CEditVariable *) pDlg;
  CVariable * variable_item = (CVariable *) pItem;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CEditVariable ) ) );
  ASSERT_VALID (variable_item);
  ASSERT( variable_item->IsKindOf( RUNTIME_CLASS( CVariable ) ) );

  dlg->m_strName     = variable_item->strLabel;
  dlg->m_strContents = Replace (variable_item->strContents, "\r", "", true);
  dlg->m_strContents = Replace (dlg->m_strContents, "\n", ENDLINE, true);

  dlg->m_pVariableMap = &m_doc->m_VariableMap;
  dlg->m_bDoingChange = true;

  }   // end of CPrefsP18::LoadDialog


void CPrefsP18::UnloadDialog (CDialog * pDlg, CObject * pItem)
  {
  CEditVariable * dlg = (CEditVariable *) pDlg;
  CVariable * variable_item = (CVariable *) pItem;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CEditVariable ) ) );
  ASSERT_VALID (variable_item);
  ASSERT( variable_item->IsKindOf( RUNTIME_CLASS( CVariable ) ) );

  variable_item->strLabel       = dlg->m_strName;
  variable_item->strContents   = dlg->m_strContents;

  }   // end of CPrefsP18::UnloadDialog


CString CPrefsP18::GetObjectName (CDialog * pDlg) const
  {
  CEditVariable * dlg = (CEditVariable *) pDlg;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CEditVariable ) ) );
        
  return dlg->m_strName;
  }    // end of CPrefsP18::GetObjectName

CString CPrefsP18::GetScriptName (CObject * pItem) const
  {
  return "";    // variables don't have scripts

  } // end of CPrefsP18::GetScriptName 

CString CPrefsP18::GetLabel (CObject * pItem) const
  {
  return "";    // variables don't have labels

  } // end of CPrefsP18::GetLabel 
 
void CPrefsP18::SetDispatchID (CObject * pItem, const DISPID dispid)
  {
  // variables don't have dispatch IDs
  } // end of CPrefsP18::SetDispatchID

void CPrefsP18::SetInternalName (CObject * pItem, const CString strName)
  {
  // variables don't have internal names
  } // end of CPrefsP18::SetInternalName

CObject * CPrefsP18::MakeNewObject (void)
  {
  return (CObject *) new CVariable;
  }    // end of CPrefsP18::MakeNewObject

__int64 CPrefsP18::GetModificationNumber (CObject * pItem) const
  {
  CVariable * variable_item = (CVariable *) pItem;

  ASSERT_VALID (variable_item);
  ASSERT( variable_item->IsKindOf( RUNTIME_CLASS( CVariable ) ) );

  return variable_item->nUpdateNumber;

  }   // end of CPrefsP18::GetModificationNumber

void CPrefsP18::SetModificationNumber (CObject * pItem, __int64 nUpdateNumber)
  {
  CVariable * variable_item = (CVariable *) pItem;

  ASSERT_VALID (variable_item);
  ASSERT( variable_item->IsKindOf( RUNTIME_CLASS( CVariable ) ) );
  
  variable_item->nUpdateNumber = nUpdateNumber;
  }    // end of CPrefsP18::SetModificationNumber


bool CPrefsP18::CheckIfChanged (CDialog * pDlg, CObject * pItem) const
  {
  CEditVariable * dlg = (CEditVariable *) pDlg;
  CVariable * variable_item = (CVariable *) pItem;

  ASSERT_VALID (dlg);
  ASSERT( dlg->IsKindOf( RUNTIME_CLASS( CEditVariable ) ) );
  ASSERT_VALID (variable_item);
  ASSERT( variable_item->IsKindOf( RUNTIME_CLASS( CVariable ) ) );

  if (variable_item->strLabel          == dlg->m_strName &&
      variable_item->strContents      == dlg->m_strContents
     ) return false;    // no need to set modified flag if nothing changed

  return true;
  }    // end of CPrefsP18::CheckIfChanged


void CPrefsP18::OnAddVariable() 
{
  CEditVariable dlg;

  OnAddItem (dlg);
}   // end of CPrefsP18::OnAddVariable

void CPrefsP18::OnChangeVariable() 
{
  CEditVariable dlg;

  OnChangeItem (dlg);
}    // end of CPrefsP18::OnChangeVariable

void CPrefsP18::OnDeleteVariable() 
{
  OnDeleteItem ();
}    // end of CPrefsP18::OnDeleteVariable

void CPrefsP18::OnDblclkVariablesList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  
  OnChangeVariable ();	
	*pResult = 0;
}   // end of CPrefsP18::OnDblclkVariableesList

int CPrefsP18::CompareObjects (const int iColumn, const CObject * item1, const CObject * item2)
  {
CVariable * variable1 = (CVariable *) item1,
       * variable2 = (CVariable *) item2;

  ASSERT_VALID (variable1);
  ASSERT( variable1->IsKindOf( RUNTIME_CLASS( CVariable ) ) );
  ASSERT_VALID (variable2);
  ASSERT( variable2->IsKindOf( RUNTIME_CLASS( CVariable ) ) );

int iResult;

  switch (iColumn)   // which sort key
    {
    case eColumnName: iResult = variable1->strLabel.CompareNoCase (variable2->strLabel); break;
    case eColumnContents: iResult = variable1->strContents.CompareNoCase (variable2->strContents); break;
    default: iResult = 0;
    } // end of switch

  return iResult;
  }    // end of CPrefsP18::CompareObjects

void CPrefsP18::OnColumnclickVariablesList(NMHDR* pNMHDR, LRESULT* pResult) 
  {
   OnColumnclickItemList (pNMHDR, pResult);
  }   // end of CPrefsP18::OnColumnclickVariableesList


int CPrefsP18::AddItem (CObject * pItem, 
                         const int item_number,
                         const BOOL insert)
  {
CVariable * variable_item = (CVariable * ) pItem;

ASSERT_VALID (variable_item);
ASSERT( variable_item->IsKindOf( RUNTIME_CLASS( CVariable ) ) );

int nItem;

  if (insert)
    {
    nItem = m_ctlVariableList.GetItemCount ();
  	m_ctlVariableList.InsertItem (nItem, variable_item->strLabel);   // eColumnName
    }
  else
    {
    nItem = item_number;
  	m_ctlVariableList.SetItemText(nItem, eColumnName, variable_item->strLabel);
    }

  // first get rid of carriage-returns
  CString strContents = Replace (variable_item->strContents, "\r", "", true);
  // now show newlines as \n
  strContents = Replace (strContents, "\n", "\\n", true);

	m_ctlVariableList.SetItemText(nItem, eColumnContents, strContents);

  return nItem;

  } // end of CPrefsP18::add_item

void CPrefsP18::OnFind() 
{
  DoFind (false);
}    // end of CPrefsP18::OnFind

void CPrefsP18::OnFindNext() 
{
  DoFind (true);
}   // end of CPrefsP18::OnFindNext


BOOL CPrefsP18::OnInitDialog() 
{
	CGenPropertyPage::OnInitDialog();

  // fix up last column to exactly fit the list view
  RECT rect;

  m_ctlVariableList.GetClientRect (&rect);

  m_iColWidth [eColumnContents] = rect.right - rect.left -            // get width of control
                                GetSystemMetrics (SM_CXVSCROLL) -   // minus vertical scroll bar
                                m_iColWidth [eColumnName];           // minus width of other items
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}   // end of CPrefsP18::OnInitDialog

void CPrefsP18::OnLoadVariables() 
{
int iBadLines = 0;
int iCount = 0;
CFile * xmlf = NULL;
CArchive * ar = NULL;

	try
	  {

	  CFileDialog	dlg(TRUE,						// TRUE for FileOpen
					  "txt",						// default extension
					  "variables.txt",						// initial file name
					  OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
					  "Text files (*.txt)|*.txt|All files (*.*)|*.*||");
	  dlg.m_ofn.lpstrTitle = "Select variables file";

    ChangeToFileBrowsingDirectory ();
    int nResult = dlg.DoModal();
    ChangeToStartupDirectory ();

	  if (nResult != IDOK)
		  return;


    // see if xml variables :)

    xmlf = new CFile (dlg.GetPathName(), CFile::modeRead|CFile::shareDenyNone);
    ar = new CArchive(xmlf, CArchive::load);;

    if (IsArchiveXML (*ar))
      {
      iCount = m_doc->Load_World_XML (*ar, XML_VARIABLES | XML_NO_PLUGINS, false); 
      delete ar;      // delete archive
      ar = NULL;
      delete xmlf;       // delete file
      xmlf = NULL;
      }    // end of XML
    else  
      {    // straight text

      CStdioFile f (dlg.GetPathName(), CFile::modeRead|CFile::shareDenyNone|CFile::typeText);

      CString sBuf;
      BOOL bOK;
      while (bOK = f.ReadString (sBuf))
        {
        sBuf.TrimLeft ();

        // ignore blank lines
        if (sBuf.IsEmpty ())
          continue;   // ignore blank lines

        int iTab = sBuf.Find ("\t");

        // no tab - error
        if (iTab == -1)
          {
          iBadLines++;
          continue;
          }


        CString strName = sBuf.Left (iTab);
        CString strContents;
      
        if (iTab < (sBuf.GetLength () - 1))
          strContents = sBuf.Mid (iTab + 1);

        strName.TrimRight ();
        strContents.TrimLeft ();
        strContents.TrimRight ();

        // add variable to variables map
        if (m_doc->DoSetVariable (strName, strContents) != eOK)
          {
          iBadLines++;
          continue;
          }
        else 
          iCount++;

        }   // end of read loop

      f.Close ();
      } // end of not XML
    }   // end of try
	catch (CException* e)
	  {
		e->ReportError();
		e->Delete();
	  }   // end of catch

  delete ar;      // delete archive
  delete xmlf;       // delete file

  if (iBadLines)
    ::UMessageBox (TFormat ("%i line%s could not be added as a variable.",
                     PLURAL (iBadLines)),
                      MB_ICONEXCLAMATION);

  if (iCount)
    {
    LoadList ();

  // They can no longer cancel the propery sheet, the document has changed

    CancelToClose ();
    m_doc->SetModifiedFlag (TRUE);
    }

  ::UMessageBox (TFormat ("Loaded %i variable%s.", PLURAL (iCount)),
                   MB_ICONINFORMATION);

}

void CPrefsP18::OnSaveVariables() 
{

int iCount = m_doc->m_VariableMap.GetCount ();

  if (m_doc->m_VariableMap.IsEmpty ()) 
    {
    ::TMessageBox ("No variables in this world.");
    return;
    }

CFile * f = NULL;
CArchive * ar = NULL;

  CPlugin * pSavedPlugin = m_doc->m_CurrentPlugin;
  m_doc->m_CurrentPlugin = NULL;   // make sure we save main triggers etc.

	try
	  {

	  CFileDialog	dlg(FALSE,						// FALSE for FileSave
					  "txt",						// default extension
					  "variables.txt",						// initial file name
					  OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST,
					  "Text files (*.txt)|*.txt||",
					  this);
	  dlg.m_ofn.lpstrTitle = "File to save variables as";

    ChangeToFileBrowsingDirectory ();
    int nResult = dlg.DoModal();
    ChangeToStartupDirectory ();

	  if (nResult != IDOK)
		  return;

    f = new CFile (dlg.GetPathName (), 
                    CFile::modeCreate | CFile::modeReadWrite);

    ar = new CArchive(f, CArchive::store);

    m_doc->Save_World_XML (*ar, XML_VARIABLES);

    }   // end of try
	catch (CException* e)
	  {
		e->ReportError();
		e->Delete();
	  }   // end of catch

  m_doc->m_CurrentPlugin = pSavedPlugin;

  delete ar;      // delete archive
  delete f;       // delete file

  ::UMessageBox (TFormat ("Saved %i variable%s.", PLURAL (iCount)),
                   MB_ICONINFORMATION);

}

void CPrefsP18::OnUpdateNeedXMLClipboard(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (IsClipboardXML ());
  } // end of CPrefsP18::OnUpdateNeedXMLClipboard

void CPrefsP18::OnCopy() 
{
OnCopyItem ();	
}

void CPrefsP18::OnPaste() 
{
OnPasteItem ();	
}

void CPrefsP18::OnUpdateNeedOneSelection(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlList->GetSelectedCount () == 1);
  } // end of OnUpdateNeedOneSelection::OnUpdateNeedOneSelection

bool CPrefsP18::GetFilterFlag ()            // is filtering enabled?
  {
  return m_ctlFilter.GetCheck ();
  }

CString CPrefsP18::GetFilterScript ()       // get the filter script
  {
  return m_doc->m_strVariablesFilter;
  }

void CPrefsP18::OnFilter() 
{
  LoadList ();  // need to reload the list now	
}

void CPrefsP18::OnEditFilter() 
{
if (EditFilterText (m_doc->m_strVariablesFilter))
  LoadList ();  // need to reload the list now	
}

void CPrefsP18::GetFilterInfo (CObject * pItem, lua_State * L)
  {
  CVariable * variable = (CVariable *) pItem;

  ASSERT_VALID (variable);
  ASSERT( variable->IsKindOf( RUNTIME_CLASS( CVariable ) ) );

  // the table of stuff
  lua_newtable(L);                                                            

  MakeTableItem     (L, "contents",     variable->strContents);

  }  // end of CPrefsP18::GetFilterInfo 



/////////////////////////////////////////////////////////////////////////////
// CPrefsP19 property page

IMPLEMENT_DYNCREATE(CPrefsP19, CPropertyPage)

CPrefsP19::CPrefsP19() : CPropertyPage(CPrefsP19::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP19)
	m_strAutoSayString = _T("");
	m_bEnableAutoSay = FALSE;
	m_bExcludeMacros = FALSE;
	m_bExcludeNonAlpha = FALSE;
	m_strOverridePrefix = _T("");
	m_bReEvaluateAutoSay = FALSE;
	//}}AFX_DATA_INIT
  m_page_number = 1;
  m_doc = NULL;

}

CPrefsP19::~CPrefsP19()
{
}

void CPrefsP19::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP19)
	DDX_Text(pDX, IDC_AUTO_SAY_STRING, m_strAutoSayString);
	DDX_Check(pDX, IDC_ENABLE_AUTOSAY, m_bEnableAutoSay);
	DDX_Check(pDX, IDC_EXCLUDE_MACROS, m_bExcludeMacros);
	DDX_Check(pDX, IDC_EXCLUDE_NON_ALPHA, m_bExcludeNonAlpha);
	DDX_Text(pDX, IDC_OVERRIDE_PREFIX, m_strOverridePrefix);
	DDX_Check(pDX, IDC_RE_EVALUATE_AUTO_SAY, m_bReEvaluateAutoSay);
	//}}AFX_DATA_MAP

   if(pDX->m_bSaveAndValidate)
     {
     if (m_bEnableAutoSay)    // test is only relevant if enabled
       {
        if(m_strAutoSayString.IsEmpty ())
            {
            ::TMessageBox("Your \"auto say\" string cannot be blank");
            DDX_Text(pDX, IDC_AUTO_SAY_STRING, m_strAutoSayString);
            pDX->Fail();
            }     // end of auto say string being blank
       }    // end of auto say enabled

     }    // end of saving and validating

}


BEGIN_MESSAGE_MAP(CPrefsP19, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP19)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsP19 message handlers

/////////////////////////////////////////////////////////////////////////////
// CPrefsP20 property page

IMPLEMENT_DYNCREATE(CPrefsP20, CPropertyPage)

CPrefsP20::CPrefsP20() : CPropertyPage(CPrefsP20::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP20)
	m_bBold0 = FALSE;
	m_bBold1 = FALSE;
	m_bBold10 = FALSE;
	m_bBold11 = FALSE;
	m_bBold12 = FALSE;
	m_bBold13 = FALSE;
	m_bBold14 = FALSE;
	m_bBold15 = FALSE;
	m_bBold2 = FALSE;
	m_bBold3 = FALSE;
	m_bBold4 = FALSE;
	m_bBold5 = FALSE;
	m_bBold6 = FALSE;
	m_bBold7 = FALSE;
	m_bBold8 = FALSE;
	m_bBold9 = FALSE;
	m_bItalic0 = FALSE;
	m_bItalic1 = FALSE;
	m_bItalic10 = FALSE;
	m_bItalic11 = FALSE;
	m_bItalic12 = FALSE;
	m_bItalic13 = FALSE;
	m_bItalic14 = FALSE;
	m_bItalic15 = FALSE;
	m_bItalic2 = FALSE;
	m_bItalic3 = FALSE;
	m_bItalic4 = FALSE;
	m_bItalic5 = FALSE;
	m_bItalic6 = FALSE;
	m_bItalic7 = FALSE;
	m_bItalic8 = FALSE;
	m_bItalic9 = FALSE;
	m_bUnderline0 = FALSE;
	m_bUnderline1 = FALSE;
	m_bUnderline10 = FALSE;
	m_bUnderline11 = FALSE;
	m_bUnderline12 = FALSE;
	m_bUnderline13 = FALSE;
	m_bUnderline14 = FALSE;
	m_bUnderline15 = FALSE;
	m_bUnderline2 = FALSE;
	m_bUnderline3 = FALSE;
	m_bUnderline4 = FALSE;
	m_bUnderline5 = FALSE;
	m_bUnderline6 = FALSE;
	m_bUnderline7 = FALSE;
	m_bUnderline8 = FALSE;
	m_bUnderline9 = FALSE;
	//}}AFX_DATA_INIT
  m_page_number = 1;
  m_doc = NULL;

}

CPrefsP20::~CPrefsP20()
{
}

void CPrefsP20::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP20)
	DDX_Check(pDX, IDC_PRINT_BOLD0, m_bBold0);
	DDX_Check(pDX, IDC_PRINT_BOLD1, m_bBold1);
	DDX_Check(pDX, IDC_PRINT_BOLD10, m_bBold10);
	DDX_Check(pDX, IDC_PRINT_BOLD11, m_bBold11);
	DDX_Check(pDX, IDC_PRINT_BOLD12, m_bBold12);
	DDX_Check(pDX, IDC_PRINT_BOLD13, m_bBold13);
	DDX_Check(pDX, IDC_PRINT_BOLD14, m_bBold14);
	DDX_Check(pDX, IDC_PRINT_BOLD15, m_bBold15);
	DDX_Check(pDX, IDC_PRINT_BOLD2, m_bBold2);
	DDX_Check(pDX, IDC_PRINT_BOLD3, m_bBold3);
	DDX_Check(pDX, IDC_PRINT_BOLD4, m_bBold4);
	DDX_Check(pDX, IDC_PRINT_BOLD5, m_bBold5);
	DDX_Check(pDX, IDC_PRINT_BOLD6, m_bBold6);
	DDX_Check(pDX, IDC_PRINT_BOLD7, m_bBold7);
	DDX_Check(pDX, IDC_PRINT_BOLD8, m_bBold8);
	DDX_Check(pDX, IDC_PRINT_BOLD9, m_bBold9);
	DDX_Check(pDX, IDC_PRINT_ITALIC0, m_bItalic0);
	DDX_Check(pDX, IDC_PRINT_ITALIC1, m_bItalic1);
	DDX_Check(pDX, IDC_PRINT_ITALIC10, m_bItalic10);
	DDX_Check(pDX, IDC_PRINT_ITALIC11, m_bItalic11);
	DDX_Check(pDX, IDC_PRINT_ITALIC12, m_bItalic12);
	DDX_Check(pDX, IDC_PRINT_ITALIC13, m_bItalic13);
	DDX_Check(pDX, IDC_PRINT_ITALIC14, m_bItalic14);
	DDX_Check(pDX, IDC_PRINT_ITALIC15, m_bItalic15);
	DDX_Check(pDX, IDC_PRINT_ITALIC2, m_bItalic2);
	DDX_Check(pDX, IDC_PRINT_ITALIC3, m_bItalic3);
	DDX_Check(pDX, IDC_PRINT_ITALIC4, m_bItalic4);
	DDX_Check(pDX, IDC_PRINT_ITALIC5, m_bItalic5);
	DDX_Check(pDX, IDC_PRINT_ITALIC6, m_bItalic6);
	DDX_Check(pDX, IDC_PRINT_ITALIC7, m_bItalic7);
	DDX_Check(pDX, IDC_PRINT_ITALIC8, m_bItalic8);
	DDX_Check(pDX, IDC_PRINT_ITALIC9, m_bItalic9);
	DDX_Check(pDX, IDC_PRINT_UNDERLINE0, m_bUnderline0);
	DDX_Check(pDX, IDC_PRINT_UNDERLINE1, m_bUnderline1);
	DDX_Check(pDX, IDC_PRINT_UNDERLINE10, m_bUnderline10);
	DDX_Check(pDX, IDC_PRINT_UNDERLINE11, m_bUnderline11);
	DDX_Check(pDX, IDC_PRINT_UNDERLINE12, m_bUnderline12);
	DDX_Check(pDX, IDC_PRINT_UNDERLINE13, m_bUnderline13);
	DDX_Check(pDX, IDC_PRINT_UNDERLINE14, m_bUnderline14);
	DDX_Check(pDX, IDC_PRINT_UNDERLINE15, m_bUnderline15);
	DDX_Check(pDX, IDC_PRINT_UNDERLINE2, m_bUnderline2);
	DDX_Check(pDX, IDC_PRINT_UNDERLINE3, m_bUnderline3);
	DDX_Check(pDX, IDC_PRINT_UNDERLINE4, m_bUnderline4);
	DDX_Check(pDX, IDC_PRINT_UNDERLINE5, m_bUnderline5);
	DDX_Check(pDX, IDC_PRINT_UNDERLINE6, m_bUnderline6);
	DDX_Check(pDX, IDC_PRINT_UNDERLINE7, m_bUnderline7);
	DDX_Check(pDX, IDC_PRINT_UNDERLINE8, m_bUnderline8);
	DDX_Check(pDX, IDC_PRINT_UNDERLINE9, m_bUnderline9);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsP20, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP20)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsP20 message handlers

/////////////////////////////////////////////////////////////////////////////
// CPrefsP21 dialog


CPrefsP21::CPrefsP21()
	: CPropertyPage(CPrefsP21::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP21)
	m_name = _T("");
	m_connect_text = _T("");
	m_password = _T("");
	m_connect_now = -1;
	//}}AFX_DATA_INIT
  m_page_number = 1;
  m_doc = NULL;

}


void CPrefsP21::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP21)
	DDX_Control(pDX, IDC_CONNECT_TEXT, m_ctlConnectText);
	DDX_Control(pDX, IDC_LINE_COUNT, m_ctlLineCount);
	DDX_Text(pDX, IDC_CHARACTER, m_name);
	DDX_Text(pDX, IDC_CONNECT_TEXT, m_connect_text);
	DDX_Text(pDX, IDC_PASSWORD, m_password);
	DDX_CBIndex(pDX, IDC_CONNECT_OPTION, m_connect_now);
	//}}AFX_DATA_MAP


   if(pDX->m_bSaveAndValidate)
   {

      m_name.TrimLeft (); // don't let them get away with a single space
      m_name.TrimRight ();

      if(m_connect_now && m_name.IsEmpty ())
          {
          ::TMessageBox("Your character name cannot be blank for auto-connect.");
          DDX_Text(pDX, IDC_CHARACTER, m_name);
          pDX->Fail();
          }     // end of character name being blank

    }   // end of saving and validating
  
}


BEGIN_MESSAGE_MAP(CPrefsP21, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP21)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_LINE_COUNT, OnUpdateLineCount)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsP21 message handlers

LRESULT CPrefsP21::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CPrefsP21::OnKickIdle

void CPrefsP21::OnUpdateLineCount(CCmdUI* pCmdUI)
  {
  CString strText = GetText (m_ctlConnectText);
  int iCount = 0;

  if (!strText.IsEmpty ())
    {
    const char * p = strText;

    // count lines
    for ( ; *p; p++)
      if (*p == '\n')
        iCount++;

    // last line will also end with a newline if it doesn't already
    if (strText.Right (2) != ENDLINE)
        iCount++;
    
    } // end of non-blank text


  pCmdUI->Enable ();
  pCmdUI->SetText  (TFormat ("(%i line%s)", PLURAL (iCount)));
  } // end of CPrefsP21::OnUpdateLineCount

/////////////////////////////////////////////////////////////////////////////
//  Remember when pages become active, so we can return to the same page next time

BOOL CPrefsP1::OnSetActive() 
{
  m_doc->m_last_prefs_page = m_page_number;
	
  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);
	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP2::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP3::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
  LoadSwatches ();

	return CPropertyPage::OnSetActive();
}


BOOL CPrefsP4::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP5::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP6::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP7::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP8::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP9::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

// set up colour swatch for the input echo colour

	FixSwatch (m_ctlInputSwatch, 
             m_ctlInputSwatch2,
             m_ctlInputColour.GetCurSel ());

  m_ctlTextSwatch.m_colour = m_input_text_colour;
  m_ctlBackgroundSwatch.m_colour = m_input_background_colour;

  int i;

  for (i = 0; i < MAX_CUSTOM; i++)
    {
    m_ctlInputColour.m_customtext [i] = m_prefsp3->m_customtext [i];
    m_ctlInputColour.m_customback [i] = m_prefsp3->m_customback [i];
    }

  m_doc->m_last_prefs_page = m_page_number;
	
  // reload combo box now
  int iCurrentColour = m_ctlInputColour.GetCurSel ();
  m_ctlInputColour.ResetContent ();
  m_ctlInputColour.AddString ("(no change)");
  for (i = 0; i < MAX_CUSTOM; i++)
    m_ctlInputColour.AddString (m_prefsp3->m_strCustomName [i]);

  // put the selection back
  m_ctlInputColour.SetCurSel (iCurrentColour);

	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP10::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP11::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP12::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP13::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP14::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP15::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP16::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP17::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
 
  FixSwatch (m_ctlTextSwatch, 
              m_ctlTextSwatch2,
              m_ctlTextColour.GetCurSel ());
	
  int i;

  for (i = 0; i < MAX_CUSTOM; i++)
    {
    m_ctlTextColour.m_customtext [i] = m_prefsp3->m_customtext [i];
    m_ctlTextColour.m_customback [i] = m_prefsp3->m_customback [i];
    }

  int iCurrentColour = m_ctlTextColour.GetCurSel ();

  // load combo box now
  m_ctlTextColour.ResetContent ();
  m_ctlTextColour.AddString ("(default)");
  for (i = 0; i < MAX_CUSTOM; i++)
    m_ctlTextColour.AddString (m_prefsp3->m_strCustomName [i]);

  // put the selection back
  m_ctlTextColour.SetCurSel (iCurrentColour);

	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP18::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP19::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP20::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}

BOOL CPrefsP21::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}


BOOL CPrefsP22::OnSetActive() 
{
  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}


BOOL CPrefsP23::OnSetActive() 
{
  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;

	return CPropertyPage::OnSetActive();
  
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsP0 property page

IMPLEMENT_DYNCREATE(CPrefsP0, CPropertyPage)

CPrefsP0::CPrefsP0() : CPropertyPage(CPrefsP0::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP0)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPrefsP0::~CPrefsP0()
{
}

void CPrefsP0::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP0)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsP0, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP0)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsP0 message handlers

BOOL CPrefsP0::OnSetActive() 
{

  ((CTreePropertySheet *) GetParent ())->PageHasChanged (this);

  m_doc->m_last_prefs_page = m_page_number;
	
	return CPropertyPage::OnSetActive();
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsP22 property page

IMPLEMENT_DYNCREATE(CPrefsP22, CPropertyPage)

CPrefsP22::CPrefsP22() : CPropertyPage(CPrefsP22::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP22)
	m_iUseMXP = -1;
	m_strMXPactive = _T("");
	m_iMXPdebugLevel = -1;
	m_bPueblo = FALSE;
	m_bEchoHyperlinkInOutputWindow = FALSE;
	m_bHyperlinkAddsToCommandHistory = FALSE;
	m_bUseCustomLinkColour = FALSE;
	m_bMudCanChangeLinkColour = FALSE;
	m_bUnderlineHyperlinks = FALSE;
	m_bMudCanRemoveUnderline = FALSE;
	m_bIgnoreMXPcolourChanges = FALSE;
	m_bSendMXP_AFK_Response = FALSE;
	m_bMudCanChangeOptions = FALSE;
	//}}AFX_DATA_INIT
}

CPrefsP22::~CPrefsP22()
{
}

void CPrefsP22::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP22)
	DDX_Control(pDX, IDC_HYPERLINK_SWATCH, m_ctlHyperlinkSwatch);
	DDX_CBIndex(pDX, IDC_MXP_OPTION, m_iUseMXP);
	DDX_Text(pDX, IDC_MXP_ACTIVE, m_strMXPactive);
	DDX_CBIndex(pDX, IDC_MXP_DEBUG_LEVEL, m_iMXPdebugLevel);
	DDX_Check(pDX, IDC_PUEBLO, m_bPueblo);
	DDX_Check(pDX, IDC_ECHO_HYPERLINK_CLICKS, m_bEchoHyperlinkInOutputWindow);
	DDX_Check(pDX, IDC_SAVE_HYPERLINK_CLICKS, m_bHyperlinkAddsToCommandHistory);
	DDX_Check(pDX, IDC_USE_CUSTOM_LINK, m_bUseCustomLinkColour);
	DDX_Check(pDX, IDC_MUD_CAN_CHANGE_LINK_COLOUR, m_bMudCanChangeLinkColour);
	DDX_Check(pDX, IDC_UNDERLINE_HYPERLINKS, m_bUnderlineHyperlinks);
	DDX_Check(pDX, IDC_MUD_CAN_REMOVE_UNDERLINE, m_bMudCanRemoveUnderline);
	DDX_Check(pDX, IDC_IGNORE_COLOUR_CHANGES, m_bIgnoreMXPcolourChanges);
	DDX_Check(pDX, IDC_SEND_AFK_RESPONSE, m_bSendMXP_AFK_Response);
	DDX_Check(pDX, IDC_MUD_CAN_CHANGE_OPTIONS, m_bMudCanChangeOptions);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsP22, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP22)
	ON_BN_CLICKED(IDC_HYPERLINK_SWATCH, OnHyperlinkSwatch)
	ON_BN_CLICKED(IDC_RESET_TAGS, OnResetTags)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsP22 message handlers


BOOL CPrefsP22::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
  m_ctlHyperlinkSwatch.m_colour = m_iHyperlinkColour;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPrefsP22::OnOK() 
{
// remember colour of hyperlink
  
  m_iHyperlinkColour = m_ctlHyperlinkSwatch.m_colour;
	
	CPropertyPage::OnOK();
}

void CPrefsP22::OnHyperlinkSwatch() 
{
  CColourPickerDlg dlg;
  dlg.m_iColour = m_ctlHyperlinkSwatch.m_colour;
  dlg.m_bPickColour = true;

  if (dlg.DoModal () != IDOK)
    return;

  m_ctlHyperlinkSwatch.m_colour = dlg.GetColor ();
  m_ctlHyperlinkSwatch.RedrawWindow();
}

void CPrefsP22::OnResetTags() 
{
  m_doc->MXP_Off ();	
}


/////////////////////////////////////////////////////////////////////////////
// CPrefsP23 property page

IMPLEMENT_DYNCREATE(CPrefsP23, CPropertyPage)

CPrefsP23::CPrefsP23() : CPropertyPage(CPrefsP23::IDD)
{
	//{{AFX_DATA_INIT(CPrefsP23)
	m_strOurChatName = _T("");
	m_bAutoAllowSnooping = FALSE;
	m_bAcceptIncomingChatConnections = FALSE;
	m_IncomingChatPort = 0;
	m_bValidateIncomingCalls = FALSE;
	m_bIgnoreChatColours = FALSE;
	m_strChatMessagePrefix = _T("");
	m_iMaxChatLinesPerMessage = 0;
	m_iMaxChatBytesPerMessage = 0;
	m_strChatSaveDirectory = _T("");
	m_bAutoAllowFiles = FALSE;
	//}}AFX_DATA_INIT
}

CPrefsP23::~CPrefsP23()
{
}

void CPrefsP23::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsP23)
	DDX_Control(pDX, IDC_CHAT_TEXT_SWATCH, m_ctlTextSwatch);
	DDX_Control(pDX, IDC_CHAT_BACK_SWATCH, m_ctlBackSwatch);
	DDX_Text(pDX, IDC_CHAT_NAME, m_strOurChatName);
	DDX_Check(pDX, IDC_AUTO_ACCEPT_SNOOP, m_bAutoAllowSnooping);
	DDX_Check(pDX, IDC_ACCEPT_CALLS, m_bAcceptIncomingChatConnections);
	DDX_Text(pDX, IDC_CHAT_PORT, m_IncomingChatPort);
	DDV_MinMaxLong(pDX, m_IncomingChatPort, 1, 65535);
	DDX_Check(pDX, IDC_VALIDATE_CALLS, m_bValidateIncomingCalls);
	DDX_Check(pDX, IDC_IGNORE_COLOURS, m_bIgnoreChatColours);
	DDX_Text(pDX, IDC_CHAT_PREFIX, m_strChatMessagePrefix);
	DDX_Text(pDX, IDC_MAX_MESSAGE_LINES, m_iMaxChatLinesPerMessage);
	DDV_MinMaxLong(pDX, m_iMaxChatLinesPerMessage, 0, 10000);
	DDX_Text(pDX, IDC_MAX_MESSAGE_BYTES, m_iMaxChatBytesPerMessage);
	DDX_Text(pDX, IDC_CHAT_SAVE_DIRECTORY, m_strChatSaveDirectory);
	DDX_Check(pDX, IDC_AUTO_ACCEPT_FILES, m_bAutoAllowFiles);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsP23, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsP23)
	ON_BN_CLICKED(IDC_CHAT_TEXT_SWATCH, OnChatTextSwatch)
	ON_BN_CLICKED(IDC_CHAT_BACK_SWATCH, OnChatBackSwatch)
	ON_BN_CLICKED(IDC_FILE_DIR_BROWSE, OnFileDirBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsP23 message handlers

void CPrefsP23::OnChatTextSwatch() 
{
  CColourPickerDlg dlg;
  dlg.m_iColour = m_ctlTextSwatch.m_colour;
  dlg.m_bPickColour = true;

  if (dlg.DoModal () != IDOK)
    return;

  m_ctlTextSwatch.m_colour = dlg.GetColor ();
  m_ctlTextSwatch.RedrawWindow();
	
}

void CPrefsP23::OnChatBackSwatch() 
{
  CColourPickerDlg dlg;
  dlg.m_iColour = m_ctlBackSwatch.m_colour;
  dlg.m_bPickColour = true;

  if (dlg.DoModal () != IDOK)
    return;

  m_ctlBackSwatch.m_colour = dlg.GetColor ();
  m_ctlBackSwatch.RedrawWindow();
	
}

void CPrefsP23::OnFileDirBrowse() 
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
		bi.lpszTitle = "Save chat files folder";
		bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
    // if possible, let them create one
    if (!bWine)  
	  	bi.ulFlags |= BIF_NEWDIALOGSTYLE | BIF_EDITBOX;     // requires CoInitialize
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = 0;
  	GetDlgItemText(IDC_CHAT_SAVE_DIRECTORY, strStartingDirectory);

		// This next call issues the dialog box.
		if ((pidl = ::SHBrowseForFolder(&bi)) != NULL)
		{
			if (::SHGetPathFromIDList(pidl, pszBuffer))
				SetDlgItemText(IDC_CHAT_SAVE_DIRECTORY, pszBuffer);

			// Free the PIDL allocated by SHBrowseForFolder.
			pMalloc->Free(pidl);
		}
		// Release the shell's allocator.
		pMalloc->Release();
    }
	
}

void CPrefsP23::OnOK() 
{
// remember colours
  
  m_iTextColour = m_ctlTextSwatch.m_colour;
  m_iBackColour = m_ctlBackSwatch.m_colour;
	
	CPropertyPage::OnOK();
}

BOOL CPrefsP23::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
  m_ctlTextSwatch.m_colour =  m_iTextColour ;
  m_ctlBackSwatch.m_colour =  m_iBackColour ;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



void CPrefsP14::OnAdjustToWidth() 
{
if (m_doc->m_pActiveCommandView || m_doc->m_pActiveOutputView)
  {

  CDC dc;

  dc.CreateCompatibleDC (NULL);

  dc.SelectObject(m_doc->m_font [0]);

  TEXTMETRIC tm;
  dc.GetTextMetrics(&tm);

  RECT rect;

  CChildFrame * pFrame;

  if (m_doc->m_pActiveCommandView)
    pFrame = m_doc->m_pActiveCommandView->m_owner_frame;
  else
    pFrame = m_doc->m_pActiveOutputView->m_owner_frame;

  // find column number they currently have
  CString strColumn = GetText (m_ctlWrapColumn);
  int iColumn = atoi (strColumn);

  if (iColumn < 20)
    iColumn = 20;
  if (iColumn > MAX_LINE_WIDTH)
    iColumn = MAX_LINE_WIDTH;

  // get current window size and position (we only want to change the width)

WINDOWPLACEMENT wp;
  wp.length = sizeof (wp);
  pFrame->GetWindowPlacement (&wp);
  rect = wp.rcNormalPosition;

  // calculate new width
  rect.right = rect.left + 
          (tm.tmAveCharWidth * iColumn) +      // columns wanted
          GetSystemMetrics (SM_CXVSCROLL) +    // scroll bar
          GetSystemMetrics (SM_CXSIZEFRAME) * 2 +  // frame
          GetSystemMetrics (SM_CXBORDER) * 2 +     // border
          m_iPixelOffset +                     // pixel offset requires more room
          5;    // fudge factor, it always seemed to be out by a column

  pFrame->MoveWindow (&rect);

  }
	
}

void CPrefsP14::OnBrowseSounds2() 
{
CString filename;

  CFileDialog filedlg (TRUE,   // loading the file
                       ".wav",    // default extension
                       m_sound_pathname,  // suggested name
                       OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
                       "Waveaudio files (*.wav)|*.wav|"
                       "MIDI files (*.mid)|*.mid|"
                       "Sequencer files (*.rmi)|*.rmi|"
                       "|",    // filter 
                       this);  // parent window

  filedlg.m_ofn.lpstrTitle = "Select sound to play";
  filedlg.m_ofn.lpstrFile = filename.GetBuffer (_MAX_PATH); // needed!! (for Win32s)  

  if (App.platform == VER_PLATFORM_WIN32s)
    strcpy (filedlg.m_ofn.lpstrFile, "");
  else
    strcpy (filedlg.m_ofn.lpstrFile, m_sound_pathname);

  if (m_sound_pathname == NOSOUNDLIT)
    strcpy (filedlg.m_ofn.lpstrFile, "");
    
  ChangeToFileBrowsingDirectory ();
  int nResult = filedlg.DoModal();
  ChangeToStartupDirectory ();

  if (nResult != IDOK)
    return;    // cancelled dialog

  m_sound_pathname = filedlg.GetPathName ();

  GetDlgItem (IDC_SOUND_PATH2)->SetWindowText(m_sound_pathname);

  GetDlgItem (IDC_TEST_SOUND2)->EnableWindow (TRUE);
  GetDlgItem (IDC_NO_SOUND)->EnableWindow (TRUE);	
}

void CPrefsP14::OnTestSound2() 
{
  if (!m_sound_pathname.IsEmpty () && m_sound_pathname != NOSOUNDLIT)
    m_doc->PlaySoundFile (m_sound_pathname);	
}

void CPrefsP14::OnNoSound() 
{
  m_sound_pathname = NOSOUNDLIT;

  GetDlgItem (IDC_SOUND_PATH2)->SetWindowText(m_sound_pathname);

  GetDlgItem (IDC_TEST_SOUND2)->EnableWindow (FALSE);
  GetDlgItem (IDC_NO_SOUND)->EnableWindow (FALSE);
}




void CPrefsP15::OnCalculateMemory() 
{
CalculateMemoryUsage ();	
}


void CPrefsP15::CalculateMemoryUsage ()
  {

  CProgressDlg * pProgressDlg = NULL;

  if (m_doc->m_LineList.GetCount () > 1000)
    {
    pProgressDlg = new CProgressDlg; 
    pProgressDlg->Create ();                           
    pProgressDlg->SetStatus (Translate ("Calculating memory usage..."));               
    pProgressDlg->SetRange (0, m_doc->m_LineList.GetCount ());
    pProgressDlg->SetWindowText (Translate ("Memory used by output buffer"));                              
    }

// work out how much memory each line takes

  long nMemory = 0;
  int iStyles = 0;
  long iCount = 0;

  Frame.SetStatusMessageNow (Translate ("Calculating size of output buffer..."));

  for (POSITION pos = m_doc->m_LineList.GetHeadPosition (); pos; )
    {
    iCount++;

    if (pProgressDlg)
      {
      if ((iCount & 63) == 0)
        pProgressDlg->SetPos (iCount); 

      if (pProgressDlg->CheckCancelButton())     // abort if user cancels
        {
        delete pProgressDlg;
        return;
        }
      }

    CLine * pLine = m_doc->m_LineList.GetNext (pos);
    nMemory += sizeof CLine;          // add the class itself
    nMemory += pLine->iMemoryAllocated;  // and the text

    // count styles and work out how much memory they take too
    for (POSITION pos2 = pLine->styleList.GetHeadPosition(); pos2; iStyles++)
      {
      pLine->styleList.GetNext (pos2);
      nMemory += sizeof CStyle;   // length of style class
      nMemory += sizeof (void *) * 3;   // and the list item
      }

    nMemory += sizeof (void *) * 3;   // and the list item
    }

  m_strBufferLines += TFormat (" (%i styles)", iStyles);
	SetDlgItemText(IDC_BUFFER_LINES, m_strBufferLines);

  CString strMemory, strKb;

  if (nMemory < (1024 * 1024))
    strKb.Format ("%ld Kb", nMemory / 1024);
  else
    strKb.Format ("%5.1f Mb", nMemory / (1024.0 * 1024.0));    // show as Mb

  // at new document time we won't have a line yet
  if (m_doc->m_LineList.GetCount ())
    strMemory.Format ("%s (%i bytes/line)", (LPCTSTR) strKb,
                      nMemory / m_doc->m_LineList.GetCount ());
  else
    strMemory.Format ("%s", (LPCTSTR) strKb);

	SetDlgItemText(IDC_OUTPUT_MEMORY, strMemory);

  m_doc->ShowStatusLine ();
	
  GetDlgItem (IDC_CALCULATE_MEMORY)->EnableWindow (FALSE);  // only do it once


  delete pProgressDlg;

  } // end of  CPrefsP15::CalculateMemoryUsage 


