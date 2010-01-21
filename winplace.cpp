////////////////////////////////////////////////////////////////
// CWindowPlacement 1996 Microsoft Systems Journal.
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
#include "stdafx.h"
#include "MUSHclient.h"
#include "mainfrm.h"
#include "winplace.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#include <MULTIMON.H>

CWindowPlacement::CWindowPlacement()
{
   // Note: "length" is inherited from WINDOWPLACEMENT
   length = sizeof(WINDOWPLACEMENT);
}

CWindowPlacement::~CWindowPlacement()
{
}


//////////////////
// Restore window placement from profile key
BOOL CWindowPlacement::Restore(LPCSTR lpKeyName, 
                               CWnd* pWnd,
                               const bool bDesktop)
{
   GetProfileWP(lpKeyName);

   // Only restore if window intersets the screen.
   //
   CRect rcTemp, rcScreen;

   if (bDesktop)
     {
      HMONITOR hMonitor;
      MONITORINFO mi;

      //
      // get the nearest monitor to the "normal" rectangle
      //
      hMonitor = MonitorFromRect(&rcNormalPosition, MONITOR_DEFAULTTONEAREST);

      //
      // get the work area rect.
      //
      mi.cbSize = sizeof(mi);
      GetMonitorInfo(hMonitor, &mi);

      rcScreen = mi.rcWork;
//      Frame.GetDesktopWindow ()->GetClientRect (rcScreen);  // prior to version 3.85
     }
   else
     {
     ::GetClientRect (((CMDIFrameWnd *)&Frame)->m_hWndMDIClient, &rcScreen);
     rcScreen.InflateRect (-10, -10); // margin for error
     }


   // see if rectangle will fit inside parent

   if (!::IntersectRect(&rcTemp, &rcNormalPosition, &rcScreen))
      return FALSE;

   pWnd->SetWindowPlacement(this);  // set placement
   return TRUE;
}

//////////////////
// Get window placement from profile.
void CWindowPlacement::GetProfileWP(LPCSTR lpKeyName)
{
   CWinApp *pApp = AfxGetApp();
   ASSERT_VALID(pApp);


  showCmd         = App.db_get_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.showCmd"), showCmd);
  flags           = App.db_get_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.flags"), flags);

  ptMinPosition.x = App.db_get_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.ptMinPosition.x"), ptMinPosition.x);
  ptMinPosition.y = App.db_get_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.ptMinPosition.y"), ptMinPosition.y);
  ptMaxPosition.x = App.db_get_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.ptMaxPosition.x"), ptMaxPosition.x);
  ptMaxPosition.y = App.db_get_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.ptMaxPosition.y"), ptMaxPosition.y);


  RECT& rc = rcNormalPosition;  // because I hate typing

  rc.left   = App.db_get_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.left"),   rc.left);
  rc.right  = App.db_get_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.right"),  rc.right);
  rc.top    = App.db_get_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.top"),    rc.top);
  rc.bottom = App.db_get_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.bottom"), rc.bottom);

}

////////////////
// Save window placement in app profile
void CWindowPlacement::Save(LPCSTR lpKeyName, CWnd* pWnd)
{
   pWnd->GetWindowPlacement(this);

   // don't save if minimized, it doesn't work properly
   if (showCmd != SW_SHOWMINIMIZED)
     WriteProfileWP(lpKeyName);
}

//////////////////
// Write window placement to app profile
void CWindowPlacement::WriteProfileWP(LPCSTR lpKeyName)
{
   CWinApp *pApp = AfxGetApp();
   ASSERT_VALID(pApp);


   int rc = App.db_execute ("BEGIN TRANSACTION", true);

   // give it up if we can't get a transaction going - it's no big deal
   if (rc == SQLITE_OK || rc == SQLITE_DONE)
    {

    App.db_write_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.showCmd"), showCmd);
    App.db_write_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.flags"), flags);

    App.db_write_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.ptMinPosition.x"), ptMinPosition.x);
    App.db_write_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.ptMinPosition.y"), ptMinPosition.y);
    App.db_write_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.ptMaxPosition.x"), ptMaxPosition.x);
    App.db_write_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.ptMaxPosition.y"), ptMaxPosition.y);

    App.db_write_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.left"),   rcNormalPosition.left);
    App.db_write_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.right"),  rcNormalPosition.right);
    App.db_write_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.top"),    rcNormalPosition.top);
    App.db_write_int ("worlds", (LPCTSTR) CFormat ("%s:%s", lpKeyName, "wp.bottom"), rcNormalPosition.bottom);

    App.db_execute ("COMMIT", true);

    }

}


//////////////////
// Write window placement to archive
// WARNING: archiving functions are untested.
CArchive& operator<<(CArchive& ar, const CWindowPlacement& wp)
{
   ar << wp.length;
   ar << wp.flags;
   ar << wp.showCmd;
   ar << wp.ptMinPosition;
   ar << wp.ptMaxPosition;
   ar << wp.rcNormalPosition;
   return ar;
}

//////////////////
// Read window placement from archive
// WARNING: archiving functions are untested.
CArchive& operator>>(CArchive& ar, CWindowPlacement& wp)
{
   ar >> wp.length;
   ar >> wp.flags;
   ar >> wp.showCmd;
   ar >> wp.ptMinPosition;
   ar >> wp.ptMaxPosition;
   ar >> wp.rcNormalPosition;
   return ar;
}
