// RecallSearchDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "RecallSearchDlg.h"
#include "CreditsDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRecallSearchDlg dialog


CRecallSearchDlg::CRecallSearchDlg(CStringList & strFindStringList,
                                  CWnd* pParent /*=NULL*/)
	: CDialog(CRecallSearchDlg::IDD, pParent), m_strFindStringList (strFindStringList)
{
	//{{AFX_DATA_INIT(CRecallSearchDlg)
	m_bMatchCase = FALSE;
	m_bRegexp = FALSE;
	m_strFindText = _T("");
	m_iLines = 0;
	m_bOutput = FALSE;
	m_bCommands = FALSE;
	m_bNotes = FALSE;
	m_strRecallLinePreamble = _T("");
	//}}AFX_DATA_INIT
}


void CRecallSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRecallSearchDlg)
	DDX_Control(pDX, IDC_FIND_WHAT, m_ctlFindText);
	DDX_Check(pDX, IDC_MATCH_CASE, m_bMatchCase);
	DDX_Check(pDX, IDC_REGEXP, m_bRegexp);
	DDX_CBString(pDX, IDC_FIND_WHAT, m_strFindText);
	DDX_Text(pDX, IDC_LINE_COUNT, m_iLines);
	DDX_Check(pDX, IDC_OUTPUT, m_bOutput);
	DDX_Check(pDX, IDC_COMMANDS, m_bCommands);
	DDX_Check(pDX, IDC_NOTES, m_bNotes);
	DDX_Text(pDX, IDC_EDIT1, m_strRecallLinePreamble);
	//}}AFX_DATA_MAP

 if(pDX->m_bSaveAndValidate)
   {

    if (m_strFindText.IsEmpty () ||
      !(m_bOutput || m_bCommands || m_bNotes))
      {
      ::TMessageBox("You must specify something to search for.");
      DDX_Text(pDX, IDC_FIND_WHAT, m_strFindText);
      pDX->Fail();
      }

    // compile regular expression to check it - will raise exception if bad

    if (m_bRegexp)
      {
      if (!CheckRegularExpression (m_strFindText, 0))
        {   // failed check
        DDX_Text(pDX, IDC_FIND_WHAT, m_strFindText);
        pDX->Fail();
        }
      } // end of checking regular expression

   } // end of saving and validating

}


BEGIN_MESSAGE_MAP(CRecallSearchDlg, CDialog)
	//{{AFX_MSG_MAP(CRecallSearchDlg)
	ON_BN_CLICKED(IDC_SUBSITUTION_HELP, OnSubsitutionHelp)
	ON_BN_CLICKED(IDC_REGEXP_HELP, OnRegexpHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecallSearchDlg message handlers

BOOL CRecallSearchDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  SetWindowText (m_strTitle);     // change find dialog title
	
  for (POSITION pos = m_strFindStringList.GetHeadPosition (); pos; )
    m_ctlFindText.AddString (m_strFindStringList.GetNext (pos));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRecallSearchDlg::OnSubsitutionHelp() 
{
CCreditsDlg dlg;

  dlg.m_iResourceID = IDR_SUBSTITUTION;
  dlg.m_strTitle = "Special characters";

  dlg.DoModal ();	
	
}

void CRecallSearchDlg::OnRegexpHelp() 
{
  ShowHelp ("DOC_", "regexp")	;	
}
