// SpellCheckDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "SpellCheckDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpellCheckDlg dialog


CSpellCheckDlg::CSpellCheckDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpellCheckDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpellCheckDlg)
	m_strMisspeltWord = _T("");
	//}}AFX_DATA_INIT
}


void CSpellCheckDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpellCheckDlg)
	DDX_Control(pDX, ID_MISSPELT_WORD, m_ctlMisspeltWord);
	DDX_Control(pDX, ID_SUGGESTIONS, m_ctlSuggestions);
	DDX_Text(pDX, ID_MISSPELT_WORD, m_strMisspeltWord);
	//}}AFX_DATA_MAP

  if (!pDX->m_bSaveAndValidate)
    m_strOriginalWord = m_strMisspeltWord;

}


BEGIN_MESSAGE_MAP(CSpellCheckDlg, CDialog)
	//{{AFX_MSG_MAP(CSpellCheckDlg)
	ON_BN_CLICKED(ID_IGNORE, OnIgnore)
	ON_BN_CLICKED(ID_IGNORE_ALL, OnIgnoreAll)
	ON_BN_CLICKED(ID_ADD, OnAdd)
	ON_BN_CLICKED(ID_CHANGE, OnChange)
	ON_BN_CLICKED(ID_CHANGE_ALL, OnChangeAll)
	ON_LBN_DBLCLK(ID_SUGGESTIONS, OnDblclkSuggestions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellCheckDlg message handlers

void CSpellCheckDlg::OnIgnore() 
{
  m_sAction = "ignore";
  CDialog::OnOK();
}

void CSpellCheckDlg::OnIgnoreAll() 
{
  m_sAction = "ignoreall";
  CDialog::OnOK();
}

void CSpellCheckDlg::OnAdd() 
{
  m_sAction = "add";
  CDialog::OnOK();
}

void CSpellCheckDlg::OnChange() 
{
  m_ctlMisspeltWord.GetWindowText (m_strMisspeltWord);

  if (m_strMisspeltWord == m_strOriginalWord)
    {
    int iWhich = m_ctlSuggestions.GetCurSel ();

    if (iWhich != LB_ERR)
      {
      m_ctlSuggestions.GetText (iWhich, m_strMisspeltWord);
      m_ctlMisspeltWord.SetWindowText (m_strMisspeltWord);
      }

    }  // they have not changed the word manually

  m_sAction = "change";
  CDialog::OnOK();
}

void CSpellCheckDlg::OnChangeAll() 
{

  m_ctlMisspeltWord.GetWindowText (m_strMisspeltWord);

  if (m_strMisspeltWord == m_strOriginalWord)
    {
    int iWhich = m_ctlSuggestions.GetCurSel ();

    if (iWhich != LB_ERR)
      {
      m_ctlSuggestions.GetText (iWhich, m_strMisspeltWord);
      m_ctlMisspeltWord.SetWindowText (m_strMisspeltWord);
      }

    }  // they have not changed the word manually

  m_sAction = "changeall";
  CDialog::OnOK();
}


BOOL CSpellCheckDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  for (vector<string>::const_iterator it = m_suggestions.begin ();
       it != m_suggestions.end ();
       it++)
    m_ctlSuggestions.AddString (it->c_str ());

  if (!m_suggestions.empty ())
     m_ctlSuggestions.SetCurSel (0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSpellCheckDlg::OnDblclkSuggestions() 
{
OnChange ();	
}

void CSpellCheckDlg::OnOK() 
{
  // do nothing on default button
}
