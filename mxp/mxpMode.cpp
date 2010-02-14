// mxpMode.cpp - MXP mode change

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


// here at mode change (eg. change from secure to insecure)
// new mode of -1 means implied mode change (eg. newline)

void CMUSHclientDoc::MXP_mode_change (int iNewMode)
  {

  // change to new mode, -1 means use the default after a newline
  if (iNewMode == -1)
    iNewMode = m_iMXP_defaultMode;

//  if (iNewMode != m_iMXP_mode)
//      TRACE2 ("MXP mode change from %i to %i\n", m_iMXP_mode, iNewMode);


// information message if moving in or out of permanent modes

char * sMode [] =
  {
  "open",               // 0
  "secure",             // 1
  "locked",             // 2
  "reset",              // 3
  "secure next tag only",  // 4
  "permanently open",   // 5
  "permanently secure", // 6
  "permanently locked"  // 7
  };

  if (iNewMode != m_iMXP_mode &&
      (iNewMode == eMXP_perm_open ||
       iNewMode == eMXP_perm_secure ||
       iNewMode == eMXP_perm_locked ||
       m_iMXP_mode == eMXP_perm_open ||
       m_iMXP_mode == eMXP_perm_secure ||
       m_iMXP_mode == eMXP_perm_locked
      )
     )
    {
    CString strOldMode,
            strNewMode;

    if (m_iMXP_mode >= 0 && m_iMXP_mode < NUMITEMS (sMode))
       strOldMode = sMode [m_iMXP_mode];
    else
       strOldMode = TFormat ("unknown mode %i", m_iMXP_mode);

    if (iNewMode >= 0 && iNewMode < NUMITEMS (sMode))
       strNewMode = sMode [iNewMode];
    else
       strNewMode = TFormat ("unknown mode %i", iNewMode);

    MXP_error (DBG_INFO, infoMXP_ModeChange, 
                TFormat ("MXP mode change from '%s' to '%s'", 
                          (LPCTSTR) strOldMode, 
                          (LPCTSTR) strNewMode)); 

    }


  // close open tags on mode change from open to not open
  if (MXP_Open () && 
      iNewMode != eMXP_open &&
      iNewMode != eMXP_perm_open)
    MXP_CloseOpenTags ();

  switch (iNewMode)
    {
    case eMXP_open:
    case eMXP_secure:
    case eMXP_locked:
      m_iMXP_defaultMode = eMXP_open;  // these make open the default mode
      break;

    case eMXP_secure_once:
      m_iMXP_previousMode = m_iMXP_mode;
      break;

    case eMXP_perm_open:
    case eMXP_perm_secure:
    case eMXP_perm_locked:
      m_iMXP_defaultMode = iNewMode;   // these make the new mode permanent
      break;

    } // end of switch

  // set the new mode right now
  m_iMXP_mode = iNewMode;

  } // end of CMUSHclientDoc::MXP_mode_change
