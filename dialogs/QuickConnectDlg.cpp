// QuickConnectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "QuickConnectDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQuickConnectDlg dialog


CQuickConnectDlg::CQuickConnectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQuickConnectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQuickConnectDlg)
	m_iPort = 0;
	//}}AFX_DATA_INIT
}


void CQuickConnectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQuickConnectDlg)
	DDX_MinMaxString(pDX, IDC_MUSH_NAME, m_strWorldName);
	DDV_MinMaxString(pDX, m_strWorldName, 1, 255);
	DDX_MinMaxString(pDX, IDC_SERVER, m_strAddress);
	DDV_MinMaxString(pDX, m_strAddress, 1, 255);
	DDX_Text(pDX, IDC_CHANNEL, m_iPort);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQuickConnectDlg, CDialog)
	//{{AFX_MSG_MAP(CQuickConnectDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQuickConnectDlg message handlers
