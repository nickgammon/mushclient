// ChooseNotepadDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "..\TextDocument.h"
#include "..\doc.h"
#include "ChooseNotepadDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseNotepadDlg dialog


CChooseNotepadDlg::CChooseNotepadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChooseNotepadDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseNotepadDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
 m_pWorld = NULL;
 m_pTextDocument = NULL;

}


void CChooseNotepadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseNotepadDlg)
	DDX_Control(pDX, IDC_NOTEPAD_LIST, m_ctlNotepadList);
	//}}AFX_DATA_MAP

  POSITION docPos;

  if (!pDX->m_bSaveAndValidate)
    {   // loading

    for (docPos = App.m_pNormalDocTemplate->GetFirstDocPosition();
        docPos != NULL; )
      {
      int nItem;

      CTextDocument * pDoc = (CTextDocument *) App.m_pWorldDocTemplate->GetNextDoc(docPos);

      // ignore unrelated worlds
      if (pDoc->m_pRelatedWorld != m_pWorld ||
          pDoc->m_iUniqueDocumentNumber != m_pWorld->m_iUniqueDocumentNumber)
        continue;

      CString strTitle = pDoc->GetPathName ();
      if (strTitle.IsEmpty ())
         strTitle = pDoc->m_strTitle;
      if (strTitle.IsEmpty ())
        strTitle = pDoc->GetTitle ();

      nItem = m_ctlNotepadList.AddString (strTitle);

      if (nItem != LB_ERR  && nItem != LB_ERRSPACE )
         m_ctlNotepadList.SetItemData (nItem, (DWORD) pDoc);

      } // end of doing each document

    }   // end of loading

}


BEGIN_MESSAGE_MAP(CChooseNotepadDlg, CDialog)
	//{{AFX_MSG_MAP(CChooseNotepadDlg)
	ON_BN_CLICKED(IDC_OPEN_EXISTING, OnOpenExisting)
	ON_LBN_DBLCLK(IDC_NOTEPAD_LIST, OnDblclkNotepadList)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_OPEN_EXISTING, OnUpdateNeedSelection)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseNotepadDlg message handlers

void CChooseNotepadDlg::OnOpenExisting() 
{
int nItem = m_ctlNotepadList.GetCurSel ();

  if (nItem == LB_ERR)
    return;

  m_pTextDocument = (CTextDocument *) m_ctlNotepadList.GetItemData (nItem);

  OnOK ();
	
}


LRESULT CChooseNotepadDlg::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CChooseNotepadDlg::OnKickIdle

void CChooseNotepadDlg::OnUpdateNeedSelection(CCmdUI* pCmdUI)
{
int nItem = m_ctlNotepadList.GetCurSel ();

	pCmdUI->Enable(nItem != LB_ERR);
}


void CChooseNotepadDlg::OnDblclkNotepadList() 
{
OnOpenExisting ();	
}
