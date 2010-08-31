/* regexp.h - Definitions etc. for regexp(3) routines. */
#ifndef __REGEXP_H
#define __REGEXP_H

#define PCRE_EXP_DECL extern
#include "pcre\pcre.h"

const short MAX_WILDCARDS = 10;

// This is Perl-Compatible Regular Expressions 

/*************************************************
*       Perl-Compatible Regular Expressions      *
*************************************************/

/* Copyright (c) 1997-2000 University of Cambridge */

// compiled regular expression type

class t_regexp 
{
public:
  t_regexp (const char* pattern, const int flags);
  ~t_regexp ();

  void Compile (const char* pattern, const int flags);
  bool Execute (const char* string, const int start_offset=0);

  // returns a wildcard by number
  string GetWildcard (const int iNumber) const;
  // returns a wildcard by name
  string GetWildcard (const string sName) const;

  bool GetWildcardOffsets (const int iNumber, int& iLeft, int& iRight) const;
  bool GetWildcardOffsets (const string& sName, int& iLeft, int& iRight) const;

  static const char* ErrorCodeToString(const int code);
  static bool CheckPattern (const char* pattern, const int iOptions,
                            const char** error, int* errorOffset);


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

private:
  void AcquirePattern(pcre* program, pcre_extra* extra);
  void ReleasePattern();

  // pairs of offsets from match
  vector<int> m_vOffsets;
};

#endif  // #ifndef __REGEXP_H
