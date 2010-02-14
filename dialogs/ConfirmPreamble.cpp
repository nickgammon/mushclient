// ConfirmPreamble.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "ConfirmPreamble.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfirmPreamble dialog


CConfirmPreamble::CConfirmPreamble(CWnd* pParent /*=NULL*/)
	: CDialog(CConfirmPreamble::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfirmPreamble)
	m_strPasteMessage = _T("");
	m_strPostamble = _T("");
	m_strPreamble = _T("");
	m_strLinePostamble = _T("");
	m_strLinePreamble = _T("");
	m_bCommentedSoftcode = FALSE;
	m_iLineDelay = 0;
	m_bEcho = FALSE;
	m_nLineDelayPerLines = 0;
	//}}AFX_DATA_INIT
}


void CConfirmPreamble::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfirmPreamble)
	DDX_Text(pDX, IDC_PASTE_MESSAGE, m_strPasteMessage);
	DDX_Text(pDX, IDC_FILE_POSTAMBLE, m_strPostamble);
	DDX_Text(pDX, IDC_FILE_PREAMBLE, m_strPreamble);
	DDX_Text(pDX, IDC_LINE_POSTAMBLE, m_strLinePostamble);
	DDX_Text(pDX, IDC_LINE_PREAMBLE, m_strLinePreamble);
	DDX_Check(pDX, IDC_COMMENTED_SOFTCODE, m_bCommentedSoftcode);
	DDX_Text(pDX, IDC_LINE_DELAY, m_iLineDelay);
	DDV_MinMaxLong(pDX, m_iLineDelay, 0, 10000);
	DDX_Check(pDX, IDC_ECHO_LINES, m_bEcho);
	DDX_Text(pDX, IDC_LINE_DELAY_PER_LINES, m_nLineDelayPerLines);
	DDV_MinMaxLong(pDX, m_nLineDelayPerLines, 1, 100000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfirmPreamble, CDialog)
	//{{AFX_MSG_MAP(CConfirmPreamble)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfirmPreamble message handlers
