// chatsock.h : interface of the CChatSocket class
//

#ifndef __CHATSOCK_H__
#define __CHATSOCK_H__

class CMUSHclientDoc;

// if an identical message is received, that we just sent, that are the same,
// in this time interval, discard the second one (if addressed to all or a group)
#define LOOP_DISCARD_SAME_MESSAGE_SECONDS 5

// MudMaster Chat Message Codes

#define CHAT_NAME_CHANGE				   1
#define CHAT_REQUEST_CONNECTIONS 	 2
#define CHAT_CONNECTION_LIST	 		 3
#define CHAT_TEXT_EVERYBODY			   4
#define CHAT_TEXT_PERSONAL			   5
#define CHAT_TEXT_GROUP				     6
#define CHAT_MESSAGE					     7
#define CHAT_DO_NOT_DISTURB			   8
#define CHAT_SEND_ACTION				   9
#define CHAT_SEND_ALIAS				    10
#define CHAT_SEND_MACRO				    11
#define CHAT_SEND_VARIABLE				12
#define CHAT_SEND_EVENT			    	13
#define CHAT_SEND_GAG				      14
#define CHAT_SEND_HIGHLIGHT				15
#define CHAT_SEND_LIST				    16
#define CHAT_SEND_ARRAY				    17
#define CHAT_SEND_BARITEM			  	18
#define CHAT_VERSION					    19
#define CHAT_FILE_START				    20
#define CHAT_FILE_DENY				    21
#define CHAT_FILE_BLOCK_REQUEST		22
#define CHAT_FILE_BLOCK			    	23
#define CHAT_FILE_END				    	24
#define CHAT_FILE_CANCEL				  25
#define CHAT_PING_REQUEST				  26
#define CHAT_PING_RESPONSE				27
#define CHAT_PEEK_CONNECTIONS			28
#define CHAT_PEEK_LIST			    	29
#define CHAT_SNOOP					      30
#define CHAT_SNOOP_DATA				    31

#define CHAT_ICON     				   100
#define CHAT_STATUS   				   101
#define CHAT_EMAIL_ADDRESS  	   102
#define CHAT_REQUEST_PGP_KEY     103
#define CHAT_PGP_KEY             104
#define CHAT_SEND_COMMAND        105  // extension, however see zChat
#define CHAT_STAMP               106

#define CHAT_END_OF_COMMAND			 '\xFF' // 255

// types of ChatNotes

enum {
     eChatConnection,          //  0 = connection attempts
     eChatSession,             //  1 = start, end of session
     eChatNameChange,          //  2 = CHAT_NAME_CHANGE
     eChatMessage,             //  3 = CHAT_MESSAGE
     eChatIncomingPersonal,    //  4 = CHAT_TEXT_PERSONAL   - inwards
     eChatIncomingEverybody,   //  5 = CHAT_TEXT_EVERYBODY  - inwards
     eChatIncomingGroup,       //  6 = CHAT_TEXT_GROUP      - inwards
     eChatOutgoingPersonal,    //  7 = CHAT_TEXT_PERSONAL   - sent
     eChatOutgoingEverybody,   //  8 = CHAT_TEXT_EVERYBODY  - sent
     eChatOutgoingGroup,       //  9 = CHAT_TEXT_GROUP      - sent
     eChatPeekList,            // 10 = CHAT_PEEK_LIST
     eChatConnectionList,      // 11 = CHAT_CONNECTION_LIST
     eChatPing,                // 12 = CHAT_PING_RESPONSE 
     eChatInformation,         // 13 = CHAT_REQUEST_CONNECTIONS, CHAT_PEEK_CONNECTIONS, CHAT_SNOOP 
     eChatFile,                // 14 = CHAT_FILE_START, CHAT_FILE_DENY, CHAT_FILE_END, CHAT_FILE_CANCEL
     eChatSnoopData,           // 15 = CHAT_SNOOP_DATA
     eChatCommand,             // 16 = CHAT_SEND_COMMAND
  };    

// this is for making an outgoing call to a chat server

// values for m_iChatStatus
enum {
     eChatClosed,           // not yet connected
     eChatConnecting,       // connecting, not yet connected to server
     eChatAwaitingConnectConfirm,     // awaiting YES: from server
     eChatAwaitingConnectionRequest,  // awaiting CHAT: from caller
     eChatConnected,
  };

// values for m_iChatConnectionType

enum {
     eChatMudMaster,
     eChatZMud,
  };    
     
class CChatSocket : public CAsyncSocket
{
	DECLARE_DYNAMIC(CChatSocket);

// Construction
public:
	CChatSocket(CMUSHclientDoc* pDoc);
	~CChatSocket();

// Operations
public:
	CMUSHclientDoc* m_pDoc;
  CString m_outstanding_output;   // stuff we have yet to send them
  CString m_outstanding_input;    // input not yet processed (eg. no \xFF yet)

  CString m_strServerName;       // server name before DNS resolution
	SOCKADDR_IN m_ServerAddr;      // Chat server address/port

  CString m_strRemoteUserName;     // remote user name
  CString m_strGroup;              // group that user is in
  CString m_strRemoteVersion;      // version string from remote user

  CString m_strAllegedAddress;    // what they claim is their IP address
  short   m_iAllegedPort;         // port to call them on

  HANDLE      m_hNameLookup;      // for name lookup
  char *      m_pGetHostStruct;   // ditto

  int m_iChatStatus;              // status of session, see enum above
  int m_iChatConnectionType;      // type of connection, see enum above
  bool m_bDeleteMe;               // if true, should be deleted
  long m_iChatID;                 // unique chat session identifier

  CTime m_tWhenStarted;           // when session started
  CTime m_tLastIncoming;          // when last incoming message
  CTime m_tLastOutgoing;          // when last outgoing message

  bool m_bIncoming;               // we received this call, not made it
  bool m_bIgnore;                 // we are ignoring him
  bool m_bCanSnoop;               // he can snoop us
  bool m_bYouAreSnooping;         // we are snooping him
  bool m_bHeIsSnooping;           // he is snooping us
  bool m_bCanSendCommands;        // can send us commands
  bool m_bPrivate;                // private connection
  bool m_bCanSendFiles;           // he can send files
  bool m_bWasConnected;           // if true, he was connected at some stage
  LARGE_INTEGER m_iPingStartTime; // time we sent a ping 
  double m_fLastPingTime;         // how long the last ping took
  long m_zChatStamp;              // unique zChat ID
  CString m_strEmailAddress;      // email address from zChat
  CString m_strPGPkey;            // PGP key from zChat
  short   m_zChatStatus;          // status code from zChat
  long   m_iUserOption;           // user-settable flags etc.

  // file sending/receiving

  bool m_bDoingFileTransfer;      // true = in middle of file transfer
  bool m_bSendFile;               // true = send file, false = receive file
  CString m_strSenderFileName;    // name from sender (or to receiver)
  CString m_strOurFileName;       // our actual file path on the local machine
  long m_iFileSize;               // bytes in file
  long m_iFileBlocks;             // total blocks in file
  long m_iBlocksTransferred;      // how many we sent/received so far
  CFile * m_pFile;                // open file we are sending/saving
  CTime m_tStartedFileTransfer;   // when file send/receive started
  long m_iFileBlockSize;          // size of file block
  unsigned char * m_pFileBuffer;  // buffer of size m_iFileBlockSize bytes
  SHS_INFO m_shsInfo;             // for hashing file data

  // counters

  long m_iCountIncomingPersonal;  // incoming personal messages
  long m_iCountIncomingAll;       // incoming personal messages
  long m_iCountIncomingGroup;     // incoming personal messages
  long m_iCountOutgoingPersonal;  // outgoing personal messages
  long m_iCountOutgoingAll;       // outgoing personal messages
  long m_iCountOutgoingGroup;     // outgoing personal messages
  long m_iCountMessages;          // other messages (CHAT_MESSAGE type)
  long m_iCountFileBytesIn;       // file bytes received
  long m_iCountFileBytesOut;      // file bytes sent



// Implementation

  virtual void OnReceive    (int nErrorCode);
	virtual void OnSend       (int nErrorCode);
	virtual void OnClose      (int nErrorCode);
	virtual void OnConnect    (int nErrorCode);

  void HostNameResolved (WPARAM wParam, LPARAM lParam);
  void MakeCall (void);
  void SendData (const CString & strText);

  void ProcessChatMessage (const int iMessage, const CString strMessage);
  void SendChatMessage (const int iMessage, 
                        const CString strMessage, 
                        const long iStamp = 0);
  void StopFileTransfer (const bool bAbort);

  long GetStamp (CString & strMessage);    // extracts message stamp

  CString MakeStamp (const long iStamp);   // makes zChat message stamp

  // message processors

  void Process_Name_change				  (const CString strMessage);
  void Process_Request_connections 	(const CString strMessage);
  void Process_Connection_list	 		(const CString strMessage);
  void Process_Text_everybody			  (const CString strMessage);
  void Process_Text_personal			  (const CString strMessage);
  void Process_Text_group				    (const CString strMessage);
  void Process_Message					    (const CString strMessage);
  void Process_Version					    (const CString strMessage);
  void Process_File_start				    (const CString strMessage);
  void Process_File_deny				    (const CString strMessage);
  void Process_File_block_request		(const CString strMessage);
  void Process_File_block			    	(const CString strMessage);
  void Process_File_end				    	(const CString strMessage);
  void Process_File_cancel				  (const CString strMessage);
  void Process_Ping_request				  (const CString strMessage);
  void Process_Ping_response				(const CString strMessage);
  void Process_Peek_connections			(const CString strMessage);
  void Process_Peek_list			    	(const CString strMessage);
  void Process_Snoop					      (const CString strMessage);
  void Process_Snoop_data				    (const CString strMessage);

  // zChat commands
  
  void Process_Icon       				  (const CString strMessage);
  void Process_Status     				  (const CString strMessage);
  void Process_EmailAddress				  (const CString strMessage);
  void Process_Stamp      				  (const CString strMessage);
  void Process_RequestPGPkey        (const CString strMessage);
  void Process_PGPkey               (const CString strMessage);
  void Process_Send_command         (const CString strMessage);

};

// we will have a list of them

typedef CTypedPtrList <CPtrList, CChatSocket*> CChatSocketList;

#endif // __CHATSOCK_H__
