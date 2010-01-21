////////////////////////////////////////////////////////////////
// CWindowPlacement 1996 Microsoft Systems Journal.
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.

////////////////
// CWindowPlacement reads and writes WINDOWPLACEMENT 
// from/to application profile and CArchive.
//
struct CWindowPlacement : public WINDOWPLACEMENT {
   CWindowPlacement();
   ~CWindowPlacement();
   
   // Read/write to app profile
   void GetProfileWP(LPCSTR lpKeyName);
   void WriteProfileWP(LPCSTR lpKeyName);

   // Save/restore window pos (from app profile)
   void Save(LPCSTR lpKeyName, CWnd* pWnd);
   BOOL Restore(LPCSTR lpKeyName, 
                CWnd* pWnd,
                const bool bDesktop);

   // Save/restore from archive
   friend CArchive& operator<<(CArchive& ar, const CWindowPlacement& wp);
   friend CArchive& operator>>(CArchive& ar, CWindowPlacement& wp);
};
