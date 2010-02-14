// MXPscriptRoutinesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "MXPscriptRoutinesDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMXPscriptRoutinesDlg dialog


CMXPscriptRoutinesDlg::CMXPscriptRoutinesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMXPscriptRoutinesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMXPscriptRoutinesDlg)
	m_strOnMXP_Start = _T("");
	m_strOnMXP_Stop = _T("");
	m_strOnMXP_OpenTag = _T("");
	m_strOnMXP_CloseTag = _T("");
	m_strOnMXP_SetVariable = _T("");
	m_strOnMXP_Error = _T("");
	//}}AFX_DATA_INIT
}


void CMXPscriptRoutinesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMXPscriptRoutinesDlg)
	DDX_Text(pDX, IDC_ON_MXP_STARTUP, m_strOnMXP_Start);
	DDX_Text(pDX, IDC_ON_MXP_SHUTDOWN, m_strOnMXP_Stop);
	DDX_Text(pDX, IDC_ON_MXP_OPENING_TAG, m_strOnMXP_OpenTag);
	DDX_Text(pDX, IDC_ON_MXP_CLOSING_TAG, m_strOnMXP_CloseTag);
	DDX_Text(pDX, IDC_ON_MXP_SET_VARIABLE, m_strOnMXP_SetVariable);
	DDX_Text(pDX, IDC_ON_MXP_ERROR, m_strOnMXP_Error);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMXPscriptRoutinesDlg, CDialog)
	//{{AFX_MSG_MAP(CMXPscriptRoutinesDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMXPscriptRoutinesDlg message handlers
