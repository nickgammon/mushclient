// UDPsocket.cpp : implementation file
//

#include "stdafx.h"
#include "mushclient.h"
#include "doc.h"
#include "UDPsocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// UDPsocket

UDPsocket::UDPsocket(CMUSHclientDoc * pDoc)
{
m_pDoc = pDoc;

}

UDPsocket::~UDPsocket()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(UDPsocket, CAsyncSocket)
	//{{AFX_MSG_MAP(UDPsocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// UDPsocket member functions

void UDPsocket::OnReceive(int nErrorCode)
  {
  char buff [1000];
  int count = Receive (buff, sizeof (buff) - 1);

  if (count == SOCKET_ERROR)
    return;   // strange

  buff [count] = 0;   // null terminate it

  // call script
  m_pDoc->CallPlugin (m_strPluginID, m_strScript, buff);

  }
