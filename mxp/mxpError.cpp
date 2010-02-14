// mxpError.cpp - MXP errors/warnings

#include "stdafx.h"
#include "..\MUSHclient.h"

#include "..\doc.h"
#include "..\MUSHview.h"
#include "..\mainfrm.h"
#include "mxp.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


// general MXP error message routine - appends a newline, and also writes to debug 
void CMUSHclientDoc::MXP_error (const int iLevel, 
                                const long iMessageNumber, 
                                CString strMessage)
  {

char * sLevel [] = 
  {
  " ",
  "E", // error
  "W", // warning
  "I", // info
  "A", // all
  };

char * p = "?";

  // turn error level into a character
  if (iLevel >= 0 && iLevel < NUMITEMS (sLevel))
    p = sLevel [iLevel];

  if (iLevel == DBG_ERROR)
    {
    m_iMXPerrors++;
    MXP_Restore_Mode (); // an error cancels secure-once mode
    } // end of error

  // call script if required
  if (m_dispidOnMXP_Error != DISPID_UNKNOWN)
    {
    long nInvocationCount = 0;

    CString strType = "MXP Error";
    CString strReason =  "processing MXP error";

    if (GetScriptEngine () && GetScriptEngine ()->IsLua ())
      {
      list<double> nparams;
      list<string> sparams;
      nparams.push_back (iMessageNumber);  // error number
      nparams.push_back (m_LineList.GetCount ());  // line
      sparams.push_back (p);    // level (a character)
      sparams.push_back ((LPCTSTR) strMessage);    // message
      bool result;
      GetScriptEngine ()->ExecuteLua (m_dispidOnMXP_Error, 
                                     m_strOnMXP_Error, 
                                     eWorldAction,
                                     strType, 
                                     strReason, 
                                     nparams,
                                     sparams, 
                                     nInvocationCount,
                                     NULL, NULL, NULL,
                                     &result);
      if (result)
         return;
      }   // end of Lua
   else
       {
      // WARNING - arguments should appear in REVERSE order to what the sub expects them!

      enum
        {
        eErrorMessage,
        eLineNumber,
        eMessageNumber,
        eErrorLevel,
        eArgCount,     // this MUST be last
        };    

      COleVariant args [eArgCount];
      DISPPARAMS params = { args, NULL, eArgCount, 0 };

      args [eErrorLevel] = p;
      args [eMessageNumber] = iMessageNumber;
      args [eLineNumber] = (long) m_LineList.GetCount ();
      args [eErrorMessage] = strMessage;

      COleVariant result;

      ExecuteScript (m_dispidOnMXP_Error,  
                     m_strOnMXP_Error,
                     eWorldAction,
                     strType, 
                     strReason,
                     params, 
                     nInvocationCount,
                     &result); 

      // if the function returns a non-zero result, don't display the message
      if (result.vt != VT_EMPTY)
        {
        result.ChangeType (VT_I4);  // make a long
        if (result.vt == VT_I4)   // conversion successful
          if (result.lVal)        // return if non-zero
            return;
        }
      }  // not Lua
    }  // end of script callback wanted


  // tell each plugin about the error tag
  if (m_bPluginProcessesError)
    for (POSITION pluginpos = m_PluginList.GetHeadPosition(); pluginpos; )
      {
      CPlugin * pPlugin = m_PluginList.GetNext (pluginpos);

      if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
        continue;

      // see what the plugin makes of this,
      pPlugin->ExecutePluginScript (ON_PLUGIN_MXP_ERROR, 
                            pPlugin->m_dispid_plugin_OnMXP_Error, 
                            CFormat ("%s,%ld,%ld,%s",
                            (LPCTSTR) p,
                            iMessageNumber,
                            (long) m_LineList.GetCount (),
                            (LPCTSTR) strMessage)
                            );
      }   // end of doing each plugin
  m_CurrentPlugin = NULL;

  CString strTitle = MXP_ERROR_WINDOW;
  strTitle += " - ";
  strTitle += m_mush_name;

  if (iLevel > m_iMXPdebugLevel)   // only show required level
    return;

  CString str = CFormat ("%s %5i: (%5i) %s%s",
                      p,     // error level
                      iMessageNumber,   // actual error number
                      m_LineList.GetCount (),  // which line
                      (LPCTSTR) strMessage,     // what message
                      ENDLINE);

  AppendToTheNotepad (strTitle, 
                      str,                 // start new line
                      false,   // append
                      eNotepadMXPdebug);
  }

// here at an unexpected termination of element collection, eg. <blah \n
void CMUSHclientDoc::MXP_unterminated_element (const char * pReason)
  {
  char * p = "thing";
  int i = errMXP_Unknown;
  switch (m_phase)
    {
    case HAVE_MXP_ELEMENT: p = "element"; i = errMXP_UnterminatedElement; break;
    case HAVE_MXP_COMMENT: p = "comment"; i = errMXP_UnterminatedComment; break;
    case HAVE_MXP_ENTITY:  p = "entity"; i = errMXP_UnterminatedEntity; break;
    case HAVE_MXP_QUOTE:   p = "quote"; i = errMXP_UnterminatedQuote; break;
    } // end of switch
  MXP_error (DBG_ERROR, 
              i,    // error level
              TFormat ("Unterminated MXP %s: %s (%s)", 
              (LPCTSTR) p, 
              (LPCTSTR) m_strMXPstring,
              pReason));
  } // end of CMUSHclientDoc::MXP_unterminated_element
