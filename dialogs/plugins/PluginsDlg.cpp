// PluginsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\mainfrm.h"
#include "..\..\mushclient.h"
#include "..\..\doc.h"
#include "..\..\TextDocument.h"
#include "PluginsDlg.h"
#include "..\..\winplace.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPluginsDlg dialog


CPluginsDlg::CPluginsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPluginsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPluginsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT  

  m_pDoc = NULL;

// default to sorting in name order

  m_last_col = eColumnName;
  m_reverse = FALSE;

}


void CPluginsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPluginsDlg)
	DDX_Control(pDX, IDC_DISABLE, m_ctlDisable);
	DDX_Control(pDX, IDC_ENABLE, m_ctlEnable);
	DDX_Control(pDX, IDCANCEL, m_ctlCancel);
	DDX_Control(pDX, IDC_SHOW_DESCRIPTION, m_ctlShowDescription);
	DDX_Control(pDX, IDC_RELOAD, m_ctlReload);
	DDX_Control(pDX, IDC_EDIT, m_ctlEdit);
	DDX_Control(pDX, IDC_DELETE_PLUGIN, m_ctlDelete);
	DDX_Control(pDX, IDC_ADD_PLUGIN, m_ctlAdd);
	DDX_Control(pDX, IDC_PLUGINS_LIST, m_ctlPluginList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPluginsDlg, CDialog)
	//{{AFX_MSG_MAP(CPluginsDlg)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_PLUGINS_LIST, OnColumnclickPluginsList)
	ON_BN_CLICKED(IDC_SHOW_DESCRIPTION, OnShowDescription)
	ON_BN_CLICKED(IDC_ADD_PLUGIN, OnAddPlugin)
	ON_BN_CLICKED(IDC_DELETE_PLUGIN, OnDeletePlugin)
	ON_BN_CLICKED(IDC_RELOAD, OnReload)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_NOTIFY(NM_DBLCLK, IDC_PLUGINS_LIST, OnDblclkPluginsList)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RDBLCLK, IDC_PLUGINS_LIST, OnRdblclkPluginsList)
	ON_BN_CLICKED(IDC_ENABLE, OnEnable)
	ON_BN_CLICKED(IDC_DISABLE, OnDisable)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_RELOAD, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_EDIT, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_OPEN_EXISTING, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_DELETE_PLUGIN, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_MOVE_UP, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_MOVE_DOWN, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_ENABLE, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_DISABLE, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDC_SHOW_DESCRIPTION, OnUpdateNeedDescription)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPluginsDlg message handlers


BOOL CPluginsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
int iColOrder [eColumnCount] = {0, 1, 2, 3, 4, 5, 6},
    iColWidth [eColumnCount] = {50, 130, 55, 55, 80, 30, 20};

// set list control to display whole row when selected and to allow column drag/drop

  App.RestoreColumnConfiguration ("Plugins List", eColumnCount, m_ctlPluginList,
                                  iColOrder, iColWidth, m_last_col, m_reverse);
 
  // for upgrading
  if (iColWidth [eColumnEnabled] < 20)
     iColWidth [eColumnEnabled] = 20;

  // for upgrading
  if (iColWidth [eColumnVersion] < 20)
     iColWidth [eColumnVersion] = 20;

 m_ctlPluginList.InsertColumn(eColumnName, TranslateHeading ("Name"), LVCFMT_LEFT, iColWidth [eColumnName]);
 m_ctlPluginList.InsertColumn(eColumnPurpose, TranslateHeading ("Purpose"), LVCFMT_LEFT, iColWidth [eColumnPurpose]);
 m_ctlPluginList.InsertColumn(eColumnAuthor, TranslateHeading ("Author"), LVCFMT_LEFT, iColWidth [eColumnAuthor]);
 m_ctlPluginList.InsertColumn(eColumnLanguage, TranslateHeading ("Language"), LVCFMT_LEFT, iColWidth [eColumnLanguage]);
 m_ctlPluginList.InsertColumn(eColumnFile, TranslateHeading ("File"), LVCFMT_LEFT, iColWidth [eColumnFile]);
 m_ctlPluginList.InsertColumn(eColumnEnabled, TranslateHeading ("Enabled"), LVCFMT_LEFT, iColWidth [eColumnEnabled]);
 m_ctlPluginList.InsertColumn(eColumnVersion, TranslateHeading ("Ver"), LVCFMT_RIGHT, iColWidth [eColumnVersion]);
                                                       
// recover column sequence

  m_ctlPluginList.SendMessage (LVM_SETCOLUMNORDERARRAY, eColumnCount, (DWORD) iColOrder);
	
  CWindowPlacement wp;
  wp.Restore ("Plugins List", this, false);

  LoadList ();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} // end of CPluginsDlg::OnInitDialog


static int CALLBACK CompareFunc ( LPARAM lParam1, 
                                 LPARAM lParam2,
                                 LPARAM lParamSort)
  { 
 CPlugin * item1 = (CPlugin *) lParam1;
 CPlugin * item2 = (CPlugin *) lParam2;

int iResult;

  switch (lParamSort & 0xFF)   // which sort key
    {
    case eColumnName:
      iResult = item1->m_strName.CompareNoCase (item2->m_strName); 
      break;
    case eColumnPurpose:
      iResult = item1->m_strPurpose.CompareNoCase (item2->m_strPurpose); 
      break;
    case eColumnAuthor:
      iResult = item1->m_strAuthor.CompareNoCase (item2->m_strAuthor);
      if (iResult == 0)
        iResult = item1->m_strName.CompareNoCase (item2->m_strName); 
      break;
    case eColumnLanguage:
      iResult = item1->m_strLanguage.CompareNoCase (item2->m_strLanguage); 
      if (iResult == 0)
        iResult = item1->m_strName.CompareNoCase (item2->m_strName); 
      break;
    case eColumnFile:
      iResult = item1->m_strSource.CompareNoCase (item2->m_strSource); 
      break;
    case eColumnEnabled:
      iResult = item1->m_bEnabled < item2->m_bEnabled; 
      if (iResult == 0)
        iResult = item1->m_strName.CompareNoCase (item2->m_strName); 
      break;
    case eColumnVersion:
      iResult = item1->m_dVersion < item2->m_dVersion; 
      if (iResult == 0)
        iResult = item1->m_strName.CompareNoCase (item2->m_strName); 
      break;

    default: return 0;
    } // end of switch

// if reverse sort wanted, reverse sense of result

  if (lParamSort & 0xFF00)
    iResult *= -1;

  return iResult;

  } // end of CompareFunc

void CPluginsDlg::LoadList (void)
  {

int nItem = 0;

   m_ctlPluginList.DeleteAllItems ();

  for (POSITION pos = m_pDoc->m_PluginList.GetHeadPosition(); pos; nItem++)
    {
    CPlugin * p = m_pDoc->m_PluginList.GetNext (pos);

 	  m_ctlPluginList.InsertItem (nItem, p->m_strName);    // eColumnName

	  m_ctlPluginList.SetItemText (nItem, eColumnPurpose, p->m_strPurpose);
	  m_ctlPluginList.SetItemText (nItem, eColumnAuthor, p->m_strAuthor);
	  m_ctlPluginList.SetItemText (nItem, eColumnLanguage, p->m_strLanguage);
	  m_ctlPluginList.SetItemText (nItem, eColumnFile, p->m_strSource);
    m_ctlPluginList.SetItemText (nItem, eColumnEnabled, p->m_bEnabled ? "Yes" : "No");
    m_ctlPluginList.SetItemText (nItem, eColumnVersion, CFormat ("%5.2f", p->m_dVersion));
    m_ctlPluginList.SetItemData (nItem, (DWORD) p);

    }

  m_ctlPluginList.SortItems (CompareFunc, m_reverse << 8 | m_last_col); 

// set the 1st item to be selected - we do this here because sorting the
// list means our first item is not necessarily the 1st item in the list

 m_ctlPluginList.SetItemState (0, LVIS_FOCUSED | LVIS_SELECTED, 
                                  LVIS_FOCUSED | LVIS_SELECTED);

  } // end of CPluginsDlg::LoadList

void CPluginsDlg::OnColumnclickPluginsList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  int col = pNMListView->iSubItem;

  if (col == m_last_col)
    m_reverse = !m_reverse;
  else
    m_reverse = FALSE;

  m_last_col = col;
    
  m_ctlPluginList.SortItems (CompareFunc, m_reverse << 8 | m_last_col); 
	
	*pResult = 0;
} // end of CPluginsDlg::OnColumnclickPluginsList

void CPluginsDlg::OnShowDescription() 
{       

// iterate through list 
for (int nItem = -1;
      (nItem = m_ctlPluginList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
  {

  CPlugin * p = (CPlugin *) m_ctlPluginList.GetItemData (nItem);

  if (p->m_strDescription.IsEmpty ())
    continue;   // ignore ones without a description

  CString strTitle = TFormat ("%s description", (LPCTSTR) p->m_strName);

  m_pDoc->AppendToTheNotepad (strTitle,
                      p->m_strDescription,                
                      true,   
                      eNotepadPluginInfo);

  // make sure they see it
  m_pDoc->ActivateNotepad (strTitle);

  } // end of doing all selected ones
}  // end of CPluginsDlg::OnShowDescription

void CPluginsDlg::OnUpdateNeedSelection(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable (m_ctlPluginList.GetSelectedCount () > 0);
}  // end of CPluginsDlg::OnUpdateNeedSelection

void CPluginsDlg::OnUpdateNeedDescription(CCmdUI* pCmdUI) 
{

  if (m_ctlPluginList.GetSelectedCount () <= 0)
    {
    pCmdUI->Enable (FALSE);
    return;
    }

// iterate through list 
for (int nItem = -1;
      (nItem = m_ctlPluginList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
  {

  CPlugin * p = (CPlugin *) m_ctlPluginList.GetItemData (nItem);


  // need a non-blank description for at least one of them
   
  if (!p->m_strDescription.IsEmpty ())
    {
    pCmdUI->Enable (TRUE);
    return;

    } // one with a description

  } // end of loop

pCmdUI->Enable (FALSE);

}  // end of CPluginsDlg::OnUpdateNeedDescription

LRESULT CPluginsDlg::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CPluginsDlg::OnKickIdle


void CPluginsDlg::OnAddPlugin() 
{
  CFileDialog filedlg (TRUE,   // loading the file
                       "",    // default extension
                       "",  // suggested name
                       OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER,
                       "XML files (*.xml)|*.xml|"
                       "Text files (*.txt)|*.txt|"
                       "All files (*.*)|*.*|"
                       "|",    // filter 
                       this);  // parent window

	char	szFileBuffer[4096];
	szFileBuffer[0] = 0;
	filedlg.m_ofn.lpstrFile = szFileBuffer;
	filedlg.m_ofn.nMaxFile = sizeof(szFileBuffer);

  filedlg.m_ofn.lpstrTitle = "Add plugin";

  // use default world file directory
  filedlg.m_ofn.lpstrInitialDir = Make_Absolute_Path (App.m_strPluginsDirectory);

  ChangeToFileBrowsingDirectory ();
	int nResult = filedlg.DoModal();
  ChangeToStartupDirectory ();


  if (nResult != IDOK)
    return;    // cancelled dialog
  
  bool bChanged = false;

  CString strPath;


  for (POSITION pos = filedlg.GetStartPosition (); pos; )
    {
    strPath = filedlg.GetNextPathName (pos);

    try

      {

      // now load it
      m_pDoc->InternalLoadPlugin (strPath);
      bChanged = true;

      } // end of try block

    catch (CFileException * e)
      {
      ::UMessageBox (TFormat ("Unable to open or read %s",
                        (LPCTSTR) strPath), MB_ICONEXCLAMATION);
      e->Delete ();
      } // end of catching a file exception

    catch (CException* e) 
      {
      ::UMessageBox (TFormat ("There was a problem loading the plugin %s. "
                       "See the output window for more details",
                       (LPCTSTR) strPath), MB_ICONEXCLAMATION);
      e->Delete ();
      EditPlugin (strPath);  // let them see the problem
      }

    } // end of doing each file

  LoadList ();
  
  if (bChanged)
    {
    m_pDoc->PluginListChanged ();
    m_pDoc->SetModifiedFlag (TRUE);
    }

}    // end of CPluginsDlg::OnAddPlugin() 


void CPluginsDlg::OnDeletePlugin() 
{
CUIntArray arySelected;
int iCount =  m_ctlPluginList.GetSelectedCount ();
int nItem,
    i;

  arySelected.SetSize (iCount);

    // first, remember selected items
  for (nItem = -1, i = 0;
        (nItem = m_ctlPluginList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
         arySelected [i++] = nItem;

  bool bChanged = false;

  // we do it this way because deleting items buggers up the position in the array
  for (i = iCount - 1; i >= 0; i--)
    {
    nItem = arySelected [i];

    CPlugin * p = (CPlugin *) m_ctlPluginList.GetItemData (nItem);
	  
    POSITION pos = m_pDoc->m_PluginList.Find (p);

    if (pos)
      {
      m_pDoc->m_PluginList.RemoveAt (pos);  // remove from list
      delete p;   // delete the plugin
      bChanged = true;
      }
    else
      ::TMessageBox ("Plugin cannot be found, unexpectedly.", MB_ICONEXCLAMATION); 
    } // end of loop

  LoadList ();

  if (bChanged)
    {
    m_pDoc->PluginListChanged ();
    m_pDoc->SetModifiedFlag (TRUE);
    }

}        // CPluginsDlg::OnDeletePlugin()

/*
void CPluginsDlg::OnMoveUp() 
{
int nItem = m_ctlPluginList.GetNextItem(-1, LVNI_SELECTED);
         
if (nItem == -1)
 return;

if (nItem == 0)
 return;      // already at top

CPlugin * p = (CPlugin *) m_ctlPluginList.GetItemData (nItem);
	
  POSITION pos = m_pDoc->m_PluginList.Find (p);
	
  if (!pos)
    return;

CPlugin * p2 = (CPlugin *) m_ctlPluginList.GetItemData (nItem - 1);
	
  POSITION pos2 = m_pDoc->m_PluginList.Find (p2);

  if (!pos2)
    return;

  m_pDoc->m_PluginList.RemoveAt (pos);  // remove from list (old position)
  m_pDoc->m_PluginList.InsertBefore (pos2, p);  // add before previous one

  LoadList ();

}

void CPluginsDlg::OnMoveDown() 
{
	// TODO: Add your control notification handler code here
	
}

  */

void CPluginsDlg::OnReload() 
{
bool bChanged = false;

// iterate through list 
for (int nItem = -1;
      (nItem = m_ctlPluginList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
  {


  CPlugin * p = (CPlugin *) m_ctlPluginList.GetItemData (nItem);
	  
    POSITION pos = m_pDoc->m_PluginList.Find (p);

    if (pos)
      {
      CString strName = p->m_strSource;
      m_pDoc->m_PluginList.RemoveAt (pos);  // remove from list
      delete p;   // delete the plugin

      try
        {
        bChanged = true;

        // now reload it
        m_pDoc->InternalLoadPlugin (strName);


        } // end of try block

      catch (CFileException * e)
        {
        ::UMessageBox (TFormat ("Unable to open or read %s",
                          (LPCTSTR) strName), MB_ICONEXCLAMATION);
        e->Delete ();
        } // end of catching a file exception

      catch (CArchiveException* e) 
        {
        ::UMessageBox (TFormat ("There was a problem loading the plugin %s. "
                         "See the output window for more details",
                         (LPCTSTR) strName), MB_ICONEXCLAMATION);
        e->Delete ();
        }

      }
    else
      ::TMessageBox ("Plugin cannot be found, unexpectedly.", MB_ICONEXCLAMATION); 

        } // end of loop
  
LoadList ();

if (bChanged)
  m_pDoc->PluginListChanged ();

  }      // end of CPluginsDlg::OnReload

void CPluginsDlg::OnDestroy() 
{
  App.SaveColumnConfiguration ("Plugins List",  eColumnCount, m_ctlPluginList,
                                m_last_col, m_reverse);

  CDialog::OnDestroy();
		
  CWindowPlacement wp;
  wp.Save ("Plugins List", this);

}  // end of CPluginsDlg::OnDestroy

void CPluginsDlg::EditPlugin (const CString strName)
  {

  if (m_pDoc->m_bEditScriptWithNotepad)
    {
    CTextDocument * pNewDoc =
      (CTextDocument *) App.OpenDocumentFile (strName);
    if (pNewDoc)
      {
      pNewDoc->SetTheFont ();
      pNewDoc->m_pRelatedWorld = m_pDoc;
      pNewDoc->m_iUniqueDocumentNumber = m_pDoc->m_iUniqueDocumentNumber;
      }
    else
      ::UMessageBox(TFormat ("Unable to edit the plugin file %s.",
                  (LPCTSTR) strName), 
                      MB_ICONEXCLAMATION);
    return;
    }   // end of using inbuilt notepad

  m_pDoc->EditFileWithEditor (strName);

  } // end of CPluginsDlg::EditPlugin 

void CPluginsDlg::OnEdit() 
{

// iterate through list 
for (int nItem = -1;
      (nItem = m_ctlPluginList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
  {

  CPlugin * p = (CPlugin *) m_ctlPluginList.GetItemData (nItem);
	  
  POSITION pos = m_pDoc->m_PluginList.Find (p);

    if (!pos)
      continue;

  EditPlugin (p->m_strSource);

  } // end of loop
}  // end of CPluginsDlg::OnEdit

void CPluginsDlg::OnDblclkPluginsList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnEdit ();
	
	*pResult = 0;
}  // end of CPluginsDlg::OnDblclkPluginsList


// helpful macro for adjusting button positions
#define ADJUST_BUTTON(ctl, item) \
  ctl.MoveWindow          (iBorder + (iWidth * (item - 1)) + (iGap * (item - 1)), \
                           iTopOfRow, iWidth, iHeight)


void CPluginsDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
  if (m_ctlPluginList.m_hWnd && m_ctlCancel.m_hWnd && m_ctlAdd.m_hWnd &&
      m_ctlEnable.m_hWnd && m_ctlDisable.m_hWnd)
    {
    // move OK and Cancel buttons
    int iHeight;
    int iWidth;
    int iTopOfRow;
    int iBorder = 10;

    const int iButtonCount = 4; // how many buttons

    // Find button size - we assume all are the same
    GetButtonSize (m_ctlDelete, iHeight, iWidth);

    // ------------------------

    // calculate gaps for middle buttons - I will assume all buttons are the same size here

    // gap (between Add and Edit buttons) will be the width of the dialog
    // less the gaps on the side of those buttons, less the width of the iButtonCount buttons themselves

    int iGap = cx - (iBorder * 2) - (iWidth * iButtonCount);

    // we need (iButtonCount - 1) gaps:  Add    --1-- Reinstall --2-- Enable  --3-- Edit
    //                                   Remove --1-- Show Info --2-- Disable --3-- Close
    iGap /= iButtonCount - 1;

    // -----------------------

    // ------------------------------------------------------------------------------------
    // bottom row

    // calculate top of row 
    iTopOfRow = cy - iHeight - 10;

    // Remove button (1)
    ADJUST_BUTTON (m_ctlDelete, 1);

    // Show Info button (2)
    ADJUST_BUTTON (m_ctlShowDescription, 2);

    // Disable button (3)
    ADJUST_BUTTON (m_ctlDisable, 3);

    // Close button (4)
    ADJUST_BUTTON (m_ctlCancel, 4);


    // ------------------------------------------------------------------------------------
    // top row

    // calculate top of row 
    iTopOfRow -= (iHeight + 10);

    // Add button (1)
    ADJUST_BUTTON (m_ctlAdd, 1);

    // Reinstall button (2)
    ADJUST_BUTTON (m_ctlReload, 2);

    // Enable button (3)
    ADJUST_BUTTON (m_ctlEnable, 3);

    // Edit button (4)
    ADJUST_BUTTON (m_ctlEdit, 4);

    // move text to just above it
	  m_ctlPluginList.MoveWindow(0, 0, cx, iTopOfRow - 10);
    }
	
}

// RIGHT double-click plugin in list to see state file
void CPluginsDlg::OnRdblclkPluginsList(NMHDR* pNMHDR, LRESULT* pResult) 
{

  if (m_pDoc->m_strWorldID.IsEmpty ())
    return;

  // iterate through list 
  for (int nItem = -1;
        (nItem = m_ctlPluginList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
    {

    CPlugin * p = (CPlugin *) m_ctlPluginList.GetItemData (nItem);
	    
    POSITION pos = m_pDoc->m_PluginList.Find (p);

    if (!pos)
      continue;

    // need a directory
    CString strName = CString (Make_Absolute_Path (App.m_strDefaultStateFilesDirectory));

    strName += m_pDoc->m_strWorldID;    // world ID
    strName += "-";
    strName += p->m_strID;                 // plugin ID
    strName += "-state.xml";            // suffix


    if (m_pDoc->m_bEditScriptWithNotepad)
      {
      CTextDocument * pNewDoc =
        (CTextDocument *) App.OpenDocumentFile (strName);
      if (pNewDoc)
        {
        pNewDoc->SetTheFont ();
        pNewDoc->m_pRelatedWorld = m_pDoc;
        pNewDoc->m_iUniqueDocumentNumber = m_pDoc->m_iUniqueDocumentNumber;
        }
      else
        ::UMessageBox(TFormat ("Unable to edit the plugin state file %s.",
                    (LPCTSTR) strName), 
                        MB_ICONEXCLAMATION);
      continue;
      }   // end of using inbuilt notepad

    m_pDoc->EditFileWithEditor (strName);

  } // end of loop
	
	*pResult = 0;
}  // end of CPluginsDlg::OnRdblclkPluginsList


void CPluginsDlg::OnEnable() 
{
bool bChanged = false;

// iterate through list 
for (int nItem = -1;
      (nItem = m_ctlPluginList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
  {

  CPlugin * p = (CPlugin *) m_ctlPluginList.GetItemData (nItem);
	  
  POSITION pos = m_pDoc->m_PluginList.Find (p);

    if (!pos)
      continue;

  m_pDoc->EnablePlugin (p->m_strID, TRUE);
  bChanged = true;

  } // end of loop
	
  LoadList ();

  if (bChanged)
    m_pDoc->PluginListChanged ();
}  // end of CPluginsDlg::OnEnable

void CPluginsDlg::OnDisable() 
{
bool bChanged = false;

// iterate through list 
for (int nItem = -1;
      (nItem = m_ctlPluginList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
  {

  CPlugin * p = (CPlugin *) m_ctlPluginList.GetItemData (nItem);
	  
  POSITION pos = m_pDoc->m_PluginList.Find (p);

    if (!pos)
      continue;

  m_pDoc->EnablePlugin (p->m_strID, FALSE);
  bChanged = true;


  } // end of loop

  LoadList ();

  if (bChanged)
    m_pDoc->PluginListChanged ();
  
}  // end of CPluginsDlg::OnDisable
