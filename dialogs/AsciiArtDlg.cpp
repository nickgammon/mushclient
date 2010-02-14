// AsciiArtDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "AsciiArtDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAsciiArtDlg dialog


CAsciiArtDlg::CAsciiArtDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAsciiArtDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAsciiArtDlg)
	m_strText = _T("");
	m_strFont = _T("");
	m_iLayout = -1;
	//}}AFX_DATA_INIT
}


void CAsciiArtDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAsciiArtDlg)
	DDX_Control(pDX, IDC_TEXT, m_ctlText);
	DDX_Control(pDX, IDC_FONT, m_ctlFont);
	DDX_Text(pDX, IDC_TEXT, m_strText);
	DDV_MaxChars(pDX, m_strText, 60);
	DDX_Text(pDX, IDC_FONT, m_strFont);
	DDX_CBIndex(pDX, IDC_LAYOUT, m_iLayout);
	//}}AFX_DATA_MAP

 if(pDX->m_bSaveAndValidate)
   {

    m_strText.TrimLeft ();
    m_strText.TrimRight ();

    if (m_strText.IsEmpty ())
      {
      ::TMessageBox("You must specify some text to insert.");
      DDX_Text(pDX, IDC_TEXT, m_strText);
      pDX->Fail();
      }

    m_strFont.TrimLeft ();
    m_strFont.TrimRight ();

    if (m_strFont.IsEmpty ())
      {
      ::TMessageBox("You must specify a font file.");
      DDX_Text(pDX, IDC_FONT, m_strFont);
      pDX->Fail();
      }
   }

}


BEGIN_MESSAGE_MAP(CAsciiArtDlg, CDialog)
	//{{AFX_MSG_MAP(CAsciiArtDlg)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDOK, OnUpdateNeedThings)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAsciiArtDlg message handlers

void CAsciiArtDlg::OnBrowse() 
{
	CFileDialog	dlg(TRUE,						// TRUE for FileOpen
					"flf",						// default extension
					GetText (m_ctlFont),						// initial file name
					OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
					"Figlet font files (*.flf)|*.flf|All files (*.*)|*.*||");
	dlg.m_ofn.lpstrTitle = "Select font file";

  ChangeToFileBrowsingDirectory ();
	int nResult = dlg.DoModal();
  ChangeToStartupDirectory ();

	if (nResult != IDOK)
		return;
  m_ctlFont.SetWindowText (dlg.GetPathName());	
}


LRESULT CAsciiArtDlg::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CAsciiArtDlg::OnKickIdle

void CAsciiArtDlg::OnUpdateNeedThings(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!GetText (m_ctlText).IsEmpty () &&
                 !GetText (m_ctlFont).IsEmpty ());
}

