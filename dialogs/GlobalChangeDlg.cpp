// GlobalChangeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "GlobalChangeDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGlobalChangeDlg dialog


CGlobalChangeDlg::CGlobalChangeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGlobalChangeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGlobalChangeDlg)
	m_strChangeTo = _T("");
	m_strChangeFrom = _T("");
	//}}AFX_DATA_INIT
}


void CGlobalChangeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalChangeDlg)
	DDX_Text(pDX, IDC_CHANGE_TO, m_strChangeTo);
	DDX_Text(pDX, IDC_CHANGE_FROM, m_strChangeFrom);
	//}}AFX_DATA_MAP

 if(pDX->m_bSaveAndValidate)
   {

    if (m_strChangeFrom.IsEmpty ())
      {
      ::TMessageBox("This field cannot be empty.");
      DDX_Text(pDX, IDC_CHANGE_FROM, m_strChangeFrom);
      pDX->Fail();
      }

   }    // end of saving and validating

}


BEGIN_MESSAGE_MAP(CGlobalChangeDlg, CDialog)
	//{{AFX_MSG_MAP(CGlobalChangeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGlobalChangeDlg message handlers

