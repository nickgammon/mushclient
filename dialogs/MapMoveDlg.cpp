// MapMoveDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "MapMoveDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMapMoveDlg dialog


CMapMoveDlg::CMapMoveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMapMoveDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapMoveDlg)
	m_strAction = _T("");
	m_strReverse = _T("");
	m_bSendToMUD = FALSE;
	//}}AFX_DATA_INIT
}


void CMapMoveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapMoveDlg)
	DDX_Control(pDX, IDC_REVERSE, m_ctlReverse);
	DDX_Control(pDX, IDC_ACTION, m_ctlAction);
	DDX_Text(pDX, IDC_ACTION, m_strAction);
	DDX_Text(pDX, IDC_REVERSE, m_strReverse);
	DDX_Check(pDX, IDC_SEND_TO_MUD, m_bSendToMUD);
	//}}AFX_DATA_MAP

    if(pDX->m_bSaveAndValidate)
     {

     m_strAction.TrimLeft ();
     m_strAction.TrimRight ();
     m_strReverse.TrimLeft ();
     m_strReverse.TrimRight ();

     int i;
      if ((i = m_strAction.FindOneOf ("{}()/\\")) != -1)
        {
        ::UMessageBox(TFormat ("The action may not contain the character \"%c\"",
                         m_strAction [i]));
        DDX_Text(pDX, IDC_ACTION, m_strAction);
        pDX->Fail();
        }   // end of bad character
      if ((i = m_strReverse.FindOneOf ("{}()/\\")) != -1)
        {
        ::UMessageBox(TFormat ("The reverse action may not contain the character \"%c\"",
                         m_strReverse [i]));
        DDX_Text(pDX, IDC_REVERSE, m_strReverse);
        pDX->Fail();
        }   // end of bad character
     }     // end of save and validate
   
  }


BEGIN_MESSAGE_MAP(CMapMoveDlg, CDialog)
	//{{AFX_MSG_MAP(CMapMoveDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDOK, OnUpdateNeedAction)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapMoveDlg message handlers

LRESULT CMapMoveDlg::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CChooseNotepadDlg::OnKickIdle

void CMapMoveDlg::OnUpdateNeedAction(CCmdUI* pCmdUI)
{

	pCmdUI->Enable(!GetText (m_ctlAction).IsEmpty () ||
                 !GetText (m_ctlReverse).IsEmpty ());
}

