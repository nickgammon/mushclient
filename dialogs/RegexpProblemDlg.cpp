// RegexpProblemDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "RegexpProblemDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRegexpProblemDlg dialog


CRegexpProblemDlg::CRegexpProblemDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRegexpProblemDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRegexpProblemDlg)
	m_strColumn = _T("");
	m_strErrorMessage = _T("");
	m_strText = _T("");
	//}}AFX_DATA_INIT
}


void CRegexpProblemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRegexpProblemDlg)
	DDX_Control(pDX, IDC_ERROR_MESSAGE, m_ctlErrorMessage);
	DDX_Control(pDX, IDC_REGEXP_TEXT, m_ctlRegexpText);
	DDX_Text(pDX, IDC_COLUMN, m_strColumn);
	DDX_Text(pDX, IDC_ERROR_MESSAGE, m_strErrorMessage);
	DDX_Text(pDX, IDC_REGEXP_TEXT, m_strText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRegexpProblemDlg, CDialog)
	//{{AFX_MSG_MAP(CRegexpProblemDlg)
	ON_EN_SETFOCUS(IDC_REGEXP_TEXT, OnSetfocusRegexpText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRegexpProblemDlg message handlers

BOOL CRegexpProblemDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
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

    m_ctlRegexpText.SendMessage (WM_SETFONT,
                                 (WPARAM) m_font.m_hObject,
                                 MAKELPARAM (TRUE, 0));

    m_ctlErrorMessage.SendMessage (WM_SETFONT,
                                 (WPARAM) m_font.m_hObject,
                                 MAKELPARAM (TRUE, 0));
    
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRegexpProblemDlg::OnSetfocusRegexpText() 
{
 
  m_ctlRegexpText.SetSel (m_strText.GetLength (), m_strText.GetLength (), FALSE);
  if (m_iColumn > 50)
	  m_ctlRegexpText.LineScroll (0, m_iColumn - 30);
}
