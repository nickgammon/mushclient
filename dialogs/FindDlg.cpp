// FindDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "FindDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFindDlg dialog


CFindDlg::CFindDlg(CStringList & strFindStringList,
                   CWnd* pParent /*=NULL*/)
	: CDialog(CFindDlg::IDD, pParent), m_strFindStringList (strFindStringList)
{
	//{{AFX_DATA_INIT(CFindDlg)
	m_bMatchCase = FALSE;
	m_bForwards = -1;
	m_bRegexp = FALSE;
	m_strFindText = _T("");
	//}}AFX_DATA_INIT
}


void CFindDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindDlg)
	DDX_Control(pDX, IDC_FIND_WHAT, m_ctlFindText);
	DDX_Check(pDX, IDC_MATCH_CASE, m_bMatchCase);
	DDX_Radio(pDX, IDC_FIND_UP, m_bForwards);
	DDX_Check(pDX, IDC_REGEXP, m_bRegexp);
	DDX_CBString(pDX, IDC_FIND_WHAT, m_strFindText);
	//}}AFX_DATA_MAP

 if(pDX->m_bSaveAndValidate)
   {

    if (m_strFindText.IsEmpty ())
      {
      ::TMessageBox("You must specify something to search for.");
      DDX_Text(pDX, IDC_FIND_WHAT, m_strFindText);
      pDX->Fail();
      }

    // compile regular expression to check it 

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


BEGIN_MESSAGE_MAP(CFindDlg, CDialog)
	//{{AFX_MSG_MAP(CFindDlg)
	ON_BN_CLICKED(IDC_REGEXP_HELP, OnRegexpHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindDlg message handlers

BOOL CFindDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  SetWindowText (m_strTitle);     // change find dialog title
	
  for (POSITION pos = m_strFindStringList.GetHeadPosition (); pos; )
    m_ctlFindText.AddString (m_strFindStringList.GetNext (pos));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFindDlg::OnRegexpHelp() 
{
  ShowHelp ("DOC_", "regexp")	;
}
