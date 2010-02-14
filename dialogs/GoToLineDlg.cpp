// GoToLineDlg.cpp : implementation file
//

/*
Copyright (C) 2000 Nick Gammon.

*/

#include "stdafx.h"
#include "..\MUSHclient.h"
#include "GoToLineDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGoToLineDlg dialog


CGoToLineDlg::CGoToLineDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGoToLineDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGoToLineDlg)
	m_iLineNumber = 0;
	//}}AFX_DATA_INIT
}


void CGoToLineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGoToLineDlg)
	DDX_Text(pDX, IDC_LINE_NUMBER, m_iLineNumber);
	//}}AFX_DATA_MAP

 if(pDX->m_bSaveAndValidate)
   {

    if (m_iLineNumber < 1 || m_iLineNumber > m_iMaxLine)
      {
      CString strMessage;
      strMessage = TFormat ("Line number must be in range 1 to %i", m_iMaxLine);
      ::UMessageBox(strMessage);
      DDX_Text(pDX, IDC_LINE_NUMBER, m_iLineNumber);
      pDX->Fail();
      }

    } // end of saving and validating


}


BEGIN_MESSAGE_MAP(CGoToLineDlg, CDialog)
	//{{AFX_MSG_MAP(CGoToLineDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGoToLineDlg message handlers
