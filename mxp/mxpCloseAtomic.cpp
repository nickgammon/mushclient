// mxpCloseAtomic.cpp - close one atomic tag

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


// do the action required to close a single atomic tag (iAction)
void CMUSHclientDoc::MXP_CloseAtomicTag (const int iAction, 
                                         const CString & strText,
                                         const POSITION firstlinepos,
                                         const POSITION firststylepos)
  {
  POSITION linepos, 
           stylepos;
  int iCount = 0;
  CAction * pAction = NULL;;

  // now take any closing action 
  switch (iAction)
    {
    case MXP_ACTION_SEND: 
    case MXP_ACTION_HYPERLINK:

      // put send action into every style

      for (linepos = firstlinepos; linepos; iCount++)
        {
        CLine * pLine2 = m_LineList.GetNext (linepos);
 
        if (iCount == 0)    // first line - start at first style
           stylepos = firststylepos;
        else
          stylepos = pLine2->styleList.GetHeadPosition ();

        for ( ; stylepos; )
          {
          CStyle * pStyle2 = pLine2->styleList.GetNext (stylepos);
          if ((pStyle2->iFlags & ACTIONTYPE) &&
              (pStyle2->iFlags & START_TAG) == 0)
            {
            CString strAction;
            CString strHint;
            CString strVariable;
            if (pStyle2->pAction)
              {
              strAction = pStyle2->pAction->m_strAction;
              strHint = pStyle2->pAction->m_strHint;
              strVariable = pStyle2->pAction->m_strVariable;
              }

            if (m_bUseCustomLinkColour && !m_bMudCanChangeLinkColour)
              {
              COLORREF colour1,
                       colour2;

              // find current foreground and background RGB values
              GetStyleRGB (pStyle2, colour1, colour2);

              pStyle2->iForeColour = m_iHyperlinkColour;    // override hyperlink colour
              pStyle2->iBackColour = colour2;    // keep background
              pStyle2->iFlags &= ~COLOURTYPE;  // clear bits, eg. custom
              pStyle2->iFlags |= COLOUR_RGB;
              } // end of changing colour back to wanted link colour

            if (m_bUnderlineHyperlinks && !m_bMudCanRemoveUnderline)
              pStyle2->iFlags |= UNDERLINE;    // make sure underlined

            if (strAction.IsEmpty ())
              { // no action defined - use &text; as the action
               if (pAction == NULL)
                 {
                 pAction = GetAction (strText, strHint, strVariable);
                 pStyle2->pAction = pAction;
                 }
               else
                 {
                 pStyle2->pAction = pAction;  // remember in case nested <send>s
                 pAction->AddRef ();
                 }
              }
            else
              {   // we have an strAction already
              // replace the &text; sequence
              strAction.Replace ("&text;", strText);
              strHint.Replace ("&text;", strText);
              pStyle2->pAction->Release ();
              pStyle2->pAction = GetAction (strAction, strHint, strVariable);
              // remember replacement text for next time
              if (pAction == NULL)
                pAction = pStyle2->pAction; 
              } // end of having text to send

            }  // end of being an ordinary "send" style
          } // end of doing each style on this line
        } // end of doing each line
      break;  // end of MXP_ACTION_SEND or MXP_ACTION_HYPERLINK

      // end script
    case MXP_ACTION_SCRIPT:
      MXP_error (DBG_INFO, infoMXP_ScriptCollectionCompleted,
                "Script collection mode completed.");
      m_bMXP_script = false;
      break;  // end of MXP_ACTION_SCRIPT

      // end new para
    case MXP_ACTION_P:
       m_bInParagraph = false;      
       StartNewLine (true, 0);
       break;  // end of MXP_ACTION_P

    case MXP_ACTION_PRE: 
      m_bPreMode = false;
      break;  // end of MXP_ACTION_PRE

     case MXP_ACTION_UL:   
       m_iListMode = eNoList;
       m_iListCount = 0;
       // wrap up previous line if necessary
       if (m_pCurrentLine->len > 0)
          StartNewLine (true, 0);
       break;  // end of MXP_ACTION_UL
     
     case MXP_ACTION_OL:   
       m_iListMode = eNoList;
       m_iListCount = 0;
       // wrap up previous line if necessary
       if (m_pCurrentLine->len > 0)
          StartNewLine (true, 0);

       break;  // end of MXP_ACTION_OL

      case MXP_ACTION_VAR: 
        {
        }
        break;
    } // end of switch on action type

  } // end of CMUSHclientDoc::MXP_CloseAtomicTag


