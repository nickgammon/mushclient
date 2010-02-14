// EditMultiLine.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "..\mainfrm.h"
#include "EditMultiLine.h"
#include "..\winplace.h"
#include "GoToLineDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CLEAR_SELECTION 10200

/////////////////////////////////////////////////////////////////////////////
// CEditMultiLine dialog


CEditMultiLine::CEditMultiLine(CWnd* pParent /*=NULL*/)
	: CDialog(CEditMultiLine::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditMultiLine)
	m_strText = _T("");
	//}}AFX_DATA_INIT

  m_font = NULL;
  m_bScript = false;
  m_bLua = false;

}


void CEditMultiLine::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditMultiLine)
	DDX_Control(pDX, IDC_GOTO_LINE, m_ctlGoToLineButton);
	DDX_Control(pDX, IDC_COMPLETE_WORD, m_ctlCompleteFunctionButton);
	DDX_Control(pDX, IDC_FUNCTION_LIST, m_ctlFunctionListButton);
	DDX_Control(pDX, IDOK, m_ctlOK);
	DDX_Control(pDX, IDCANCEL, m_ctlCancel);
	DDX_Control(pDX, IDC_TEXT, m_ctlText);
	DDX_Text(pDX, IDC_TEXT, m_strText);
	//}}AFX_DATA_MAP

 if(!pDX->m_bSaveAndValidate)
   FixFont (m_font, m_ctlText, App.m_strFixedPitchFont, App.m_iFixedPitchFontSize, FW_NORMAL, DEFAULT_CHARSET);

}


BEGIN_MESSAGE_MAP(CEditMultiLine, CDialog)
	//{{AFX_MSG_MAP(CEditMultiLine)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_FUNCTION_LIST, OnFunctionList)
	ON_BN_CLICKED(IDC_COMPLETE_WORD, OnCompleteWord)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_GOTO_LINE, OnGotoLine)
	//}}AFX_MSG_MAP

  ON_COMMAND(CLEAR_SELECTION, OnRemoveSelection)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditMultiLine message handlers


// helpful macro for adjusting button positions
#define ADJUST_BUTTON(ctl, item) \
  ctl.MoveWindow          (iBorder + (iWidth * (item - 1)) + (iGap * (item - 1)), \
                           iTopOfRow, iWidth, iHeight)


void CEditMultiLine::OnSize(UINT nType, int cx, int cy) 
  {
	CDialog::OnSize(nType, cx, cy);
	
  if (m_ctlText.m_hWnd && 
      m_ctlCancel.m_hWnd && 
      m_ctlFunctionListButton.m_hWnd &&
      m_ctlCompleteFunctionButton.m_hWnd &&
      m_ctlGoToLineButton.m_hWnd &&
      m_ctlOK)
    {
    // move OK and Cancel buttons
    WINDOWPLACEMENT wndpl;
    int iHeight;
    int iWidth;
    int iBorder = 10;

    const int iButtonCount = 5; // how many buttons

    // -----------------------
    // where is OK button?
    GetButtonSize (m_ctlOK, iHeight, iWidth);

    int iTopOfRow = cy - iHeight - 10;

    // ------------------------

    // calculate gaps for middle buttons - I will assume all buttons are the same size here

    // gap (between OK and cancel buttons) will be the width of the dialog
    // less the gaps on the side of those buttons, less the width of the iButtonCount buttons themselves

    int iGap = cx - (iBorder * 2) - (iWidth * iButtonCount);

    // we need (iButtonCount - 1) gaps:  OK --1-- Functions --2-- Complete --3-- GoToLine --4-- Cancel
    iGap /= iButtonCount - 1;

    // -----------------------

    // OK button (1)
    ADJUST_BUTTON (m_ctlOK, 1);

    // Function list button (2)
    ADJUST_BUTTON (m_ctlFunctionListButton, 2);

    // Complete function name button (3)
    ADJUST_BUTTON (m_ctlCompleteFunctionButton, 3);

    // Go to Line button (4)
    ADJUST_BUTTON (m_ctlGoToLineButton, 4);

    // Cancel Button (5)
    ADJUST_BUTTON (m_ctlCancel, 5);

    // -----------------------
    // where is Cancel button now?
    m_ctlCancel.GetWindowPlacement (&wndpl);

    // move text to just above it
	  m_ctlText.MoveWindow(0, 0, cx, wndpl.rcNormalPosition.top - 10);
    }

  }

BOOL CEditMultiLine::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  CWindowPlacement wp;
  wp.Restore ("Edit MultiLine Dialog", this, false);

  if (!m_strTitle.IsEmpty ())
    SetWindowText (m_strTitle);	
	
  PostMessage (WM_COMMAND, CLEAR_SELECTION);

  if (!m_bScript)
    {
    m_ctlFunctionListButton.ShowWindow (SW_HIDE); 
    m_ctlCompleteFunctionButton.ShowWindow (SW_HIDE);
    m_ctlGoToLineButton.ShowWindow (SW_HIDE);
    }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditMultiLine::OnRemoveSelection()
  {

  m_ctlText.SetSel (m_strText.GetLength (), m_strText.GetLength ());

  }

void CEditMultiLine::OnFunctionList() 
{
int nStartChar,
    nEndChar;
CString strSelection;

  // find the selection range
  m_ctlText.GetSel(nStartChar, nEndChar);
  // get window text
  m_ctlText.GetWindowText (strSelection);

  ShowFunctionslist (strSelection, nStartChar, nEndChar, m_bLua);
}

void CEditMultiLine::OnCompleteWord() 
{
  FunctionMenu (m_ctlText, m_bScript && m_bLua);
}

void CEditMultiLine::OnDestroy() 
{
	CDialog::OnDestroy();
	
  CWindowPlacement wp;
  wp.Save ("Edit MultiLine Dialog", this);
	
}

void CEditMultiLine::OnGotoLine() 
{
CGoToLineDlg dlg;

  dlg.m_iMaxLine = 	m_ctlText.GetLineCount ();

  // default to the current line number
  dlg.m_iLineNumber = m_ctlText.LineFromChar () + 1;

  if (dlg.DoModal () != IDOK)
    return;

  // find position in file - this needs to be zero-relative
  int iIndex = m_ctlText.LineIndex (dlg.m_iLineNumber - 1);

  if (iIndex == -1)
    return;   // failed to find position

  // go to the start of that line
  m_ctlText.SetSel(iIndex, iIndex);
	
  // ensure text box has the focus if you click on the button
  m_ctlText.SetFocus ();

}
