// ChatCallDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\mushclient.h"
#include "ChatCallDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChatCallDlg dialog


CChatCallDlg::CChatCallDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChatCallDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChatCallDlg)
	m_iPort = 0;
	m_bzChat = FALSE;
	//}}AFX_DATA_INIT
}


void CChatCallDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChatCallDlg)
	DDX_Text(pDX, IDC_PORT, m_iPort);
	DDV_MinMaxLong(pDX, m_iPort, 1, 65535);
	DDX_MinMaxString(pDX, IDC_IP_ADDRESS, m_strAddress);
	DDV_MinMaxString(pDX, m_strAddress, 1, 512);
	DDX_Check(pDX, IDC_ZCHAT, m_bzChat);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChatCallDlg, CDialog)
	//{{AFX_MSG_MAP(CChatCallDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChatCallDlg message handlers
