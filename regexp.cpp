// PCRE or regexp

#include "stdafx.h"
#include "MUSHclient.h"
#include "doc.h"
#include "dialogs\RegexpProblemDlg.h"

#include "pcre\config.h"
#include "pcre\pcre_internal.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// for duplicate named wildcards
int njg_get_first_set(const pcre *code, const char *stringname, const int *ovector);

t_regexp::t_regexp ()
  : m_program(NULL), m_extra(NULL), iTimeTaken(0),
    m_iCount(0), m_iMatchAttempts(0), m_iExecutionError(0)
{}

t_regexp::~t_regexp ()
{
  if (m_program) 
    pcre_free (m_program); 
  if (m_extra) 
    pcre_free (m_extra); 
}

t_regexp * regcomp(const char *exp, const int options)
{
  const char *error = NULL;
  int erroroffset;

  pcre* program = pcre_compile(exp, options, &error, &erroroffset, NULL);
  if (!program)
    ThrowErrorException("Failed: %s at offset %d", Translate (error), erroroffset);

  // study it for speed purposes
  pcre_extra* extra = pcre_study(program, 0, &error);
  if (error)
    {
    pcre_free (program);
    ThrowErrorException("Regexp study failed: %s", error);
    }

  // we need to allocate memory for the substring offsets
  t_regexp* re = new t_regexp;

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
  // exit if no regexp program to process (possibly because of previous error)
  if (prog->m_program == NULL)
    return false;

  // inspired by a suggestion by Twisol (to remove a hard-coded limit on the number of wildcards)
  int capturecount = 0;
  // how many captures did we get?
  pcre_fullinfo(prog->m_program, NULL, PCRE_INFO_CAPTURECOUNT, &capturecount);
  // allocate enough memory
  vector<int> offsets ((capturecount + 1) * 3);  // we always get offset 0 - the whole match

  LARGE_INTEGER start, finish;
  if (App.m_iCounterFrequency)
    QueryPerformanceCounter (&start);

  int options = App.m_bRegexpMatchEmpty ? 0 : PCRE_NOTEMPTY; // don't match on an empty string

  pcre_callout = NULL;
  int count = pcre_exec(prog->m_program, prog->m_extra, string, strlen (string),
                        start_offset, options, &offsets [0], offsets.size ());

  if (App.m_iCounterFrequency)
    {
    QueryPerformanceCounter (&finish);
    prog->iTimeTaken += finish.QuadPart - start.QuadPart;
    }

  prog->m_iMatchAttempts += 1; // how many times did we try to match?

  if (count == PCRE_ERROR_NOMATCH)
    return false; // no match - don't save matching string etc.

  // cotherwise free program as an indicator that we can't keep trying to do this one
  if (count <= 0)
    {
    pcre_free (prog->m_program);
    prog->m_program = NULL;
    pcre_free (prog->m_extra);
    prog->m_extra = NULL;

    prog->m_iExecutionError = count; // remember reason
    ThrowErrorException (TFormat ("Error executing regular expression: %s",
                                  t_regexp::ErrorCodeToString (count)));
    }

  // if, and only if, we match, we will save the matching string, the count
  // and offsets, so we can extract the wildcards later on
  prog->m_sTarget = string;  // for extracting wildcards
  prog->m_iCount = count;    // ditto
  prog->m_vOffsets.clear (); 
  copy (offsets.begin (), offsets.end (), back_inserter (prog->m_vOffsets));

  return true; // match
}

// returns a numbered wildcard
string t_regexp::GetWildcard (const int iNumber) const
{
  if (iNumber >= 0 && iNumber < m_iCount)
    return string (
    &m_sTarget.c_str () [m_vOffsets [iNumber * 2]],
    m_vOffsets [(iNumber * 2) + 1] - m_vOffsets [iNumber * 2]).c_str ();
  else
    return "";
}

// returns a named wildcard
string t_regexp::GetWildcard (const string sName) const
{
  int iNumber;

  if (IsStringNumber (sName))
    iNumber = atoi (sName.c_str ());
  else if (m_program == NULL)
    iNumber = PCRE_ERROR_NOSUBSTRING;
  else
    iNumber = njg_get_first_set (m_program, sName.c_str (), &m_vOffsets [0]);

  return GetWildcard (iNumber);
}

const char* t_regexp::ErrorCodeToString(const int code)
{
  const char* error_msg = NULL;
  switch (code)
    {
    case PCRE_ERROR_NOMATCH:        error_msg = "No match";
    case PCRE_ERROR_NULL:           error_msg = "Null";
    case PCRE_ERROR_BADOPTION:      error_msg = "Bad option";
    case PCRE_ERROR_BADMAGIC:       error_msg = "Bad magic";
    case PCRE_ERROR_UNKNOWN_OPCODE: error_msg = "Unknown Opcode";
    case PCRE_ERROR_NOMEMORY:       error_msg = "No Memory";
    case PCRE_ERROR_NOSUBSTRING:    error_msg = "No Substring";
    case PCRE_ERROR_MATCHLIMIT:     error_msg = "Match Limit";
    case PCRE_ERROR_CALLOUT:        error_msg = "Callout";
    case PCRE_ERROR_BADUTF8:        error_msg = "Bad UTF8";
    case PCRE_ERROR_BADUTF8_OFFSET: error_msg = "Bad UTF8 Offset";
    case PCRE_ERROR_PARTIAL:        error_msg = "Partial";
    case PCRE_ERROR_BADPARTIAL:     error_msg = "Bad Partial";
    case PCRE_ERROR_INTERNAL:       error_msg = "Internal";
    case PCRE_ERROR_BADCOUNT:       error_msg = "Bad Count";
    case PCRE_ERROR_DFA_UITEM:      error_msg = "Dfa Uitem";
    case PCRE_ERROR_DFA_UCOND:      error_msg = "Dfa Ucond";
    case PCRE_ERROR_DFA_UMLIMIT:    error_msg = "Dfa Umlimit";
    case PCRE_ERROR_DFA_WSSIZE:     error_msg = "Dfa Wssize";
    case PCRE_ERROR_DFA_RECURSE:    error_msg = "Dfa Recurse";
    case PCRE_ERROR_RECURSIONLIMIT: error_msg = "Recursion Limit";
    case PCRE_ERROR_NULLWSLIMIT:    error_msg = "Null Ws Limit";
    case PCRE_ERROR_BADNEWLINE:     error_msg = "Bad Newline";
    default:                        error_msg = "Unknown PCRE error";
    }
  return Translate(error_msg);
}

// checks a regular expression, raises a dialog if bad
bool CheckRegularExpression (const CString strRegexp, const int iOptions)
{
  const char *error;
  int erroroffset;

  pcre* program = pcre_compile(strRegexp, iOptions, &error, &erroroffset, NULL);
  if (program)
    {
    pcre_free (program);
    return true; // good
    }

  CRegexpProblemDlg dlg;
  dlg.m_strErrorMessage = Translate (error);
  dlg.m_strErrorMessage += ".";   // end the sentence

  // make first character upper-case, so it looks like a sentence. :)
  dlg.m_strErrorMessage.SetAt (0, toupper (dlg.m_strErrorMessage [0]));

  dlg.m_iColumn = erroroffset + 1;
  dlg.m_strColumn = TFormat ("Error occurred at column %i.", dlg.m_iColumn);

  dlg.m_strText = strRegexp;
  dlg.m_strText += ENDLINE;
  if (erroroffset > 0)
    dlg.m_strText += CString ('-', erroroffset - 1);
  dlg.m_strText += '^';

  dlg.DoModal ();
  return false;   // bad
}


/*************************************************
*    Find first set of multiple named strings    *
*************************************************/

// taken from pcre_get.c - with minor modifications

/* This function allows for duplicate names in the table of named substrings.
It returns the number of the first one that was set in a pattern match.

Arguments:
  code         the compiled regex
  stringname   the name of the capturing substring
  ovector      the vector of matched substrings

Returns:       the number of the first that is set,
               or the number of the last one if none are set,
               or a negative number on error
*/

typedef unsigned char uschar;
int njg_get_first_set(const pcre *code, const char *stringname, const int *ovector)
{
  const real_pcre *re = (const real_pcre *)code;
  if ((re->options & (PCRE_DUPNAMES | PCRE_JCHANGED)) == 0)
    return pcre_get_stringnumber(code, stringname);

  char *first, *last;
  int entrysize = pcre_get_stringtable_entries(code, stringname, &first, &last);
  if (entrysize <= 0)
    return entrysize;

  for (uschar* entry = (uschar*)first; entry <= (uschar*)last; entry += entrysize)
    {
    int n = (entry[0] << 8) + entry[1];
    if (ovector[n*2] >= 0)
      return n;
    }

  return (first[0] << 8) + first[1];
}