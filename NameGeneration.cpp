#include "stdafx.h"
#include "MUSHclient.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static CStringArray FirstNames;
static CStringArray MiddleNames;
static CStringArray LastNames;

bool bNamesRead = false;

// asterisk as the filename means forced reload
void ReadNames (const LPCTSTR sName, const bool bNoDialog)
  {

  bNamesRead = false;
  CString strFileName = sName;


    CString sBuf;
    BOOL bOK;

    FirstNames.RemoveAll ();
    MiddleNames.RemoveAll ();
    LastNames.RemoveAll ();

    // if file name not specified, take one from last time
    if (strFileName.IsEmpty () || strFileName == "*")
      strFileName = App.db_get_string  
        ("prefs", "DefaultNameGenerationFile", "names.txt");

    bool bFoundFile = false;
    if (strcmp (sName, "*"))  // don't check if forced reload wanted
      {
      try
        {
        CStdioFile f (strFileName, CFile::modeRead|CFile::shareDenyNone|CFile::typeText);
        bFoundFile = true;
        }
	    catch (CException* e)
	      {
		    e->Delete();
	      }
      } // end of not wanting another file anyway

    // if the file isn't there, and they don't want us to ignore that,
    // ask the user where it is
    if (!bFoundFile && !bNoDialog)
      {
	    CFileDialog	dlg(TRUE,						// TRUE for FileOpen
					    "txt",						// default extension
					    strFileName,						// default file name
					    OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
					    "Name files (*.txt;*.nam)|*.txt;*.nam|All files (*.*)|*.*||");
	    dlg.m_ofn.lpstrTitle = "Select names file";
      ChangeToFileBrowsingDirectory ();
	    int nResult = dlg.DoModal();
      ChangeToStartupDirectory ();

	    if (nResult != IDOK)
		    return;
      strFileName = dlg.GetPathName();

      // remember its location to stop annoying me every time
      App.db_write_string  ("prefs", "DefaultNameGenerationFile", 
                                  strFileName);   

      } // end of no file name supplied

    CStdioFile f (strFileName, CFile::modeRead|CFile::shareDenyNone|CFile::typeText);
  
    // look for [start]
    while (bOK = f.ReadString (sBuf))
      {
      sBuf.TrimRight ();
      sBuf.TrimLeft ();
      if (sBuf.Left (2) == "/*")
         continue;    // ignore comments
      if (sBuf.CompareNoCase ("[start]") == 0)
        break;
      if (sBuf.CompareNoCase ("[startstav]") == 0)
        break;
      }
    if (!bOK)
      ThrowErrorException ("No [start] in names file");

    // look for [middle]
    while (bOK = f.ReadString (sBuf))
      {
      sBuf.TrimRight ();
      sBuf.TrimLeft ();              
      if (sBuf.Left (2) == "/*")
         continue;    // ignore comments
      if (sBuf.CompareNoCase ("[middle]") == 0)
        break;
      else if (sBuf.CompareNoCase ("[mittstav]") == 0)
        break;
      else
        FirstNames.Add (sBuf);
      }
    if (!bOK)
      ThrowErrorException ("No [middle] in names file");

    // look for [end]
    while (bOK = f.ReadString (sBuf))
      {
      sBuf.TrimRight ();
      sBuf.TrimLeft ();
      if (sBuf.Left (2) == "/*")
         continue;    // ignore comments
      if (sBuf.CompareNoCase ("[end]") == 0)
        break;
      else if (sBuf.CompareNoCase ("[slutstav]") == 0)
        break;
      else
        MiddleNames.Add (sBuf);
      }
    if (!bOK)
      ThrowErrorException ("No [end] in names file");

    // look for [stop]
    while (bOK = f.ReadString (sBuf))
      {
      sBuf.TrimRight ();
      sBuf.TrimLeft ();
      if (sBuf.Left (2) == "/*")
         continue;    // ignore comments
      if (sBuf.CompareNoCase ("[stop]") == 0)
        break;
      else
        LastNames.Add (sBuf);
      }
    if (!bOK)
      ThrowErrorException ("No [stop] in names file");
    bNamesRead = true;


  } // end of ReadNames

CString GenerateName (void)
  {
CString strName;

	try
	  {
    if (!bNamesRead)
       ReadNames ("");
    }
	catch (CException* e)
	  {
		e->ReportError();
		e->Delete();
	  }

  if (bNamesRead)
    {
    strName = FirstNames [rand () % FirstNames.GetSize ()];
    strName += MiddleNames [rand () % MiddleNames.GetSize ()];
    strName += LastNames [rand () % LastNames.GetSize ()];
    }

  return strName;

  } // end of GenerateName
