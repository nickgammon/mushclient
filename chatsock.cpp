// chatsock.cpp : implementation file
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

#define CHAT_DEBUG 0

IMPLEMENT_DYNAMIC(CChatSocket, CAsyncSocket)

//#define new DEBUG_NEW 

CChatSocket::CChatSocket(CMUSHclientDoc* pDoc)
{
	m_pDoc = pDoc;
  m_hNameLookup = NULL;
  m_pGetHostStruct = NULL;
  ZeroMemory (&m_ServerAddr, sizeof m_ServerAddr);
  m_bDeleteMe = false;  
  m_bIncoming = false;
  m_bIgnore = false;
  m_bCanSnoop = false;          
  m_bYouAreSnooping = false;
  m_bHeIsSnooping = false;
  m_bCanSendCommands = false;   
  m_bPrivate = false;   
  m_bCanSendFiles = false;
  m_bDoingFileTransfer = false;
  m_bWasConnected = false;
  m_iChatStatus = eChatClosed;
  m_iChatConnectionType = eChatMudMaster;
  m_iUserOption = 0;

  // session ID

  if (++m_pDoc->m_iNextChatID > LONG_MAX)
    m_pDoc->m_iNextChatID = 1;    // wrap around
  
  m_iChatID = m_pDoc->m_iNextChatID;  // unique session ID

  // zChat

  SHS_INFO shsInfo;
  MakeRandomNumber (m_pDoc, shsInfo);

  m_zChatStamp = shsInfo.digest [0];
  m_zChatStatus = 1;      // normal status

  // times

  m_tWhenStarted = CTime::GetCurrentTime();
  m_tLastIncoming = 0;
  m_tLastOutgoing = 0;
  m_iPingStartTime.QuadPart = 0;
  m_fLastPingTime = 0.0;

  // their address, port

  m_iAllegedPort = DEFAULT_CHAT_PORT;
  m_strAllegedAddress = "<Unknown>";

  // file stuff

  m_bDoingFileTransfer = false;  
  m_bSendFile = false;           
  m_iFileSize = 0;           
  m_iFileBlocks = 0;         
  m_iBlocksTransferred = 0;  
  m_pFile = NULL;  
  m_pFileBuffer = NULL;
  m_tStartedFileTransfer = 0;
  m_iFileBlockSize = 500;

  // zero counters

  m_iCountIncomingPersonal = 0; 
  m_iCountIncomingAll = 0;      
  m_iCountIncomingGroup = 0;    
  m_iCountOutgoingPersonal = 0; 
  m_iCountOutgoingAll = 0;      
  m_iCountOutgoingGroup = 0;    
  m_iCountMessages = 0;
  m_iCountFileBytesIn = 0;   
  m_iCountFileBytesOut = 0;  
  
  }

CChatSocket::~CChatSocket()
{

  StopFileTransfer (true);

  // cancel pending host name lookup
  if (m_hNameLookup)
    WSACancelAsyncRequest (m_hNameLookup);  // cancel host name lookup in progress

  delete [] m_pGetHostStruct;

  ShutDownSocket (*this);

  // if he was ever connected, we will tell our plugins he has gone

  if (m_bWasConnected)
    {
    CPlugin * pSavedPlugin = m_pDoc->m_CurrentPlugin;

    // tell each plugin about the departing user
    for (POSITION pluginpos = m_pDoc->m_PluginList.GetHeadPosition(); pluginpos; )
      {
      CPlugin * pPlugin = m_pDoc->m_PluginList.GetNext (pluginpos);
      
      if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
        continue;

      // see what the plugin makes of this,
      pPlugin->ExecutePluginScript (ON_PLUGIN_CHAT_USERDISCONNECT, 
                            pPlugin->m_dispid_plugin_On_Chat_UserDisconnect, 
                            m_iChatID,     // user ID
                            string (m_strRemoteUserName)    // user name
                            );
      }   // end of doing each plugin
    m_pDoc->m_CurrentPlugin = pSavedPlugin;
    }   // end of needing to notify about him
}

void CChatSocket::StopFileTransfer (const bool bAbort)
  {
  if (!m_bDoingFileTransfer || m_pDoc == NULL)
    return;

  delete m_pFile;     // close file
  m_pFile = NULL;  

  delete [] m_pFileBuffer;
  m_pFileBuffer = NULL;

  if (bAbort)
    {
    // tell them to cancel it
    if (m_iChatStatus == eChatConnected)
      SendChatMessage (CHAT_FILE_CANCEL, "");

    // half-received - delete it
    if (m_bSendFile)
      m_pDoc->ChatNote (eChatFile,
              CFormat (_T("Aborted sending file %s"), (LPCTSTR) m_strOurFileName));
    else
      {
      m_pDoc->ChatNote (eChatFile,
              CFormat (_T("Aborted receiving file %s"), (LPCTSTR) m_strOurFileName));
      m_pDoc->ChatNote (eChatFile,
              CFormat (_T("File %s deleted."), (LPCTSTR) m_strOurFileName));
      CFile::Remove (m_strOurFileName);
      }
    }

  m_bDoingFileTransfer = false;  
  m_bSendFile = false;           
  m_iFileSize = 0;           
  m_iFileBlocks = 0;         
  m_iBlocksTransferred = 0;  
  m_tStartedFileTransfer = 0;

  } // end of CChatSocket::StopFileTransfer 


void CChatSocket::OnReceive(int nErrorCode)
{

char buff [1000];
int count = Receive (buff, sizeof (buff) - 1);
POSITION pluginpos;

  if (count == SOCKET_ERROR)
    {
    OnClose (GetLastError ());
    return;
    }

  if (count <= 0)
    return;

  m_tLastIncoming = CTime::GetCurrentTime();

  switch (m_iChatStatus)
    {
    case eChatAwaitingConnectConfirm:
      {

#if CHAT_DEBUG

      m_pDoc->Note (TFormat (_T("Incoming packet on %i: \"%s\""),
                    m_iChatID,
                    (LPCTSTR) CString (buff, count)));

#endif

      if (count < 5 || memcmp (buff, "YES:", 4) != 0)
        {
        m_pDoc->ChatNote (eChatConnection,
                _T("Server rejected chat session attempt."));
        OnClose (0);
        return;
        } // end of not being accepted

      // get rid of "YES:" (4 bytes)
      memmove (buff, &buff [4], count - 4);
      count -= 4;

      CString strName = CString (buff, count);
      int i = strName.Find ('\n');
      if (i == -1)
        m_strRemoteUserName = strName;
      else
        m_strRemoteUserName = strName.Left (i);

      m_pDoc->ChatNote (eChatSession,
                CFormat (_T("Chat session accepted, remote server: \"%s\""),
              (LPCTSTR) m_strRemoteUserName));

      if (i == -1)
        count = 0;
      else
        {   // get rid of initial acceptance message
        i++;     // get rid of \n too
        memmove (buff, &buff [i], count - i);
        count -= i;
        }

      // connected now
      m_iChatStatus = eChatConnected;
      m_bWasConnected = true;

      // identify ourselves
      CString strVersion = "MUSHclient v";
      strVersion += MUSHCLIENT_VERSION;
      SendChatMessage (CHAT_VERSION, strVersion);
      
      if (m_iChatConnectionType == eChatZMud)
        {
        SendChatMessage (CHAT_STATUS, (char) 1);  // normal status
        SendChatMessage (CHAT_STAMP, MakeStamp (m_zChatStamp));  // send it
        }

      CPlugin * pSavedPlugin = m_pDoc->m_CurrentPlugin;

      // tell each plugin about the new user
      for (pluginpos = m_pDoc->m_PluginList.GetHeadPosition(); pluginpos; )
        {
        CPlugin * pPlugin = m_pDoc->m_PluginList.GetNext (pluginpos);

        if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
          continue;

        // see what the plugin makes of this,
        pPlugin->ExecutePluginScript (ON_PLUGIN_CHAT_NEWUSER, 
                              pPlugin->m_dispid_plugin_On_Chat_NewUser, 
                              m_iChatID,     // user ID
                              string (m_strRemoteUserName)    // user name
                              );
        }   // end of doing each plugin
      m_pDoc->m_CurrentPlugin = pSavedPlugin;

      if (count <= 0)
        return;       // only the negotiation text here

      }
     break;   // end of eChatAwaitingConnectConfirm

    case eChatAwaitingConnectionRequest:

#if CHAT_DEBUG

      m_pDoc->Note (TFormat ("Incoming packet on %i: \"%s\"",
                    m_iChatID,
                    (LPCTSTR) CString (buff, count)));

#endif

      if (count < 7 || !(memcmp (buff, "CHAT:", 5) == 0 ||
                         memcmp (buff, "ZCHAT:", 6) == 0))
        {
        m_pDoc->ChatNote (eChatConnection,
                      _T("Unexpected chat negotiation."));
        SendData ("NO");
        OnClose (0);
        return;
        } // end of not accepting it

      // get rid of "CHAT:" (5 bytes)
      if (memcmp (buff, "CHAT:", 5) == 0)
        {
        memmove (buff, &buff [5], count - 5);
        count -= 5;
        m_iChatConnectionType = eChatMudMaster;
        }
      else
        {
        memmove (buff, &buff [6], count - 6);
        count -= 6;
        m_iChatConnectionType = eChatZMud;
        m_iFileBlockSize = 1024;    // larger size for zChat
        }

      CString strName = CString (buff, count);
      int i = strName.Find ('\n');
      if (i == -1)
        m_strRemoteUserName = strName;
      else
        m_strRemoteUserName = strName.Left (i);

      // zChat has a tab after the name
      int iTab = m_strRemoteUserName.Find ('\t');
      if (iTab != -1)
        m_strRemoteUserName = m_strRemoteUserName.Left (iTab);

      if (i == -1)
        count = 0;
      else
        {   // get rid of initial connection message
        i++;     // get rid of \n too
        memmove (buff, &buff [i], count - i);
        count -= i;
        }

      CString strRest = CString (buff, count);

      // throw away zChat security stuff (after the next newline)
      i = strRest.Find ('\n');
      if (i != -1)
        strRest = strRest.Left (i);

      if (strRest.GetLength () > 5)
        {
        m_iAllegedPort = atoi (strRest.Right (5));
        m_strAllegedAddress = strRest.Left (strRest.GetLength () - 5);
        }

      count = 0;    // can't see how this packet can be reasonably terminated

      CPlugin * pSavedPlugin = m_pDoc->m_CurrentPlugin;

      // tell each plugin what we are about to display
      for (pluginpos = m_pDoc->m_PluginList.GetHeadPosition(); pluginpos; )
        {
        CPlugin * pPlugin = m_pDoc->m_PluginList.GetNext (pluginpos);

        if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
          continue;

        // see what the plugin makes of this,
        if (!pPlugin->ExecutePluginScript (ON_PLUGIN_CHAT_ACCEPT, 
                              pPlugin->m_dispid_plugin_On_Chat_Accept, 
                              CFormat ("%s,%s",
                              (LPCTSTR) inet_ntoa (m_ServerAddr.sin_addr),
                              (LPCTSTR) m_strRemoteUserName
                              )))
          {
          m_pDoc->m_CurrentPlugin = pSavedPlugin;
          // tell them our rejection
          SendData ("NO");
          OnClose (0);
          return;   // false means plugin rejects him 
          }
        }   // end of doing each plugin
      m_pDoc->m_CurrentPlugin = pSavedPlugin;

      if (m_pDoc->m_bValidateIncomingCalls)
        {
        m_pDoc->Activate ();
        if (::UMessageBox (
              TFormat ("Incoming chat call to world %s from %s, IP address: %s.\n\nAccept it?",
                       (LPCTSTR) m_pDoc->m_mush_name,
                       (LPCTSTR) m_strRemoteUserName,
                       (LPCTSTR) inet_ntoa (m_ServerAddr.sin_addr)),
            MB_YESNO) != IDYES)
          {
          // tell them our rejection
          SendData ("NO");
          OnClose (0);
          return;   
          }

        }  // end of needing to validate calls

      m_pDoc->ChatNote (eChatSession,
                  TFormat ("Chat session accepted, remote user: \"%s\"",
                    (LPCTSTR) m_strRemoteUserName));

      // tell them our acceptance, and chat name
      SendData (CFormat ("YES:%s\n", m_pDoc->m_strOurChatName));

      // connected now
      m_iChatStatus = eChatConnected;
      m_bWasConnected = true;

      // identify ourselves
      CString strVersion = "MUSHclient v";
      strVersion += MUSHCLIENT_VERSION;
      SendChatMessage (CHAT_VERSION, strVersion);

      if (m_iChatConnectionType == eChatZMud)
        {
        SendChatMessage (CHAT_STATUS, (char) 1);  // normal status
        SendChatMessage (CHAT_STAMP, MakeStamp (m_zChatStamp));  // send it
        }

      pSavedPlugin = m_pDoc->m_CurrentPlugin;

      // tell each plugin about the new user
      for (POSITION pluginpos = m_pDoc->m_PluginList.GetHeadPosition(); pluginpos; )
        {
        CPlugin * pPlugin = m_pDoc->m_PluginList.GetNext (pluginpos);

        if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
          continue;

        // see what the plugin makes of this,
        pPlugin->ExecutePluginScript (ON_PLUGIN_CHAT_NEWUSER, 
                              pPlugin->m_dispid_plugin_On_Chat_NewUser, 
                              m_iChatID,     // user ID
                              string (m_strRemoteUserName)    // user name
                              );
        }   // end of doing each plugin
      m_pDoc->m_CurrentPlugin = pSavedPlugin;

      if (count <= 0)
        return;       // only the negotiation text here

      break;  // end of eChatAwaitingConnectionRequest
    } // end of switch on m_iChatStatus

// now take the incoming text and break into blocks

  m_outstanding_input += CString (buff, count); // add to any left over

  while (!m_bDeleteMe && !m_outstanding_input.IsEmpty ())
    {

    switch (m_iChatConnectionType)
      {

      case eChatMudMaster:
        {
        // file blocks are fixed length and might contain nulls

        if (m_outstanding_input [0] == CHAT_FILE_BLOCK)
          {
          int iLength = m_outstanding_input.GetLength ();

          // must have file block size + 2 (command and terminator)
          // otherwise we will get them later
          if (iLength < (m_iFileBlockSize + 2))
            return;

          // make a buffer of exactly the file data
          CString strBuffer = m_outstanding_input.Mid (1, m_iFileBlockSize);

          // discard the file block and message number and terminator byte
          m_outstanding_input = m_outstanding_input.Mid (m_iFileBlockSize + 2);

          // now process the incoming file block
          ProcessChatMessage (CHAT_FILE_BLOCK, strBuffer);

          }   // end of file block
        else
          {   // not a file block - use variable-length terminator

          int iTerminator = m_outstanding_input.Find ((char) CHAT_END_OF_COMMAND);

          // if no terminator, wait for one to arrive in the next packet
          if (iTerminator == -1)
            return;   

          CString strChatMessage = m_outstanding_input.Left (iTerminator);
          m_outstanding_input = m_outstanding_input.Mid (iTerminator + 1);

          ProcessChatMessage ((unsigned char) strChatMessage [0], strChatMessage.Mid (1));
          }   // end of not file block
        } // end of MudMaster chat type
        break;

      case eChatZMud:
        {
        int iInputLength = m_outstanding_input.GetLength ();
        // must have 4 bytes (command:2, and length:2) or we cannot have a packet
        if (iInputLength < 4)
          return;    // wait for them

        int iCommand = (int) (unsigned char) m_outstanding_input [0] |
                       (((int) (unsigned char) m_outstanding_input [1]) << 8);

        int iLength = (int) (unsigned char) m_outstanding_input [2] |
                       (((int) (unsigned char) m_outstanding_input [3]) << 8);

        if (iInputLength < (iLength + 4))
          return;   // whole block has not arrived yet

        CString strChatMessage = m_outstanding_input.Mid (4, iLength);
        m_outstanding_input = m_outstanding_input.Mid (iLength + 4);

        ProcessChatMessage (iCommand, strChatMessage);

        } // end of zChat chat type
        break;

      } // end of switch on chat connection type

    }   // end of extracting messages from the incoming stream


}

void CChatSocket::OnSend(int nErrorCode)
{
int count; 

  // receive nothing if shutting down
  if (m_bDeleteMe)
    return;

   if (nErrorCode)    // had an error, give up!
      return;

// if we have outstanding data to send, do it

  if (m_outstanding_output.GetLength () <= 0)
    return;

  count = Send (m_outstanding_output, m_outstanding_output.GetLength ());

  if (count != SOCKET_ERROR)
    m_pDoc->m_nBytesOut += count; // count bytes out

  if (count > 0)    // good send - do rest later
    m_outstanding_output = m_outstanding_output.Mid (count);
  else
    {
    int nError = GetLastError ();
    if (count == SOCKET_ERROR && nError != WSAEWOULDBLOCK)
      {
      m_pDoc->ChatNote (eChatConnection,
                  TFormat ("Unable to send to \"%s\", code = %i (%s)", 
                      (LPCTSTR) m_strServerName, 
                      nError,
                      m_pDoc->GetSocketError (nError)));
      ShutDownSocket (*this);
      m_outstanding_output.Empty ();
      OnClose (nError);      // ????
      }   // end of an error other than "would block"
    } // end of an error

}

void CChatSocket::OnClose(int nErrorCode)
  {

  if (m_iChatStatus == eChatConnected)
    m_pDoc->ChatNote (eChatSession,
              CFormat ("Chat session to %s closed.",
                        (LPCTSTR) m_strRemoteUserName));

  m_iChatStatus = eChatClosed;
  m_bDeleteMe = true;

  } // end of OnClose

void CChatSocket::OnConnect(int nErrorCode)
  {

  if (nErrorCode != 0)
    {
    m_pDoc->ChatNote (eChatConnection,
                TFormat ("Unable to connect to \"%s\", code = %i (%s)", 
                    (LPCTSTR) m_strServerName, 
                    nErrorCode,
                    m_pDoc->GetSocketError (nErrorCode)));
    OnClose (nErrorCode);
    return;
    }

  m_pDoc->ChatNote (eChatSession,
              TFormat ("Session established to %s.", 
                    (LPCTSTR) m_strServerName));

CString strHostName;
CString strAddresses;

  GetHostNameAndAddresses (strHostName, strAddresses);
  
  // if more than one IP address, take first
  int i = strAddresses.Find (',');
  if (i != -1)
     strAddresses = strAddresses.Left (i);

  // tell them our chat name, IP address, incoming port
  if (m_iChatConnectionType == eChatZMud)
    SendData (CFormat ("ZCHAT:%s\t\n%s%05u",
              m_pDoc->m_strOurChatName, strAddresses, m_pDoc->m_IncomingChatPort));
  else
    SendData (CFormat ("CHAT:%s\n%s%-5u",
              m_pDoc->m_strOurChatName, strAddresses, m_pDoc->m_IncomingChatPort));

  m_iChatStatus = eChatAwaitingConnectConfirm;

  } // end of OnConnect

void CChatSocket::HostNameResolved (WPARAM wParam, LPARAM lParam)
  {
  m_hNameLookup = NULL;   // handle not needed now

  if (WSAGETASYNCERROR (lParam))
    {
    m_pDoc->ChatNote (eChatConnection,
                TFormat ("Chat session cannot resolve host name: %s.",
                (LPCTSTR) m_strServerName));
    m_bDeleteMe = true;
    m_iChatStatus = eChatClosed;
    return;
    }   // end of error in host name lookup

struct hostent * pHostent = (struct hostent * ) m_pGetHostStruct;

  m_ServerAddr.sin_addr.s_addr = ((LPIN_ADDR)pHostent->h_addr)->s_addr;

  delete [] m_pGetHostStruct;   // delete buffer used by host name lookup
  m_pGetHostStruct = NULL;

  // we know the address - get on with it

  MakeCall ();

  } // end of HostNameResolved

void CChatSocket::MakeCall (void)
  {

  // the alleged address and port are what we actually used :)
  m_strAllegedAddress = inet_ntoa (m_ServerAddr.sin_addr);
  m_iAllegedPort = ntohs (m_ServerAddr.sin_port);


// first check if we are already connected
  for (POSITION chatpos = m_pDoc->m_ChatList.GetHeadPosition (); chatpos; )
    {
    CChatSocket * pSocket = m_pDoc->m_ChatList.GetNext (chatpos);
    if (pSocket == this || pSocket->m_iChatStatus != eChatConnected)
      continue;

    if (m_strAllegedAddress == inet_ntoa (pSocket->m_ServerAddr.sin_addr)  &&
        ntohs (pSocket->m_ServerAddr.sin_port) == m_iAllegedPort)
      {
      m_pDoc->ChatNote (eChatConnection,
                  TFormat ("You are already connected to %s port %d",
                        (LPCTSTR) m_strAllegedAddress, 
                        m_iAllegedPort));

      OnClose (0);
      return;
      }       // end of found it
    } // end of checking them

  m_pDoc->ChatNote (eChatConnection,
              TFormat ("Calling chat server at %s port %d",
                    (LPCTSTR) m_strAllegedAddress,
                    m_iAllegedPort));

  m_iChatStatus = eChatConnecting;


BOOL connected = Connect((SOCKADDR*)&m_ServerAddr, sizeof(m_ServerAddr));

  if (connected)
    {
    OnConnect (0);    // we have connected already! Do logon of character etc.
    return;
    }

// if error code is "would block" then it will finish later

  int iStatus = GetLastError ();

  if (iStatus == WSAEWOULDBLOCK)
    return;

  // this will display the error message
  OnConnect (iStatus);

  } // end of MakeCall


void CChatSocket::SendData (const CString & strText)
  {

  // send nothing if shutting down
  if (m_bDeleteMe)
    return;

  m_tLastOutgoing = CTime::GetCurrentTime();

  m_outstanding_output += strText;

  OnSend (0);   // in case FD_WRITE message got lost, try to send again
  
  return;
  } // end of SendData

void CChatSocket::ProcessChatMessage (const int iMessage, const CString strMessage)
  {
  if (m_bDeleteMe || m_iChatStatus != eChatConnected)
    return;

#if CHAT_DEBUG
  m_pDoc->Note (TFormat ("Received chat message %i on %i, data: \"%s\"",
                        iMessage, m_iChatID,
                        (LPCTSTR) strMessage));

#endif

  CPlugin * pSavedPlugin = m_pDoc->m_CurrentPlugin;

  // tell each plugin what we are about to display
  for (POSITION pluginpos = m_pDoc->m_PluginList.GetHeadPosition(); pluginpos; )
    {
    CPlugin * pPlugin = m_pDoc->m_PluginList.GetNext (pluginpos);

    if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
      continue;

    // see what the plugin makes of this,
    if (!pPlugin->ExecutePluginScript (ON_PLUGIN_CHAT_MESSAGE, 
                          pPlugin->m_dispid_plugin_On_Chat_Message,
                          m_iChatID,    // who we are
                          iMessage,     // message number
                          string (strMessage)    // message text
                          ))                                           
      {                                                                
      m_pDoc->m_CurrentPlugin = pSavedPlugin;                                  
      return;   // false means plugin handled it                       
      }
    }   // end of doing each plugin
  m_pDoc->m_CurrentPlugin = pSavedPlugin;

  switch (iMessage)
    {
    case CHAT_TEXT_EVERYBODY:      Process_Text_everybody       (strMessage); break;     
    case CHAT_TEXT_PERSONAL:       Process_Text_personal        (strMessage); break;     
    case CHAT_MESSAGE:             Process_Message              (strMessage); break;     
    case CHAT_TEXT_GROUP:          Process_Text_group           (strMessage); break;     
    case CHAT_PING_REQUEST:        Process_Ping_request         (strMessage); break;     
    case CHAT_PING_RESPONSE:       Process_Ping_response        (strMessage); break;    
    case CHAT_VERSION:             Process_Version              (strMessage); break;    
    case CHAT_REQUEST_CONNECTIONS: Process_Request_connections  (strMessage); break;  
    case CHAT_CONNECTION_LIST:     Process_Connection_list      (strMessage); break;    
    case CHAT_PEEK_CONNECTIONS:    Process_Peek_connections     (strMessage); break;   
    case CHAT_PEEK_LIST:           Process_Peek_list            (strMessage); break;          
    case CHAT_SNOOP:               Process_Snoop                (strMessage); break;              
    case CHAT_SNOOP_DATA:          Process_Snoop_data           (strMessage); break;        
    case CHAT_NAME_CHANGE:         Process_Name_change          (strMessage); break;       
    case CHAT_SEND_COMMAND:        Process_Send_command         (strMessage); break; 
    case CHAT_FILE_START:          Process_File_start				    (strMessage); break;
    case CHAT_FILE_DENY:           Process_File_deny				    (strMessage); break;
    case CHAT_FILE_BLOCK_REQUEST:  Process_File_block_request   (strMessage); break;
    case CHAT_FILE_BLOCK:          Process_File_block           (strMessage); break;
    case CHAT_FILE_END:            Process_File_end				      (strMessage); break;
    case CHAT_FILE_CANCEL:         Process_File_cancel				  (strMessage); break;

    case CHAT_ICON:                Process_Icon                 (strMessage); break;
    case CHAT_STATUS:              Process_Status               (strMessage); break;
    case CHAT_EMAIL_ADDRESS:       Process_EmailAddress				  (strMessage); break;
    case CHAT_STAMP:               Process_Stamp				        (strMessage); break;
    case CHAT_REQUEST_PGP_KEY:     Process_RequestPGPkey				(strMessage); break;
    case CHAT_PGP_KEY:             Process_PGPkey				        (strMessage); break;

    default:

      // tell them we don't support that
      SendChatMessage (CHAT_MESSAGE, 
          TFormat ("\n%s does not support the chat command %i.\n",
                    m_pDoc->m_strOurChatName, iMessage));
      // tell us we got it
      m_pDoc->ChatNote (eChatInformation,
                  TFormat ("Received unsupported chat command %i from %s",
                           iMessage,
                           (LPCTSTR) m_strRemoteUserName));
      break;

    } // end of switch


  } // end of ProcessChatMessage

// sends a chat message to the other end
void CChatSocket::SendChatMessage (const int iMessage, 
                                   const CString strMessage,
                                   const long iStamp)
  {
  if (m_bDeleteMe || m_iChatStatus != eChatConnected)
    return;

#if CHAT_DEBUG
  m_pDoc->Note (TFormat ("Sending chat message %i on %i, data: \"%s\"",
                        iMessage, m_iChatID,
                        (LPCTSTR) strMessage));

#endif

  CPlugin * pSavedPlugin = m_pDoc->m_CurrentPlugin;
  // tell each plugin what we are about to display
  for (POSITION pluginpos = m_pDoc->m_PluginList.GetHeadPosition(); pluginpos; )
    {
    CPlugin * pPlugin = m_pDoc->m_PluginList.GetNext (pluginpos);

    if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
      continue;

    // see what the plugin makes of this,
    if (!pPlugin->ExecutePluginScript (ON_PLUGIN_CHAT_MESSAGE_OUT, 
                          pPlugin->m_dispid_plugin_On_Chat_MessageOut,
                          m_iChatID,    // which chat ID
                          iMessage,     // message number
                          string (strMessage)    // message text
                          ))
      {
      m_pDoc->m_CurrentPlugin = pSavedPlugin;
      return;   // false means plugin discarded it 
      }
    }   // end of doing each plugin
  m_pDoc->m_CurrentPlugin = pSavedPlugin;

  if (iMessage == CHAT_SNOOP && m_bYouAreSnooping)
    m_bYouAreSnooping = false;
  
  CString strData;

  switch (m_iChatConnectionType)
    {

    case eChatMudMaster:
      {
      // messages start with a message number and end with 0xFF
      strData = (char) (unsigned char) iMessage;
      strData += strMessage;
      // can't have 0xFF imbedded in messages, except file blocks :)
      // 0xFF is y with 2 dots on it so y will look reasonable 
      if (iMessage != CHAT_FILE_BLOCK)
        strData.Replace ((unsigned char) CHAT_END_OF_COMMAND, 'y');
      strData += (unsigned char) CHAT_END_OF_COMMAND;
      }
      break;

    case eChatZMud:
      {
      // start of with 2 bytes of message number
      strData =  (char) (unsigned char) (iMessage & 0xFF);
      strData += (unsigned char) ((iMessage >> 8) & 0xFF);
      CString strStampedMessage;

      // now add message stamp if message requires
      switch (iMessage)
        {
        // stamp it if necessary
        case CHAT_TEXT_EVERYBODY:
        case CHAT_TEXT_PERSONAL:
        case CHAT_TEXT_GROUP:
          // zero means use this connection's stamp
          strStampedMessage = MakeStamp (iStamp ? iStamp : m_zChatStamp) +
                              strMessage;
          break;

        default:
          strStampedMessage = strMessage;
          break;

        }

      // then 2 bytes of message data length
      int iLength = strStampedMessage.GetLength ();
      strData += (char) (unsigned char) (iLength & 0xFF);
      strData += (char) (unsigned char) ((iLength >> 8) & 0xFF);

      // then the message itself
      strData += strStampedMessage;
      }
      break;

    } // end of switch on chat connection type

  SendData (strData);

  } // end of ProcessChatMessage


void CChatSocket::Process_Name_change				  (const CString strMessage)
  {
  CString strOldName = m_strRemoteUserName;
  m_strRemoteUserName = strMessage;

  m_pDoc->ChatNote (eChatNameChange,
              TFormat ("%s has changed his/her name to %s.",
                            (LPCTSTR) strOldName,
                            (LPCTSTR) m_strRemoteUserName));

  } // end of CChatSocket::Process_Name_change	

void CChatSocket::Process_Request_connections (const CString strMessage)
  {
  CString strResult;

  for (POSITION chatpos = m_pDoc->m_ChatList.GetHeadPosition (); chatpos; )
    {
    CChatSocket * pSocket = m_pDoc->m_ChatList.GetNext (chatpos);
    if (pSocket->m_iChatStatus == eChatConnected && !pSocket->m_bPrivate)
      {
      // omit self
      if (pSocket == this)
        continue;

      if (!strResult.IsEmpty ())
        strResult += ",";

      strResult += pSocket->m_strAllegedAddress;
      strResult += ",";
      strResult += CFormat ("%d", pSocket->m_iAllegedPort);

      }   // end of this one wanted
    }   // end of doing all

  m_pDoc->ChatNote (eChatInformation,
              TFormat ("%s has requested your public connections", 
                    (LPCTSTR) m_strRemoteUserName));
  SendChatMessage (CHAT_CONNECTION_LIST, strResult);
  } // end of CChatSocket::Process_Request_connections

void CChatSocket::Process_Connection_list	 		(const CString strMessage)
  {
  CStringList strList;
  StringToList (strMessage, ",", strList);
  int iCount = strList.GetCount () / 2;   // number of connections
  m_pDoc->ChatNote (eChatConnectionList,
                TFormat ("Found %i connection%s to %s", 
                    PLURAL (iCount),
                    (LPCTSTR) m_strRemoteUserName));
  
  // for each one, connect to it
  for (int i = 0; i < iCount; i++)
    {
    CString strIP;
    long iPort;

    strIP = strList.RemoveHead ();
    iPort = atol (strList.RemoveHead ());

    m_pDoc->ChatCall(strIP, iPort);
    }   // end of doing each connection
  } // end of CChatSocket::Process_Connection_list	

void CChatSocket::Process_Text_everybody			(const CString strMessage)
  {

  CString strFixedMessage = strMessage;
  long iStamp = GetStamp (strFixedMessage);

  if (!m_bIgnore)
    {
    // anti-message loop provision
    if (strFixedMessage == m_pDoc->m_strLastMessageSent)
      {
      CTimeSpan timediff = CTime::GetCurrentTime() -  m_pDoc->m_tLastMessageTime;
      if (timediff.GetTotalSeconds () < LOOP_DISCARD_SAME_MESSAGE_SECONDS)
        return;
      } // end of same message that we just sent

    // zChat anti-loop provision
    if (m_iChatConnectionType == eChatZMud &&
        m_zChatStamp == iStamp)
        return;

    m_iCountIncomingAll++;
    m_pDoc->ChatNote (eChatIncomingEverybody, strFixedMessage);
    if (m_bIncoming)
      m_pDoc->SendChatMessageToAll (CHAT_TEXT_EVERYBODY,
                              strFixedMessage,
                              true,      // unless ignoring them
                              false,     // to everyone
                              false,     
                              m_iChatID, // except us
                              "",        // no particular group
                              iStamp);   // use this message stamp
    else
      m_pDoc->SendChatMessageToAll (CHAT_TEXT_EVERYBODY,
                              strFixedMessage,
                              true,      // unless ignoring them
                              true,      // to incoming only (ones we serve)
                              false,
                              m_iChatID, // except us
                              "",        // no particular group
                              iStamp);   // use this message stamp

    } // end of not ignoring him
  }  // end of CChatSocket::Process_Text_everybody

void CChatSocket::Process_Text_personal			  (const CString strMessage)
  {
  if (!m_bIgnore)
    {

    CString strFixedMessage = strMessage;
    long iStamp = GetStamp (strFixedMessage);

    // zChat anti-loop provision
    if (m_iChatConnectionType == eChatZMud &&
        m_zChatStamp == iStamp)
        return;

    m_iCountIncomingPersonal++;
    m_pDoc->ChatNote (eChatIncomingPersonal, strFixedMessage);
    }
  } // end of CChatSocket::Process_Text_personal 

void CChatSocket::Process_Text_group				  (const CString strMessage)
  {
  if (!m_bIgnore)
    {
    // anti-message loop provision
    if (strMessage == m_pDoc->m_strLastGroupMessageSent)
      {
      CTimeSpan timediff = CTime::GetCurrentTime() -  m_pDoc->m_tLastGroupMessageTime;
      if (timediff.GetTotalSeconds () < LOOP_DISCARD_SAME_MESSAGE_SECONDS)
        return;
      } // end of same message that we just sent

    CString strFixedMessage = strMessage;
    long iStamp = GetStamp (strFixedMessage);

    // zChat anti-loop provision
    if (m_iChatConnectionType == eChatZMud &&
        m_zChatStamp == iStamp)
        return;

    if (strFixedMessage.GetLength () > 15)
      {
      m_iCountIncomingGroup++;
      CString strGroup = strFixedMessage.Left (15);
      strGroup.TrimLeft ();
      strGroup.TrimRight ();
      m_pDoc->ChatNote (eChatIncomingGroup, strFixedMessage.Mid (15));   // strip group
      if (!m_bIncoming)   // if from one we called
        m_pDoc->SendChatMessageToAll (CHAT_TEXT_GROUP,
                                strFixedMessage,
                                true,      // unless ignoring them
                                true,      // to incoming only (ones we serve)
                                false,
                                m_iChatID,  // except us
                                strGroup, // onsend to same group
                                iStamp);    // use this message stamp
      }   // end of message not ridiculously small
    } // end of not ignoring

  } // end of CChatSocket::Process_Text_group

void CChatSocket::Process_Message					    (const CString strMessage)
  {
  if (!m_bIgnore)
    {
    m_iCountMessages++;
    m_pDoc->ChatNote (eChatMessage, strMessage);
    }

  } // end of CChatSocket::Process_Message	

void CChatSocket::Process_Version					    (const CString strMessage)
  {
  m_strRemoteVersion = strMessage;    // remember it for later
  } // end of CChatSocket::Process_Version	

void CChatSocket::Process_File_start				  (const CString strMessage)
  {
  if (!m_bCanSendFiles)
    {
    SendChatMessage (CHAT_FILE_DENY, 
                    TFormat ("%s is not allowing file transfers from you.",
                    (LPCTSTR) m_pDoc->m_strOurChatName));
    return;
    }

  CStringList strList;
  StringToList (strMessage, ",", strList);
  
  if (strList.GetCount () != 2)
    {
    SendChatMessage (CHAT_FILE_DENY, 
                    "Expected \"filename,filesize\" but did not get that.");
    return;
    }

  m_strSenderFileName = strList.RemoveHead ();

  if (m_strSenderFileName.Find (_T("/")) != -1 ||
      m_strSenderFileName.Find (_T("\\")) != -1)
    {
    SendChatMessage (CHAT_FILE_DENY, 
                    TFormat ("Supplied file name of \"%s\" may not contain slashes.",
                    (LPCTSTR) m_strSenderFileName));
    return;
    }

  CString strSize = strList.RemoveHead ();

  if (!IsNumber (strSize))
    {
    SendChatMessage (CHAT_FILE_DENY, 
                    "File size was not numeric.");
    return;
    }
  
  m_iFileSize = atol (strSize);

  bool bWanted = true;

  double K = m_iFileSize / 1024.0;

  if (!m_pDoc->m_bAutoAllowFiles)
    {
     m_pDoc->Activate ();
     if (UMessageBox (
        TFormat ("%s wishes to send you the file \"%s\", size %ld bytes (%1.1f Kb).\n\nAccept it?",
                 (LPCTSTR) m_strRemoteUserName,
                 (LPCTSTR) m_strSenderFileName,
                 m_iFileSize,
                 K),
      MB_YESNO) != IDYES)
      bWanted = false;
    }

  if (bWanted)
    if (m_pDoc->m_strChatSaveDirectory.IsEmpty () || 
         !m_pDoc->m_bAutoAllowFiles)  // no auto-save
      {
      CFileDialog filedlg (FALSE,   // saving the file
                           _T(""),    // default extension
                           m_strSenderFileName,  // suggested name
                           OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                           _T("All files (*.*)|*.*|")
                           _T("|"),    // filter 
                           &Frame);  // parent window

      CString strTitle =  TFormat ("Chat: Save file from %s as ...",
                                          (LPCTSTR) m_strRemoteUserName);
      filedlg.m_ofn.lpstrTitle = strTitle;

      // use file save directory if defined  
      if (!m_pDoc->m_strChatSaveDirectory.IsEmpty ())
       filedlg.m_ofn.lpstrInitialDir = m_pDoc->m_strChatSaveDirectory;

      ChangeToFileBrowsingDirectory ();
      if (filedlg.DoModal () != IDOK)
        bWanted = false;    // cancelled dialog
      ChangeToStartupDirectory ();

      m_strOurFileName = filedlg.GetPathName ();
      }
    else
      {
      // want backslashes not forwards slashes
      m_pDoc->m_strChatSaveDirectory.Replace (_T("/"), _T("\\"));
      // ensure trailing slash
      if (m_pDoc->m_strChatSaveDirectory.Right (1) != TEXT ('\\'))
        m_pDoc->m_strChatSaveDirectory += "\\";
  
      m_strOurFileName = m_pDoc->m_strChatSaveDirectory + m_strSenderFileName;
      }

  if (!bWanted)
    {
    // tell them our rejection
    SendChatMessage (CHAT_FILE_DENY, 
                    TFormat ("%s does not want that particular file.",
                    (LPCTSTR) m_pDoc->m_strOurChatName));
    return;
    }   // end of this file not wanted right now

  try
    {
    m_pFile = new CFile (m_strOurFileName, CFile::modeCreate | CFile::modeReadWrite); // new file
    } // end of try block

  catch (CFileException * e)
    {
    SendChatMessage (CHAT_FILE_DENY, 
                    TFormat ("%s can not open that file.",
                    (LPCTSTR) m_pDoc->m_strOurChatName));
    m_pDoc->ChatNote (eChatFile, TFormat ("File %s cannot be opened.", 
                      (LPCTSTR) m_strOurFileName));
    e->Delete ();
    // reset the two fields we changed so far
    m_strOurFileName.Empty ();
    m_iFileSize = 0;

    delete m_pFile;    // in case it was set up

    return;
    } // end of catching a file exception

  // tell them our acceptance - get first block
  SendChatMessage (CHAT_FILE_BLOCK_REQUEST, "");

  m_pDoc->ChatNote (eChatFile,
              TFormat (
        "Receiving a file from %s -- Filename: %s, Length: %ld bytes (%1.1f Kb).",
                            (LPCTSTR) m_strRemoteUserName,
                            (LPCTSTR) m_strOurFileName,
                            m_iFileSize,
                            K));

  // get ready for transfer
  m_tStartedFileTransfer = CTime::GetCurrentTime();  // when started
  m_bSendFile = false;   // we are receiving
  m_bDoingFileTransfer = true;           
  m_iBlocksTransferred = 0;   // no blocks yet
  // add block-size minus one to allow for final partial block
  m_iFileBlocks = (m_iFileSize + m_iFileBlockSize - 1L) /
                  m_iFileBlockSize;

  shsInit  (&m_shsInfo);

  } // end of CChatSocket::Process_File_start

void CChatSocket::Process_File_deny				    (const CString strMessage)
  {
  m_pDoc->ChatNote (eChatFile, strMessage);
  StopFileTransfer (true);    // close file
  } // end of CChatSocket::Process_File_deny

void CChatSocket::Process_File_block_request	(const CString strMessage)
  {
  // MudMaster seems to send an extra request - just ignore it
  if (!m_bDoingFileTransfer)
    return;

  if (!m_bSendFile)
    {
    SendChatMessage (CHAT_MESSAGE, "We are supposed to be receiving a file.");
    return;
    }


  ZeroMemory (m_pFileBuffer, m_iFileBlockSize);
  UINT iBytesRead;

  try
    { 
    iBytesRead = m_pFile->Read (m_pFileBuffer, m_iFileBlockSize);
    }
  catch (CFileException * e)
    {
    e->Delete ();
    iBytesRead = 0;
    } // end of catching a file exception

  // which block we are up to
  m_iBlocksTransferred++;

  UINT expected = m_iFileBlockSize;
  if (m_iBlocksTransferred == m_iFileBlocks)
    expected = m_iFileSize % m_iFileBlockSize;

  if (expected == 0)
     expected = m_iFileBlockSize; // must be an exact multiple of block size

  if (iBytesRead != expected)
    {    
    SendChatMessage (CHAT_MESSAGE, "File transfer aborted due to read error by sender.");
    m_pDoc->ChatNote (eChatFile, 
                      TFormat ("Send of file \"%s\" aborted due to read error.",
                    (LPCTSTR) m_strOurFileName ));
    StopFileTransfer (true);    // close file
    return;
    }

  // update sumcheck
  shsUpdate (&m_shsInfo, m_pFileBuffer, iBytesRead);

  SendChatMessage (CHAT_FILE_BLOCK, CString ((char *) m_pFileBuffer, m_iFileBlockSize));
  m_iCountFileBytesOut += iBytesRead;

  if (m_iBlocksTransferred >= m_iFileBlocks)
    {
    SendChatMessage (CHAT_FILE_END, "");
    m_pDoc->ChatNote (eChatFile, TFormat ("Send of file \"%s\" complete.",
                    (LPCTSTR) m_strOurFileName ));
    shsFinal  (&m_shsInfo);
    SendChatMessage (CHAT_MESSAGE,
            TFormat ("Sumcheck from sender was: %08X %08X %08X %08X %08X", 
            m_shsInfo.digest [0],
            m_shsInfo.digest [1],
            m_shsInfo.digest [2],
            m_shsInfo.digest [3],
            m_shsInfo.digest [4]
            ));
    // tell sender sumcheck
    m_pDoc->ChatNote (eChatFile,
            TFormat ("Sumcheck we calculated:   %08X %08X %08X %08X %08X", 
            m_shsInfo.digest [0],
            m_shsInfo.digest [1],
            m_shsInfo.digest [2],
            m_shsInfo.digest [3],
            m_shsInfo.digest [4]
            ));
    StopFileTransfer (false);    // close file
    return;
    }

  } // end of CChatSocket::Process_File_block_request

void CChatSocket::Process_File_block			    (const CString strMessage)
  {
  // ignore if not doing file transfer
  if (!m_bDoingFileTransfer)
    return;

  if (m_bSendFile)
    {
    SendChatMessage (CHAT_MESSAGE, "We are supposed to be sending a file.");
    return;
    }

  // which block we are up to
  m_iBlocksTransferred++;

  UINT expected = m_iFileBlockSize;
  if (m_iBlocksTransferred == m_iFileBlocks)
    expected = m_iFileSize % m_iFileBlockSize;   // smaller final block

  if (expected == 0)
     expected = m_iFileBlockSize; // must be an exact multiple of block size

  CString strBuffer = strMessage;
  char * pBuff = strBuffer.GetBuffer (strMessage.GetLength ());

  try
    { 
    m_pFile->Write (pBuff, expected);
    }
  catch (CFileException * e)
    {
    e->Delete ();

    strBuffer.ReleaseBuffer ();

    SendChatMessage (CHAT_MESSAGE, "File transfer aborted due to write error by receiver.");
    m_pDoc->ChatNote (eChatFile, 
                      TFormat ("Receive of file \"%s\" aborted due to write error.",
                    (LPCTSTR) m_strOurFileName ));
    StopFileTransfer (true);    // close file
    return;

    } // end of catching a file exception

  // update sumcheck
  shsUpdate (&m_shsInfo, (unsigned char *) pBuff, expected);

  strBuffer.ReleaseBuffer ();
  m_iCountFileBytesIn += expected;

  // get next block - even if at end - see MudMaster
  SendChatMessage (CHAT_FILE_BLOCK_REQUEST, "");

  if (m_iBlocksTransferred >= m_iFileBlocks)
    {
    m_pDoc->ChatNote (eChatFile,TFormat ("Receive of file \"%s\" complete.",
                    (LPCTSTR) m_strOurFileName ));
    shsFinal  (&m_shsInfo);
    m_pDoc->ChatNote (eChatFile,
            TFormat ("Sumcheck as written was:  %08X %08X %08X %08X %08X", 
            m_shsInfo.digest [0],
            m_shsInfo.digest [1],
            m_shsInfo.digest [2],
            m_shsInfo.digest [3],
            m_shsInfo.digest [4]
            ));
    // notify sender
    SendChatMessage (CHAT_MESSAGE,
            TFormat ("Sumcheck as received was: %08X %08X %08X %08X %08X", 
            m_shsInfo.digest [0],
            m_shsInfo.digest [1],
            m_shsInfo.digest [2],
            m_shsInfo.digest [3],
            m_shsInfo.digest [4]
            ));
    StopFileTransfer (false);    // close file
    return;
    }

  } // end of CChatSocket::Process_File_block

void CChatSocket::Process_File_end				    (const CString strMessage)
  {
  // no transfer? ignore it
  if (!m_bDoingFileTransfer)
    return;

  // we probably shouldn't get here because we detect the end of file
  // at the moment it occurs, without waiting for the FILE_END block

  if (m_iFileBlocks != m_iBlocksTransferred)
    {
    m_pDoc->ChatNote (eChatFile, 
                      TFormat ("Transfer of file \"%s\" stopped prematurely.",
                    (LPCTSTR) m_strOurFileName ));

    StopFileTransfer (true);    // too soon - abort it
    }
  else
    StopFileTransfer (false);   // close file
 

  } // end of CChatSocket::Process_File_end

void CChatSocket::Process_File_cancel				  (const CString strMessage)
  {
  StopFileTransfer (true);    // too soon - abort it
  } // end of CChatSocket::Process_File_cancel	

void CChatSocket::Process_Ping_request				(const CString strMessage)
  {
  SendChatMessage (CHAT_PING_RESPONSE, strMessage);
  } // end of CChatSocket::Process_Ping_request

void CChatSocket::Process_Ping_response				(const CString strMessage)
  {
  if (App.m_iCounterFrequency && m_iPingStartTime.QuadPart)
    {
    LARGE_INTEGER finish;
    LONGLONG iTimeTaken;
    QueryPerformanceCounter (&finish);
    iTimeTaken = finish.QuadPart - m_iPingStartTime.QuadPart;
    m_iPingStartTime.QuadPart = 0;    // ready for next time
    CString strMessage;
    m_fLastPingTime = ((double) iTimeTaken) / 
                    ((double) App.m_iCounterFrequency);

    CString strPingTime;
    strPingTime.Format ("%9.3f seconds", m_fLastPingTime);
    m_pDoc->ChatNote (eChatPing,
                TFormat ("Ping time to %s: %s", 
                        (LPCTSTR) m_strRemoteUserName,
                        (LPCTSTR) strPingTime));
    }
  else
    m_pDoc->ChatNote (eChatPing,
                TFormat ("Ping response: %s", (LPCTSTR) strMessage));
  } // end of CChatSocket::Process_Ping_response	

void CChatSocket::Process_Peek_connections		(const CString strMessage)
  {
  CString strResult;

  for (POSITION chatpos = m_pDoc->m_ChatList.GetHeadPosition (); chatpos; )
    {
    CChatSocket * pSocket = m_pDoc->m_ChatList.GetNext (chatpos);
    if (pSocket->m_iChatStatus == eChatConnected && !pSocket->m_bPrivate)
      {
      // omit self
      if (pSocket == this)
        continue;

      if (m_iChatConnectionType == eChatZMud)
        {
        // different format here
        strResult += pSocket->m_strAllegedAddress;
        strResult += ",";
        strResult += CFormat ("%d", pSocket->m_iAllegedPort);
        strResult += ",";
        }
      else
        {
        strResult += pSocket->m_strAllegedAddress;
        strResult += "~";
        strResult += CFormat ("%d", pSocket->m_iAllegedPort);
        strResult += "~";
        strResult += pSocket->m_strRemoteUserName;
        strResult += "~";
        }

      }   // end of this one wanted
    }   // end of doing all

  SendChatMessage (CHAT_PEEK_LIST, strResult);
  m_pDoc->ChatNote (eChatInformation,
                TFormat ("%s is peeking at your connections", 
                    (LPCTSTR) m_strRemoteUserName));

  } // end of CChatSocket::Process_Peek_connections

void CChatSocket::Process_Peek_list			    	(const CString strMessage)
  {
  CStringList strList;
  int iCount;

  if (m_iChatConnectionType == eChatZMud)
    {
    StringToList (strMessage, ",", strList);
    iCount = strList.GetCount () / 2;   // number of connections

    m_pDoc->ChatNote (eChatPeekList,
                TFormat ("Peek found %i connection%s to %s", 
                      PLURAL (iCount),
                      (LPCTSTR) m_strRemoteUserName));

    if (iCount)
      {
      m_pDoc->ChatNote (eChatPeekList,
                  "Address                    Port");
      m_pDoc->ChatNote (eChatPeekList,
                  "=========================  =====");
      }
  
    for (int i = 0; i < iCount; i++)
      {
      CString strIP,
              strPort;

      strIP = strList.RemoveHead ();
      strPort = strList.RemoveHead ();

      m_pDoc->ChatNote (eChatPeekList,
                    CFormat ("%-26s %-5s", 
                        (LPCTSTR) strIP,
                        (LPCTSTR) strPort
                        ));
      }   // end of doing each connection

    }  // end of zChat
  else
    {
    StringToList (strMessage, "~", strList);
    iCount = strList.GetCount () / 3;   // number of connections

    m_pDoc->ChatNote (eChatPeekList,
                TFormat ("Peek found %i connection%s to %s", 
                      PLURAL (iCount),
                      (LPCTSTR) m_strRemoteUserName));

    if (iCount)
      {
      m_pDoc->ChatNote (eChatPeekList,
                  "Name                    Address                    Port");
      m_pDoc->ChatNote (eChatPeekList,
                  "======================  =========================  =====");
      }
  
    for (int i = 0; i < iCount; i++)
      {
      CString strIP,
              strPort,
              strName;

      strIP = strList.RemoveHead ();
      strPort = strList.RemoveHead ();
      strName = strList.RemoveHead ();

      m_pDoc->ChatNote (eChatPeekList,
                    CFormat ("%-23s %-26s %-5s", 
                        (LPCTSTR) strName,
                        (LPCTSTR) strIP,
                        (LPCTSTR) strPort
                        ));


      }   // end of doing each connection

    }  // end of MudMaster


  } // end of CChatSocket::Process_Peek_list	

void CChatSocket::Process_Snoop					      (const CString strMessage)
  {
  // if he is snooping us, then stop!
  if (m_bHeIsSnooping)
    {
    SendChatMessage (CHAT_MESSAGE, 
      TFormat ("\nYou are no longer snooping %s.\n",
                m_pDoc->m_strOurChatName));
    m_pDoc->ChatNote (eChatInformation,
                TFormat ("%s has stopped snooping you.",
                              (LPCTSTR) m_strRemoteUserName));
    m_bHeIsSnooping = false;
    return;    // all done
    }

  if (m_bCanSnoop)
    {   
    if (!m_pDoc->m_bAutoAllowSnooping &&  // don't bother if they don't mind
          ::UMessageBox (
          TFormat ("%s wishes to start snooping you.\n\nPermit it?",
                   (LPCTSTR) m_strRemoteUserName),
        MB_YESNO) != IDYES)
      {
      // tell them our rejection
      SendChatMessage (CHAT_MESSAGE, 
        TFormat ("\n%s does not want you to snoop just now.\n",
                  (LPCTSTR) m_pDoc->m_strOurChatName));
      }   // end of snoop not allowed right now
    else
      {
      // tell them our acceptance
      SendChatMessage (CHAT_MESSAGE, 
        TFormat ("\nYou are now snooping %s.\n",
                  (LPCTSTR) m_pDoc->m_strOurChatName));
      m_pDoc->ChatNote (eChatInformation,
                  TFormat ("%s is now snooping you.",
                                (LPCTSTR) m_strRemoteUserName));
      m_bHeIsSnooping = true;
      }

    }   // end of allowed to snoop
  else
    {   // not allowed to snoop
    SendChatMessage (CHAT_MESSAGE, 
        TFormat ("\n%s has not given you permission to snoop.\n",
                  (LPCTSTR) m_pDoc->m_strOurChatName));
    m_pDoc->ChatNote (eChatInformation,
                TFormat ("%s attempted to snoop you.",
                              (LPCTSTR) m_strRemoteUserName));
    }   // end of not allowed to snoop

  } // end of CChatSocket::Process_Snoop

void CChatSocket::Process_Snoop_data				  (const CString strMessage)
  {
  // save old colours - because we switch to the chat colour below
  bool bOldNotesInRGB = m_pDoc->m_bNotesInRGB;
  COLORREF iOldNoteColourFore = m_pDoc->m_iNoteColourFore;
  COLORREF iOldNoteColourBack = m_pDoc->m_iNoteColourBack;

  m_pDoc->ColourTell ("springgreen", "black", ">");

  // undocumented feature - first 4 digits are foreground/background colours
  if (strMessage.GetLength () >= 4 && IsNumber (strMessage.Left (4)))
    {
    int iFore = atoi (strMessage.Left (2));
    int iBack = atoi (strMessage.Mid (2, 2));
    // foreground colour should be 0 to 15 (0 to 7 is normal, 8 to 15 is bold)
    if (iFore >= 0 && iFore <= 7)
      m_pDoc->SetNoteColourFore (m_pDoc->m_normalcolour [iFore]);
    else
      if (iFore >= 8 && iFore <= 15)
        m_pDoc->SetNoteColourFore (m_pDoc->m_boldcolour [iFore - 8]);
    // background colour should be 0 to 7
    if (iBack >= 0 && iBack <= 7)
      m_pDoc->SetNoteColourBack (m_pDoc->m_normalcolour [iBack]);
    m_pDoc->AnsiNote (strMessage.Mid (4));
    }
  else
    // must be as documented - whole message is the text
    m_pDoc->AnsiNote (strMessage);

  // put the colours back
  if (bOldNotesInRGB)
    {
    m_pDoc->m_iNoteColourFore = iOldNoteColourFore;
    m_pDoc->m_iNoteColourBack = iOldNoteColourBack;
    }
  else  
    m_pDoc->m_bNotesInRGB = false;

  m_bYouAreSnooping = true;   // we seem to be snooping them :)

  } // end of CChatSocket::Process_Snoop_data


void CChatSocket::Process_Send_command				  (const CString strMessage)
  {
  if (m_bCanSendCommands)
    {   
      // tell them our acceptance
      SendChatMessage (CHAT_MESSAGE, 
        TFormat ("\nYou command %s to '%s'.\n",
                  (LPCTSTR) m_pDoc->m_strOurChatName ,
                  (LPCTSTR) strMessage));
      m_pDoc->ChatNote (eChatCommand,
                  TFormat ("%s commands you to '%s'.",
                                (LPCTSTR) m_strRemoteUserName,
                                (LPCTSTR) strMessage));
    m_pDoc->m_iExecutionDepth = 0;
    m_pDoc->Execute (strMessage);
    }   // end of allowed to send commands
  else
    {   // not allowed to send commands
    SendChatMessage (CHAT_MESSAGE, 
        TFormat ("\n%s has not given you permission to send commands.\n",
                  m_pDoc->m_strOurChatName));
    m_pDoc->ChatNote (eChatCommand,
                TFormat ("%s attempted to send you a command.",
                              (LPCTSTR) m_strRemoteUserName));
    }   // end of not allowed to end commands

  } // end of CChatSocket::Process_Send_command


void CChatSocket::Process_Icon               (const CString strMessage)
  {
  // ignore it
  } // end of CChatSocket::Process_Icon 

void CChatSocket::Process_Status             (const CString strMessage)
  {
  if (!strMessage.IsEmpty ())
    m_zChatStatus = (unsigned char) strMessage [0];
  } // end of CChatSocket::Process_Status

void CChatSocket::Process_EmailAddress			 (const CString strMessage)
  {
  m_strEmailAddress = strMessage;
  } // end of CChatSocket::Process_EmailAddress

void CChatSocket::Process_Stamp				       (const CString strMessage)
  {

  if (m_iChatConnectionType != eChatZMud)
    return;

  if (strMessage.GetLength () != 4)
    return;   // should be 4-byte stamp

  CString strFixedMessage = strMessage;
  long iStamp = GetStamp (strFixedMessage);

  if (iStamp == m_zChatStamp)    // if same as ours, make a new one
    {
    // make new stamp
    SHS_INFO shsInfo;
    MakeRandomNumber (m_pDoc, shsInfo);

    m_zChatStamp = shsInfo.digest [0];

    // broadcast new stamp so they know we have changed it
    for (POSITION chatpos = m_pDoc->m_ChatList.GetHeadPosition (); chatpos; )
      {
      CChatSocket * pSocket = m_pDoc->m_ChatList.GetNext (chatpos);
      if (pSocket->m_iChatStatus == eChatConnected)
        {
        if (m_iChatConnectionType != eChatZMud)
          continue;

        pSocket->SendChatMessage (CHAT_STAMP, CFormat ("%ld", m_zChatStamp));  // send it
        }   // end of chat session not being deleted
      } // end of all chat sessions

    }   // end of new stamp wanted

  } // end of CChatSocket::Process_Stamp	

void CChatSocket::Process_RequestPGPkey			 (const CString strMessage)
  {
  SendChatMessage (CHAT_MESSAGE, "Cannot send PGP key.");
  } // end of CChatSocket::Process_RequestPGPkey

void CChatSocket::Process_PGPkey				     (const CString strMessage)  
  {
  m_strPGPkey = strMessage;
  } // end of CChatSocket::Process_PGPkey


long CChatSocket::GetStamp (CString & strMessage)    // extracts message stamp
  {
  if (m_iChatConnectionType != eChatZMud)
    return 0;

  if (strMessage.GetLength () < 4)
    return 0;   // should be 4-byte stamp

  long iStamp = (int) (unsigned char) strMessage [0] |
              (((int) (unsigned char) strMessage [1]) << 8) |
              (((int) (unsigned char) strMessage [2]) << 16) |
              (((int) (unsigned char) strMessage [3]) << 24);

  strMessage = strMessage.Mid (4);  // get rid of stamp part

  return iStamp;

  }  // end of CChatSocket::GetStamp


CString CChatSocket::MakeStamp (const long iStamp)
  {
  if (m_iChatConnectionType != eChatZMud)
    return "";

  CString strStamp;

  unsigned char * p = (unsigned char *) strStamp.GetBuffer (4);

  p [0] = (unsigned char) iStamp & 0xFF;
  p [1] = (unsigned char) (iStamp >> 8) & 0xFF;
  p [2] = (unsigned char) (iStamp >> 16) & 0xFF;
  p [3] = (unsigned char) (iStamp >> 24) & 0xFF;

  strStamp.ReleaseBuffer (4);

  return strStamp;

}  // end of CChatSocket::MakeStamp
