// Finding.cpp : implementation file
//

#include "stdafx.h"
#include "MUSHclient.h"
#include "mainfrm.h"

#include "dialogs\FindDlg.h"
#include "dialogs\ProgDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/*

This is my generalised "find" routine. It is designed to be used from the output buffer,
command history, triggers, aliases, and the lot.

It uses callback routines to find the first line (ie. the buffer start), and the next
line, when required.

*/

#pragma warning (disable : 4800)  // forcing value to bool 'true' or 'false' (performance warning)


static void WrapUpFind (CFindInfo & FindInfo)
  {
  Frame.SetStatusNormal (); 

  if (FindInfo.m_pProgressDlg)
    {
    delete FindInfo.m_pProgressDlg;
    FindInfo.m_pProgressDlg = NULL;
    }

  };

// displays a message about the text not being found

bool NotFound (CFindInfo & FindInfo)
  {
  WrapUpFind (FindInfo);
  CString strMsg;
  CString strFindType = "text";
  if (FindInfo.m_bRegexp)
    strFindType = "regular expression";
  strMsg = TFormat ("The %s \"%s\" was not found%s", 
                (LPCTSTR) strFindType,
                (LPCTSTR) FindInfo.m_strFindStringList.GetHead (),
                FindInfo.m_bAgain ? " again." : " .");
  ::UMessageBox (strMsg, MB_ICONINFORMATION);
  FindInfo.m_iStartColumn = -1;
  return false;
  } // end of NotFound

// find routine - returns TRUE if wanted text found

bool FindRoutine (const CObject * pObject,       // passed back to callback routines
                  CFindInfo & FindInfo,          // details about find
                  const InitiateSearch pInitiateSearch, // how to re-initiate a find
                  const GetNextLine pGetNextLine)
  {
CFindDlg dlg (FindInfo.m_strFindStringList);

  FindInfo.m_iStartColumn = -1;     // return consistent column number

// find what the heck the user wants to do

  if (!FindInfo.m_bAgain || FindInfo.m_strFindStringList.IsEmpty ())
    {

    if (!FindInfo.m_strFindStringList.IsEmpty ())
      dlg.m_strFindText = FindInfo.m_strFindStringList.GetHead ();

    dlg.m_bMatchCase  = FindInfo.m_bMatchCase;
    dlg.m_bForwards   = FindInfo.m_bForwards;
    dlg.m_strTitle    = FindInfo.m_strTitle;
    dlg.m_bRegexp     = FindInfo.m_bRegexp;

    if (dlg.DoModal () != IDOK)
      return false;

    FindInfo.m_bMatchCase    = dlg.m_bMatchCase;
    FindInfo.m_bForwards     = dlg.m_bForwards;
    FindInfo.m_bRegexp       = dlg.m_bRegexp;

    // re-initiate the search - this will set up the POSITION parameter, if it wants to
    // we need to do it here to get the current line number for subsequent calculations

    (*pInitiateSearch) (pObject, FindInfo);

    // add find string to head of list, provided it is not empty, and not the same as before
    if (!dlg.m_strFindText.IsEmpty () &&
        (FindInfo.m_strFindStringList.IsEmpty () ||
        FindInfo.m_strFindStringList.GetHead () != dlg.m_strFindText))
      FindInfo.m_strFindStringList.AddHead (dlg.m_strFindText);

    if (FindInfo.m_bForwards)
       FindInfo.m_nCurrentLine = 0;
    else
      FindInfo.m_nCurrentLine = FindInfo.m_nTotalLines - 1;

    FindInfo.m_bAgain = false;

    delete FindInfo.m_regexp;    // get rid of earlier regular expression
    FindInfo.m_regexp = NULL;

    // compile regular expression if needed
    if (FindInfo.m_bRegexp )
      FindInfo.m_regexp = regcomp (FindInfo.m_strFindStringList.GetHead (),
      (FindInfo.m_bMatchCase ? 0 :  PCRE_CASELESS) | (FindInfo.m_bUTF8 ? PCRE_UTF8 : 0));

    }   // end of not repeating the last find
  else
    {
    // doing a "find again" - step past the line we were on

    if (FindInfo.m_bForwards)
      FindInfo.m_nCurrentLine++;
    else
      FindInfo.m_nCurrentLine--;

    // re-initiate the search - this will set up the POSITION parameter, if it wants to

    (*pInitiateSearch) (pObject, FindInfo);

    }    // end of "find again"

  // if we have shuffled off the end of the buffer, say "not found"
  if (FindInfo.m_nCurrentLine < 0 || 
      FindInfo.m_nCurrentLine >= FindInfo.m_nTotalLines)
    {
    // reset line pointer to a reasonable figure
    if (FindInfo.m_bForwards)
       FindInfo.m_nCurrentLine = 0;
    else
      FindInfo.m_nCurrentLine = FindInfo.m_nTotalLines - 1;
    return NotFound (FindInfo);
    }

  // re-initiate the search - this will set up the POSITION parameter, if it wants to

//  (*pInitiateSearch) (pObject, FindInfo);

// loop until end of text, or text found
  
CString strLine;
CString strFindString = FindInfo.m_strFindStringList.GetHead ();
CString strStatus = TFormat ("Finding: %s", (LPCTSTR) FindInfo.m_strFindStringList.GetHead ());

  Frame.SetStatusMessageNow (strStatus);

// find how many more lines we have to search

  long nToGo;
  
  if (FindInfo.m_bForwards)
    nToGo = FindInfo.m_nTotalLines -  FindInfo.m_nCurrentLine;    // from here to end
  else
    nToGo = FindInfo.m_nCurrentLine;    // from here to start

  if (nToGo > 500)
    {
    FindInfo.m_pProgressDlg = new CProgressDlg;
    FindInfo.m_pProgressDlg->Create ();
    FindInfo.m_pProgressDlg->SetStatus (strStatus);
    FindInfo.m_pProgressDlg->SetRange (0, FindInfo.m_nTotalLines);     
    FindInfo.m_pProgressDlg->SetWindowText (Translate ("Finding..."));                              
    }   // end of having enough lines to warrant a progress bar

// if case-insensitive search wanted, force "text to find" to lower case

  if (!FindInfo.m_bMatchCase)
    strFindString.MakeLower ();

  try
    {
    int iMilestone = 0;

    while (true)
      {

  // get the next line, return "not found" if end of text

      if ((*pGetNextLine) (pObject, FindInfo, strLine))
        return NotFound (FindInfo);

      iMilestone++;

  // update progress control

      if (FindInfo.m_pProgressDlg && iMilestone > 31)   // every 31 lines
        {
        iMilestone = 0;
        if (FindInfo.m_bForwards)
          FindInfo.m_pProgressDlg->SetPos (FindInfo.m_nCurrentLine); 
        else
          FindInfo.m_pProgressDlg->SetPos (FindInfo.m_nTotalLines - FindInfo.m_nCurrentLine); 

        if(FindInfo.m_pProgressDlg->CheckCancelButton())
          {
          WrapUpFind (FindInfo);
          FindInfo.m_iStartColumn = -1;
          return false;
          }
        } // end of having a progress control

  // if text found on this line, then we have done it!

      if (FindInfo.m_bRegexp )
        {

        if (regexec (FindInfo.m_regexp, strLine))
          {
          // work out what column it must have been at
          FindInfo.m_iStartColumn = FindInfo.m_regexp->m_vOffsets [0];
          FindInfo.m_iEndColumn = FindInfo.m_regexp->m_vOffsets [1];
          WrapUpFind (FindInfo);
          return true;    // found it!
          }
        } // end of regular expression
      else
        { // not regular expression 
        // if case-insensitive search wanted, force this line to lower case
        if (!FindInfo.m_bMatchCase)
          strLine.MakeLower ();
        if ((FindInfo.m_iStartColumn = strLine.Find (strFindString)) != -1)
          {
          // work out ending column
          FindInfo.m_iEndColumn = FindInfo.m_iStartColumn + 
                          FindInfo.m_strFindStringList.GetHead ().GetLength ();
          WrapUpFind (FindInfo);
          return true;    // found it!
          } // end of found 
        } // end of not regular expression

  // keep track of line count

      if (FindInfo.m_bForwards)
        FindInfo.m_nCurrentLine++;
      else
        FindInfo.m_nCurrentLine--;

      }   // end of looping through each line

    } // end of try

    catch(CException* e)
      {
      e->ReportError ();
      e->Delete ();
      return NotFound (FindInfo);
      }

  } // end of FindRoutine
