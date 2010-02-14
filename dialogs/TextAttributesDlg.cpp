// TextAttributesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "TextAttributesDlg.h"
#include "..\doc.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextAttributesDlg dialog


CTextAttributesDlg::CTextAttributesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTextAttributesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTextAttributesDlg)
	m_strBackColour = _T("");
	m_strLetter = _T("");
	m_bBold = FALSE;
	m_bInverse = FALSE;
	m_bItalic = FALSE;
	m_strModified = _T("");
	m_strTextColour = _T("");
	m_strCustomColour = _T("");
	m_strTextColourRGB = _T("");
	m_strBackgroundColourRGB = _T("");
	//}}AFX_DATA_INIT

  m_pDoc = NULL;
  m_pLine = 0;
  m_iLine = 0;

}


void CTextAttributesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTextAttributesDlg)
	DDX_Control(pDX, IDC_RGB_TEXT_SWATCH, m_ctlTextSwatch);
	DDX_Control(pDX, IDC_RGB_BACKGROUND_SWATCH, m_ctlBackgroundSwatch);
	DDX_Text(pDX, IDC_BACK_COLOUR, m_strBackColour);
	DDX_Text(pDX, IDC_LETTER, m_strLetter);
	DDX_Check(pDX, IDC_BOLD, m_bBold);
	DDX_Check(pDX, IDC_INVERSE, m_bInverse);
	DDX_Check(pDX, IDC_ITALIC, m_bItalic);
	DDX_Text(pDX, IDC_MODIFIED, m_strModified);
	DDX_Text(pDX, IDC_TEXT_COLOUR, m_strTextColour);
	DDX_Text(pDX, IDC_CUSTOM_COLOUR, m_strCustomColour);
	DDX_Text(pDX, IDC_TEXT_COLOUR_RGB, m_strTextColourRGB);
	DDX_Text(pDX, IDC_BACK_COLOUR_RGB, m_strBackgroundColourRGB);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTextAttributesDlg, CDialog)
	//{{AFX_MSG_MAP(CTextAttributesDlg)
	ON_BN_CLICKED(IDC_LINE_INFO, OnLineInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextAttributesDlg message handlers


/*

This stuff is for my debugging of style runs, although the end-user can play with it
if s/he wants to.

*/

void ShowInfo (CString arg, CMUSHclientDoc * pDoc)
  {
  CString strTitle = "Line Information - ";
  strTitle += pDoc->m_mush_name;

  arg += ENDLINE;       
  pDoc->AppendToTheNotepad (strTitle,   
                            arg,    
                            false,
                            eNotepadLineInfo); 
  } // end of ShowInfo

#define INFO(arg)  ShowInfo (arg, m_pDoc)

#define YES_OR_NO(arg) arg ? "YES" : "no"

void CTextAttributesDlg::OnLineInfo() 
{
  if (!m_pDoc || !m_pLine)
    return;

  CString strDate;
  strDate = m_pLine->m_theTime.Format ("%A, %B %d, %#I:%M:%S %p");

  INFO (TFormat ("Line %i (%i), %s", 
                m_iLine, 
                m_pLine->m_nLineNumber,
                (LPCTSTR) strDate));
  
  INFO (TFormat (" Flags = End para: %s, Note: %s, User input: %s, Log: %s, Bookmark: %s",
        YES_OR_NO (m_pLine->hard_return),
        YES_OR_NO (m_pLine->flags & COMMENT),
        YES_OR_NO (m_pLine->flags & USER_INPUT),
        YES_OR_NO (m_pLine->flags & LOG_LINE),
        YES_OR_NO (m_pLine->flags & BOOKMARK)
        ));

  INFO (TFormat (" Length = %i, last space = %i", 
                  m_pLine->len,
                  m_pLine->last_space));

  CString strText = CString (m_pLine->text, m_pLine->len);

  INFO (TFormat (" Text = \"%s\"", (LPCTSTR) strText));

  INFO ("");
  INFO (TFormat ("%i style run%s", PLURAL (m_pLine->styleList.GetCount ())));
  INFO ("");
    
  CStyle * pStyle;
  POSITION pos;
  int iCol = 0;
  int iCount = 1;

  for (pos = m_pLine->styleList.GetHeadPosition(); pos; iCount++)
    {
    pStyle = m_pLine->styleList.GetNext (pos);

    // offset, length and text affected
    INFO (TFormat ("%i: Offset = %i, Length = %i, Text = \"%s\"",
          iCount,
          iCol, 
          pStyle->iLength,
          (LPCTSTR) strText.Mid (iCol, pStyle->iLength)
          ));

    CString strAction, strHint, strVariable;
    CAction * pAction = pStyle->pAction;

    if (pAction)
      {
      strAction = pAction->m_strAction;
      strHint = pAction->m_strHint;
      strVariable = pAction->m_strVariable;
      }

    // action, eg. hyperlink
    switch (pStyle->iFlags & ACTIONTYPE)
      {
      case ACTION_NONE: 
        INFO (Translate (" No action."));
        break;
      case ACTION_SEND: 
        INFO (TFormat (" Action - send to MUD: \"%s\"", 
              (LPCTSTR) strAction));
        if (!strHint.IsEmpty ())
          INFO (TFormat (" Hint: \"%s\"", 
                (LPCTSTR) strHint));
        break;
      case ACTION_HYPERLINK: 
        INFO (TFormat (" Action - hyperlink: \"%s\"", 
              (LPCTSTR) strAction));
        if (!strHint.IsEmpty ())
          INFO (TFormat (" Hint: \"%s\"", 
                (LPCTSTR) strHint));
        break;
      case ACTION_PROMPT: 
        INFO (TFormat (" Action - send to command window: \"%s\"", 
              (LPCTSTR) strAction));
        if (!strHint.IsEmpty ())
          INFO (TFormat (" Hint: \"%s\"", 
                (LPCTSTR) strHint));
        break;

      } // end of switch

    // variable to set?
    if (!strVariable.IsEmpty ())
      INFO (TFormat (" Set variable: %s", (LPCTSTR) strVariable));

    INFO (TFormat (" Flags = Hilite: %s, Underline: %s, "
                   "Blink: %s, Inverse: %s, Changed: %s",
          YES_OR_NO (pStyle->iFlags & HILITE),
          YES_OR_NO (pStyle->iFlags & UNDERLINE),
          YES_OR_NO (pStyle->iFlags & BLINK),
          YES_OR_NO (pStyle->iFlags & INVERSE),
          YES_OR_NO (pStyle->iFlags & CHANGED)
          ));

    if (pStyle->iFlags & START_TAG)
        INFO (TFormat (" Start MXP tag: %s", 
              (LPCTSTR) pStyle->pAction->m_strAction));

  char * sColours [8] = 
  {
  "Black",
  "Red",
  "Green",
  "Yellow",
  "Blue",
  "Magenta",
  "Cyan",
  "White"
   };

    // colours
    switch (pStyle->iFlags & COLOURTYPE)
      {
      case COLOUR_ANSI: 
        INFO (TFormat (" Foreground colour ANSI  : %i (%s)", 
              pStyle->iForeColour,
              sColours [pStyle->iForeColour & 7]));
        INFO (TFormat (" Background colour ANSI  : %i (%s)", 
              pStyle->iBackColour,
              sColours [pStyle->iBackColour & 7]));
        break;
      case COLOUR_CUSTOM: 
        INFO (TFormat (" Custom colour: %i (%s)", 
              pStyle->iForeColour,
              (LPCTSTR) m_pDoc->m_strCustomColourName [pStyle->iForeColour & 0xFF]));
        break;
      case COLOUR_RGB: 
        INFO (TFormat (" Foreground colour RGB   : R=%i, G=%i, B=%i", 
                        GetRValue (pStyle->iForeColour),
                        GetGValue (pStyle->iForeColour),
                        GetBValue (pStyle->iForeColour)
              ));
        INFO (TFormat (" Background colour RGB   : R=%i, G=%i, B=%i", 
                        GetRValue (pStyle->iBackColour),
                        GetGValue (pStyle->iBackColour),
                        GetBValue (pStyle->iBackColour)
              ));
        break;
      case COLOUR_RESERVED: 
        INFO (TFormat (" Foreground colour rsvd  : %i", 
              pStyle->iForeColour & 0xFFFFFF));
        INFO (TFormat (" Background colour rsvd  : %i", 
              pStyle->iBackColour & 0xFFFFFF));
        break;

      } // end of switch


    INFO ("");
    iCol += pStyle->iLength; // new column

    }   // end of counting number to move

  
  INFO (TFormat ("%i column%s in %i style run%s", PLURAL (iCol), PLURAL (m_pLine->styleList.GetCount ())));
  if (iCol != m_pLine->len)
    INFO (Translate ("** WARNING - length discrepancy **"));

  INFO ("");
  INFO (Translate ("------ (end line information) ------"));
  INFO ("");

  OnOK();   // close dialog box
}

