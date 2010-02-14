// CG: This file was added by the Splash Screen component.
// Splash.cpp : implementation file
//

#include "stdafx.h"  // e. g. stdafx.h
#include "..\MUSHclient.h"
#include "..\mainfrm.h"
#include "..\doc.h"

#include "Splash.h"  // e.g. splash.h

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//   Splash Screen class

BOOL CSplashWnd::c_bShowSplashWnd;
CSplashWnd* CSplashWnd::c_pSplashWnd;
CSplashWnd::CSplashWnd()
{
  m_bNick = false;
}

CSplashWnd::~CSplashWnd()
{
	// Clear the static window pointer.
	ASSERT(c_pSplashWnd == this);
	c_pSplashWnd = NULL;
}

BEGIN_MESSAGE_MAP(CSplashWnd, CWnd)
	//{{AFX_MSG_MAP(CSplashWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSplashWnd::EnableSplashScreen(BOOL bEnable /*= TRUE*/)
{
	c_bShowSplashWnd = bEnable;
}

// returns true if we have a splash screen
bool CSplashWnd::HaveSplashScreen (void)
  {
  return c_pSplashWnd != NULL;
  } // end of CSplashWnd::HaveSplashScreen


void CSplashWnd::ShowSplashScreen(CWnd* pParentWnd, const int iResourceID)
{
	if (!c_bShowSplashWnd || c_pSplashWnd != NULL)
		return;

	// Allocate a new splash screen, and create the window.
	c_pSplashWnd = new CSplashWnd;
	if (!c_pSplashWnd->Create(pParentWnd, iResourceID))
		delete c_pSplashWnd;
	else
		c_pSplashWnd->UpdateWindow();

}

BOOL CSplashWnd::PreTranslateAppMessage(MSG* pMsg)
{
	if (c_pSplashWnd == NULL || c_pSplashWnd->m_bNick)
		return FALSE;

	// If we get a keyboard or mouse message, hide the splash screen.
	if (pMsg->message == WM_KEYDOWN ||
	    pMsg->message == WM_SYSKEYDOWN ||
	    pMsg->message == WM_LBUTTONDOWN ||
	    pMsg->message == WM_RBUTTONDOWN ||
	    pMsg->message == WM_MBUTTONDOWN ||
	    pMsg->message == WM_NCLBUTTONDOWN ||
	    pMsg->message == WM_NCRBUTTONDOWN ||
	    pMsg->message == WM_NCMBUTTONDOWN)
	{
		c_pSplashWnd->HideSplashScreen();
		return TRUE;	// message handled here
	}

	return FALSE;	// message not handled
}

BOOL CSplashWnd::Create(CWnd* pParentWnd, const int iResourceID)
{
   LPCTSTR lpszResourceName = (LPCTSTR) iResourceID;

   m_bNick = iResourceID == IDB_NICK;

   HBITMAP hBmp = (HBITMAP)::LoadImage( AfxGetResourceHandle(), 
                   lpszResourceName, IMAGE_BITMAP, 0,0, LR_CREATEDIBSECTION );

   if( hBmp == NULL ) 
           return false;

   m_bitmap.Attach( hBmp );

   // Create a logical palette for the bitmap
   DIBSECTION ds;
   BITMAPINFOHEADER &bmInfo = ds.dsBmih;
   m_bitmap.GetObject( sizeof(ds), &ds );

   int nColors = bmInfo.biClrUsed ? bmInfo.biClrUsed : 1 << bmInfo.biBitCount;

   CClientDC dc(NULL);                     // Desktop DC

   if( nColors > 256 )
     m_pal.CreateHalftonePalette (&dc);
   else
     {

     // Create the palette

     RGBQUAD *pRGB = new RGBQUAD[nColors];
     CDC memDC;
     memDC.CreateCompatibleDC(&dc);

     memDC.SelectObject( &m_bitmap );
     ::GetDIBColorTable( memDC, 0, nColors, pRGB );

     UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
     LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];

     pLP->palVersion = 0x300;
     pLP->palNumEntries = nColors;

     for( int i=0; i < nColors; i++)
       {
       pLP->palPalEntry[i].peRed = pRGB[i].rgbRed;
       pLP->palPalEntry[i].peGreen = pRGB[i].rgbGreen;
       pLP->palPalEntry[i].peBlue = pRGB[i].rgbBlue;
       pLP->palPalEntry[i].peFlags = 0;
       }

     m_pal.CreatePalette( pLP );

     delete[] pLP;
     delete[] pRGB;
  }


  BITMAP bm;
  m_bitmap.GetBitmap(&bm);

	return CreateEx(0,
		AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
    NULL, WS_POPUP | WS_VISIBLE | WS_BORDER, 0, 0, bm.bmWidth, bm.bmHeight, pParentWnd->GetSafeHwnd(), NULL);
}

void CSplashWnd::HideSplashScreen()
{
	// Destroy the window, and update the mainframe.
	DestroyWindow();
	AfxGetMainWnd()->UpdateWindow();

}

void CSplashWnd::PostNcDestroy()
{
	// Free the C++ class.
	delete this;
}

int CSplashWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Center the window.
	CenterWindow();

	// Set a timer to destroy the splash screen.
  SetTimer(SPLASH_SCREEN_TIMER_ID, m_bNick ? 10000 : 
#ifdef _DEBUG
  500,
#else
  4000, 
#endif
    
  NULL);

	return 0;
}

void CSplashWnd::OnPaint()
{
  CPaintDC dc(this); // device context for painting

  // Create a memory DC compatible with the paint DC
  CDC memDC;
  memDC.CreateCompatibleDC( &dc );

//  CBitmap bitmap;
//  CPalette palette;

  BITMAP bm;
  m_bitmap.GetBitmap(&bm);

  //   GetBitmapAndPalette( IDB_BITMAP, bitmap, palette );
  CBitmap* pOldBitmap = memDC.SelectObject( &m_bitmap );
  CPalette* pOldPal = NULL;

  // Select and realize the palette
  if( dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE && m_pal.m_hObject != NULL )
    {
     pOldPal = dc.SelectPalette( &m_pal, FALSE );
     dc.RealizePalette();
    }
  dc.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &memDC, 0, 0,SRCCOPY);
 	memDC.SelectObject(pOldBitmap);
  if (pOldPal)
    memDC.SelectPalette(pOldPal, TRUE); 

  if (m_bNick)
    return;

// show if is registered or not

CSize textsize;

  dc.SetBkMode (TRANSPARENT);
 
// show version

  CString strVersion = "Version " + MUSHCLIENT_VERSION;

  textsize = dc.GetTextExtent (strVersion);

  dc.SetTextColor (RGB (120, 0, 0));  

  dc.TextOut (15, bm.bmHeight + textsize.cy - 38, strVersion);

}

void CSplashWnd::OnTimer(UINT nIDEvent)
{
	// Destroy the splash screen window.
	HideSplashScreen();
}
