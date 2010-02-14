// WinsockInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "WinsockInfoDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWinsockInfoDlg dialog


CWinsockInfoDlg::CWinsockInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWinsockInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWinsockInfoDlg)
	m_szDescription = _T("");
	m_szSystemStatus = _T("");
	m_iMaxSockets = 0;
	m_strVersion = _T("");
	m_strHighVersion = _T("");
	m_strHostName = _T("");
	m_strAddresses = _T("");
	//}}AFX_DATA_INIT
}


void CWinsockInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWinsockInfoDlg)
	DDX_Text(pDX, IDC_DESCRIPTION, m_szDescription);
	DDX_Text(pDX, IDC_STATUS, m_szSystemStatus);
	DDX_Text(pDX, IDC_MAX_SOCKETS, m_iMaxSockets);
	DDX_Text(pDX, IDC_VERSION, m_strVersion);
	DDX_Text(pDX, IDC_HIGH_VERSION, m_strHighVersion);
	DDX_Text(pDX, IDC_HOST_NAME, m_strHostName);
	DDX_Text(pDX, IDC_ADDRESSES, m_strAddresses);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWinsockInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CWinsockInfoDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinsockInfoDlg message handlers
