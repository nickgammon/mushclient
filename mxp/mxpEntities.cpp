// mxpEntities.cpp - MXP entities

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


// here when entity collection complete

void CMUSHclientDoc::MXP_collected_entity (void)
  {
  m_strMXPstring.TrimLeft ();
  m_strMXPstring.TrimRight ();

  // case insensitive
//  m_strMXPstring.MakeLower ();

  // count them
  m_iMXPentities++;

//  TRACE1 ("MXP collected entity %s\n", (LPCTSTR) m_strMXPstring);

  MXP_error (DBG_ALL, msgMXP_CollectedEntity,
              TFormat ("MXP entity: &%s;", 
              (LPCTSTR) m_strMXPstring)); 

  if (!IsValidName (m_strMXPstring) && m_strMXPstring.Left (1) != "#")
    {
    MXP_error (DBG_ERROR, errMXP_InvalidEntityName,
                TFormat ("Invalid MXP entity name \"%s\" supplied.",
                m_strMXPstring)); 
    return;
    }

// see if we know of this entity

CString strEntityContents = MXP_GetEntity (m_strMXPstring);
  
  if (!strEntityContents.IsEmpty ())
    AddToLine (strEntityContents, 0);

  } // end of CMUSHclientDoc::MXP_collected_entity



CString CMUSHclientDoc::MXP_GetEntity (CString & strName)
  {

CString strLowerCaseName = strName;

  strLowerCaseName.MakeLower ();

CString strEntityContents;

  // look for &#nnn; 

  if (strName [0] == '#')
    {
    int iResult = 0;

    // validate and work out number
    if (strName [1] == 'x')
      {
      for (int i = 2; i < strName.GetLength (); i++)
        {
        if (!isxdigit (strName [i]))
          {
          MXP_error (DBG_ERROR, errMXP_InvalidEntityNumber,
                    TFormat ("Invalid hex number in MXP entity: &%s;" ,
                              (LPCTSTR) strName));

          return "";
          }

        int iNewDigit = toupper (strName [i]);
        if (iNewDigit >= 'A')
          iNewDigit -= 7;
        if (iResult & 0xF0)
          {
          MXP_error (DBG_ERROR, errMXP_InvalidEntityNumber,
                    TFormat ("Invalid hex number in MXP entity: &%s;" 
                             "- maximum of 2 hex digits",
                              (LPCTSTR) strName));
          return "";
          }
          iResult = (iResult << 4) + iNewDigit - '0';
        }
      } // end of hex entity
    else
      for (int i = 1; i < strName.GetLength (); i++)
        {
        if (!isdigit (strName [i]))
          {
          MXP_error (DBG_ERROR, errMXP_InvalidEntityNumber,
                    TFormat ("Invalid number in MXP entity: &%s;" ,
                              (LPCTSTR) strName));
          return "";
          }
        iResult *= 10;
        iResult += strName [i] - '0';
        }
    if (iResult != 9  && iResult != 10 )       // we will accept tabs and newlines ;)
      if (iResult < 32 ||   // don't allow nonprintable characters
          iResult > 255)   // don't allow characters more than 1 byte
          {
          MXP_error (DBG_ERROR, errMXP_DisallowedEntityNumber,
                    TFormat ("Disallowed number in MXP entity: &%s;" ,
                              (LPCTSTR) strName));
          return "";
          }
    unsigned char cOneCharacterLine [2] = { (unsigned char) iResult, 0};
    return (char *) cOneCharacterLine;
    } // end of entity starting with #

    
  // look up global entities first
  if (App.m_EntityMap.Lookup (strName, strEntityContents))
    return strEntityContents;
  // then try ones for this document
  else if (m_CustomEntityMap.Lookup (strLowerCaseName, strEntityContents))
    return strEntityContents;

  MXP_error (DBG_ERROR, errMXP_UnknownEntity,
              TFormat ("Unknown MXP entity: &%s;" ,
                        (LPCTSTR) strName));

  return "";
  }  // end of CMUSHclientDoc::MXP_GetEntity 

