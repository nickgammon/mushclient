////////////////////////////////////////////////////////////////
// 1997 Microsoft Systems Journal
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
#include "StdAfx.h"
#include "HyprLink.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////
// Navigate link -- ie, execute the file
// Returns instance handle of app run, or error code (just like ShellExecute)
//
HINSTANCE CHyperlink::Navigate()
{
	return  IsEmpty() ? NULL :
		ShellExecute(0, _T("open"), *this, 0, 0, SW_SHOWNORMAL);
}

