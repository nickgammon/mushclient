// mxpDefs.cpp - MXP definitions

// <!ELEMENT ...>
// <!ATTLIST ...>
// <!ENTITY ...>

#include "stdafx.h"
#include "..\MUSHclient.h"

#include "..\doc.h"
#include "..\MUSHview.h"
#include "..\mainfrm.h"
#include "mxp.h"
#include "..\scripting\errors.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// make false for debugging

#define DEFINITIONS_MUST_BE_SECURE true

// handle definition-style tag, eg. <!ELEMENT blah> or <!ENTITY blah>

void CMUSHclientDoc::MXP_Definition (CString strTag)
  {
bool bSecure = MXP_Secure ();

  MXP_Restore_Mode ();  // cancel secure-once mode

// check in secure mode
if (DEFINITIONS_MUST_BE_SECURE &&
    !bSecure)
  {
  MXP_error (DBG_ERROR, errMXP_DefinitionWhenNotSecure,
             TFormat ("MXP definition ignored when not in secure mode: <!%s>" ,
                      (LPCTSTR) strTag));
  return;
  }


CString strDefinition;

// get first word (eg. ELEMENT or ENTITY)

  GetWord (strDefinition, strTag);

  if (!IsValidName (strDefinition))
    {
    MXP_error (DBG_ERROR, errMXP_InvalidDefinition,
              TFormat ("Invalid MXP definition name: <!%s>", 
              (LPCTSTR) m_strMXPstring)); 
    return;
    }
  
  strDefinition.MakeLower (); // case-insensitive?

// get name of what we are defining

CString strName;

  GetWord (strName, strTag);

  if (!IsValidName (strName))
    {
    MXP_error (DBG_ERROR, errMXP_InvalidElementName,
              TFormat ("Invalid MXP element/entity name: \"%s\"", 
              (LPCTSTR) strName)); 
    return;
    }
  
/*
  if (m_bPuebloActive)
    {
    MXP_error (DBG_ERROR, errMXP_DefinitionAttemptInPueblo,
              TFormat ("Defining elements/entities not valid in Pueblo: <%s>", 
              (LPCTSTR) strName)); 
    return;  
    }

*/

  // debugging
  MXP_error (DBG_INFO, msgMXP_GotDefinition,
              TFormat ("Got Definition: !%s %s %s", 
                      (LPCTSTR) strDefinition, 
                      (LPCTSTR) strName,
                      (LPCTSTR) strTag)); 

  if (strDefinition == "element" ||
      strDefinition == "el")
     MXP_Element (strName, strTag);
  else
  if (strDefinition == "entity" ||
      strDefinition == "en")
     MXP_Entity (strName, strTag);
  else
  if (strDefinition == "attlist" ||
      strDefinition == "at")
     MXP_Attlist (strName, strTag);
  else
    MXP_error (DBG_ERROR, errMXP_InvalidDefinition,
              TFormat ("Unknown definition type: <!%s>", 
              (LPCTSTR) strDefinition)); 
    
  } // end of CMUSHclientDoc::MXP_Definition

// here for <!ELEMENT blah>
void CMUSHclientDoc::MXP_Element (CString strName, CString strTag)
  {
static CArgumentList ArgumentList;

  // get arguments to !ELEMENT definition
  if (BuildArgumentList (ArgumentList, strTag))
    {
    DELETE_LIST (ArgumentList);
    return;
    }

CElement * pElement;
bool bDelete = GetKeyword (ArgumentList, "delete");

  strName.MakeLower (); // case-insensitive?

  // see if we know of this atom

  CAtomicElement * element_item;

  if (App.m_ElementMap.Lookup (strName, element_item))
    {
    MXP_error (DBG_ERROR, errMXP_CannotRedefineElement,
               TFormat ("Cannot redefine built-in MXP element: <%s>" ,
                        (LPCTSTR) strName));
    return;
    }

// if element already defined, delete old one
  if (m_CustomElementMap.Lookup (strName, pElement))
    {
    if (!bDelete)
      MXP_error (DBG_WARNING, wrnMXP_ReplacingElement, 
                 TFormat ("Replacing previously-defined MXP element: <%s>", 
                (LPCTSTR) strName)); 
    DELETE_LIST (pElement->ElementItemList);
    DELETE_LIST (pElement->AttributeList);
    delete pElement;
    } // end of existing element

  if (bDelete)
    return; // all done!

// add new element to map
m_CustomElementMap.SetAt (strName, pElement = new CElement);

  pElement->strName = strName;

// get keywords first, so we won't mistake them for arguments 
// (eg. so OPEN doesn't become an argument)

  // look for keyword OPEN
  pElement->bOpen = GetKeyword (ArgumentList, "open");

  // look for keyword EMPTY
  pElement->bCommand = GetKeyword (ArgumentList, "empty");

CString strArgument;

  // get definition ( <COLOR &col;> )
  strArgument = GetArgument (ArgumentList, "", 1, false);  // get definition

// add atomic items here  --------------------------

  CString strAtom;

  const char * p = strArgument; 
  const char * pStart; 

  while (*p)
    {
    // check opening <
    if (*p != '<')
      {
      MXP_error (DBG_ERROR, errMXP_NoTagInDefinition,
                TFormat ("No opening \"<\" in MXP element definition \"%s\"", 
                (LPCTSTR) strArgument)); 
      return;
      }
    p++;  // skip <

    pStart = p;   // remember start of tag

    // skip <, look for >
    for (; *p && *p != '>'; p++) // look for closing tag
      {
      if (*p == '<')
        {
        MXP_error (DBG_ERROR, errMXP_UnexpectedDefinitionSymbol,
                  TFormat ("Unexpected \"<\" in MXP element definition \"%s\"", 
                  (LPCTSTR) strArgument)); 
        return;
        }
      if (*p == '\'' || *p == '\"') // quoted string?
        {
        char c = *p;    // remember opening quote
        for (p++; *p && *p != c; p++) // look for closing quote
          ; // just keep looking
        if (*p != c)
          {
          MXP_error (DBG_ERROR, errMXP_NoClosingDefinitionQuote,
                     TFormat ("No closing quote in MXP element definition \"%s\"", 
                    (LPCTSTR) strArgument)); 
          return;
          }
        } // end of quoted string

      } // end of search for closing tag  

    // check closing >
    if (*p != '>')
      {
      MXP_error (DBG_ERROR, errMXP_NoClosingDefinitionTag,
                TFormat ("No closing \">\" in MXP element definition \"%s\"", 
                (LPCTSTR) strArgument)); 
      return;
      }

    strAtom = CString (pStart, p - pStart);   // build tag, excluding < and >

    CString strAtomName;
    
    if (GetWord (strAtomName, strAtom))
      {
      MXP_error (DBG_ERROR, errMXP_NoDefinitionTag,
                TFormat ("No element name in MXP element definition \"<%s>\"", 
                (LPCTSTR) CString (pStart, p - pStart))); 
      return;
      }

    if (strAtomName == "/")
      {
      GetWord (strAtomName, strAtom);   // try to get next word
      strAtomName.MakeLower (); // case insensitive?
      MXP_error (DBG_ERROR, errMXP_DefinitionCannotCloseElement,
                TFormat ("Element definitions cannot close other elements: </%s>" ,
                          (LPCTSTR) strAtomName));
      return;
      }

    if (strAtomName == "!")
      {
      GetWord (strAtomName, strAtom);   // try to get next word
      strAtomName.MakeLower (); // case insensitive?
      MXP_error (DBG_ERROR, errMXP_DefinitionCannotDefineElement,
                TFormat ("Element definitions cannot define other elements: <!%s>" ,
                          (LPCTSTR) strAtomName));
      return;
      }

    strAtomName.MakeLower (); // case insensitive?

    // see if we know of this atom

    CAtomicElement * element_item;
  
    if (!App.m_ElementMap.Lookup (strAtomName, element_item))
      {
      MXP_error (DBG_ERROR, errMXP_NoInbuiltDefinitionTag,
                TFormat ("Unknown MXP element: <%s>" ,
                          (LPCTSTR) strAtomName));
      return;
      }

    // yes?  add to list

    CElementItem * pElementItem = new CElementItem;

    if (BuildArgumentList (pElementItem->ArgumentList, strAtom))  // add arguments
      {     // bad arguments
      DELETE_LIST (pElementItem->ArgumentList);
      delete pElementItem;
      return;
      }

    pElement->ElementItemList.AddTail (pElementItem );
    pElementItem->pAtomicElement = element_item;    // which atomic element

    p++; // skip >


    } // end of processing each atomic item

// end of add atomic items  --------------------------


  // get attributes  (COLOR=RED NAME=FRED)
  if (BuildArgumentList (pElement->AttributeList, GetArgument (ArgumentList, "att", 2, false)))
    {     // bad arguments
    DELETE_LIST (pElement->AttributeList);
    return;
    }


  // get tag (TAG=22)
  strArgument = GetArgument (ArgumentList, "tag", 3, true);  // get tag number

  if (IsNumeric (strArgument))
    {
    int i = atoi (strArgument);
    if (i >= 20 && i <= 99)
       pElement->iTag = i;
    }

  // get tag (FLAG=roomname)
  strArgument = GetArgument (ArgumentList, "flag", 4, true);  // get flag name

  if (!strArgument.IsEmpty ())
    {
/*
//   I won't check names right now ...

   if (strArgument == "roomname" ||
        strArgument == "roomdesc" ||
        strArgument == "roomexit" ||
        strArgument == "roomnum" ||
        strArgument == "prompt")
       pElement->strFlag = strArgument;
    else
*/
    if (strArgument.Left (4) == "set ")
      pElement->strFlag = strArgument.Mid (4);  // what variable to set
    else 
      pElement->strFlag = strArgument;
        
    pElement->strFlag.TrimLeft ();
    pElement->strFlag.TrimRight ();

    // make things a bit easier - let spaces through but change to underscores
    pElement->strFlag.Replace (" ", "_");

    // check variable name is OK
    if (CheckObjectName (pElement->strFlag) != eOK)
      {
      MXP_error (DBG_ERROR, errMXP_BadVariableName,
                 TFormat ("Bad variable name \"%s\" - for MXP FLAG definition", 
                (LPCTSTR) pElement->strFlag)); 
      pElement->strFlag.Empty ();
      }


    } // end of having a flag

  DELETE_LIST (ArgumentList);

  } // end of CMUSHclientDoc::MXP_Element

  // here for <!ATTLIST blah>
void CMUSHclientDoc::MXP_Attlist (CString strName, CString strTag)
  {
// append attributes strTag to element strName

CElement * pElement;

  strName.MakeLower (); // case-insensitive?

// check element already defined
  if (!m_CustomElementMap.Lookup (strName, pElement))
    {
    MXP_error (DBG_ERROR, errMXP_UnknownElementInAttlist,
              TFormat ("Cannot add attributes to undefined MXP element: <%s>", 
                (LPCTSTR) strName)); 
    return;
    } // end of no element matching

CArgumentList ArgumentList;

  // build into an argument list
  if (BuildArgumentList (ArgumentList, strTag))
    {
    DELETE_LIST (ArgumentList);
    return;
    }

  // add to any existing arguments - is this wise? :)
  pElement->AttributeList.AddTail (&ArgumentList);

  // nb - arguments get moved to argument list - no need to delete them
  } // end of CMUSHclientDoc::MXP_Attlist

// here for <!ENTITY blah>
void CMUSHclientDoc::MXP_Entity (CString strName, CString strTag)
  {

  // case insensitive
  strName.MakeLower ();

  CString strEntityContents;

  if (App.m_EntityMap.Lookup (strName, strEntityContents))
    {
    MXP_error (DBG_ERROR, errMXP_CannotRedefineEntity,
              TFormat ("Cannot redefine entity: &%s;", 
              (LPCTSTR) strName)); 
    return;
    }

  GetWord (strEntityContents, strTag);

    // blank contents deletes the entity
  if (strEntityContents.IsEmpty ())
     m_CustomEntityMap.RemoveKey (strName);
  else
    {

    // look for entities imbedded in the definition, eg. <!EN blah '&lt;Nick&gt;'>
    const char * p = strEntityContents;
    const char * pStart;
    CString strFixedValue;
    CString strEntity;

    strFixedValue.Empty ();

    for ( ; *p; p++)
      {
      if (*p == '&')
        {

        p++;    // skip ampersand
        pStart = p; // where entity starts
        for ( ; *p && *p != ';'; p++) // look for closing semicolon
          ; // just keep looking
        if (*p != ';')
          {
          MXP_error (DBG_ERROR, errMXP_NoClosingSemicolon,
                    TFormat ("No closing \";\" in MXP entity argument \"%s\"", 
                    (LPCTSTR) strEntityContents)); 
          return;
          }

        CString s (pStart, p - pStart);
        strFixedValue += MXP_GetEntity (s);    // add to list
      
        } // end of having an ampersand 
      else
         strFixedValue += *p;   // just add ordinary characters to list

      } // end of processing the value

    // add entity to map
    m_CustomEntityMap.SetAt (strName, strFixedValue);

    // tell each plugin what we have received
    if (m_bPluginProcessesSetEntity)
      for (POSITION pluginpos = m_PluginList.GetHeadPosition(); pluginpos; )
        {
        CPlugin * pPlugin = m_PluginList.GetNext (pluginpos);

        if (!(pPlugin->m_bEnabled))   // ignore disabled plugins
          continue;

        // see what the plugin makes of this,
        pPlugin->ExecutePluginScript (ON_PLUGIN_MXP_SETENTITY, 
                              pPlugin->m_dispid_plugin_OnMXP_SetEntity, 
                              CFormat ("%s=%s",
                              (LPCTSTR) strName,
                              (LPCTSTR) strFixedValue)
                              );
        }   // end of doing each plugin
    m_CurrentPlugin = NULL;

    }

  // check they didn't supply any other arguments
  strTag.TrimLeft ();

  while (!strTag.IsEmpty ())
    {
    CString strKeyword;
    GetWord (strKeyword, strTag);

    strKeyword.MakeLower ();

    if (strKeyword == "desc")
      {
      GetWord (strKeyword, strTag);
      if (strKeyword != "=")
        GetWord (strKeyword, strTag);   // get description
      }
    else if (strKeyword == "private")
      {
      // do nothing
      } 
    else if (strKeyword == "publish")
      {
      // do nothing
      } 
    else if (strKeyword == "delete")
      {
      m_CustomEntityMap.RemoveKey (strName);
      } 
    else if (strKeyword == "add")
      {
      // do nothing
      } 
    else if (strKeyword == "remove")
      {
      m_CustomEntityMap.RemoveKey (strName);
      } 
    else
      {
      MXP_error (DBG_WARNING, errMXP_UnexpectedEntityArguments,
                TFormat ("Unexpected word \"%s\" in entity definition for &%s; ignored", 
                (LPCTSTR) strKeyword,
                (LPCTSTR) strName)); 
      return;
      }

    } // of processing optional words

  } // end of CMUSHclientDoc::MXP_Entity
