// methods_logging.cpp

// Relating to logging

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\errors.h"

// Implements:

//    CloseLog
//    FlushLog
//    IsLogOpen
//    OpenLog
//    WriteLog


long CMUSHclientDoc::OpenLog(LPCTSTR LogFileName, BOOL Append) 
{

  if (m_logfile)
  	return eLogFileAlreadyOpen;

  m_logfile_name = LogFileName;

  // if no log file name, use auto-log file name if we can

  if (m_logfile_name.IsEmpty ())
    {
    CTime theTime;
    theTime = CTime::GetCurrentTime();

    m_logfile_name = FormatTime (theTime, m_strAutoLogFileName, false);

    }

  // no file name? can't open it then
  if (m_logfile_name.IsEmpty ())
    return eCouldNotOpenFile;

  m_logfile = fopen (m_logfile_name, Append ? "a+" : "w");
	
  // close and re-open to make sure it is in the disk directory
  if (m_logfile)
    {
    fclose (m_logfile);
    m_logfile = fopen (m_logfile_name, Append ? "a+" : "w");
    }

	if (m_logfile)
    return eOK;
  else
    return eCouldNotOpenFile;

}   // end of CMUSHclientDoc::OpenLog

long CMUSHclientDoc::CloseLog() 
{

  if (m_logfile)
    {

  // write log file Postamble if wanted

    if (!m_strLogFilePostamble.IsEmpty () && !m_bLogRaw)
      {
      CTime theTime;
      theTime = CTime::GetCurrentTime();

      // allow %n for newline
      CString strPostamble = ::Replace (m_strLogFilePostamble, "%n", "\n");

      // allow for time-substitution strings
      strPostamble = FormatTime (theTime, strPostamble, m_bLogHTML);

      // this is open in text mode, don't want \r\r\n
      strPostamble.Replace (ENDLINE, "\n");

      WriteToLog (strPostamble); 
      WriteToLog ("\n", 1);
      }

    fclose (m_logfile);
    m_logfile = NULL;
    return eOK;
    }

	return eLogFileNotOpen;
}   // end of CMUSHclientDoc::CloseLog


long CMUSHclientDoc::WriteLog(LPCTSTR Message) 
{
  if (m_logfile)
    {

    CString strMessage = Message;

    // append newline if there isn't one already
    if (strMessage.Right (2) != "\n")
      strMessage += "\n";

    size_t count;
    size_t len = strMessage.GetLength ();

    count = fwrite (strMessage, 1, len, m_logfile);

    if (count != len)
      return eLogFileBadWrite;

    return eOK;
    }

	return eLogFileNotOpen;
}   // end of CMUSHclientDoc::WriteLog

BOOL CMUSHclientDoc::IsLogOpen() 
{
	return m_logfile != NULL;
}   // end of CMUSHclientDoc::IsLogOpen


long CMUSHclientDoc::FlushLog() 
{
  
  if (m_logfile)
    {
    fflush (m_logfile);
    return eOK;
    }

	return eLogFileNotOpen;

}  // end of CMUSHclientDoc::FlushLog
