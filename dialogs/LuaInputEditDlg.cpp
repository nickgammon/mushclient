// LuaInputEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "LuaInputEditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLuaInputEditDlg dialog

#define CLEAR_SELECTION 10200

CLuaInputEditDlg::CLuaInputEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLuaInputEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLuaInputEditDlg)
	m_strMessage = _T("");
	m_strReply = _T("");
	//}}AFX_DATA_INIT

  m_font = NULL;

}


void CLuaInputEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLuaInputEditDlg)
	DDX_Control(pDX, IDCANCEL, m_ctlCancel);
	DDX_Control(pDX, IDOK, m_ctlOK);
	DDX_Control(pDX, IDC_INPUT_BOX_REPLY, m_ctlReply);
	DDX_Text(pDX, IDC_INPUT_BOX_MESSAGE, m_strMessage);
	DDX_Text(pDX, IDC_INPUT_BOX_REPLY, m_strReply);
	//}}AFX_DATA_MAP


 if(!pDX->m_bSaveAndValidate)
   {
   if (!m_strFont.IsEmpty () && m_iFontSize > 3)
      FixFont (m_font, m_ctlReply, m_strFont, m_iFontSize, FW_NORMAL, DEFAULT_CHARSET);
   }

}


BEGIN_MESSAGE_MAP(CLuaInputEditDlg, CDialog)
	//{{AFX_MSG_MAP(CLuaInputEditDlg)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP

  ON_COMMAND(CLEAR_SELECTION, OnRemoveSelection)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLuaInputEditDlg message handlers

BOOL CLuaInputEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  SetWindowText (m_strTitle);	
	
  PostMessage (WM_COMMAND, CLEAR_SELECTION);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLuaInputEditDlg::OnRemoveSelection()
  {

  m_ctlReply.SetSel (m_strReply.GetLength (), m_strReply.GetLength ());

  }

void CLuaInputEditDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
  if (m_ctlReply.m_hWnd && m_ctlCancel.m_hWnd && m_ctlOK)
    {
    // move OK and Cancel buttons
    WINDOWPLACEMENT wndpl;
    int iHeight;
    int iWidth;
    int iBorder = 10;

    // -----------------------
    // where is OK button?
    GetButtonSize (m_ctlOK, iHeight, iWidth);

    // move to near bottom

    m_ctlOK.MoveWindow (iBorder, cy - iHeight - 10, iWidth, iHeight);

    // -----------------------
    // where is Cancel button?
    GetButtonSize (m_ctlCancel, iHeight, iWidth);

    // move to near bottom

    m_ctlCancel.MoveWindow (cx - iWidth - iBorder, cy - iHeight - 10, iWidth, iHeight);

    // -----------------------
    // where is Cancel button now?
    m_ctlCancel.GetWindowPlacement (&wndpl);

    const int iTop = 32;

    // move text to just above it
	  m_ctlReply.MoveWindow(iBorder, iTop, cx - (iBorder * 2), wndpl.rcNormalPosition.top - 10 - iTop);
    }
	
}
