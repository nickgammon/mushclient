// methods_xml.cpp

// XML load/save stuff

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"

// Implements:

//    ExportXML
//    GetEntity
//    GetXMLEntity
//    ImportXML
//    Reset
//    SetEntity


extern CString strMacroDescriptions [MACRO_COUNT];
extern CString strKeypadNames [eKeypad_Max_Items];



BSTR CMUSHclientDoc::GetEntity(LPCTSTR Name) 
{
	CString strResult;

  m_CustomEntityMap.Lookup (Name, strResult);

	return strResult.AllocSysString();
}   // end of CMUSHclientDoc::GetEntity



long CMUSHclientDoc::ImportXML(LPCTSTR XML) 
{
long iCount = 0;

  CMemFile f ((unsigned char *)  XML, strlen (XML));
  CArchive ar (&f, CArchive::load);

    try
    {

    if (IsArchiveXML (ar))
      {

      UINT iTriggers = 0;
      UINT iAliases = 0;
      UINT iTimers = 0;
      UINT iMacros = 0;
      UINT iVariables = 0;
      UINT iColours = 0;
      UINT iKeypad = 0;
      UINT iPrinting = 0;

      // do it
      Load_World_XML (ar, 
                      // don't load plugins or general world config here  (note, this sets XML_OVERWRITE)
                      (unsigned long) ~(XML_PLUGINS | XML_NO_PLUGINS | XML_GENERAL | XML_PASTE_DUPLICATE | XML_IMPORT_MAIN_FILE_ONLY), 
                      0,          // load flags
                      &iTriggers,  
                      &iAliases,   
                      &iTimers,    
                      &iMacros,    
                      &iVariables, 
                      &iColours,   
                      &iKeypad,    
                      &iPrinting);  

      iCount =  iTriggers +  
                iAliases +   
                iTimers +    
                iMacros +    
                iVariables + 
                iColours +   
                iKeypad +    
                iPrinting;  

      }
    else
      iCount = -1;    // not in XML

     } // end of try block
  catch (CArchiveException* ) 
    {
    iCount = -1;    // error parsing XML
    }

	return iCount;
}  // end of CMUSHclientDoc::ImportXML


/*
Type:
  0 = trigger
  1 = alias
  2 = timer
  3 = macro
  4 = variable
  5 = keypad

  */

BSTR CMUSHclientDoc::ExportXML(short Type, LPCTSTR Name) 
{
	CString strResult;
  CString strName = Name;

  // trim spaces, force name to lower-case
  CheckObjectName (strName, false);

  char * p = NULL;

  try
    {
    CMemFile f;      // open memory file for writing
    CArchive ar(&f, CArchive::store);


    // see if trigger exists, if not return EMPTY

    switch (Type)
      {
      case 0:   // trigger
        {
        CTrigger * t;
        if (GetTriggerMap ().Lookup (strName, t))
          {
          Save_Header_XML (ar, "triggers", false);
          Save_One_Trigger_XML (ar, t);
          Save_Footer_XML (ar, "triggers");
          } // end of item existing
        }
        break;

      case 1:   // alias
        {
        CAlias * t;
        if (GetAliasMap ().Lookup (strName, t))
          {
          Save_Header_XML (ar, "aliases", false);
          Save_One_Alias_XML (ar, t);
          Save_Footer_XML (ar, "aliases");
          } // end of item existing
        }
        break;

      case 2:   // timer
        {
        CTimer * t;
        if (GetTimerMap ().Lookup (strName, t))
          {
          Save_Header_XML (ar, "timers", false);
          Save_One_Timer_XML (ar, t);
          Save_Footer_XML (ar, "timers");
          } // end of item existing
        }
        break;

      case 3:   // macro
        {
        for (int i = 0; i < NUMITEMS (strMacroDescriptions); i++)
          {
          if (strMacroDescriptions [i].CompareNoCase (strName) == 0)
            {
            Save_Header_XML (ar, "macros", false);
            Save_One_Macro_XML (ar, i);
            Save_Footer_XML (ar, "macros");
            } // end of item existing
          } // end of finding which one
        }
        break;

      case 4:   // variable
        {
        CVariable * t;
        if (GetVariableMap ().Lookup (strName, t))
          {
          Save_Header_XML (ar, "variables", false);
          Save_One_Variable_XML (ar, t);
          Save_Footer_XML (ar, "variables");
          } // end of item existing
        }
        break;

      case 5:   // keypad
        {
        for (int i = 0; i < NUMITEMS (strKeypadNames); i++)
          {
          if (strKeypadNames [i].CompareNoCase (strName) == 0)
            {
            Save_Header_XML (ar, "keypad", false);
            Save_One_Keypad_XML (ar, i);
            Save_Footer_XML (ar, "keypad");
            } // end of item existing
          } // end of finding which one

        }
        break;

      } // end of switch

    ar.Close();

    int nLength = f.GetLength ();
    p = (char *) f.Detach ();

    strResult = CString (p, nLength);

    free (p);   // remove memory allocated in CMemFile
    p = NULL;

    }   // end of try block

  catch (CException* e)
	  {
    if (p)
      free (p);   // remove memory allocated in CMemFile
	  e->Delete();
    strResult.Empty ();
	  }   // end of catch


	return strResult.AllocSysString();
}   // end of CMUSHclientDoc::ExportXML


void CMUSHclientDoc::SetEntity(LPCTSTR Name, LPCTSTR Contents) 
{
  if (strlen (Contents) == 0)
    m_CustomEntityMap.RemoveKey (Name);
  else
    m_CustomEntityMap.SetAt (Name, Contents);
 
}   // end of CMUSHclientDoc::SetEntity


BSTR CMUSHclientDoc::GetXMLEntity(LPCTSTR Entity) 
{
	CString strResult;
  CString strEntity = Entity;
  CXMLparser parser;

  try 
    {
    strResult = parser.Get_XML_Entity (strEntity);
    }   // end of try
  catch(CException* e)
    {
    e->Delete ();
    } // end of catch

	return strResult.AllocSysString();
}  // end of CMUSHclientDoc::GetXMLEntity



// reset MXP
void CMUSHclientDoc::Reset() 
{
MXP_Off ();
}   // end of CMUSHclientDoc::Reset


