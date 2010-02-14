// welcome.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "..\doc.h"
#include "..\StatLink.h"

#include "welcome.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWelcomeDlg dialog


CWelcomeDlg::CWelcomeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWelcomeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWelcomeDlg)
	m_strMessage = _T("");
	m_strChangeHistoryAddress = _T("");
	//}}AFX_DATA_INIT
}


void CWelcomeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWelcomeDlg)
	DDX_Text(pDX, IDC_MESSAGE, m_strMessage);
	DDX_Text(pDX, IDC_CHANGES_ADDRESS, m_strChangeHistoryAddress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWelcomeDlg, CDialog)
	//{{AFX_MSG_MAP(CWelcomeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWelcomeDlg message handlers

BOOL CWelcomeDlg::OnInitDialog() 
{
	// subclass static controls.
	m_ChangesLink.SubclassDlgItem(IDC_CHANGES_ADDRESS,  this);
 
  m_strChangeHistoryAddress = CHANGES_WEB_PAGE;

	return CDialog::OnInitDialog();
}
