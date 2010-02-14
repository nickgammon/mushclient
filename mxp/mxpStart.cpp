// mxpStart.cpp - MXP start tag

// eg. <send ...>

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


#define OUTSTANDING_TAG_WARNING 100  // warn every so many tags unclosed

// make false for debugging

#define SECURE_ELEMENT_CHECK true


// here for start tag, eg. <bold> <underline> <usertag>
void CMUSHclientDoc::MXP_StartTag (CString strTag)
  {
// are we in secure mode right now?
bool bSecure = MXP_Secure ();
bool bNoReset = false;

  MXP_Restore_Mode ();  // cancel secure-once mode

static CArgumentList ArgumentList;

CString strName;

  GetWord (strName, strTag);

  // count them
  m_iMXPtags++;

  if (!IsValidName (strName))
    {
    MXP_error (DBG_ERROR, errMXP_InvalidElementName,
                TFormat ("Invalid MXP element name \"%s\" supplied.",
                strName)); 
    return;
    }

  // case insensitive?
  strName.MakeLower ();

// see if we know of this element

CAtomicElement * pAtomicElement = NULL;
CElement * pElement = NULL;
bool bOpen;
bool bCommand;
POSITION atompos;

// find existing styles

CStyle * pStyle = m_pCurrentLine->styleList.GetTail ();

unsigned short iFlags = pStyle->iFlags;      
COLORREF       iForeColour = pStyle->iForeColour; 
COLORREF       iBackColour = pStyle->iBackColour; 
CAction *      pAction = pStyle->pAction;

CString strAction;    
CString strHint;      
CString strVariable;  

  // get old action, hint etc. so that something like:
  // <send href="nick"> <b> blah </b> </send> will work
  // in this case we want the href (action) to persist through the <b>

  if (pAction)
    {
    strAction = pAction->m_strAction;
    strHint = pAction->m_strHint;
    strVariable = pAction->m_strVariable;
    } // end of having an action etc.


  if (App.m_ElementMap.Lookup (strName, pAtomicElement))
    {
    bOpen    = (pAtomicElement->iFlags & TAG_OPEN) != 0;
    bCommand = (pAtomicElement->iFlags & TAG_COMMAND) != 0;
    bNoReset = (pAtomicElement->iFlags & TAG_NO_RESET) != 0;

    // check for mixing Pueblo and MXP tags

/*

  // ALLOW BOTH for now

    if ((pAtomicElement->iFlags & TAG_PUEBLO) &&
        !m_bPuebloActive)
      {
      MXP_error (DBG_ERROR, errMXP_PuebloOnly,
                TFormat ("Using Pueblo-only element in MXP mode: <%s>" ,
                          (LPCTSTR) strName));
      return;
      }
    
    if ((pAtomicElement->iFlags & TAG_MXP) &&
        m_bPuebloActive)
      {
      MXP_error (DBG_ERROR, errMXP_MXPOnly,
                TFormat ("Using MXP-only element in Pueblo mode: <%s>" ,
                          (LPCTSTR) strName));
      return;
      }

*/

    } // end of atomic element found
  else
    {
    if (!m_CustomElementMap.Lookup (strName, pElement))
      {
      MXP_error (DBG_ERROR, errMXP_UnknownElement,
                 TFormat ("Unknown MXP element: <%s>" ,
                          (LPCTSTR) strName));
      return;
      }
    pAtomicElement = NULL;
    bOpen    = pElement->bOpen;
    bCommand = pElement->bCommand;
    if (!pElement->strFlag.IsEmpty ())
      strVariable = pElement->strFlag;  // might have a variable to set
    } // end of not atomic


  // check for secure tags
  if (!bOpen && 
      !bSecure &&
      SECURE_ELEMENT_CHECK)
    {
    MXP_error (DBG_ERROR, errMXP_ElementWhenNotSecure,
                TFormat ("Secure MXP tag ignored when not in secure mode: <%s>" ,
                        (LPCTSTR) strName));
    return;
    }


  if (BuildArgumentList (ArgumentList, strTag))
    {
    DELETE_LIST (ArgumentList);
    return;
    }

  // call script if required for user-defined elements
  // atomic elements have their script called in MXP_OpenAtomicTag

  if ((m_dispidOnMXP_OpenTag != DISPID_UNKNOWN || m_bPluginProcessesOpenTag)
        && pAtomicElement == NULL)
    {
    bool bNotWanted = MXP_StartTagScript (strName, strTag, ArgumentList);

    // re-get current style in case the script did a world.note
    pStyle = m_pCurrentLine->styleList.GetTail ();

    // put things backt to how they were
    pStyle->iFlags      = iFlags;      
    pStyle->iForeColour = iForeColour; 
    pStyle->iBackColour = iBackColour; 

    if (bNotWanted)
      return;   // they didn't want to go ahead with this tag

    }

// If existing run is zero length, get rid of it, unless it
// is a tag marker

  if (pStyle->iLength == 0 && (pStyle->iFlags & START_TAG) == 0)
    {
    DELETESTYLE (pStyle);
    m_pCurrentLine->styleList.RemoveTail ();
    }

  // command tags are not popped from the stack, and thus don't need a record

  if (!bCommand)
    {
  // add a marker to the current line for the tag itself
  // this is a record of what the text style was at this point

    AddStyle (iFlags | START_TAG, 
              iForeColour, 
              iBackColour, 
              0, 
              strName,
              "",
              strVariable);

    // remember what is outstanding

    CActiveTag * pTag = new CActiveTag;

    pTag->strName = strName;
    pTag->bSecure = bSecure;
    pTag->bNoReset = bNoReset;
    m_ActiveTagList.AddTail (pTag);  // add to outstanding tag list

    // warn if they are overdoing the outstanding tags
    if (m_ActiveTagList.GetCount () % OUTSTANDING_TAG_WARNING == 0 &&
        m_ActiveTagList.GetCount () != m_iLastOutstandingTagCount)
      {
      MXP_error (DBG_WARNING, wrnMXP_ManyOutstandingTags,
                  TFormat (
                          "Now have %i outstanding MXP tags" ,
                          m_ActiveTagList.GetCount ()));
      m_iLastOutstandingTagCount = m_ActiveTagList.GetCount ();
      }


    } // end of not command tag

//if (strName == "ex")
  /*
  {

  iFlags &= ~COLOURTYPE;  // clear bits, eg. custom
  iFlags |= COLOUR_RGB;

  iForeColour = 255;
  iBackColour = 255 * 256;
  }
  */

// now add another style entry to the line - this will have the
// style adjusted for the new attributes (eg. bold)

CStyle * pNewStyle = AddStyle (iFlags & STYLE_BITS, 
                               iForeColour, 
                               iBackColour, 
                               0, 
                               NULL);  // we will add the action later

  
  // atomic element?  (looked-up earlier)
  if (pAtomicElement)
    {
    CArgument * pArgument;

    for (atompos = ArgumentList.GetHeadPosition (); atompos; )
      {
      pArgument = ArgumentList.GetNext (atompos);

      // Walk the value for &xxx; entries

      if (pArgument->strValue.Find ('&') != -1)
        {

        const char * p = pArgument->strValue;
        const char * pStart = pArgument->strValue;
        CString strEntity;
        CString strFixedValue;
        strFixedValue.Empty ();
        long length;

        for ( ; *p; p++)
          {
          if (*p == '&')
            {

            // copy up to ampersand
            length = p - pStart;
      
            if (length > 0)
              strFixedValue += CString (pStart, length);

            p++;    // skip ampersand
            pStart = p; // where entity starts
            for ( ; *p && *p != ';'; p++) // look for closing semicolon
              ; // just keep looking
            if (*p != ';')
              {
              MXP_error (DBG_ERROR, errMXP_NoClosingSemicolonInArgument,
                        TFormat ("No closing \";\" in MXP element argument \"%s\"", 
                        (LPCTSTR) pArgument->strValue)); 
              return;
              }

            strEntity = CString (pStart, p - pStart);   // build entity, excluding & and ;

            // b. i. Look up entity in attribute list by name (eg. "col")

            CString strReplacement = "&text;";

            if (strEntity != "text")
              strReplacement = MXP_GetEntity (strEntity);
            strFixedValue += strReplacement;    // add to list

            pStart = p + 1;   // move on past the entity
            
            } // end of having an ampersand 

          } // end of processing the value

        strFixedValue += pStart;

        pArgument->strValue = strFixedValue;
        } // end of subsitution needed
      } // end of processing each argument in the atomic list

    MXP_OpenAtomicTag (strName,
                       pAtomicElement->iAction, 
                       pNewStyle,
                       strAction,
                       strHint,
                       strVariable,
                       ArgumentList);

    // new style might have changed if they started a new line
    // (eg. BR)
    if (!m_pCurrentLine->styleList.IsEmpty ())
      {
      pNewStyle = m_pCurrentLine->styleList.GetTail ();
      RememberStyle (pNewStyle);
      if (pNewStyle->pAction)
        {
        strAction = pNewStyle->pAction->m_strAction;
        strHint =  pNewStyle->pAction->m_strHint;
        strVariable = pNewStyle->pAction->m_strVariable;
        }
      }

    pNewStyle->pAction = GetAction (strAction, strHint, strVariable);

    DELETE_LIST (ArgumentList);  // clean up memory
    return;
    }

// --------- end of processing for ATOMIC element ------------

// must be a user-defined element

CElementItem * pElementItem;

  for (POSITION pos = pElement->ElementItemList.GetHeadPosition (); pos; )
    {
    pElementItem = pElement->ElementItemList.GetNext (pos);

    CArgumentList BuiltArgumentList;
    BuiltArgumentList.RemoveAll ();

    /* we need to build up the arguments to the atomic element from 3 places:

    1. The atom itself needs an argument list (eg. <COLOR &col;> )
    2. The user-defined element has an attribute list that lists possible arguments
       possibly with defaults, eg. col=red
    3. The tag itself may supply values, eg. col=blue

    These are:
       1.  pElementItem->ArgumentList     - what the atomic element wants
       2.  pElement->AttributeList        - what it can get, including defaults
       3.  ArgumentList                   - what is specified on *this* tag

    So, I think we need to:
     
       a. Walk the atomic item's argument list
       b. For any entries in the form &xxx; ...
           0. The special case &text; is left alone (for processing later)
           i. Look up the entity in the attribute list (2) above. 
          ii. If found, go to (c).
         iii. Look up entity in entities map (could be &lt; for instance)
          iv. If found, just replace it and continue
           v. If not found still, error
       c. If entity is found in attribute list (2) above, then:
       d. Look up value in supplied arguments (3) by name and position.
       e. If found, take supplied value.
       f. If not found, take default from (2) including <nothing> if applicable.

     Each entry gets built into BuiltArgumentList and passed down to the atomic element
     processing routine.

    */

    // a. Walk the atom's list
    CArgument * pArgument;
    int iArgumentNumber = 0;

    for (atompos = pElementItem->ArgumentList.GetHeadPosition (); atompos; )
      {
      pArgument = pElementItem->ArgumentList.GetNext (atompos);

      // b. Walk the value for &xxx; entries

      const char * p = pArgument->strValue;
      const char * pStart = pArgument->strValue;
      CString strEntity;
      CString strFixedValue;
      long length;

      strFixedValue.Empty ();

      for ( ; *p; p++)
        {
        if (*p == '&')
          {

          // copy up to ampersand
          length = p - pStart;
    
          if (length > 0)
            strFixedValue += CString (pStart, length);

          p++;    // skip ampersand
          pStart = p; // where entity starts
          for ( ; *p && *p != ';'; p++) // look for closing semicolon
            ; // just keep looking
          if (*p != ';')
            {
            MXP_error (DBG_ERROR, errMXP_NoClosingSemicolonInArgument,
                      TFormat ("No closing \";\" in MXP element argument \"%s\"", 
                      (LPCTSTR) pArgument->strValue)); 
            return;
            }

          strEntity = CString (pStart, p - pStart);   // build entity, excluding & and ;

          // b. i. Look up entity in attribute list by name (eg. "col")

          CString strReplacement = "&text;";

          if (strEntity != "text")
            {

            CString strDefault;

            CArgument * pAttribute = NULL;
            int iSequence = 1;

            for (POSITION attpos = pElement->AttributeList.GetHeadPosition (); 
                 attpos; 
                 iSequence++)
              {
              pAttribute = pElement->AttributeList.GetNext (attpos);
              if (pAttribute->strName.IsEmpty ())  // no name, value is name (ie. no default)
                {
                if (pAttribute->strValue.CompareNoCase (strEntity) == 0)
                  break;
                }
              else
                if (pAttribute->strName.CompareNoCase (strEntity) == 0)
                  {
                  strDefault = pAttribute->strValue;
                  break;
                  }
              pAttribute = NULL;   // indicates it wasn't found
              }  // end of looking for the attribute

            if (pAttribute)
              {

              // we now have a default and a position - look it up in the supplied arguments

              strReplacement = GetArgument (ArgumentList, strEntity, iSequence, false); 
              if (strReplacement.IsEmpty ())   // empty? take default
                {
                strReplacement = strDefault; 

                // stil empty? Warn them.
                if (strReplacement.IsEmpty ())
                  MXP_error (DBG_WARNING, wrnMXP_ArgumentNotSupplied,
                             TFormat ("Non-default argument \"%s\" not supplied to <%s>", 
                            (LPCTSTR) strEntity,
                            (LPCTSTR) strName
                            ));     
                }

              } // end of attribute found
            else
              strReplacement = MXP_GetEntity (strEntity);
            } // end of not being the entity &text;
          strFixedValue += strReplacement;    // add to list

          pStart = p + 1;   // move on past the entity
          
          } // end of having an ampersand 

        } // end of processing the value

      strFixedValue += pStart;

      // add fixed argument to the built argument list
      CArgument * pNewArgument;

      if (pArgument->strName.IsEmpty ())  // just a positional argument
        {
        pNewArgument = new CArgument ("", strFixedValue, ++iArgumentNumber);
        BuiltArgumentList.AddTail (pNewArgument);
        }
      else
        {
        pNewArgument = new CArgument (pArgument->strName, strFixedValue, 0);
        BuiltArgumentList.AddTail (pNewArgument);
        }


      } // end of processing each argument in the atomic list

    MXP_OpenAtomicTag (pElementItem->pAtomicElement->strName,
                       pElementItem->pAtomicElement->iAction, 
                       pNewStyle,
                       strAction,
                       strHint,
                       strVariable,
                       BuiltArgumentList); 

    DELETE_LIST (BuiltArgumentList);  // just a temporary list
    } // end of doing each atomic element

  // new style might have changed if they started a new line
  // (eg. BR)
  if (!m_pCurrentLine->styleList.IsEmpty ())
    {
    pNewStyle = m_pCurrentLine->styleList.GetTail ();
    RememberStyle (pNewStyle);
    }

  // make an action for the built-up action/hint/variable
  pNewStyle->pAction = GetAction (strAction, strHint, strVariable);

// check all arguments used

  CheckArgumentsUsed (strName, ArgumentList);

  DELETE_LIST (ArgumentList);  // clean up memory

  } // end of CMUSHclientDoc::MXP_StartTag


bool CMUSHclientDoc::MXP_StartTagScript  (const CString & strName, 
                     const CString & strArguments,
                     CArgumentList & ArgumentList)
  {

  // don't make it too easy to dummy up AFK replies
  if (strName == "afk")
    return false;

  // tell each plugin what we have received
  for (POSITION pluginpos = m_PluginList.GetHeadPosition(); pluginpos; )
    {
    CPlugin * pPlugin = m_PluginList.GetNext (pluginpos);

    if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
      continue;

    // see what the plugin makes of this,
    if (!pPlugin->ExecutePluginScript (ON_PLUGIN_MXP_OPENTAG, 
                          pPlugin->m_dispid_plugin_OnMXP_OpenTag, 
                          CFormat ("%s,%s",
                          (LPCTSTR) strName,
                          (LPCTSTR) strArguments)
                          ))
      {
      m_CurrentPlugin = NULL;
      return true;    
      }
    }   // end of doing each plugin
  m_CurrentPlugin = NULL;

  // see if main script wants to do anything
  if (m_dispidOnMXP_OpenTag == DISPID_UNKNOWN)
    return false;

  long nInvocationCount = 0;
  long iCount = ArgumentList.GetCount ();

  CString strType = "MXP open tag";
  CString strReason =  TFormat ("opening MXP tag %s", (LPCTSTR) strName);

  if (GetScriptEngine () && GetScriptEngine ()->IsLua ())
    {
    list<double> nparams;
    list<string> sparams;
    sparams.push_back ((LPCTSTR) strName);    // name of tag
    sparams.push_back ((LPCTSTR) strArguments);  // all arguments

    map <string, string> table;

    CArgument * pArgument;
    POSITION pos;

    // put the arguments into the table

    for (iCount = 0, pos = ArgumentList.GetHeadPosition (); pos; iCount++)
      {
      pArgument = ArgumentList.GetNext (pos);
      CString strName = pArgument->strName;

      // empty ones we will put there by position
      if (strName.IsEmpty ())
        strName = CFormat ("%i",
                      pArgument->iPosition);
      
      table [(LPCTSTR) strName] = pArgument->strValue;
      }      // end of looping through each argument

    bool result;
    GetScriptEngine ()->ExecuteLua (m_dispidOnMXP_OpenTag, 
                                   m_strOnMXP_OpenTag, 
                                   eWorldAction,
                                   strType, 
                                   strReason, 
                                   nparams,
                                   sparams, 
                                   nInvocationCount,
                                   NULL,
                                   &table,
                                   NULL,
                                   &result);
    return result;
    }   // end of Lua

  COleSafeArray sa;   // for wildcard list

  if (iCount) // cannot create empty array dimension
    {
    sa.CreateOneDim (VT_VARIANT, iCount);

    CArgument * pArgument;
    POSITION pos;

    // put the arguments into the array

    for (iCount = 0, pos = ArgumentList.GetHeadPosition (); pos; iCount++)
      {
      pArgument = ArgumentList.GetNext (pos);

      // the array must be a bloody array of variants, or VBscript kicks up
      COleVariant v;
      
      // empty ones we will put there by position
      if (pArgument->strName.IsEmpty ())
        v = CFormat ("%i=%s",
                      pArgument->iPosition,
                      (LPCTSTR) pArgument->strValue);
      else
        v = CFormat ("%s=%s",
                      (LPCTSTR) pArgument->strName,
                      (LPCTSTR) pArgument->strValue);
      sa.PutElement (&iCount, &v);
      }      // end of looping through each argument
    } // end of having at least one

  // WARNING - arguments should appear in REVERSE order to what the sub expects them!

  enum
    {
    eArgumentArray,
    eArguments,
    eTagName,
    eArgCount,     // this MUST be last
    };    

  COleVariant args [eArgCount];
  DISPPARAMS params = { args, NULL, eArgCount, 0 };

  args [eTagName] = strName;
  args [eArguments] = strArguments;
  args [eArgumentArray] = sa;

  COleVariant result;

  ExecuteScript (m_dispidOnMXP_OpenTag,  
                 m_strOnMXP_OpenTag,
                 eWorldAction,
                 strType, 
                 strReason,
                 params, 
                 nInvocationCount,
                 &result); 

  // if the function returns a non-zero result, don't go ahead
  if (result.vt != VT_EMPTY)
    {
    result.ChangeType (VT_I4);  // make a long
    if (result.vt == VT_I4)   // conversion successful
      if (result.lVal)        // return if non-zero
        return true;
    }

  return false;
  } // end of CMUSHclientDoc::MXP_StartTagScript 
