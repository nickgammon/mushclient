// textchildfrm.cpp : implementation of the CTextChildFrame class
//

#include "stdafx.h"
#include "MUSHclient.h"
#include "mainfrm.h"
#include "TextDocument.h"
#include "textchildfrm.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextChildFrame

IMPLEMENT_DYNCREATE(CTextChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CTextChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CTextChildFrame)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//#define new DEBUG_NEW 

/////////////////////////////////////////////////////////////////////////////
// CTextChildFrame construction/destruction

CTextChildFrame::CTextChildFrame()
{
  m_pDoc = NULL;

}

CTextChildFrame::~CTextChildFrame()
{
}

/////////////////////////////////////////////////////////////////////////////
// CTextChildFrame diagnostics

#ifdef _DEBUG
void CTextChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CTextChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTextChildFrame message handlers


BOOL CTextChildFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	
	cs.style &= ~FWS_ADDTOTITLE;  // do not add document name to window title
		
	return CMDIChildWnd::PreCreateWindow(cs);
}



void CTextChildFrame::OnUpdateFrameMenu(BOOL bActivate, CWnd* pActivateWnd, HMENU hMenuAlt)
  {

  if (Frame.IsFullScreen ())
    return;

  CMDIChildWnd::OnUpdateFrameMenu (bActivate, pActivateWnd, hMenuAlt);

  }

BOOL CTextChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
   m_pDoc = (CTextDocument*) pContext->m_pCurrentDoc;
   ASSERT_VALID(m_pDoc);
	
	return CMDIChildWnd::OnCreateClient(lpcs, pContext);
}
