// PCRE or regexp

#include "stdafx.h"
#include "MUSHclient.h"
#include "doc.h"
#include "dialogs\RegexpProblemDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


t_regexp * regcomp(const char *exp, const int options)
  {
const char *error;
int erroroffset;
t_regexp * re;
pcre * program;
pcre_extra * extra;

  program = pcre_compile(exp, options, &error, &erroroffset, NULL);

  if (!program)
    ThrowErrorException("Failed: %s at offset %d", Translate (error), erroroffset);

  // study it for speed purposes
  extra =  pcre_study(program, 0, &error);        

  if (error)
    {
    pcre_free (program);
    ThrowErrorException("Regexp study failed: %s", error);
    }

  // we need to allocate memory for the substring offsets
  re = new t_regexp;

  // remember program and extra stuff
  re->m_program = program;
  re->m_extra = extra;
  re->m_iExecutionError = 0; // no error now


  return re;
  }

int regexec(register t_regexp *prog, 
            register const char *string,
            const int start_offset)
  {
int options = App.m_bRegexpMatchEmpty ? 0 : PCRE_NOTEMPTY;    // don't match on an empty string
int count;

  // exit if no regexp program to process (possibly because of previous error)
  if (prog->m_program == NULL)
    return false;

  // inspired by a suggestion by Twisol (to remove a hard-coded limit on the number of wildcards)
  int capturecount = 0;
  // how many captures did we get?
  pcre_fullinfo(prog->m_program, NULL, PCRE_INFO_CAPTURECOUNT, &capturecount);
  // allocate enough memory
  vector<int> offsets ((capturecount + 1) * 3);  // we always get offset 0 - the whole match

  LARGE_INTEGER start, 
                finish;


  if (App.m_iCounterFrequency)
    QueryPerformanceCounter (&start);
  else
    {
    start.QuadPart = 0;
    finish.QuadPart = 0;
    }

  pcre_callout = NULL;
  count = pcre_exec(prog->m_program, prog->m_extra, string, strlen (string),
                    start_offset, options, &offsets [0], offsets.size ());

  if (App.m_iCounterFrequency)
    {
    QueryPerformanceCounter (&finish);
    prog->iTimeTaken += finish.QuadPart - start.QuadPart;
    }

  prog->m_iMatchAttempts++;  // how many times did we try to match?

  if (count == PCRE_ERROR_NOMATCH)
    return false;  // no match  - don't save matching string etc.

  //  otherwise free program as an indicator that we can't keep trying to do this one
  if (count <= 0)
    {
    pcre_free (prog->m_program);
    prog->m_program = NULL;
    pcre_free (prog->m_extra);
    prog->m_extra = NULL;
    prog->m_iExecutionError = count; // remember reason
    ThrowErrorException (TFormat ("Error executing regular expression: %s",
                         Convert_PCRE_Runtime_Error (count)));
    }


  // if, and only if, we match, we will save the matching string, the count
  // and offsets, so we can extract the wildcards later on

  prog->m_sTarget = string;  // for extracting wildcards
  prog->m_iCount = count;    // ditto
  prog->m_vOffsets.clear (); 
  copy (offsets.begin (), offsets.end (), back_inserter (prog->m_vOffsets));
  return true; // match
  }

// checks a regular expression, raises a dialog if bad

bool CheckRegularExpression (const CString strRegexp, const int iOptions)
  {
const char *error;
int erroroffset;
pcre * program;

  program = pcre_compile(strRegexp, iOptions, &error, &erroroffset, NULL);

  if (program)
    {
    pcre_free (program);
    return true;     // good
    }

  CRegexpProblemDlg dlg;
  dlg.m_strErrorMessage = Translate (error);

  dlg.m_strErrorMessage += ".";   // end the sentence
  // make first character upper-case, so it looks like a sentence. :)
  dlg.m_strErrorMessage.SetAt (0, toupper (dlg.m_strErrorMessage [0]));

  dlg.m_strColumn = TFormat ("Error occurred at column %i.", erroroffset + 1);
  dlg.m_strText = strRegexp;
  dlg.m_strText += ENDLINE;
  if (erroroffset > 0)
    dlg.m_strText += CString ('-', erroroffset - 1);
  dlg.m_strText += '^';
  dlg.m_iColumn = erroroffset + 1;
  dlg.DoModal ();
  return false;   // bad
  }