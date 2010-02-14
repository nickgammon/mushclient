// logdlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MUSHclient.h"
#include "..\doc.h"
#include "logdlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogDlg dialog


CLogDlg::CLogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLogDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLogDlg)
	m_lines = 0;
	m_bAppendToLogFile = FALSE;
	m_bWriteWorldName = FALSE;
	m_strPreamble = _T("");
	m_bLogNotes = FALSE;
	m_bLogInput = FALSE;
	m_bLogOutput = FALSE;
	//}}AFX_DATA_INIT
}


void CLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLogDlg)
	DDX_Text(pDX, IDC_LINES, m_lines);
	DDV_MinMaxLong(pDX, m_lines, 0, 500000);
	DDX_Check(pDX, IDC_APPEND_TO_LOG_FILE, m_bAppendToLogFile);
	DDX_Check(pDX, IDC_WRITE_WORLD_NAME, m_bWriteWorldName);
	DDX_Text(pDX, IDC_LOG_FILE_PREAMBLE, m_strPreamble);
	DDX_Check(pDX, IDC_LOG_NOTES, m_bLogNotes);
	DDX_Check(pDX, IDC_LOG_INPUT, m_bLogInput);
	DDX_Check(pDX, IDC_LOG_OUTPUT, m_bLogOutput);
	//}}AFX_DATA_MAP

  if (pDX->m_bSaveAndValidate)
    {
    // check if output is not logged, sounds strange.
    if (!m_bLogOutput)
      if (::TMessageBox ("You are not logging output from the MUD - is this intentional?", 
              MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) 
          != IDYES)
         pDX->Fail();  

    } // end of save and validate
}


BEGIN_MESSAGE_MAP(CLogDlg, CDialog)
	//{{AFX_MSG_MAP(CLogDlg)
	ON_BN_CLICKED(IDC_HELPBUTTON, OnHelpbutton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLogDlg message handlers

void CLogDlg::OnHelpbutton() 
{
//	  OnCmdMsg(ID_HELP, CN_COMMAND, NULL, NULL);	
	App.WinHelp(m_nIDHelp + HID_BASE_COMMAND);

}
