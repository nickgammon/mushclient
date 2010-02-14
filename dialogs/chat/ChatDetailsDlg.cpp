// ChatDetailsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\mushclient.h"
#include "ChatDetailsDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChatDetailsDlg dialog


CChatDetailsDlg::CChatDetailsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChatDetailsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChatDetailsDlg)
	m_strRemoteUserName = _T("");
	m_strGroup = _T("");
	m_strChatID = _T("");
	m_strRemoteVersion = _T("");
	m_bIncoming = FALSE;
	m_bCanSnoop = FALSE;
	m_bCanSendFiles = FALSE;
	m_bPrivate = FALSE;
	m_bIgnore = FALSE;
	m_bCanSendCommands = FALSE;
	m_strAllegedAddress = _T("");
	m_strAllegedPort = _T("");
	m_strActualIPaddress = _T("");
	//}}AFX_DATA_INIT
}


void CChatDetailsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChatDetailsDlg)
	DDX_Text(pDX, IDC_CHATNAME, m_strRemoteUserName);
	DDX_Text(pDX, IDC_GROUP, m_strGroup);
	DDX_Text(pDX, IDC_CHAT_ID, m_strChatID);
	DDX_Text(pDX, IDC_CLIENT, m_strRemoteVersion);
	DDX_Check(pDX, IDC_INCOMING, m_bIncoming);
	DDX_Check(pDX, IDC_CAN_SNOOP, m_bCanSnoop);
	DDX_Check(pDX, IDC_CAN_SEND_FILES, m_bCanSendFiles);
	DDX_Check(pDX, IDC_PRIVATE, m_bPrivate);
	DDX_Check(pDX, IDC_IGNORE, m_bIgnore);
	DDX_Check(pDX, IDC_CAN_SEND_COMMANDS, m_bCanSendCommands);
	DDX_Text(pDX, IDC_CALL_IP, m_strAllegedAddress);
	DDX_Text(pDX, IDC_PORT, m_strAllegedPort);
	DDX_Text(pDX, IDC_ACTUAL_IP, m_strActualIPaddress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChatDetailsDlg, CDialog)
	//{{AFX_MSG_MAP(CChatDetailsDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChatDetailsDlg message handlers
