// ChatToAllDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\mushclient.h"
#include "ChatToAllDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChatToAllDlg dialog


CChatToAllDlg::CChatToAllDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChatToAllDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChatToAllDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChatToAllDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChatToAllDlg)
	DDX_MinMaxString(pDX, IDC_SAY_TEXT, m_strText);
	DDV_MinMaxString(pDX, m_strText, 1, 1000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChatToAllDlg, CDialog)
	//{{AFX_MSG_MAP(CChatToAllDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChatToAllDlg message handlers

BOOL CChatToAllDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  SetWindowText (m_strTitle);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
