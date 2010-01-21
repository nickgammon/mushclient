// Replace.cpp - for replacing one string with another
//

#include "stdafx.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CString Replace (const CString strSource, 
                 const CString strTarget,
                 const CString strReplacement,
                 const bool bAll)
  {
CString strResult = strSource;  // make copy so we can change repeatedly

// simplify - use CString.Replace if we can
if (bAll)
  {
  strResult.Replace (strTarget, strReplacement);   
  return strResult;
  } // end of all wanted

int iTargetLength = strTarget.GetLength ();
int iReplacementLength = strReplacement.GetLength ();
int iStartPos = 0;
int iFoundPos;

  // loop until we are at end of source string
  if (!strTarget.IsEmpty ())
    while (iStartPos < strResult.GetLength ())
      {
      // search for string, exit loop if not found
      if ((iFoundPos = strResult.Mid (iStartPos).Find (strTarget)) == -1)
        break;    // done!
    
      iFoundPos += iStartPos;   // where it really was

      strResult = strResult.Left (iFoundPos) + 
                  strReplacement +
                  strResult.Mid (iFoundPos + iTargetLength);

      // skip replacement text so we don't search inside it
      iStartPos = iFoundPos + iReplacementLength;

      // only do one if that is all that is wanted
      if (!bAll)
        break;

      } // end of replacement loop
  
  return strResult;
  } // end of Replace