// methods_clipboard.cpp

// Clipboard handling

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"

// Implements:

//    GetClipboard
//    SetClipboard


BSTR CMUSHclientDoc::GetClipboard() 
{
CString strContents; 

 GetClipboardContents (strContents, m_bUTF_8, false);   // no warning

	return strContents.AllocSysString();
}  // end of CMUSHclientDoc::GetClipboard



void CMUSHclientDoc::SetClipboard(LPCTSTR Text) 
{
putontoclipboard (Text);
}  // end of CMUSHclientDoc::SetClipboard
