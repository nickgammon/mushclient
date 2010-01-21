// chatlistensock.h : interface of the CChatListenSocket class
//

#pragma once

class CMUSHclientDoc;
     
class CChatListenSocket : public CAsyncSocket
{
	DECLARE_DYNAMIC(CChatListenSocket);

// Construction
public:
	CChatListenSocket(CMUSHclientDoc* pDoc);
	~CChatListenSocket();

// Operations
public:
	CMUSHclientDoc* m_pDoc;

// Implementation

  virtual void OnAccept( int nErrorCode );

//  virtual void OnReceive(int nErrorCode);
//	virtual void OnSend(int nErrorCode);
//	virtual void OnClose(int nErrorCode);
//	virtual void OnConnect(int nErrorCode);


};
