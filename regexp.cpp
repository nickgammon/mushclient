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
    ThrowErrorException("Regexp study failed: %s", error);

  // we need to allocate memory for the substring offsets
  re = new t_regexp;

  if (!re)
    ThrowErrorException("Could not allocate memory for regular expression");

  // remember program and extra stuff
  re->m_program = program;
  re->m_extra = extra;
  re->m_iExecutionError = 0; // no error now

  return re;
  }

#define MAX_PCRE_WILDCARDS 1000

int regexec(register t_regexp *prog, 
            register const char *string,
            const int start_offset)
  {
int options = App.m_bRegexpMatchEmpty ? 0 : PCRE_NOTEMPTY;    // don't match on an empty string
int count;
static int offsets [MAX_PCRE_WILDCARDS * 3];  // hopefully we won't recurse and crash ;)

LARGE_INTEGER start, 
              finish;

  // exit if no regexp program to process (possibly because of previous error)
  if (prog->m_program == NULL)
    return false;

  if (App.m_iCounterFrequency)
    QueryPerformanceCounter (&start);

  pcre_callout = NULL;
  count = pcre_exec(prog->m_program, prog->m_extra, string, strlen (string),
                    start_offset, options, offsets, NUMITEMS (offsets));

  if (App.m_iCounterFrequency)
    {
    QueryPerformanceCounter (&finish);
    prog->iTimeTaken += finish.QuadPart - start.QuadPart;
    }

  if (count == PCRE_ERROR_NOMATCH)
    return false;  // no match

  // free program as an indicator that we can't keep trying to do this one
  if (count <= 0)
    {
    free (prog->m_program);
    prog->m_program = NULL;
    prog->m_iExecutionError = count; // remember reason
    }

  if (count == 0)
    ThrowErrorException (Translate ("Too many substrings in regular expression"));

  if (count < 0)
    ThrowErrorException (TFormat ("Error executing regular expression: %s",
      Convert_PCRE_Runtime_Error (count)));


  // if, and only if, we match we will save the matching string, the count
  // and offsets, so we can extract the wildcards later on

  prog->m_sTarget = string;  // for extracting wildcards
  prog->m_iCount = count;    // ditto
  prog->m_vOffsets.clear ();
  // only need first 2/3 of offsets
  copy (offsets, &offsets [count * 2], back_inserter (prog->m_vOffsets));

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
    free (program);
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