/////////////////////////////////////////////////////////////////////////////
// Name:        line.h
// Purpose:     Definitions for output window lines, and styles
// Author:      Nick Gammon  <nick@gammon.com.au>
// Modified by:
// Created:     20/Feb/2004
// Copyright:   (c) Nick Gammon
/////////////////////////////////////////////////////////////////////////////

#pragma once

// The output window consists of style runs
// Each style run consists of text in an identical style
//   (eg. green on blue, italic)

// text styles
// note: by or'ing these together we get a number in the range 0-7
// which will help select the correct output font

#define NORMAL     0x0000   // plain, not bold, underline or italic
#define HILITE     0x0001   // bold
#define UNDERLINE  0x0002   // underline
#define ITALIC     0x0004   // italic


// standard colours
#define RGB_BLACK    RGB (0, 0, 0)
#define RGB_WHITE    RGB (255, 255, 255)
#define RGB_BLUE     RGB (0, 0, 128)     
#define RGB_CYAN     RGB (0, 128, 128)   
#define RGB_GREEN    RGB (0, 128, 0)     
#define RGB_MAGENTA  RGB (128, 0, 128)   
#define RGB_RED      RGB (128, 0, 0)     
#define RGB_YELLOW   RGB (128, 128, 0)   
                                         

unsigned int utf8_strlen (const char * s);  
unsigned int CharacterOffset (const char * s,  
                              const unsigned int iWantedCharacter, 
                              const bool bUTF8);

/////////////////////////////////////////////////////////////////////////////
// a style run in the output window
/////////////////////////////////////////////////////////////////////////////

class CPaneStyle
  {
  public:

  string    m_sText;    // text in this style run
  COLORREF  m_cText;    // text colour
  COLORREF  m_cBack;    // background colour
  int       m_iStyle;   // bold/italic/underline - see define above

  // constructor
  CPaneStyle (const string sText,
          const COLORREF & cText = RGB_BLACK,
          const COLORREF & cBack = RGB_BLACK,
          const int iStyle = NORMAL) :
            m_sText (sText), 
            m_cText (cText), 
            m_cBack (cBack), 
            m_iStyle (iStyle)
           {};

  // copy constructor
  CPaneStyle (const CPaneStyle & s) :
            m_sText (s.m_sText), 
            m_cText (s.m_cText), 
            m_cBack (s.m_cBack), 
            m_iStyle (s.m_iStyle)
           {};

  // operator =
  const CPaneStyle & operator= (const CPaneStyle & rhs)
    {
    m_sText =   rhs.m_sText;  
    m_cText =   rhs.m_cText;  
    m_cBack =   rhs.m_cBack;  
    m_iStyle =  rhs.m_iStyle; 
    return *this;
    };  // end of operator =

  // get width of style run in *characters* (not bytes)
  unsigned int GetWidth (const bool bUTF8 = false) const
    {
    if (bUTF8)
      return utf8_strlen (m_sText.c_str ());
    else
      return m_sText.size ();
    }

  }; // end of class CPaneStyle

typedef vector <CPaneStyle *> CPaneStyleVector;

/////////////////////////////////////////////////////////////////////////////
// a line in the output window
/////////////////////////////////////////////////////////////////////////////

class CPaneLine
  {
  const CTime        m_dTime;    // time arrived

  bool  m_bNewline;     // true if newline received for this line

  public:

  unsigned int   m_iWidth;         // width of this line (in characters)

  CPaneStyleVector m_vStyles;  // style runs

  // constructor
  CPaneLine () : 
      m_dTime (CTime::GetCurrentTime ()),   // created now
      m_bNewline (false),      // not newline yet
      m_iWidth (0)             // zero length
        {};

  // destructor - deletes styles belonging to this line
  ~CPaneLine ()
    {
    for (CPaneStyleVector::iterator it = m_vStyles.begin (); 
         it != m_vStyles.end (); 
         it++)
       delete *it;
    };

  // adds text to a line, adds to current style run if possible
  void AddStyle (const CPaneStyle style);

  // indicate newline received  (or not)
  void SetNewline (const bool bNewline = true) 
      { m_bNewline = bNewline; };

  // return time line created
  CTime GetDateTime (void) const 
      { return m_dTime; };

  // return true if newline
  bool GetNewline (void) const 
      { return m_bNewline; };

  // calculates line width
  void CalculateWidth (void);

  };  // end of class CPaneLine

