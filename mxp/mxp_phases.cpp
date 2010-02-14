// mxp_phases.cpp - handle MXP phases in incoming text stream

#include "stdafx.h"
#include "..\MUSHclient.h"
#include "..\doc.h"

// mxp phases

void CMUSHclientDoc::Phase_MXP_ELEMENT (const unsigned char c)
  {  
  switch (c)
    {
        // end of element
    case '>':
        MXP_collected_element (); // we have an element now
        m_phase = NONE;
        break;

        // shouldn't have a < inside a <
    case '<':
        MXP_unterminated_element ("Got \"<\" inside \"<\"");
        m_strMXPstring.Empty ();  // start again
        break;

        // quote inside element
    case '\'':
    case '\"':
         m_cMXPquoteTerminator = c;
         m_phase = HAVE_MXP_QUOTE;
         m_strMXPstring += c;  // collect this character
         break;

        // may be a comment? check on a hyphen
    case '-':
          m_strMXPstring += c;  // collect this character
          if (m_strMXPstring.Left (3) == "!--")
          m_phase = HAVE_MXP_COMMENT;
          break;

         // any other character, add to string
    default:
          m_strMXPstring += c;  // collect this character
          break;
    } // end of switch
  }   // end of Phase_MXP_ELEMENT           

void CMUSHclientDoc::Phase_MXP_COMMENT (const unsigned char c)
  {  
  switch (c)
    {
        // end of element
    case '>':
        if (m_strMXPstring.Right (2) == "--")
          {          
          // discard comment, just switch phase back to none
          m_phase = NONE;
          break;
          }   // end of comment terminator

        // NB no break here!
        // if > is not preceded by -- then just collect it

        // any other character, add to string
    default:
          m_strMXPstring += c;  // collect this character
          break;
    } // end of switch
  }   // end of Phase_MXP_COMMENT           

void CMUSHclientDoc::Phase_MXP_QUOTE (const unsigned char c)
  {
  // closing quote? changes phases back
  if (c == m_cMXPquoteTerminator)
    m_phase = HAVE_MXP_ELEMENT;

  m_strMXPstring += c;  // collect this character

  }   // end of Phase_MXP_QUOTE       

void CMUSHclientDoc::Phase_MXP_ENTITY (const unsigned char c)
  {  
  switch (c)
    {
    case ';':
        MXP_collected_entity (); // we have an entity now
        m_phase = NONE;
        break;

        // shouldn't have a & inside a &
    case '&':
        MXP_unterminated_element ("Got \"&\" inside \"&\"");
        m_strMXPstring.Empty ();  // start again
        break;

        // shouldn't have a < inside a &
    case '<':
        MXP_unterminated_element ("Got \"<\" inside \"&\"");
        m_phase = HAVE_MXP_ELEMENT;  // however we are now collecting an element
        m_strMXPstring.Empty ();  // start again
        break;

    default:
          m_strMXPstring += c;  // collect this character
          break;
    } // end of switch

  }   // end of Phase_MXP_ENTITY          

void CMUSHclientDoc::Phase_MXP_COLLECT_SPECIAL (const unsigned char c)
  {  
  }   // end of Phase_MXP_COLLECT_SPECIAL 

