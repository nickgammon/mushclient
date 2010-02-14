// UniqueIDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "UniqueIDDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUniqueIDDlg dialog


CUniqueIDDlg::CUniqueIDDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUniqueIDDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUniqueIDDlg)
	m_strUniqueID = _T("");
	//}}AFX_DATA_INIT
}


void CUniqueIDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUniqueIDDlg)
	DDX_Text(pDX, IDC_ID, m_strUniqueID);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUniqueIDDlg, CDialog)
	//{{AFX_MSG_MAP(CUniqueIDDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUniqueIDDlg message handlers
