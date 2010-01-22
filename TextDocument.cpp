// TextDocument.cpp : implementation file
//

/*
Copyright (c) 2000 Nick Gammon.

*/

#include "stdafx.h"
#include "MUSHclient.h"
#include "TextDocument.h"
#include "TextView.h"
#include "MainFrm.h"
#include <process.h>    
#include "doc.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
// for documenting menus, accelerators
void ListAccelerators (CDocument * pDoc, const int iType);
#endif 

/////////////////////////////////////////////////////////////////////////////
// CTextDocument

IMPLEMENT_DYNCREATE(CTextDocument, CDocument)

CTextDocument::CTextDocument()
  :	m_eventFileChanged(FALSE, TRUE)
{
  m_bInFileChanged = false;
  m_pThread = NULL;
  m_pRelatedWorld = NULL;

  m_iUniqueDocumentNumber = 0;
  m_strFontName = App.m_strDefaultInputFont;
  m_iFontSize = App.m_iDefaultInputFontHeight;
  m_iFontWeight = App.m_iDefaultInputFontWeight;
  m_iFontCharset = App.m_iDefaultInputFontCharset;
  m_bFontItalic = false;
  m_bFontUnderline = false;
  m_bFontStrikeout = false;
  m_textColour = RGB (0, 0, 0);        // window colour
  m_backColour = RGB (255, 255, 255);
  m_bMatchCase = false;
  m_bRegexp = false;         
  m_bCommands = false;
  m_bOutput = false;
  m_bNotes = false;
  m_iLines = false;
  m_bReadOnly = FALSE;

  // save method
  m_iSaveOnChange = eNotepadSaveDefault;

  // scripting support

	EnableAutomation();     // not needed?

  if (!bWine)
  	AfxOleLockApp();        // not needed?

}

BOOL CTextDocument::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
  KillThread (m_pThread, m_eventFileChanged);
	return TRUE;
}

CTextDocument::~CTextDocument()
{
  KillThread (m_pThread, m_eventFileChanged);

  if (!bWine)
  	AfxOleUnlockApp();        // not needed?

}


BEGIN_MESSAGE_MAP(CTextDocument, CDocument)
	//{{AFX_MSG_MAP(CTextDocument)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_UPDATE_COMMAND_UI(ID_STATUSLINE_MUSHNAME, OnUpdateStatuslineMushname)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_EDIT_CONVERTCLIPBOARDFORUMCODES, OnEditConvertclipboardforumcodes)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextDocument diagnostics

#ifdef _DEBUG
void CTextDocument::AssertValid() const
{
	CDocument::AssertValid();
}

void CTextDocument::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTextDocument serialization

void CTextDocument::Serialize(CArchive& ar)
{
	// CEditView contains an edit control which handles all serialization
	((CEditView*)m_viewList.GetHead())->SerializeRaw(ar);

  if (ar.IsLoading ())
    CreateMonitoringThread (ar.GetFile ()->GetFilePath ());
}

/////////////////////////////////////////////////////////////////////////////
// CTextDocument commands

/*
void CTextDocument::OnUpdateStatusModified(CCmdUI* pCmdUI) 
{
  if (IsModified ())
  	pCmdUI->SetText ("Modified");
  else
  	pCmdUI->SetText ("");
	
}

  */

void CTextDocument::OnCloseDocument() 
{
  KillThread (m_pThread, m_eventFileChanged);
	CDocument::OnCloseDocument();
}


// ------------------- file change monitoring thread -------------------------

void ThreadFunc(LPVOID pParam)
{
  CThreadData*	pData = (CThreadData*) pParam;
	char * strDir = pData->m_strFilename;
  DWORD pDoc = pData->m_pDoc;
	char * p = strrchr (strDir, '\\');
	if (!p)
		p = strrchr (strDir, ':');   // why?
  if (p)
    *p = 0;
	HWND	hWnd = pData->m_hWnd;
	HANDLE	hEvent = pData->m_hEvent;

	delete pData;

  // Get a handle to a file change notification object.
  HANDLE	hChange = ::FindFirstChangeNotification(strDir, TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE);

  delete [] strDir;

  // Return now if ::FindFirstChangeNotification failed.
  if (hChange == INVALID_HANDLE_VALUE)
    return;

	HANDLE	aHandles[2];
	aHandles[0] = hChange;
	aHandles[1] = hEvent;
	BOOL	bContinue = TRUE;

    // Sleep until a file change notification wakes this thread or
    // m_eventScriptFileChanged becomes set indicating it's time for the thread to end.
    while (bContinue)
	{
		switch ((::WaitForMultipleObjects(2, aHandles, FALSE, INFINITE)))
		{
		case 0:
			// Respond to a change notification.
			::PostMessage(hWnd, WM_USER_FILE_CONTENTS_CHANGED, (WPARAM) pDoc, 0);
			::FindNextChangeNotification(hChange);
			break;

		default:
			// Kill this thread (m_event became signaled).
            bContinue = FALSE;
			break;
		}
	}

	// Close the file change notification handle and return.
	::FindCloseChangeNotification(hChange);
	return;
}

void KillThread (HANDLE & pThread, CEvent & eventFileChanged)
  {
	// Kill the file change notification thread
	if (pThread)
	  {
    // setting this event *should* cause the thread to wrapup gracefully
		eventFileChanged.SetEvent();

    // wait for thread to go away, or 10 seconds, whichever is sooner
		DWORD waitstatus = ::WaitForSingleObject(pThread, 10000L);

    // if unable to terminate thread properly, get rid of the blasted thing
    if (waitstatus == WAIT_TIMEOUT || waitstatus == WAIT_FAILED)   
      TerminateThread (pThread, 1);

		pThread = NULL;
	  }
  } // end of KillThread

// Create script source file monitoring thread
//
HANDLE CreateMonitoringThread(const char * sName, DWORD pDoc, CEvent & eventFileChanged)
{
  HANDLE pThread;

	CThreadData*	pData = new CThreadData;
	pData->m_strFilename = new char [strlen (sName) + 1];
  strcpy (pData->m_strFilename, sName);
	pData->m_hWnd = Frame.GetSafeHwnd ();
	pData->m_hEvent = eventFileChanged;
  pData->m_pDoc = pDoc;
	eventFileChanged.ResetEvent();

	pThread = (HANDLE) _beginthread (ThreadFunc, 0, pData);
  SetThreadPriority (pThread, THREAD_PRIORITY_IDLE);

  return pThread;

	// Thread will delete data object
}


// ------------------- handle change to file -------------------------

void CTextDocument::OnFileChanged(void)
{
  // exit if we are already asking them the question

	if (m_bInFileChanged)
		return;

  m_bInFileChanged = true;

	// Check if this file has changed
	CFileStatus	status;
	CFile::GetStatus(GetPathName (), status);
	if (m_timeFileMod != status.m_mtime)
	  {

    // remember time for next time

    m_timeFileMod = status.m_mtime;

		CString	strText;
    strText = TFormat ("The file \"%s\" has been modified. Do you wish to reload it?",
      (LPCTSTR) GetPathName ());
    if (::TMessageBox (strText, MB_YESNO | MB_ICONQUESTION) == IDYES)
      {
		  CWaitCursor	wait;

	    try
	      {

        // Reload existing file
		    CFile	f(GetPathName (), CFile::modeRead|CFile::shareDenyWrite);
        CArchive ar(&f, CArchive::load);
	      Serialize (ar);
        ar.Close();
        SetModifiedFlag (false);
	      }
	    catch(CException* e)
  	    {
        ::TMessageBox ("Unable to read file", MB_ICONEXCLAMATION);
		    e->Delete();
	      }
      } // end of approving modification or wanting it anyway
    } // end of time changing

	m_bInFileChanged = false;
}


// kill the monitoring thread during the save
BOOL CTextDocument::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
  {
  KillThread (m_pThread, m_eventFileChanged);

  BOOL bResult = CDocument::DoSave (lpszPathName, bReplace);

  // monitor this file again
  if (!GetPathName ().IsEmpty ())
    CreateMonitoringThread (GetPathName ());
  return bResult;
  }



void CTextDocument::CreateMonitoringThread(const char * sName)
{
  // kill any old thread
  KillThread (m_pThread, m_eventFileChanged);

  // find when the file was last modified

	CFileStatus	status;
	CFile::GetStatus(sName, status);
  m_timeFileMod = status.m_mtime;

  // create the thread
  m_pThread = ::CreateMonitoringThread (sName, (DWORD) this, m_eventFileChanged);

  UpdateAllViews  (NULL);     // force window title to be redrawn
}

void CTextDocument::OnFileOpen() 
{
	CString title;
	VERIFY(title.LoadString(AFX_IDS_OPENFILE));

  CString fileName;

  CFileDialog dlgFile (TRUE,   // loading the file
                 "txt",        // default extension
                 "",           // suggested name
                 OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
                 "Text files (*.txt)|*.txt|"
                 "Plugins (*.xml)|*.xml|"
                 "Lua scripts (*.lua)|*.lua|"
                 "All files (*.*)|*.*||",    // filter 
                 NULL);        // parent window

	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);
  ChangeToFileBrowsingDirectory ();
	int nResult = dlgFile.DoModal();
  ChangeToStartupDirectory ();
	fileName.ReleaseBuffer();

  if (nResult != IDOK)
    return;

  CTextDocument * pNewDoc = (CTextDocument *)
     App.m_pNormalDocTemplate->OpenDocumentFile (dlgFile.m_ofn.lpstrFile);	

  if (!pNewDoc)
    return;

  SetTheFont ();

}

void CTextDocument::SetTheFont (void)
  {
  
  // find first view
  POSITION pos = GetFirstViewPosition();

  if (!pos)
    return;

  // find view
  CTextView * pView = (CTextView *) GetNextView(pos);

  // ensure font displayed correctly
  pView->SetTheFont ();  

  }


// returns a pointer to the related world, or NULL if none

CMUSHclientDoc * CTextDocument::FindWorld (void)
  {

  if (!m_pRelatedWorld)
    return NULL;

  // make sure world still in world list
  for (POSITION docPos = App.m_pWorldDocTemplate->GetFirstDocPosition();
      docPos != NULL; )
    {

    CMUSHclientDoc * pDoc = (CMUSHclientDoc *) App.m_pWorldDocTemplate->GetNextDoc(docPos);

    // it might be the same pointer, but is it the same *world*?
    if (pDoc == m_pRelatedWorld)
      if (pDoc->m_iUniqueDocumentNumber == m_iUniqueDocumentNumber)
        return pDoc;
      else
        return NULL;    // no, unique ID number has changed
    } // end of doing each document

  return NULL;    // no longer in world list
  }

void CTextDocument::OnUpdateStatuslineMushname(CCmdUI* pCmdUI) 
{
 
  if (FindWorld ())
    {
    pCmdUI->Enable(TRUE);
    pCmdUI->SetText(m_pRelatedWorld->m_mush_name);	
	  }
  else
    {
    pCmdUI->Enable(TRUE);
    pCmdUI->SetText("(no related world)");	
    }
}


void CTextDocument::OnFileNew() 
{
CString strTitle = m_strTitle;

CMUSHclientDoc * pDoc = FindWorld ();

  // don't make another recall window
  if (!m_strSearchString.IsEmpty ())
    {
    if (pDoc)
      strTitle = TFormat ("Notepad: %s", (LPCTSTR) pDoc->m_mush_name);
    else
      strTitle = Translate ("Untitled");
    }

  CreateTextWindow ("",     // contents
                    strTitle,     // title
                    pDoc,   // document
                    m_iUniqueDocumentNumber,      // document number
                    m_strFontName,
                    m_iFontSize,
                    m_iFontWeight,
                    m_iFontCharset,
                    m_textColour,
                    m_backColour,
                    "",       // search string
                    "",       // line preamble
                    false,
                    false,
                    false,
                    false,  
                    false,
                    false,
                    eNotepadNormal
                    );

#ifdef _DEBUG
//  ListAccelerators (this, 2);     // for documenting menus, accelerators
#endif 

}

BOOL CTextDocument::SaveModified() 
{
// don't bother asking if they want to save an empty document
CTextView* pView = (CTextView*) m_viewList.GetHead();
  
	if (GetText (pView->GetEditCtrl ()).IsEmpty ())
    return TRUE;

  switch (m_iSaveOnChange)
    {
    case eNotepadSaveAlways: break;  // fall through to save if changed
    case eNotepadSaveNever: return TRUE;    // don't save
    case eNotepadSaveDefault:

      // default processing
      switch (m_iNotepadType)
        {
        case eNotepadMXPdebug:
          if (!App.m_bConfirmBeforeClosingMXPdebug)
            return TRUE;
          break;

        // these ones close without asking
        case eNotepadPacketDebug:
        case eNotepadLineInfo:
        case eNotepadWorldLoadError: 
        case eNotepadXMLcomments:
        case eNotepadPluginInfo:
        case eNotepadRecall:
          return TRUE;

        default:
          break;  // normal check
        } // end of switch on notepad type
        break;

    } // end of switch on m_iSaveOnChange


	return CDocument::SaveModified();
} // end of CTextDocument::SaveModified


void CTextDocument::OnEditConvertclipboardforumcodes() 
{
CString strContents; 

  if (!GetClipboardContents (strContents, false, false))
    return;

 putontoclipboard  (QuoteForumCodes (strContents));
	
} // end of CTextDocument::OnEditConvertclipboardforumcodes


void CTextDocument::SetReadOnly (BOOL bReadOnly)
  {
CTextView* pView = (CTextView*) m_viewList.GetHead();
  
  if (pView)
    pView->GetEditCtrl ().SetReadOnly (bReadOnly);

  m_bReadOnly = bReadOnly;
  } // end of CTextDocument::SetReadOnly
