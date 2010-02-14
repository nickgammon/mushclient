// cmdhist.cpp : implementation file
//

#include "stdafx.h"
#include "..\MUSHclient.h"
#include "..\doc.h"
#include "..\sendvw.h"
#include "cmdhist.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//#define new DEBUG_NEW 

/////////////////////////////////////////////////////////////////////////////
// CCmdHistory dialog


CCmdHistory::CCmdHistory(CWnd* pParent /*=NULL*/)
	: CDialog(CCmdHistory::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCmdHistory)
	//}}AFX_DATA_INIT
}


void CCmdHistory::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCmdHistory)
	DDX_Control(pDX, IDC_HISTORY_ITEM, m_ctlHistoryItem);
	//}}AFX_DATA_MAP

if(!(pDX->m_bSaveAndValidate))
    {
CListBox* pList = (CListBox*) GetDlgItem (IDC_COMMANDS);
ASSERT (pList);
int count = 0;

   pList->SetRedraw (FALSE);
   pList->ResetContent ();

  CString str;
  POSITION pos = m_msgList->GetHeadPosition ();

  while (pos)
    {
    int nItem;
    POSITION itemPos = pos;
    str = m_msgList->GetNext (pos);

    // truncate long strings or we might get a nasty crash with long strings

    if (str.GetLength () > 500)
      {
      str = str.Left (500);
      str += " ...";
      }

    nItem = pList->AddString(str);  // add to list (truncate to 500 chars)
    if (nItem != LB_ERR  && nItem != LB_ERRSPACE)
      pList->SetItemData (nItem, (DWORD) itemPos);   // remember all about this string
    count++;
    }

   pList->SetCurSel(count - 1);
   pList->SetRedraw (TRUE);
   OnSelchangeCommands ();    // make sure item is displayed in history box
    }   // end of loading the dialog

}


BEGIN_MESSAGE_MAP(CCmdHistory, CDialog)
	//{{AFX_MSG_MAP(CCmdHistory)
	ON_LBN_DBLCLK(IDC_COMMANDS, OnDblclkCommands)
	ON_BN_CLICKED(IDC_HELPBUTTON, OnHelpbutton)
	ON_BN_CLICKED(IDC_FIND, OnFind)
	ON_BN_CLICKED(IDC_FIND_NEXT, OnFindNext)
	ON_LBN_SELCHANGE(IDC_COMMANDS, OnSelchangeCommands)
	ON_BN_CLICKED(IDC_DO, OnDo)
	ON_BN_CLICKED(IDC_NOTEPAD, OnNotepad)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCmdHistory message handlers

void CCmdHistory::OnOK() 
{
CListBox* pList = (CListBox*) GetDlgItem (IDC_COMMANDS);
ASSERT (pList);
int selection = pList->GetCurSel ();
CString str;

if (selection == LB_ERR)
  return;

POSITION pos = (POSITION) pList->GetItemData (selection);
str = m_msgList->GetAt (pos);

// check they want to wipe out their typing

if (m_sendview->CheckTyping (m_pDoc, str))
  return;

m_sendview->SetCommand (str);
	
	CDialog::OnOK();
}

void CCmdHistory::OnDblclkCommands() 
{
  OnOK ();    // same as clicking on OK button
}


void CCmdHistory::OnHelpbutton() 
{
//	  OnCmdMsg(ID_HELP, CN_COMMAND, NULL, NULL);	
	App.WinHelp(m_nIDHelp + HID_BASE_RESOURCE);
}

void CCmdHistory::OnFind() 
{
  DoFind (false);
}

void CCmdHistory::OnFindNext() 
{
  DoFind (true);
}


void CCmdHistory::DoFind (bool bAgain)
  {
CListBox* pList = (CListBox*) GetDlgItem (IDC_COMMANDS);
ASSERT (pList);

m_pHistoryFindInfo->m_bAgain = bAgain;
m_pHistoryFindInfo->m_nTotalLines = m_msgList->GetCount ();

bool found = FindRoutine (m_msgList,             // passed back to callback routines
                          *m_pHistoryFindInfo,   // finding structure
                          InitiateSearch,        // how to re-initiate a find
                          GetNextLine);          // get the next line
	
  if (found)
    pList->SetCurSel (m_pHistoryFindInfo->m_nCurrentLine);
  else
    pList->SetCurSel (-1);

  OnSelchangeCommands ();

  } // end of CCmdHistory::DoFind



void CCmdHistory::InitiateSearch (const CObject * pObject,
                                  CFindInfo & FindInfo)
  {
CStringList* pMsgList = (CStringList*) pObject;

  if (FindInfo.m_bAgain)
    FindInfo.m_pFindPosition = pMsgList->FindIndex (FindInfo.m_nCurrentLine);
  else
    if (FindInfo.m_bForwards)
      FindInfo.m_pFindPosition = pMsgList->GetHeadPosition ();
    else
      FindInfo.m_pFindPosition = pMsgList->GetTailPosition ();

  } // end of CSendView::InitiateSearch

bool CCmdHistory::GetNextLine (const CObject * pObject,
                             CFindInfo & FindInfo, 
                             CString & strLine)
  {
CStringList* pMsgList = (CStringList*) pObject;

  if (FindInfo.m_pFindPosition == NULL)
    return true;

  if (FindInfo.m_bForwards)
    strLine = pMsgList->GetNext (FindInfo.m_pFindPosition);
  else
    strLine = pMsgList->GetPrev (FindInfo.m_pFindPosition);

  return false;
  } // end of CSendView::GetNextLine

void CCmdHistory::OnSelchangeCommands() 
{
CListBox* pList = (CListBox*) GetDlgItem (IDC_COMMANDS);
ASSERT (pList);
int selection = pList->GetCurSel ();
CString str;

if (selection == LB_ERR)
  {
  m_ctlHistoryItem.SetWindowText ("");
  return;
  }

pList->GetText (selection, str);

m_ctlHistoryItem.SetWindowText (str);
	
}


void CCmdHistory::OnDo() 
{

CListBox* pList = (CListBox*) GetDlgItem (IDC_COMMANDS);
ASSERT (pList);
int selection = pList->GetCurSel ();
CString str;

if (selection == LB_ERR)
  return;

POSITION pos = (POSITION) pList->GetItemData (selection);
str = m_msgList->GetAt (pos);

m_sendview->SendCommand (str, TRUE);
	
}

void CCmdHistory::OnNotepad() 
{
CListBox* pList = (CListBox*) GetDlgItem (IDC_COMMANDS);
ASSERT (pList);
int selection = pList->GetCurSel ();
CString str;

  if (selection == LB_ERR)
    return;

  POSITION pos = (POSITION) pList->GetItemData (selection);
  str = m_msgList->GetAt (pos);

  // edit current input window
  CreateTextWindow (str,     // command
                    TFormat ("Notepad: %s", (LPCTSTR) m_pDoc->m_mush_name),     // title
                    m_pDoc,   // document
                    m_pDoc->m_iUniqueDocumentNumber,      // document number
                    m_pDoc->m_input_font_name,
                    m_pDoc->m_input_font_height,
                    m_pDoc->m_input_font_weight,
                    m_pDoc->m_input_font_charset,
                    m_pDoc->m_input_text_colour,
                    m_pDoc->m_input_background_colour,
                    "",       // search string
                    "",       // line preamble
                    false,
                    false,
                    false,
                    false,  
                    false,
                    false,
                    eNotepadNormal
                    );

  // bye-bye dialog box
	CDialog::OnOK();

}
