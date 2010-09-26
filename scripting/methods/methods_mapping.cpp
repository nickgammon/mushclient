// methods_mapping.cpp

// For "mapping" (capturing speedwalks as you walk around)

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\errors.h"

// Implements:

//    AddMapperComment
//    AddToMapper
//    DeleteAllMapItems
//    DeleteLastMapItem
//    EnableMapping
//    GetMappingCount
//    GetMappingItem
//    GetMappingString



long CMUSHclientDoc::AddToMapper(LPCTSTR Direction, LPCTSTR Reverse) 
{
  // look for bad characters
  if (_tcspbrk (Direction, "{}()/\\"))  
    return eBadMapItem;

  if (_tcspbrk (Reverse, "{}()/\\"))  
    return eBadMapItem;

  // can't both be empty
  if (Direction [0] == 0 && Reverse [0] == 0)
    return eBadMapItem;

  CString strDirection;

  strDirection = Direction;
  strDirection += "/";
  strDirection += Reverse;

  m_strMapList.AddTail (strDirection);
	
  DrawMappingStatusLine ();    // update status line

  return eOK;
}  // end of CMUSHclientDoc::AddToMapper


long CMUSHclientDoc::AddMapperComment(LPCTSTR Comment) 
{
  // look for bad characters
  if (_tcspbrk (Comment, "{}()/\\"))  
    return eBadMapItem;

  CString strDirection;

  strDirection = "{";
  strDirection += Comment;
  strDirection += "}";

  m_strMapList.AddTail (strDirection);
	
  DrawMappingStatusLine ();    // update status line

  return eOK;
}   // end of CMUSHclientDoc::AddMapperComment


long CMUSHclientDoc::GetMappingCount() 
{
	return m_strMapList.GetCount ();
}   // end of CMUSHclientDoc::GetMappingCount

VARIANT CMUSHclientDoc::GetMappingItem(long Item) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_NULL;

  if (Item < 0 || Item >= m_strMapList.GetCount ())
    return vaResult;

  POSITION pos = m_strMapList.FindIndex (Item);

  if (pos == NULL)
    return vaResult;

  CString strItem = m_strMapList.GetAt (pos);

  SetUpVariantString (vaResult, strItem);

	return vaResult;
}   // end of CMUSHclientDoc::GetMappingItem

VARIANT CMUSHclientDoc::GetMappingString() 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  CString str = CalculateSpeedWalkString ();

  SetUpVariantString (vaResult, str);

	return vaResult;
}  // end of CMUSHclientDoc::GetMappingString


long CMUSHclientDoc::DeleteLastMapItem() 
{

  if (m_strMapList.IsEmpty ())
    return eNoMapItems;

  m_strMapList.RemoveTail ();

  DrawMappingStatusLine ();    // update status line

	return eOK;
}   // end of CMUSHclientDoc::DeleteLastMapItem

long CMUSHclientDoc::DeleteAllMapItems() 
{
  if (m_strMapList.IsEmpty ())
    return eNoMapItems;

  m_strMapList.RemoveAll ();

  DrawMappingStatusLine ();    // update status line

	return eOK;
}   // end of CMUSHclientDoc::DeleteAllMapItems


// enable/disable auto-mapper
void CMUSHclientDoc::EnableMapping(BOOL Enabled) 
{
 m_bMapping =	Enabled != 0;
}   // end of CMUSHclientDoc::EnableMapping
