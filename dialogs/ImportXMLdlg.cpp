// ImportXMLdlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "plugins\PluginsDlg.h"
#include "ImportXMLdlg.h"
#include "..\doc.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImportXMLdlg dialog


CImportXMLdlg::CImportXMLdlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImportXMLdlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImportXMLdlg)
	m_bGeneral = FALSE;
	m_bTriggers = FALSE;
	m_bAliases = FALSE;
	m_bTimers = FALSE;
	m_bMacros = FALSE;
	m_bVariables = FALSE;
	m_bColours = FALSE;
	m_bKeypad = FALSE;
	m_bPrinting = FALSE;
	//}}AFX_DATA_INIT

  m_pDoc = NULL;

}


void CImportXMLdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportXMLdlg)
	DDX_Control(pDX, IDC_PRINTING, m_ctlPrinting);
	DDX_Control(pDX, IDC_KEYPAD, m_ctlKeypad);
	DDX_Control(pDX, IDC_COLOURS, m_ctlColours);
	DDX_Control(pDX, IDC_VARIABLES, m_ctlVariables);
	DDX_Control(pDX, IDC_MACROS, m_ctlMacros);
	DDX_Control(pDX, IDC_TIMERS, m_ctlTimers);
	DDX_Control(pDX, IDC_ALIASES, m_ctlAliases);
	DDX_Control(pDX, IDC_TRIGGERS, m_ctlTriggers);
	DDX_Control(pDX, IDC_GENERAL, m_ctlGeneral);
	DDX_Check(pDX, IDC_GENERAL, m_bGeneral);
	DDX_Check(pDX, IDC_TRIGGERS, m_bTriggers);
	DDX_Check(pDX, IDC_ALIASES, m_bAliases);
	DDX_Check(pDX, IDC_TIMERS, m_bTimers);
	DDX_Check(pDX, IDC_MACROS, m_bMacros);
	DDX_Check(pDX, IDC_VARIABLES, m_bVariables);
	DDX_Check(pDX, IDC_COLOURS, m_bColours);
	DDX_Check(pDX, IDC_KEYPAD, m_bKeypad);
	DDX_Check(pDX, IDC_PRINTING, m_bPrinting);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImportXMLdlg, CDialog)
	//{{AFX_MSG_MAP(CImportXMLdlg)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDC_CLIPBOARD, OnClipboard)
	ON_BN_CLICKED(IDC_PLUGINS_LIST, OnPluginsList)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_CLIPBOARD, OnUpdateNeedXMLClipboard)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportXMLdlg message handlers

void CImportXMLdlg::OnOk() 
{
	
  CFileDialog filedlg (TRUE,   // loading the file
                       "",    // default extension
                       "",  // suggested name
                       OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
                       "All files (*.*)|*.*|"
                       "XML files (*.xml)|*.xml|"
                       "Text files (*.txt)|*.txt|"
                       "MUSHclient worlds (*.mcl)|*.mcl|"
                       "MUSHclient triggers (*.mct)|*.mct|"
                       "MUSHclient aliases (*.mca)|*.mca|"
                       "MUSHclient colours (*.mcc)|*.mcc|"
                       "MUSHclient macros (*.mcm)|*.mcm|"
                       "MUSHclient timers (*.mci)|*.mci|"
                       "|",    // filter 
                       this);  // parent window

  filedlg.m_ofn.lpstrTitle = "Import";

  ChangeToFileBrowsingDirectory ();
	int nResult = filedlg.DoModal();
  ChangeToStartupDirectory ();

  if (nResult != IDOK)
    return;    // cancelled dialog


CFile * f = NULL;
CArchive * ar = NULL;

  try
    {
    f = new CFile (filedlg.GetPathName (), CFile::modeRead | CFile::shareDenyWrite);

    ar = new CArchive(f, CArchive::load);

    ImportArchive (*ar);

    } // end of try block

  catch (CFileException * e)
    {
    ::UMessageBox (TFormat ("Unable to open or read %s",
                      (LPCTSTR) filedlg.GetPathName ()), MB_ICONEXCLAMATION);
    e->Delete ();
    } // end of catching a file exception


  delete ar;      // delete archive
  delete f;       // delete file

}

void CImportXMLdlg::OnClipboard() 
{
CString strContents; 

  if (!GetClipboardContents (strContents, CF_TEXT, false))
    return; // can't do it

  CMemFile f ((unsigned char *) (const char *) strContents, strContents.GetLength ());
  CArchive ar (&f, CArchive::load);
	
  ImportArchive (ar);

}

#define ADDCOUNT(count, name, plural) \
  if (count)   \
    {             \
    if (!strCounts.IsEmpty ())   \
      strCounts += ", ";     \
    strCounts += CFormat ("%lu " name "%s", plural (count));  \
    }


void CImportXMLdlg::ImportArchive (CArchive & ar)
  {

  try
    {

    if (IsArchiveXML (ar))
      {
      unsigned long iMask = 0;

      // work out what to import
      if (m_ctlGeneral.GetCheck ())
         iMask |= XML_GENERAL;
      if (m_ctlTriggers.GetCheck ())
         iMask |= XML_TRIGGERS;
      if (m_ctlAliases.GetCheck ())
         iMask |= XML_ALIASES;
      if (m_ctlTimers.GetCheck ())
         iMask |= XML_TIMERS;
      if (m_ctlMacros.GetCheck ())
         iMask |= XML_MACROS;
      if (m_ctlVariables.GetCheck ())
         iMask |= XML_VARIABLES;
      if (m_ctlColours.GetCheck ())
         iMask |= XML_COLOURS;
      if (m_ctlKeypad.GetCheck ())
         iMask |= XML_KEYPAD;
      if (m_ctlPrinting.GetCheck ())
         iMask |= XML_PRINTING;

      UINT iTriggers = 0;
      UINT iAliases = 0;
      UINT iTimers = 0;
      UINT iMacros = 0;
      UINT iVariables = 0;
      UINT iColours = 0;
      UINT iKeypad = 0;
      UINT iPrinting = 0;

      // do it
      m_pDoc->Load_World_XML (ar, 
                              iMask | XML_NO_PLUGINS, 
                              0,          // load flags
                              &iTriggers,  
                              &iAliases,   
                              &iTimers,    
                              &iMacros,    
                              &iVariables, 
                              &iColours,   
                              &iKeypad,    
                              &iPrinting);  

      CString strCounts = TFormat (
            "%lu trigger%s, "
            "%lu alias%s, "
            "%lu timer%s, "
            "%lu macro%s, "
            "%lu variable%s, "
            "%lu colour%s, "
            "%lu keypad%s, "
            "%lu printing style%s loaded. ",
             PLURAL (iTriggers),
             PLURALES (iAliases),
             PLURAL (iTimers),
             PLURAL (iMacros),
             PLURAL (iVariables),
             PLURAL (iColours),
             PLURAL (iKeypad),
             PLURAL (iPrinting));

      if (strCounts.IsEmpty ())
        strCounts = Translate ("No items loaded.");

      ::UMessageBox (strCounts, MB_ICONINFORMATION);

      
      m_pDoc->SetModifiedFlag (TRUE);   // document has now changed


    	CDialog::OnOK();

      }
    else
      ::TMessageBox ("Not in XML format");

     } // end of try block
  catch (CArchiveException* ) 
    {
    ::TMessageBox ("There was a problem parsing the XML. "
                     "See the error window for more details");

    }
  } // end of CImportXMLdlg::ImportArchive 

void CImportXMLdlg::OnUpdateNeedXMLClipboard(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (IsClipboardXML ());
  } // end of CImportXMLdlg::OnUpdateNeedXMLClipboard


LRESULT CImportXMLdlg::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CImportXMLdlg::OnKickIdle

void CImportXMLdlg::OnPluginsList() 
{
CPluginsDlg dlg;
  dlg.m_pDoc = m_pDoc;
  dlg.DoModal (); 
}
