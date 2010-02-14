// welcome1.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "..\doc.h"
#include "..\StatLink.h"

#include "welcome1.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWelcome1Dlg dialog


CWelcome1Dlg::CWelcome1Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWelcome1Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWelcome1Dlg)
	m_strMessage1 = _T("");
	m_strMessage2 = _T("");
	m_strChangeHistoryAddress = _T("");
	//}}AFX_DATA_INIT
}


void CWelcome1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWelcome1Dlg)
	DDX_Text(pDX, IDC_MESSAGE1, m_strMessage1);
	DDX_Text(pDX, IDC_MESSAGE2, m_strMessage2);
	DDX_Text(pDX, IDC_CHANGES_ADDRESS, m_strChangeHistoryAddress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWelcome1Dlg, CDialog)
	//{{AFX_MSG_MAP(CWelcome1Dlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWelcome1Dlg message handlers

BOOL CWelcome1Dlg::OnInitDialog() 
{
	// subclass static controls.
	m_ChangesLink.SubclassDlgItem(IDC_CHANGES_ADDRESS,  this);
  m_strChangeHistoryAddress = CHANGES_WEB_PAGE;
	
	return CDialog::OnInitDialog();
}
