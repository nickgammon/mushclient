// mxpEnd.cpp - MXP end tag

// eg. </send>

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



void CMUSHclientDoc::MXP_EndTag (CString strTag)
  {

bool bSecure = MXP_Secure () ;

  MXP_Restore_Mode ();  // cancel secure-once mode

CString strName;

  GetWord (strName, strTag);

  if (!IsValidName (strName))
    {
    MXP_error (DBG_ERROR, errMXP_InvalidElementName,
              TFormat ("Invalid MXP tag name: </%s>", 
              (LPCTSTR) m_strMXPstring)); 
    return;
    }

  strName.MakeLower (); // case insensitive

  // should just have tag name, not </tag blah blah>
  if (!strTag.IsEmpty ())
    MXP_error (DBG_WARNING, wrnMXP_ArgumentsToClosingTag,
              TFormat ("Closing MXP tag </%s %s> has inappropriate arguments", 
              (LPCTSTR) strName,
              (LPCTSTR) strTag)); 


  // make sure tag is in active taglist
  // eg. </unknown> will not close any open tags
  // this test effectively checks that tag is known (otherwise it won't be in the list)

  CActiveTag * pTag = NULL;

  for (POSITION pos = m_ActiveTagList.GetTailPosition (); pos; )
    {
    pTag = m_ActiveTagList.GetPrev (pos);

    if (pTag->strName == strName)
      break;
    else
      {
      // check we don't cross over some secure tags when finding it
      if (!bSecure && pTag->bSecure)
        {
        MXP_error (DBG_WARNING, wrnMXP_OpenTagBlockedBySecureTag,
                  TFormat ("Cannot close open MXP tag <%s> "
                                         "- blocked by secure tag <%s>", 
                                        (LPCTSTR) strName,
                                        (LPCTSTR) pTag->strName)); 
        return;
        }
      pTag = NULL;
      }

    } // end of doing each one

  if (!pTag)
    {
    MXP_error (DBG_WARNING, wrnMXP_OpenTagNotThere,
              TFormat ("Closing MXP tag </%s> does not have corresponding opening tag", 
              (LPCTSTR) strName)); 
    return;
    }

  if (!bSecure && pTag->bSecure)
    {
    MXP_error (DBG_WARNING, wrnMXP_TagOpenedInSecureMode,
              TFormat ("Cannot close open MXP tag <%s> "
                       "- it was opened in secure mode.", 
                      (LPCTSTR) strName)); 
    return;
    }

  // we know it is in the list - close all tags until we reach this one
  // eg.  <b> <i> </b> </i>  
  //  in the above example the </b> will also close the <i>

  while (true)
    {

    pTag = m_ActiveTagList.RemoveTail ();
    CString strTag = pTag->strName;
    delete pTag;
    
    if (strTag != strName)
      MXP_error (DBG_WARNING, wrnMXP_ClosingOutOfSequenceTag,
                TFormat ("Closing out-of-sequence MXP tag: <%s>", 
                (LPCTSTR) strTag)); 

    MXP_CloseTag (strTag);


    if (strTag == strName)
      break;  // stop once we have found the tag we are supposed to close
    }

  } // end of CMUSHclientDoc::MXP_EndTag
