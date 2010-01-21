#if !defined(AFX_UDPSOCKET_H__7F93BD46_41BB_11D9_9953_00008C012785__INCLUDED_)
#define AFX_UDPSOCKET_H__7F93BD46_41BB_11D9_9953_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UDPsocket.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// UDPsocket command target

class UDPsocket : public CAsyncSocket
{
// Attributes
public:

// Operations
public:
	UDPsocket(CMUSHclientDoc * pDoc);
	virtual ~UDPsocket();

  CMUSHclientDoc * m_pDoc;
  CString m_strScript;
  CString m_strPluginID;

// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(UDPsocket)
	//}}AFX_VIRTUAL

  virtual void OnReceive    (int nErrorCode);

	// Generated message map functions
	//{{AFX_MSG(UDPsocket)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

// Implementation
protected:
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UDPSOCKET_H__7F93BD46_41BB_11D9_9953_00008C012785__INCLUDED_)
