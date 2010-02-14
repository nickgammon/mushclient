// HighlightPhraseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "HighlightPhraseDlg.h"

#include "ColourPickerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHighlightPhraseDlg dialog


CHighlightPhraseDlg::CHighlightPhraseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHighlightPhraseDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHighlightPhraseDlg)
	m_iColour = -1;
	m_strText = _T("");
	m_bWord = FALSE;
	m_bMatchCase = FALSE;
	//}}AFX_DATA_INIT
}


void CHighlightPhraseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHighlightPhraseDlg)
	DDX_Control(pDX, IDC_HIGHLIGHT_TEXT, m_ctlText);
	DDX_Control(pDX, IDC_TRIGGER_SWATCH_BGND, m_ctlTriggerSwatchBgnd);
	DDX_Control(pDX, IDC_TRIGGER_SWATCH_TEXT, m_ctlTriggerSwatchText);
	DDX_Control(pDX, IDC_TRIGGER_COLOUR, m_ctlTriggerColour);
	DDX_CBIndex(pDX, IDC_TRIGGER_COLOUR, m_iColour);
	DDX_Text(pDX, IDC_HIGHLIGHT_TEXT, m_strText);
	DDV_MaxChars(pDX, m_strText, 255);
	DDX_Check(pDX, IDC_WHOLE_WORD, m_bWord);
	DDX_Check(pDX, IDC_MATCH_CASE, m_bMatchCase);
	//}}AFX_DATA_MAP

 if(pDX->m_bSaveAndValidate)
   {
    // can't highlight an empty string
    if (m_strText.IsEmpty ())
      {
      ::TMessageBox("The text to highlight cannot be empty.", MB_ICONSTOP);
      DDX_Text(pDX, IDC_HIGHLIGHT_TEXT, m_strText);
      pDX->Fail();
      }     // end of text being blank

    // no change doesn't make sense here
    if (m_iColour <= 0)
      {
      ::TMessageBox("Please choose a colour other than '(no change)'.", MB_ICONSTOP);
      DDX_Text(pDX, IDC_HIGHLIGHT_TEXT, m_strText);
      pDX->Fail();
      }     // end of no colour selected

    // check they selected a different colour
    if (m_iColour == (OTHER_CUSTOM + 1) &&
        m_iOtherForeground_orig == m_iOtherForeground &&
        m_iOtherBackground_orig == m_iOtherBackground)

      {
      ::TMessageBox("Please choose a different colour than the original one.", MB_ICONSTOP);
      DDX_Text(pDX, IDC_HIGHLIGHT_TEXT, m_strText);
      pDX->Fail();
      }

   }    // end of saving
 else
   {
   m_iOtherForeground_orig = m_iOtherForeground;
   m_iOtherBackground_orig = m_iOtherBackground;
   }

}


BEGIN_MESSAGE_MAP(CHighlightPhraseDlg, CDialog)
	//{{AFX_MSG_MAP(CHighlightPhraseDlg)
	ON_BN_CLICKED(IDC_TRIGGER_SWATCH_TEXT, OnTriggerSwatchText)
	ON_BN_CLICKED(IDC_TRIGGER_SWATCH_BGND, OnTriggerSwatchBgnd)
	ON_CBN_SELCHANGE(IDC_TRIGGER_COLOUR, OnSelchangeTriggerColour)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHighlightPhraseDlg message handlers


void CHighlightPhraseDlg::OnTriggerSwatchText() 
{
  CColourPickerDlg dlg;
  dlg.m_iColour = m_ctlTriggerSwatchText.m_colour;
  dlg.m_bPickColour = true;

  if (dlg.DoModal () != IDOK)
    return;
         
  m_iOtherForeground = dlg.GetColor ();
  m_ctlTriggerSwatchText.m_colour = m_iOtherForeground;
  m_ctlTriggerSwatchText.RedrawWindow();

  m_ctlTriggerColour.m_iOtherForeground = m_iOtherForeground;  
  m_ctlTriggerColour.RedrawWindow();
	
}

void CHighlightPhraseDlg::OnTriggerSwatchBgnd() 
{
  CColourPickerDlg dlg;
  dlg.m_iColour = m_ctlTriggerSwatchBgnd.m_colour;
  dlg.m_bPickColour = true;

  if (dlg.DoModal () != IDOK)
    return;

  m_iOtherBackground = dlg.GetColor ();
  m_ctlTriggerSwatchBgnd.m_colour = m_iOtherBackground;
  m_ctlTriggerSwatchBgnd.RedrawWindow();

  m_ctlTriggerColour.m_iOtherBackground = m_iOtherBackground;  
  m_ctlTriggerColour.RedrawWindow();
	
}

void CHighlightPhraseDlg::OnSelchangeTriggerColour() 
{
  if (m_ctlTriggerColour.GetCurSel () != CB_ERR)
     FixSwatch (m_ctlTriggerSwatchText, m_ctlTriggerSwatchBgnd, m_ctlTriggerColour.GetCurSel ());
	
}

BOOL CHighlightPhraseDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  FixSwatch (m_ctlTriggerSwatchText, m_ctlTriggerSwatchBgnd, m_iColour);

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
  m_ctlTriggerColour.SetCurSel (m_iColour);
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHighlightPhraseDlg::FixSwatch (CColourButton & m_ctlSwatch, 
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

