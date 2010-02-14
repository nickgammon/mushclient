// LuaGsubDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "LuaGsubDlg.h"
#include "EditDlg.h"
#include "EditMultiLine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLuaGsubDlg dialog


CLuaGsubDlg::CLuaGsubDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLuaGsubDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLuaGsubDlg)
	m_bEachLine = FALSE;
	m_strFindPattern = _T("");
	m_strReplacement = _T("");
	m_bEscapeSequences = FALSE;
	m_bCallFunction = FALSE;
	m_strSelectionSize = _T("");
	//}}AFX_DATA_INIT
}


void CLuaGsubDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLuaGsubDlg)
	DDX_Control(pDX, IDC_REPLACEMENT, m_ctlReplacementText);
	DDX_Control(pDX, IDC_CALL, m_ctlCallFunction);
	DDX_Control(pDX, IDC_FIND_PATTERN, m_ctlFindText);
	DDX_Check(pDX, IDC_EACH_LINE, m_bEachLine);
	DDX_Text(pDX, IDC_FIND_PATTERN, m_strFindPattern);
	DDX_Text(pDX, IDC_REPLACEMENT, m_strReplacement);
	DDX_Check(pDX, IDC_ESCAPE_SEQUENCES, m_bEscapeSequences);
	DDX_Check(pDX, IDC_CALL, m_bCallFunction);
	DDX_Text(pDX, IDC_SELECTION_SIZE, m_strSelectionSize);
	//}}AFX_DATA_MAP

  if (pDX->m_bSaveAndValidate)
    {
  // compile function, if they supplied one

    if (m_bCallFunction)
      {

      m_strReplacement.TrimLeft ();
      m_strReplacement.TrimRight ();

      if (m_strReplacement.IsEmpty ())
        {
        ::TMessageBox ("When calling a function the replacement text must be the function name",
                         MB_ICONEXCLAMATION);
        DDX_Text(pDX, IDC_REPLACEMENT, m_strReplacement);
        pDX->Fail();
        }

      // note - an error here is a *compile* error
      if (luaL_loadbuffer(L, m_strFunctionText, m_strFunctionText.GetLength (), "Replacement function")
        || lua_pcall(L, 0, 0, 0))
        {
        LuaError (L, "Compile error");
        DDX_Text(pDX, IDC_REPLACEMENT, m_strReplacement);
        pDX->Fail();
        }

      // try to get replacement function
//      lua_getglobal (L, m_strReplacement);  

      if (! FindLuaFunction (L, m_strReplacement))
        {

        ::UMessageBox (TFormat ("Function '%s' not found in script text", (LPCTSTR) m_strReplacement),
                         MB_ICONEXCLAMATION);
        DDX_Text(pDX, IDC_REPLACEMENT, m_strReplacement);
        pDX->Fail();

        }

      }   // end of calling a function

    }   // end of saving and validating

}


BEGIN_MESSAGE_MAP(CLuaGsubDlg, CDialog)
	//{{AFX_MSG_MAP(CLuaGsubDlg)
	ON_BN_CLICKED(IDC_EDIT_FUNCTION, OnEditFunction)
	ON_BN_CLICKED(IDC_EDIT_FIND, OnEditFind)
	ON_BN_CLICKED(IDC_EDIT_REPLACE, OnEditReplace)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDOK, OnUpdateNeedFindText)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLuaGsubDlg message handlers


LRESULT CLuaGsubDlg::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CAsciiArtDlg::OnKickIdle

void CLuaGsubDlg::OnUpdateNeedFindText(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!GetText (m_ctlFindText).IsEmpty ());
}


void CLuaGsubDlg::OnEditFunction() 
{
CEditMultiLine dlg;

  dlg.m_strText = m_strFunctionText;

  dlg.m_strTitle = "Edit replacement function";

//  dlg.m_bScript = true;  // don't do this, world functions are not available

  if (dlg.DoModal () != IDOK)
      return;

  m_strFunctionText = dlg.m_strText;

  // if they have supplied function text, check the "call" box for them
  if (!m_strFunctionText.IsEmpty ())
    m_ctlCallFunction.SetCheck (1);
	
}

void CLuaGsubDlg::OnEditFind() 
{
CEditDlg dlg;

  dlg.m_strText = GetText (m_ctlFindText);

  dlg.m_strTitle = Translate ("Edit 'find pattern'");

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlFindText.SetWindowText (dlg.m_strText);
	
}

void CLuaGsubDlg::OnEditReplace() 
{
CEditDlg dlg;

  dlg.m_strText = GetText (m_ctlReplacementText);

  dlg.m_strTitle = Translate ("Edit 'replacement' text");

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlReplacementText.SetWindowText (dlg.m_strText);
	
}
