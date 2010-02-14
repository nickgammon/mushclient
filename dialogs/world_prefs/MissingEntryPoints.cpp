// MissingEntryPoints.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\mushclient.h"
#include "MissingEntryPoints.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMissingEntryPoints dialog


CMissingEntryPoints::CMissingEntryPoints(CWnd* pParent /*=NULL*/)
	: CDialog(CMissingEntryPoints::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMissingEntryPoints)
	m_strErrorMessage = _T("");
	//}}AFX_DATA_INIT
}


void CMissingEntryPoints::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMissingEntryPoints)
	DDX_Text(pDX, IDC_ERROR_LIST, m_strErrorMessage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMissingEntryPoints, CDialog)
	//{{AFX_MSG_MAP(CMissingEntryPoints)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMissingEntryPoints message handlers
