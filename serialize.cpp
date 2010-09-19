// serialize.cpp : world serialization (load/save)
//

#include "stdafx.h"
#include "MUSHclient.h"
#include "doc.h"
#include "MUSHview.h"
#include "mainfrm.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// debugging - true means it will use (or attempt to use) XML
#define XML_LOAD 
#define XML_SAVE 

void SortAliases (CAliasMap & AliasMap,
                  CAliasArray & AliasArray);

void SortVariables (CVariableMap & VariableMap,
                    CVariableArray & VariableArray);

extern tConfigurationNumericOption OptionsTable [];
extern tConfigurationAlphaOption AlphaOptionsTable [];



/////////////////////////////////////////////////////////////////////////////
// CMUSHclientDoc serialization


void CMUSHclientDoc::Serialize(CArchive& ar)
{

	if (ar.IsStoring())
    {
    Serialize_World_XML (ar);
    }
	    // end of storing

	else
	{  //  loading

  CString strMsg;
  strMsg = TFormat ("Opening world \"%s\"", (LPCTSTR) ar.GetFile ()->GetFilePath ());

  Frame.SetStatusMessageNow (strMsg);

  SetPathName((LPCTSTR) ar.GetFile ()->GetFilePath ());  // see: http://www.gammon.com.au/forum/?id=10596

  if (IsArchiveXML (ar))
    Serialize_World_XML (ar);
  else
    {
    ::TMessageBox ("File does not have a valid MUSHclient XML signature.",
                     MB_ICONSTOP);
    AfxThrowArchiveException (CArchiveException::badSchema);
    }

  if (m_bCustom16isDefaultColour)
    {
    m_iForeColour = 15;   // use custom colour 16
    m_iBackColour = 15;
    m_iFlags |= COLOUR_CUSTOM;
    }

  // if defaults are wanted, overwrite what we loaded with them :)

  OnFileReloaddefaults ();

  if (m_bMapFailureRegexp && !m_strMappingFailure.IsEmpty ())
    {
    try
      {
      m_MapFailureRegexp = new t_regexp (m_strMappingFailure, (m_bUTF_8 ? PCRE_UTF8 : 0));
      }
    catch (CException* e)
      {
      char sMessage [1000];
      e->GetErrorMessage (sMessage, sizeof sMessage);
      ::UMessageBox (TFormat ("Error \"%s\" processing mapping failure regular expression \"%s\"", 
                       sMessage,
                       (LPCTSTR) m_strMappingFailure));
      e->Delete ();
      m_bMapFailureRegexp = FALSE; 
      m_MapFailureRegexp = NULL;
      } // end of catch

    } // end compiling regular expression

  // kick off speed walk timer
  m_pTimerWnd->ChangeTimerRate (m_iSpeedWalkDelay);

  // remember loaded option values

  long i;

  for (i = 0; OptionsTable [i].pName; i++)
    if (!m_NumericConfiguration [i]->bInclude)
      m_NumericConfiguration [i]->iValue = GetOptionItem (i);

  for (i = 0; AlphaOptionsTable [i].pName; i++)
    if (!m_AlphaConfiguration [i]->bInclude)
      m_AlphaConfiguration [i]->sValue = GetAlphaOptionItem (i);

	}   // end of loading 

}
