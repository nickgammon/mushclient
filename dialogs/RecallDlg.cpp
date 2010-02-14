// RecallDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "RecallDlg.h"
#include "..\winplace.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRecallDlg dialog


CRecallDlg::CRecallDlg(COLORREF back_colour,
                       CWnd* pParent /*=NULL*/)
	: CDialog(CRecallDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRecallDlg)
	m_strText = _T("");
	//}}AFX_DATA_INIT

  m_first_time = true;
  m_font = NULL;
  m_bReadOnly = TRUE;


  // Instantiate and initialize the background brush to black.
  m_pEditBkBrush = new CBrush(back_colour);


}


void CRecallDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRecallDlg)
	DDX_Control(pDX, IDC_TEXT, m_ctlText);
	DDX_Text(pDX, IDC_TEXT, m_strText);
	//}}AFX_DATA_MAP

  if (!pDX->m_bSaveAndValidate)
    {
    FixFont (m_font, m_ctlText, m_strFontName, m_iFontSize, m_iFontWeight, m_iFontCharset);
    }

}


BEGIN_MESSAGE_MAP(CRecallDlg, CDialog)
	//{{AFX_MSG_MAP(CRecallDlg)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_EN_SETFOCUS(IDC_TEXT, OnSetfocusText)
	//}}AFX_MSG_MAP
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecallDlg message handlers

void CRecallDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
  if (m_ctlText.m_hWnd)
    {
	  m_ctlText.MoveWindow(0, 0, cx, cy);
    }
	
}

BOOL CRecallDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// ID_SYS_SAVE must be in the system command range.
	ASSERT((ID_SYS_SAVE & 0xFFF0) == ID_SYS_SAVE);
	ASSERT(ID_SYS_SAVE < 0xF000);
  
RECT rect;

  GetClientRect (&rect);

  m_ctlText.MoveWindow (&rect);
 
  if (!m_strTitle.IsEmpty ())
    SetWindowText (m_strTitle);

	// Add "Save As Text" menu command to system menu
	CMenu*	pSysMenu = GetSystemMenu(FALSE);
	pSysMenu->AppendMenu(MF_SEPARATOR);
	pSysMenu->AppendMenu(MF_STRING, ID_SYS_SAVE, "Sa&ve");
	pSysMenu->AppendMenu(MF_STRING, ID_SYS_SAVE_AS, "Save &As...");

  HICON hicon = AfxGetApp()->LoadIcon(IDR_MUSHCLTYPE);   ;

  SetIcon (hicon, FALSE);

  CWindowPlacement wp;
  wp.Restore ("Recall window", this, false);

  // read only or not?
  m_ctlText.SetReadOnly (m_bReadOnly);

  m_ctlText.SetFocus();

  m_ctlText.SetModify (FALSE);    // not modified yet

  SetWindowPos(&CWnd::wndNoTopMost, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRecallDlg::PostNcDestroy() 
{
	CDialog::PostNcDestroy();
  delete this;
}



void CRecallDlg::OnOK (void)
  {
  if (m_ctlText.GetModify ())
    {
    int iReply = ::TMessageBox ("Window contents have changed. Save changes?",
                                  MB_ICONQUESTION | MB_YESNOCANCEL);

    if (iReply == IDCANCEL)
      return;

    if (iReply == IDYES)
      if (!SaveModified (false))
        return;   // must have cancelled save

    }

  DestroyWindow ();
  }

void CRecallDlg::OnCancel (void)
  {
  OnOK ();    // check to see if we need to save it
  }

bool CRecallDlg::SaveModified (const bool bSaveAs)
  {
  if (m_strFilename.IsEmpty () || bSaveAs)
    {
	  CFileDialog	dlg(FALSE,						// FALSE for FileSave
					  "txt",						// default extension
					  "",
					  OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST,
					  "Text files (*.txt)|*.txt||",
					  this);
	  dlg.m_ofn.lpstrTitle = "File to save as";

    ChangeToFileBrowsingDirectory ();
	  int nResult = dlg.DoModal();
    ChangeToStartupDirectory ();

	  if (nResult != IDOK)
		  return FALSE;

    m_strFilename = dlg.GetPathName();
    }

	try
	  {
		CFile	f (m_strFilename, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive);

    CString strText;

    m_ctlText.GetWindowText (strText);
    
    f.Write (strText, strText.GetLength ());
    
    m_ctlText.SetModify (FALSE);    // not modified now

    f.Close ();

    }
	catch (CException* e)
	{
		e->ReportError();
		e->Delete();
    return FALSE;
	}

  return TRUE;

  }


void CRecallDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == ID_SYS_SAVE)
    SaveModified (false);
  else if ((nID & 0xFFF0) == ID_SYS_SAVE_AS)
    SaveModified (true);
  else
		CDialog::OnSysCommand(nID, lParam);
}


HBRUSH CRecallDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
  pDC->SetTextColor (m_textColour); 
  pDC->SetBkColor  (m_backColour);
  pDC->SetBkMode (OPAQUE);

  return (HBRUSH)(m_pEditBkBrush->GetSafeHandle());

}



void CRecallDlg::OnDestroy() 
{


  CWindowPlacement wp;
  wp.Save ("Recall window", this);

	CDialog::OnDestroy();
	

  // Free the space allocated for the background brush
  delete m_pEditBkBrush;
      	
}

void CRecallDlg::OnSetfocusText() 
{
  if (m_first_time)
    {
    m_ctlText.SetSel (0, 0, FALSE);
    m_first_time = false;
    }
}

