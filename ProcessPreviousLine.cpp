// ProcessPreviousLine.cpp : called when a line ends with a newline
//

#include "stdafx.h"
#include "MUSHclient.h"
#include "doc.h"
#include "MUSHview.h"
#include "mxp\mxp.h"
#include "scripting\errors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static inline unsigned short get_foreground (int style)
  {
  return (style >> 4) & 0x07;
  } // end of get_foreground

static inline unsigned short get_background (int style)
  {
  return (style >> 8) & 0x07;
  } // end of get_background

static inline unsigned short get_style (int style)
  {
  return style & 0x0F;
  } // end of get_style


// shared stuff for logging in colour

void CMUSHclientDoc::LogLineInHTMLcolour (POSITION startpos)
  {
   COLORREF prevcolour = NO_COLOUR;
   bool bInSpan = false;
   COLORREF lastforecolour = 0;
   COLORREF lastbackcolour = 0;

   for (POSITION pos = startpos; pos; )
   {
   CLine * pLine = m_LineList.GetNext (pos);

   if (!pLine->styleList.IsEmpty ())
     {

      int iCol = 0;
      CString strLine = CString (pLine->text, pLine->len);

      for (POSITION style_pos = pLine->styleList.GetHeadPosition(); style_pos; )
        {
        COLORREF colour1,
                 colour2;

        CStyle * pStyle = pLine->styleList.GetNext (style_pos);

        int iLength = pStyle->iLength;

        // ignore zero length styles
        if (iLength <= 0)
           continue;

        GetStyleRGB (pStyle, colour1, colour2); // find what colour this style is

      if (colour1 != lastforecolour || colour2 != lastbackcolour)
        {
        // cancel earlier span
        if (bInSpan)
          {
           WriteToLog ("</span>");
           bInSpan = false;
          }

        // wrap up last colour change
        if (prevcolour != NO_COLOUR)
          WriteToLog ("</font>");

        WriteToLog (CFormat ("<font color=\"#%02X%02X%02X\">",
                            GetRValue (colour1),
                            GetGValue (colour1),
                            GetBValue (colour1)
                            ));

        prevcolour = colour1;

        // for efficiency we will only use <span> if we need to change the
        // background colour
        if (colour2 != 0)          // ie. not black
          {
          WriteToLog (CFormat ("<span style=\"color: #%02X%02X%02X; "
                                   "background: #%02X%02X%02X\">",
                                      GetRValue (colour1),
                                      GetGValue (colour1),
                                      GetBValue (colour1),
                                      GetRValue (colour2),
                                      GetGValue (colour2),
                                      GetBValue (colour2)
                                      ));
          bInSpan = true;
          }
        lastforecolour = colour1;
        lastbackcolour = colour2;
        }

        if (pStyle->iFlags & UNDERLINE)
          WriteToLog ("<u>");

        WriteToLog (FixHTMLString (strLine.Mid (iCol, iLength)));

        if (pStyle->iFlags & UNDERLINE)
          WriteToLog ("</u>");

        iCol += iLength; // new column

        }   // end of doing each style

     }  // end of having at least one style


   WriteToLog ("\n", 1);
   if (pLine->hard_return)    // just in case we erroneously end up at start of file
     break;
   }  // end of each line in the paragraph

  if (bInSpan)
     WriteToLog ("</span>");

  // wrap up last colour change
  if (prevcolour != NO_COLOUR)
    WriteToLog ("</font>");

  } // end of  CMUSHclientDoc::LogLineInHTMLcolour


// here when a newline is reached - process triggers etc. for the previous line
// (ie. the current one, the one just ended)
// returns true if omitting from output

bool CMUSHclientDoc::ProcessPreviousLine (void)
  {
POSITION pos, 
         prevpos = NULL;
int flags = NOTE_OR_COMMAND;
int iLineCount = 0;

CString strCurrentLine;   // we will assemble the full line here
CPaneLine StyledLine;     // and here, with style information
/*

New technique - we are going to scan *completed* lines for triggers. We know
we have a completed line (because we are here in the first place) because the
current line (m_pCurrentLine) ends with a newline.

Now, we will scan backwards until we get a line with a hard return on it (excluding this
one of course), which will be the end of the *previous* line, then work forwards and
assemble the full text of the original line.

  eg.

  22. AAAAAA \n
  23. BBBBBB
  24. CCCCCC
  25. DDDDDD \n  <- current line

  Thus our assembled line will be "BBBBBB CCCCCC DDDDDD"


*/

  // if we *do* have a hard return then we must be being called recursively
  // (eg. because a trigger sent something to the world) - better bail out
  // now before we get a stack overflow.

  if (m_pCurrentLine->hard_return)
    return false;

  // get tail of list
  
  flags = m_pCurrentLine->flags;

  // we haven't yet recorded a hard return on *this* line so this is safe
  for (pos = m_LineList.GetTailPosition (); pos; )
   {
   prevpos = pos;   // remember line which did have a hard return
   CLine * pLine = m_LineList.GetPrev (pos);
   if (pLine->hard_return || pLine->flags != flags)
     break;
   }

  // if prevpos is non-null it is now the position of the last line with a hard return
  // so, get the next one, that is the one which starts *our* sequence

  if (prevpos)
    m_LineList.GetNext (prevpos);
  else       // must be the only line in the buffer
    prevpos = m_LineList.GetHeadPosition ();

  // if no prevpos, we must be at the start of the buffer (ie. no previous line 
  // to this one, so we'll just take the current line)

  if (!prevpos)
    prevpos = m_LineList.GetHeadPosition ();

  // prevpos now points to the first line from the previous batch of lines
  for (pos = prevpos; pos; )
   {
   CLine * pLine = m_LineList.GetNext (pos);
   CString strLine = CString (pLine->text, pLine->len);
   strCurrentLine += strLine;

   // assemble styled line information
   int iCol = 0;
   for (POSITION stylepos = pLine->styleList.GetHeadPosition(); stylepos; )
     {
     CStyle * pStyle = pLine->styleList.GetNext (stylepos);

     COLORREF cText,
              cBack;

     // find actual RGB colour of style
     GetStyleRGB (pStyle, cText, cBack); 

     StyledLine.AddStyle (CPaneStyle ((const char *)
                          strLine.Mid (iCol, pStyle->iLength),
                          cText, cBack,
                          pStyle->iFlags & 7));
                          
     iCol += pStyle->iLength; // new column
     }

   flags = pLine->flags;    // flags should be the same for all lines
   iLineCount++;    // count lines in this batch
   if (pLine->hard_return)    // just in case we erroneously end up at start of file
     break;
   }


// *this* line ends with a hard break

  m_pCurrentLine->hard_return = true;

// for people with screen-readers

  if (flags & COMMENT)
    Screendraw (COMMENT, m_bLogNotes, strCurrentLine);
  else if (flags & USER_INPUT)
    Screendraw (USER_INPUT, m_log_input, strCurrentLine);
 
// if this is a world.note, and we want to log it, do so

  if ((flags & COMMENT) && m_bLogNotes)         // this is a note and we want it
    {
    // remember that we want to log it (them), for retrospective logging
    for (pos = prevpos; pos; )
      (m_LineList.GetNext (pos))->flags |= LOG_LINE;

    // log it now?
    if (m_logfile && !m_bLogRaw)
      {
      // get appropriate preamble
      CString strPreamble = m_strLogLinePreambleNotes;

      // allow %n for newline
      strPreamble.Replace ("%n", "\n");

      if (strPreamble.Find ('%') != -1)
        strPreamble = FormatTime (CTime::GetCurrentTime(), strPreamble, m_bLogHTML);

      // get appropriate Postamble
      CString strPostamble = m_strLogLinePostambleNotes;

      // allow %n for newline
      strPostamble.Replace ("%n", "\n");

      if (strPostamble.Find ('%') != -1)
        strPostamble = FormatTime (CTime::GetCurrentTime(), strPostamble, m_bLogHTML);

      // line preamble
      WriteToLog (strPreamble); 
      // line itself
      CString strMessage = strCurrentLine;

      // output as HTML if required
      if (m_bLogHTML && m_bLogInColour)
        LogLineInHTMLcolour (prevpos);
      // not colour - just straight HTML?
      else if (m_bLogHTML)
          WriteToLog (FixHTMLString (strMessage));
      else
        // just straight text
        WriteToLog (strMessage);

      // line Postamble
      WriteToLog (strPostamble); 
      if (!(m_bLogHTML && m_bLogInColour))  // colour logging has already got a newline
        WriteToLog ("\n", 1);
      } // end of having a log file and logging a comment
    }   // end of logging wanted


  // do chat snooping now :)

  // I won't snoop notes because if you snoop yourself you would probably
  // get into a big loop
  if (!(flags & COMMENT))
    for (POSITION chatpos = m_ChatList.GetHeadPosition (); chatpos; )
      {
      CChatSocket * pSocket = m_ChatList.GetNext (chatpos);

      if (pSocket->m_iChatStatus == eChatConnected) 
        {
        // if we turned "can snoop" flag off recently, turn snooping off
        //  - in the process tell him and us
        if (!pSocket->m_bCanSnoop && pSocket->m_bHeIsSnooping)
          pSocket->Process_Snoop ("");  // dummy up "stop snooping" message
        else if (pSocket->m_bHeIsSnooping)
          {
// uncoloured lines
//            pSocket->SendChatMessage (CHAT_SNOOP_DATA, 
//                      CFormat ("1500%s", (LPCTSTR) strCurrentLine));

          // prevpos now points to the first line from the previous batch of lines
          bool bBold = false,
               bUnderline = false,
               bBlink = false,
               bInverse = false;
          unsigned int  iForeground = NO_COLOUR;
          unsigned int  iBackground = NO_COLOUR;
          CString str;
          CString strRun;

          for (pos = prevpos; pos; )
           {
           CLine * pLine = m_LineList.GetNext (pos);
           int iCol = 0;

           for (POSITION stylepos = pLine->styleList.GetHeadPosition(); stylepos; )
             {
             CStyle * pStyle = pLine->styleList.GetNext (stylepos);
             strRun = CString (pLine->text, pLine->len).Mid (iCol, pStyle->iLength);
             iCol += pStyle->iLength; // new column

             if ((pStyle->iFlags & COLOURTYPE) == COLOUR_ANSI)
               {
               // do style changes

               // change to bold
               if ((pStyle->iFlags & HILITE) == HILITE &&
                   !bBold)
                 {
                 str += AnsiCode (ANSI_BOLD);
                 bBold = true;
                 }
               // change to not bold
               if ((pStyle->iFlags & HILITE) != HILITE &&
                   bBold)
                 {
                 str += AnsiCode (ANSI_CANCEL_BOLD);
                 bBold = false;
                 }

               // change to blink
               if ((pStyle->iFlags & BLINK) == BLINK &&
                   !bBlink)
                 {
                 str += AnsiCode (ANSI_BLINK);
                 bBlink = true;
                 }
               // change to not blink
               if ((pStyle->iFlags & BLINK) != BLINK &&
                   bBlink)
                 {
                 str += AnsiCode (ANSI_CANCEL_BLINK);
                 bBlink = false;
                 }

               // change to underline
               if ((pStyle->iFlags & UNDERLINE) == UNDERLINE &&
                   !bUnderline)
                 {
                 str += AnsiCode (ANSI_UNDERLINE);
                 bUnderline = true;
                 }
               // change to not underline
               if ((pStyle->iFlags & UNDERLINE) != UNDERLINE &&
                   bUnderline)
                 {
                 str += AnsiCode (ANSI_CANCEL_UNDERLINE);
                 bUnderline = false;
                 }

               // change to inverse
               if ((pStyle->iFlags & INVERSE) == INVERSE &&
                   !bInverse)
                 {
                 str += AnsiCode (ANSI_INVERSE);
                 bInverse = true;
                 }
               // change to not inverse
               if ((pStyle->iFlags & INVERSE) != INVERSE &&
                   bInverse)
                 {
                 str += AnsiCode (ANSI_CANCEL_INVERSE);
                 bInverse = false;
                 }

               // change foreground
               if (pStyle->iForeColour != iForeground)
                 {
                 iForeground = pStyle->iForeColour;
                 str += AnsiCode (iForeground + ANSI_TEXT_BLACK);
                 }

               // change background
               if (pStyle->iBackColour != iBackground)
                 {
                 iBackground = pStyle->iBackColour;
                 str += AnsiCode (iBackground + ANSI_BACK_BLACK);
                 }

               }  // end of ANSI colour
             str += strRun;
             }  // end of doing each style

           if (pLine->hard_return)    // just in case we erroneously end up at start of file
             break;
           }  // end of each line
          pSocket->SendChatMessage (CHAT_SNOOP_DATA,               
                    CFormat ("1500%s", (LPCTSTR) str)); 
          } // end of snooping

        } // end of being connected
      } // end of all chat sessions

// if a world.note or an input line, do *not* process triggers
// nb. if you get tempted to you will get a stack overflow when
//     a trigger does a world.note

  if (flags & (NOTE_OR_COMMAND | HORIZ_RULE))
    return false;   // done all we need to do, unless input from MUD

//  We can be in a plugin if we had a prompt, which was not terminated, then
//   user input. The user input calls an alias, the alias does a world.note
//   inside a plugin, however we are here right now because we are terminating the
//   *previous* line (the prompt line).
//  So, we save and restore the current plugin pointer.

  CPlugin * pSavedPlugin = m_CurrentPlugin;
  m_CurrentPlugin = NULL;

  // triggers might set these
  bool bNoLog = !m_bLogOutput;
  m_bLineOmittedFromOutput = false;
  bool bChangedColour = false;

  m_iCurrentActionSource = eInputFromServer;

  if (!SendToAllPluginCallbacks (ON_PLUGIN_LINE_RECEIVED, strCurrentLine))
    m_bLineOmittedFromOutput = true;

  m_iCurrentActionSource = eUnknownActionSource;

// next! see if we have a "mapping failure" line. If so, remove from map list

  if (m_bMapping && !m_strMappingFailure.IsEmpty ())
    {
    if (( m_bMapFailureRegexp && regexec (m_MapFailureRegexp, strCurrentLine)) ||
        (!m_bMapFailureRegexp && (strCurrentLine == m_strMappingFailure)))
       if (!m_strMapList.IsEmpty ())      // only if we have one
         {
          m_strMapList.RemoveTail ();     // last direction didn't work
          // update status line
          DrawMappingStatusLine ();
         }  // end of having something to remove
    }   // end of mapping active

// next! look for Pueblo announce string

  if (strCurrentLine.Left (strlen (PUEBLO_ID_STRING1)).  // and we have the string
                    CompareNoCase (PUEBLO_ID_STRING1) == 0 ||
      strCurrentLine.Left (strlen (PUEBLO_ID_STRING2)).  // and we have the string
                    CompareNoCase (PUEBLO_ID_STRING2) == 0 ||
      strCurrentLine.Left (strlen (PUEBLO_ID_STRING3)).  // and we have the string
                    CompareNoCase (PUEBLO_ID_STRING3) == 0)
    {
    if (!m_bPueblo) 
      MXP_error (DBG_WARNING, wrnMXP_NotStartingPueblo,
                  "Pueblo initiation string received, however Pueblo detection not enabled.");
    else
    if (m_iUseMXP == eNoMXP) 
      MXP_error (DBG_WARNING, wrnMXP_NotStartingPueblo,
                  "Pueblo initiation string received, however \"Use MXP/Pueblo\" set to \"No\".");
    else
    if (m_bPuebloActive) 
      MXP_error (DBG_WARNING, wrnMXP_NotStartingPueblo,
                  "Pueblo initiation string received, however Pueblo already active.");
    else
      
      {
      // so turn it on :)

      if (strCurrentLine.Left (strlen (PUEBLO_ID_STRING1)).  // and we have the string
                              CompareNoCase (PUEBLO_ID_STRING1) == 0)
        m_iPuebloLevel = "1.10";
      else if (strCurrentLine.Left (strlen (PUEBLO_ID_STRING3)).  // and we have the string
                              CompareNoCase (PUEBLO_ID_STRING3) == 0)
        m_iPuebloLevel = "2.50";
      else
        m_iPuebloLevel = "1.0";

      if (m_iPuebloLevel == "1.10" || m_iPuebloLevel == "2.50")
        {
        // first make a hash for the md5 field which hopefully others won't guess
        // in a hurry :)

        SHS_INFO shsInfo;
        MakeRandomNumber (this, shsInfo);

        // convert into 32 characters of hex
        m_strPuebloMD5  = CFormat ("%08x", shsInfo.digest [0]);
        m_strPuebloMD5 += CFormat ("%08x", shsInfo.digest [1]);
        m_strPuebloMD5 += CFormat ("%08x", shsInfo.digest [2]);
        m_strPuebloMD5 += CFormat ("%08x", shsInfo.digest [3]);

        // tell the server we are ready
        SendMsg (CFormat (PUEBLO_REPLY1, 
                  (LPCTSTR) m_iPuebloLevel, 
                  (LPCTSTR) m_strPuebloMD5), false, false, false);
        }
      else
        SendMsg (CFormat (PUEBLO_REPLY2), false, false, false);

      MXP_Off (true);   // turn MXP off so the initialisation string will be accepted
      } // end of Pueblo not active yet
    } // end of getting Pueblo initiation string

// process any trigger
      
// first (first? lol) add to our recent triggers for multi-line triggers.


  m_sRecentLines.push_back ((const char *) strCurrentLine);
  m_newlines_received++;

  // too many? remove oldest one
  if (m_sRecentLines.size () > MAX_RECENT_LINES)
    m_sRecentLines.pop_front ();

  CString strResponse;
  CTrigger * trigger_item;

  CTriggerList triggerList;
  CString strExtraOutput;   // for sending to output
  ScriptItemMap mapDeferredScripts;
  OneShotItemMap mapOneShotItems;

  int iBad = -1;   // default to good, if not using UTF-8
  
  // check for bad UTF8 in the line - otherwise all triggers will fail
  if (m_bUTF_8)
    {
    int erroroffset;
    iBad = _pcre_valid_utf ((const unsigned char *) (const char *) strCurrentLine, 
                             strCurrentLine.GetLength (), &erroroffset);
    if (iBad > 0)
      {
       m_iUTF8ErrorCount++;

      // every 128 lines, warn them

      if ((m_iUTF8ErrorCount & 0x7F) == 1)
        ColourNote ("white", "red", 
          TFormat ("Previous line had a bad UTF-8 sequence at column %i, and was not evaluated for trigger matches",
            iBad + 1));
        }

    }

  // only evaluate if can process them
  if (iBad <= 0)
    {
//    timer t ("Process all triggers");

    m_CurrentPlugin = NULL;
    // allow trigger evaluation for the moment
    m_iStopTriggerEvaluation = eKeepEvaluatingTriggers;

   PluginListIterator pit;

   // Do plugins (stop if one stops trigger evaluation).
   // Do only negative sequence number plugins at this point
   // Suggested by Fiendish. Added in version 4.97.
   for (pit = m_PluginList.begin ();
        pit != m_PluginList.end () &&
        (*pit)->m_iSequence < 0 &&
        m_iStopTriggerEvaluation != eStopEvaluatingTriggersInAllPlugins;
        ++pit)
      {
      m_CurrentPlugin = *pit;
      // allow trigger evaluation for the moment (ie. the next plugin)
      m_iStopTriggerEvaluation = eKeepEvaluatingTriggers;
      if (m_CurrentPlugin->m_bEnabled)
        ProcessOneTriggerSequence (strCurrentLine, 
                                   StyledLine, 
                                   strResponse, 
                                   prevpos, 
                                   bNoLog, 
                                   m_bLineOmittedFromOutput,
                                   bChangedColour, 
                                   triggerList, 
                                   strExtraOutput, 
                                   mapDeferredScripts, 
                                   mapOneShotItems);
      } // end of doing each plugin

    m_CurrentPlugin = NULL; // not in a plugin any more

    // do main triggers
    if (m_iStopTriggerEvaluation != eStopEvaluatingTriggersInAllPlugins)
      {
      m_iStopTriggerEvaluation = eKeepEvaluatingTriggers;
      ProcessOneTriggerSequence (strCurrentLine, 
                             StyledLine, 
                             strResponse, 
                             prevpos, 
                             bNoLog, 
                             m_bLineOmittedFromOutput,
                             bChangedColour, 
                             triggerList, 
                             strExtraOutput, 
                             mapDeferredScripts, 
                             mapOneShotItems);
      } // end of trigger evaluation not stopped

   // do plugins (stop if one stops trigger evaluation, or if it was stopped by the main world triggers)
   for (pit = m_PluginList.begin ();
        pit != m_PluginList.end () &&
         m_iStopTriggerEvaluation != eStopEvaluatingTriggersInAllPlugins;
         ++pit)
      {
      // skip past negative sequence numbers
      if ((*pit)->m_iSequence < 0)
         continue;
      m_CurrentPlugin = *pit;
      // allow trigger evaluation for the moment (ie. the next plugin)
      m_iStopTriggerEvaluation = eKeepEvaluatingTriggers;
      if (m_CurrentPlugin->m_bEnabled)
        ProcessOneTriggerSequence (strCurrentLine, 
                                   StyledLine, 
                                   strResponse, 
                                   prevpos, 
                                   bNoLog, 
                                   m_bLineOmittedFromOutput,
                                   bChangedColour, 
                                   triggerList, 
                                   strExtraOutput, 
                                   mapDeferredScripts, 
                                   mapOneShotItems);
      } // end of doing each plugin

    m_CurrentPlugin = NULL; // not in a plugin any more
    } // if iBad <= 0

// if we have changed the colour of this trigger, or omitted it from output,
//        we must force an update or they won't see it
  if (m_bLineOmittedFromOutput || bChangedColour)
    {
  // notify view to update their selection ranges

  	for(pos = GetFirstViewPosition(); pos != NULL; )
    	{
  		CView* pView = GetNextView(pos);
		
  		if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
  	  	{
  			CMUSHView* pmyView = (CMUSHView*)pView;

  			pmyView->Invalidate ();

  		  }	  // end of being an output view
  	  }   // end of doing each view

    }   // end of colour change or output omission

  // logging wanted?
  if (!bNoLog)
    {
    // remember that we want to log it (them), for retrospective logging
    for (pos = prevpos; pos; )
      (m_LineList.GetNext (pos))->flags |= LOG_LINE;

    // log it now?
    if (m_logfile && !m_bLogRaw) 
      {
      // get appropriate preamble
      CString strPreamble = m_strLogLinePreambleOutput;

      // allow %n for newline
      strPreamble.Replace ("%n", "\n");

      if (strPreamble.Find ('%') != -1)
        strPreamble = FormatTime (m_pCurrentLine->m_theTime, strPreamble, m_bLogHTML);

      // get appropriate Postamble
      CString strPostamble = m_strLogLinePostambleOutput;

      // allow %n for newline
      strPostamble.Replace ("%n", "\n");

      if (strPostamble.Find ('%') != -1)
        strPostamble = FormatTime (m_pCurrentLine->m_theTime, strPostamble, m_bLogHTML);

      // line preamble
      WriteToLog (strPreamble); 
      // line itself
      CString strMessage = strCurrentLine;
      // fix up HTML sequences
      if (m_bLogHTML && m_bLogInColour)
        LogLineInHTMLcolour (prevpos);
      // not colour - just straight HTML?
      else if (m_bLogHTML)
        WriteToLog (FixHTMLString (strMessage));
      else
        // straight text
        WriteToLog (strMessage);
      // line Postamble
      WriteToLog (strPostamble); 
      if (!(m_bLogHTML && m_bLogInColour))  // colour logging has already got a newline
        WriteToLog ("\n", 1);
      } // end of having a log file
    }


// if omitting from output do that now

  if (m_bLineOmittedFromOutput)
    {

  // delete all lines in this set
    for (pos = m_LineList.GetTailPosition (); pos; )
     {
     // if this particular line was added to the line positions array, then make it null
      if (m_LineList.GetCount () % JUMP_SIZE == 1)
            m_pLinePositions [m_LineList.GetCount () / JUMP_SIZE] = NULL;

      // version 4.54 - keep notes, even if omitted from output ;)
      // version 4.72 - also player input

      // we have to push_front because we are going through the lines backwards
      // this means the newline goes first. Also we process the styles in reverse order.

      CLine* pLine = m_LineList.GetTail ();

      if (pLine->flags & NOTE_OR_COMMAND)
        {
       CString strLine = CString (pLine->text, pLine->len);
       int iCol = 0;

       // throw in the newline if required
       if (pLine->hard_return)
         m_OutstandingLines.push_front (CPaneStyle (ENDLINE, 0, 0, 0));

       for (POSITION stylepos = pLine->styleList.GetTailPosition(); stylepos; )
         {
         CStyle * pStyle = pLine->styleList.GetPrev (stylepos);

         COLORREF cText,
                  cBack;

         // find actual RGB colour of style
         GetStyleRGB (pStyle, cText, cBack); 
                          
         m_OutstandingLines.push_front (CPaneStyle ((const char *)
                              strLine.Mid (pLine->len -  pStyle->iLength - iCol, pStyle->iLength), 
                              cText, cBack, pStyle->iFlags & 7));
         iCol += pStyle->iLength; // new column
         }     // end of each style


        }  // end of coming across a note or command line
      else
        {  // must be an output line
        // consider that this line is no longer a "recent line"
        // if a trigger stopped all trigger evaluation.
        // Suggested by Fiendish - version 5.06
        if (m_iStopTriggerEvaluation == eStopEvaluatingTriggersInAllPlugins)
          if (!m_sRecentLines.empty ())  // if sane to do so
            m_sRecentLines.pop_back ();
        }

      delete pLine; // delete contents of tail iten -- version 3.85
      m_LineList.RemoveTail ();   // get rid of the line
      m_total_lines--;            // don't count as received

      // if this was the first line, we have done enough
      if (pos == prevpos)
        break;    
     m_LineList.GetPrev (pos);
     }

    // try to allow world.tells to span omitted lines
    if (!m_LineList.IsEmpty ())
      {
      m_pCurrentLine = m_LineList.GetTail ();
      if (((m_pCurrentLine->flags & COMMENT) == 0) ||
          m_pCurrentLine->hard_return)
          m_pCurrentLine = NULL;
      }
    else
      m_pCurrentLine = NULL;

    if (!m_pCurrentLine)
      {
      // restart with a blank line at the end of the list
      m_pCurrentLine = new CLine (++m_total_lines, 
                                  m_nWrapColumn,
                                  m_iFlags,
                                  m_iForeColour,
                                  m_iBackColour,
                                  m_bUTF_8);
      pos = m_LineList.AddTail (m_pCurrentLine);

      if (m_LineList.GetCount () % JUMP_SIZE == 1)
        m_pLinePositions [m_LineList.GetCount () / JUMP_SIZE] = pos;
      }

    }
  else
    Screendraw (0, !bNoLog, strCurrentLine);

  // put note lines back
  OutputOutstandingLines ();

  // display any stuff sent to output window

  if (!strExtraOutput.IsEmpty ())
    DisplayMsg (strExtraOutput, strExtraOutput.GetLength (), COMMENT);

  // execute scripts now *after* we have done our omitting from output

  m_bInSendToScript = false;   // they can do DeleteLines here

// now that lines have been omitted run scripts now that wanted to be deferred      

  for (ScriptItemMap::const_iterator deferred_it = mapDeferredScripts.begin ();
       deferred_it != mapDeferredScripts.end ();
       deferred_it++)
   {
    int iSavedDepth = m_iExecutionDepth;
    m_iExecutionDepth = 0;    // no execution depth yet
    m_iCurrentActionSource = eTriggerFired;
    m_CurrentPlugin = deferred_it->pWhichPlugin;   // set back to correct plugin
    
    // if Lua, add style info to script space
    if (GetScriptEngine () && GetScriptEngine ()->L)
      {
      lua_State * L = GetScriptEngine ()->L;
      lua_newtable(L);                                                            
      int i = 1;          // style run number

      for (CPaneStyleVector::iterator style_it = StyledLine.m_vStyles.begin (); 
           style_it != StyledLine.m_vStyles.end (); 
           style_it++, i++)
        {
        lua_newtable(L);                                                            
        MakeTableItem     (L, "text",         (*style_it)->m_sText); 
        MakeTableItem     (L, "length",       (*style_it)->m_sText.length ());  
        MakeTableItem     (L, "textcolour",   (*style_it)->m_cText);  
        MakeTableItem     (L, "backcolour",   (*style_it)->m_cBack);  
        MakeTableItem     (L, "style",        (*style_it)->m_iStyle); 

        lua_rawseti (L, -2, i);  // set table item as number of style
        }

      lua_setglobal (L, "TriggerStyleRuns");
      }

    SendTo (eSendToScriptAfterOmit, 
            deferred_it->sScriptText.c_str (),   // text to execute
            false,         // omit from output flag
            false,         // omit from log flag
            deferred_it->sScriptSource.c_str (),  // eg. 'Trigger xyz'
            "",            // variable to set
            strExtraOutput // won't use this anyway
            );

    m_iCurrentActionSource = eUnknownActionSource;
    m_iExecutionDepth =  iSavedDepth;
   }    // end of doing each deferred script

// now do scripts in the script file (ie. script name in the "script" box)

  m_CurrentPlugin = NULL;
    
  bool bFoundIt;
  int iItem;
  for (pos = triggerList.GetHeadPosition (); pos; )
    {
    trigger_item = triggerList.GetNext (pos);
    bFoundIt = false;

    // check that trigger still exists, in case a script deleted it - and also
    // to work out which plugin it is in

    m_CurrentPlugin = NULL;
    // main triggers
    for (iItem = 0; !bFoundIt && iItem < GetTriggerArray ().GetSize (); iItem++)
      if (GetTriggerArray () [iItem] == trigger_item)
        {
        bFoundIt = true;
        // execute trigger script
        ExecuteTriggerScript (trigger_item, strCurrentLine, StyledLine);
        }

    // do plugins
    for (PluginListIterator pit = m_PluginList.begin (); 
         !bFoundIt && pit != m_PluginList.end (); 
          ++pit)
      {
      m_CurrentPlugin = *pit;

      if (m_CurrentPlugin->m_bEnabled)
        for (iItem = 0; !bFoundIt && iItem < GetTriggerArray ().GetSize (); iItem++)
          if (GetTriggerArray () [iItem] == trigger_item)
            {
            bFoundIt = true;
            // execute trigger script
            ExecuteTriggerScript (trigger_item, strCurrentLine, StyledLine);
            }

      } // end of doing each plugin

    }  // end of doing each trigger that had a script

  m_bInSendToScript = true;

// now that we have run all scripts etc., delete one-shot triggers      

  int iDeletedCount = 0;
  int iDeletedNonTemporaryCount = 0;
  set<CPlugin *> pluginsWithDeletions;

  for (OneShotItemMap::const_iterator one_shot_it = mapOneShotItems.begin ();
       one_shot_it != mapOneShotItems.end ();
       one_shot_it++)
   {
    CTrigger * trigger_item;
    CString strTriggerName = one_shot_it->sItemKey.c_str ();

    m_CurrentPlugin = one_shot_it->pWhichPlugin;   // set back to correct plugin

    if (!GetTriggerMap ().Lookup (strTriggerName, trigger_item))
      continue;

    // can't if executing a script
    if (trigger_item->bExecutingScript)
      continue;

    if (!m_CurrentPlugin && !trigger_item->bTemporary)
      iDeletedNonTemporaryCount++;

    iDeletedCount++;

    // the trigger seems to exist - delete its pointer
    delete trigger_item;

    // now delete its entry
    GetTriggerMap ().RemoveKey (strTriggerName);

    pluginsWithDeletions.insert (m_CurrentPlugin);

   }  // end of deleting one-shot items

   if (iDeletedCount > 0)
     {
     // make sure we sort the correct plugin(s)
     for ( set<CPlugin *>::iterator i = pluginsWithDeletions.begin (); i != pluginsWithDeletions.end (); i++)
       {
       m_CurrentPlugin = *i;
       SortTriggers ();
       }

     if (iDeletedNonTemporaryCount > 0) // plugin mods don't really count
       SetModifiedFlag (TRUE);   // document has changed
     }

// go back to current plugin

  m_CurrentPlugin = pSavedPlugin;

  // check memory still OK
//  _ASSERTE( _CrtCheckMemory( ) );

  return m_bLineOmittedFromOutput;

  }   // end of CMUSHclientDoc::ProcessPreviousLine


void CMUSHclientDoc::ProcessOneTriggerSequence (CString & strCurrentLine,
                                          CPaneLine & StyledLine,
                                          CString & strResponse,
                                          const POSITION prevpos,
                                          bool & bNoLog,
                                          bool & bNoOutput,
                                          bool & bChangedColour,
                                          CTriggerList & triggerList,
                                          CString & strExtraOutput,
                                          ScriptItemMap & mapDeferredScripts,
                                          OneShotItemMap & mapOneShotItems)
  {
// timer t ("ProcessOneTriggerSequence");

int iItem;
CTrigger * trigger_item;
int iStartCol,
    iEndCol;
POSITION pos;

  for (iItem = 0; iItem < GetTriggerArray ().GetSize (); iItem++)
    {
    trigger_item = EvaluateTrigger (strCurrentLine, 
                                        strResponse,
                                        iItem,
                                        iStartCol,
                                        iEndCol);
    if (trigger_item)
      {  


    /*
    Just one moment now! The text may have matched but do the *colours*?

    Find the colour of the first matching character and do a comparison

    */

      if (trigger_item->iMatch && !trigger_item->bMultiLine)
        {
        int iFlags = 0;
        COLORREF iForeColour = NO_COLOUR; 
        COLORREF iBackColour = NO_COLOUR; 
        int iCurrentCol = 0;
        for (pos = prevpos; pos; )  // scan to end of buffer
         {
         CLine * pLine = m_LineList.GetNext (pos);
         int iThisEnd = iCurrentCol + pLine->len;  // column this line ends at
         if (iStartCol < iThisEnd)  // starting col is in this line
           {
           int i = iStartCol - iCurrentCol;
           if (i < 0)
             i = 0;

           CStyle * pStyle = NULL;
           int iStyleCol = 0;
     
           // find style run
           for (pos = pLine->styleList.GetHeadPosition(); pos; )
             {
             pStyle = pLine->styleList.GetNext (pos);
             iStyleCol += pStyle->iLength;
             if (iStyleCol > i)   // we are at the column
               break;
             }   // end of finding style item
                  
           if (pStyle)
             {
             iFlags = pStyle->iFlags & STYLE_BITS;  // get style
             iForeColour = pStyle->iForeColour;
             iBackColour = pStyle->iBackColour;
             }
           break;                      // done
           }
         iCurrentCol += pLine->len;   // next line starts where this left off
         }    // end of doing each line


       if (trigger_item->iMatch & TRIGGER_MATCH_TEXT)
          if (get_foreground (trigger_item->iMatch) !=
              ((iFlags & INVERSE) ? iBackColour : iForeColour))
            continue;   // wrong text colour
        if (trigger_item->iMatch & TRIGGER_MATCH_BACK)
          if (get_background (trigger_item->iMatch) !=
              ((iFlags & INVERSE) ? iForeColour : iBackColour))
            continue;   // wrong background colour
        if (trigger_item->iMatch & TRIGGER_MATCH_HILITE)
          if ((get_style (trigger_item->iMatch) & HILITE) != 
              (iFlags & HILITE))
            continue;   // wrong highlighting
        if (trigger_item->iMatch & TRIGGER_MATCH_BLINK)
          if ((get_style (trigger_item->iMatch) & BLINK) != 
              (iFlags & BLINK))
            continue;   // wrong blink
        if (trigger_item->iMatch & TRIGGER_MATCH_INVERSE)
          if ((get_style (trigger_item->iMatch) & INVERSE) != 
              (iFlags & INVERSE))
            continue;   // wrong inverse

        } // end of some matching wanted


    // copy the wildcard contents to the clipboard, if required

      if (trigger_item->iClipboardArg && 
          trigger_item->iClipboardArg >= 0 &&
          trigger_item->iClipboardArg < MAX_WILDCARDS)
         putontoclipboard (trigger_item->wildcards [trigger_item->iClipboardArg].c_str (), m_bUTF_8);

      if (trigger_item->strLabel.IsEmpty ())
        Trace ("Matched trigger \"%s\"", (LPCTSTR) trigger_item->trigger);
      else
        Trace ("Matched trigger %s", (LPCTSTR) trigger_item->strLabel);

    // play the trigger sound, if we matched on a trigger

    if (!trigger_item->sound_to_play.IsEmpty () && trigger_item->sound_to_play != NOSOUNDLIT)
      if (m_enable_trigger_sounds)
        if (!trigger_item->bSoundIfInactive ||   // don't care?
            (trigger_item->bSoundIfInactive && 
             m_pActiveCommandView == NULL &&
             m_pActiveOutputView == NULL))
        PlaySoundFile (trigger_item->sound_to_play);

      /*
        // how to play a .wave file
        if (!sndPlaySound (trigger_item->sound_to_play, SND_ASYNC | SND_NODEFAULT))
          ::MessageBeep (MB_ICONEXCLAMATION);   // couldn't play it, beep instead
      */


      /*
        // how to execute any old file
        if ((int) ShellExecute(Frame.m_hWnd, "open", 
                     trigger_item->sound_to_play, 
                     NULL, NULL, 
                     SW_SHOWMINNOACTIVE) <= 32)
          ::MessageBeep (MB_ICONEXCLAMATION);   // couldn't play it, beep instead

      */


    // If we got here the colours match, or we don't care

      if (trigger_item->bOneShot)
        mapOneShotItems.push_back (
            OneShotItem (m_CurrentPlugin, 
                        (const char *) trigger_item->strInternalName));


      trigger_item->nMatched++;   // count trigger matches
      m_iTriggersMatchedCount++;
      m_iTriggersMatchedThisSessionCount++;
      
      CString strScriptSource = TFormat ("Trigger: %s", (LPCTSTR) trigger_item->strInternalName);

      if (trigger_item->iSendTo == eSendToScriptAfterOmit)
        {
        mapDeferredScripts.push_back (
            ScriptItem (m_CurrentPlugin, 
                        (const char *) strResponse, 
                        (const char *) strScriptSource));
        }
      else
        {
        int iSavedDepth = m_iExecutionDepth;
        m_iExecutionDepth = 0;    // no execution depth yet

  #ifdef PANE
        /*
        // pane sending needs colours, so do it differently :)
        if (trigger_item->iSendTo == eSendToPane)
          TriggerToPane (trigger_item, StyledLine);
        else
        */
  #endif  // PANE
          {
          trigger_item->bExecutingScript = true;     // cannot be deleted now
          m_iCurrentActionSource = eTriggerFired;
          SendTo (trigger_item->iSendTo, 
                  strResponse, 
                  trigger_item->bOmitFromOutput,
                  trigger_item->omit_from_log,  // Hmmm? is this correct? It is really the trigger line not the sent text
                  strScriptSource,
                  trigger_item->strVariable,
                  strExtraOutput
                  );
          m_iCurrentActionSource = eUnknownActionSource;
          trigger_item->bExecutingScript = false;     // can be deleted now
          }
      
        m_iExecutionDepth =  iSavedDepth;
        }    // not doing after the omitting

      // if colouring wanted, work our way through all lines to do it
      if ((trigger_item->colour != SAMECOLOUR || 
          trigger_item->iStyle != NORMAL) &&
          !trigger_item->bMultiLine)  // multi-line won't change colours
        {
        bChangedColour = true;

    /*
    Trigger colouring is going to be fiddly, because of the style runs.

    Let's say we have a line: man bites dog
    which was initially in one colour (style) and the word "bites" has to be coloured,
    then we need to split that into three styles.

    In fact, I think we potentially will need two extra styles:

    1. The left boundary (in this case "man ")
    2. The right boundary (in this case " dog")

    */
        while (true)    // repeat until we break (for repeated regexps)
          {
          int iCurrentCol = 0;   // how far through paragraph
          int iCount = iEndCol - iStartCol;   // how many columns to colour
          if (iCount <= 0)
            break;    // can't exactly colour zero columns

          //
          // Do each line (in the paragraph)
          //
          for (pos = prevpos; iCount > 0 && pos; )   // work way to end of buffer
           {
           CLine * pLine = m_LineList.GetNext (pos);
           int iThisEnd = iCurrentCol + pLine->len;   // where this line ends, in para
           if (iStartCol < iThisEnd)  // does trigger start on this line?
             {
             //
             // Trigger matching part is on *this* line
             //
             int ThisCol = iStartCol - iCurrentCol;   // start column on line
             if (ThisCol < 0)
               ThisCol = 0;

             CStyle * pStyle;
             int iCol;
             POSITION oldpos, pos;

             if (!FindStyle (pLine, ThisCol, iCol, pStyle, oldpos))
                continue;

             pos = oldpos;
             pLine->styleList.GetNext (pos);    // move pos on

             // if iCol > ThisCol then the style change occurs in the middle of this
             // style - we need to split it into two

             // NB - style changes take place in lower loop
             if (iCol > ThisCol)
               {
               // if the amount we overshot is the whole thing, we don't need to split it
               if ((iCol - ThisCol) >= pStyle->iLength)
                  pos = oldpos;
               else
                 {
                 int iDiff = iCol - ThisCol;  // amount we overshot
                 CStyle * pNewStyle = NEWSTYLE;  // make another
                 pNewStyle->iLength = iDiff;
                 pNewStyle->iFlags = pStyle->iFlags & STYLE_BITS;
                 pNewStyle->iForeColour = pStyle->iForeColour;
                 pNewStyle->iBackColour = pStyle->iBackColour ;
                 pNewStyle->pAction = pStyle->pAction;
                 if (pNewStyle->pAction)
                   pNewStyle->pAction->AddRef ();

                 pStyle->iLength -= iDiff;  // old one is that much smaller
                 // add to list
                 pos = pLine->styleList.InsertAfter (oldpos, pNewStyle); // insert
                 }
               } // end of shared style

             // we are at position where we need to change things
             for (; pos && iCount > 0; )
               {
               oldpos = pos;  // position to insert after
               pStyle = pLine->styleList.GetNext (pos);

               // if length > iCount - we would change too much - make a copy

               if (pStyle->iLength > iCount)
                 {
                 int iDiff = pStyle->iLength - iCount;  // amount we overshot
                 CStyle * pNewStyle = NEWSTYLE;  // make another
                 pNewStyle->iLength = iDiff;
                 pNewStyle->iFlags = pStyle->iFlags & STYLE_BITS;
                 pNewStyle->iForeColour = pStyle->iForeColour;
                 pNewStyle->iBackColour = pStyle->iBackColour ;
                 pNewStyle->pAction = pStyle->pAction;
                 if (pNewStyle->pAction)
                   pNewStyle->pAction->AddRef ();
                 pStyle->iLength -= iDiff;  // old one is that much smaller
                 // add to list
                 pos = pLine->styleList.InsertAfter (oldpos, pNewStyle); // insert
                 }

               // colour change wanted? switch to custom colour specified
               if (trigger_item->colour != SAMECOLOUR)
                 {

                   // get old colours in case we need to put one back
                   COLORREF cOldText, 
                            cOldBack,
                            cNewText,
                            cNewBack;

                   GetStyleRGB (pStyle, cOldText, cOldBack);

                   pStyle->iFlags &= ~(COLOURTYPE |   // clear bits, eg. RGB
                                     HILITE |       // clear other style bits
                                     UNDERLINE | 
                                     BLINK | 
                                     INVERSE);  

                   // or maybe
                   /*
                   if (pStyle->iFlags & INVERSE)
                      GetStyleRGB (pStyle, cOldBack, cOldText); 
                    else
                      GetStyleRGB (pStyle, cOldText, cOldBack); 
                   */

                   if (trigger_item->colour == OTHER_CUSTOM)
                     {
                     pStyle->iForeColour = trigger_item->iOtherForeground;
                     pStyle->iBackColour = trigger_item->iOtherBackground;
                     pStyle->iFlags |= COLOUR_RGB;
                     }  // end of other RGB colour
                   else
                     {
                     pStyle->iForeColour = trigger_item->colour;
                     pStyle->iBackColour = BLACK; // doesn't really apply
                     pStyle->iFlags |= COLOUR_CUSTOM;
                     } // end of not other colour

                 // to change only text or background we had better change to RGB mode
                 if (trigger_item->iColourChangeType != TRIGGER_COLOUR_CHANGE_BOTH)
                   {

                   // put one back if necessary
                   GetStyleRGB (pStyle, cNewText, cNewBack);

                   pStyle->iFlags &=  ~INVERSE;   // turn inverse off now
                   pStyle->iFlags &= ~COLOURTYPE;  // clear custom bits
                   pStyle->iFlags |= COLOUR_RGB;    // we have to use RGB
                   pStyle->iForeColour = cNewText;
                   pStyle->iBackColour = cNewBack;
                   if (trigger_item->iColourChangeType == TRIGGER_COLOUR_CHANGE_FOREGROUND)
                     pStyle->iBackColour = cOldBack;
                   else
                     pStyle->iForeColour = cOldText;
                   }  // end of not changing both

                 }   // end of not same colour

               pStyle->iFlags |= CHANGED | (trigger_item->iStyle &
                                    (HILITE | UNDERLINE | BLINK | INVERSE));

               iCount -= pStyle->iLength;
               }

             }  // end of being on this line
           iCurrentCol += pLine->len;   // next line starts where this left off
           }    // end of doing each line


          // cool new feature in version 4.43 - also colour the style runs which eventually get passed to a script

          int iStyleCol = 0;   // how far through paragraph
          for (CPaneStyleVector::iterator style_it = StyledLine.m_vStyles.begin (); 
               style_it != StyledLine.m_vStyles.end (); 
               style_it++)
                 {
                 CPaneStyle * pStyle =  *style_it;    // this style run
                 string sText = pStyle->m_sText;      // original style text
                 int iStyleLength = sText.size ();    // and how long it was
                 int iStyleEndCol = iStyleCol + iStyleLength;   // start column of next style

                 if (iEndCol > iStyleCol &&    // the trigger match ends after the start of the style
                     iStartCol < iStyleEndCol)  // and it starts before the end of the style
                   {
                   // if the change starts past the first column of the style we need to split into before and after
                   if (iStartCol > iStyleCol)
                     {
                     CPaneStyle * pNewStyle = new CPaneStyle (*pStyle);   // make a new style
                     pNewStyle->m_sText = sText.substr (0, iStartCol - iStyleCol);    // put front part in new style run
                     pStyle->m_sText = pStyle->m_sText.substr (iStartCol - iStyleCol);  // trim front of original style run
                     style_it = StyledLine.m_vStyles.insert (style_it, pNewStyle);   // add new style run before this one
                     style_it++;  // advance past newly inserted item
                     }  // end if we need to split before the matching text

                   // if the change ends before the last column of the style we need to split into before and after
                   if (iEndCol < (iStyleEndCol - 1))
                     {
                     CPaneStyle * pNewStyle = new CPaneStyle (*pStyle);  // make a new style
                     pNewStyle->m_sText = sText.substr (iStyleLength - (iStyleEndCol - iEndCol));    // put back part in new style run
                     pStyle->m_sText = pStyle->m_sText.substr (0, pStyle->m_sText.size () - (iStyleEndCol - iEndCol)); // trim back part
                     style_it++;  // add new style run after this one
                     style_it = StyledLine.m_vStyles.insert (style_it, pNewStyle);  // insert it
                     }  // end if we need to split after the matching text

                   // Right, we have now split up the style if necessary into another two pieces
                   // we now colour the original style (pStyle). The before and after have been placed
                   // before and after this one in the style run vector.

                   pStyle->m_iStyle = (trigger_item->iStyle & (HILITE | UNDERLINE | BLINK | INVERSE));
                   int iChangeType = trigger_item->iColourChangeType;

                   if (trigger_item->colour == OTHER_CUSTOM)   // already an RGB colour
                     {
                     if (iChangeType == TRIGGER_COLOUR_CHANGE_FOREGROUND ||
                         iChangeType == TRIGGER_COLOUR_CHANGE_BOTH)
                       pStyle->m_cText = trigger_item->iOtherForeground;    // plonk in the foreground
                     if (iChangeType == TRIGGER_COLOUR_CHANGE_BACKGROUND ||
                         iChangeType == TRIGGER_COLOUR_CHANGE_BOTH)
                       pStyle->m_cBack = trigger_item->iOtherBackground;    // and the background
                     }  // end of RGB colour
                   else
                     {  // must be normal colour - look up the RGB equivalents
                     if (iChangeType == TRIGGER_COLOUR_CHANGE_FOREGROUND ||
                         iChangeType == TRIGGER_COLOUR_CHANGE_BOTH)
                       pStyle->m_cText = m_customtext [trigger_item->colour];    // lookup foreground custom colour
                     if (iChangeType == TRIGGER_COLOUR_CHANGE_BACKGROUND ||
                         iChangeType == TRIGGER_COLOUR_CHANGE_BOTH)
                       pStyle->m_cBack = m_customback [trigger_item->colour];    // lookup background custom colour
                     }  // end normal colour

                   }  // trigger match inside this style run

                 iStyleCol += iStyleLength;   // move column past the size of this style run
                 }  // for each style run

          // end cool new feature

          // repeating only applies to regular expressions, with repeat requested
          if (!trigger_item->bRegexp || !trigger_item->bRepeat)
            break;
          // can there be any more?
          if (iEndCol > strCurrentLine.GetLength ())
            break;
          // re-scan the line
          if (!regexec (trigger_item->regexp, strCurrentLine, iEndCol))
            break;  // no more matches
          // calculate new offsets
          iStartCol = trigger_item->regexp->m_vOffsets [0];
          iEndCol   = trigger_item->regexp->m_vOffsets [1];          

          }  // end of repeating it
        } // end of some sort of colour change wanted
      // set a few flags
      if (trigger_item->omit_from_log && !trigger_item->bMultiLine)
        bNoLog = true;
  
      if (trigger_item->bOmitFromOutput && !trigger_item->bMultiLine)
        bNoOutput = true;

      if (!trigger_item->strProcedure.IsEmpty ())        // if we have a script routine
         triggerList.AddTail (trigger_item);

      if (!trigger_item->bKeepEvaluating) // exit loop if no more evaluation wanted
        break;

      // exit loop if this trigger cancelled evaluation
      if (m_iStopTriggerEvaluation != eKeepEvaluatingTriggers)
        break;

      }   // end of successful evaluation
    else
      break;
    } // end of processing triggers



  } // end of CMUSHclientDoc::ProcessOneTriggerSequence

#ifdef PANE
/*
void CMUSHclientDoc::TriggerToPane (CTrigger * pTrigger, CPaneLine & StyledLine)
  {
string sName = pTrigger->strPane;
string sTitle = pTrigger->strPane;  // name is default title

PaneMapIterator pit;

  // see if we already have this pane
  pit = m_PaneMap.find (sName); 

  // not found? Make one ...
  if (pit == m_PaneMap.end ())
    {
    if (CreatePane (sName.c_str (), sTitle.c_str (), 
        0, 0, 0, 0, // Left, Top, Width, Height
        0,          // Flags
        RGB_WHITE,  // text colour
        RGB_BLACK,   // background colour
        "",         // font
        0,          // size
        1000,          // lines
        80)          // width
        != eOK)
      return;  // cannot create pane
    pit = m_PaneMap.find (sName);
    if (pit == m_PaneMap.end ())
      return;   // cannot find new pane
    }

  CPaneView * pView = pit->second;

  // if ended with newline, or new type of line, start new one
  CPaneLine * line = pView->GetLastLine ();
  if (!line || line->GetNewline ())
    pView->AddLine ();


  for (CPaneStyleVector::iterator style_it = StyledLine.m_vStyles.begin ();
       style_it != StyledLine.m_vStyles.end ();
       style_it++)
      pView->AddText ((*style_it)->m_sText, 
                      (*style_it)->m_cText, 
                      (*style_it)->m_cBack, 
                      (*style_it)->m_iStyle); 

  pView->MarkLastLineNewLine ();

  } // end of CMUSHclientDoc::TriggerToPane
  */
#endif // PANE