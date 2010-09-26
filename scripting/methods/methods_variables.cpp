// methods_variables.cpp

// Related to variables

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\errors.h"

// Implements:

//    DeleteVariable
//    GetVariable
//    GetVariableList
//    SetVariable



// world.SetVariable (variable_name, new_contents) - sets the named variable's contents
//                                                 - creating it if necessary

long CMUSHclientDoc::SetVariable(LPCTSTR VariableName, LPCTSTR Contents) 
{
CString strVariableName = VariableName;
CVariable * variable_item;
long nStatus;

  // return if bad name
  if (nStatus = CheckObjectName (strVariableName))
    return nStatus;

  // get rid of old variable, if any
  if (GetVariableMap ().Lookup (strVariableName, variable_item))
    delete variable_item;

  // create new variable item and insert in variable map
  GetVariableMap ().SetAt (strVariableName, variable_item = new CVariable);
  m_bVariablesChanged = true;
//  SetModifiedFlag (TRUE); // set flag instead now
  variable_item->nUpdateNumber = App.GetUniqueNumber ();   // for concurrency checks

  // set up variable item contents
  variable_item->strLabel = VariableName;
  variable_item->strContents = Contents;

	return eOK;
}    // end of CMUSHclientDoc::SetVariable

// world.GetVariable (variable_name) - gets details about the named variable
//                                     returns "EMPTY" variable if not there
//                                     returns "NULL" variable if a bad name

VARIANT CMUSHclientDoc::GetVariable(LPCTSTR VariableName) 
{
CString strVariableName = VariableName;
CVariable * variable_item;
VARIANT vaResult;

  VariantInit(&vaResult);

  vaResult.vt = VT_NULL;

  // return if bad name, if so return NULL
  if (CheckObjectName (strVariableName))
    return vaResult;

  vaResult.vt = VT_EMPTY;

  // see if variable exists, if not return EMPTY
  if (!GetVariableMap ().Lookup (strVariableName, variable_item))
	  return vaResult;

  SetUpVariantString (vaResult, variable_item->strContents);

  return vaResult;

}  // end of CMUSHclientDoc::GetVariable

// world.GetVariableList - returns a variant array which is a list of variable names

VARIANT CMUSHclientDoc::GetVariableList() 
{
  COleSafeArray sa;   // for variable list

  CString strVariableName;
  CVariable * variable_item;

  POSITION pos;
  long iCount;
  
  // put the Variables into the array
  if (!GetVariableMap ().IsEmpty ())    // cannot create empty dimension
    {
    sa.CreateOneDim (VT_VARIANT, GetVariableMap ().GetCount ());

    for (iCount = 0, pos = GetVariableMap ().GetStartPosition(); pos; )
      {
      GetVariableMap ().GetNextAssoc (pos, strVariableName, variable_item);

      // the array must be a bloody array of variants, or VBscript kicks up
      COleVariant v (strVariableName);
      sa.PutElement (&iCount, &v);
      iCount++;
      }      // end of looping through each Variable
    } // end of having at least one

	return sa.Detach ();
}   // end of CMUSHclientDoc::GetVariableList

// world.DeleteVariable (variable_name) - deletes the named variable

long CMUSHclientDoc::DeleteVariable(LPCTSTR VariableName) 
{
CString strVariableName = VariableName;
CVariable * variable_item;
long nStatus;

  // return if bad name
  if (nStatus = CheckObjectName (strVariableName))
    return nStatus;

  if (!GetVariableMap ().Lookup (strVariableName, variable_item))
    return eVariableNotFound;

  // the variable seems to exist - delete its pointer
  delete variable_item;

  // now delete its entry
  if (!GetVariableMap ().RemoveKey (strVariableName))
    return eVariableNotFound;

  if (!m_CurrentPlugin) // plugin mods don't really count
    SetModifiedFlag (TRUE);   // document has changed

	return eOK;
} // end of DeleteVariable

