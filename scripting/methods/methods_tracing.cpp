// methods_tracing.cpp

// Related to the tracing feature

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\errors.h"

// Implements:

//    Trace
//    TraceOut



// routine for doing traces

void CMUSHclientDoc::Trace (LPCTSTR lpszFormat, ...)
{         
  // do nothing if not tracing

  if (!m_bTrace)
    return;

	ASSERT(AfxIsValidString(lpszFormat, FALSE));

CString strMsg;

	va_list argList;
	va_start(argList, lpszFormat);
	strMsg.FormatV(lpszFormat, argList);
	va_end(argList);

  // see if a plugin will handle trace message

  m_bTrace = false;  // stop infinite loops, where we report that the trace script was called
  if (SendToFirstPluginCallbacks (ON_PLUGIN_TRACE, strMsg))
    {
    m_bTrace = true;
    return;   // sent to plugin? don't display it
    }

  m_bTrace = true;

  strMsg += ENDLINE;      // add a new line

CString strFullMsg = "TRACE: ";

  strFullMsg += strMsg;


  DisplayMsg (strFullMsg, strFullMsg.GetLength (), COMMENT);

}  // end of CMUSHclientDoc::Trace 


void CMUSHclientDoc::TraceOut(LPCTSTR Message) 
{
  Trace (Message);
}   // end of CMUSHclientDoc::TraceOut


BOOL CMUSHclientDoc::GetTrace() 
{
	return m_bTrace;
}  // end of CMUSHclientDoc::GetTrace

void CMUSHclientDoc::SetTrace(BOOL bNewValue) 
{

bNewValue = bNewValue != 0;   // make boolean

// if they are changing the value - go ahead and do it
if (bNewValue && !m_bTrace ||
    !bNewValue && m_bTrace)
  OnGameTrace ();

} // end of CMUSHclientDoc::SetTrace
