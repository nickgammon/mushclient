
/* regexp.h - Definitions etc. for regexp(3) routines. */

#ifndef __REGEXP_H
#define __REGEXP_H



#define MAX_WILDCARDS 10

// This is Perl-Compatible Regular Expressions 

/*************************************************
*       Perl-Compatible Regular Expressions      *
*************************************************/

/* Copyright (c) 1997-2000 University of Cambridge */

#define PCRE_EXP_DECL extern

#include "pcre\pcre.h"
                 
// for duplicate named wildcards
int njg_get_first_set(const pcre *code, const char *stringname, const int *ovector);

// compiled regular expression type

class t_regexp 
  {

  public:

  t_regexp () { 
      m_program = NULL; 
      m_extra = NULL;
      iTimeTaken = 0;
      m_iCount = 0;
      m_iMatchAttempts = 0;
      m_iExecutionError = 0;
                };   // constructor
  ~t_regexp () { 
    if (m_program) 
      pcre_free (m_program); 
    if (m_extra) 
      pcre_free (m_extra); 
    };  // destructor

  // pairs of offsets from match
  vector<int> m_vOffsets;
  // count of matching wildcards
  int m_iCount;
  long m_iMatchAttempts;
  // the string we match on (to extract wildcards from)
  string m_sTarget;
  // the program itself
  pcre * m_program;	    

  // extra stuff for speed
  pcre_extra * m_extra;

  LONGLONG iTimeTaken;

  int m_iExecutionError;  // error code if failed execution

  // returns a numbered wildcard
  string GetWildcard (const int iNumber) const
    {
    if (iNumber >= 0 && iNumber < m_iCount)
      return string (
      &m_sTarget.c_str () [m_vOffsets [iNumber * 2]],
      m_vOffsets [(iNumber * 2) + 1] - m_vOffsets [iNumber * 2]).c_str ();
    else
      return "";
    };

  // returns a named wildcard
  string GetWildcard (const string sName) const
    {
    int iNumber;
    if (IsStringNumber (sName))
      iNumber = atoi (sName.c_str ());
    else
      {
      if (m_program == NULL)
        iNumber = PCRE_ERROR_NOSUBSTRING;
      else
        iNumber = njg_get_first_set (m_program, sName.c_str (), &m_vOffsets [0]);
      }
    return GetWildcard (iNumber);
    }

  };

t_regexp * regcomp(const char *exp, const int options = 0);
int regexec(register t_regexp *prog,
            register const char *string,
            const int start_offset = 0);

bool CheckRegularExpression (const CString strRegexp, const int iOptions);

#endif  // #ifndef __REGEXP_H
