// mxputils.cpp - MXP utilities


#include "stdafx.h"
#include "..\MUSHclient.h"

#include "..\doc.h"
#include "..\MUSHview.h"
#include "..\mainfrm.h"
#include "mxp.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


// extract a word, return it in strResult, remove from str
// return true if end, false if not end
bool GetWord (CString & strResult, CString & str)
  {
  const char * p = str;
  const char * pStart;
  char quote;
  int len = 0;

  // skip leading spaces
  while (*p == ' ')
    p++;

  // is it quoted string?
  if (*p == '\'' || *p == '\"')
    {
    quote = *p;
    pStart = ++p; // bypass opening quote
    for ( ; *p != quote && *p; p++)
      len++;    // count up to closing quote
    strResult = CString (pStart, len);
    str = ++p; // return rest of line
    return false; // not end, even if empty
    }

  // where word starts
  pStart = p;

  // is it a word or number?
  if (isalnum (*p) ||
      *p == '+' ||
      *p == '-')
    {
    // skip initial character, and then look for terminator
    for (p++, len++; *p; p++, len++)
      {
      if (isalnum (*p))
        continue;
      if (*p == '_' ||  // underscore
          *p == '-' ||  // hyphen
          *p == '.' ||  // period
          *p == ','     // comma
          )
        continue;

      break;    // stop if not alpha, digit, or above special characters
      }
    }
  else
  // is it a colour?  ie. #xxxxxx
  if (*p == '#')
    {
    for (p++, len++; *p; p++, len++)
      {
      if (isxdigit (*p))
        continue;
      break;    // stop if not hex digit
      }
    }
  else
  // is it an argument?  ie. &xxx;
  if (*p == '&')
    {
    for (p++, len++; *p && *p != ';'; p++, len++)
      ; // keep going until ;
    // include the ; in the word
    if (*p == ';')
      {
      len++;
      p++;
      }
    }
  else
    if (*p) // provided not end of line
      // assume single character, eg. '=' or ',' or something
      {
      len = 1;
      p++;
      }

  strResult = CString (pStart, len);
  str = p; // return rest of line
  return len == 0;  // true if nothing found
  
  } // end of GetWord

// builds an argument list from a string (eg. "color=red blink=yes number=10")
// un-named arguments are numbered from 1 upwards

// returns true if invalid name (eg. *434='22' )
// returns false if OK

bool CMUSHclientDoc::BuildArgumentList (CArgumentList & ArgumentList, 
                                        CString strTag) 
  {
CArgument * pArgument;
int iArgumentNumber = 0;
bool bEnd;
CString strArgumentName;
CString strEquals;
CString strArgumentValue;


// first get rid of old arguments

  DELETE_LIST (ArgumentList);

// collect all arguments

  // get first word
  bEnd = GetWord (strArgumentName, strTag);

  while (!bEnd)
    {

    if (strArgumentName == "/")
      {
      strTag.TrimLeft ();
      if (!strTag.IsEmpty ())
          {
          MXP_error (DBG_ERROR, errMXP_InvalidArgumentName,
                    TFormat ("Invalid parameter name: \"%s\"", 
                    (LPCTSTR) strArgumentName)); 
          return true;
          }
    
      // NB - not implemented yet - we have detected an empty tag.
      //      eg.  <sound blah blah />

      return false;   // OK return

      } // end of / at end of list

// #error fix it here ...

    // is it folllowed by equals?
    bEnd = GetWord (strEquals, strTag);
    if (strEquals == "=") // yes
      {
      if (!IsValidName (strArgumentName))   // check name valid
        {
        MXP_error (DBG_ERROR, errMXP_InvalidArgumentName,
                  TFormat ("Invalid parameter name: \"%s\"", 
                  (LPCTSTR) strArgumentName)); 
        return true;
        }

      bEnd = GetWord (strArgumentValue, strTag);  // so get value
      if (bEnd)
        {
        MXP_error (DBG_ERROR, errMXP_NoArgument,
                    TFormat ("No argument value supplied for: \"%s\"", 
                  (LPCTSTR) strArgumentName)); 
        return true;
        }

      strArgumentName.MakeLower ();
      // named arguments don't have a numbered position
      pArgument = new CArgument (strArgumentName, strArgumentValue, 0);
      ArgumentList.AddTail (pArgument);
      bEnd = GetWord (strArgumentName, strTag); // get next argument
      }   // end of name=value
    else
      { // positional argument, no name=value
      // thus, name is value
      pArgument = new CArgument ("", strArgumentName, ++iArgumentNumber);
      ArgumentList.AddTail (pArgument);
      strArgumentName = strEquals;    // and strEquals is next argument, if any      
      }   // end of value alone
    }  // end of processing each argument                             

  return false; // all OK
  } // end of CMUSHclientDoc::BuildArgumentList 

// get an argument by name or position
CString GetArgument (const CArgumentList & ArgumentList, 
                     CString strName, 
                     const int iPosition, 
                     const bool bLowerCase)
  {
  CArgument * pArgument;
  CArgument * pFoundArgument = NULL;
  CString strValue;

  strName.MakeLower ();   // name is not case-sensitive

  for (POSITION pos = ArgumentList.GetHeadPosition (); pos; )
    {
    pArgument = ArgumentList.GetNext (pos);

    // keywords are not, strictly speaking, arguments
    if (pArgument->bKeyword)
      continue;

    // found name?
    if (!strName.IsEmpty () && pArgument->strName == strName)
      {
      pFoundArgument = pArgument;
      break;
      }
    
    // remember position match in case no name match
    if (iPosition && pArgument->iPosition == iPosition)
      pFoundArgument = pArgument;

    }

  // if we found it, note used
  if (pFoundArgument)
    {
    // found name, take value
    strValue = pFoundArgument->strValue;
    pFoundArgument->bUsed = true;
    if (bLowerCase)
      strValue.MakeLower ();
    }

  // return value - will be empty if not found
  return strValue;
  } // end of GetArgument


// get an keyword in argument list (eg. is OPEN there?)
bool GetKeyword (CArgumentList & ArgumentList, 
                     CString strName)
  {
  CArgument * pArgument;
  CString strValue;

  for (POSITION pos = ArgumentList.GetHeadPosition (); pos; )
    {
    pArgument = ArgumentList.GetNext (pos);
    // found keyword (something with no name)?
    if (pArgument->strValue.CompareNoCase (strName) == 0 && 
        pArgument->strName.IsEmpty ())
      {

      pArgument->bUsed = true;   // note it was used

      // any positional arguments after this one effectively are one lower
      // (because although we allocated it a number it doesn't deserve one)

      if (!pArgument->bKeyword)     // don't do it twice
        {
        pArgument->bKeyword = true; // note it for future reference
        while (pos)
          {
          pArgument = ArgumentList.GetNext (pos);
          if (pArgument->iPosition)
             pArgument->iPosition--;
          } // end of renumbering
        } // end of not renumbered yet

      return true;
      }
    }

  return false;
  } // end of GetKeyword

// tests a string to see if it is numeric
bool IsNumeric (const CString & str)
  {
const char * p = str;

  // bypass leading sign
  if (*p == '+' || *p == '-')
    p++;

  if (!*p)
    return false; // empty now? can't be a number

  for ( ; *p; p++)
    if (!isdigit (*p))
      return false;

  return true;
  }

// sets the requested colour - either colour name or #F1F2F3
// returns true if bad colour name
bool SetColour (const CString & strName, COLORREF & iColour)
  {
CString strNameFixed = strName;

  strNameFixed.TrimLeft ();
  strNameFixed.TrimRight ();
  strNameFixed.MakeLower ();

  // no colour? just leave it alone
  if (strNameFixed.IsEmpty ())
    return false;

  if (strNameFixed [0] == '#')
    {
    long i = 0;
    const char * p = strNameFixed;

    for (p++; *p && isxdigit (*p) ; p++)
      {
      int iNewDigit = toupper (*p);
      if (iNewDigit >= 'A')
        iNewDigit -= 7;
      if (i & 0xF000000L)   // check not too long
        return true;

    	i = (i << 4) + iNewDigit - '0';
      } // end of building up colour sequence

    // we need to change the order because of the endian-ness I suspect
    iColour = RGB (
                  GetBValue (i),
                  GetGValue (i),
                  GetRValue (i)
                  );

    } // end of #xxxxxx type colour
  else
    { // must be name
    CColours * colour_item;

    // look up colour name

    if (!App.m_ColoursMap.Lookup (strNameFixed, colour_item))
      return true; // not found

    iColour = colour_item->iColour;
    }   // end of colour name

  return false;   // good colour

  }

// takes a string like "apples,peaches,pears" and converts to a list of strings
void StringToList (const CString str, const char * delim, CStringList & thelist)
  {
  int iStart = 0, 
      iEnd;
  CString strFound;

  thelist.RemoveAll ();

  if (str.IsEmpty ())
    return;

  while ((iEnd = str.Find (delim, iStart)) != -1)
    {
    strFound = str.Mid (iStart, iEnd - iStart);
//    strFound.TrimLeft ();
//    strFound.TrimRight ();
    thelist.AddTail (strFound);
    iStart = iEnd + strlen (delim);  // bypass delimiter(s)
    } // end of adding each one

  // add final one after last delimiter

  strFound = str.Mid (iStart);                            
//  strFound.TrimLeft ();
//  strFound.TrimRight ();
  thelist.AddTail (strFound);

  } // end of StringToList 

// takes a list of strings and converts it to a string like "apples,peaches,pears" 
void ListToString (const CStringList & thelist, const char delim, CString & str)
  {
  int iCount = 0;

  str.Empty ();

  if (thelist.IsEmpty ())
    return;

  for (POSITION pos = thelist.GetHeadPosition (); pos; )
    {
    str += thelist.GetNext (pos);
    if (++iCount < thelist.GetCount ())
      str += delim;
    } // end of getting each one

  } // end of ListToString


bool IsValidName (const CString & str)
  {
  const char * p = str;

  // must start with a letter
  if (!isalpha (*p))
    return false;

  for ( ; *p; p++)
    {
    if (isalnum (*p))
      continue;
    if (*p == '_' ||  // underscore
        *p == '-' ||  // hyphen
        *p == '.'     // period
        )
      continue;

    return false;    // stop if not alpha, digit, or above special characters
    }

  return true;
  }


  // finds an action, adding it if necessary
CAction * CMUSHclientDoc::GetAction (const CString & strAction,
                                     const CString & strHint,
                                     const CString & strVariable)
  {

  // not needed if all these are empty
  if (strAction.IsEmpty () &&
      strHint.IsEmpty () &&
      strVariable.IsEmpty ())
      return NULL;

unsigned long iHash = MakeActionHash (strAction, strHint, strVariable);

  for (POSITION pos = m_ActionList.GetHeadPosition (); pos; )
    {
    CAction * pAction = m_ActionList.GetNext (pos);

    if (pAction->m_iHash == iHash &&
        pAction->m_strAction == strAction &&
        pAction->m_strHint == strHint &&
        pAction->m_strVariable == strVariable)
      {
      pAction->AddRef (); // one more user of it
      return pAction;
      }

    }   // end of lookup loop

  // here when action not found

  CAction * pAction = new CAction (strAction, strHint, strVariable, this);

  pAction->AddRef ();
  m_ActionList.AddTail (pAction);   
  
  return pAction;
  } // end of CMUSHclientDoc::GetAction



CAction::CAction (const CString & strAction,
                  const CString & strHint,
                  const CString & strVariable,
                  CMUSHclientDoc * pDoc) 
  { 
  m_iRefCount = 0;
  m_strAction = strAction;
  m_strHint = strHint;
  m_strVariable = strVariable;
  m_pDoc = pDoc;

  m_iHash = MakeActionHash (strAction, strHint, strVariable);

  }; // end of CAction::CAction  constructor

// call AddRef when another style uses this action

// BTW - AddRef and Release have nothing to do with COM - I just called them that
// because they perform a similar function, conceptually

void CAction::AddRef (void) 
  { 
  m_iRefCount++; 
  }; // end of CAction::AddRef

// call Release when a style no longer uses the action

void CAction::Release (void) 
  { 
  m_iRefCount--; 
  if (m_iRefCount <= 0)
    {
    POSITION pos = m_pDoc->m_ActionList.Find (this);
    ASSERT (pos);
    if (pos)
       m_pDoc->m_ActionList.RemoveAt (pos);
    delete this;
    } // this one not wanted any more
  }; // end of CAction::Release


void CMUSHclientDoc::CheckArgumentsUsed (const CString strTag, 
                                         CArgumentList & ArgumentList)
  {

  for (POSITION pos = ArgumentList.GetHeadPosition (); pos; )
    {
    CArgument *  pArgument = ArgumentList.GetNext (pos);

    if (!pArgument->bUsed)
      {
      if (pArgument->strName.IsEmpty ())
        MXP_error (DBG_WARNING, wrnMXP_UnusedArgument,
                   TFormat ("Unused argument (%i) for <%s>: %s", 
                   pArgument->iPosition,
                  (LPCTSTR) strTag,
                  (LPCTSTR) pArgument->strValue
                  ));     
      else
        MXP_error (DBG_WARNING, wrnMXP_UnusedArgument,
                   TFormat ("Unused argument for <%s>: %s=\"%s\"", 
                  (LPCTSTR) strTag,
                  (LPCTSTR) pArgument->strName,
                  (LPCTSTR) pArgument->strValue
                  ));
      } // end of unused argument
    } // end of checking each argument used

  } // end of CMUSHclientDoc::CheckArgumentsUsed 
