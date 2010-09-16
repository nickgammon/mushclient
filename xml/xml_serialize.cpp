// xml_serialize.cpp : XML world serialization (load/save)
//

#include "stdafx.h"
#include "..\MUSHclient.h"
#include "..\doc.h"
#include "..\MUSHview.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


void CMUSHclientDoc::Serialize_World_XML (CArchive& ar)
  {
	if (ar.IsStoring())
	  {
    CPlugin * pSavedPlugin = m_CurrentPlugin;
    m_CurrentPlugin = NULL;   // make sure we save main triggers etc.
    
    try
      {
    // ensure world has an ID
      if (m_strWorldID.IsEmpty ())
        m_strWorldID = GetUniqueID ();

      Save_World_XML (ar, (unsigned long) ~0);    // save all options

      // ensure all plugins save their state right now :)
      for (POSITION pos = m_PluginList.GetHeadPosition (); pos; )
          m_PluginList.GetNext (pos)->SaveState ();

      }

    catch (CException *)
      {    
      m_CurrentPlugin = pSavedPlugin;
      throw;
      }

    m_CurrentPlugin = pSavedPlugin;
    }
  else
    { // loaidng
    Load_World_XML (ar, (unsigned long) ~(XML_PLUGINS | XML_NO_PLUGINS));    // load all options, except plugins
    m_bLoaded = true;   // this world has been loaded from disk
    }


  }  // end of CMUSHclientDoc::Serialize_World_XML 


/*

We will auto-detect XML files by looking for certain initial tags,
preceded by some whitespace. To be reasonable, we expect this to occur
within the first 500 bytes (ie. we can tolerate around 485 spaces).

  Thus, a valid file might be:

  <triggers defaults="1">
    <trigger
     match="Your victim is no longer logged on."
     name="Skill_BackStab_Failed_NoTarget"
     script="Casting_False"
    >
    </trigger>
  </triggers>
    
  */

#define COMPARE_MEMORY(arg) memcmp (p, arg, strlen (arg)) == 0

static const char * sigs [] =
  {
  "<?xml",
  "<!--",
  "<!DOCTYPE",
  "<muclient",
  "<world",
  "<triggers",
  "<aliases",
  "<timers",
  "<macros",
  "<variables",
  "<colours",
  "<keypad",
  "<printing",
  "<comment",
  "<include",
  "<plugin",
  "<script",
  };

// returns true if archive turns out to be XML
bool IsArchiveXML (CArchive& ar)
  {

  // auto-detect XML files

  char buf [500],     // should be even number of bytes in case Unicode
       buf2 [500];

  memset (buf, 0, sizeof (buf));
  ar.GetFile ()->Read (buf, sizeof (buf) - 2);   // allow for Unicode 00 00
  ar.GetFile ()->SeekToBegin ();  // back to start for further serialisation

  // look for Unicode (FF FE)
  if ((unsigned char) buf [0] == 0xFF &&
      (unsigned char) buf [1] == 0xFE)
    WideCharToMultiByte (CP_UTF8, 0, (LPCWSTR) &buf [2], -1, buf2, sizeof buf2, NULL, NULL);
  else
    // look for UTF-8 indicator bytes (EF BB BF)
    if ((unsigned char) buf [0] == 0xEF &&
        (unsigned char) buf [1] == 0xBB &&
        (unsigned char) buf [2] == 0xBF)
      strcpy (buf2, &buf [3]);   // skip them
  else 
    strcpy (buf2, buf);

  char * p = buf2;

  // skip leading whitespace
  while (isspace (*p))
    p++;

  // can't see them squeezing much into less than 15 chars
  //  (eg. minimum would be <macros></macros> )
  if (strlen (p) < 15)
    return false;

  for (int i = 0; i < NUMITEMS (sigs); i++)
    if (COMPARE_MEMORY (sigs [i]))
      return true;

  return false;

  } // end of IsArchiveXML

void SeeIfBase64 (CString & strText)
  {
  const char * p = strText;
  char c;
  bool bBase64 = true;
  int iCount;

  for (iCount = 0 ; c = *p; p++)
    if (isspace (c))
      continue;
    else
    if (isalnum (c) || c == '+' || c == '/' || c == '=')
      iCount++;
    else
      {
      bBase64 = false;
      break;
      }
  
  // might be base 64 if only base64 characters, and mod 4 non-space length
  // (if base64 can't be plain XML because plain XML will have < in it)

  if (bBase64 && (iCount % 4 == 0))
    strText = decodeBase64 (strText);

  } // end of SeeIfBase64

// returns true if clipboard turns out to be XML
bool IsClipboardXML (void)
  {
CString strContents; 

  if (!GetClipboardContents (strContents, false, false))
    return false;

// see if base64 encoded

  SeeIfBase64 (strContents);

  CMemFile f ((unsigned char *) (const char *) strContents, strContents.GetLength ());
  CArchive ar (&f, CArchive::load);
  
  if (IsArchiveXML (ar))
    return true;

  return false;
  } // end of IsClipboardXML

static int CompareAlias (const void * elem1, const void * elem2)
  {
  CAlias * alias1 = (*((CAlias **) elem1));
  CAlias * alias2 = (*((CAlias **) elem2));

  if (alias1->name < alias2->name)
    return -1;
  else if (alias1->name > alias2->name)
    return 1;
  else
    return 0;

  }   // end of CompareAlias


void SortAliases (CAliasMap & AliasMap,
                  CAliasArray & AliasArray)
  {
int iCount = AliasMap.GetCount ();
int i;
CString strAliasName;
CAlias * pAlias;
POSITION pos;

  AliasArray.SetSize (iCount);

  // extract pointers into a simple array
  for (i = 0, pos = AliasMap.GetStartPosition(); pos; i++)
    {
     AliasMap.GetNextAssoc (pos, strAliasName, pAlias);
     AliasArray.SetAt (i, pAlias); 
    }


  // sort the array
  qsort (AliasArray.GetData (), 
         iCount,
         sizeof (CAlias *),
         CompareAlias);

  } // end of SortAliases


static int CompareVariable (const void * elem1, const void * elem2)
  {
  CVariable * variable1 = (*((CVariable **) elem1));
  CVariable * variable2 = (*((CVariable **) elem2));

  if (variable1->strLabel < variable2->strLabel)
    return -1;
  else if (variable1->strLabel > variable2->strLabel)
    return 1;
  else
    return 0;

  }   // end of CompareVariable


void SortVariables (CVariableMap & VariableMap,
                    CVariableArray & VariableArray)
  {
int iCount = VariableMap.GetCount ();
int i;
CString strVariableName;
CVariable * pVariable;
POSITION pos;

  VariableArray.SetSize (iCount);

  // extract pointers into a simple array
  for (i = 0, pos = VariableMap.GetStartPosition(); pos; i++)
    {
     VariableMap.GetNextAssoc (pos, strVariableName, pVariable);
     VariableArray.SetAt (i, pVariable); 
    }


  // sort the array
  qsort (VariableArray.GetData (), 
         iCount,
         sizeof (CVariable *),
         CompareVariable);

  } // end of SortVariables


