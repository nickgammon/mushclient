// methods_worlds.cpp

// World management (finding other open world files)

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\errors.h"

// Implements:

//    GetWorld
//    GetWorldById
//    GetWorldIdList
//    GetWorldList


LPDISPATCH CMUSHclientDoc::GetWorld(LPCTSTR WorldName) 
{
  for (POSITION docPos = App.m_pWorldDocTemplate->GetFirstDocPosition();
      docPos != NULL; )
    {
    CMUSHclientDoc * pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(docPos);
  
    if (pDoc->m_mush_name.CompareNoCase (WorldName) == 0)
      return pDoc->GetIDispatch (TRUE);
    } // end of doing each document

	return NULL;
}    // end of CMUSHclientDoc::GetWorld

VARIANT CMUSHclientDoc::GetWorldList() 
{
  COleSafeArray sa;   // for list

  CMUSHclientDoc * pDoc;
  long iCount = 0;
  POSITION pos;

  // count number of worlds
  for (pos = App.m_pWorldDocTemplate->GetFirstDocPosition(); pos != NULL; iCount++)
    pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(pos);

  if (iCount) // cannot create empty array dimension
    {
    sa.CreateOneDim (VT_VARIANT, iCount);
  
    // put the worlds into the array
    for (iCount = 0, pos = App.m_pWorldDocTemplate->GetFirstDocPosition(); pos != NULL; iCount++)
      {
      pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(pos);

      // the array must be a bloody array of variants, or VBscript kicks up
      COleVariant v (pDoc->m_mush_name);
      sa.PutElement (&iCount, &v);
      }      // end of looping through each world
    } // end of having at least one

	return sa.Detach ();
}   // end of CMUSHclientDoc::GetWorldList


VARIANT CMUSHclientDoc::GetWorldIdList() 
{
  COleSafeArray sa;   // for list

  CMUSHclientDoc * pDoc;
  long iCount = 0;
  POSITION pos;

  // count number of worlds
  for (pos = App.m_pWorldDocTemplate->GetFirstDocPosition(); pos != NULL; iCount++)
    pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(pos);

  if (iCount) // cannot create empty array dimension
    {
    sa.CreateOneDim (VT_VARIANT, iCount);
  
    // put the worlds into the array
    for (iCount = 0, pos = App.m_pWorldDocTemplate->GetFirstDocPosition(); pos != NULL; iCount++)
      {
      pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(pos);

      // the array must be a bloody array of variants, or VBscript kicks up
      COleVariant v (pDoc->m_strWorldID);
      sa.PutElement (&iCount, &v);
      }      // end of looping through each world
    } // end of having at least one

	return sa.Detach ();
}   // end of CMUSHclientDoc::GetWorldIdList

LPDISPATCH CMUSHclientDoc::GetWorldById(LPCTSTR WorldID) 
{
  for (POSITION docPos = App.m_pWorldDocTemplate->GetFirstDocPosition();
      docPos != NULL; )
    {
    CMUSHclientDoc * pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(docPos);
  
    if (pDoc->m_strWorldID.CompareNoCase (WorldID) == 0)
      return pDoc->GetIDispatch (TRUE);
    } // end of doing each document

	return NULL;
}   // end of CMUSHclientDoc::GetWorldById

