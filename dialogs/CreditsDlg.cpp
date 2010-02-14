// CreditsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "CreditsDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreditsDlg dialog


CCreditsDlg::CCreditsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCreditsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreditsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

  m_first_time = true;

}


void CCreditsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreditsDlg)
	DDX_Control(pDX, IDC_CREDITS_LIST, m_ctlCreditsText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreditsDlg, CDialog)
	//{{AFX_MSG_MAP(CCreditsDlg)
	ON_EN_SETFOCUS(IDC_CREDITS_LIST, OnSetfocusCreditsList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreditsDlg message handlers

BOOL CCreditsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  HRSRC hRsrc;
  HGLOBAL hRsrc_text = NULL;
  char * p = NULL;


// Load the text of the licence agreement from the appropriate resource

  hRsrc = FindResource (App.m_hInstDLL, MAKEINTRESOURCE (m_iResourceID), "TEXT");

  DWORD iLength = 0;

  if (hRsrc)
    {
    hRsrc_text = LoadResource (App.m_hInstDLL, hRsrc);
    iLength = SizeofResource (App.m_hInstDLL, hRsrc);
    }

  if (hRsrc_text)
    p = (char *) LockResource (hRsrc_text);

    
  if (p)
    m_ctlCreditsText.SetWindowText (CString (p, iLength));
  else
    m_ctlCreditsText.SetWindowText (Translate ("Could not load text"));
	

CDC dc;

  dc.CreateCompatibleDC (NULL);

   int lfHeight = -MulDiv(App.m_iFixedPitchFontSize, dc.GetDeviceCaps(LOGPIXELSY), 72);

   m_font.CreateFont(lfHeight, // int nHeight, 
				  0, // int nWidth, 
				  0, // int nEscapement, 
				  0, // int nOrientation, 
				  FW_DONTCARE, // int nWeight, 
				  0, // BYTE bItalic, 
				  0, // BYTE bUnderline, 
          0, // BYTE cStrikeOut, 
          MUSHCLIENT_FONT_CHARSET, // BYTE nCharSet, 
          0, // BYTE nOutPrecision, 
          0, // BYTE nClipPrecision, 
          0, // BYTE nQuality, 
          MUSHCLIENT_FONT_FAMILY, // BYTE nPitchAndFamily,  
          App.m_strFixedPitchFont);// LPCTSTR lpszFacename );

    // Get the metrics of the font.

    dc.SelectObject(&m_font);

    m_ctlCreditsText.SendMessage (WM_SETFONT,
                                 (WPARAM) m_font.m_hObject,
                                 MAKELPARAM (TRUE, 0));

    // update window title
    SetWindowText (m_strTitle);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCreditsDlg::OnSetfocusCreditsList() 
{
  if (m_first_time)
    {
    m_ctlCreditsText.SetSel (0, 0, FALSE);
    m_first_time = false;
    }
	
}
