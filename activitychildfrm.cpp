// childfrm.cpp : implementation of the CActivityChildFrame class
//

#include "stdafx.h"
#include "MUSHclient.h"
#include "mainfrm.h"

#include "activitychildfrm.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CActivityChildFrame

IMPLEMENT_DYNCREATE(CActivityChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CActivityChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CActivityChildFrame)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//#define new DEBUG_NEW 

/////////////////////////////////////////////////////////////////////////////
// CActivityChildFrame construction/destruction

CActivityChildFrame::CActivityChildFrame()
{
	// TODO: add member initialization code here

}

CActivityChildFrame::~CActivityChildFrame()
{
}

/////////////////////////////////////////////////////////////////////////////
// CActivityChildFrame diagnostics

#ifdef _DEBUG
void CActivityChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CActivityChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CActivityChildFrame message handlers


BOOL CActivityChildFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	
	cs.style &= ~FWS_ADDTOTITLE;  // do not add document name to window title
		
	return CMDIChildWnd::PreCreateWindow(cs);
}




void CActivityChildFrame::OnUpdateFrameMenu(BOOL bActivate, CWnd* pActivateWnd, HMENU hMenuAlt)
  {

  if (Frame.IsFullScreen ())
    return;

  CMDIChildWnd::OnUpdateFrameMenu (bActivate, pActivateWnd, hMenuAlt);

  }

