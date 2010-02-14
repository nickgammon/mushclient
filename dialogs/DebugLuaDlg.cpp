// DebugLuaDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "DebugLuaDlg.h"
#include "EditMultiLine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDebugLuaDlg dialog


CDebugLuaDlg::CDebugLuaDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDebugLuaDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDebugLuaDlg)
	m_strCommand = _T("");
	m_strCurrentLine = _T("");
	m_strFunctionName = _T("");
	m_strFunction = _T("");
	m_strLines = _T("");
	m_strNups = _T("");
	m_strSource = _T("");
	m_strWhat = _T("");
	//}}AFX_DATA_INIT

  L = NULL;
  m_bAbort = false;
  m_font = NULL;

}


void CDebugLuaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDebugLuaDlg)
	DDX_Control(pDX, IDC_COMMAND, m_ctlCommand);
	DDX_Text(pDX, IDC_COMMAND, m_strCommand);
	DDX_Text(pDX, IDC_CURRENT_LINE, m_strCurrentLine);
	DDX_Text(pDX, IDC_FUNC_NAME, m_strFunctionName);
	DDX_Text(pDX, IDC_FUNCTION, m_strFunction);
	DDX_Text(pDX, IDC_LINES, m_strLines);
	DDX_Text(pDX, IDC_NUPS, m_strNups);
	DDX_Text(pDX, IDC_SOURCE, m_strSource);
	DDX_Text(pDX, IDC_WHAT, m_strWhat);
	//}}AFX_DATA_MAP

 if(!pDX->m_bSaveAndValidate)
   FixFont (m_font, m_ctlCommand, App.m_strFixedPitchFont, App.m_iFixedPitchFontSize, FW_NORMAL, DEFAULT_CHARSET);

}


BEGIN_MESSAGE_MAP(CDebugLuaDlg, CDialog)
	//{{AFX_MSG_MAP(CDebugLuaDlg)
	ON_BN_CLICKED(ID_DO_COMMAND, OnDoCommand)
	ON_BN_CLICKED(IDC_EDIT_COMMAND, OnEditCommand)
	ON_BN_CLICKED(IDC_SHOW_UPS, OnShowUps)
	ON_BN_CLICKED(IDC_SHOW_LOCALS, OnShowLocals)
	ON_BN_CLICKED(IDC_TRACEBACK, OnTraceback)
	ON_BN_CLICKED(IDC_ABORT, OnAbort)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDebugLuaDlg message handlers

void DoLua (lua_State *L, const CString strCommand) 
  {
  if (luaL_loadbuffer(L, strCommand, strCommand.GetLength (), "Debug command")
    || lua_pcall(L, 0, 0, 0))
    LuaError (L, "Error");
  }   // end of DoLua

void CDebugLuaDlg::OnDoCommand() 
{
CString strCommand;

  m_ctlCommand.GetWindowText (strCommand);

  DoLua (L, strCommand);
	
}

void CDebugLuaDlg::OnEditCommand() 
{
CEditMultiLine dlg;

  dlg.m_strText = GetText (m_ctlCommand);

  dlg.m_strTitle = Translate ("Edit command");

  dlg.m_bScript = true;
  dlg.m_bLua = true;

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlCommand.SetWindowText (dlg.m_strText);
	
	
}

void CDebugLuaDlg::OnCancel() 
{
  // save the expression text for next time
  m_ctlCommand.GetWindowText (m_strCommand);
	
	CDialog::OnCancel();
}

// show upvalues
// stack level 1 is this function
// stack level 2 is the debug.debug function
// stack level 3 is the caller of debug.debug
void CDebugLuaDlg::OnShowUps() 
{
DoLua (L, 
"do local i, name, val = 1 "
"local t = debug.getinfo (3, 'f') "
"local f = t.func "
"repeat "
" name, val = debug.getupvalue (f, i) "
"  if name then  "
"    ColourNote ('limegreen', 'black', name .. ' = ' .. tostring (val))  "
"    i = i + 1  "
"  end "
"until not name	"
"ColourNote ('limegreen', 'black', tostring (i - 1) .. ' upvalue(s)') "
"end" );
}

// show local variables
// stack level 1 is this function
// stack level 2 is the debug.debug function
// stack level 3 is the caller of debug.debug
void CDebugLuaDlg::OnShowLocals() 
{
DoLua (L, 
"do local i, name, val = 1 "
"repeat "
" name, val = debug.getlocal (3, i) "
"  if name then  "
"    ColourNote ('limegreen', 'black', name .. ' = ' .. tostring (val))  "
"    i = i + 1  "
"  end "
"until not name	 "
"ColourNote ('limegreen', 'black', tostring (i - 1) .. ' local(s)') "
"end" );
}

void CDebugLuaDlg::OnTraceback() 
{
  DoLua (L, "ColourNote ('limegreen', 'black', debug.traceback ())");	
}

void CDebugLuaDlg::OnAbort() 
{
  m_bAbort = true;
  CDialog::OnCancel();

}
