// KeyNameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "KeyNameDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeyNameDlg dialog


CKeyNameDlg::CKeyNameDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKeyNameDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CKeyNameDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CKeyNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKeyNameDlg)
	DDX_Control(pDX, IDC_HOTKEY_VALUE, m_ctlHotkeyValue);
	DDX_Control(pDX, IDC_TEXT_NAME, m_ctlTextName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CKeyNameDlg, CDialog)
	//{{AFX_MSG_MAP(CKeyNameDlg)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_TEXT_NAME, OnUpdateKeyName)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyNameDlg message handlers


LRESULT CKeyNameDlg::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CKeyNameDlg::OnKickIdle

void CKeyNameDlg::OnUpdateKeyName(CCmdUI* pCmdUI)
  {
  WORD wVirtualKeyCode;
  WORD wModifiers;

  m_ctlHotkeyValue.GetHotKey (wVirtualKeyCode, wModifiers);

  BYTE fVirt = 0;

  if (wModifiers & HOTKEYF_ALT)
    fVirt |= FALT;
  if (wModifiers & HOTKEYF_CONTROL)
    fVirt |= FCONTROL;
  if (wModifiers & HOTKEYF_SHIFT)
    fVirt |= FSHIFT;

  pCmdUI->SetText (KeyCodeToString (fVirt, wVirtualKeyCode));
  
  } // end of CKeyNameDlg::OnUpdateKeyName

