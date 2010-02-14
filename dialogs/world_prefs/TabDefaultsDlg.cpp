// TabDefaultsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\mushclient.h"
#include "TabDefaultsDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabDefaultsDlg dialog


CTabDefaultsDlg::CTabDefaultsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTabDefaultsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabDefaultsDlg)
	m_strWordList = _T("");
	m_iTabCompletionLines = 0;
	m_bTabCompletionSpace = FALSE;
	//}}AFX_DATA_INIT
}


void CTabDefaultsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabDefaultsDlg)
	DDX_Text(pDX, IDC_WORD_LIST, m_strWordList);
	DDX_Text(pDX, IDC_TAB_COMPLETION_LINES, m_iTabCompletionLines);
	DDV_MinMaxUInt(pDX, m_iTabCompletionLines, 1, 500000);
	DDX_Check(pDX, IDC_TAB_COMPLETION_SPACE, m_bTabCompletionSpace);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabDefaultsDlg, CDialog)
	//{{AFX_MSG_MAP(CTabDefaultsDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabDefaultsDlg message handlers
