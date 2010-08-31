// triggdlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\mushclient.h"
#include "..\..\doc.h"
#include "..\..\mainfrm.h"
#include "..\EditDlg.h"
#include "..\EditMultiLine.h"
#include "..\CreditsDlg.h"

#include "triggdlg.h"
#include "..\ColourPickerDlg.h"

#include "mmsystem.h"   // for sound playing

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTriggerDlg dialog

IMPLEMENT_DYNAMIC(CTriggerDlg, CDialog)

CTriggerDlg::CTriggerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTriggerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTriggerDlg)
	m_ignore_case = FALSE;
	m_trigger = _T("");
	m_contents = _T("");
	m_colour = -1;
	m_sound_pathname = _T("");
	m_omit_from_log = FALSE;
	m_bEnabled = FALSE;
	m_bOmitFromOutput = FALSE;
	m_bKeepEvaluating = FALSE;
	m_strLabel = _T("");
	m_iClipboardArg = -1;
	m_strProcedure = _T("");
	m_bRegexp = FALSE;
	m_bRepeat = FALSE;
	m_iSequence = 0;
	m_bBold = FALSE;
	m_bItalic = FALSE;
	m_bUnderline = FALSE;
	m_bSoundIfInactive = FALSE;
	m_iMatchTextColour = -1;
	m_iMatchBackColour = -1;
	m_iMatchBold = FALSE;
	m_iMatchItalic = FALSE;
	m_iMatchInverse = FALSE;
	m_iSendTo = -1;
	m_bExpandVariables = FALSE;
	m_bTemporary = FALSE;
	m_bLowercaseWildcard = FALSE;
	m_strGroup = _T("");
	m_strVariable = _T("");
	m_strIncluded = _T("");
	m_bMultiLine = FALSE;
	m_iLinesToMatch = 0;
	m_iColourChangeType = -1;
	m_strRegexpError = _T("");
	m_bOneShot = FALSE;
	//}}AFX_DATA_INIT

  m_pTriggerMap = NULL;
  m_current_trigger = NULL;
  m_bUTF_8 = false;

  m_font = NULL;
  m_font2 = NULL;

// default to black on white in case they never set them
  for (int i = 0; i < MAX_CUSTOM; i++)
    {
    m_customtext [i] = RGB (0, 0, 0);;
    m_customback [i] = RGB (255, 255, 255);;
    }

  m_pDoc = NULL;

}


void CTriggerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

  if(!pDX->m_bSaveAndValidate)
    {
    if (m_current_trigger && m_current_trigger->bIncluded)
      m_strIncluded = "(included)";
    else 
      m_strIncluded.Empty ();
    }

	//{{AFX_DATA_MAP(CTriggerDlg)
	DDX_Control(pDX, IDC_VARIABLE_LABEL, m_ctlVariableLabel);
	DDX_Control(pDX, IDC_LINES_TO_MATCH, m_ctlLinesToMatch);
	DDX_Control(pDX, IDC_MULTI_LINE, m_ctlMultiLine);
	DDX_Control(pDX, IDC_VARIABLE, m_ctlVariable);
	DDX_Control(pDX, IDC_SEND_TO, m_ctlSendTo);
	DDX_Control(pDX, IDC_TRIGGER_CONTENTS, m_ctlTriggerContents);
	DDX_Control(pDX, IDC_TRIGGER, m_ctlTrigger);
	DDX_Control(pDX, IDC_TRIGGER_COLOUR, m_ctlTriggerColour);
	DDX_Control(pDX, IDC_REGEXP, m_ctlRegexp);
	DDX_Control(pDX, IDC_TRIGGER_SWATCH2, m_ctlTriggerSwatch2);
	DDX_Control(pDX, IDC_TRIGGER_SWATCH, m_ctlTriggerSwatch);
	DDX_Check(pDX, IDC_IGNORE_CASE, m_ignore_case);
	DDX_Text(pDX, IDC_TRIGGER, m_trigger);
	DDX_Text(pDX, IDC_TRIGGER_CONTENTS, m_contents);
	DDX_CBIndex(pDX, IDC_TRIGGER_COLOUR, m_colour);
	DDX_Text(pDX, IDC_SOUND_PATH, m_sound_pathname);
	DDX_Check(pDX, IDC_OMIT_FROM_LOG, m_omit_from_log);
	DDX_Check(pDX, IDC_ENABLED, m_bEnabled);
	DDX_Check(pDX, IDC_OMIT_FROM_OUTPUT, m_bOmitFromOutput);
	DDX_Check(pDX, IDC_KEEP_EVALUATING, m_bKeepEvaluating);
	DDX_Text(pDX, IDC_TRIGGER_NAME, m_strLabel);
	DDX_CBIndex(pDX, IDC_WILDCARD_CLIPBOARD, m_iClipboardArg);
	DDX_Text(pDX, IDC_SCRIPT_NAME, m_strProcedure);
	DDX_Check(pDX, IDC_REGEXP, m_bRegexp);
	DDX_Check(pDX, IDC_REPEAT, m_bRepeat);
	DDX_Text(pDX, IDC_SEQUENCE, m_iSequence);
	DDV_MinMaxInt(pDX, m_iSequence, 0, 10000);
	DDX_Check(pDX, IDC_BOLD, m_bBold);
	DDX_Check(pDX, IDC_ITALIC, m_bItalic);
	DDX_Check(pDX, IDC_UNDERLINE, m_bUnderline);
	DDX_Check(pDX, IDC_SOUND_IF_INACTIVE, m_bSoundIfInactive);
	DDX_CBIndex(pDX, IDC_MATCH_TEXT_COLOUR, m_iMatchTextColour);
	DDX_CBIndex(pDX, IDC_MATCH_BACK_COLOUR, m_iMatchBackColour);
	DDX_Check(pDX, IDC_MATCH_BOLD, m_iMatchBold);
	DDX_Check(pDX, IDC_MATCH_ITALIC, m_iMatchItalic);
	DDX_Check(pDX, IDC_MATCH_INVERSE, m_iMatchInverse);
	DDX_CBIndex(pDX, IDC_SEND_TO, m_iSendTo);
	DDX_Check(pDX, IDC_EXPAND_VARIABLES, m_bExpandVariables);
	DDX_Check(pDX, IDC_TEMPORARY, m_bTemporary);
	DDX_Check(pDX, IDC_LOWERCASE_WILDCARD, m_bLowercaseWildcard);
	DDX_Text(pDX, IDC_GROUP, m_strGroup);
	DDX_Text(pDX, IDC_VARIABLE, m_strVariable);
	DDX_Text(pDX, IDC_INCLUDED, m_strIncluded);
	DDX_Check(pDX, IDC_MULTI_LINE, m_bMultiLine);
	DDX_CBIndex(pDX, IDC_COLOUR_CHANGE_TYPE, m_iColourChangeType);
	DDX_Text(pDX, IDC_REGEXP_ERROR, m_strRegexpError);
	DDX_Check(pDX, IDC_ONE_SHOT, m_bOneShot);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_LINES_TO_MATCH, m_iLinesToMatch);
	DDV_MinMaxInt(pDX, m_iLinesToMatch, 0, MAX_RECENT_LINES);

 if(pDX->m_bSaveAndValidate)
   {
    POSITION pos;
    CTrigger * trigger_item;
    CString strTriggerName;

    m_strLabel.TrimLeft ();
    m_strLabel.TrimRight ();

    m_strProcedure.TrimLeft ();
    m_strProcedure.TrimRight ();

    m_strGroup.TrimLeft ();
    m_strGroup.TrimRight ();

    m_strVariable.TrimLeft ();
    m_strVariable.TrimRight ();

    if(m_trigger.IsEmpty ())
        {
        ::TMessageBox("The trigger match text cannot be blank.", MB_ICONSTOP);
        DDX_Text(pDX, IDC_TRIGGER, m_trigger);
        pDX->Fail();
        }     // end of trigger being blank

    // check for foolishly using ** in a non-regular expression
    if (m_trigger.Find ("**") != -1 && !m_bRegexp)
      {
      CCreditsDlg dlg;

      dlg.m_iResourceID = IDR_MULTIPLE_ASTERISKS;
      dlg.m_strTitle = "Warning";

      dlg.DoModal ();	

      DDX_Text(pDX, IDC_TRIGGER, m_trigger);
      pDX->Fail();

      }
    
    int iInvalidPos = m_trigger.FindOneOf ("\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"); 

    if (iInvalidPos != -1)
        {
        char c = m_trigger [iInvalidPos];

        ::UMessageBox(TFormat (
           "The trigger 'match' text contains an invalid non-printable character (hex %02X) at position %i.", c, iInvalidPos + 1), MB_ICONSTOP);
        DDX_Text(pDX, IDC_TRIGGER, m_trigger);
        pDX->Fail();
        }     // end of trigger match having weird characters in it


    // we allow carriage-return, linefeed, tab here
    iInvalidPos = m_contents.FindOneOf ("\x01\x02\x03\x04\x05\x06\x07\x08\x0B\x0C\x0E\x0F"); 

    if (iInvalidPos != -1)
        {
        char c = m_contents [iInvalidPos];

        ::UMessageBox(TFormat (
           "The trigger 'send' text contains an invalid non-printable character (hex %02X) at position %i.", c, iInvalidPos + 1), MB_ICONSTOP);
        DDX_Text(pDX, IDC_TRIGGER_CONTENTS, m_contents);
        pDX->Fail();
        }     // end of trigger send having weird characters in it

    // compile regular expression to check it
    if (m_bRegexp)
      {
      if (!CheckRegularExpression (m_trigger, 
                          (m_bUTF_8 ? PCRE_UTF8 : 0) |     // options
                          (m_bMultiLine  ? PCRE_MULTILINE : 0)))
        {   // failed check
        DDX_Text(pDX, IDC_TRIGGER, m_trigger);
        pDX->Fail();
        }
      } // end of checking regular expression

    for (pos = m_pTriggerMap->GetStartPosition (); pos; )
      {
      m_pTriggerMap->GetNextAssoc (pos, strTriggerName, trigger_item);

// don't compare against itself

      if (trigger_item == m_current_trigger)
        continue;

// now check label for duplicates

      if (!m_strLabel.IsEmpty ())    // we can have duplicate blank names
        if (m_strLabel.CompareNoCase (trigger_item->strLabel) == 0)
          {
          CString strMsg;
          strMsg = TFormat ("The trigger label \"%s\" is already in the list of triggers.",
                          (LPCTSTR) m_strLabel);
          ::UMessageBox(strMsg, MB_ICONSTOP);
          DDX_Text(pDX, IDC_TRIGGER_NAME, m_strLabel);
          pDX->Fail();
          }

      } // end of checking trigger, but not against itself

    if (m_strVariable.IsEmpty ())    
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

// check variable is valid

      if (CheckLabel (m_strVariable))
        {
        ::TMessageBox("The variable must start with a letter and consist of letters"
                        ", numbers or the underscore character.", MB_ICONSTOP);
        DDX_Text(pDX, IDC_VARIABLE, m_strVariable);
        pDX->Fail();
        }

      }   // end of having non-blank variable

    if (!m_strLabel.IsEmpty ())    
      {

// check label is valid

      if (CheckLabel (m_strLabel))
        {
        ::TMessageBox("The label must start with a letter and consist of letters"
                        ", numbers or the underscore character.", MB_ICONSTOP);
        DDX_Text(pDX, IDC_TRIGGER_NAME, m_strLabel);
        pDX->Fail();
        }

      }   // end of having non-blank label

    if (!m_strProcedure.IsEmpty ())    // blank procedure is OK
      {

// check procedure is valid

      if (CheckLabel (m_strProcedure, true))
        {
        ::TMessageBox("The script subroutine name must start with a letter and consist of letters"
                        ", numbers or the underscore character.", MB_ICONSTOP);
        DDX_Text(pDX, IDC_SCRIPT_NAME, m_strProcedure);
        pDX->Fail();
        }

      }   // end of having non-blank procedure


    // warn if contents empty
    if (m_iSendTo != eSendToWorld &&
        m_contents.IsEmpty ())
      {

      if (::UMessageBox(TFormat ("Your trigger is set to 'send to %s' however the 'Send:' field is blank.\n\n"
                      "You can use \"%%0\" to send the entire matching line to the specified place.\n\n"
                      "(You can eliminate this message by sending to 'world')\n\n"
                      "Do you want to change the trigger to fix this?",
                      (LPCTSTR) GetSendToString (m_iSendTo)), MB_YESNO | MB_ICONINFORMATION) 
                      == IDYES)
        {
        DDX_Text(pDX, IDC_TRIGGER_CONTENTS, m_contents);
        pDX->Fail();
        }   // end of them saying no, it wasn't intentional

      }     // end of warn if send text empty


    if (m_bMultiLine && !m_bRegexp)
      {
        ::TMessageBox("Multi-line triggers must be a regular expression", MB_ICONSTOP);
        pDX->Fail();
      }

    if (m_bMultiLine && m_iLinesToMatch < 2)
      {
        ::TMessageBox("Multi-line triggers must match at least 2 lines", MB_ICONSTOP);
        DDX_Text(pDX, IDC_LINES_TO_MATCH, m_iLinesToMatch);
        pDX->Fail();
      }

   } // end of saving and validating
  else
    {
    if (App.m_bFixedFontForEditing)
      {     
      FixFont (m_font, m_ctlTrigger, App.m_strFixedPitchFont, App.m_iFixedPitchFontSize, FW_NORMAL, DEFAULT_CHARSET);
      FixFont (m_font2, m_ctlTriggerContents, App.m_strFixedPitchFont, App.m_iFixedPitchFontSize, FW_NORMAL, DEFAULT_CHARSET);
      }
    }

}


BEGIN_MESSAGE_MAP(CTriggerDlg, CDialog)
	//{{AFX_MSG_MAP(CTriggerDlg)
	ON_BN_CLICKED(IDC_BROWSE_SOUNDS, OnBrowseSounds)
	ON_BN_CLICKED(IDC_NO_SOUND, OnNoSound)
	ON_BN_CLICKED(IDC_TEST_SOUND, OnTestSound)
	ON_BN_CLICKED(IDC_HELPBUTTON, OnHelpbutton)
	ON_CBN_SELCHANGE(IDC_TRIGGER_COLOUR, OnSelchangeTriggerColour)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_BN_CLICKED(IDC_CONVERT_TO_REGEXP, OnConvertToRegexp)
	ON_BN_CLICKED(IDC_EDIT_SEND, OnEditSend)
	ON_CBN_SELCHANGE(IDC_SEND_TO, OnSelchangeSendTo)
	ON_BN_CLICKED(IDC_TRIGGER_SWATCH, OnTriggerSwatch)
	ON_BN_CLICKED(IDC_TRIGGER_SWATCH2, OnTriggerSwatch2)
	ON_BN_CLICKED(IDC_MULTI_LINE, OnMultiLine)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDOK, OnUpdateOK)
  ON_UPDATE_COMMAND_UI(IDC_INVOCATIONS, OnUpdateInvocationCount)
  ON_UPDATE_COMMAND_UI(IDC_MATCHES, OnUpdateMatchCount)
  ON_UPDATE_COMMAND_UI(IDC_TIME_TAKEN, OnUpdateTimeTaken)
  ON_UPDATE_COMMAND_UI(IDC_TEST_SOUND, OnUpdateNeedSound)
  ON_UPDATE_COMMAND_UI(IDC_NO_SOUND, OnUpdateNeedSound)
  ON_UPDATE_COMMAND_UI(IDC_REPEAT, OnUpdateNeedRegexpButNotMultiline)
  ON_UPDATE_COMMAND_UI(IDC_MULTI_LINE, OnUpdateNeedRegexp)
  ON_UPDATE_COMMAND_UI(IDC_CONVERT_TO_REGEXP, OnUpdateNeedNoRegexp)
  ON_UPDATE_COMMAND_UI(IDC_BOLD, OnUpdateNeedBold)
  ON_UPDATE_COMMAND_UI(IDC_ITALIC, OnUpdateNeedItalic)
  ON_UPDATE_COMMAND_UI(IDC_UNDERLINE, OnUpdateNeedUnderline)
  ON_UPDATE_COMMAND_UI(IDC_TRIGGER_COLOUR, OnUpdateCheckComboColour)
  ON_UPDATE_COMMAND_UI(IDC_LINES_TO_MATCH, OnUpdateNeedMultiline)
  ON_UPDATE_COMMAND_UI(IDC_OMIT_FROM_OUTPUT, OnUpdateNeedNoMultiline)
  ON_UPDATE_COMMAND_UI(IDC_OMIT_FROM_LOG, OnUpdateNeedNoMultiline)
  ON_UPDATE_COMMAND_UI(IDC_MATCH_BOLD, OnUpdateNeedNoMultiline)
  ON_UPDATE_COMMAND_UI(IDC_MATCH_ITALIC, OnUpdateNeedNoMultiline)
  ON_UPDATE_COMMAND_UI(IDC_MATCH_INVERSE, OnUpdateNeedNoMultiline)
  ON_UPDATE_COMMAND_UI(IDC_MATCH_TEXT_COLOUR, OnUpdateNeedNoMultiline)
  ON_UPDATE_COMMAND_UI(IDC_MATCH_BACK_COLOUR, OnUpdateNeedNoMultiline)
  ON_UPDATE_COMMAND_UI(IDC_COLOUR_CHANGE_TYPE, OnUpdateNeedNoMultiline)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTriggerDlg message handlers

void CTriggerDlg::OnBrowseSounds() 
{
CString filename;

  CFileDialog filedlg (TRUE,   // loading the file
                       ".wav",    // default extension
                       m_sound_pathname,  // suggested name
                       OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
                       "Waveaudio files (*.wav)|*.wav|"
                       "MIDI files (*.mid)|*.mid|"
                       "Sequencer files (*.rmi)|*.rmi|"
                       "|",    // filter 
                       this);  // parent window

  filedlg.m_ofn.lpstrTitle = "Select sound to play";
  filedlg.m_ofn.lpstrFile = filename.GetBuffer (_MAX_PATH); // needed!! (for Win32s)  

  if (App.platform == VER_PLATFORM_WIN32s)
    strcpy (filedlg.m_ofn.lpstrFile, "");
  else
    strcpy (filedlg.m_ofn.lpstrFile, m_sound_pathname);

  if (m_sound_pathname == NOSOUNDLIT)
    strcpy (filedlg.m_ofn.lpstrFile, "");
    
  ChangeToFileBrowsingDirectory ();
  int nResult = filedlg.DoModal();
  ChangeToStartupDirectory ();

  if (nResult != IDOK)
    return;    // cancelled dialog

  m_sound_pathname = filedlg.GetPathName ();

  GetDlgItem (IDC_SOUND_PATH)->SetWindowText(m_sound_pathname);

}

void CTriggerDlg::OnNoSound() 
{
  m_sound_pathname = NOSOUNDLIT;

  GetDlgItem (IDC_SOUND_PATH)->SetWindowText(m_sound_pathname);

}

void CTriggerDlg::OnTestSound() 
{
  if (!m_sound_pathname.IsEmpty () && m_sound_pathname != NOSOUNDLIT)
    Frame.PlaySoundFile (m_sound_pathname);
}

void CTriggerDlg::OnHelpbutton() 
{
//  OnCmdMsg(ID_HELP, CN_COMMAND, NULL, NULL);
	App.WinHelp(m_nIDHelp + HID_BASE_RESOURCE);
}


void CTriggerDlg::FixSwatch (CColourButton & m_ctlSwatch, 
                             CColourButton & m_ctlSwatch2,
                             int iNewColour)
  {
  if (iNewColour == 0)
    {
     m_ctlSwatch.ShowWindow (SW_HIDE);
     m_ctlSwatch2.ShowWindow (SW_HIDE);
    }
  else
    {
    m_ctlSwatch.ShowWindow (SW_SHOW | SW_SHOWNOACTIVATE);
    m_ctlSwatch2.ShowWindow (SW_SHOW | SW_SHOWNOACTIVATE);
    if ((iNewColour - 1) == OTHER_CUSTOM)
      {
      m_ctlSwatch.EnableWindow (TRUE);
      m_ctlSwatch2.EnableWindow (TRUE);
      m_ctlSwatch.m_colour = m_iOtherForeground;
      m_ctlSwatch2.m_colour = m_iOtherBackground;
      }
    else
      {
      m_ctlSwatch.EnableWindow (FALSE);
      m_ctlSwatch2.EnableWindow (FALSE);
      m_ctlSwatch.m_colour = m_customtext [iNewColour - 1];
      m_ctlSwatch2.m_colour = m_customback [iNewColour - 1];
      }
    m_ctlSwatch.RedrawWindow ();
    m_ctlSwatch2.RedrawWindow ();
    }
  } // end of FixSwatch


BOOL CTriggerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  FixSwatch (m_ctlTriggerSwatch, m_ctlTriggerSwatch2, m_colour);

  m_ctlVariable.EnableWindow (m_iSendTo == eSendToVariable 
#ifdef PANE
                              || m_iSendTo == eSendToPane
#endif // PANE
                              );

  switch (m_iSendTo)
    {
    case eSendToVariable: m_ctlVariableLabel.SetWindowText ("Variable:"); break;
#ifdef PANE
    case eSendToPane: m_ctlVariableLabel.SetWindowText ("Pane:"); break;
#endif // PANE
    default: m_ctlVariableLabel.SetWindowText ("n/a:"); break;
    } // end of switch
   
  int i;
  for (i = 0; i < MAX_CUSTOM; i++)
    {
    m_ctlTriggerColour.m_customtext [i] = m_customtext [i];
    m_ctlTriggerColour.m_customback [i] = m_customback [i];
    }
  m_ctlTriggerColour.m_iOtherForeground = m_iOtherForeground;  
  m_ctlTriggerColour.m_iOtherBackground = m_iOtherBackground;  

  // load combo box now
  m_ctlTriggerColour.ResetContent ();
  m_ctlTriggerColour.AddString ("(no change)");
  for (i = 0; i < MAX_CUSTOM; i++)
    m_ctlTriggerColour.AddString (m_strCustomName [i]);
  m_ctlTriggerColour.AddString ("Other ...");

  // put the selection back
  m_ctlTriggerColour.SetCurSel (m_colour);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTriggerDlg::OnSelchangeTriggerColour() 
{
  if (m_ctlTriggerColour.GetCurSel () != CB_ERR)
     FixSwatch (m_ctlTriggerSwatch, m_ctlTriggerSwatch2, m_ctlTriggerColour.GetCurSel ());
}


LRESULT CTriggerDlg::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CTriggerDlg::OnKickIdle

void CTriggerDlg::OnUpdateOK(CCmdUI* pCmdUI)
  {

CString strTrigger;

  // cannot modify included triggers
  if (m_current_trigger && m_current_trigger->bIncluded)
    {
    pCmdUI->Enable (FALSE);
    return;
    }

  GetDlgItemText (IDC_TRIGGER, strTrigger);

  pCmdUI->Enable (!strTrigger.IsEmpty ());
  } // end of CTriggerDlg::OnUpdateOK


void CTriggerDlg::OnUpdateInvocationCount(CCmdUI* pCmdUI)
  {

  if (m_current_trigger)
    {
    CString strMessage;

    strMessage.Format ("%ld call%s.", 
                        m_current_trigger->nInvocationCount,
                        m_current_trigger->nInvocationCount == 1 ? "" : "s");

    pCmdUI->SetText (strMessage);
    }   // end of having a trigger item

  } // end of CTriggerDlg::OnUpdateInvocationCount


void CTriggerDlg::OnUpdateMatchCount(CCmdUI* pCmdUI)
  {

  if (m_current_trigger)
    {
    CString strMessage;

    strMessage.Format ("%ld match%s.", PLURALES (m_current_trigger->nMatched));
    
    pCmdUI->SetText (strMessage);
    }   // end of having a trigger item

  } // end of CTriggerDlg::OnUpdateMatchCount

void CTriggerDlg::OnUpdateTimeTaken(CCmdUI* pCmdUI)
  {

  if (App.m_iCounterFrequency == 0)
    return;

  if (m_current_trigger && m_current_trigger->regexp)
    {
    CString strMessage;
    double   elapsed_time;
    elapsed_time = ((double) m_current_trigger->regexp->TimeTaken ()) / 
                    ((double) App.m_iCounterFrequency);

    strMessage.Format ("%12.6f sec.", elapsed_time);

    pCmdUI->SetText (strMessage);
    }   // end of having a trigger item

  } // end of CTriggerDlg::OnUpdateMatchCount


void CTriggerDlg::OnUpdateNeedSound(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_sound_pathname != NOSOUNDLIT);
  } // end of CTriggerDlg::OnUpdateNeedSound


void CTriggerDlg::OnUpdateNeedRegexp(CCmdUI* pCmdUI)
  {

  pCmdUI->Enable (m_ctlRegexp.GetCheck ());

  }   // end of CTriggerDlg::OnUpdateNeedRegexp

void CTriggerDlg::OnUpdateNeedMultiline(CCmdUI* pCmdUI)
  {

  pCmdUI->Enable (m_ctlMultiLine.GetCheck ());

  }   // end of CTriggerDlg::OnUpdateNeedMultiline

void CTriggerDlg::OnUpdateNeedNoMultiline(CCmdUI* pCmdUI)
  {

  pCmdUI->Enable (!m_ctlMultiLine.GetCheck ());

  }   // end of CTriggerDlg::OnUpdateNeedNoMultiline

void CTriggerDlg::OnUpdateNeedRegexpButNotMultiline(CCmdUI* pCmdUI)
  {

  pCmdUI->Enable (m_ctlRegexp.GetCheck () && !m_ctlMultiLine.GetCheck ());

  }   // end of CTriggerDlg::OnUpdateNeedRegexpButNotMultiline

void CTriggerDlg::OnUpdateNeedNoRegexp(CCmdUI* pCmdUI)
  {

  pCmdUI->Enable (!m_ctlRegexp.GetCheck ());

  }   // end of CTriggerDlg::OnUpdateNeedNoRegexp

void CTriggerDlg::OnUpdateNeedBold(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_bShowBold && !m_ctlMultiLine.GetCheck ());
  }   // end of CTriggerDlg::OnUpdateNeedBold

void CTriggerDlg::OnUpdateNeedItalic(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_bShowItalic && !m_ctlMultiLine.GetCheck ());
  }   // end of CTriggerDlg::OnUpdateNeedItalic

void CTriggerDlg::OnUpdateNeedUnderline(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (m_bShowUnderline && !m_ctlMultiLine.GetCheck ());
  }   // end of CTriggerDlg::OnUpdateNeedUnderline

void CTriggerDlg::OnUpdateCheckComboColour(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (!m_ctlMultiLine.GetCheck ());

  // provided "other" selected in combo-box
  if (m_ctlTriggerColour.GetCurSel () == (OTHER_CUSTOM + 1))
    // colour has changed ? paste, probably. Redraw combo-box
    if (m_ctlTriggerSwatch.m_colour != m_ctlTriggerColour.m_iOtherForeground ||
        m_ctlTriggerSwatch2.m_colour != m_ctlTriggerColour.m_iOtherBackground )
      {
      m_ctlTriggerColour.m_iOtherForeground = m_ctlTriggerSwatch.m_colour;
      m_ctlTriggerColour.m_iOtherBackground = m_ctlTriggerSwatch2.m_colour;
      m_ctlTriggerColour.RedrawWindow();
      }

  }   // end of CTriggerDlg::OnUpdateCheckComboColour

void CTriggerDlg::OnEdit() 
{
CEditDlg dlg;

  dlg.m_strText = GetText (m_ctlTrigger);

  dlg.m_strTitle = Translate ("Edit trigger 'match' text");

  dlg.m_bRegexp = m_ctlRegexp.GetCheck ();

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlTrigger.SetWindowText (dlg.m_strText);

}

void CTriggerDlg::OnConvertToRegexp() 
{
CString strText = GetText (m_ctlTrigger);

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

 m_ctlTrigger.SetWindowText (strText);

 m_ctlRegexp.SetCheck (TRUE);

}

void CTriggerDlg::OnEditSend() 
{
CEditMultiLine dlg;

  dlg.m_strText = GetText (m_ctlTriggerContents);

  dlg.m_strTitle = "Edit trigger 'send' text";

  m_iSendTo = m_ctlSendTo.GetCurSel ();

  if (m_iSendTo == eSendToScript || m_iSendTo == eSendToScriptAfterOmit)
    dlg.m_bScript = true;

  if (m_pDoc->GetScriptEngine () && m_pDoc->GetScriptEngine ()->L)
    dlg.m_bLua = true;

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlTriggerContents.SetWindowText (dlg.m_strText);
	
}

void CTriggerDlg::OnSelchangeSendTo() 
{
int iSelection = m_ctlSendTo.GetCurSel ();

  if (iSelection == CB_ERR)
    return;

  m_ctlVariable.EnableWindow (iSelection == eSendToVariable 
#ifdef PANE
                              || iSelection == eSendToPane
#endif // PANE
                              );

  switch (iSelection)
    {
    case eSendToVariable: m_ctlVariableLabel.SetWindowText (Translate ("Variable:")); break;
#ifdef PANE
    case eSendToPane: m_ctlVariableLabel.SetWindowText (Translate ("Pane:")); break;
#endif // PANE
    default: m_ctlVariableLabel.SetWindowText (Translate ("(n/a)")); break;
    } // end of switch

}

void CTriggerDlg::OnTriggerSwatch() 
{
  CColourPickerDlg dlg;
  dlg.m_iColour = m_ctlTriggerSwatch.m_colour;
  dlg.m_bPickColour = true;

  if (dlg.DoModal () != IDOK)
    return;
         
  m_iOtherForeground = dlg.GetColor ();
  m_ctlTriggerSwatch.m_colour = m_iOtherForeground;
  m_ctlTriggerSwatch.RedrawWindow();

  m_ctlTriggerColour.m_iOtherForeground = m_iOtherForeground;  
  m_ctlTriggerColour.RedrawWindow();
  
}

void CTriggerDlg::OnTriggerSwatch2() 
{
  CColourPickerDlg dlg;
  dlg.m_iColour = m_ctlTriggerSwatch2.m_colour;
  dlg.m_bPickColour = true;

  if (dlg.DoModal () != IDOK)
    return;

  m_iOtherBackground = dlg.GetColor ();
  m_ctlTriggerSwatch2.m_colour = m_iOtherBackground;
  m_ctlTriggerSwatch2.RedrawWindow();

  m_ctlTriggerColour.m_iOtherBackground = m_iOtherBackground;  
  m_ctlTriggerColour.RedrawWindow();
  
}

void CTriggerDlg::OnMultiLine() 
{

// help them not choose to match zero lines
if (GetText (m_ctlLinesToMatch) == "0")
  m_ctlLinesToMatch.SetWindowText ("2");

}
