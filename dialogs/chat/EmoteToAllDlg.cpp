// EmoteToAllDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\mushclient.h"
#include "EmoteToAllDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEmoteToAllDlg dialog


CEmoteToAllDlg::CEmoteToAllDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEmoteToAllDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEmoteToAllDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEmoteToAllDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmoteToAllDlg)
	DDX_MinMaxString(pDX, IDC_EMOTE_TEXT, m_strText);
	DDV_MinMaxString(pDX, m_strText, 1, 1000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmoteToAllDlg, CDialog)
	//{{AFX_MSG_MAP(CEmoteToAllDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmoteToAllDlg message handlers

BOOL CEmoteToAllDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  SetWindowText (m_strTitle);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
