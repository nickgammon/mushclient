// ImmediateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "..\doc.h"
#include "ImmediateDlg.h"
#include "..\mainfrm.h"
#include "EditMultiLine.h"

#define CLEAR_SELECTION 10200

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImmediateDlg dialog


CImmediateDlg::CImmediateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImmediateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImmediateDlg)
	m_strExpression = _T("");
	//}}AFX_DATA_INIT
}


void CImmediateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImmediateDlg)
	DDX_Control(pDX, IDC_EXPRESSION, m_ctlExpression);
	DDX_Text(pDX, IDC_EXPRESSION, m_strExpression);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImmediateDlg, CDialog)
	//{{AFX_MSG_MAP(CImmediateDlg)
	ON_BN_CLICKED(IDOK, OnRun)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	//}}AFX_MSG_MAP

  ON_COMMAND(CLEAR_SELECTION, OnRemoveSelection)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImmediateDlg message handlers

void CImmediateDlg::OnRun() 
{
CString strText;

  if (!m_pDoc->m_ScriptEngine)
    return;

  m_ctlExpression.GetWindowText (strText);

  Frame.SetStatusMessageNow (Translate ("Executing immediate script"));

  m_pDoc->m_bInSendToScript = false;  // they can do DeleteLines here I think

  m_pDoc->m_ScriptEngine->Parse (strText, "Immediate");

  m_pDoc->m_bInSendToScript = true;

  m_pDoc->ShowStatusLine ();

}

void CImmediateDlg::OnCancel( )
  {
  // save the expression text for next time
  m_ctlExpression.GetWindowText (m_strExpression);
  CDialog::OnCancel ();
  }

void CImmediateDlg::OnEdit() 
{
CEditMultiLine dlg;

  dlg.m_strText = GetText (m_ctlExpression);

  dlg.m_strTitle = "Edit immediate expression";

  dlg.m_bScript = true;

  if (m_pDoc->GetScriptEngine () && m_pDoc->GetScriptEngine ()->L)
    dlg.m_bLua = true;

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlExpression.SetWindowText (dlg.m_strText);
	
}

BOOL CImmediateDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  PostMessage (WM_COMMAND, CLEAR_SELECTION);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CImmediateDlg::OnRemoveSelection()
  {

  m_ctlExpression.SetSel (m_strExpression.GetLength (), m_strExpression.GetLength ());

  }
