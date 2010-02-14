// mxpOpenAtomic.cpp - open one atomic tag

#include "stdafx.h"
#include "..\MUSHclient.h"

#include "..\doc.h"
#include "..\MUSHview.h"
#include "..\mainfrm.h"
#include "mxp.h"
#include "..\scripting\errors.h"
#include "..\color.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// version of MXP to report to servers
#define MXP_VERSION "0.5"

// for now we'll treat Pueblo and MXP the same
#define PUEBLO_ACTIVE true

extern tConfigurationNumericOption OptionsTable [];

// do the action required to open a single atomic tag (iAction)
void CMUSHclientDoc::MXP_OpenAtomicTag (const CString strTag,
                                        int iAction, 
                                        CStyle * pStyle,
                                        CString & strAction,    // new action
                                        CString & strHint,      // new hint
                                        CString & strVariable,   // new variable
                                        CArgumentList & ArgumentList)
  {
CString strArgument;
CString strArgumentName;
bool bIgnoreUnusedArgs = false; // cut down on some spam by setting this
COLORREF colour1,
         colour2;

unsigned short iFlags      = pStyle->iFlags;      
COLORREF       iForeColour = pStyle->iForeColour; 
COLORREF       iBackColour = pStyle->iBackColour; 

  // call script if required
  if (m_dispidOnMXP_OpenTag != DISPID_UNKNOWN || m_bPluginProcessesOpenTag)
    {
    // dummy-up an argument list
    CString strArgument;
    CArgument * pArgument;
    POSITION pos;

    // put the arguments into the array

    for (pos = ArgumentList.GetHeadPosition (); pos; )
      {
      pArgument = ArgumentList.GetNext (pos);
      
      // empty ones we will put there by position
      if (pArgument->strName.IsEmpty ())
        strArgument += CFormat ("'%s'",
                      (LPCTSTR) pArgument->strValue);
      else
        strArgument += CFormat ("%s='%s'",
                      (LPCTSTR) pArgument->strName,
                      (LPCTSTR) pArgument->strValue);

      if (pos)
        strArgument += " ";

      }      // end of looping through each argument

    bool bNotWanted = MXP_StartTagScript (strTag, strArgument, ArgumentList);

    // re-get current style in case the script did a world.note
    pStyle = m_pCurrentLine->styleList.GetTail ();

    // put things backt to how they were
    pStyle->iFlags      = iFlags;      
    pStyle->iForeColour = iForeColour; 
    pStyle->iBackColour = iBackColour; 

    if (bNotWanted)
      return;   // they didn't want to go ahead with this tag

    }


// find current foreground and background RGB values
  GetStyleRGB (pStyle, colour1, colour2);

// special processing for Pueblo
// a tag like this: <A XCH_CMD="examine #1"> 
// will convert to a SEND tag

  if (iAction == MXP_ACTION_HYPERLINK &&
      PUEBLO_ACTIVE)
    {
    strArgument = GetArgument (ArgumentList, "xch_cmd", 0, true);
    if (!strArgument.IsEmpty ())
      {
      m_bPuebloActive = true;  // for correct newline processing
      iAction = MXP_ACTION_SEND;
      }
    }    

  // now take the action 
  switch (iAction)
    {

    // temporarily make headlines the same as bold
    case MXP_ACTION_H1: 
    case MXP_ACTION_H2: 
    case MXP_ACTION_H3: 
    case MXP_ACTION_H4: 
    case MXP_ACTION_H5: 
    case MXP_ACTION_H6: 

    case MXP_ACTION_BOLD: pStyle->iFlags |= HILITE; break;
    case MXP_ACTION_UNDERLINE: pStyle->iFlags |= UNDERLINE; break;
    case MXP_ACTION_ITALIC: pStyle->iFlags |= BLINK; break;

    case MXP_ACTION_COLOR:
         {

         pStyle->iForeColour = colour1;
         pStyle->iBackColour = colour2;
         // convert to RGB colour to start with in case only FORE or BACK supplied
         pStyle->iFlags &= ~COLOURTYPE;  // clear bits, eg. custom
         pStyle->iFlags |= COLOUR_RGB;

         // foreground colour
         strArgument = GetArgument (ArgumentList, "fore", 1, true);  // get foreground colour
         if (!m_bIgnoreMXPcolourChanges)
           if (SetColour (strArgument, pStyle->iForeColour)) 
             MXP_error (DBG_ERROR, errMXP_UnknownColour,
                        TFormat ("Unknown colour: \"%s\"" ,
                                 (LPCTSTR) strArgument));

         // background colour
         strArgument = GetArgument (ArgumentList, "back", 2, true);  // get background colour
         if (!m_bIgnoreMXPcolourChanges)
           if (SetColour (strArgument, pStyle->iBackColour)) 
             MXP_error (DBG_ERROR, errMXP_UnknownColour,
                        TFormat ("Unknown colour: \"%s\"" ,
                                 (LPCTSTR) strArgument));
         }
         break;   // end of COLOR

    case MXP_ACTION_HIGH:
         {
         CColor clr;

         pStyle->iForeColour = colour1;
         pStyle->iBackColour = colour2;
         // convert to RGB colour to start with 
         pStyle->iFlags &= ~COLOURTYPE;  // clear bits, eg. custom
         pStyle->iFlags |= COLOUR_RGB;

         clr.SetColor (colour1);
         float lum = clr.GetLuminance ();
         lum += 0.15f;
         if (lum > 1.0f)
           lum = 1.0f;
         clr.SetLuminance (lum);
         pStyle->iForeColour = clr; 
         
         }
         break;   // end of COLOR

    case MXP_ACTION_SEND: 
          // send to mud hyperlink

          pStyle->iFlags &= ~ACTIONTYPE;   // cancel old actions
          if (GetKeyword (ArgumentList, "prompt"))
            pStyle->iFlags |= ACTION_PROMPT;   // prompt action
          else
            pStyle->iFlags |= ACTION_SEND;   // send-to action

          if (m_bUnderlineHyperlinks)
            pStyle->iFlags |= UNDERLINE;   // underline it

          if (m_bUseCustomLinkColour)
            {
            // find current background RGB value
            pStyle->iForeColour = m_iHyperlinkColour;    // use hyperlink colour
            pStyle->iBackColour = colour2;
            pStyle->iFlags &= ~COLOURTYPE;  // clear bits, eg. custom
            pStyle->iFlags |= COLOUR_RGB;
            }

          strArgument = GetArgument (ArgumentList,"href", 1, false);  // get link
          if (strArgument.IsEmpty ())
            strArgument = GetArgument (ArgumentList,"xch_cmd", 1, false);  // get link
            
          strAction = strArgument;   // hyperlink
         
          strArgument = GetArgument (ArgumentList, "hint", 2, false);  // get hints
          if (strArgument.IsEmpty ())
            strArgument = GetArgument (ArgumentList,"xch_hint", 2, false);  // get hint
          
          strHint = strArgument;     // hints

          break;  // end of MXP_ACTION_SEND

    case MXP_ACTION_HYPERLINK: 
          // hyperlink

          strArgument = GetArgument (ArgumentList,"href", 1, false);  // get link
          strAction = strArgument;   // hyperlink

          pStyle->iFlags &= ~ACTIONTYPE;   // cancel old actions
          pStyle->iFlags |= ACTION_HYPERLINK | UNDERLINE;   // send-to action

          if (m_bUseCustomLinkColour)
            {
            pStyle->iForeColour = m_iHyperlinkColour;    // use hyperlink colour
            pStyle->iBackColour = colour2;
            pStyle->iFlags &= ~COLOURTYPE;  // clear bits, eg. custom
            pStyle->iFlags |= COLOUR_RGB;
            }

          break;  // end of MXP_ACTION_HYPERLINK

    case MXP_ACTION_FONT:
          {
          pStyle->iForeColour = colour1;
          pStyle->iBackColour = colour2;
          // convert to RGB colour to start with in case only FORE or BACK supplied
          pStyle->iFlags &= ~COLOURTYPE;  // clear bits, eg. custom
          pStyle->iFlags |= COLOUR_RGB;

          // eg. <FONT COLOR=Red,Blink>
          CStringList list;

          strArgument = GetArgument (ArgumentList,"color", 1, true);  // get color etc.
          if (strArgument.IsEmpty () && PUEBLO_ACTIVE)
            strArgument = GetArgument (ArgumentList,"fgcolor", 1, true);  // get color
          StringToList (strArgument, ",", list);   // break into components

          for (POSITION pos = list.GetHeadPosition (); pos; )
            {
            CString strItem = list.GetNext (pos); // get action item

            if (strItem == "blink")
               pStyle->iFlags |= BLINK;
            else
            if (strItem == "italic")
               pStyle->iFlags |= BLINK;
            else
            if (strItem == "underline")
               pStyle->iFlags |= UNDERLINE;
            else
            if (strItem == "bold")
               pStyle->iFlags |= HILITE;
            else
            if (strItem == "inverse")
               pStyle->iFlags |= INVERSE;
            else
              {  // must be colour name, yes?

              // foreground colour
              if (!m_bIgnoreMXPcolourChanges)
                if (SetColour (strItem, pStyle->iForeColour)) 
                  MXP_error (DBG_ERROR, errMXP_UnknownColour,
                              TFormat ("Unknown colour: \"%s\"" ,
                                      (LPCTSTR) strItem));
              } // end of colour

            } // end of handling each item in the list
          strArgument = GetArgument (ArgumentList,"back", 2, true);  // get back color
          if (strArgument.IsEmpty () && PUEBLO_ACTIVE)
            strArgument = GetArgument (ArgumentList,"bgcolor", 2, true);  // get back color
          // background colour

          if (!m_bIgnoreMXPcolourChanges)
            if (SetColour (strArgument, pStyle->iBackColour)) 
              MXP_error (DBG_ERROR, errMXP_UnknownColour,
                        TFormat ("Unknown colour: \"%s\"" ,
                                  (LPCTSTR) strArgument));

          // get font size argument to avoid warnings about unused arguments
          strArgument = GetArgument (ArgumentList,"size", 0, true);  // get font size
          }
          break; // end of FONT

    case MXP_ACTION_VERSION:
            {

            CString strVersion = CFormat ("\x1B[1z<VERSION MXP=\"%s\" CLIENT=MUSHclient "
                      "VERSION=\"%s\" REGISTERED=YES>%s",
                     MXP_VERSION,
                     MUSHCLIENT_VERSION,
                     ENDLINE
                     );

            SendPacket (strVersion, strVersion.GetLength ());  // send version info back
            MXP_error (DBG_INFO, infoMXP_VersionSent,
                      TFormat ("Sent version response: %s" ,
                                (LPCTSTR) strVersion.Mid (4)));

            }
          break;  // end of VERSION

    case MXP_ACTION_AFK:
          if (m_bSendMXP_AFK_Response)    // if player wants us to
            {
            strArgument = GetArgument (ArgumentList,"challenge", 1, false);  // get challenge

            // find time since last player input
            CTimeSpan ts = CTime::GetCurrentTime() - m_tLastPlayerInput;
            CString strAFK = CFormat ("\x1B[1z<AFK %ld %s>%s",
                      ts.GetTotalSeconds  (),
                      (LPCTSTR) strArgument,
                      ENDLINE
                     );

            SendPacket (strAFK, strAFK.GetLength ());  // send AFK info back
            MXP_error (DBG_INFO, infoMXP_AFKSent,
                      TFormat ("Sent AFK response: %s" ,
                                (LPCTSTR) strAFK.Mid (4)));
            } // end of AFK
          break;

    case MXP_ACTION_SUPPORT:
            {
            CString strSupports;
            CAtomicElement * pElement;
            CStringList list;
            CString strName;       

            if (ArgumentList.IsEmpty ())
              {
              for (POSITION pos = App.m_ElementMap.GetStartPosition(); pos; ) 
                {                                                
                App.m_ElementMap.GetNextAssoc (pos, strName, pElement);

                if ((pElement->iFlags & TAG_NOT_IMP) == 0)
                  {
                  strSupports += "+";
                  strSupports += pElement->strName;
                  strSupports += " ";

                  // now list the sub-items it supports
                  StringToList (pElement->strArgs, ",", list);   // break into components
                  for (POSITION argpos = list.GetHeadPosition (); argpos; )
                    {
                    CString strItem = list.GetNext (argpos); // get argument item
                    strSupports += "+";
                    strSupports += pElement->strName;
                    strSupports += ".";
                    strSupports += strItem;
                    strSupports += " ";
                    } // end of doing each sub-item
                  } // end of being implemented
                }  // end of looping through all atomic elements
              } // end of wanting complete list
            else
              {
              for (POSITION pos = ArgumentList.GetHeadPosition (); pos; )
                {
                CArgument * pArgument = ArgumentList.GetNext (pos); 
                CStringList questionlist;
                StringToList (pArgument->strValue, ".", questionlist);   // break into components

                // should be one or two words, eg. send.prompt or color
                if (questionlist.GetCount () > 2)
                  {
                  MXP_error (DBG_ERROR, errMXP_InvalidSupportArgument,
                            TFormat ("Invalid <support> argument: %s" ,
                                      (LPCTSTR) pArgument->strValue));
                  return;
                  }
                
                CString strTag =  questionlist.RemoveHead ();
                strTag.MakeLower ();

                // check valid name requested
                if (!IsValidName (strTag))
                  {
                  MXP_error (DBG_ERROR, errMXP_InvalidSupportArgument,
                            TFormat ("Invalid <support> argument: %s" ,
                                      (LPCTSTR) strTag));
                  return;
                  }

                // look up main element name

                if (!App.m_ElementMap.Lookup (strTag, pElement) ||
                   (pElement->iFlags & TAG_NOT_IMP) != 0)
                  {     // not supported
                  strSupports += "-";
                  strSupports += strTag;
                  strSupports += " ";
                  continue;   // all done for this argument
                  }

                // only one word - they aren't looking for a suboption
                if (questionlist.IsEmpty ())
                  {     // supported
                  strSupports += "+";
                  strSupports += strTag;
                  strSupports += " ";
                  continue;   // all done for this argument
                  }
                  
                CString strSubtag =  questionlist.RemoveHead ();
                strSubtag.MakeLower ();

                if (strSubtag == "*")
                  {   // they want list of options for this tag
                  // now list the sub-items it supports
                  StringToList (pElement->strArgs, ",", list);   // break into components
                  for (POSITION argpos = list.GetHeadPosition (); argpos; )
                    {
                    CString strItem = list.GetNext (argpos); // get argument item
                    strSupports += "+";
                    strSupports += pElement->strName;
                    strSupports += ".";
                    strSupports += strItem;
                    strSupports += " ";
                    } // end of doing each sub-item
                  } // end of wildcard
                else
                  {  // not wildcard - must be name
                  // check valid name requested
                  if (!IsValidName (strSubtag))
                    {
                    MXP_error (DBG_ERROR, errMXP_InvalidSupportArgument,
                              TFormat ("Invalid <support> argument: %s" ,
                                        (LPCTSTR) strSubtag));
                    return;
                    }

                  // so, see if that word is in our arguments list
                  StringToList (pElement->strArgs, ",", list);   // break into components
                  if (list.Find (strSubtag))
                    {
                    strSupports += "+";
                    strSupports += pArgument->strValue;
                    strSupports += " ";
                    }
                  else
                    {
                    strSupports += "-";
                    strSupports += pArgument->strValue;
                    strSupports += " ";
                    }
                  }    // end of not looking for wildcard
                } // end of doing each argument

              } // find individual items

            CString strMessage = CFormat ("\x1B[1z<SUPPORTS %s>%s",
                                          (LPCTSTR) strSupports,
                                          ENDLINE);

            SendPacket (strMessage, strMessage.GetLength ());  // send version info back
            MXP_error (DBG_INFO, infoMXP_SupportsSent,
                      TFormat ("Sent supports response: %s" ,
                                (LPCTSTR) strMessage.Mid (4)));

            }
          bIgnoreUnusedArgs = true;

          break;  // end of MXP_ACTION_SUPPORT

    case MXP_ACTION_OPTION:
            {
            CString strOptions;
            CStringList list;
            CString strName;       

            if (ArgumentList.IsEmpty ())
              {

              for (long i = 0; OptionsTable [i].pName; i++)
                {
                char * pName = OptionsTable [i].pName;
                strOptions += CFormat ("%s=%ld ",
                               pName, 
                               (LPCTSTR) GetOptionItem (i));
                }

              } // end of wanting complete list
            else
              {
              for (POSITION pos = ArgumentList.GetHeadPosition (); pos; )
                {
                CArgument * pArgument = ArgumentList.GetNext (pos); 

                strOptions += CFormat ("%s=%ld",
                               (LPCTSTR) pArgument->strValue, 
                               (LPCTSTR) GetOption (pArgument->strValue));

                } // end of doing each argument

              } // find individual items

            CString strMessage = CFormat ("\x1B[1z<OPTIONS %s>%s",
                                          (LPCTSTR) strOptions,
                                          ENDLINE);

            SendPacket (strMessage, strMessage.GetLength ());  // send version info back
            MXP_error (DBG_INFO, infoMXP_OptionsSent,
                      TFormat ("Sent options response: %s" ,
                                (LPCTSTR) strMessage.Mid (4)));

            }
          bIgnoreUnusedArgs = true;

          break;  // end of MXP_ACTION_OPTION

    case MXP_ACTION_RECOMMEND_OPTION:
          if (m_bMudCanChangeOptions)
            {
            CString strOptions;
            CStringList list;
            CString strName;       

            for (POSITION pos = ArgumentList.GetHeadPosition (); pos; )
              {
              CArgument * pArgument = ArgumentList.GetNext (pos); 

              int iItem;
              int iResult = FindBaseOption (pArgument->strName, OptionsTable, iItem);

              if (iResult != eOK)
                MXP_error (DBG_ERROR, errMXP_InvalidOptionArgument,
                          TFormat ("Option named '%s' not known.",
                          (LPCTSTR) pArgument->strName));      
              else if (!(OptionsTable [iItem].iFlags & OPT_SERVER_CAN_WRITE))
                MXP_error (DBG_ERROR, errMXP_CannotChangeOption,
                          TFormat ("Option named '%s' cannot be changed.",
                          (LPCTSTR) pArgument->strName));      
              else
                {
                iResult = SetOptionItem (iItem, atol (pArgument->strValue), true, false);
                if (iResult == eOK)
                  MXP_error (DBG_INFO, infoMXP_OptionChanged,
                            TFormat ("Option named '%s' changed to '%s'.",
                            (LPCTSTR) pArgument->strName,
                            (LPCTSTR) pArgument->strValue)); 
                else
                  MXP_error (DBG_ERROR, errMXP_OptionOutOfRange,
                            TFormat ("Option named '%s' could not be changed to '%s' (out of range).",
                            (LPCTSTR) pArgument->strName,
                            (LPCTSTR) pArgument->strValue));      
                }

              } // end of doing each argument

            }
          bIgnoreUnusedArgs = true;

          break;  // end of MXP_ACTION_RECOMMEND_OPTION


    case MXP_ACTION_USER:
            if (!m_name.IsEmpty () && 
                m_connect_now == eConnectMXP)
              {
              CString strPacket = m_name + ENDLINE;
              SendPacket (strPacket, strPacket.GetLength ());  // send name to MUD
              MXP_error (DBG_INFO, infoMXP_CharacterNameSent,
                          TFormat ("Sent character name: %s" ,
                                  (LPCTSTR) m_name));      
              }
            else if (m_connect_now != eConnectMXP)
              MXP_error (DBG_WARNING, wrnMXP_CharacterNameRequestedButNotDefined,
                        Translate ("Character name requested but auto-connect not set to MXP."));      
            else
              MXP_error (DBG_WARNING, wrnMXP_CharacterNameRequestedButNotDefined,
                        Translate ("Character name requested but none defined."));      
            break;  // end of USER

    case MXP_ACTION_PASSWORD:
            if (m_nTotalLinesSent > 10)     // security check
              MXP_error (DBG_WARNING, wrnMXP_PasswordNotSent,
                        "Too many lines sent to MUD - password not sent.");      
            else
            if (!m_password.IsEmpty () && 
                m_connect_now == eConnectMXP)
              {
              CString strPacket = m_password + ENDLINE;
              SendPacket (strPacket, strPacket.GetLength ());  // send password to MUD
              MXP_error (DBG_INFO, infoMXP_PasswordSent,
                        "Sent password to world.");      
              }
            else if (m_connect_now != eConnectMXP)
              MXP_error (DBG_WARNING, wrnMXP_PasswordRequestedButNotDefined,
                        "Password requested but auto-connect not set to MXP.");      
            else
              MXP_error (DBG_WARNING, wrnMXP_PasswordRequestedButNotDefined,
                        "Password requested but none defined.");      
            break;  // end of PASSWORD

         // new para
    case MXP_ACTION_P:
          // experimental
          m_cLastChar = 0;
          m_bInParagraph = true;      
          break;  // end of MXP_ACTION_P
    
          // new line
    case MXP_ACTION_BR:
          bIgnoreUnusedArgs = true; // don't worry about args for now :)

          StartNewLine (true, 0);
          SetNewLineColour (0);
          break;  // end of MXP_ACTION_BR

          // reset
    case MXP_ACTION_RESET:
          MXP_Off ();
          break;  // end of MXP_ACTION_RESET

          // MXP options  (MXP OFF, MXP DEFAULT_OPEN, MXP DEFAULT_SECURE etc.
    case MXP_ACTION_MXP:
          
          if (GetKeyword (ArgumentList, "off"))
            MXP_Off (true);

          /*
          if (GetKeyword (ArgumentList, "default_open"))
            {
            MXP_error (DBG_INFO, "MXP default mode now OPEN.");
            m_iMXP_defaultMode = eMXP_open;
            }  // end of DEFAULT_OPEN

          if (GetKeyword (ArgumentList, "default_secure"))
            {
            MXP_error (DBG_INFO, "MXP default mode now SECURE.");
            m_iMXP_defaultMode = eMXP_secure;
            }  // end of DEFAULT_SECURE

          if (GetKeyword (ArgumentList, "default_locked"))
            {
            MXP_error (DBG_INFO, "MXP default mode now LOCKED.");
            m_iMXP_defaultMode = eMXP_locked;
            }  // end of DEFAULT_LOCKED


          if (GetKeyword (ArgumentList, "use_newlines"))
            {
            MXP_error (DBG_INFO, "Now interpreting newlines as normal.");
            m_bInParagraph = false;      
            }   // end of USE_NEWLINES

          if (GetKeyword (ArgumentList, "ignore_newlines"))
            {
            MXP_error (DBG_INFO, "Now ignoring newlines.");
            m_bInParagraph = true;      
            }   // end of IGNORE_NEWLINES

          */

          break;  // end of MXP_ACTION_MXP

    case MXP_ACTION_SCRIPT:
          MXP_error (DBG_INFO, infoMXP_ScriptCollectionStarted,
                      "Script collection mode entered (discarding script).");
          m_bMXP_script = true;
          break;  // end of MXP_ACTION_SCRIPT

    case MXP_ACTION_HR: 

          {
          // wrap up previous line if necessary
          if (m_pCurrentLine->len > 0)
             StartNewLine (true, 0);

          /*
          CString strLine;
          char * p = strLine.GetBuffer (m_nWrapColumn);
          memset (p, 175, m_nWrapColumn);
          strLine.ReleaseBuffer (m_nWrapColumn);
          AddToLine (strLine, 0);
          */
          // mark line as HR line
          m_pCurrentLine->flags = HORIZ_RULE;
          
          StartNewLine (true, 0); // now finish this line
          }
          break;  // end of MXP_ACTION_HR

    case MXP_ACTION_PRE: 
          m_bPreMode = true;
          break;  // end of MXP_ACTION_PRE

     case MXP_ACTION_UL:   
          m_iListMode = eUnorderedList;
          m_iListCount = 0;
          break;  // end of MXP_ACTION_UL
     case MXP_ACTION_OL:   
          m_iListMode = eOrderedList;
          m_iListCount = 0;
          break;  // end of MXP_ACTION_OL
     case MXP_ACTION_LI:   
         {
          // wrap up previous line if necessary
          if (m_pCurrentLine->len > 0)
             StartNewLine (true, 0);
          CString strListItem = " * ";
          if (m_iListMode == eOrderedList)
            strListItem.Format (" %i. ", ++m_iListCount);
          AddToLine (strListItem, 0);
          }
          break;  // end of MXP_ACTION_LI

    // pueblo tags we put here so we don't get warnings

      case MXP_ACTION_BODY : bIgnoreUnusedArgs = true; break; // just ignore it
      case MXP_ACTION_HEAD : bIgnoreUnusedArgs = true; break; // just ignore it
      case MXP_ACTION_HTML : bIgnoreUnusedArgs = true; break; // just ignore it
      case MXP_ACTION_TITLE: bIgnoreUnusedArgs = true; break; // just ignore it
      case MXP_ACTION_SAMP : bIgnoreUnusedArgs = true; break; // just ignore it
      case MXP_ACTION_CENTER : bIgnoreUnusedArgs = true; break; // just ignore it
      case MXP_ACTION_XCH_PANE : bIgnoreUnusedArgs = true; break; // just ignore it

      case MXP_ACTION_IMG  : 
      case MXP_ACTION_IMAGE:

        {
          GetKeyword (ArgumentList, "ismap"); // make sure we realise it is a keyword

          // detect newline treatment
          strArgument = GetArgument (ArgumentList,"xch_mode", 0, false);  // get mode
          if (!strArgument.IsEmpty ())
            {
            m_bPuebloActive = true;  // for correct newline processing
            if (strArgument.CompareNoCase ("purehtml") == 0)
               m_bSuppressNewline = true;
            else
            if (strArgument.CompareNoCase ("html") == 0)
               m_bSuppressNewline = false;
            } // end of some sort of Pueblo

          strArgument = GetArgument (ArgumentList,"url", 0, false);  // get link
          if (strArgument.IsEmpty () && PUEBLO_ACTIVE)   
            strArgument = GetArgument (ArgumentList,"src", 0, false);  // get link

          CString strFilename = GetArgument (ArgumentList,"fname", 0, false); // and file name

          if (!strArgument.IsEmpty ())
            {

            CString strOldAction = strAction;
            int iFlags = pStyle->iFlags;
            COLORREF iForeColour = pStyle->iForeColour;
            COLORREF iBackColour = pStyle->iBackColour;

            // ensure on new line
            if (m_pCurrentLine->len > 0)
               StartNewLine (true, 0);

            // starting a new line may have deleted pStyle

            pStyle = m_pCurrentLine->styleList.GetTail ();

            if (m_bUseCustomLinkColour)
              {
              pStyle->iForeColour = m_iHyperlinkColour;    // use hyperlink colour
              pStyle->iBackColour = colour2;
              pStyle->iFlags &= ~COLOURTYPE;  // clear bits, eg. custom
              pStyle->iFlags |= COLOUR_RGB;
              }

            strArgument += strFilename;   // append filename to URL
            strAction = strArgument;   // hyperlink
            pStyle->iFlags &= ~ACTIONTYPE;   // cancel old actions
            pStyle->iFlags |= ACTION_HYPERLINK;   // send-to action

            if (m_bUnderlineHyperlinks)
              pStyle->iFlags |= UNDERLINE;   // send-to action

            AddToLine ("[", 0);          
            AddToLine (strArgument, 0);
            AddToLine ("]", 0);

            // have to add the action now, before we start a new line
            pStyle->pAction = GetAction (strAction, strHint, strVariable);
            strAction.Empty ();

            StartNewLine (true, 0);   // new line after image tag
            // go back to old style (ie. lose the underlining)
            AddStyle (iFlags, 
                     iForeColour, 
                     iBackColour, 
                     0, 
                     strOldAction);

            }
        }
        break; // end of MXP_ACTION_IMG

    case MXP_ACTION_XCH_PAGE:
         bIgnoreUnusedArgs = true;
         m_bPuebloActive = true;  // for correct newline processing
         MXP_Off ();    // same as <reset>?
      break;  // end of MXP_ACTION_XCH_PAGE

    case MXP_ACTION_VAR: 
          // set variable

          strVariable = GetArgument (ArgumentList,"", 1, false);  // get name

          // case insensitive
          strVariable.MakeLower ();

          if (!IsValidName (strVariable))
            {
            MXP_error (DBG_ERROR, errMXP_InvalidDefinition,
                      TFormat ("Invalid MXP entity name: <!%s>", 
                      (LPCTSTR) strVariable)); 
            strVariable.Empty ();
            return;
            }

            { // protect local variable
            CString strEntityContents;

            if (App.m_EntityMap.Lookup (strVariable, strEntityContents))
              {
              MXP_error (DBG_ERROR, errMXP_CannotRedefineEntity,
                        TFormat ("Cannot redefine entity: &%s;", 
                        (LPCTSTR) strVariable)); 
              strVariable.Empty ();
              return;
              }
              }

          break;  // end of MXP_ACTION_VAR


    default:
          {
          // warn them it is not implemented
          MXP_error (DBG_WARNING, wrnMXP_TagNotImplemented,
                     TFormat ("MXP tag <%s> is not implemented" ,
                             (LPCTSTR) strTag));
          }   // end of default

    } // end of switch on iAction

  if (!bIgnoreUnusedArgs)
    CheckArgumentsUsed (strTag, ArgumentList);

  } // end of CMUSHclientDoc::MXP_OpenAtomicTag

