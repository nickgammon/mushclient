// DebugWorldInputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "DebugWorldInputDlg.h"
#include "InsertUnicodeDlg.h"
#include "..\doc.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDebugWorldInputDlg dialog


CDebugWorldInputDlg::CDebugWorldInputDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDebugWorldInputDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDebugWorldInputDlg)
	m_strText = _T("");
	//}}AFX_DATA_INIT
}


void CDebugWorldInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDebugWorldInputDlg)
	DDX_Control(pDX, IDC_INSERT_SPECIAL, m_ctlSpecials);
	DDX_Control(pDX, IDC_TEXT, m_ctlText);
	DDX_Text(pDX, IDC_TEXT, m_strText);
	//}}AFX_DATA_MAP

}


BEGIN_MESSAGE_MAP(CDebugWorldInputDlg, CDialog)
	//{{AFX_MSG_MAP(CDebugWorldInputDlg)
	ON_BN_CLICKED(IDC_SPECIAL, OnSpecial)
	ON_BN_CLICKED(IDC_INSERT_UNICODE, OnInsertUnicode)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_SPECIAL, OnUpdateNeedSelection)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDebugWorldInputDlg message handlers

BOOL CDebugWorldInputDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
CDC dc;

  dc.CreateCompatibleDC (NULL);

 int lfHeight = -MulDiv(App.m_iFixedPitchFontSize, dc.GetDeviceCaps(LOGPIXELSY), 72);

 m_font.CreateFont(lfHeight, // int nHeight, 
				0, // int nWidth, 
				0, // int nEscapement, 
				0, // int nOrientation, 
				FW_DONTCARE, // int nWeight, 
				0, // BYTE bItalic, 
				0, // BYTE bUnderline, 
        0, // BYTE cStrikeOut, 
        MUSHCLIENT_FONT_CHARSET, // BYTE nCharSet, 
        0, // BYTE nOutPrecision, 
        0, // BYTE nClipPrecision, 
        0, // BYTE nQuality, 
        MUSHCLIENT_FONT_FAMILY, // BYTE nPitchAndFamily,  
        App.m_strFixedPitchFont);// LPCTSTR lpszFacename );

  // Get the metrics of the font.

  dc.SelectObject(&m_font);

  m_ctlText.SendMessage (WM_SETFONT,
                               (WPARAM) m_font.m_hObject,
                               MAKELPARAM (TRUE, 0));

  m_ctlSpecials.AddString ("Backslash - \\\\");
  AddOtherSpecial ("New Line", '\n');
  AddOtherSpecial ("IAC", IAC);
  AddOtherSpecial ("ESC", ESC);
  AddOtherSpecial ("DO", DO);
  AddOtherSpecial ("DONT", DONT);
  AddOtherSpecial ("WILL", WILL);
  AddOtherSpecial ("WONT", WONT);
  AddOtherSpecial ("SB", SB);
  AddOtherSpecial ("SE", SE);

  AddAnsiSpecial ("Reset",                   ANSI_RESET);
  AddAnsiSpecial ("Bold",                    ANSI_BOLD);
  AddAnsiSpecial ("Blink",                   ANSI_BLINK);
  AddAnsiSpecial ("Underline",               ANSI_UNDERLINE);
  AddAnsiSpecial ("Slow Blink",              ANSI_SLOW_BLINK);
  AddAnsiSpecial ("Fast Blink",              ANSI_FAST_BLINK);
  AddAnsiSpecial ("Inverse",                 ANSI_INVERSE);
  AddAnsiSpecial ("Cancel Bold",             ANSI_CANCEL_BOLD);
  AddAnsiSpecial ("Cancel Blink",            ANSI_CANCEL_BLINK);
  AddAnsiSpecial ("Cancel Underline",        ANSI_CANCEL_UNDERLINE);
  AddAnsiSpecial ("Cancel Slow Blink",       ANSI_CANCEL_SLOW_BLINK);
  AddAnsiSpecial ("Cancel Inverse",          ANSI_CANCEL_INVERSE);
  AddAnsiSpecial ("Text Black",              ANSI_TEXT_BLACK);
  AddAnsiSpecial ("Text Red",                ANSI_TEXT_RED);
  AddAnsiSpecial ("Text Green",              ANSI_TEXT_GREEN);
  AddAnsiSpecial ("Text Yellow",             ANSI_TEXT_YELLOW);
  AddAnsiSpecial ("Text Blue",               ANSI_TEXT_BLUE);
  AddAnsiSpecial ("Text Magenta",            ANSI_TEXT_MAGENTA);
  AddAnsiSpecial ("Text Cyan",               ANSI_TEXT_CYAN);
  AddAnsiSpecial ("Text White",              ANSI_TEXT_WHITE);
  AddAnsiSpecial ("Back Black",              ANSI_BACK_BLACK);
  AddAnsiSpecial ("Back Red",                ANSI_BACK_RED);
  AddAnsiSpecial ("Back Green",              ANSI_BACK_GREEN);
  AddAnsiSpecial ("Back Yellow",             ANSI_BACK_YELLOW);
  AddAnsiSpecial ("Back Blue",               ANSI_BACK_BLUE);
  AddAnsiSpecial ("Back Magenta",            ANSI_BACK_MAGENTA);
  AddAnsiSpecial ("Back Cyan",               ANSI_BACK_CYAN);
  AddAnsiSpecial ("Back White",              ANSI_BACK_WHITE);
  AddAnsiSpecial ("Set Foreground Default",  ANSI_SET_FOREGROUND_DEFAULT);
  AddAnsiSpecial ("Set Background Default",  ANSI_SET_BACKGROUND_DEFAULT);

  AddMXPSpecial ("Open",             eMXP_open);
  AddMXPSpecial ("Secure",           eMXP_secure);
  AddMXPSpecial ("Locked",           eMXP_locked);
  AddMXPSpecial ("Reset",            eMXP_reset);
  AddMXPSpecial ("Secure Once",      eMXP_secure_once);
  AddMXPSpecial ("Permanent Open",   eMXP_perm_open);
  AddMXPSpecial ("Permanent Secure", eMXP_perm_secure);
  AddMXPSpecial ("Permanent Locked", eMXP_perm_locked);

  m_ctlSpecials.SetCurSel (0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDebugWorldInputDlg::AddAnsiSpecial (const char * sName, const int iCode)
  {
  m_ctlSpecials.AddString (CFormat ("ANSI %s - \\1B[%im", sName, iCode));
  }   // end of CDebugWorldInputDlg::AddAnsiSpecial

void CDebugWorldInputDlg::AddMXPSpecial (const char * sName, const int iCode)
  {
  m_ctlSpecials.AddString (CFormat ("MXP %s - \\1B[%iz", sName, iCode));
  } // end of CDebugWorldInputDlg::AddMXPSpecial

void CDebugWorldInputDlg::AddOtherSpecial (const char * sName, const int iCode)
  {
  m_ctlSpecials.AddString (CFormat ("%s - \\%02X", sName, iCode));
  } // end of CDebugWorldInputDlg::AddOtherSpecial


void CDebugWorldInputDlg::OnSpecial() 
{
	
int nItem = m_ctlSpecials.GetCurSel ();

  if (nItem == LB_ERR)
    return;

  CString strText;

  m_ctlSpecials.GetLBText(nItem, strText) ;

  int iHyphen = strText.Find ("-");

  if (iHyphen == -1)
    return;

  strText = strText.Mid (iHyphen + 1);
  strText.TrimLeft ();
  strText.TrimRight ();

  m_ctlText.ReplaceSel (strText);

} // end of CDebugWorldInputDlg::OnSpecial

LRESULT CDebugWorldInputDlg::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CChooseNotepadDlg::OnKickIdle

void CDebugWorldInputDlg::OnUpdateNeedSelection(CCmdUI* pCmdUI)
{
int nItem = m_ctlSpecials.GetCurSel ();

	pCmdUI->Enable(nItem != LB_ERR);
}

extern "C" int _pcre_ord2utf8(int cvalue, unsigned char *buffer);

void CDebugWorldInputDlg::OnInsertUnicode() 
{
CInsertUnicodeDlg dlg;

  dlg.m_bHex = true;

  if (dlg.DoModal () != IDOK)
    return;  // cancelled

unsigned char utf8 [10];

int iLen = _pcre_ord2utf8 (dlg.m_iCode, utf8);


CString strText;


for (int i = 0; i < iLen; i++)
  strText += CFormat ("\\%02X", utf8 [i]);

  m_ctlText.ReplaceSel (strText);

}
