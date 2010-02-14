// mxpOnOff.cpp - turning MXP on and off

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


// this is for an MXP reset or MXP off
// if bCompletely is true then we are turning MXP right off, otherwise
// we are resetting to normal behaviour
void CMUSHclientDoc::MXP_Off (const bool bCompletely)
  {

  if (m_pCurrentLine)          // provided we have a line yet :)
    InterpretANSIcode (0);    // go back to white on black

  // do nothing else if already off
  if (!m_bMXP)
    return;

  if (bCompletely)
    Frame.SetStatusMessageNow (Translate ("Closing down MXP"));

  MXP_CloseAllTags ();
  m_bInParagraph = false; 
  m_bMXP_script = false;    // cancel scripts
  m_bPreMode = false;       // no more preformatted text
  m_iListMode = eNoList;    // no more ordered/unordered lists
  m_iListCount = 0;

  MXP_error (DBG_INFO, infoMXP_ResetReceived, "MXP reset.");

  if (bCompletely)
    {
    MXP_mode_change (eMXP_open);  // back to open mode

    // if not using MXP (any more) then turn collection phase off
    if (
       m_phase == HAVE_MXP_ELEMENT ||
       m_phase == HAVE_MXP_COMMENT ||
       m_phase == HAVE_MXP_QUOTE ||
       m_phase == HAVE_MXP_ENTITY ||
       m_phase == HAVE_MXP_ROOM_NAME ||
       m_phase == HAVE_MXP_ROOM_DESCRIPTION ||
       m_phase == HAVE_MXP_ROOM_EXITS ||
       m_phase == HAVE_MXP_WELCOME
       )
      m_phase = NONE;
    if (m_bPuebloActive)
      MXP_error (DBG_INFO, infoMXP_off, "Pueblo turned off.");
    else
      MXP_error (DBG_INFO, infoMXP_off, "MXP turned off.");
    m_bPuebloActive = false;
    m_bMXP = false;

    // execute "close" script
    if (m_dispidOnMXP_Stop != DISPID_UNKNOWN)
      {
      if (SeeIfHandlerCanExecute (m_strOnMXP_Stop))
        {
        DISPPARAMS params = { NULL, NULL, 0, 0 };
        long nInvocationCount = 0;

        ExecuteScript (m_dispidOnMXP_Stop,  
                     m_strOnMXP_Stop,
                     eWorldAction,
                     "MXP shutdown", 
                     "stopping MXP",
                     params, 
                     nInvocationCount); 
        }
      } // end of executing close script

    // tell each plugin we have connected
    for (POSITION pos = m_PluginList.GetHeadPosition(); pos; )
      {
      CPlugin * pPlugin = m_PluginList.GetNext (pos);

      if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
        continue;

      pPlugin->ExecutePluginScript (ON_PLUGIN_MXP_STOP, pPlugin->m_dispid_plugin_OnMXP_Stop);
      }   // end of doing each plugin

    } // end of turn MXP off completely
  }  // end of CMUSHclientDoc::MXP_Off


void CMUSHclientDoc::MXP_On (const bool bPueblo, const bool bManual)
 {

  // do nothing if already on
  if (m_bMXP)
    return;

  if (bPueblo)
    MXP_error (DBG_INFO, infoMXP_on, "Pueblo turned on.");
  else
    MXP_error (DBG_INFO, infoMXP_on, "MXP turned on.");

  // execute "open" script
  if (m_dispidOnMXP_Start != DISPID_UNKNOWN)
    {
    if (SeeIfHandlerCanExecute (m_strOnMXP_Start))
      {

      DISPPARAMS params = { NULL, NULL, 0, 0 };
      long nInvocationCount = 0;

      ExecuteScript (m_dispidOnMXP_Start,  
                   m_strOnMXP_Start,
                   eWorldAction,
                   "MXP startup", 
                   "starting MXP",
                   params, 
                   nInvocationCount); 
        }
    } // end of executing open script

  // tell each plugin MXP is starting
  for (POSITION pos = m_PluginList.GetHeadPosition(); pos; )
    {
    CPlugin * pPlugin = m_PluginList.GetNext (pos);

    if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
      continue;

    pPlugin->ExecutePluginScript (ON_PLUGIN_MXP_START, pPlugin->m_dispid_plugin_OnMXP_Start);
    }   // end of doing each plugin

 m_bMXP = true;
 m_bPuebloActive = bPueblo;
 m_bMXP_script = false;
 m_bPreMode = false;
 m_iLastOutstandingTagCount = 0;
 m_iMXPerrors = 0;     
 m_iMXPtags = 0;       
 m_iMXPentities = 0; 
 m_iListMode = eNoList;
 m_iListCount = 0;

 // if they turn it on manually we want to leave everything set up
 //  (eg. they turn it off, they turn it on again)

 if (!bManual)
   {
   // make sure we are back to open as default

   m_iMXP_defaultMode = m_iMXP_mode = eMXP_open;

  // delete old definitions - we will assume we start from scratch here

  // delete custom elements map

    DELETE_MAP (m_CustomElementMap, CElement); 

  // delete active tags list

    DELETE_LIST (m_ActiveTagList);

  // delete custom entities list

    m_CustomEntityMap.RemoveAll ();
   }

 }  // end of CMUSHclientDoc::MXP_On

