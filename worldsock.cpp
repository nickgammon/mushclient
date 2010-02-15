// worldsock.cpp : implementation file
//

// But does it get goat's blood out?



#include "stdafx.h"
#include "MUSHclient.h"

#include "mainfrm.h"

#include "doc.h"

#include <stddef.h>

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CWorldSocket, CAsyncSocket)

//#define new DEBUG_NEW 

CWorldSocket::CWorldSocket(CMUSHclientDoc* pDoc)
{
	m_pDoc = pDoc;
}

void CWorldSocket::OnReceive(int nErrorCode)
{

  // CSocket::OnReceive(nErrorCode);

	m_pDoc->ProcessPendingRead();
}

void CWorldSocket::OnSend(int nErrorCode)
{


int count; 

   if (nErrorCode)    // had an error, give up!
      return;

// if we have outstanding data to send, do it

  if (m_outstanding_data.GetLength () <= 0)
    return;

  count = Send (m_outstanding_data, m_outstanding_data.GetLength ());

  if (count != SOCKET_ERROR)
    m_pDoc->m_nBytesOut += count; // count bytes out

  if (count > 0)    // good send - do rest later
    m_outstanding_data = m_outstanding_data.Mid (count);
  else
    {
    int nError = GetLastError ();
    if (count == SOCKET_ERROR && nError != WSAEWOULDBLOCK)
      {
      ShutDownSocket (*this);
//       m_pSocket->OnClose (nError);      // ????
      m_outstanding_data.Empty ();

      }   // end of an error other than "would block"
    } // end of an error

}

void CWorldSocket::OnClose(int nErrorCode)
  {

bool bWasClosed = m_pDoc->m_iConnectPhase == eConnectNotConnected;

  TRACE1 ("CWorldSocket::OnClose, error code %i\n", nErrorCode);

  m_pDoc->m_iConnectPhase = eConnectDisconnecting;

  m_pDoc->UpdateAllViews (NULL);

  m_pDoc->MXP_Off (true);   // turn off MXP now

  // update button bar - make button red
  if (m_pDoc->m_view_number >= 1 &&
      m_pDoc->m_view_number <= 10)
    Frame.OnUpdateBtnWorlds (m_pDoc->m_view_number, NULL);

  // execute "disconnect" script
  if (m_pDoc->m_ScriptEngine)
    {
    if (m_pDoc->SeeIfHandlerCanExecute (m_pDoc->m_strWorldDisconnect))
      {
      DISPPARAMS params = { NULL, NULL, 0, 0 };
      long nInvocationCount = 0;

      m_pDoc->ExecuteScript (m_pDoc->m_dispidWorldDisconnect,  
                             m_pDoc->m_strWorldDisconnect,
                             eWorldAction,
                             "world disconnect", 
                             "disconnecting from world",
                             params, 
                             nInvocationCount); 
      }
    } // end of executing disconnect script

  // tell each plugin we have disconnected
  for (POSITION pos = m_pDoc->m_PluginList.GetHeadPosition(); pos; )
    {
    CPlugin * pPlugin = m_pDoc->m_PluginList.GetNext (pos);

    if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
      continue;

    pPlugin->ExecutePluginScript (ON_PLUGIN_DISCONNECT, pPlugin->m_dispid_plugin_disconnect);
    }   // end of doing each plugin

  // close log file if we auto-opened it
  if (!m_pDoc->m_strAutoLogFileName.IsEmpty ())
    m_pDoc->CloseLog ();


  if (m_pDoc->m_bShowConnectDisconnect && !bWasClosed)
    {
    CTime theTime;
    theTime = CTime::GetCurrentTime();

    CString strConnected;
    strConnected = theTime.Format (TranslateTime ("--- Disconnected on %A, %B %d, %Y, %#I:%M %p ---"));

    m_pDoc->Note (strConnected);  

    // find time spent connected
    CTimeSpan ts = CTime::GetCurrentTime() - m_pDoc->m_tConnectTime;
      
    CString strDuration = TFormat ("--- Connected for %i day%s, %i hour%s, %i minute%s, %i second%s. ---",
                  PLURAL ((long) ts.GetDays()),
                  PLURAL ((long) ts.GetHours()),
                  PLURAL ((long) ts.GetMinutes()),
                  PLURAL ((long) ts.GetSeconds()));

    m_pDoc->Note (strDuration);  

    // and a horizontal rule

    m_pDoc->m_pCurrentLine->flags = HORIZ_RULE;  
    m_pDoc->StartNewLine (true, 0); 
    }  // end of message in world window wanted

  CString strInfo = TFormat ("--- Received %i line%s, sent %i line%s.",
                PLURAL (m_pDoc->m_nTotalLinesReceived),
                PLURAL (m_pDoc->m_nTotalLinesSent)
                );
  
  m_pDoc->Note (strInfo);  

  strInfo = TFormat ("--- Output buffer has %i/%i line%s in it (%.1f%% full).",
                m_pDoc->m_LineList.GetCount (),
                m_pDoc->m_maxlines,
                (m_pDoc->m_LineList.GetCount ()) == 1 ? "" : "s",
                (double) m_pDoc->m_LineList.GetCount () / (double) m_pDoc->m_maxlines * 100.0
                );

  m_pDoc->Note (strInfo);  

  strInfo = TFormat ("--- Matched %i trigger%s, %i alias%s, and %i timer%s fired.",
                PLURAL (m_pDoc->m_iTriggersMatchedThisSessionCount),   
                PLURALES (m_pDoc->m_iAliasesMatchedThisSessionCount),    
                PLURAL (m_pDoc->m_iTimersFiredThisSessionCount)       
                );

  m_pDoc->Note (strInfo);  

CString str;

  str = TFormat ("The \"%s\" server has closed the connection", 
              (const char *) m_pDoc->m_mush_name);

  if (App.m_bNotifyOnDisconnect && !m_pDoc->m_bDisconnectOK)
    {
    if (App.m_bErrorNotificationToOutputWindow)
      m_pDoc->Note (str);
    else
      ::UMessageBox (str, MB_ICONEXCLAMATION);
    }
  else
    Frame.SetStatusMessage (str);
  m_pDoc->m_iConnectPhase = eConnectNotConnected;

  } // end of OnClose

void CWorldSocket::OnConnect(int nErrorCode)
  {

  m_pDoc->OnConnect (nErrorCode);
  } // end of OnConnect
