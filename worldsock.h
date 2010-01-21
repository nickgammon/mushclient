// worldsock.h : interface of the CWorldSocket class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1994 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef __WORLDSOCK_H__
#define __WORLDSOCK_H__

class CMUSHclientDoc;

class CWorldSocket : public CAsyncSocket
{
	DECLARE_DYNAMIC(CWorldSocket);

// Construction
public:
	CWorldSocket(CMUSHclientDoc* pDoc);

// Operations
public:
	CMUSHclientDoc* m_pDoc;
  CString m_outstanding_data;

// Implementation

  virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);
	virtual void OnClose(int nErrorCode);
	virtual void OnConnect(int nErrorCode);

};

#endif // __WORLDSOCK_H__
