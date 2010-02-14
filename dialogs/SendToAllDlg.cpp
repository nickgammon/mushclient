// SendToAllDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "..\doc.h"
#include "SendToAllDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSendToAllDlg dialog


CSendToAllDlg::CSendToAllDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSendToAllDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSendToAllDlg)
	m_strSendText = _T("");
	m_bEcho = FALSE;
	//}}AFX_DATA_INIT
}


void CSendToAllDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendToAllDlg)
	DDX_Control(pDX, IDC_SEND_TEXT, m_ctlSendText);
	DDX_Control(pDX, IDC_WORLD_LIST, m_ctlWorldList);
	DDX_Text(pDX, IDC_SEND_TEXT, m_strSendText);
	DDX_Check(pDX, IDC_ECHO, m_bEcho);
	//}}AFX_DATA_MAP

  POSITION docPos;

  if (pDX->m_bSaveAndValidate)
    {
    // make an array of selected items
    int nCount = m_ctlWorldList.GetSelCount();
    CArray<int,int> aryListBoxSel;

    aryListBoxSel.SetSize(nCount);
    m_ctlWorldList.GetSelItems(nCount, aryListBoxSel.GetData()); 

    // mark all as unselected so far
    for (docPos = App.m_pWorldDocTemplate->GetFirstDocPosition();
        docPos != NULL; )
        ((CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(docPos))->m_bSelected = false;
    
    // they might have closed the world while we were thinking so do this cautiously
    for (docPos = App.m_pWorldDocTemplate->GetFirstDocPosition();
        docPos != NULL; )
      {
      CMUSHclientDoc * pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(docPos);

      // ignore closed worlds
      if (pDoc->m_iConnectPhase != eConnectConnectedToMud || !pDoc->m_pSocket)
        continue;

      for (int i = 0; i < nCount; i++)
        if (m_ctlWorldList.GetItemData (aryListBoxSel [i]) == (DWORD) pDoc)
          {
          pDoc->m_bSelected = true;
          break;
          }

      } // end of doing each document


    } // end of saving
  else
    {   // loading

    for (docPos = App.m_pWorldDocTemplate->GetFirstDocPosition();
        docPos != NULL; )
      {
      int nItem;

      CMUSHclientDoc * pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(docPos);

      // ignore closed worlds
      if (pDoc->m_iConnectPhase != eConnectConnectedToMud || !pDoc->m_pSocket)
        continue;

      nItem = m_ctlWorldList.AddString (pDoc->m_mush_name);

      if (nItem != LB_ERR  && nItem != LB_ERRSPACE )
        {
         m_ctlWorldList.SetItemData (nItem, (DWORD) pDoc);
         if (pDoc->m_bSelected)
           m_ctlWorldList.SetSel (nItem, TRUE);		
        }

      } // end of doing each document

    }   // end of loading

}


BEGIN_MESSAGE_MAP(CSendToAllDlg, CDialog)
	//{{AFX_MSG_MAP(CSendToAllDlg)
	ON_BN_CLICKED(IDC_SELECT_ALL, OnSelectAll)
	ON_BN_CLICKED(IDC_SELECT_NONE, OnSelectNone)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDOK, OnUpdateCheckIfReady)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSendToAllDlg message handlers

BOOL CSendToAllDlg::OnInitDialog() 
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

    m_ctlSendText.SendMessage (WM_SETFONT,
                                 (WPARAM) m_font.m_hObject,
                                 MAKELPARAM (TRUE, 0));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSendToAllDlg::OnSelectAll() 
{
 for (int i = 0; i < m_ctlWorldList.GetCount (); i++)
   m_ctlWorldList.SetSel(i, TRUE);		
	
}

void CSendToAllDlg::OnSelectNone() 
{
 for (int i = 0; i < m_ctlWorldList.GetCount (); i++)
   m_ctlWorldList.SetSel(i, FALSE);	
	
}


LRESULT CSendToAllDlg::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CSendToAllDlg::OnKickIdle

void CSendToAllDlg::OnUpdateCheckIfReady(CCmdUI* pCmdUI)
{
CString strMessage;

  m_ctlSendText.GetWindowText (strMessage);

	pCmdUI->Enable(m_ctlWorldList.GetSelCount () > 0 && !strMessage.IsEmpty ());
}

