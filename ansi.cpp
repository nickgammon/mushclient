//  ansi.cpp - ANSI colour handling

#include "stdafx.h"
#include "MUSHclient.h"

#include "doc.h"

/*

  We get here when an ANSI colour code comes in (or maybe bold, italic etc.)
  - we now need to close off the previous style run, and start a new one.

*/

void CMUSHclientDoc::InterpretANSIcode (const int iCode)
  {

  switch (iCode)
    {
    case ANSI_TEXT_256_COLOUR:   
        m_phase = HAVE_FOREGROUND_256_START;   
        return;       
    case ANSI_BACK_256_COLOUR:     
        m_phase = HAVE_BACKGROUND_256_START;   
        return;         

    } // end of switch on iCode

// find current style
CStyle * pOldStyle = m_pCurrentLine->styleList.GetTail ();

// find current flags and colour
unsigned short iFlags       = pOldStyle->iFlags & STYLE_BITS;        
COLORREF       iForeColour  = pOldStyle->iForeColour;   
COLORREF       iBackColour  = pOldStyle->iBackColour; 
CAction *      pAction      = pOldStyle->pAction;

  // switch back to ANSI colour if required
/*   obsolete
  if ((iCode >= 30 && iCode <= 37) ||
      (iCode >= 40 && iCode <= 47))
    {
    // we can't mix them - if we go back to ANSI colours we have to discard
    // the RGB codes
    if ((iFlags & COLOURTYPE) == COLOUR_RGB)
      {
      iForeColour = WHITE;
      iBackColour = BLACK;     
      }
    iFlags &= ~COLOURTYPE;  // clear RGB or custom bits
    iFlags |= COLOUR_ANSI;
    }

*/

  // if they are in custom mode, we'll have to switch to RGB mode

  if ((iFlags & COLOURTYPE) == COLOUR_CUSTOM)
    if ((iCode >= ANSI_TEXT_BLACK && iCode <= ANSI_TEXT_WHITE) ||    // foreground colour
        (iCode >= ANSI_BACK_BLACK && iCode <= ANSI_BACK_WHITE) ||    // background colour
        (iCode == ANSI_SET_FOREGROUND_DEFAULT) ||                    // foreground to default
        (iCode == ANSI_SET_BACKGROUND_DEFAULT)                       // background to default
        )

      {
      GetStyleRGB (pOldStyle, iForeColour, iBackColour);
      iFlags &= ~COLOURTYPE;  // clear custom bits
      iFlags |= COLOUR_RGB;
      } // end of switching to RGB mode

  // if they are in RGB mode we have to do the RGB conversion now, not at display time
  if ((iFlags & COLOURTYPE) == COLOUR_RGB)
    {
    int i;

    if ((iCode >= ANSI_TEXT_BLACK && iCode <= ANSI_TEXT_WHITE) ||   // foreground colour
        (iCode == ANSI_SET_FOREGROUND_DEFAULT)
        )
      {
      if (iCode == ANSI_SET_FOREGROUND_DEFAULT)
        i = WHITE;
      else
        i = iCode - ANSI_TEXT_BLACK; // index into colour array

      if (iFlags & INVERSE)    // inverse inverts foreground and background
        {
        if (m_bAlternativeInverse)
          {
          if (iFlags & HILITE)
            iBackColour = m_boldcolour [i];   // highlighted inverse
          else
            iBackColour = m_normalcolour [i]; // normal inverse
          } // end of alternate way
        else
          iBackColour = m_normalcolour [i];    // always normal inverse
        }   // end of inverse
      else
        {   // not inverse
        if (m_bCustom16isDefaultColour && (iCode == ANSI_SET_FOREGROUND_DEFAULT))
           iForeColour = m_customtext [15];
        else
          {
          if (iFlags & HILITE)
            iForeColour = m_boldcolour [i];
          else
            iForeColour = m_normalcolour [i];
          }
        }   // end of not inverse
      } // end of foreground colour change
    else
    if ((iCode >= ANSI_BACK_BLACK && iCode <= ANSI_BACK_WHITE) ||    // background colour
        (iCode == ANSI_SET_BACKGROUND_DEFAULT)
        )
      {
      if (iCode == ANSI_SET_BACKGROUND_DEFAULT)
        i = BLACK;
      else
        i = iCode - ANSI_BACK_BLACK; // index into colour array

      if (iFlags & INVERSE)    // inverse inverts foreground and background
        {
        if (m_bAlternativeInverse)
          {
          if (iFlags & HILITE)
            iForeColour = m_boldcolour [i];   // highlighted inverse
          else
            iForeColour = m_normalcolour [i]; // normal inverse
          } // end of alternate way
        else
          iForeColour = m_normalcolour [i];    // always normal inverse
        }   // end of inverse
      else
        {   // not inverse
        if (m_bCustom16isDefaultColour && (iCode == ANSI_SET_BACKGROUND_DEFAULT))
           iBackColour = m_customback [15];
        else
          iBackColour = m_normalcolour [i];
        }   // end of not inverse

      } // end of background colour change

    }   // end style in RGB
  else    // not RGB - just pop in the appropriate colour code
    switch (iCode)
      {
      case ANSI_TEXT_BLACK:   iForeColour = BLACK;   break;       
      case ANSI_TEXT_RED:     iForeColour = RED;     break;         
      case ANSI_TEXT_GREEN:   iForeColour = GREEN;   break;       
      case ANSI_TEXT_YELLOW:  iForeColour = YELLOW;  break;      
      case ANSI_TEXT_BLUE:    iForeColour = BLUE;    break;        
      case ANSI_TEXT_MAGENTA: iForeColour = MAGENTA; break;     
      case ANSI_TEXT_CYAN:    iForeColour = CYAN;    break;        
      case ANSI_TEXT_WHITE:   iForeColour = WHITE;   break;  

      case ANSI_SET_FOREGROUND_DEFAULT:
             if (m_bCustom16isDefaultColour)
               {
               iForeColour = 15;   // use custom colour 16
               iFlags |= COLOUR_CUSTOM;
               }
             else
               {
               iForeColour = WHITE;
               }
             break;

        
      case ANSI_BACK_BLACK:   iBackColour = BLACK;   break;       
      case ANSI_BACK_RED:     iBackColour = RED;     break;         
      case ANSI_BACK_GREEN:   iBackColour = GREEN;   break;       
      case ANSI_BACK_YELLOW:  iBackColour = YELLOW;  break;      
      case ANSI_BACK_BLUE:    iBackColour = BLUE;    break;        
      case ANSI_BACK_MAGENTA: iBackColour = MAGENTA; break;     
      case ANSI_BACK_CYAN:    iBackColour = CYAN;    break;        
      case ANSI_BACK_WHITE:   iBackColour = WHITE;   break; 
        
      case ANSI_SET_BACKGROUND_DEFAULT:

             if (m_bCustom16isDefaultColour)
               {
               iBackColour = 15;
               iFlags |= COLOUR_CUSTOM;
               }
             else
               {
               iBackColour = BLACK;
               }
             break;

      } // end of switch

  // ANSI codes
  //
  // see: ftp://ftp.ecma.ch/ecma-st/Ecma-048.pdf Section 8.3.117 
  //     (page 75 in the PDF, 61 by the numbering) 

  // whether or not we are in RGB mode these codes will have the same effect
  switch (iCode)
    {
    case ANSI_RESET:   // normal text - reset everything
             iFlags &= ~(STYLE_BITS & ~ACTIONTYPE);  // clear RGB, custom mode, not underlined etc.
             if (m_bCustom16isDefaultColour)
               {
               iForeColour = 15;   // use custom colour 16
               iBackColour = 15;
               iFlags |= COLOUR_CUSTOM;
               }
             else
               {
               iForeColour = WHITE;
               iBackColour = BLACK;
               }
             break;

    case ANSI_BOLD:  
      // if we are in RGB mode then we need to make the colour bold manually
      // if possible
      if (m_bCustom16isDefaultColour &&   // only if likely to be a problem
         (iFlags & COLOURTYPE) == COLOUR_RGB &&  // if RGB mode
         !(iFlags & HILITE)     // if not done already
         )
        {
        // we will have to look for the specific normal colour and replace it
        for (int i = 0; i < 7; i++)
          if (iForeColour == m_normalcolour [i])
            {
            iForeColour = m_boldcolour [i];
            break;
            }
        }

      iFlags |= HILITE;      
      break;
    case ANSI_BLINK:  iFlags |= BLINK;       break;  // italic - same as blink right now :)
    case ANSI_UNDERLINE:  iFlags |= UNDERLINE;   break;
    case ANSI_SLOW_BLINK:  iFlags |= BLINK;       break;  // slow blink
    case ANSI_FAST_BLINK:  iFlags |= BLINK;       break;  // fast blink
    case ANSI_INVERSE:  iFlags |= INVERSE;     break;    

    // new in 3.27 - support cancel bold, underline, blink, reverse
    case ANSI_CANCEL_BOLD:  
      // if we are in RGB mode then we need to make the colour normal manually
      // if possible
      if (m_bCustom16isDefaultColour &&   // only if likely to be a problem
         (iFlags & COLOURTYPE) == COLOUR_RGB &&  // if RGB mode
         (iFlags & HILITE)     // if already bold
         )
        {
        // we will have to look for the specific normal colour and replace it
        for (int i = 0; i < 7; i++)
          if (iForeColour == m_boldcolour [i])
            {
            iForeColour = m_normalcolour [i];
            break;
            }
        }

      iFlags &= ~HILITE;      
      break;
    case ANSI_CANCEL_BLINK: iFlags &= ~BLINK;       break;   // not italic
    case ANSI_CANCEL_UNDERLINE: iFlags &= ~UNDERLINE;   break;   // not underlined
    case ANSI_CANCEL_SLOW_BLINK: iFlags &= ~BLINK;       break;   // not blinking
    case ANSI_CANCEL_INVERSE: iFlags &= ~INVERSE;     break;   // not inverse
    } // end of switch

// if the net effect is that nothing changed (eg. blue following blue) leave
// the same style running

  if (iFlags       == pOldStyle->iFlags &&      
      iForeColour  == pOldStyle->iForeColour && 
      iBackColour  == pOldStyle->iBackColour)
    return;

   RememberStyle (AddStyle (iFlags & STYLE_BITS, iForeColour, iBackColour,
                            0, pAction));

  } // end of CMUSHclientDoc::InterpretANSIcode


void CMUSHclientDoc::Interpret256ANSIcode (const int iCode)
  {

  switch (m_phase)
    {
    case HAVE_FOREGROUND_256_START:
      if (iCode == 5)
        {
        m_code = 0;
        m_phase = HAVE_FOREGROUND_256_FINISH;
        }
      else
        m_phase = NONE;
      return;

    case HAVE_BACKGROUND_256_START:
      if (iCode == 5)
        {
        m_code = 0;
        m_phase = HAVE_BACKGROUND_256_FINISH;
        }
      else
        m_phase = NONE;
      return;

    } // end of switch on m_phase



  // code out of range, give up
  if (iCode < 0 || iCode > 255)
    {
    m_phase = DOING_CODE;
    return;
    }

// find current style
CStyle * pOldStyle = m_pCurrentLine->styleList.GetTail ();

// find current flags and colour
unsigned short iFlags       = pOldStyle->iFlags & STYLE_BITS;        
COLORREF       iForeColour  = pOldStyle->iForeColour;   
COLORREF       iBackColour  = pOldStyle->iBackColour; 
CAction *      pAction      = pOldStyle->pAction;


  // if they are not in RGB mode, have to switch to it

  if ((iFlags & COLOURTYPE) != COLOUR_RGB)
    {
    GetStyleRGB (pOldStyle, iForeColour, iBackColour);
    iFlags &= ~COLOURTYPE;  // clear custom bits
    iFlags |= COLOUR_RGB;
    } // end of switching to RGB mode

  // change foreground or background as appropriate (honouring inverse flag)
  switch (m_phase)
    {
    case HAVE_FOREGROUND_256_FINISH:
      if (iFlags & INVERSE)
        iBackColour = xterm_256_colours [iCode];
      else
        iForeColour = xterm_256_colours [iCode];
      break;

    case HAVE_BACKGROUND_256_FINISH:
      if (iFlags & INVERSE)
        iForeColour = xterm_256_colours [iCode];
      else  
        iBackColour = xterm_256_colours [iCode];
      break;

    } // end of switch on m_phase

  m_phase = DOING_CODE;

// if the net effect is that nothing changed (eg. blue following blue) leave
// the same style running

  if (iFlags       == pOldStyle->iFlags &&      
      iForeColour  == pOldStyle->iForeColour && 
      iBackColour  == pOldStyle->iBackColour)
    return;

   RememberStyle (AddStyle (iFlags & STYLE_BITS, iForeColour, iBackColour,
                            0, pAction));

  }  // end of CMUSHclientDoc::Interpret256ANSIcode