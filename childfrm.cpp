// childfrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "MUSHclient.h"

#include "doc.h"
#include "childfrm.h"
#include "MUSHview.h"
#include "mainfrm.h"
#include "sendvw.h"


#include "winplace.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNeedText)
END_MESSAGE_MAP()

//#define new DEBUG_NEW 

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{

  m_topview = NULL;
  m_pDoc = NULL;
	
}

CChildFrame::~CChildFrame()
{
}


BOOL CChildFrame::OnCreateClient( LPCREATESTRUCT lpcs,
	CCreateContext* pContext)
{

   m_pDoc = (CMUSHclientDoc*) pContext->m_pCurrentDoc;
   ASSERT_VALID(m_pDoc);


  m_wndSplitter.m_pDoc = m_pDoc;

#ifdef USE_EXTRA_PANE
  if (m_wndSplitter.CreateStatic(this,3,1))
#else
  if (m_wndSplitter.CreateStatic(this,2,1))
#endif

	  {
		CRect rect;
		GetOwner ()->GetClientRect(&rect);
		CSize size = rect.Size();
		size.cy-=150;
		if (!m_wndSplitter.CreateView(OUTPUT_PANE,0,RUNTIME_CLASS(CMUSHView),size,pContext))
      return FALSE;
		if (!m_wndSplitter.CreateView(COMMAND_PANE,0,RUNTIME_CLASS(CSendView),CSize(0,0),pContext))
      return FALSE;

    CMUSHView * topview     = (CMUSHView *) m_wndSplitter.GetPane(OUTPUT_PANE,0);
    CSendView * bottomview  = (CSendView *) m_wndSplitter.GetPane(COMMAND_PANE,0);
    m_topview = topview;    // for idle processing
    bottomview->m_topview = topview;
    topview->m_owner_frame = this;
    topview->m_bottomview = bottomview;
    bottomview->m_owner_frame = this;
		SetActiveView((CView*) bottomview);

#ifdef USE_EXTRA_PANE

		if (!m_wndSplitter.CreateView(EXTRA_PANE,0,RUNTIME_CLASS(CPaneView),CSize(20,0),pContext))
      return FALSE;

#endif

    return TRUE;
		}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers


BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	
	cs.style &= ~FWS_ADDTOTITLE;  // do not add document name to window title
		
  // make window end at bottom of client area
  RECT rect;
  ::GetClientRect (((CMDIFrameWnd *)&Frame)->m_hWndMDIClient, &rect);
  cs.y = 0;
  cs.cy = rect.bottom;

	return CMDIChildWnd::PreCreateWindow(cs);
}


void CChildFrame::OnClose() 
{

  
	CMDIChildWnd::OnClose();
}


void CChildFrame::FixUpSplitterBar (void)
  {
   int cyCurTop,
       cyCurBottom;

   // we want the input area to be at least high enough for our input font
   // (2 lines) (plus a couple of pixels)
   // nb FixedSys takes 15 pixels

   int iDefaultHeight = (15 * 2) + 9;

   if (m_pDoc)
     iDefaultHeight = (m_pDoc->m_InputFontHeight * 2) + 9;   // 2 lines plus 7 pixels for add, below

   int iMinimumHeight = 15 + 9;

   if (m_pDoc)
     iMinimumHeight = m_pDoc->m_InputFontHeight + 9;   // 2 lines plus 7 pixels for add, below

	CRect rectInside;
	m_wndSplitter.GetClientRect(rectInside);
	rectInside.InflateRect(-9, -9); // allow for borders and splitter bar

   cyCurTop = App.db_get_int("worlds", (LPCTSTR) CFormat ("%s:%s", (LPCTSTR) m_pDoc->m_mush_name, "Top Height"), 20);

   // if this world is existing, take command height from registry
   if (!(m_pDoc->m_mush_name.IsEmpty ()))
     cyCurBottom = App.db_get_int("worlds", (LPCTSTR) CFormat ("%s:%s", (LPCTSTR) m_pDoc->m_mush_name, "Bottom Height"), iDefaultHeight);
   else
     // otherwise, allow for a 2-line command area
    cyCurBottom = iDefaultHeight;

//  TRACE1 ("Bottom height (loaded) = %i\n", cyCurBottom);

   // if height from registry is smaller, make the input font height
   // REMOVED in version 3.83
//   if (cyCurBottom < iMinimumHeight)
//     cyCurBottom = iMinimumHeight;

   // we have to root around like this, because recalclayout lays out the top
   // view first, and allocates the rest (possibly nothing) to the bottom view.
   // I don't really want this, the important thing is that you can see where
   // you are going to type. Thus I work out how much the maxmimum top view
   // can be.

   int iRoom = rectInside.bottom - rectInside.top - cyCurBottom 
        + 7;    // 7 pixels for the splitter bar

   cyCurTop = iRoom;

   // however, don't make ridiculously tiny
   if (cyCurTop < 20)
     cyCurTop = 20;

   // set the info for the top view
   m_wndSplitter.SetRowInfo (OUTPUT_PANE, cyCurTop, 20);

   // set the info for the bottom view
   m_wndSplitter.SetRowInfo (COMMAND_PANE, cyCurBottom, 9);

   // recalculate it all
   m_wndSplitter.RecalcLayout ();

  }

// the tool tip messages end up here rather than in the child window, so send them on
BOOL CChildFrame::OnToolTipNeedText(UINT id, NMHDR * pNMHDR, LRESULT * pResult)
{
  if (m_topview)
    return m_topview->OnToolTipNeedText (id, pNMHDR, pResult);
  else
    return FALSE;
}

void CChildFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMDIChildWnd::OnSize(nType, cx, cy);
	
  if (m_topview)
      FixUpSplitterBar ();
	
}


void CChildFrame::OnUpdateFrameMenu(BOOL bActivate, CWnd* pActivateWnd, HMENU hMenuAlt)
  {

  if (Frame.IsFullScreen ())
    return;

  CMDIChildWnd::OnUpdateFrameMenu (bActivate, pActivateWnd, hMenuAlt);

  }

/*
BOOL CChildFrame::PreTranslateMessage(MSG* pMsg) 
{
//	CMDIChildWnd* pActiveChild = MDIGetActive();
	
	int iResult = CMDIChildWnd::PreTranslateMessage(pMsg);

  if (pMsg->message == WM_KEYDOWN && iResult)

    {
    int x = 2;

    }
  return iResult;
}

*/
