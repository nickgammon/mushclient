// methods_noting.cpp

// Stuff for writing to the output window (notes, colour notes etc.)

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\..\MUSHview.h"
#include "..\errors.h"

// Implements:

//    AnsiNote
//    ColourNote
//    ColourTell
//    GetNoteStyle
//    Hyperlink
//    Note
//    NoteColourName
//    NoteColourRGB
//    NoteHr
//    NoteStyle
//    Tell



// world.note - makes a comment in the output buffer (with newline)

void CMUSHclientDoc::Note(LPCTSTR Message) 
{
  CString strMsg = Message;

  if (strMsg.Right (2) != ENDLINE)
    strMsg += ENDLINE;      // add a new line if necessary

  Tell (strMsg);
  SetNewLineColour(0);
}   // end of CMUSHclientDoc::Note

// world.tell - makes a comment in the output buffer (without newline)

void CMUSHclientDoc::Tell(LPCTSTR Message) 
{

  // don't muck around if empty message
  if (Message [0] == 0)
    return;

  // if output buffer doesn't exist yet, remember note for later
  // or ... if this isn't a good time to be doing notes, like in a telnet
  // negotiation sequence callback
  if (m_pCurrentLine == NULL || m_pLinePositions == NULL ||
     (m_bNotesNotWantedNow && m_pCurrentLine->len != 0 && ((m_pCurrentLine->flags & NOTE_OR_COMMAND) == 0)))  // don't stick a note inside another line
    {
    COLORREF fore = m_iNoteColourFore, 
             back = m_iNoteColourBack;

    // need to do this in case a normal Note follows a ColourNote ...

    // select correct colour, if needed, from custom pallette
    if (!m_bNotesInRGB)
      {
      if (m_iNoteTextColour >= 0 && m_iNoteTextColour < MAX_CUSTOM)
        {   
        fore = m_customtext [m_iNoteTextColour];
        back = m_customback [m_iNoteTextColour];
        }  // end of notes in custom colour
      }

    m_OutstandingLines.push_back (CPaneStyle (Message, fore, back, m_iNoteStyle));
    return;
    }

  // If current line is not a note line, force a line change (by displaying
  // an empty string), so that the style change is on the note line and not
  // the back of the previous line. This has various implications, including
  // the way that "bleed colour to edge" will work. Otherwise it bleeds the
  // forthcoming note colour to the edge of the previous (mud) line.

  if ((m_pCurrentLine->flags & NOTE_OR_COMMAND) != COMMENT)
    DisplayMsg ("", 0, COMMENT);

CStyle * pOldStyle = NULL;

  if (!m_pCurrentLine->styleList.IsEmpty ())
    pOldStyle = m_pCurrentLine->styleList.GetTail ();

  if (m_bNotesInRGB)
    {
    // change style if we need to
    if (!(pOldStyle &&
        (pOldStyle->iFlags & COLOURTYPE) == COLOUR_RGB &&
        pOldStyle->iForeColour == m_iNoteColourFore &&
        pOldStyle->iBackColour == m_iNoteColourBack &&
        (pOldStyle->iFlags & TEXT_STYLE) == m_iNoteStyle
        ))
        AddStyle (COLOUR_RGB | m_iNoteStyle, 
                  m_iNoteColourFore, m_iNoteColourBack, 0, NULL);
    }   // end or RGB notes
  else
    {
    // finally found Poremenos's bug - he was doing a world.colournote
    // with the note colour as  -1 (65535) which gave an access violation
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      // change style if we need to
      if (!(pOldStyle &&
          (pOldStyle->iFlags & COLOURTYPE) == COLOUR_ANSI &&
          pOldStyle->iForeColour == WHITE &&
          pOldStyle->iBackColour == BLACK &&
          (pOldStyle->iFlags & TEXT_STYLE) == m_iNoteStyle
          ))
          AddStyle (COLOUR_ANSI | m_iNoteStyle, 
                    WHITE, BLACK, 0, NULL);
      }
    else
      {
      // change style if we need to
      if (!(pOldStyle &&
          (pOldStyle->iFlags & COLOURTYPE) == COLOUR_CUSTOM &&
          pOldStyle->iForeColour == m_iNoteTextColour &&
          pOldStyle->iBackColour == BLACK &&
          (pOldStyle->iFlags & TEXT_STYLE) == m_iNoteStyle
          ))
          AddStyle (COLOUR_CUSTOM | m_iNoteStyle, 
                    m_iNoteTextColour, BLACK, 0, NULL);
      } // not samecolour
    } // end of palette notes

  DisplayMsg (Message, strlen (Message), COMMENT);
} // end of CMUSHclientDoc::Tell


short CMUSHclientDoc::GetNoteColour() 
{
  if (m_bNotesInRGB)
    return -1;

  return m_iNoteTextColour == SAMECOLOUR ? 0 : m_iNoteTextColour + 1;
}   // end of CMUSHclientDoc::GetNoteColour

void CMUSHclientDoc::SetNoteColour(short nNewValue) 
{
  if (nNewValue >= 0 && nNewValue <= MAX_CUSTOM)
    {
    m_iNoteTextColour = nNewValue - 1;
    m_bNotesInRGB = false;
    }
}    // end of CMUSHclientDoc::SetNoteColour

long CMUSHclientDoc::GetNoteColourFore() 
{
  if (m_bNotesInRGB)
    return m_iNoteColourFore;

  if (m_iNoteTextColour == SAMECOLOUR)
    {
    if (m_bCustom16isDefaultColour)
      return m_customtext [15];
    else
      return m_normalcolour [WHITE];
    } // not same colour
  else
    return m_customtext [m_iNoteTextColour];
}   // end of CMUSHclientDoc::GetNoteColourFore

void CMUSHclientDoc::SetNoteColourFore(long nNewValue) 
{
// convert background to RGB if necessary

  if (!m_bNotesInRGB)
    {
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      if (m_bCustom16isDefaultColour)
        m_iNoteColourBack = m_customback [15];
      else
        m_iNoteColourBack = m_normalcolour [BLACK];
      } // not same colour
    else
      m_iNoteColourBack = m_customback [m_iNoteTextColour];
    }  // end of not notes in RGB

  m_bNotesInRGB = true;
  m_iNoteColourFore = nNewValue & 0x00FFFFFF;

}   // end of CMUSHclientDoc::SetNoteColourFore

long CMUSHclientDoc::GetNoteColourBack() 
{
  if (m_bNotesInRGB)
    return m_iNoteColourBack;

  if (m_iNoteTextColour == SAMECOLOUR)
    {
    if (m_bCustom16isDefaultColour)
      return m_customback [15];
    else
      return m_normalcolour [BLACK];
    } // not same colour
  else
    return m_customback [m_iNoteTextColour];
}   // end of CMUSHclientDoc::GetNoteColourBack

void CMUSHclientDoc::SetNoteColourBack(long nNewValue) 
{
// convert foreground to RGB if necessary
  if (!m_bNotesInRGB)
    {
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      if (m_bCustom16isDefaultColour)
        m_iNoteColourFore = m_customtext [15];
      else
        m_iNoteColourFore = m_normalcolour [WHITE];
      } // not same colour
    else
      m_iNoteColourFore = m_customtext [m_iNoteTextColour];
    }  // end of not notes in RGB

  m_bNotesInRGB = true;
  m_iNoteColourBack = nNewValue & 0x00FFFFFF;

}    // end of CMUSHclientDoc::SetNoteColourBack

void CMUSHclientDoc::NoteColourRGB(long Foreground, long Background) 
{
  m_bNotesInRGB = true;
  m_iNoteColourFore = Foreground & 0x00FFFFFF;
  m_iNoteColourBack = Background & 0x00FFFFFF;
}   // end of CMUSHclientDoc::NoteColourRGB

void CMUSHclientDoc::NoteColourName(LPCTSTR Foreground, LPCTSTR Background) 
{
// convert background to RGB if necessary
  if (!m_bNotesInRGB)
    {
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      if (m_bCustom16isDefaultColour)
        {
        m_iNoteColourFore = m_customtext [15];
        m_iNoteColourBack = m_customback [15];
        }
      else
        {
        m_iNoteColourFore = m_normalcolour [WHITE];
        m_iNoteColourBack = m_normalcolour [BLACK];
        }
      }   // end of notes were "same colour" (as what?)
    else
      // just normal custom colours - however check in range just in case
      if (m_iNoteTextColour >= 0 && m_iNoteTextColour < MAX_CUSTOM)
        {   // notes were a custom colour
        m_iNoteColourFore = m_customtext [m_iNoteTextColour];
        m_iNoteColourBack = m_customback [m_iNoteTextColour];
        }  // end of notes in custom colour
      else
        {   // white on black - shouldn't get here
        m_iNoteColourFore = RGB (255, 255, 255);
        m_iNoteColourBack = RGB (0, 0, 0);
        }  // end of notes in custom colour

    }   // end of not notes in RGB already

  m_bNotesInRGB = true;
  SetColour (Foreground, m_iNoteColourFore);
  SetColour (Background, m_iNoteColourBack);
}   // end of CMUSHclientDoc::NoteColourName


// rewritten: 20th November 2016 - version 5.04
void CMUSHclientDoc::AnsiNote(LPCTSTR Text) 
{
// save old colours
bool bOldNotesInRGB = m_bNotesInRGB;
COLORREF iOldNoteColourFore = m_iNoteColourFore;
COLORREF iOldNoteColourBack = m_iNoteColourBack;
unsigned short iOldNoteStyle = m_iNoteStyle;
unsigned short iOldNoteTextColour = m_iNoteTextColour;

COLORREF rgbNormalForeGround = GetNoteColourFore ();
COLORREF rgbNormalBackGround = GetNoteColourBack ();
COLORREF rgbBoldForeGround   = GetNoteColourFore ();  // not sure about these

// state machine control variable
int ansiState = NONE;

m_iNoteStyle = NORMAL;   // start off with normal style

const char * p,
           * start;
char c;
int iCode = 0;

  // p points to the current character in the text
  p = start = Text;
  while ((c = *p++))  // intentional assignment
    {

    // we basically have ESC followed by various things, or normal text
    switch (c)
      {
      // ESC potentially starts and ANSI sequence
      case ESC:
        // output earlier block
        if ((p - start) > 0)
          Tell (CString (start, p - start));
        ansiState = HAVE_ESC;
        break;  // end of ESC

      // it has to be ESC [ ...
      case '[':
        if (ansiState == HAVE_ESC)
          {
          ansiState = DOING_CODE;
          iCode = 0;
          }
        else
          ansiState = NONE;  // unexpected [ inside an ANSI sequence
        break;  // end of '['

      // digits?  (eg. ESC [ 5
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        {
        switch (ansiState)
            {
            case DOING_CODE:
            case HAVE_FOREGROUND_256_START:
            case HAVE_FOREGROUND_256_FINISH:
            case HAVE_BACKGROUND_256_START:
            case HAVE_BACKGROUND_256_FINISH:
                  iCode *= 10;
                  iCode += c - '0';
                  break;

            } // end of switch on ansiState
          }  // end of digits
        break;  // end of digits

      // either 'm' or ';' terminates the current number, eg. ESC [1;
      case ';':
      case 'm':

        // interpret the previous code in iCode
        switch (ansiState)
          {
          case DOING_CODE:

            switch (iCode)
              {
              // reset colours to defaults
              case ANSI_RESET:
                 m_iNoteStyle = NORMAL;
                 // save colours for later (eg. if it becomes bold or not bold)
                 rgbNormalForeGround = m_normalcolour [WHITE];
                 rgbNormalBackGround = m_normalcolour [BLACK];
                 rgbBoldForeGround   = m_boldcolour   [WHITE];
                 // Note: this sets m_bNotesInRGB mode on
                 SetNoteColourFore (rgbNormalForeGround);
                 SetNoteColourBack (rgbNormalBackGround);
                 break;

              // bold
              case ANSI_BOLD:
                 // Note: this sets m_bNotesInRGB mode on
                 SetNoteColourFore (rgbBoldForeGround);
                 m_iNoteStyle |= HILITE;
                 break;

              // inverse
              case ANSI_INVERSE:
                 m_iNoteStyle |= INVERSE;
                 break;

              // blink
              case ANSI_BLINK:
              case ANSI_SLOW_BLINK:
              case ANSI_FAST_BLINK:
                 m_iNoteStyle |= BLINK;
                 break;

              // underline
              case ANSI_UNDERLINE:
                 m_iNoteStyle |= UNDERLINE;
                 break;

              // not bold
              case ANSI_CANCEL_BOLD:
                 SetNoteColourFore (rgbNormalForeGround);
                 m_iNoteStyle &= ~HILITE;
                 break;

              // not inverse
              case ANSI_CANCEL_INVERSE:
                 m_iNoteStyle &= ~INVERSE;
                 break;

              // not blink
              case ANSI_CANCEL_BLINK:
              case ANSI_CANCEL_SLOW_BLINK:
                 m_iNoteStyle &= ~BLINK;
                 break;

              // not underline
              case ANSI_CANCEL_UNDERLINE:
                 m_iNoteStyle &= ~UNDERLINE;
                 break;

              // different foreground colour
              case ANSI_TEXT_BLACK:
              case ANSI_TEXT_RED    :
              case ANSI_TEXT_GREEN  :
              case ANSI_TEXT_YELLOW :
              case ANSI_TEXT_BLUE   :
              case ANSI_TEXT_MAGENTA:
              case ANSI_TEXT_CYAN   :
              case ANSI_TEXT_WHITE  :
                // save colour for possible later use
                rgbNormalForeGround = m_normalcolour [iCode - ANSI_TEXT_BLACK];
                rgbBoldForeGround   = m_boldcolour   [iCode - ANSI_TEXT_BLACK];
                if (m_iNoteStyle & HILITE)
                  SetNoteColourFore (rgbBoldForeGround);
                else
                  SetNoteColourFore (rgbNormalForeGround);
                break;

              // different background colour
              case ANSI_BACK_BLACK  :
              case ANSI_BACK_RED    :
              case ANSI_BACK_GREEN  :
              case ANSI_BACK_YELLOW :
              case ANSI_BACK_BLUE   :
              case ANSI_BACK_MAGENTA:
              case ANSI_BACK_CYAN   :
              case ANSI_BACK_WHITE  :
                // save colour for possible later use
                rgbNormalBackGround = m_normalcolour [iCode - ANSI_BACK_BLACK];
                SetNoteColourBack (rgbNormalBackGround);
                break;

              // we have: ESC 38;
              case ANSI_TEXT_256_COLOUR :
                ansiState = HAVE_FOREGROUND_256_START;
                break;

              // we have: ESC 48;
              case ANSI_BACK_256_COLOUR :
                ansiState = HAVE_BACKGROUND_256_START;
                break;

              } // end of switch on iCode

            iCode = 0;  // possibly starting a new code
            break;      // end of DOING_CODE


          // we now have: ESC [ 38;5;nnn;
          case HAVE_FOREGROUND_256_START:
            if (iCode == 5)  // if it isn't 5 I don't know what the hell it is!
              {
              ansiState = HAVE_FOREGROUND_256_FINISH;
              iCode = 0;  // start collecting the colour number
              }
            else
              ansiState = DOING_CODE;  // give up and look for another code
            break;  // end of HAVE_FOREGROUND_256_START

          case HAVE_FOREGROUND_256_FINISH:
            rgbNormalForeGround = xterm_256_colours [iCode & 0xFF];
            rgbBoldForeGround   = xterm_256_colours [iCode & 0xFF];  // both the same
            SetNoteColourFore (rgbNormalForeGround);
            ansiState = DOING_CODE; // no longer collecting 256-colours
            break;  // end of HAVE_FOREGROUND_256_FINISH

          // we now have: ESC [ 48;5;nnn;
          case HAVE_BACKGROUND_256_START:
            if (iCode == 5)  // if it isn't 5 I don't know what the hell it is!
              {
              ansiState = HAVE_BACKGROUND_256_FINISH;
              iCode = 0;  // start collecting the colour number
              }
            else
              ansiState = DOING_CODE;  // give up and look for another code
            break;  // end of HAVE_BACKGROUND_256_START

          case HAVE_BACKGROUND_256_FINISH:
            rgbNormalBackGround = xterm_256_colours [iCode & 0xFF];
            SetNoteColourBack (rgbNormalBackGround);
            ansiState = DOING_CODE; // no longer collecting 256-colours
            break;  // end of HAVE_FOREGROUND_256_FINISH

          default:
            // just accumulate the text
            break;

          } // end of switch on ansiState

        // an 'm' terminates this ANSI sequence (unless we weren't in one)
        if (ansiState != NONE && c == 'm')
          {
          start = p;  // ready for outputting the next batch of non-ANSI sequences
          ansiState = NONE;
          }
       break;   // end of 'm' or ';'

      } // end of switch on current character

    } // end of processing each character

  // output remaining text  - and newline
  Note (start);

  // put the colours back
  if (bOldNotesInRGB)
    {
    m_iNoteColourFore = iOldNoteColourFore;
    m_iNoteColourBack = iOldNoteColourBack;
    }
  else  
    {
    m_bNotesInRGB = false;
    // put old text colour back
    m_iNoteTextColour = iOldNoteTextColour;
    }

  // put style back
  m_iNoteStyle = iOldNoteStyle;
  SetNewLineColour(0);
} // end of CMUSHclientDoc::AnsiNote

void CMUSHclientDoc::ColourNote(LPCTSTR TextColour, 
                                LPCTSTR BackgroundColour, 
                                LPCTSTR Text) 
{
  CString strMsg = Text;

  if (strMsg.Right (2) != ENDLINE)
    strMsg += ENDLINE;      // add a new line if necessary

  ColourTell (TextColour, BackgroundColour, strMsg);
  SetNewLineColour(0);
}  // end of CMUSHclientDoc::ColourNote

void CMUSHclientDoc::ColourTell(LPCTSTR TextColour, LPCTSTR BackgroundColour, LPCTSTR Text) 
{

// save old colours
bool bOldNotesInRGB = m_bNotesInRGB;
COLORREF iOldNoteColourFore = m_iNoteColourFore;
COLORREF iOldNoteColourBack = m_iNoteColourBack;

// change colours
  NoteColourName(TextColour, BackgroundColour);

// do the tell
  Tell (Text);

// put the colours back
if (bOldNotesInRGB)
  {
  m_iNoteColourFore = iOldNoteColourFore;
  m_iNoteColourBack = iOldNoteColourBack;
  }
else  
  m_bNotesInRGB = false;

}   // end of CMUSHclientDoc::ColourTell


/*

  WARNING!

  Do not use in sendto: script, with "omit from output"!

  The saved and restored styles will lose the hyperlink action. 
  
  Use sendto:script (after omit) instead.

  http://www.gammon.com.au/forum/?id=11067

*/

void CMUSHclientDoc::Hyperlink_Helper (LPCTSTR Action, 
                       LPCTSTR Text, 
                       LPCTSTR Hint, 
                       LPCTSTR TextColour, 
                       LPCTSTR BackColour, 
                       BOOL URL,
                       BOOL NoUnderline)

{
  // return if attempt to do tell (or note) before output buffer exists
  if (m_pCurrentLine == NULL || m_pLinePositions == NULL)
    return;

  // don't muck around if empty message
  if (Action [0] == 0)
    return;

  // If current line is not a note line, force a line change (by displaying
  // an empty string), so that the style change is on the note line and not
  // the back of the previous line. This has various implications, including
  // the way that "bleed colour to edge" will work. Otherwise it bleeds the
  // forthcoming note colour to the edge of the previous (mud) line.

  if ((m_pCurrentLine->flags & NOTE_OR_COMMAND) != COMMENT)
    DisplayMsg ("", 0, COMMENT);

  COLORREF forecolour = m_iHyperlinkColour,
           backcolour;

  if (m_bNotesInRGB)
    backcolour = m_iNoteColourBack;
  else
    {
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      if (m_bCustom16isDefaultColour)
        backcolour =  m_customback [15];
      else
        backcolour =  m_normalcolour [BLACK];
      } // not same colour
    else
      backcolour =  m_customback [m_iNoteTextColour];
    }

  SetColour (TextColour, forecolour);
  SetColour (BackColour, backcolour);

  // change to underlined hyperlink
  AddStyle (COLOUR_RGB | 
            (URL ? ACTION_HYPERLINK : ACTION_SEND) | 
            (NoUnderline ? 0 : UNDERLINE), 
            forecolour, 
            backcolour, 0, 
            GetAction (Action, 
                        Hint [0] == 0 ? Action : Hint, 
                        ""));

  // output the link text
  if (strlen (Text) > 0)
    AddToLine (Text, COMMENT);
  else
    AddToLine (Action, COMMENT);

  // add another style to finish the hyperlink

  if (m_bNotesInRGB)
    AddStyle (COLOUR_RGB, m_iNoteColourFore, m_iNoteColourBack, 0, NULL);
  else
    {
    if (m_iNoteTextColour == SAMECOLOUR)
      {
      if (m_bCustom16isDefaultColour)
        AddStyle (COLOUR_CUSTOM, 15, BLACK, 0, NULL);
      else
        AddStyle (COLOUR_ANSI, WHITE, BLACK, 0, NULL);
      } // not same colour
    else
      AddStyle (COLOUR_CUSTOM, m_iNoteTextColour, BLACK, 0, NULL);
    } // not RGB

}   // end of CMUSHclientDoc::Hyperlink_Helper


void CMUSHclientDoc::Hyperlink(LPCTSTR Action, 
                               LPCTSTR Text, 
                               LPCTSTR Hint, 
                               LPCTSTR TextColour, 
                               LPCTSTR BackColour, 
                               BOOL URL) 
  {

  Hyperlink_Helper (Action, 
                   Text, 
                   Hint, 
                   TextColour, 
                   BackColour, 
                   URL,
                   FALSE);
  } // end of CMUSHclientDoc::Hyperlink


/*

  Notes text style:

  0 = normal
  1 = bold
  2 = underline
  4 = blink (italic)
  8 = inverse

  */

void CMUSHclientDoc::NoteStyle(short Style) 
{
  m_iNoteStyle = Style & TEXT_STYLE;    // only want those bits
}   // end of CMUSHclientDoc::NoteStyle

short CMUSHclientDoc::GetNoteStyle() 
{
  return m_iNoteStyle & TEXT_STYLE;
}   // end of CMUSHclientDoc::GetNoteStyle


// horizontal rule (HR)

void CMUSHclientDoc::NoteHr() 
{
  // wrap up previous line if necessary
  if (m_pCurrentLine->len > 0)
     StartNewLine (true, 0);

  // mark line as HR line
  m_pCurrentLine->flags = HORIZ_RULE;
  
  StartNewLine (true, 0); // now finish this line

  // refresh views

  for(POSITION pos=GetFirstViewPosition();pos!=NULL;)
    {
    CView* pView = GetNextView(pos);
    
    if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
      {
      CMUSHView* pmyView = (CMUSHView*)pView;

      pmyView->addedstuff();
      } 
    }

}  // end of CMUSHclientDoc::NoteHr
