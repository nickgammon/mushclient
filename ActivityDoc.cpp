// ActivityDoc.cpp : implementation file
//

#include "stdafx.h"
#include "mushclient.h"
#include "ActivityDoc.h"

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
// CActivityDoc

IMPLEMENT_DYNCREATE(CActivityDoc, CDocument)

CActivityDoc::CActivityDoc()
{
App.m_pActivityDoc = this;
}

BOOL CActivityDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CActivityDoc::~CActivityDoc()
{
App.m_pActivityDoc = NULL;
}


BEGIN_MESSAGE_MAP(CActivityDoc, CDocument)
	//{{AFX_MSG_MAP(CActivityDoc)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CActivityDoc diagnostics

#ifdef _DEBUG
void CActivityDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CActivityDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CActivityDoc serialization

void CActivityDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CActivityDoc commands

void CActivityDoc::OnFileOpen() 
{

#ifdef _DEBUG
//  ListAccelerators (this, 4);     // for documenting menus, accelerators
#endif 

	CString title;
	VERIFY(title.LoadString(AFX_IDS_OPENFILE));

  CString fileName;

  CFileDialog dlgFile (TRUE,   // loading the file
                 "mcl",        // default extension
                 "",           // suggested name
                 OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
                 "World or text files (*.mcl;*.txt)|*.mcl;*.txt|All files (*.*)|*.*||",    // filter 
                 NULL);        // parent window

	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);

  // use default world file directory
  dlgFile.m_ofn.lpstrInitialDir = Make_Absolute_Path (App.m_strDefaultWorldFileDirectory);

  ChangeToFileBrowsingDirectory ();
	int nResult = dlgFile.DoModal();
  ChangeToStartupDirectory ();

	fileName.ReleaseBuffer();

  if (nResult != IDOK)
    return;

  App.OpenDocumentFile (dlgFile.m_ofn.lpstrFile);	
	
}
