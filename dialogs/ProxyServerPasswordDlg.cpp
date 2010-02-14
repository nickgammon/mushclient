// ProxyServerPasswordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "ProxyServerPasswordDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProxyServerPasswordDlg dialog


CProxyServerPasswordDlg::CProxyServerPasswordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProxyServerPasswordDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProxyServerPasswordDlg)
	m_strProxyUserName = _T("");
	m_strProxyPassword = _T("");
	//}}AFX_DATA_INIT
}


void CProxyServerPasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProxyServerPasswordDlg)
	DDX_Text(pDX, IDC_LOGON_NAME, m_strProxyUserName);
	DDX_Text(pDX, IDC_LOGON_PASSWORD, m_strProxyPassword);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProxyServerPasswordDlg, CDialog)
	//{{AFX_MSG_MAP(CProxyServerPasswordDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProxyServerPasswordDlg message handlers
