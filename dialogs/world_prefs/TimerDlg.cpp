// TimerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\mushclient.h"
#include "..\..\doc.h"
#include "..\world_prefs\TimerDlg.h"
#include "..\EditDlg.h"
#include "..\EditMultiLine.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTimerDlg dialog


IMPLEMENT_DYNAMIC(CTimerDlg, CDialog)

CTimerDlg::CTimerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTimerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTimerDlg)
	m_bEnabled = FALSE;
	m_strLabel = _T("");
	m_iAtHour = 0;
	m_iAtMinute = 0;
	m_iEveryHour = 0;
	m_iEveryMinute = 0;
	m_iOffsetHour = 0;
	m_iOffsetMinute = 0;
	m_iType = -1;
	m_strProcedure = _T("");
	m_bOneShot = FALSE;
	m_bTemporary = FALSE;
	m_bActiveWhenClosed = FALSE;
	m_strGroup = _T("");
	m_iSendTo = -1;
	m_strVariable = _T("");
	m_strIncluded = _T("");
	m_bOmitFromOutput = FALSE;
	m_bOmitFromLog = FALSE;
	m_fAtSecond = 0.0;
	m_fEverySecond = 0.0;
	m_fOffsetSecond = 0.0;
	//}}AFX_DATA_INIT

  m_current_timer = NULL;
  m_pTimerMap = NULL;
  m_pDoc = NULL;

}


void CTimerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

  if(!pDX->m_bSaveAndValidate)
    {
    if (m_current_timer && m_current_timer->bIncluded)
      m_strIncluded = "(included)";
    else 
      m_strIncluded.Empty ();
    }

	//{{AFX_DATA_MAP(CTimerDlg)
	DDX_Control(pDX, IDC_SEND_TO, m_ctlSendTo);
	DDX_Control(pDX, IDC_SCRIPT_NAME, m_ctlProcedure);
	DDX_Control(pDX, IDC_TIMER_CONTENTS, m_ctlContents);
	DDX_Control(pDX, IDC_BUTTON_AT, m_ctlButtonAt);
	DDX_Check(pDX, IDC_ENABLED, m_bEnabled);
	DDX_Text(pDX, IDC_TIMER_LABEL, m_strLabel);
	DDX_Text(pDX, IDC_AT_HOUR, m_iAtHour);
	DDV_MinMaxInt(pDX, m_iAtHour, 0, 23);
	DDX_Text(pDX, IDC_AT_MINUTE, m_iAtMinute);
	DDV_MinMaxInt(pDX, m_iAtMinute, 0, 59);
	DDX_Text(pDX, IDC_INTERVAL_HOUR, m_iEveryHour);
	DDV_MinMaxInt(pDX, m_iEveryHour, 0, 23);
	DDX_Text(pDX, IDC_INTERVAL_MINUTE, m_iEveryMinute);
	DDV_MinMaxInt(pDX, m_iEveryMinute, 0, 59);
	DDX_Text(pDX, IDC_OFFSET_HOUR, m_iOffsetHour);
	DDV_MinMaxInt(pDX, m_iOffsetHour, 0, 23);
	DDX_Text(pDX, IDC_OFFSET_MINUTE, m_iOffsetMinute);
	DDV_MinMaxInt(pDX, m_iOffsetMinute, 0, 59);
	DDX_MinMaxString(pDX, IDC_TIMER_CONTENTS, m_strContents);
	DDV_MinMaxString(pDX, m_strContents, 0, 32000);
	DDX_Radio(pDX, IDC_BUTTON_EVERY, m_iType);
	DDX_Text(pDX, IDC_SCRIPT_NAME, m_strProcedure);
	DDX_Check(pDX, IDC_ONE_SHOT_TIMER, m_bOneShot);
	DDX_Check(pDX, IDC_TEMPORARY, m_bTemporary);
	DDX_Check(pDX, IDC_ACTIVE_WHEN_CLOSED, m_bActiveWhenClosed);
	DDX_Text(pDX, IDC_GROUP, m_strGroup);
	DDX_CBIndex(pDX, IDC_SEND_TO, m_iSendTo);
	DDX_Text(pDX, IDC_VARIABLE, m_strVariable);
	DDX_Text(pDX, IDC_INCLUDED, m_strIncluded);
	DDX_Check(pDX, IDC_OMIT_FROM_OUTPUT, m_bOmitFromOutput);
	DDX_Check(pDX, IDC_OMIT_FROM_LOG, m_bOmitFromLog);
	DDX_Text(pDX, IDC_AT_SECOND, m_fAtSecond);
	DDV_MinMaxDouble(pDX, m_fAtSecond, 0., 59.99999);
	DDX_Text(pDX, IDC_INTERVAL_SECOND, m_fEverySecond);
	DDV_MinMaxDouble(pDX, m_fEverySecond, 0., 59.99999);
	DDX_Text(pDX, IDC_OFFSET_SECOND, m_fOffsetSecond);
	DDV_MinMaxDouble(pDX, m_fOffsetSecond, 0., 59.99999);
	//}}AFX_DATA_MAP

  if(pDX->m_bSaveAndValidate)
    {

    
    m_strLabel.TrimLeft ();
    m_strLabel.TrimRight ();

    m_strProcedure.TrimLeft ();
    m_strProcedure.TrimRight ();

    m_strGroup.TrimLeft ();
    m_strGroup.TrimRight ();

    if (m_iType == CTimer::eInterval)
      {
      CmcDateTimeSpan ts1 (0, m_iEveryHour, m_iEveryMinute, m_fEverySecond);
      CmcDateTimeSpan ts2 (0, m_iOffsetHour, m_iOffsetMinute, m_fOffsetSecond);

      if (ts1 <= CmcDateTimeSpan (0, 0, 0, 0))
          {
          ::TMessageBox("The timer interval must be greater than zero.");
          DDX_Text(pDX, IDC_INTERVAL_HOUR, m_iEveryHour);
          pDX->Fail();
          return;
          }     // end of interval <= 0

      if(ts2 >= ts1)
          {
          ::TMessageBox("The timer offset must be less than the timer period.");
          DDX_Text(pDX, IDC_OFFSET_HOUR, m_iOffsetHour);
          pDX->Fail();
          return;
          }     // end of offset >= period
        }   // end of doing a periodical timer


    CString strTimerName;
    CTimer * timer_item;
    POSITION pos;

    for (pos = m_pTimerMap->GetStartPosition (); pos; )
      {
      m_pTimerMap->GetNextAssoc (pos, strTimerName, timer_item);

// don't compare against itself

      if (timer_item == m_current_timer)
        continue;

// now check the label for duplicates

      if (!m_strLabel.IsEmpty ())    // we can have duplicate blank names
        if (m_strLabel.CompareNoCase (timer_item->strLabel) == 0)
          {
          CString strMsg;
          strMsg = TFormat ("The timer label \"%s\" is already in the list of timers.",
                          (LPCTSTR) m_strLabel);
          ::UMessageBox(strMsg);
          DDX_Text(pDX, IDC_TIMER_LABEL, m_strLabel);
          pDX->Fail();
          return;
          }

      }   // end of checking each Timer

    if (!m_strLabel.IsEmpty ())    // we can have blank labels
      {

// check label is valid

      if (CheckLabel (m_strLabel))
        {
        ::TMessageBox ("The label must start with a letter and consist of letters"
                        ", numbers or the underscore character.");
        DDX_Text(pDX, IDC_TIMER_LABEL, m_strLabel);
        pDX->Fail();
        return;
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

    if (m_iSendTo == eSendToSpeedwalk)
      {

      CString strResult = m_pDoc->DoEvaluateSpeedwalk (m_strContents);

      if (!strResult.IsEmpty ())
        {
        if (strResult [0] == '*')    // error in speedwalk string?
          {
          ::UMessageBox (strResult.Mid (1));  // already translated, I think
          DDX_Text(pDX, IDC_TIMER_CONTENTS, m_strContents);
          pDX->Fail();
          }   // end of error message
        } // end of non-empty speedwalk          
      }   // end of speed walking wanted

    if(m_strContents.IsEmpty () && m_strProcedure.IsEmpty ())
        {
        ::TMessageBox("The timer contents cannot be blank unless you specify a script subroutine.");
        DDX_Text(pDX, IDC_TIMER_CONTENTS, m_strContents);
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

    }   // end of saving and validating

}


BEGIN_MESSAGE_MAP(CTimerDlg, CDialog)
	//{{AFX_MSG_MAP(CTimerDlg)
	ON_BN_CLICKED(IDC_BUTTON_AT, OnButtonAt)
	ON_BN_CLICKED(IDC_BUTTON_EVERY, OnButtonEvery)
	ON_BN_CLICKED(IDC_HELPBUTTON, OnHelpbutton)
	ON_BN_CLICKED(IDC_EDIT_SEND, OnEditSend)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOK)
  ON_UPDATE_COMMAND_UI(IDC_INVOCATIONS, OnUpdateInvocationCount)
  ON_UPDATE_COMMAND_UI(IDC_MATCHES, OnUpdateMatchCount)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimerDlg message handlers

void CTimerDlg::OnButtonAt() 
{
BOOL bAt = IsDlgButtonChecked(IDC_BUTTON_AT);

  GetDlgItem (IDC_AT_HOUR)->EnableWindow (bAt);
  GetDlgItem (IDC_AT_MINUTE)->EnableWindow (bAt);
  GetDlgItem (IDC_AT_SECOND)->EnableWindow (bAt);

  GetDlgItem (IDC_INTERVAL_HOUR)->EnableWindow (!bAt);
  GetDlgItem (IDC_INTERVAL_MINUTE)->EnableWindow (!bAt);
  GetDlgItem (IDC_INTERVAL_SECOND)->EnableWindow (!bAt);
  
  GetDlgItem (IDC_OFFSET_HOUR)->EnableWindow (!bAt);
  GetDlgItem (IDC_OFFSET_MINUTE)->EnableWindow (!bAt);
  GetDlgItem (IDC_OFFSET_SECOND)->EnableWindow (!bAt);
}

void CTimerDlg::OnButtonEvery() 
{
  OnButtonAt ();    // same test	
}

void CTimerDlg::OnHelpbutton() 
{
//	  OnCmdMsg(ID_HELP, CN_COMMAND, NULL, NULL);	
	App.WinHelp(m_nIDHelp + HID_BASE_RESOURCE);
}

BOOL CTimerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  OnButtonAt ();    // fix up enable or disable of various edit controls

// fix up spin controls
  
	((CSpinButtonCtrl*) GetDlgItem(IDC_SPIN_AT_HOUR))->SetRange(0, 23);
	((CSpinButtonCtrl*) GetDlgItem(IDC_SPIN_EVERY_HOUR))->SetRange(0, 23);
	((CSpinButtonCtrl*) GetDlgItem(IDC_SPIN_OFFSET_HOUR))->SetRange(0, 23);

	((CSpinButtonCtrl*) GetDlgItem(IDC_SPIN_AT_MINUTE))->SetRange(0, 59);
	((CSpinButtonCtrl*) GetDlgItem(IDC_SPIN_EVERY_MINUTE))->SetRange(0, 59);
	((CSpinButtonCtrl*) GetDlgItem(IDC_SPIN_OFFSET_MINUTE))->SetRange(0, 59);

  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


LRESULT CTimerDlg::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CTimerDlg::OnKickIdle

void CTimerDlg::OnUpdateOK(CCmdUI* pCmdUI)
  {

  // cannot modify included timers
  if (m_current_timer && m_current_timer->bIncluded)
    {
    pCmdUI->Enable (FALSE);
    return;
    }

CString strContents;
CString strProcedure;

  m_ctlContents.GetWindowText (strContents);

  m_ctlProcedure.GetWindowText (strProcedure);

  pCmdUI->Enable (!strContents.IsEmpty () || !strProcedure.IsEmpty ());

  } // end of CTimerDlg::OnUpdateOK

void CTimerDlg::OnUpdateInvocationCount(CCmdUI* pCmdUI)
  {

  if (m_current_timer)
    {
    CString strMessage;

    strMessage.Format ("%ld call%s.", 
                        m_current_timer->nInvocationCount,
                        m_current_timer->nInvocationCount == 1 ? "" : "s");

    pCmdUI->SetText (strMessage);
    }   // end of having a Timer item

  } // end of CTimerDlg::OnUpdateInvocationCount

void CTimerDlg::OnUpdateMatchCount(CCmdUI* pCmdUI)
  {

  if (m_current_timer)
    {
    CString strMessage;

    strMessage.Format ("Fired %ld time%s.", 
                        m_current_timer->nMatched,
                        m_current_timer->nMatched == 1 ? "" : "s");

    pCmdUI->SetText (strMessage);
    }   // end of having a Timer item

  } // end of CTimerDlg::OnUpdateMatchCount

void CTimerDlg::OnEditSend() 
{
CEditMultiLine dlg;

  dlg.m_strText = GetText (m_ctlContents);

  dlg.m_strTitle = "Edit timer 'send' text";

  m_iSendTo = m_ctlSendTo.GetCurSel ();

  if (m_iSendTo == eSendToScript || m_iSendTo == eSendToScriptAfterOmit)
    dlg.m_bScript = true;

  if (m_pDoc->GetScriptEngine () && m_pDoc->GetScriptEngine ()->L)
    dlg.m_bLua = true;

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlContents.SetWindowText (dlg.m_strText);
	
}

