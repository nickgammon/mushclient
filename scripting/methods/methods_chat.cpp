//  methods_chat.cpp

// Stuff for the chat system

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\..\MUSHview.h"
#include "..\..\mainfrm.h"
#include "..\errors.h"

// Implements:

//    ChatAcceptCalls
//    ChatCall
//    ChatCallzChat
//    ChatDisconnect
//    ChatDisconnectAll
//    ChatEverybody
//    ChatGetID
//    ChatGroup
//    ChatID
//    ChatMessage
//    ChatNameChange
//    ChatNote
//    ChatPasteEverybody
//    ChatPasteText
//    ChatPeekConnections
//    ChatPersonal
//    ChatPing
//    ChatRequestConnections
//    ChatSendFile
//    ChatStopAcceptingCalls
//    ChatStopFileTransfer
//    GetChatInfo
//    GetChatList
//    GetChatOption
//    SetChatOption


#define CHATO(arg) offsetof (CChatSocket, arg), sizeof (((CChatSocket *)NULL)->arg)

//-----config name ----------------  default ---variable name ---- min, max, flags ---
tConfigurationNumericOption ChatOptionsTable [] = {

// boolean

{"can_send_commands",    false, CHATO(m_bCanSendCommands)},               
{"can_send_files",       false, CHATO(m_bCanSendFiles)},               
{"can_snoop",            false, CHATO(m_bCanSnoop)},               
{"ignore",               false, CHATO(m_bIgnore)},               
{"served",               false, CHATO(m_bIncoming)},               
{"private",              false, CHATO(m_bPrivate)},

// numbers 
               
{"user",                 0, CHATO(m_iUserOption), LONG_MIN, LONG_MAX},               

{NULL}   // end of table marker            

  };  // end of ChatOptionsTable 

#define CHATA(arg) offsetof (CChatSocket, arg)
                    
// first  optional argument: multiline flag
// second optional argument: preserve spaces flag (defaults to true for multiline)

tConfigurationAlphaOption ChatAlphaOptionsTable  [] =
{
{"server",      "",         CHATA(m_strServerName), OPT_CANNOT_WRITE},
{"username",    "",         CHATA(m_strRemoteUserName), OPT_CANNOT_WRITE},
{"group",       "",         CHATA(m_strGroup)},
{"version",     "",         CHATA(m_strRemoteVersion), OPT_CANNOT_WRITE},
{"address",     "",         CHATA(m_strAllegedAddress), OPT_CANNOT_WRITE},

{NULL}  // end of table marker
};    // end of ChatAlphaOptionsTable




long CMUSHclientDoc::ChatCallGeneral (LPCTSTR Server, long Port, const bool zChat)
  {
  // if not chat name, try our character name
  if (m_strOurChatName.IsEmpty () && !m_name.IsEmpty ())
    {
    m_strOurChatName = m_name;
    SetModifiedFlag (TRUE);   // document has changed
    }
  
  // still empty? use a default
  if (m_strOurChatName.IsEmpty ())
    m_strOurChatName = DEFAULT_CHAT_NAME;

  // default to port 4050 if none supplied
  if (Port == 0)
    Port = DEFAULT_CHAT_PORT;

CChatSocket * pSocket = new CChatSocket (this);

  if (zChat)
    {
    pSocket->m_iChatConnectionType = eChatZMud;
    pSocket->m_iFileBlockSize = 1024;
    }
  else
    pSocket->m_iChatConnectionType = eChatMudMaster;

	if (!pSocket->Create(0,
                         SOCK_STREAM,
                         FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE,
                         NULL))
	  {
		delete pSocket;
		return eCannotCreateChatSocket;
	  }     // end of can't create socket

  pSocket->AsyncSelect ();

  pSocket->m_strServerName = Server;
  pSocket->m_ServerAddr.sin_port = htons((u_short)Port); 

	pSocket->m_ServerAddr.sin_family = AF_INET;
	pSocket->m_ServerAddr.sin_addr.s_addr = inet_addr(Server);

// if address is INADDR_NONE then address was a name, not a number

	if (pSocket->m_ServerAddr.sin_addr.s_addr == INADDR_NONE)
	 {
    pSocket->m_pGetHostStruct = new char [MAXGETHOSTSTRUCT];

    if (!pSocket->m_pGetHostStruct)
      {
  		delete pSocket;
      return eCannotLookupDomainName;
      }

    if (Frame.GetSafeHwnd ())   // forget it if we don't have a window yet
      pSocket->m_hNameLookup = WSAAsyncGetHostByName (Frame.GetSafeHwnd (),
                                                     WM_USER_HOST_NAME_RESOLVED,
                                                     Server,
                                                     pSocket->m_pGetHostStruct,
                                                     MAXGETHOSTSTRUCT);

   if (!pSocket->m_hNameLookup)
     {
		  delete pSocket;
      return eCannotLookupDomainName;
     }

    m_ChatList.AddTail (pSocket);
  	return eOK;

	 }   // end of address not being an IP address


// the name was a dotted IP address - just make the connection

  m_ChatList.AddTail (pSocket);

  pSocket->MakeCall ();
  return eOK;   // OK for now, eh?

  }   // end of CMUSHclientDoc::ChatCallGeneral

long CMUSHclientDoc::ChatCall(LPCTSTR Server, long Port) 
{
  return ChatCallGeneral (Server, Port, false);  // not zChat
} // end of CMUSHclientDoc::ChatCall

long CMUSHclientDoc::ChatCallzChat(LPCTSTR Server, long Port) 
{
  return ChatCallGeneral (Server, Port, true);  // zChat
}  // end of CMUSHclientDoc::ChatCallzChat


long CMUSHclientDoc::ChatNameChange(LPCTSTR NewName) 
{
CString strNewName = NewName;

  strNewName.TrimLeft ();
  strNewName.TrimRight ();

  if (strNewName.IsEmpty ())
    return eBadParameter;

  if (m_strOurChatName.IsEmpty ())
    m_strOurChatName = "<no name>";

  ChatNote (eChatNameChange,
              TFormat ("Your chat name changed from %s to %s", 
                     (LPCTSTR) m_strOurChatName, 
                     (LPCTSTR) strNewName));

  if (m_strOurChatName != strNewName)
    SetModifiedFlag (TRUE);   // document has changed

  m_strOurChatName = strNewName;

  SendChatMessageToAll (CHAT_NAME_CHANGE, strNewName, false, false, false, 0, "", 0);
	return eOK;
}    // end of CMUSHclientDoc::ChatNameChange

long CMUSHclientDoc::ChatEverybody(LPCTSTR Message, BOOL Emote) 
{
CString strMessage;

  if (Emote)
    strMessage = CFormat ("\n%s%s%s %s%s\n",
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                              (LPCTSTR) m_strOurChatName,
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED));   
    else
    strMessage = TFormat ("\n%s chats to everybody, '%s%s%s%s'\n",
                              (LPCTSTR) m_strOurChatName,
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED));   

  if (SendChatMessageToAll (CHAT_TEXT_EVERYBODY,
                                  strMessage,
                                  true,    // unless ignoring them
                                  false,
                                  false,
                                  0,    // don't ignore us
                                  "",   // no group
                                  0))   // use our stamp
  {
  if (Emote)
    ChatNote (eChatOutgoingEverybody,
              TFormat ("You emote to everybody: %s%s%s %s%s", 
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                              (LPCTSTR) m_strOurChatName,
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED)));   
  else
    ChatNote (eChatOutgoingEverybody,
              TFormat ("You chat to everybody, '%s%s%s%s'", 
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED)));   
  return eOK;
  }
else
  {
  ChatNote (eChatOutgoingEverybody, 
            Translate ("No (relevant) chat connections."));
  return eNoChatConnections;
  }
}   // end of  CMUSHclientDoc::ChatEverybody


long CMUSHclientDoc::ChatID(long ID, LPCTSTR Message, BOOL Emote) 
{

  // quiet failure on ID zero - probably caused by a ChatGetID lookup failure
  if (ID == 0)
    return eChatIDNotFound;

CString strMessage;

  CChatSocket * pSocket = GetChatSocket (ID); // find them

  if (!pSocket)
    {
    ChatNote (eChatOutgoingPersonal, TFormat ("Chat ID %i is not connected.", ID));
	  return eChatIDNotFound;
    }

  if (Emote)  
   strMessage = TFormat ("\nTo you, %s%s%s %s%s\n",
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                              (LPCTSTR) m_strOurChatName,
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED)   
                               );
  else
    strMessage = TFormat ("\n%s chats to you, '%s%s%s%s'\n",
                              (LPCTSTR) m_strOurChatName,
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED)   
                               );

  pSocket->m_iCountOutgoingPersonal++;
  pSocket->SendChatMessage (CHAT_TEXT_PERSONAL, strMessage);

  if (Emote)  
    ChatNote (eChatOutgoingPersonal,
              TFormat ("You emote to %s: %s%s%s %s%s", 
                  (LPCTSTR) pSocket->m_strRemoteUserName, 
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                              (LPCTSTR) m_strOurChatName,
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED)));   
  else
    ChatNote (eChatOutgoingPersonal,
              TFormat ("You chat to %s, '%s%s%s%s'", 
                        (LPCTSTR) pSocket->m_strRemoteUserName, 
                        (LPCTSTR) AnsiCode (ANSI_BOLD),  
                        (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                         Message,
                        (LPCTSTR) AnsiCode (ANSI_TEXT_RED)));   
  return eOK;

} // end of CMUSHclientDoc::ChatID


long CMUSHclientDoc::ChatPersonal(LPCTSTR Who, LPCTSTR Message, BOOL Emote) 
{
int iCount = 0;

  if (strlen (Who) == 0)
    return eBadParameter;

  for (POSITION chatpos = m_ChatList.GetHeadPosition (); chatpos; )
    {
    CChatSocket * pSocket = m_ChatList.GetNext (chatpos);
    if (pSocket->m_iChatStatus == eChatConnected)
      {
      if (pSocket->m_strRemoteUserName.CompareNoCase (Who) == 0)
        {
        if (ChatID (pSocket->m_iChatID, Message, Emote) == eOK)
          iCount++; // count matches
        }
      }   // end of chat session not being deleted
    } // end of all chat sessions

  if (iCount == 0)
    {
    ChatNote (eChatOutgoingPersonal, TFormat ("%s is not connected.", Who));
	  return eChatPersonNotFound;
    }
  else
    if (iCount > 1)
      ChatNote (eChatOutgoingPersonal, TFormat ("%i matches.", iCount));

  return eOK;
}  // end of CMUSHclientDoc::ChatPersonal

long CMUSHclientDoc::ChatGroup(LPCTSTR Group, LPCTSTR Message, BOOL Emote) 
{
  if (strlen (Group) == 0)
    return eBadParameter;

  CString strMessage;
  
  if (Emote)
    strMessage = TFormat ("%-15s\nTo the group, %s%s%s %s%s\n",
                                Group,
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                              (LPCTSTR) m_strOurChatName,
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED));   
  else
    strMessage = TFormat ("%-15s\n%s chats to the group, '%s%s%s%s'\n",
                                Group,
                                (LPCTSTR) m_strOurChatName,
                                (LPCTSTR) AnsiCode (ANSI_BOLD),  
                                (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                                 Message,
                                (LPCTSTR) AnsiCode (ANSI_TEXT_RED));   

if (SendChatMessageToAll (CHAT_TEXT_GROUP,
                                  strMessage,
                                  true,      // unless ignoring them
                                  false,     // to all connections
                                  false,
                                  0,
                                  Group,
                                  0))    
  {
  if (Emote)
    ChatNote (eChatOutgoingGroup,
              TFormat ("You emote to the group %s: %s%s%s %s%s", 
                              Group, 
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                              (LPCTSTR) m_strOurChatName,
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED)));   
  else
    ChatNote (eChatOutgoingGroup,
              TFormat ("You chat to the group %s, '%s%s%s%s'", 
                               Group, 
                              (LPCTSTR) AnsiCode (ANSI_BOLD),  
                              (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                               Message,
                              (LPCTSTR) AnsiCode (ANSI_TEXT_RED)));   

  return eOK;
  }
else
  {
  ChatNote (eChatOutgoingGroup,
              TFormat ("No chat connections in the group %s.", Group));
  return eNoChatConnections;
  }

}   // end of CMUSHclientDoc::ChatGroup

// Does a note in the chat colour - including processing ANSI codes

void CMUSHclientDoc::ChatNote(short NoteType, LPCTSTR Message) 
{

  CString strMessage = Message;

  // strip leading \n
  if (strMessage.GetLength () >= 1 && strMessage.Left (1) == '\n')
    strMessage = strMessage.Mid (1);

  // strip trailing \n
  if (strMessage.GetLength () >= 1 && strMessage.Right (1) == '\n')
    strMessage = strMessage.Left (strMessage.GetLength () - 1);

  // check message is not too long for what we accept - bytes

  if (m_iMaxChatBytesPerMessage > 0)
    {
    if (strMessage.GetLength () > m_iMaxChatBytesPerMessage)
      {
      strMessage = strMessage.Left (m_iMaxChatBytesPerMessage);
      strMessage += TFormat ("\n[Chat message truncated, exceeds %i bytes]", 
            m_iMaxChatBytesPerMessage);
      }
    }   // end of byte count check wanted

  // check message is not too long for what we accept - lines
  
  if (m_iMaxChatLinesPerMessage > 0)
    {
    int iLines = 0;
    char * p;

    for (p = strMessage.GetBuffer (strMessage.GetLength ());
         *p;
         p++)
      {
      if (*p == '\n')
        if (++iLines >= m_iMaxChatLinesPerMessage)
          {
          *p = 0;
          break;    // stop scanning for newlines
          }
      } // end of buffer scan
    strMessage.ReleaseBuffer (-1);
    if (iLines >= m_iMaxChatLinesPerMessage && p [1])
      strMessage += TFormat ("\n[Chat message truncated, exceeds %i lines]", 
            m_iMaxChatLinesPerMessage);
    }   // end of line count check wanted

  if (!SendToAllPluginCallbacks (ON_PLUGIN_CHAT_DISPLAY, 
                                NoteType,     // message number
                                string (strMessage),    // message text
                                false,
                                true))  // stop on false response
    return;   // false means plugin handled it 


// save old colours - because we switch to the chat colour below
bool bOldNotesInRGB = m_bNotesInRGB;
COLORREF iOldNoteColourFore = m_iNoteColourFore;
COLORREF iOldNoteColourBack = m_iNoteColourBack;

  if (m_cChatForegroundColour == m_cChatBackgroundColour) 
    NoteColourRGB (RGB (255, 0, 0), RGB (0, 0, 0));
  else
    NoteColourRGB (m_cChatForegroundColour, m_cChatBackgroundColour);

  // get rid of unwanted incoming colour codes
  if (m_bIgnoreChatColours)
    strMessage = ::StripAnsi (strMessage);
  
  AnsiNote (m_strChatMessagePrefix + strMessage);

  // put the colours back
  if (bOldNotesInRGB)
    {
    m_iNoteColourFore = iOldNoteColourFore;
    m_iNoteColourBack = iOldNoteColourBack;
    }
  else  
    m_bNotesInRGB = false;

}  // end of  CMUSHclientDoc::ChatNote

long CMUSHclientDoc::ChatAcceptCalls(short Port) 
{

  if (m_pChatListenSocket)
    return eChatAlreadyListening;

  if (Port && m_IncomingChatPort != Port)
    {
    SetModifiedFlag (TRUE);   // document has changed
    m_IncomingChatPort = Port;
    }

  if (m_IncomingChatPort == 0)
    m_IncomingChatPort = DEFAULT_CHAT_PORT;

// set flag for next session if necessary
 if (!m_bAcceptIncomingChatConnections)
   {
   SetModifiedFlag (TRUE);   // document has changed
   m_bAcceptIncomingChatConnections = true;
   }

 Frame.SetStatusMessageNow (TFormat ("Accepting chat calls on port %d",
                              m_IncomingChatPort));

 m_pChatListenSocket = new CChatListenSocket (this);

	if (!m_pChatListenSocket->Create (m_IncomingChatPort,
                         SOCK_STREAM,
                         FD_ACCEPT | FD_CLOSE ,
                         NULL))
	  {
    int nError = GetLastError ();
		delete m_pChatListenSocket;
    m_pChatListenSocket = NULL;
    ChatNote (eChatConnection,
              TFormat (
              "Cannot accept calls on port %i, code = %i (%s)", 
                    m_IncomingChatPort, 
                    nError,
                    GetSocketError (nError)));
		return eCannotCreateChatSocket;
	  }     // end of can't create socket

  m_pChatListenSocket->Listen ();

  ChatNote (eChatConnection,
            TFormat (
    "Listening for chat connections on port %d", m_IncomingChatPort));

	return eOK;
}    // end of CMUSHclientDoc::ChatAcceptCalls

void CMUSHclientDoc::ChatStopAcceptingCalls() 
{
if (m_pChatListenSocket)
  {
  ShutDownSocket (*m_pChatListenSocket);

  ChatNote (eChatConnection, "Stopped accepting chat connections.");
  delete m_pChatListenSocket;
  m_pChatListenSocket = NULL;

  // remember they don't want them
  SetModifiedFlag (TRUE);   // document has changed
  m_bAcceptIncomingChatConnections = false;
  }

}   // end of CMUSHclientDoc::ChatStopAcceptingCalls

// get list of chat IDs
VARIANT CMUSHclientDoc::GetChatList() 
{
  COleSafeArray sa;   // for list

  long iCount = 0;
  POSITION pos;

  // count ones not marked for deletion
  for (pos = m_ChatList.GetHeadPosition (); pos; )
    {
    CChatSocket * pSocket = m_ChatList.GetNext (pos);
    if (pSocket->m_iChatStatus == eChatConnected)
      iCount++;
    } // end of all chat sessions

  if (iCount) // cannot create empty array dimension
    {
    sa.CreateOneDim (VT_VARIANT, iCount);
  
    // put the chat sessions into the array
      for (iCount = 0, pos = m_ChatList.GetHeadPosition (); pos; )
        {
        CChatSocket * pSocket = m_ChatList.GetNext (pos);

        if (pSocket->m_iChatStatus == eChatConnected)
          {
          // the array must be a bloody array of variants, or VBscript kicks up
          COleVariant v (pSocket->m_iChatID);
          sa.PutElement (&iCount, &v);
          iCount++;
          }
        }      // end of looping through each chat session
    } // end of having at least one

	return sa.Detach ();
}  // end of CMUSHclientDoc::GetChatList


// get info about one chat ID
VARIANT CMUSHclientDoc::GetChatInfo(long ChatID, short InfoType) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_NULL;

  CChatSocket * pSocket = GetChatSocket (ChatID);

  if (pSocket)
    switch (InfoType)
      {
       case   1: SetUpVariantString (vaResult, pSocket->m_strServerName); break;
       case   2: SetUpVariantString (vaResult, pSocket->m_strRemoteUserName); break;
       case   3: SetUpVariantString (vaResult, pSocket->m_strGroup); break;
       case   4: SetUpVariantString (vaResult, pSocket->m_strRemoteVersion); break;
       case   5: SetUpVariantString (vaResult, pSocket->m_strAllegedAddress); break;
       case   6: SetUpVariantString (vaResult, inet_ntoa (pSocket->m_ServerAddr.sin_addr)); break;
       case   7: SetUpVariantLong   (vaResult, ntohs (pSocket->m_ServerAddr.sin_port)); break;
       case   8: SetUpVariantLong   (vaResult, pSocket->m_iAllegedPort); break;
       case   9: SetUpVariantLong   (vaResult, pSocket->m_iChatStatus); break;
       case  10: SetUpVariantLong   (vaResult, pSocket->m_iChatConnectionType); break;
       case  11: SetUpVariantLong   (vaResult, pSocket->m_iChatID); break;
       case  12: SetUpVariantBool   (vaResult, pSocket->m_bIncoming); break;
       case  13: SetUpVariantBool   (vaResult, pSocket->m_bCanSnoop); break;
       case  14: SetUpVariantBool   (vaResult, pSocket->m_bYouAreSnooping); break;
       case  15: SetUpVariantBool   (vaResult, pSocket->m_bHeIsSnooping); break;
       case  16: SetUpVariantBool   (vaResult, pSocket->m_bCanSendCommands); break;
       case  17: SetUpVariantBool   (vaResult, pSocket->m_bPrivate); break;
       case  18: SetUpVariantBool   (vaResult, pSocket->m_bCanSendFiles); break;
       case  19: SetUpVariantBool   (vaResult, pSocket->m_bIgnore); break;
       case  20: SetUpVariantDouble (vaResult, pSocket->m_fLastPingTime); break;
         
       case  21: 
         if (pSocket->m_tWhenStarted.GetTime ())     // only if non-zero, otherwise return empty
           SetUpVariantDate   (vaResult, COleDateTime (pSocket->m_tWhenStarted.GetTime ())); 
         break;
       case  22: 
         if (pSocket->m_tLastIncoming.GetTime ())     // only if non-zero, otherwise return empty
           SetUpVariantDate   (vaResult, COleDateTime (pSocket->m_tLastIncoming.GetTime ())); 
         break;
       case  23: 
         if (pSocket->m_tLastOutgoing.GetTime ())     // only if non-zero, otherwise return empty
           SetUpVariantDate   (vaResult, COleDateTime (pSocket->m_tLastOutgoing.GetTime ())); 
         break;

       case  24: 
         if (pSocket->m_tStartedFileTransfer.GetTime ())     // only if non-zero, otherwise return empty
           SetUpVariantDate   (vaResult, COleDateTime (pSocket->m_tStartedFileTransfer.GetTime ())); 
         break;

       case  25: SetUpVariantBool   (vaResult, pSocket->m_bDoingFileTransfer); break;
       case  26: SetUpVariantBool   (vaResult, pSocket->m_bSendFile); break;
       case  27: SetUpVariantString (vaResult, pSocket->m_strSenderFileName); break;
       case  28: SetUpVariantString (vaResult, pSocket->m_strOurFileName); break;
       case  29: SetUpVariantLong   (vaResult, pSocket->m_iFileSize); break;
       case  30: SetUpVariantLong   (vaResult, pSocket->m_iFileBlocks); break;
       case  31: SetUpVariantLong   (vaResult, pSocket->m_iBlocksTransferred); break;
       case  32: SetUpVariantLong   (vaResult, pSocket->m_iFileBlockSize); break;
       case  33: SetUpVariantLong   (vaResult, pSocket->m_iCountIncomingPersonal); break;
       case  34: SetUpVariantLong   (vaResult, pSocket->m_iCountIncomingAll); break;
       case  35: SetUpVariantLong   (vaResult, pSocket->m_iCountIncomingGroup); break;
       case  36: SetUpVariantLong   (vaResult, pSocket->m_iCountOutgoingPersonal); break;
       case  37: SetUpVariantLong   (vaResult, pSocket->m_iCountOutgoingAll); break;
       case  38: SetUpVariantLong   (vaResult, pSocket->m_iCountOutgoingGroup); break;
       case  39: SetUpVariantLong   (vaResult, pSocket->m_iCountMessages); break;
       case  40: SetUpVariantLong   (vaResult, pSocket->m_iCountFileBytesIn); break;
       case  41: SetUpVariantLong   (vaResult, pSocket->m_iCountFileBytesOut); break;
       case  42: SetUpVariantLong   (vaResult, pSocket->m_zChatStamp); break;
       case  43: SetUpVariantString (vaResult, pSocket->m_strEmailAddress); break;
       case  44: SetUpVariantString (vaResult, pSocket->m_strPGPkey); break;
       case  45: SetUpVariantShort  (vaResult, pSocket->m_zChatStatus); break;
       case  46: SetUpVariantLong   (vaResult, pSocket->m_iUserOption); break;
         
      default:
        vaResult.vt = VT_EMPTY;
        break;

      } // end of switch

  return vaResult;
}    // end of CMUSHclientDoc::GetChatInfo


long CMUSHclientDoc::ChatPing(long ID) 
{
  CChatSocket * pSocket = GetChatSocket (ID);
  if (!pSocket)
    return eChatIDNotFound;
  
  if (App.m_iCounterFrequency)
    QueryPerformanceCounter (&pSocket->m_iPingStartTime);

  CString strMessage = CTime::GetCurrentTime().Format (TranslateTime ("%A, %B %d, %Y, %#I:%M %p"));

  pSocket->SendChatMessage (CHAT_PING_REQUEST, strMessage);

	return eOK;
}   // end of CMUSHclientDoc::ChatPing


long CMUSHclientDoc::ChatPeekConnections(long ID) 
{
  CChatSocket * pSocket = GetChatSocket (ID);
  if (!pSocket)
    return eChatIDNotFound;
  
  pSocket->SendChatMessage (CHAT_PEEK_CONNECTIONS, "");

	return eOK;
}   // end of CMUSHclientDoc::ChatPeekConnections

long CMUSHclientDoc::ChatRequestConnections(long ID) 
{
  CChatSocket * pSocket = GetChatSocket (ID);
  if (!pSocket)
    return eChatIDNotFound;
  
  pSocket->SendChatMessage (CHAT_REQUEST_CONNECTIONS, "");

	return eOK;
}   // end of CMUSHclientDoc::ChatRequestConnections


long CMUSHclientDoc::ChatDisconnect(long ID) 
{

  CChatSocket * pSocket = GetChatSocket (ID);
  if (!pSocket)
    return eChatIDNotFound;

  ChatNote (eChatConnection,
            TFormat ("Connection to %s dropped.",
            (LPCTSTR) pSocket->m_strRemoteUserName));

  pSocket->OnClose (0);    // close connection

	return eOK;
}    // end of CMUSHclientDoc::ChatDisconnect

long CMUSHclientDoc::ChatDisconnectAll() 
{
  int iCount = 0;
  for (POSITION pos = m_ChatList.GetHeadPosition (); pos; )
    {
    CChatSocket * pSocket = m_ChatList.GetNext (pos);
    if (!pSocket->m_bDeleteMe)
      {
      iCount++;
      pSocket->OnClose (0);    // close connection
      }
    } // end of all chat sessions
  ChatNote (eChatConnection,
            TFormat ("%i connection%s closed.", 
            PLURAL (iCount)));

	return iCount;
  }   // end of CMUSHclientDoc::ChatDisconnectAll

// generic chat message sender, for plugins to fool around with
long CMUSHclientDoc::ChatMessage(long ID, short Message, LPCTSTR Text) 
{
  CChatSocket * pSocket = GetChatSocket (ID);
  if (!pSocket)
    return eChatIDNotFound;
  
  pSocket->SendChatMessage (Message, Text);

	return eOK;
}   // end of CMUSHclientDoc::ChatMessage



VARIANT CMUSHclientDoc::GetChatOption(long ID, LPCTSTR OptionName) 
{
CChatSocket * chat_item;

	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;

  chat_item = GetChatSocket (ID);
  if (!chat_item)
	  return vaResult;

int iItem;
int iResult = FindBaseOption (OptionName, ChatOptionsTable, iItem);

  
  if (iResult == eOK)
    {

    // numeric option

    if (m_CurrentPlugin &&
        (ChatOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_READ))
    	return vaResult;  // not available to plugin

    long Value =  GetBaseOptionItem (iItem, 
                              ChatOptionsTable, 
                              NUMITEMS (ChatOptionsTable),
                              (char *) chat_item);  

    SetUpVariantLong (vaResult, Value);
    }  // end of found numeric option
  else
    { // not numeric option, try alpha
    int iResult = FindBaseAlphaOption (OptionName, ChatAlphaOptionsTable, iItem);
    if (iResult == eOK)
      {

      // alpha option

      if (m_CurrentPlugin &&
          (ChatAlphaOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_READ))
    	  return vaResult;  // not available to plugin

      CString strValue =  GetBaseAlphaOptionItem (iItem, 
                                                 ChatAlphaOptionsTable,
                                                 NUMITEMS (ChatAlphaOptionsTable),
                                                 (char *) chat_item);

      SetUpVariantString (vaResult, strValue);
      }  // end of found
    }

	return vaResult;
}  // end of CMUSHclientDoc::GetChatOption

long CMUSHclientDoc::SetChatOption(long ID, LPCTSTR OptionName, LPCTSTR Value) 
{
CString strValue = Value;
CChatSocket * chat_item;

  chat_item = GetChatSocket (ID);
  if (!chat_item)
    return eChatIDNotFound;

CString strOptionName = OptionName;

  strOptionName.MakeLower();
  strOptionName.TrimLeft ();
  strOptionName.TrimRight ();

int iItem;
int iResult = FindBaseOption (strOptionName, ChatOptionsTable, iItem);

bool bChanged;

  if (iResult == eOK)
    {
    // this is a numeric option
    
    // for boolean options, accept "y" or "n"
    if (ChatOptionsTable [iItem].iMinimum == 0 &&
      ChatOptionsTable [iItem].iMaximum == 0)
      {
      if (strValue == "Y" || strValue == "y")
        Value = "1";
      else if (strValue == "N" || strValue == "n")
        Value = "0";
      }

    if (!IsNumber (Value, true))
       return eOptionOutOfRange;

    long iValue = atol (Value);

    if (m_CurrentPlugin &&
        (ChatOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_WRITE))
    	return ePluginCannotSetOption;  // not available to plugin

    if (ChatOptionsTable [iItem].iFlags & OPT_CANNOT_WRITE)
    	return ePluginCannotSetOption;  // not available for writing at all    

    iResult = SetBaseOptionItem (iItem,
                        ChatOptionsTable,
                        NUMITEMS (ChatOptionsTable),
                        (char *) chat_item, 
                        iValue,
                        bChanged);

    if (iResult == eOK && bChanged)
      {
      if (strOptionName == "can_send_commands")
         if (iValue)
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("You can now send %s commands", 
                (LPCTSTR) m_strOurChatName));
      else
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("You can no longer send %s commands", 
                (LPCTSTR) m_strOurChatName));
      else
      if (strOptionName == "can_send_files")
         if (iValue)
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("You can now send %s files", 
                (LPCTSTR) m_strOurChatName));
      else
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("You can no longer send %s files", 
                (LPCTSTR) m_strOurChatName));
      else
      if (strOptionName == "can_snoop")
         if (iValue)
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("You can now snoop %s", 
                (LPCTSTR) m_strOurChatName));
      else
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("You can no longer snoop %s", 
                (LPCTSTR) m_strOurChatName));
      else
      if (strOptionName == "ignore")
         if (iValue)
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("%s is ignoring you", 
                (LPCTSTR) m_strOurChatName));
      else
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("%s is no longer ignoring you", 
                (LPCTSTR) m_strOurChatName));
      else
      if (strOptionName == "private")
         if (iValue)
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("%s has marked your connection as private", 
                (LPCTSTR) m_strOurChatName));
      else
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("%s has marked your connection as public", 
                (LPCTSTR) m_strOurChatName));

      }  // end of succesfully changing the option

    return iResult;

    }  // end of found
  else
    { // not numeric option, try alpha
    int iResult = FindBaseAlphaOption (strOptionName, ChatAlphaOptionsTable, iItem);
    if (iResult == eOK)
      {

      // alpha option

      if (m_CurrentPlugin &&
          (ChatAlphaOptionsTable [iItem].iFlags & OPT_PLUGIN_CANNOT_WRITE))
    	  return ePluginCannotSetOption;  // not available to plugin

      if (ChatAlphaOptionsTable [iItem].iFlags & OPT_CANNOT_WRITE)
    	  return ePluginCannotSetOption;  // not available for writing at all    

      // ------ preliminary validation before setting the option

      // set the option now

      iResult = SetBaseAlphaOptionItem (iItem,
                        ChatAlphaOptionsTable,
                        NUMITEMS (ChatAlphaOptionsTable),
                        (char *) chat_item,  
                        strValue,
                        bChanged);

    if (iResult == eOK && bChanged)
      {

      if (strOptionName == "group")
        if (strlen (Value) > 0)
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("%s has added you to the group %s", 
                (LPCTSTR) m_strOurChatName, Value));
        else
         chat_item->SendChatMessage (CHAT_MESSAGE, 
             TFormat ("%s has removed you from the chat group", 
                (LPCTSTR) m_strOurChatName));
      }  // end of succesfully changing the option

      return iResult;
      }  // end of found alpha option
    }  // end of not numeric option

 return eUnknownOption;
}  // end of CMUSHclientDoc::SetChatOption

// for scripting, a simple routine to convert a name to an ID
// (so we can use the same alias to work on a name or number)

long CMUSHclientDoc::ChatGetID(LPCTSTR Who) 
{

CString strWho = Who;

  strWho.TrimLeft ();
  strWho.TrimRight ();

  if (strWho.IsEmpty ())
    return 0;   // can't find a blank name

  // for scripts, assume if they supply a number they already know the ID
  if (IsNumber (strWho))
    {
    // however, still warn if they aren't there
    long id = atol (strWho);
    if (GetChatSocket (id) == NULL)
      {
      ChatNote (eChatInformation, 
                TFormat ("Chat ID %ld is not connected.", 
                        id));
      return 0;
      }
    else
      return id;
    }    // end of numeric person

  for (POSITION chatpos = m_ChatList.GetHeadPosition (); chatpos; )
    {
    CChatSocket * pSocket = m_ChatList.GetNext (chatpos);
    if (pSocket->m_iChatStatus == eChatConnected)
      if (pSocket->m_strRemoteUserName.CompareNoCase (strWho) == 0)
        return pSocket->m_iChatID;
    } // end of all chat sessions

  ChatNote (eChatInformation, 
            TFormat ("Cannot find connection \"%s\".", 
                    (LPCTSTR) strWho));

  return 0;  // not found
}   // end of CMUSHclientDoc::ChatGetID

// "chats" the clipboard to a remote user (eg. for pasting aliases etc.)
long CMUSHclientDoc::ChatPasteText(long ID) 
{

  // quiet failure on ID zero - probably caused by a ChatGetID lookup failure
  if (ID == 0)
    return eChatIDNotFound;

  CChatSocket * pSocket = GetChatSocket (ID); // find them

  if (!pSocket)
    {
    ChatNote (eChatOutgoingPersonal, TFormat ("Chat ID %i is not connected.", ID));
	  return eChatIDNotFound;
    }

CString strContents; 

  if (!GetClipboardContents (strContents, m_bUTF_8, false))
    return eClipboardEmpty; // can't do it

CString strMessage;

  strMessage = TFormat ("\n%s pastes to you: \n\n%s%s%s%s\n",
                            (LPCTSTR) m_strOurChatName,
                            (LPCTSTR) AnsiCode (ANSI_BOLD),  
                            (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                            (LPCTSTR) strContents,
                            (LPCTSTR) AnsiCode (ANSI_TEXT_RED)   
                             );

  pSocket->SendChatMessage (CHAT_TEXT_PERSONAL, strMessage);
  ChatNote (eChatOutgoingPersonal,
            TFormat ("You paste to %s: \n\n%s%s%s%s", 
                      (LPCTSTR) pSocket->m_strRemoteUserName, 
                      (LPCTSTR) AnsiCode (ANSI_BOLD),  
                      (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                      (LPCTSTR) strContents,
                      (LPCTSTR) AnsiCode (ANSI_TEXT_RED)));   
    return eOK;

}    // end of CMUSHclientDoc::ChatPasteText

long CMUSHclientDoc::ChatPasteEverybody() 
{
CString strContents; 

  if (!GetClipboardContents (strContents, m_bUTF_8, false))
    return eClipboardEmpty; // can't do it

CString strMessage;

  strMessage = TFormat ("\n%s pastes to everybody: \n\n%s%s%s%s\n",
                            (LPCTSTR) m_strOurChatName,
                            (LPCTSTR) AnsiCode (ANSI_BOLD),  
                            (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                            (LPCTSTR) strContents,
                            (LPCTSTR) AnsiCode (ANSI_TEXT_RED)   
                             );

  if (SendChatMessageToAll (CHAT_TEXT_EVERYBODY,
                                  strMessage,
                                  true,    // unless ignoring them
                                  false,
                                  false,
                                  0,    // don't ignore us
                                  "",   // no group
                                  0))   // use our stamp
  {
  ChatNote (eChatOutgoingEverybody,
            TFormat ("You paste to everybody: \n\n%s%s%s%s", 
                      (LPCTSTR) AnsiCode (ANSI_BOLD),  
                      (LPCTSTR) AnsiCode (ANSI_TEXT_CYAN),  
                      (LPCTSTR) strContents,
                      (LPCTSTR) AnsiCode (ANSI_TEXT_RED)));   
  return eOK;
  }
else
  {
  ChatNote (eChatOutgoingEverybody, 
            "No (relevant) chat connections.");
  return eNoChatConnections;
  }


} // end of CMUSHclientDoc::ChatPasteEverybody


long CMUSHclientDoc::ChatSendFile(long ID, LPCTSTR FileName) 
{

  // quiet failure on ID zero - probably caused by a ChatGetID lookup failure
  if (ID == 0)
    return eChatIDNotFound;

  CChatSocket * pSocket = GetChatSocket (ID); // find them

  if (!pSocket)
    {
    ChatNote (eChatFile, TFormat ("Chat ID %i is not connected.", ID));
	  return eChatIDNotFound;
    }

  if (pSocket->m_bDoingFileTransfer)
    {
    if (pSocket->m_bSendFile)
      ChatNote (eChatFile, TFormat ("Already sending file %s",
                  (LPCTSTR) pSocket->m_strOurFileName));
    else
      ChatNote (eChatFile, TFormat ("Already receiving file %s",
                  (LPCTSTR) pSocket->m_strOurFileName));

    return eAlreadyTransferringFile;
    }

  CString strName = FileName;

  // if no file name, put up standard file dialog
  if (strName.IsEmpty ())
    {

	  CFileDialog	dlg(TRUE,						// TRUE for FileOpen
					  "",						// default extension
					  "",						// initial file name
					  OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
					  "All files (*.*)|*.*||");
	  
    dlg.m_ofn.lpstrTitle = "Select file to send";
	  
    ChangeToFileBrowsingDirectory ();
    int nResult = dlg.DoModal();
    ChangeToStartupDirectory ();

    if (nResult != IDOK)
		  return eFileNotFound;

    strName = dlg.GetPathName();

    }   // end of no file name supplied

  // remember file to open
  pSocket->m_strOurFileName = strName;

         
  try
    {
    pSocket->m_pFile = new CFile (strName, CFile::modeRead | CFile::shareDenyWrite); // open file
    pSocket->m_pFileBuffer = new unsigned char [pSocket->m_iFileBlockSize]; // get buffer    
    pSocket->m_iFileSize = pSocket->m_pFile->GetLength ();
    
    } // end of try block

  catch (CFileException * e)
    {
    ChatNote (eChatFile, TFormat ("File %s cannot be opened.", (LPCTSTR) strName));
    e->Delete ();
    // reset the two fields we changed so far
    pSocket->m_strOurFileName.Empty ();
    pSocket->m_iFileSize = 0;

    delete pSocket->m_pFile;    // in case it was set up
    delete [] pSocket->m_pFileBuffer;  // and get rid of buffer

    return eFileNotFound;
    } // end of catching a file exception

  // find last part of file name (ie. actual file name, not full path)
  pSocket->m_strSenderFileName = pSocket->m_pFile->GetFileName ();

  // ask them to receive the file
  pSocket->SendChatMessage (CHAT_FILE_START, 
      TFormat ("%s,%ld",
                (LPCTSTR) pSocket->m_strSenderFileName,
                pSocket->m_iFileSize));

  // get ready for transfer
  pSocket->m_tStartedFileTransfer = CTime::GetCurrentTime();  // when started
  pSocket->m_bSendFile = true;   // we are sending
  pSocket->m_bDoingFileTransfer = true;           
  pSocket->m_iBlocksTransferred = 0;   // no blocks yet
  // add block-size minus one to allow for final partial block
  pSocket->m_iFileBlocks = (pSocket->m_iFileSize + pSocket->m_iFileBlockSize - 1L) /
                  pSocket->m_iFileBlockSize;

  shsInit  (&pSocket->m_shsInfo);
  double K = pSocket->m_iFileSize / 1024.0;

  ChatNote (eChatFile, TFormat (
              "Initiated transfer of file %s, %ld bytes (%1.1f Kb).", 
                    (LPCTSTR) strName, 
                    pSocket->m_iFileSize,
                    K));
	return eOK;
}    // end of CMUSHclientDoc::ChatSendFile

// abort a file transfer in progress

long CMUSHclientDoc::ChatStopFileTransfer(long ID) 
{

  // quiet failure on ID zero - probably caused by a ChatGetID lookup failure
  if (ID == 0)
    return eChatIDNotFound;

  CChatSocket * pSocket = GetChatSocket (ID); // find them

  if (!pSocket)
    {
    ChatNote (eChatOutgoingPersonal, TFormat ("Chat ID %i is not connected.", ID));
	  return eChatIDNotFound;
    }

  if (!pSocket->m_bDoingFileTransfer)
    return eNotTransferringFile;

  pSocket->StopFileTransfer (true);

	return eOK;
}  // end of CMUSHclientDoc::ChatStopFileTransfer
