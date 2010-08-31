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

  string LastTarget () const;
  int LastError () const;

  // returns a wildcard by number
  string GetWildcard (const int iNumber) const;
  // returns a wildcard by name
  string GetWildcard (const string sName) const;

  bool GetWildcardOffsets (const int iNumber, int& iLeft, int& iRight) const;
  bool GetWildcardOffsets (const string& sName, int& iLeft, int& iRight) const;

  int MatchedCapturesCount () const;
  long MatchAttempts () const;

  LONGLONG TimeTaken () const;

  int GetInfo (int info_type, void* out) const;
  bool DupNamesAllowed () const;

  static const char* ErrorCodeToString(const int code);
  static bool CheckPattern (const char* pattern, const int iOptions,
                            const char** error, int* errorOffset);

private:
  void AcquirePattern(pcre* program, pcre_extra* extra);
  void ReleasePattern();

  string m_sTarget;       // the string we last matched on (to extract wildcards from)
  int m_iCount;           // count of matching wildcards
  vector<int> m_vOffsets; // pairs of offsets from match

  int m_iExecutionError; // error code from last execution

  // TODO: Consider moving m_iMatchAttempts and m_iTimeTaken out of t_regexp and into a
  // class that actually uses them, like CAlias and CTrigger. It's just extra bookkeeping
  // that t_regexp itself has no need for.
  long m_iMatchAttempts; // number of times a match has been attempted
  LONGLONG m_iTimeTaken; // total time taken to match

  pcre * m_program;     // the program itself
  pcre_extra * m_extra; // extra stuff for speed
};

#endif  // #ifndef __REGEXP_H
