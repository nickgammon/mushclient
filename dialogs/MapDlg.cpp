// MapDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "..\doc.h"
#include "MapDlg.h"
#include "world_prefs\aliasdlg.h"
#include "EditDlg.h"
#include "CreditsDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMapDlg dialog


CMapDlg::CMapDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMapDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapDlg)
	m_bEnable = FALSE;
	m_bRemoveMapReverses = FALSE;
	m_strMappingFailure = _T("");
	m_bMapFailureRegexp = FALSE;
	//}}AFX_DATA_INIT
  m_pDoc = NULL;
  m_iItemCount = 0;
  m_font = NULL;

}


void CMapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapDlg)
	DDX_Control(pDX, IDC_REGEXP, m_ctlRegexp);
	DDX_Control(pDX, IDC_MAPPING_FAILURE, m_ctlFailure);
	DDX_Control(pDX, IDC_FORWARDS, m_ctlForwards);
	DDX_Control(pDX, IDC_BACKWARDS, m_ctlBackwards);
	DDX_Check(pDX, IDC_ENABLE, m_bEnable);
	DDX_Check(pDX, IDC_REMOVE_REVERSES, m_bRemoveMapReverses);
	DDX_Text(pDX, IDC_MAPPING_FAILURE, m_strMappingFailure);
	DDX_Check(pDX, IDC_REGEXP, m_bMapFailureRegexp);
	//}}AFX_DATA_MAP

   if(pDX->m_bSaveAndValidate)
     {

      // compile regular expression to check it - will raise exception if bad

    if (m_bMapFailureRegexp)
      {
      if (!CheckRegularExpression (m_strMappingFailure, 0))
        {   // failed check
        DDX_Text(pDX, IDC_MAPPING_FAILURE, m_strMappingFailure);
        pDX->Fail();
        }
      } // end of checking regular expression

     }    // end of save and validate
   else
     FixFont (m_font, m_ctlFailure, App.m_strFixedPitchFont, App.m_iFixedPitchFontSize, FW_NORMAL, DEFAULT_CHARSET);


}


BEGIN_MESSAGE_MAP(CMapDlg, CDialog)
	//{{AFX_MSG_MAP(CMapDlg)
	ON_BN_CLICKED(IDC_REMOVE_ALL, OnRemoveAll)
	ON_BN_CLICKED(IDC_REMOVE_LAST, OnRemoveLast)
	ON_BN_CLICKED(IDC_SPECIAL_MOVE, OnSpecialMove)
	ON_BN_CLICKED(IDC_MAKE_ALIAS, OnMakeAlias)
	ON_BN_CLICKED(IDC_MAKE_REVERSE_ALIAS, OnMakeReverseAlias)
	ON_BN_CLICKED(IDC_TAKE_REVERSE, OnTakeReverse)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_BN_CLICKED(IDC_CONVERT_TO_REGEXP, OnConvertToRegexp)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_REMOVE_ALL, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_REMOVE_LAST, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_MAKE_ALIAS, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_MAKE_REVERSE_ALIAS, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_TAKE_REVERSE, OnUpdateNeedEntries)
  ON_UPDATE_COMMAND_UI(IDC_CONVERT_TO_REGEXP, OnUpdateNeedNoRegexp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapDlg message handlers

void CMapDlg::OnRemoveAll() 
{

  if (m_pDoc->m_strMapList.GetCount () > 1)
    if (UMessageBox (TFormat ("Remove existing %i directions from the map?",
                         m_pDoc->m_strMapList.GetCount ()),
         MB_YESNO | MB_ICONQUESTION) != IDYES)
         return;
  
  m_pDoc->m_strMapList.RemoveAll ();	
  CalculateSpeedWalks ();

  // update status line
  m_pDoc->DrawMappingStatusLine ();

}

void CMapDlg::OnRemoveLast() 
{

if (!m_pDoc->m_strMapList.IsEmpty ())
  {
  m_pDoc->m_strMapList.RemoveTail ();
  CalculateSpeedWalks ();
  // update status line
  m_pDoc->DrawMappingStatusLine ();
  }


}


void CMapDlg::CalculateSpeedWalks (void)
  {
CString strForwards = m_pDoc->CalculateSpeedWalkString ();
CString strBackwards = CMUSHclientDoc::DoReverseSpeedwalk (strForwards);

  m_ctlForwards.SetWindowText (strForwards);
  m_ctlBackwards.SetWindowText (strBackwards);

  // remember count so we can see if it changes
  m_iItemCount =  m_pDoc->m_strMapList.GetCount ();

  }

BOOL CMapDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  CalculateSpeedWalks ();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMapDlg::OnSpecialMove() 
{

  m_pDoc->OnGameDomapperspecial ();

  CalculateSpeedWalks ();

}


LRESULT CMapDlg::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);

  // recalculate speed walk string if number of list items changes
  if (m_iItemCount != m_pDoc->m_strMapList.GetCount ())
    CalculateSpeedWalks ();

  return 0;
  } // end of CChooseNotepadDlg::OnKickIdle

void CMapDlg::OnUpdateNeedEntries(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_pDoc->m_strMapList.IsEmpty ());
}

void CMapDlg::AddAlias (CString strDirection)
  {
CAliasDlg dlg;

  dlg.m_contents = strDirection;
  dlg.m_bEnabled = TRUE;
  dlg.m_iSendTo = eSendToSpeedwalk;
  dlg.m_pAliasMap = &m_pDoc->m_AliasMap;
  dlg.m_current_alias = NULL;
  dlg.m_bTemporary = FALSE;
  dlg.m_pDoc = m_pDoc;

	if (dlg.DoModal () != IDOK)
    return;

  CAlias * alias_item = new CAlias;

  alias_item->name            = dlg.m_name;
  alias_item->contents        = dlg.m_contents;
  alias_item->bIgnoreCase     = dlg.m_bIgnoreCase;
//  alias_item->bDelayed        = dlg.m_bDelayed;
  alias_item->strLabel        = dlg.m_strLabel;
  alias_item->strProcedure    = dlg.m_strProcedure;
  alias_item->bEnabled        = dlg.m_bEnabled;
  alias_item->bExpandVariables = dlg.m_bExpandVariables;
  alias_item->bOmitFromLog    = dlg.m_bOmitFromLogFile;
  alias_item->bRegexp         = dlg.m_bRegexp;
  alias_item->bOmitFromOutput = dlg.m_bOmitFromOutput;
  alias_item->iSendTo         = dlg.m_iSendTo;
  alias_item->bMenu           = dlg.m_bMenu;
  alias_item->strGroup        = dlg.m_strGroup;

  delete alias_item->regexp;    // get rid of earlier regular expression
  alias_item->regexp = NULL;

// all aliass are now regular expressions

  CString strRegexp; 

  if (alias_item->bRegexp)
    strRegexp = alias_item->name;
  else
    strRegexp = ConvertToRegularExpression (alias_item->name);

  alias_item->regexp = new t_regexp (strRegexp,
                                  (alias_item->bIgnoreCase  ? PCRE_CASELESS : 0)
#if ALIASES_USE_UTF8
                                  | (m_pDoc->m_bUTF_8 ? PCRE_UTF8 : 0)
#endif // ALIASES_USE_UTF8
                                  );

// add to map - generate a name if it doesn't have one

  CString strAliasName = alias_item->strLabel;
  if (strAliasName.IsEmpty ())
    strAliasName.Format ("*alias%s", (LPCTSTR) App.GetUniqueString ());
  else
    strAliasName.MakeLower ();

  m_pDoc->m_AliasMap.SetAt (strAliasName, alias_item);

  m_pDoc->SortAliases ();

  }


void CMapDlg::OnMakeAlias() 
{
AddAlias (GetText (m_ctlForwards));
}    

void CMapDlg::OnMakeReverseAlias() 
{
AddAlias (GetText (m_ctlBackwards));
}


void CMapDlg::OnTakeReverse() 
{
 m_pDoc->SendMsg (m_pDoc->DoEvaluateSpeedwalk (GetText (m_ctlBackwards)),
                  m_pDoc->m_display_my_input,
                  true,
                  m_pDoc->LoggingInput ());
}

void CMapDlg::OnEdit() 
{
CEditDlg dlg;

  dlg.m_strText = GetText (m_ctlFailure);

  dlg.m_strTitle = Translate ("Edit mapping failure 'match' text");

  dlg.m_bRegexp = m_ctlRegexp.GetCheck ();

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlFailure.SetWindowText (dlg.m_strText);
	
}

void CMapDlg::OnConvertToRegexp() 
{
CString strText = GetText (m_ctlFailure);

// check for foolishly using ** in a non-regular expression
if (strText.Find ("**") != -1 && m_ctlRegexp.GetCheck () == 0)
  {
  CCreditsDlg dlg;

  dlg.m_iResourceID = IDR_MULTIPLE_ASTERISKS;
  dlg.m_strTitle = "Warning";

  dlg.DoModal ();	

  return;
  }

 strText = ConvertToRegularExpression (strText);

 m_ctlFailure.SetWindowText (strText);

 m_ctlRegexp.SetCheck (TRUE);
	
}

void CMapDlg::OnUpdateNeedNoRegexp(CCmdUI* pCmdUI)
  {

  pCmdUI->Enable (!m_ctlRegexp.GetCheck ());

  }   // end of CMapDlg::OnUpdateNeedNoRegexp
