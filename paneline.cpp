/////////////////////////////////////////////////////////////////////////////
// Name:        paneline.cpp
// Purpose:     Processing for output window lines, and styles
// Author:      Nick Gammon  <nick@gammon.com.au>
// Modified by:
// Created:     20/Feb/2004
// Copyright:   (c) Nick Gammon
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "mushclient.h"

// adds text to a line, adds to current style run if possible
void CPaneLine::AddStyle (const CPaneStyle style)
    {
    // don't need to if no text in style  (maybe)
    if (style.m_sText.empty ())
      return;

    // if first style for line, must be a new style
    if (m_vStyles.empty ())
      m_vStyles.push_back (new CPaneStyle (style));  // must add it
    else
      {
      CPaneStyle * oldstyle = m_vStyles.back ();        // last style
      if (oldstyle->m_cText   == style.m_cText &&   // text colour same?
          oldstyle->m_cBack   == style.m_cBack &&   // and background colour?
          oldstyle->m_iStyle  == style.m_iStyle)    // and type of text?
        oldstyle->m_sText += style.m_sText;       // yep - just append text
      else
        // different style, add to list
        m_vStyles.push_back (new CPaneStyle (style));   // need new style
      } // end of style list not empty
    };

// count number of *characters* in a UTF-8 string
unsigned int utf8_strlen (const char * s)
  {
  unsigned int iCount = 0;

  while (*s)
    if ((*s++ & 0xC0) != 0x80)
      iCount++;

  return iCount;
  }

unsigned int CharacterOffset (const char * s,  
                              const unsigned int iWantedCharacter, 
                              const bool bUTF8)
  {
  if (bUTF8)
    {
    unsigned int iByte = 0,
                 iCharacter = 0;

    // count characters until wanted one found
    for (iByte = 0; *s; iByte++)
      if ((*s++ & 0xC0) != 0x80)
        if (iCharacter >= iWantedCharacter)
          return iByte;   // if found, return byte position of it
        else
          iCharacter++;

    return iByte;    // not found, return length of string

    }
  else
    return iWantedCharacter;  // easy case

  };

// function object to calculate width of line
class fCalculateWidth 
  {

  unsigned int  m_iWidth;
  const bool    m_bUTF8;

  public:

  // constructor - zero width
  fCalculateWidth (const bool bUTF8 = false) : m_iWidth (0), m_bUTF8 (bUTF8) {};

  // operator() - called from for_each
  void operator() (const CPaneStyle * s)
    {
    m_iWidth += s->GetWidth (m_bUTF8);
    };

  // return calculated width
  int GetWidth (void) { return m_iWidth; };

  };  // end of class fCalculateWidth


  // calculates line width
void CPaneLine::CalculateWidth (void)
  {

  m_iWidth = (for_each (m_vStyles.begin (), 
                        m_vStyles.end (), 
                        fCalculateWidth ())).GetWidth ();

  };  // end of CPaneLine::CalculateWidth

