// aliasdlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\mushclient.h"
#include "..\..\doc.h"
#include "aliasdlg.h"
#include "..\EditDlg.h"
#include "..\EditMultiLine.h"
#include "..\CreditsDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAliasDlg dialog


IMPLEMENT_DYNAMIC(CAliasDlg, CDialog)

CAliasDlg::CAliasDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAliasDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAliasDlg)
	m_contents = _T("");
	m_name = _T("");
	m_strLabel = _T("");
	m_bEnabled = FALSE;
	m_strProcedure = _T("");
	m_bIgnoreCase = FALSE;
	m_bExpandVariables = FALSE;
	m_bOmitFromLogFile = FALSE;
	m_bRegexp = FALSE;
	m_bOmitFromOutput = FALSE;
	m_bMenu = FALSE;
	m_bTemporary = FALSE;
	m_strGroup = _T("");
	m_strVariable = _T("");
	m_iSendTo = -1;
	m_bKeepEvaluating = FALSE;
	m_iSequence = 0;
	m_bEchoAlias = FALSE;
	m_bOmitFromCommandHistory = FALSE;
	m_strIncluded = _T("");
	m_strRegexpError = _T("");
	m_bOneShot = FALSE;
	//}}AFX_DATA_INIT

  m_pAliasMap = NULL;
  m_current_alias = NULL;
  m_font = NULL;
  m_font2 = NULL;
  m_pDoc = NULL;

}


void CAliasDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

  if(!pDX->m_bSaveAndValidate)
    {
    if (m_current_alias && m_current_alias->bIncluded)
      m_strIncluded = "(included)";
    else 
      m_strIncluded.Empty ();
    }

	//{{AFX_DATA_MAP(CAliasDlg)
	DDX_Control(pDX, IDC_SEND_TO, m_ctlSendTo);
	DDX_Control(pDX, IDC_VARIABLE, m_ctlVariable);
	DDX_Control(pDX, IDC_REGEXP, m_ctlRegexp);
	DDX_Control(pDX, IDC_ALIAS_CONTENTS, m_ctlAliasContents);
	DDX_Control(pDX, IDC_ALIAS_NAME, m_ctlAliasName);
	DDX_Text(pDX, IDC_ALIAS_CONTENTS, m_contents);
	DDX_Text(pDX, IDC_ALIAS_NAME, m_name);
	DDX_Text(pDX, IDC_ALIAS_LABEL, m_strLabel);
	DDX_Check(pDX, IDC_ENABLED, m_bEnabled);
	DDX_Text(pDX, IDC_SCRIPT_NAME, m_strProcedure);
	DDX_Check(pDX, IDC_IGNORE_CASE, m_bIgnoreCase);
	DDX_Check(pDX, IDC_EXPAND_VARIABLES, m_bExpandVariables);
	DDX_Check(pDX, IDC_OMIT_FROM_LOG, m_bOmitFromLogFile);
	DDX_Check(pDX, IDC_REGEXP, m_bRegexp);
	DDX_Check(pDX, IDC_OMIT_FROM_OUTPUT, m_bOmitFromOutput);
	DDX_Check(pDX, IDC_ALIAS_MENU, m_bMenu);
	DDX_Check(pDX, IDC_TEMPORARY_ALIAS, m_bTemporary);
	DDX_Text(pDX, IDC_GROUP, m_strGroup);
	DDX_Text(pDX, IDC_VARIABLE, m_strVariable);
	DDX_CBIndex(pDX, IDC_SEND_TO, m_iSendTo);
	DDX_Check(pDX, IDC_KEEP_EVALUATING, m_bKeepEvaluating);
	DDX_Text(pDX, IDC_SEQUENCE, m_iSequence);
	DDV_MinMaxInt(pDX, m_iSequence, 0, 10000);
	DDX_Check(pDX, IDC_ECHO_ALIAS, m_bEchoAlias);
	DDX_Check(pDX, IDC_OMIT_FROM_COMMAND_HISTORY, m_bOmitFromCommandHistory);
	DDX_Text(pDX, IDC_INCLUDED, m_strIncluded);
	DDX_Text(pDX, IDC_REGEXP_ERROR, m_strRegexpError);
	DDX_Check(pDX, IDC_ONE_SHOT, m_bOneShot);
	//}}AFX_DATA_MAP

 if(pDX->m_bSaveAndValidate)
   {
    POSITION pos;
    CAlias * alias_item;

    m_strLabel.TrimLeft ();
    m_strLabel.TrimRight ();

    m_strProcedure.TrimLeft ();
    m_strProcedure.TrimRight ();

    m_strGroup.TrimLeft ();
    m_strGroup.TrimRight ();

    m_strVariable.TrimLeft ();
    m_strVariable.TrimRight ();

    if (m_name.IsEmpty ())
        {
        ::TMessageBox("The alias cannot be blank.");
        DDX_Text(pDX, IDC_ALIAS_NAME, m_name);
        pDX->Fail();
        }     // end of alias being blank

    // check for foolishly using ** in a non-regular expression
    if (m_name.Find ("**") != -1 && !m_bRegexp)
      {
      CCreditsDlg dlg;

      dlg.m_iResourceID = IDR_MULTIPLE_ASTERISKS;
      dlg.m_strTitle = "Warning";

      dlg.DoModal ();	

      DDX_Text(pDX, IDC_ALIAS_NAME, m_name);
      pDX->Fail();

      }

    int iInvalidPos = m_name.FindOneOf ("\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"); 

    if (iInvalidPos != -1)
        {
        char c = m_name [iInvalidPos];

        ::UMessageBox(TFormat (
           "The alias 'match' text contains an invalid non-printable character (hex %02X) at position %i.", c, iInvalidPos + 1), MB_ICONSTOP);
        DDX_Text(pDX, IDC_ALIAS_NAME, m_name);
        pDX->Fail();
        }     // end of alias match having weird characters in it


    // we allow carriage-return, linefeed, tab here
    iInvalidPos = m_contents.FindOneOf ("\x01\x02\x03\x04\x05\x06\x07\x08\x0B\x0C\x0E\x0F"); 

    if (iInvalidPos != -1)
        {
        char c = m_contents [iInvalidPos];

        ::UMessageBox(TFormat (
           "The alias 'send' text contains an invalid non-printable character (hex %02X) at position %i.", c, iInvalidPos + 1), MB_ICONSTOP);
        DDX_Text(pDX, IDC_ALIAS_CONTENTS, m_contents);
        pDX->Fail();
        }     // end of alias send having weird characters in it


    // compile regular expression to check it
    if (m_bRegexp)
      {
      if (!CheckRegularExpression (m_name, (m_bIgnoreCase ?  0 : PCRE_CASELESS) 
          | (m_pDoc->m_bUTF_8 ? PCRE_UTF8 : 0)
#if ALIASES_USE_UTF8
                                  | (m_pDoc->m_bUTF_8 ? PCRE_UTF8 : 0)
#endif // ALIASES_USE_UTF8
          
          ))
        {   // failed check
        DDX_Text(pDX, IDC_ALIAS_NAME, m_name);
        pDX->Fail();
        }
      } // end of checking regular expression

    CString strAliasName;

    for (pos = m_pAliasMap->GetStartPosition (); pos; )
      {
      m_pAliasMap->GetNextAssoc (pos, strAliasName, alias_item);

// don't compare against itself

      if (alias_item == m_current_alias)
        continue;

      /*
      
      bool bDuplicate = false;

      // if either one is case-insensitive, compare insensitive
      if ((alias_item->bIgnoreCase || m_bIgnoreCase) &&
           alias_item->name.CompareNoCase (m_name) == 0)
            bDuplicate = true; 
      // if both are case-sensitive, compare exactly
      else if (alias_item->name == m_name)
            bDuplicate = true; 

      // we won't consider it a match if they send to different places
      if (alias_item->iSendTo != m_iSendTo)
         bDuplicate = false;

      if (bDuplicate)
        {
        ::TMessageBox("This alias is already in the list of aliases.");
        DDX_Text(pDX, IDC_ALIAS_NAME, m_name);
        pDX->Fail();
        }

         */

// now check the label for duplicates

      if (!m_strLabel.IsEmpty ())    // we can have duplicate blank names
        if (m_strLabel.CompareNoCase (alias_item->strLabel) == 0)
          {
          CString strMsg;
          strMsg = TFormat ("The alias label \"%s\" is already in the list of aliases.",
                          (LPCTSTR) m_strLabel);
          ::UMessageBox(strMsg);
          DDX_Text(pDX, IDC_ALIAS_LABEL, m_strLabel);
          pDX->Fail();
          }

      }   // end of checking each alias

    if (!m_strLabel.IsEmpty ())    // we can have blank labels
      {

// check label is valid

      if (CheckLabel (m_strLabel))
        {
        ::TMessageBox("The label must start with a letter and consist of letters"
                        ", numbers or the underscore character.");
        DDX_Text(pDX, IDC_ALIAS_LABEL, m_strLabel);
        pDX->Fail();
        }

      }   // end of having non-blank label

    if (m_strVariable.IsEmpty ())    // we can have blank variables
      {
      if (m_iSendTo == eSendToVariable)
        {
        ::TMessageBox("When sending to a variable you must specify a variable name. ",
                        MB_ICONSTOP);
        DDX_Text(pDX, IDC_VARIABLE, m_strVariable);
        pDX->Fail();
        }
      }
    else
      {

// check variable name is valid

      if (CheckLabel (m_strVariable))
        {
        ::TMessageBox("The variable name must start with a letter and consist of letters"
                        ", numbers or the underscore character.");
        DDX_Text(pDX, IDC_VARIABLE, m_strVariable);
        pDX->Fail();
        }

      }   // end of having non-blank variable

    // check for speed walking OK, unless they are substituting

    if (!m_bExpandVariables || m_contents.Find ('@') == -1)
      if (m_iSendTo == eSendToSpeedwalk && m_contents.Find ('%') == -1)
        {

        CString strResult = m_pDoc->DoEvaluateSpeedwalk (m_contents);

        if (!strResult.IsEmpty ())
          {
          if (strResult [0] == '*')    // error in speedwalk string?
            {
            ::UMessageBox (strResult.Mid (1));
            DDX_Text(pDX, IDC_ALIAS_CONTENTS, m_contents);
            pDX->Fail();
            }   // end of error message
          } // end of non-empty speedwalk          
        }   // end of speed walking wanted


    if(m_contents.IsEmpty () && m_strProcedure.IsEmpty ())
        {
        ::TMessageBox("The alias contents cannot be blank unless you specify a script subroutine.");
        DDX_Text(pDX, IDC_ALIAS_CONTENTS, m_contents);
        pDX->Fail();
        return;
        }     // end of contents being blank

    
    if (!m_strProcedure.IsEmpty ())    // blank procedure is OK
      {

// check procedure is valid

      if (CheckLabel (m_strProcedure, true))
        {
        ::TMessageBox("The script subroutine name must start with a letter and consist of letters"
                        ", numbers or the underscore character.");
        DDX_Text(pDX, IDC_SCRIPT_NAME, m_strProcedure);
        pDX->Fail();
        return;
        }

      }   // end of having non-blank procedure

   } // end of saving and validating
  else
    if (App.m_bFixedFontForEditing)
      {     
      FixFont (m_font, m_ctlAliasName, App.m_strFixedPitchFont, App.m_iFixedPitchFontSize, FW_NORMAL, DEFAULT_CHARSET);
      FixFont (m_font2, m_ctlAliasContents, App.m_strFixedPitchFont, App.m_iFixedPitchFontSize, FW_NORMAL, DEFAULT_CHARSET);
      }

}


BEGIN_MESSAGE_MAP(CAliasDlg, CDialog)
	//{{AFX_MSG_MAP(CAliasDlg)
	ON_BN_CLICKED(IDC_HELPBUTTON, OnHelpbutton)
	ON_BN_CLICKED(IDC_REVERSE, OnReverse)
	ON_BN_CLICKED(IDC_EDIT2, OnEdit2)
	ON_BN_CLICKED(IDC_CONVERT_TO_REGEXP, OnConvertToRegexp)
	ON_BN_CLICKED(IDC_EDIT_SEND, OnEditSend)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOK)
  ON_UPDATE_COMMAND_UI(IDC_INVOCATIONS, OnUpdateInvocationCount)
  ON_UPDATE_COMMAND_UI(IDC_MATCHES, OnUpdateMatchCount)
  ON_UPDATE_COMMAND_UI(IDC_ALIAS_MENU, OnUpdateProcedure)
  ON_UPDATE_COMMAND_UI(IDC_REVERSE, OnUpdateReverse)
//  ON_UPDATE_COMMAND_UI(IDC_DELAYED, OnUpdateDelayed)
  ON_UPDATE_COMMAND_UI(IDC_CONVERT_TO_REGEXP, OnUpdateNeedNoRegexp)
  
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAliasDlg message handlers

void CAliasDlg::OnHelpbutton() 
{
//	  OnCmdMsg(ID_HELP, CN_COMMAND, NULL, NULL);	
	App.WinHelp(m_nIDHelp + HID_BASE_RESOURCE);
}


LRESULT CAliasDlg::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CAliasDlg::OnKickIdle

void CAliasDlg::OnUpdateOK(CCmdUI* pCmdUI)
  {

CString strAlias;

  // cannot modify included aliases
  if (m_current_alias && m_current_alias->bIncluded)
    {
    pCmdUI->Enable (FALSE);
    return;
    }

  GetDlgItemText (IDC_ALIAS_NAME, strAlias);

  pCmdUI->Enable (!strAlias.IsEmpty ());
  } // end of CAliasDlg::OnUpdateOK


void CAliasDlg::OnUpdateInvocationCount(CCmdUI* pCmdUI)
  {

  if (m_current_alias)
    {
    CString strMessage;

    strMessage.Format ("%ld call%s.", 
                        m_current_alias->nInvocationCount,
                        m_current_alias->nInvocationCount == 1 ? "" : "s");

    pCmdUI->SetText (strMessage);
    }   // end of having an Alias item

  } // end of CAliasDlg::OnUpdateInvocationCount

void CAliasDlg::OnUpdateMatchCount(CCmdUI* pCmdUI)
  {

  if (m_current_alias)
    {
    CString strMessage;

    strMessage.Format ("%ld match%s.", PLURALES (m_current_alias->nMatched));

    pCmdUI->SetText (strMessage);
    }   // end of having an Alias item

  } // end of CAliasDlg::OnUpdateMatchCount

void CAliasDlg::OnUpdateProcedure(CCmdUI* pCmdUI)
  {

CString strLabel;

  GetDlgItemText (IDC_ALIAS_LABEL, strLabel);

  pCmdUI->Enable (!strLabel.IsEmpty ());

  }   // end of CAliasDlg::OnUpdateProcedure

void CAliasDlg::OnUpdateReverse(CCmdUI* pCmdUI)
  {
CString strContents = GetText (m_ctlAliasContents);

int iSelection = m_ctlSendTo.GetCurSel ();

  if (iSelection == CB_ERR)
    return;

bool bSpeedWalk = iSelection == eSendToSpeedwalk;

// don't bother if there are substitution characters in it
  pCmdUI->Enable (bSpeedWalk && 
                 !strContents.IsEmpty () &&
                 strContents.Find ('%') == -1);

  }   // end of CAliasDlg::OnUpdateReverse

void CAliasDlg::OnReverse() 
{
CString strContents = GetText (m_ctlAliasContents);

  strContents =  m_pDoc->DoReverseSpeedwalk (strContents);

  if (!strContents.IsEmpty ())
    {
    if (strContents [0] == '*')    // error in speedwalk string?
      {
      ::UMessageBox (strContents.Mid (1));
      return;
      }   // end of error message
    } // end of non-empty speedwalk          

 // set the new contents
 m_ctlAliasContents.SetWindowText (strContents);

}

void CAliasDlg::OnEdit2() 
{
CEditDlg dlg;

  dlg.m_strText = GetText (m_ctlAliasName);

  dlg.m_strTitle = Translate ("Edit alias 'match' text");

  dlg.m_bRegexp = m_ctlRegexp.GetCheck ();

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlAliasName.SetWindowText (dlg.m_strText);
	
}


void CAliasDlg::OnConvertToRegexp() 
{
CString strText = GetText (m_ctlAliasName);

// check for foolishly using ** in a non-regular expression
if (strText.Find ("**") != -1 && !m_bRegexp)
  {
  CCreditsDlg dlg;

  dlg.m_iResourceID = IDR_MULTIPLE_ASTERISKS;
  dlg.m_strTitle = "Warning";

  dlg.DoModal ();	

  return;
  }

 strText = ConvertToRegularExpression (strText);

 m_ctlAliasName.SetWindowText (strText);

 m_ctlRegexp.SetCheck (TRUE);
	
}


void CAliasDlg::OnUpdateNeedNoRegexp(CCmdUI* pCmdUI)
  {

  pCmdUI->Enable (!m_ctlRegexp.GetCheck ());

  }   // end of CAliasDlg::OnUpdateNeedNoRegexp

void CAliasDlg::OnEditSend() 
{
CEditMultiLine dlg;

  dlg.m_strText = GetText (m_ctlAliasContents);

  dlg.m_strTitle = "Edit alias 'send' text";

  m_iSendTo = m_ctlSendTo.GetCurSel ();

  if (m_iSendTo == eSendToScript || m_iSendTo == eSendToScriptAfterOmit)
    dlg.m_bScript = true;

  if (m_pDoc->GetScriptEngine () && m_pDoc->GetScriptEngine ()->L)
    dlg.m_bLua = true;

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlAliasContents.SetWindowText (dlg.m_strText);
	
}

