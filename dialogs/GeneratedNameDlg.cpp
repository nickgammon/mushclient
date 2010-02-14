// GeneratedNameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "..\doc.h"
#include "GeneratedNameDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeneratedNameDlg dialog


CGeneratedNameDlg::CGeneratedNameDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGeneratedNameDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGeneratedNameDlg)
	m_strName = _T("");
	m_strFileName = _T("");
	//}}AFX_DATA_INIT

  m_pDoc = NULL;

}


void CGeneratedNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGeneratedNameDlg)
	DDX_Control(pDX, IDC_FILENAME, m_ctlFileName);
	DDX_Control(pDX, IDC_GENERATED_NAME, m_ctlName);
	DDX_Text(pDX, IDC_GENERATED_NAME, m_strName);
	DDX_Text(pDX, IDC_FILENAME, m_strFileName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGeneratedNameDlg, CDialog)
	//{{AFX_MSG_MAP(CGeneratedNameDlg)
	ON_BN_CLICKED(IDC_TRY_AGAIN, OnTryAgain)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	ON_BN_CLICKED(IDC_SEND_TO_WORLD, OnSendToWorld)
	ON_BN_CLICKED(IDC_BROWSE_NAME, OnBrowseName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeneratedNameDlg message handlers

void CGeneratedNameDlg::OnTryAgain() 
{
CString strName = GenerateName ();

  	m_ctlName.SetWindowText (strName);
    m_ctlName.SetSel (0, -1);
	
}

void CGeneratedNameDlg::OnCopy() 
{
  putontoclipboard (GetText (m_ctlName));
}

void CGeneratedNameDlg::OnSendToWorld() 
{
  if (m_pDoc)
    m_pDoc->SendMsg (GetText (m_ctlName), m_pDoc->m_display_my_input,
                      false, m_pDoc->LoggingInput ());
	
}

void CGeneratedNameDlg::OnBrowseName() 
{
	try
	  {
    ReadNames ("*");
    // update file name
    m_ctlFileName.SetWindowText (App.db_get_string  
      ("prefs", "DefaultNameGenerationFile", "names.txt"));
    }
	catch (CException* e)
	  {
		e->ReportError();
		e->Delete();
	  }	
}
