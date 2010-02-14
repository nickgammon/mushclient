// EditVariable.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\mushclient.h"
#include "..\..\doc.h"
#include "EditVariable.h"
#include "..\EditMultiLine.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditVariable dialog


IMPLEMENT_DYNAMIC(CEditVariable, CDialog)

CEditVariable::CEditVariable(CWnd* pParent /*=NULL*/)
	: CDialog(CEditVariable::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditVariable)
	m_strContents = _T("");
	//}}AFX_DATA_INIT
}


void CEditVariable::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditVariable)
	DDX_Control(pDX, IDC_VARIABLE_CONTENTS, m_ctlContents);
	DDX_Control(pDX, IDC_VARIABLE_NAME, m_ctlName);
	DDX_Text(pDX, IDC_VARIABLE_CONTENTS, m_strContents);
	DDX_MinMaxString(pDX, IDC_VARIABLE_NAME, m_strName);
	DDV_MinMaxString(pDX, m_strName, 1, 1000);
	//}}AFX_DATA_MAP

 if(pDX->m_bSaveAndValidate)
   {

    m_strName.TrimLeft ();
    m_strName.TrimRight ();

// check name is valid

    if (CheckLabel (m_strName))
      {
      ::TMessageBox("The variable name must start with a letter and consist of letters"
                      ", numbers or the underscore character.");
      DDX_Text(pDX, IDC_VARIABLE_NAME, m_strName);
      pDX->Fail();
      return;
      }


    CVariable * variable_item;

    if (!m_bDoingChange)    // only do this on an add
      if (m_pVariableMap->Lookup (m_strName, variable_item))
        {
        ::TMessageBox("This variable name is already in the list of variables.");
        DDX_Text(pDX, IDC_VARIABLE_NAME, m_strName);
        pDX->Fail();
        return;
        }

   } // end of saving and validating

}


BEGIN_MESSAGE_MAP(CEditVariable, CDialog)
	//{{AFX_MSG_MAP(CEditVariable)
	ON_BN_CLICKED(IDC_HELPBUTTON, OnHelpbutton)
	ON_BN_CLICKED(IDC_EDIT_CONTENTS, OnEditContents)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditVariable message handlers

void CEditVariable::OnHelpbutton() 
{
//	  OnCmdMsg(ID_HELP, CN_COMMAND, NULL, NULL);
	App.WinHelp(m_nIDHelp + HID_BASE_RESOURCE);
    
}

BOOL CEditVariable::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  // when editing, set focus to contents
  if (!m_strName.IsEmpty ())
    {
    m_ctlContents.SetFocus ();
    m_ctlContents.SetSel (0, -1);
    }

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditVariable::OnEditContents() 
{
CEditMultiLine dlg;

  dlg.m_strText = GetText (m_ctlContents);

  if (m_strName.IsEmpty ())
    dlg.m_strTitle = Translate ("Edit variable");
  else
    dlg.m_strTitle = TFormat ("Edit variable '%s'", (LPCTSTR) m_strName);

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlContents.SetWindowText (dlg.m_strText);
	
}
