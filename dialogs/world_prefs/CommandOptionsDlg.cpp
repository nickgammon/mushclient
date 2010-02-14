// CommandOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\mushclient.h"
#include "CommandOptionsDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCommandOptionsDlg dialog


CCommandOptionsDlg::CCommandOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCommandOptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCommandOptionsDlg)
	m_bDoubleClickInserts = FALSE;
	m_bDoubleClickSends = FALSE;
	m_bArrowKeysWrap = FALSE;
	m_bArrowsChangeHistory = FALSE;
	m_bArrowRecallsPartial = FALSE;
	m_bAltArrowRecallsPartial = FALSE;
	m_bEscapeDeletesInput = FALSE;
	m_bSaveDeletedCommand = FALSE;
	m_bConfirmBeforeReplacingTyping = FALSE;
	m_bCtrlZGoesToEndOfBuffer = FALSE;
	m_bCtrlPGoesToPreviousCommand = FALSE;
	m_bCtrlNGoesToNextCommand = FALSE;
	//}}AFX_DATA_INIT
}


void CCommandOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommandOptionsDlg)
	DDX_Control(pDX, IDC_ESCAPE_DELETES_TYPING, m_ctlEscapeDeletesTypeing);
	DDX_Control(pDX, IDC_DOUBLE_CLICK_SENDS, m_ctlDoubleClickSends);
	DDX_Control(pDX, IDC_DOUBLE_CLICK_INSERTS, m_ctlDoubleClickPastesWord);
	DDX_Check(pDX, IDC_DOUBLE_CLICK_INSERTS, m_bDoubleClickInserts);
	DDX_Check(pDX, IDC_DOUBLE_CLICK_SENDS, m_bDoubleClickSends);
	DDX_Check(pDX, IDC_ARROWS_WRAP, m_bArrowKeysWrap);
	DDX_Check(pDX, IDC_ARROW_KEYS_TRAVERSE_HISTORY, m_bArrowsChangeHistory);
	DDX_Check(pDX, IDC_ARROW_KEYS_RECALL_PARTIAL_COMMAND, m_bArrowRecallsPartial);
	DDX_Check(pDX, IDC_RECALL_PARTIAL, m_bAltArrowRecallsPartial);
	DDX_Check(pDX, IDC_ESCAPE_DELETES_TYPING, m_bEscapeDeletesInput);
	DDX_Check(pDX, IDC_SAVE_DELETED, m_bSaveDeletedCommand);
	DDX_Check(pDX, IDC_CONFIRM_REPLACE_TYPING, m_bConfirmBeforeReplacingTyping);
	DDX_Check(pDX, IDC_CTRL_Z_END_WINDOW, m_bCtrlZGoesToEndOfBuffer);
	DDX_Check(pDX, IDC_CTRL_P_PREVIOUS_COMMAND, m_bCtrlPGoesToPreviousCommand);
	DDX_Check(pDX, IDC_CTRL_N_PREVIOUS_COMMAND, m_bCtrlNGoesToNextCommand);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCommandOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(CCommandOptionsDlg)
	ON_BN_CLICKED(IDC_DOUBLE_CLICK_SENDS, OnDoubleClickSends)
	ON_BN_CLICKED(IDC_DOUBLE_CLICK_INSERTS, OnDoubleClickInserts)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_SAVE_DELETED, OnUpdateNeedEscapeDeletesTyping)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCommandOptionsDlg message handlers

LRESULT CCommandOptionsDlg::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CCommandOptionsDlg::OnKickIdle

void CCommandOptionsDlg::OnUpdateNeedEscapeDeletesTyping(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_ctlEscapeDeletesTypeing.GetCheck ());
  } // end of CCommandOptionsDlg::OnUpdateNeedEscapeDeletesTyping

BOOL CCommandOptionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  m_ctlDoubleClickPastesWord.EnableWindow (!m_bDoubleClickSends);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCommandOptionsDlg::OnDoubleClickSends() 
{
m_ctlDoubleClickPastesWord.EnableWindow (!m_ctlDoubleClickSends.GetCheck ());	

}

void CCommandOptionsDlg::OnDoubleClickInserts() 
{
m_ctlDoubleClickSends.EnableWindow (!m_ctlDoubleClickPastesWord.GetCheck ());	

}
