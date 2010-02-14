// EditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "EditDlg.h"
#include "..\winplace.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define REGEXP_FIRST_MENU 10000
#define CLEAR_SELECTION 10200

/////////////////////////////////////////////////////////////////////////////
// CEditDlg dialog


CEditDlg::CEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditDlg)
	m_strText = _T("");
	//}}AFX_DATA_INIT

  m_font = NULL;
  m_bRegexp = false;

}

void CEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditDlg)
	DDX_Control(pDX, IDC_REGEXP_BUTTON, m_ctlRegexpButton);
	DDX_Control(pDX, IDOK, m_ctlOK);
	DDX_Control(pDX, IDCANCEL, m_ctlCancel);
	DDX_Control(pDX, IDC_TEXT, m_ctlText);
	DDX_Text(pDX, IDC_TEXT, m_strText);
	//}}AFX_DATA_MAP
 if(pDX->m_bSaveAndValidate)
   {
   if (m_strText.FindOneOf ("\n\r") != -1)
     {
      ::TMessageBox("Line breaks not permitted here.");
      DDX_Text(pDX, IDC_TEXT, m_strText);
      pDX->Fail();
     }
   }
 else
   {
   FixFont (m_font, m_ctlText, App.m_strFixedPitchFont, App.m_iFixedPitchFontSize, FW_NORMAL, DEFAULT_CHARSET);
   }

}


BEGIN_MESSAGE_MAP(CEditDlg, CDialog)
	//{{AFX_MSG_MAP(CEditDlg)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_REGEXP_BUTTON, OnRegexpButton)
	ON_COMMAND(ID_POPUP_HELP, OnPopupHelp)
	//}}AFX_MSG_MAP

  // MXP pop-up menu
  ON_COMMAND_RANGE(REGEXP_FIRST_MENU, 
                    REGEXP_FIRST_MENU + 100, 
                    OnRegexpMenu)

	ON_COMMAND(CLEAR_SELECTION, OnRemoveSelection)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditDlg message handlers


void CEditDlg::OnSize(UINT nType, int cx, int cy) 
  {
	CDialog::OnSize(nType, cx, cy);
	
  if (m_ctlText.m_hWnd && m_ctlCancel.m_hWnd && m_ctlOK.m_hWnd)
    {
    // move OK and Cancel buttons
    WINDOWPLACEMENT wndpl;
    int iHeight;
    int iWidth;

    // where is OK button?
    GetButtonSize (m_ctlOK, iHeight, iWidth);

    // move to near bottom

    m_ctlOK.MoveWindow (10, cy - iHeight - 10, iWidth, iHeight);

    // where is Cancel button?
    GetButtonSize (m_ctlCancel, iHeight, iWidth);

    // move to near bottom

    m_ctlCancel.MoveWindow (cx - iWidth - 10, cy - iHeight - 10, iWidth, iHeight);

    // where is regexp button?
    GetButtonSize (m_ctlRegexpButton, iHeight, iWidth);

    // move to near bottom

    m_ctlRegexpButton.MoveWindow (cx / 2 - iWidth / 2, cy - iHeight - 10, iWidth, iHeight);

    // where is Cancel button now?
    m_ctlCancel.GetWindowPlacement (&wndpl);

    // move text to just above it
	  m_ctlText.MoveWindow(0, 0, cx, wndpl.rcNormalPosition.top - 10);
    }

  }

void CEditDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
  CWindowPlacement wp;
  wp.Save ("Edit Dialog", this);
	
}

BOOL CEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  CWindowPlacement wp;
  wp.Restore ("Edit Dialog", this, false);
	
  if (!m_strTitle.IsEmpty ())
    SetWindowText (m_strTitle);	

  if (m_bRegexp)
    m_ctlRegexpButton.ShowWindow (SW_SHOW);

  PostMessage (WM_COMMAND, CLEAR_SELECTION);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/*
void CEditDlg::OnRegexp() 
{
SendMessage(iconPointer, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hInstance, IDI_GRIND));
	
}


void CEditDlg::OnSelchangeInsertRegexp() 
{
int nItem = m_ctlSpecials.GetCurSel ();

  if (nItem == LB_ERR)
    return;

  CString strText;

  m_ctlSpecials.GetLBText(nItem, strText) ;

  string s = m_regexp_specials_map [(const char *) strText];

  m_ctlText.ReplaceSel (s.c_str ());	
}


void  CEditDlg::AddSpecial (const string & description,
                             const string & action)
  {
  m_regexp_specials_map [description] = action;
  
  m_ctlSpecials.AddString (description.c_str ());
  }

      */

void CEditDlg::InsertRegexpItem (CMenu* pPopup, 
                       const string & sLabel, 
                       const string & sInsert,
                       const int iBack)
  {

  // add menu item
  pPopup->AppendMenu (MF_STRING | MF_ENABLED, m_next_item, sLabel.c_str ());     
  m_regexp_specials_map [m_next_item] = sInsert;
  m_char_offset [m_next_item] = iBack;
  m_next_item++;
  }

void CEditDlg::OnRegexpButton() 
{
WINDOWPLACEMENT wndpl;

  // where is regexp button?
  m_ctlRegexpButton.GetWindowPlacement (&wndpl);

  CPoint menupoint (wndpl.rcNormalPosition.right, wndpl.rcNormalPosition.top);

  CMenu menu;
	VERIFY(menu.LoadMenu(IDR_REGEXP_MENU));  

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	CWnd* pWndPopupOwner = this;

	CMenu* pSubMenu;
  
  // Character classes submenu

  pSubMenu = pPopup->GetSubMenu(0);

  pSubMenu->DeleteMenu (0, MF_BYPOSITION);  // get rid of dummy item

  m_next_item = REGEXP_FIRST_MENU;
  m_regexp_specials_map.clear ();

  InsertRegexpItem (pSubMenu, "Any Character", ".");
  InsertRegexpItem (pSubMenu, "Character In Range", "[]", -1);
  InsertRegexpItem (pSubMenu, "Character Not In Range", "[^]", -1);

  InsertRegexpItem (pSubMenu, "Whitespace", "\\s");
  InsertRegexpItem (pSubMenu, "Not Whitespace", "\\S");
  InsertRegexpItem (pSubMenu, "Decimal Digit (0-9)", "\\d");
  InsertRegexpItem (pSubMenu, "Not Decimal Digit", "\\D");
  InsertRegexpItem (pSubMenu, "Word (A-Z, a-z, 0-9, _ )", "\\w");
  InsertRegexpItem (pSubMenu, "Not Word", "\\W");

  pSubMenu->AppendMenu (MF_SEPARATOR);
  
  InsertRegexpItem (pSubMenu, "Escape Next Character", "\\");
  InsertRegexpItem (pSubMenu, "[", "\\[");
  InsertRegexpItem (pSubMenu, "]", "\\]");
  InsertRegexpItem (pSubMenu, "(", "\\(");
  InsertRegexpItem (pSubMenu, ")", "\\)");

  pSubMenu->AppendMenu (MF_SEPARATOR);

  InsertRegexpItem (pSubMenu, "Letter (A-Z, a-z)", "[[:alpha:]]");
  InsertRegexpItem (pSubMenu, "Not Letter", "[[:^alpha:]]");
  InsertRegexpItem (pSubMenu, "Letter or Digit (A-Z, a-z, 0-9)", "[[:alnum:]]");
  InsertRegexpItem (pSubMenu, "Not Letter or Digit", "[[:^alnum:]]");
  InsertRegexpItem (pSubMenu, "Lower-case Letter (a-z)", "[[:lower:]]");
  InsertRegexpItem (pSubMenu, "Upper-case Letter (A-Z)", "[[:upper:]]");
  InsertRegexpItem (pSubMenu, "Hex digits (0-9, A-F)", "[[:xdigit:]]");

  // Quantifiers submenu

	pSubMenu = pPopup->GetSubMenu(1);

  pSubMenu->DeleteMenu (0, MF_BYPOSITION);  // get rid of dummy item

  InsertRegexpItem (pSubMenu, "0 or 1 Match", "{0,1}");
  InsertRegexpItem (pSubMenu, "0 or More Matches", "{0,}");
  InsertRegexpItem (pSubMenu, "1 or More Matches", "{1,}");
  InsertRegexpItem (pSubMenu, "4 Matches Exactly", "{4}");
  InsertRegexpItem (pSubMenu, "5 to 10 Matches", "{5,10}");
  
  pSubMenu->AppendMenu (MF_SEPARATOR);
  
  InsertRegexpItem (pSubMenu, "Not Greedy", "?");

  // Grouping submenu

	pSubMenu = pPopup->GetSubMenu(2);

  pSubMenu->DeleteMenu (0, MF_BYPOSITION);  // get rid of dummy item

  InsertRegexpItem (pSubMenu, "Tagged Expression", "()", -1);
  InsertRegexpItem (pSubMenu, "Named Expression", "(?P<name>)", -1);
  InsertRegexpItem (pSubMenu, "Group Without Capture", "(?:)", -1);
  
  pSubMenu->AppendMenu (MF_SEPARATOR);

  InsertRegexpItem (pSubMenu, "Or", "|");

  // Positions submenu

	pSubMenu = pPopup->GetSubMenu(3);

  pSubMenu->DeleteMenu (0, MF_BYPOSITION);  // get rid of dummy item
  
  InsertRegexpItem (pSubMenu, "Beginning Of Line", "^");
  InsertRegexpItem (pSubMenu, "End Of Line", "$");
  
  pSubMenu->AppendMenu (MF_SEPARATOR);

  InsertRegexpItem (pSubMenu, "Start Of Subject", "\\A");
  InsertRegexpItem (pSubMenu, "End Of Subject or Newline At End", "\\Z");
  InsertRegexpItem (pSubMenu, "End Of Subject", "\\z");
  
  pSubMenu->AppendMenu (MF_SEPARATOR);

  InsertRegexpItem (pSubMenu, "Word Boundary", "\\b");
  InsertRegexpItem (pSubMenu, "Not Word Boundary", "\\B");

  
  // Assertions submenu

	pSubMenu = pPopup->GetSubMenu(4);

  pSubMenu->DeleteMenu (0, MF_BYPOSITION);  // get rid of dummy item

  InsertRegexpItem (pSubMenu, "Positive Lookahead", "(?=)", -1);
  InsertRegexpItem (pSubMenu, "Negative Lookahead", "(?!)", -1);
  InsertRegexpItem (pSubMenu, "Positive Lookbehind", "(?<=)", -1);
  InsertRegexpItem (pSubMenu, "Negative Lookbehind", "(?<!)", -1);

  // Options submenu

	pSubMenu = pPopup->GetSubMenu(5);

  pSubMenu->DeleteMenu (0, MF_BYPOSITION);  // get rid of dummy item

  InsertRegexpItem (pSubMenu, "Caseless", "(?i)");
  InsertRegexpItem (pSubMenu, "Dot Matches All", "(?s)");
  InsertRegexpItem (pSubMenu, "Duplicate Names", "(?J)");
  InsertRegexpItem (pSubMenu, "Extended Syntax", "(?x)");
  InsertRegexpItem (pSubMenu, "Match Case", "(?-i)");
  InsertRegexpItem (pSubMenu, "Multiline", "(?m)");
  InsertRegexpItem (pSubMenu, "Ungreedy", "(?U)");
  
  pSubMenu->AppendMenu (MF_SEPARATOR);

  InsertRegexpItem (pSubMenu, "Comment", "(?#)", -1);


	while (pWndPopupOwner->GetStyle() & WS_CHILD)
		pWndPopupOwner = pWndPopupOwner->GetParent();

  ClientToScreen(&menupoint);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
                        menupoint.x, 
                        menupoint.y,
			                  pWndPopupOwner);
  
}   // end of CEditDlg::OnRegexpButton


void CEditDlg::OnRegexpMenu (UINT nID)
  {
  map<int, string>::const_iterator it = m_regexp_specials_map.find (nID);

  if (it != m_regexp_specials_map.end ())
    {
    m_ctlText.ReplaceSel (it->second.c_str (), TRUE);	  // can undo it
    int iBack = m_char_offset [nID];
    if (iBack)   // adjust cursor?
      {
      int nStart, nEnd; 
      m_ctlText.GetSel (nStart, nEnd);
      nEnd += iBack;
      m_ctlText.SetSel (nEnd, nEnd);
      }   // end of adjustment wanted
    m_ctlText.SetFocus ();
    } // end of found in map

  }   // end of CEditDlg::OnRegexpMenu


void CEditDlg::OnRemoveSelection()
  {

  m_ctlText.SetSel (m_strText.GetLength (), m_strText.GetLength ());

  }

void CEditDlg::OnPopupHelp() 
{
  ShowHelp ("DOC_", "regexp")	;
}
