// LuaInputBox.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "LuaInputBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable : 4800)  // forcing value to bool 'true' or 'false' (performance warning)

/////////////////////////////////////////////////////////////////////////////
// CLuaInputBox dialog

#define CLEAR_SELECTION 10200

CLuaInputBox::CLuaInputBox(CWnd* pParent /*=NULL*/)
	: CDialog(CLuaInputBox::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLuaInputBox)
	m_strMessage = _T("");
	m_strReply = _T("");
	//}}AFX_DATA_INIT

  m_font = NULL;
  m_L = NULL;
  m_iValidationIndex = LUA_NOREF;

}


void CLuaInputBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLuaInputBox)
	DDX_Control(pDX, IDC_INPUT_BOX_REPLY, m_ctlReply);
	DDX_Text(pDX, IDC_INPUT_BOX_MESSAGE, m_strMessage);
	DDX_Text(pDX, IDC_INPUT_BOX_REPLY, m_strReply);
	//}}AFX_DATA_MAP

 if(!pDX->m_bSaveAndValidate)
   {
   if (!m_strFont.IsEmpty () && m_iFontSize > 3)
      FixFont (m_font, m_ctlReply, m_strFont, m_iFontSize, FW_NORMAL, DEFAULT_CHARSET);
   return;
   }


 if (m_L && m_iValidationIndex != LUA_NOREF)
  {
  bool bWanted = false;

  lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_iValidationIndex);

  // Lua validation:  function f (value)  ... end

  // what they have currently typed
  lua_pushlstring (m_L, m_strReply, m_strReply.GetLength ());

  // call the function: arg1: what they typed
  if (lua_pcall (m_L, 1, 1, 0))   // call with 1 arg and 1 result
    {
    LuaError (m_L);    // note that this clears the stack, so we won't call it again
    lua_settop (m_L, 0);   // clear stack, just in case LuaError changes behaviour
    pDX->Fail();
    }   // end of error
  else
    {
    bWanted = lua_toboolean (m_L, -1);
    lua_pop (m_L, 1);  // pop result
    }  // end of no error

  if (!bWanted)
    {
    DDX_Text(pDX, IDC_INPUT_BOX_REPLY, m_strReply);
    pDX->Fail();
    }

  }  // end of Lua validation function available


}


BEGIN_MESSAGE_MAP(CLuaInputBox, CDialog)
	//{{AFX_MSG_MAP(CLuaInputBox)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP

  ON_COMMAND(CLEAR_SELECTION, OnRemoveSelection)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLuaInputBox message handlers

BOOL CLuaInputBox::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  SetWindowText (m_strTitle);	
	
  PostMessage (WM_COMMAND, CLEAR_SELECTION);

  // move dialog into position
  WINDOWPLACEMENT wndpl;

  GetWindowPlacement (&wndpl);

  if (m_iBoxWidth < 180)  // need room for OK and Cancel buttons
    m_iBoxWidth = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;

  if (m_iBoxHeight < 125) // need room for prompt and reply
    m_iBoxHeight = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;

  MoveWindow(wndpl.rcNormalPosition.left, wndpl.rcNormalPosition.top, m_iBoxWidth, m_iBoxHeight);

  // limit text entry length if desired
  if (m_iMaxReplyLength > 0)
    ::SendMessage(m_ctlReply, EM_LIMITTEXT, m_iMaxReplyLength, 0);

  // new OK button label
  if (!m_strOKbuttonLabel.IsEmpty ())
     GetDlgItem (IDOK)->SetWindowText (m_strOKbuttonLabel);

  // new Cancel button label
  if (!m_strCancelbuttonLabel.IsEmpty ())
     GetDlgItem (IDCANCEL)->SetWindowText (m_strCancelbuttonLabel);

  CWnd * ctlOK      = GetDlgItem (IDOK);
  CWnd * ctlCancel  = GetDlgItem (IDCANCEL);

  int iHeight;
  int iWidth;
  WINDOWPLACEMENT buttonwndpl;        // where button is

  // make OK button requested size
  if (m_iOKbuttonWidth > 0)
    {
    GetButtonSize (*ctlOK, iHeight, iWidth);
    ctlOK->GetWindowPlacement (&buttonwndpl);
    ctlOK->MoveWindow (buttonwndpl.rcNormalPosition.left, 
                       buttonwndpl.rcNormalPosition.top,
                       m_iOKbuttonWidth, iHeight);
    }

  // make Cancel button requested size
  if (m_iCancelbuttonWidth > 0)
    {
    GetButtonSize (*ctlCancel, iHeight, iWidth);
    ctlCancel->GetWindowPlacement (&buttonwndpl);
    ctlCancel->MoveWindow (buttonwndpl.rcNormalPosition.left, 
                       buttonwndpl.rcNormalPosition.top,
                       m_iCancelbuttonWidth, iHeight);
    }

  // layout dialog based on new button etc. sizes
  Calculate_Button_Positions ();

  if (m_bReadOnly)
    {
    m_ctlReply.SetReadOnly (TRUE);
    GetDlgItem (IDOK)->SetFocus ();
    return FALSE;
    }
  else
	  return TRUE;  // return TRUE unless you set the focus to a control
}   // end of CLuaInputBox::OnInitDialog

void CLuaInputBox::OnRemoveSelection()
  {

//  m_ctlReply.SetSel (m_strReply.GetLength (), m_strReply.GetLength ());

  }   // end of CLuaInputBox::OnRemoveSelection


void CLuaInputBox::Calculate_Button_Positions ()
  {

  int cx, cy;

  CRect rect;

	GetClientRect(&rect);

  cx = rect.right - rect.left;
  cy = rect.bottom - rect.top;

  CWnd * ctlOK      = GetDlgItem (IDOK);
  CWnd * ctlCancel  = GetDlgItem (IDCANCEL);
  CWnd * ctlMessage = GetDlgItem (IDC_INPUT_BOX_MESSAGE);

  if (ctlCancel && 
      ctlCancel->m_hWnd && 
      ctlOK &&
      ctlOK->m_hWnd &&
      ctlMessage &&
      ctlMessage->m_hWnd 
      
      )
    {
    // move OK and Cancel buttons
    int iOKHeight;
    int iOKWidth;
    int iCancelHeight;
    int iCancelWidth;
    int iBorder = 10;

    // -----------------------
    // where is OK button?
    GetButtonSize (*ctlOK, iOKHeight, iOKWidth);
    GetButtonSize (*ctlCancel, iCancelHeight, iCancelWidth);

    int iTopOfRow = cy - iOKHeight - 10;

    // ------------------------

    // calculate gaps for middle buttons  

    // gap (between OK and cancel buttons) will be the width of the dialog
    // less the gaps on the side of those buttons, less the width of the buttons themselves

    int iGap = cx - (iBorder * 2) - (iOKWidth + iCancelWidth);

    // -----------------------


    // OK button (1)
    ctlOK->MoveWindow (iBorder, iTopOfRow, iOKWidth, iOKHeight);

    // Cancel Button (2)
    ctlCancel->MoveWindow (iBorder + iOKWidth + iGap, iTopOfRow, iCancelWidth, iCancelHeight);


    WINDOWPLACEMENT promptwndpl;        // where prompt is
    WINDOWPLACEMENT cancelwndpl;        // where cancel button is

    // where is prompt?
    ctlMessage->GetWindowPlacement (&promptwndpl);

    // where it he cancel button now?
    ctlCancel->GetWindowPlacement (&cancelwndpl);

    // if prompt too small, make it width of window
    if (m_iPromptWidth <= 0)
      m_iPromptWidth = cancelwndpl.rcNormalPosition.right - promptwndpl.rcNormalPosition.left;

    if (m_iPromptWidth < 10)
      m_iPromptWidth = 10;

    if (m_iPromptHeight <= 0)
      m_iPromptHeight = promptwndpl.rcNormalPosition.bottom - promptwndpl.rcNormalPosition.top;

    if (m_iPromptHeight < 12)
      m_iPromptHeight = 12;

    // new prompt size
    ctlMessage->MoveWindow(promptwndpl.rcNormalPosition.left, promptwndpl.rcNormalPosition.top, m_iPromptWidth, m_iPromptHeight);

    // where is the prompt now?
    ctlMessage->GetWindowPlacement (&promptwndpl);

    // default reply width is from left of prompt to right of cancel button
    int iReplyWidth = cancelwndpl.rcNormalPosition.right - promptwndpl.rcNormalPosition.left;

    // default reply height is from bottom of prompt (+ gap) to top of cancel button (+ gap)
    int iReplyHeight = cancelwndpl.rcNormalPosition.top - promptwndpl.rcNormalPosition.bottom - iBorder * 2;

    // override with user-supplied if smaller
    if (m_iReplyWidth < iReplyWidth && m_iReplyWidth > 10)
       iReplyWidth = m_iReplyWidth;

    // ditto for height
    if (m_iReplyHeight < iReplyHeight && m_iReplyHeight > 10)
       iReplyHeight = m_iReplyHeight;

    // move reply area to fit
    m_ctlReply.MoveWindow (promptwndpl.rcNormalPosition.left,       // left
                           promptwndpl.rcNormalPosition.bottom + iBorder,      // top
                           iReplyWidth,  // width
                           iReplyHeight);  // height

                           


    }  // end of controls available


  }   // end of CLuaInputBox::Calculate_Button_Positions 

void CLuaInputBox::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
  Calculate_Button_Positions ();

	
}   // end of CLuaInputBox::OnSize

BOOL CLuaInputBox::PreTranslateMessage(MSG* pMsg) 
{
  // if no default button wanted, throw away <enter> key
  if( pMsg->message==WM_KEYDOWN &&
      pMsg->wParam==VK_RETURN &&
      m_bNoDefault)
     return TRUE;

  return CDialog::PreTranslateMessage(pMsg);
}     // end of CLuaInputBox::PreTranslateMessage
