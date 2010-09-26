// methods_sending.cpp

// Stuff for sending things to the MUD

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\..\MUSHview.h"
#include "..\..\sendvw.h"
#include "..\errors.h"

// Implements:

//    LogSend
//    Send
//    SendImmediate
//    SendNoEcho
//    SendPush
//    SendSpecial

// world.Send - sends the text to the world

long CMUSHclientDoc::Send(LPCTSTR Message) 
{
  if (m_iConnectPhase != eConnectConnectedToMud)
    return eWorldClosed;             

  // cannot change what we are sending in OnPluginSent
  if (m_bPluginProcessingSent)
    return eItemInUse;

  SendMsg (Message, m_display_my_input, false, false);  // don't queue it

	return eOK;

}   // end of CMUSHclientDoc::Send

// send disregarding the queue (ie. queue jump)

long CMUSHclientDoc::SendImmediate(LPCTSTR Message) 
{
  if (m_iConnectPhase != eConnectConnectedToMud)
    return eWorldClosed;             

  // cannot change what we are sending in OnPluginSent
  if (m_bPluginProcessingSent)
    return eItemInUse;

  DoSendMsg (Message, m_display_my_input, m_log_input); 

	return eOK;
}    // end of CMUSHclientDoc::SendImmediate

long CMUSHclientDoc::SendNoEcho(LPCTSTR Message) 
{
  if (m_iConnectPhase != eConnectConnectedToMud)
    return eWorldClosed;             

  // cannot change what we are sending in OnPluginSent
  if (m_bPluginProcessingSent)
    return eItemInUse;

  SendMsg (Message, false, false, false);  // don't queue it, don't echo it

	return eOK;
}   // end of CMUSHclientDoc::SendNoEcho


void CMUSHclientDoc::AddToCommandHistory (LPCTSTR Message)
  {

  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  CView* pView = GetNextView(pos);
	  
	  if (pView->IsKindOf(RUNTIME_CLASS(CSendView)))
  	  {
		  CSendView* pmyView = (CSendView*)pView;

      if (strlen (Message) > 0 && strcmp (Message, pmyView->m_last_command) != 0)
        {
        if (pmyView->m_inputcount >= m_nHistoryLines)
          {
          pmyView->m_msgList.RemoveHead ();   // keep max of "m_nHistoryLines" previous commands
          pmyView->m_HistoryFindInfo.m_nCurrentLine--;     // adjust for a "find again"
          if (pmyView->m_HistoryFindInfo.m_nCurrentLine < 0)
            pmyView->m_HistoryFindInfo.m_nCurrentLine = 0;
          }   // end of buffer full
        else
          pmyView->m_inputcount++;
        pmyView->m_msgList.AddTail (Message);
        pmyView->m_last_command = Message;
        } // end of different from last one

      break;
	    }	  // end of being a CSendView
    }   // end of loop through views

  } // end of CMUSHclientDoc::AddToCommandHistory

long CMUSHclientDoc::SendPush(LPCTSTR Message) 
{
  if (m_iConnectPhase != eConnectConnectedToMud)
    return eWorldClosed;             

  // cannot change what we are sending in OnPluginSent
  if (m_bPluginProcessingSent)
    return eItemInUse;

  SendMsg (Message, m_display_my_input, false, false);  // don't queue it

  AddToCommandHistory (Message);

	return eOK;
}   // end of SendPush

long CMUSHclientDoc::SendSpecial(LPCTSTR Message, BOOL Echo, BOOL Queue, BOOL Log, BOOL History) 
{
  if (m_iConnectPhase != eConnectConnectedToMud)
    return eWorldClosed;             

  // cannot change what we are sending in OnPluginSent
  if (m_bPluginProcessingSent)
    return eItemInUse;

  // send with appropriate options
  SendMsg (Message, Echo, Queue, Log);  

  if (History)
     AddToCommandHistory (Message);

	return eOK;

}   // end of CMUSHclientDoc::SendSpecial


long CMUSHclientDoc::LogSend(LPCTSTR Message) 
{
long iResult = Send (Message);  // send it

  if (iResult == eOK)
    LogCommand (Message);   // and log it

	return iResult;
}   // end of CMUSHclientDoc::LogSend

