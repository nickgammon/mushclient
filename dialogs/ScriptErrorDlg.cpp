// ScriptErrorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "ScriptErrorDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScriptErrorDlg dialog


CScriptErrorDlg::CScriptErrorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScriptErrorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScriptErrorDlg)
	m_iError = 0;
	m_strEvent = _T("");
	m_strDescription = _T("");
	m_strCalledBy = _T("");
	m_strRaisedBy = _T("");
	m_bUseOutputWindow = FALSE;
	//}}AFX_DATA_INIT
  m_font = NULL;
  m_bHaveDoc = false;
}


void CScriptErrorDlg::DoDataExchange(CDataExchange* pDX)
{

  if (!pDX->m_bSaveAndValidate)
    {
    // get rid of carriage-returns
    m_strDescription = Replace (m_strDescription, "\r", "");
    // make newlinse cr/nl
    m_strDescription = Replace (m_strDescription, "\n", "\r\n");

    }

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScriptErrorDlg)
	DDX_Control(pDX, IDC_EDIT1, m_ctlRaisedByString);
	DDX_Control(pDX, IDC_RAISED_BY_LABEL, m_ctlRaisedByLabel);
	DDX_Control(pDX, IDC_USE_OUTPUT_WINDOW, m_ctlUseOutputWindow);
	DDX_Control(pDX, IDC_DESCRIPTION, m_ctlDescription);
	DDX_Text(pDX, IDC_ERROR_NUMBER, m_iError);
	DDX_Text(pDX, IDC_EVENT, m_strEvent);
	DDX_Text(pDX, IDC_DESCRIPTION, m_strDescription);
	DDX_Text(pDX, IDC_CALLED_BY, m_strCalledBy);
	DDX_Text(pDX, IDC_EDIT1, m_strRaisedBy);
	DDX_Check(pDX, IDC_USE_OUTPUT_WINDOW, m_bUseOutputWindow);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScriptErrorDlg, CDialog)
	//{{AFX_MSG_MAP(CScriptErrorDlg)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScriptErrorDlg message handlers

void CScriptErrorDlg::OnCopy() 
{

  putontoclipboard (Replace (TFormat ("Error number: %i\n" 
                            "Event:        %s\n" 
                            "Description:  %s\n" 
                            "Called by:    %s\n" ,
                            m_iError,
                            (LPCTSTR) m_strEvent,
                            (LPCTSTR) m_strDescription,
                            (LPCTSTR) m_strCalledBy), "\n", "\r\n"));

}

BOOL CScriptErrorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  FixFont (m_font, m_ctlDescription, App.m_strFixedPitchFont, App.m_iFixedPitchFontSize, FW_NORMAL, DEFAULT_CHARSET);
	
  if (!m_bHaveDoc)
    {
    m_ctlUseOutputWindow.ShowWindow (SW_HIDE);
    m_ctlRaisedByString.ShowWindow (SW_HIDE);
    m_ctlRaisedByLabel.ShowWindow (SW_HIDE);
    }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
