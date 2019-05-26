#include "stdafx.h"
#include "MUSHclient.h"
#include "mainfrm.h"

#include "MDIClientWnd.h"

// CMDIClientWnd

IMPLEMENT_DYNAMIC(CMDIClientWnd, CWnd)

CMDIClientWnd::CMDIClientWnd()
{
}

CMDIClientWnd::~CMDIClientWnd()
{
}


BEGIN_MESSAGE_MAP(CMDIClientWnd, CWnd)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// CMDIClientWnd message handlers

// utils.setbackgroundcolour (ColourNameToRGB ("yellow") )

BOOL CMDIClientWnd::OnEraseBkgnd(CDC* pDC)
{

  // if colour is 0xFFFFFFFF then use Windows default (same as before)
  if (Frame.m_backgroundColour == 0xFFFFFFFF)
      return CWnd::OnEraseBkgnd(pDC);

	// Set brush to desired background color
	CBrush backBrush(Frame.m_backgroundColour);

	// Save old brush
	CBrush* pOldBrush = pDC->SelectObject(&backBrush);

	CRect rect;
	pDC->GetClipBox(&rect);     // Erase the area needed
	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);

  // put brush back
	pDC->SelectObject(pOldBrush);

	return TRUE;
}