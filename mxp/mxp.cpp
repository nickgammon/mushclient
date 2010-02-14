// mxp.cpp - MXP management

#include "stdafx.h"
#include "..\MUSHclient.h"

#include "..\doc.h"
#include "mxp.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/*

Testing with bash:

  First - turn MXP to ON.

  Then - turn on secure mode permanently:

  echo -e '\033[6z'

  Then send your commands:

  echo '<send>blah</send>'

  */

// here when element collection complete

// here at end of element collection
// we look for opening tag, closing tag or element/entity definition

void CMUSHclientDoc::MXP_collected_element (void)
  {
  m_strMXPstring.TrimLeft ();
  m_strMXPstring.TrimRight ();

//  TRACE1 ("MXP collected element: <%s>\n", (LPCTSTR) m_strMXPstring);

  MXP_error (DBG_ALL, msgMXP_CollectedElement,
              TFormat ("MXP element: <%s>", 
              (LPCTSTR) m_strMXPstring)); 

  if (m_strMXPstring.IsEmpty ())
    {
    MXP_error (DBG_ERROR, errMXP_EmptyElement, Translate ("Empty MXP element supplied.")); 
    return;
    }

  // we have four possibilities here ...

  // <bold>  - normal element
  // </bold> - terminating element
  // <!element blah >  - defining thing

  char c = m_strMXPstring [0];

  if (!isalnum (c) && m_strMXPstring.GetLength () < 2)
    {
    MXP_error (DBG_ERROR, errMXP_ElementTooShort,
                TFormat ("MXP element too short: <%s>", 
                (LPCTSTR) m_strMXPstring)); 
    return;
    }


  // test first character, that will tell us
  switch (c)
    {
    case '!' :   // comment or definition
        MXP_Definition (m_strMXPstring.Mid (1));  // process the definition
        break;
    case '/':   // end of tag
        MXP_EndTag (m_strMXPstring.Mid (1));  // process the tag ending
        break;        
    default:  // start of tag
        MXP_StartTag (m_strMXPstring);  // process the tag
        break;
    } // end of switch

  } // end of CMUSHclientDoc::MXP_collected_element

