// MultiLineTriggerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "..\doc.h"
#include "MultiLineTriggerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CLEAR_SELECTION 10200

/////////////////////////////////////////////////////////////////////////////
// CMultiLineTriggerDlg dialog


CMultiLineTriggerDlg::CMultiLineTriggerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMultiLineTriggerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMultiLineTriggerDlg)
	m_strTriggerText = _T("");
	m_bMatchCase = FALSE;
	//}}AFX_DATA_INIT

  m_font = NULL;

}


void CMultiLineTriggerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiLineTriggerDlg)
	DDX_Control(pDX, IDC_TRIGGER_TEXT, m_ctlTriggerText);
	DDX_Text(pDX, IDC_TRIGGER_TEXT, m_strTriggerText);
	DDX_Check(pDX, IDC_MATCH_CASE, m_bMatchCase);
	//}}AFX_DATA_MAP

   if(pDX->m_bSaveAndValidate)
     {

    // can't make an empty trigger
    if (m_strTriggerText.IsEmpty ())
      {
      ::TMessageBox("The trigger match text cannot be empty.", MB_ICONSTOP);
      DDX_Text(pDX, IDC_TRIGGER_TEXT, m_strTriggerText);
      pDX->Fail();
      }     // end of text being blank

    int iCount = 1;

    const char * p;

    for (p = m_strTriggerText; *p; p++)
      if (*p == '\n')
        iCount++;

    if (iCount < 1)
      {
      ::TMessageBox("Multi-line triggers must match at least 2 lines.", MB_ICONSTOP);
      DDX_Text(pDX, IDC_TRIGGER_TEXT, m_strTriggerText);
      pDX->Fail();
      }     // end of text being blank

    if (iCount > MAX_RECENT_LINES)
      {
      ::UMessageBox(
        TFormat ("Multi-line triggers can match a maximum of %i lines.", MAX_RECENT_LINES), 
                  MB_ICONSTOP);
      DDX_Text(pDX, IDC_TRIGGER_TEXT, m_strTriggerText);
      pDX->Fail();
      }     // end of text being blank

    CString strText = ::Replace (m_strTriggerText, ENDLINE, "\\n");

    // check they haven't put crap into the regular expression
    if (!CheckRegularExpression (strText, PCRE_MULTILINE))
      {   // failed check
      DDX_Text(pDX, IDC_TRIGGER_TEXT, m_strTriggerText);
      pDX->Fail();
      }

     }    // end of save and validate
   else
     FixFont (m_font, m_ctlTriggerText, "Courier New", 9, FW_NORMAL, DEFAULT_CHARSET);

}


BEGIN_MESSAGE_MAP(CMultiLineTriggerDlg, CDialog)
	//{{AFX_MSG_MAP(CMultiLineTriggerDlg)
	//}}AFX_MSG_MAP
	ON_COMMAND(CLEAR_SELECTION, OnRemoveSelection)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiLineTriggerDlg message handlers

BOOL CMultiLineTriggerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  PostMessage (WM_COMMAND, CLEAR_SELECTION);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CMultiLineTriggerDlg::OnRemoveSelection()
  {

  m_ctlTriggerText.SetSel (m_strTriggerText.GetLength (), m_strTriggerText.GetLength ());

  }
