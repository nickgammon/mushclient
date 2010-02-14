// PluginWizard.cpp : implementation file
//

#include "stdafx.h"
#include "..\..\resource.h"
#include "..\..\MUSHclient.h"
#include "..\..\mainfrm.h"
#include "..\..\doc.h"
#include "PluginWizard.h"
#include "..\EditMultiLine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPluginWizardPage1, CPropertyPage)
IMPLEMENT_DYNCREATE(CPluginWizardPage2, CPropertyPage)
IMPLEMENT_DYNCREATE(CPluginWizardPage3, CPropertyPage)
IMPLEMENT_DYNCREATE(CPluginWizardPage4, CPropertyPage)
IMPLEMENT_DYNCREATE(CPluginWizardPage5, CPropertyPage)
IMPLEMENT_DYNCREATE(CPluginWizardPage6, CPropertyPage)
IMPLEMENT_DYNCREATE(CPluginWizardPage7, CPropertyPage)
IMPLEMENT_DYNCREATE(CPluginWizardPage8, CPropertyPage)


/////////////////////////////////////////////////////////////////////////////
// CPluginWizardPage1 property page

CPluginWizardPage1::CPluginWizardPage1() : CPropertyPage(CPluginWizardPage1::IDD)
{
	//{{AFX_DATA_INIT(CPluginWizardPage1)
	m_strAuthor = _T("");
	m_strDateWritten = _T("");
	m_strPurpose = _T("");
	m_strVersion = _T("");
	m_fRequires = 0.0;
	m_bRemoveItems = FALSE;
	//}}AFX_DATA_INIT
}

CPluginWizardPage1::~CPluginWizardPage1()
{
}

void CPluginWizardPage1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPluginWizardPage1)
	DDX_Text(pDX, IDC_AUTHOR, m_strAuthor);
	DDV_MaxChars(pDX, m_strAuthor, 32);
	DDX_Text(pDX, IDC_DATE_WRITTEN, m_strDateWritten);
	DDX_Text(pDX, IDC_PURPOSE, m_strPurpose);
	DDV_MaxChars(pDX, m_strPurpose, 100);
	DDX_Text(pDX, IDC_VERSION2, m_strVersion);
	DDX_MinMaxString(pDX, IDC_NAME, m_strName);
	DDV_MinMaxString(pDX, m_strName, 1, 32);
	DDX_MinMaxString(pDX, IDC_ID, m_strID);
	DDV_MinMaxString(pDX, m_strID, 24, 24);
	DDX_Text(pDX, IDC_REQUIRES, m_fRequires);
	DDV_MinMaxDouble(pDX, m_fRequires, 0., 100000.);
	DDX_Check(pDX, IDC_REMOVE_ITEMS, m_bRemoveItems);
	//}}AFX_DATA_MAP


  if (pDX->m_bSaveAndValidate)
    {
    CString strTemp = m_strName;  // no name doesn't become lower case
    if (m_doc->CheckObjectName (strTemp))
      {
      ::TMessageBox ("The plugin name must start with a letter and consist of letters"
                      ", numbers or the underscore character.", MB_ICONSTOP);
      DDX_Text(pDX, IDC_NAME, m_strName);
      pDX->Fail();
      }

    if (m_pPage2->m_strHelpAlias.IsEmpty ())
       m_pPage2->m_strHelpAlias = m_strName + ":help";

    } // end of saving and validating


}


BEGIN_MESSAGE_MAP(CPluginWizardPage1, CPropertyPage)
	//{{AFX_MSG_MAP(CPluginWizardPage1)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPluginWizardPage2 property page

CPluginWizardPage2::CPluginWizardPage2() : CPropertyPage(CPluginWizardPage2::IDD)
{
	//{{AFX_DATA_INIT(CPluginWizardPage2)
	m_strDescription = _T("");
	m_bGenerateHelp = FALSE;
	m_strHelpAlias = _T("");
	//}}AFX_DATA_INIT
}

CPluginWizardPage2::~CPluginWizardPage2()
{
}

void CPluginWizardPage2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPluginWizardPage2)
	DDX_Control(pDX, IDC_GENERATE_HELP, m_ctlGenerateHelp);
	DDX_Control(pDX, IDC_DESCRIPTION, m_ctlDescription);
	DDX_Text(pDX, IDC_DESCRIPTION, m_strDescription);
	DDX_Check(pDX, IDC_GENERATE_HELP, m_bGenerateHelp);
	DDX_Text(pDX, IDC_HELP_ALIAS, m_strHelpAlias);
	//}}AFX_DATA_MAP


  if (pDX->m_bSaveAndValidate)
    {
    // description cannot have ]]> in it
    if (m_strDescription.Find ("]]>") != -1)
      {
      ::TMessageBox ("Description may not contain the sequence \"]]>\"", MB_ICONSTOP);
      DDX_Text(pDX, IDC_DESCRIPTION, m_strDescription);
      pDX->Fail();
      }

    } // end of saving and validating


}


BEGIN_MESSAGE_MAP(CPluginWizardPage2, CPropertyPage)
	//{{AFX_MSG_MAP(CPluginWizardPage2)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_GENERATE_HELP, OnUpdateNeedDescription)
  ON_UPDATE_COMMAND_UI(IDC_HELP_ALIAS, OnUpdateNeedDescription)
END_MESSAGE_MAP()


void CPluginWizardPage2::OnEdit() 
{
CEditMultiLine dlg;

  dlg.m_strText = GetText (m_ctlDescription);

  dlg.m_strTitle = "Edit plugin description";

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlDescription.SetWindowText (dlg.m_strText);
	
}

LRESULT CPluginWizardPage2::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CPluginWizardPage2::OnKickIdle

void CPluginWizardPage2::OnUpdateNeedDescription(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (!GetText (m_ctlDescription).IsEmpty ());
  } // end of CPluginWizardPage2::OnUpdateNeedSound

/////////////////////////////////////////////////////////////////////////////
// CPluginWizardPage3 property page

CPluginWizardPage3::CPluginWizardPage3() : CPropertyPage(CPluginWizardPage3::IDD)
{
	//{{AFX_DATA_INIT(CPluginWizardPage3)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPluginWizardPage3::~CPluginWizardPage3()
{
}

void CPluginWizardPage3::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPluginWizardPage3)
	DDX_Control(pDX, IDC_TRIGGER_LIST, m_ctlList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPluginWizardPage3, CPropertyPage)
	//{{AFX_MSG_MAP(CPluginWizardPage3)
	ON_BN_CLICKED(IDC_SELECT_ALL, OnSelectAll)
	ON_BN_CLICKED(IDC_SELECT_NONE, OnSelectNone)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CPluginWizardPage3::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
  m_ctlList.SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, 0, 
                        m_ctlList.SendMessage (LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) 
                        | LVS_EX_FULLROWSELECT 
                        | (App.m_bShowGridLinesInListViews ? LVS_EX_GRIDLINES : 0) );

  m_ctlList.InsertColumn(0, TranslateHeading ("Name"), LVCFMT_LEFT, 100);
  m_ctlList.InsertColumn(1, TranslateHeading ("Match"), LVCFMT_LEFT, 120);
  m_ctlList.InsertColumn(2, TranslateHeading ("Send"), LVCFMT_LEFT, 120);
  m_ctlList.InsertColumn(3, TranslateHeading ("Group"), LVCFMT_LEFT, 50);
 
  int iTrigger,
      iItem;

  for (iTrigger = iItem = 0; iTrigger < m_doc->m_TriggerArray.GetSize (); iTrigger++)
    {
    CTrigger * t = m_doc->m_TriggerArray [iTrigger];

    if (t->bTemporary)
      continue;   // ignore temporary ones

    if (m_ctlList.InsertItem (iItem, t->strLabel) != -1)
      {
  	  m_ctlList.SetItemText(iItem, 1, t->trigger); 
  	  m_ctlList.SetItemText(iItem, 2, t->contents); 
  	  m_ctlList.SetItemText(iItem, 3, t->strGroup); 
      m_ctlList.SetItemData (iItem, (unsigned long) t);
      m_ctlList.SetItemState (iItem, LVIS_SELECTED, LVIS_SELECTED);
      }
    iItem++;   

    }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPluginWizardPage3::OnSelectAll() 
{
  for (int iItem = 0; iItem < m_ctlList.GetItemCount (); iItem++)
     m_ctlList.SetItemState (iItem, LVIS_SELECTED, LVIS_SELECTED);
	
}

void CPluginWizardPage3::OnSelectNone() 
{
  for (int iItem = 0; iItem < m_ctlList.GetItemCount (); iItem++)
     m_ctlList.SetItemState (iItem, 0, LVIS_SELECTED);
}

void CPluginWizardPage3::OnOK() 
{
  int i;

  // de-select everything
  for (i = 0; i < m_doc->m_TriggerArray.GetSize (); i++)
    m_doc->m_TriggerArray [i]->bSelected = false;
	
  for (int nItem = -1;
        (nItem = m_ctlList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
    {
    CTrigger * t = (CTrigger *) m_ctlList.GetItemData (nItem);
    // find in trigger array (in case it doesn't exist any more) and select it
    for (i = 0; i < m_doc->m_TriggerArray.GetSize (); i++)
      if (t == m_doc->m_TriggerArray [i])
        {
        t->bSelected = true;
        break;  // found it
        }
    }

	CPropertyPage::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CPluginWizardPage4 property page

CPluginWizardPage4::CPluginWizardPage4() : CPropertyPage(CPluginWizardPage4::IDD)
{
	//{{AFX_DATA_INIT(CPluginWizardPage4)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPluginWizardPage4::~CPluginWizardPage4()
{
}

void CPluginWizardPage4::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPluginWizardPage4)
	DDX_Control(pDX, IDC_ALIAS_LIST, m_ctlList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPluginWizardPage4, CPropertyPage)
	//{{AFX_MSG_MAP(CPluginWizardPage4)
	ON_BN_CLICKED(IDC_SELECT_ALL, OnSelectAll)
	ON_BN_CLICKED(IDC_SELECT_NONE, OnSelectNone)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CPluginWizardPage4::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
  m_ctlList.SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, 0, 
                        m_ctlList.SendMessage (LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) 
                        | LVS_EX_FULLROWSELECT 
                        | (App.m_bShowGridLinesInListViews ? LVS_EX_GRIDLINES : 0) );

  m_ctlList.InsertColumn(0, TranslateHeading ("Name"), LVCFMT_LEFT, 100);
  m_ctlList.InsertColumn(1, TranslateHeading ("Match"), LVCFMT_LEFT, 120);
  m_ctlList.InsertColumn(2, TranslateHeading ("Send"), LVCFMT_LEFT, 120);
  m_ctlList.InsertColumn(3, TranslateHeading ("Group"), LVCFMT_LEFT, 50);
 
  int iAlias,
      iItem = 0;

  for (iAlias = iItem = 0; iAlias < m_doc->m_AliasArray.GetSize (); iAlias++)
    {
    CAlias * a = m_doc->m_AliasArray [iAlias];

    if (a->bTemporary)
      continue;   // ignore temporary ones

    if (m_ctlList.InsertItem (iItem, a->strLabel) != -1)
      {
  	  m_ctlList.SetItemText(iItem, 1, a->name); 
  	  m_ctlList.SetItemText(iItem, 2, a->contents); 
  	  m_ctlList.SetItemText(iItem, 3, a->strGroup); 
      m_ctlList.SetItemData (iItem, (unsigned long) a);
      m_ctlList.SetItemState (iItem, LVIS_SELECTED, LVIS_SELECTED);
      }
    iItem++;   

    }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPluginWizardPage4::OnSelectAll() 
{
  for (int iItem = 0; iItem < m_ctlList.GetItemCount (); iItem++)
     m_ctlList.SetItemState (iItem, LVIS_SELECTED, LVIS_SELECTED);
	
}

void CPluginWizardPage4::OnSelectNone() 
{
  for (int iItem = 0; iItem < m_ctlList.GetItemCount (); iItem++)
     m_ctlList.SetItemState (iItem, 0, LVIS_SELECTED);
	
}


void CPluginWizardPage4::OnOK() 
{

  int i;

  // de-select everything
  for (i = 0; i < m_doc->m_AliasArray.GetSize (); i++)
    m_doc->m_AliasArray [i]->bSelected = false;
  
  for (int nItem = -1;
        (nItem = m_ctlList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
    {
    CAlias * a = (CAlias *) m_ctlList.GetItemData (nItem);
    // find in Alias array (in case it doesn't exist any more) and select it
    for (i = 0; i < m_doc->m_AliasArray.GetSize (); i++)
      if (a == m_doc->m_AliasArray [i])
        {
        a->bSelected = true;
        break;  // found it
        }
    }   // end of each list item

	CPropertyPage::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CPluginWizardPage5 property page

CPluginWizardPage5::CPluginWizardPage5() : CPropertyPage(CPluginWizardPage5::IDD)
{
	//{{AFX_DATA_INIT(CPluginWizardPage5)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPluginWizardPage5::~CPluginWizardPage5()
{
}

void CPluginWizardPage5::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPluginWizardPage5)
	DDX_Control(pDX, IDC_TIMER_LIST, m_ctlList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPluginWizardPage5, CPropertyPage)
	//{{AFX_MSG_MAP(CPluginWizardPage5)
	ON_BN_CLICKED(IDC_SELECT_ALL, OnSelectAll)
	ON_BN_CLICKED(IDC_SELECT_NONE, OnSelectNone)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
 
BOOL CPluginWizardPage5::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
  m_ctlList.SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, 0, 
                        m_ctlList.SendMessage (LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) 
                        | LVS_EX_FULLROWSELECT 
                        | (App.m_bShowGridLinesInListViews ? LVS_EX_GRIDLINES : 0) );

  m_ctlList.InsertColumn(0, TranslateHeading ("Name"), LVCFMT_LEFT, 100);
  m_ctlList.InsertColumn(1, TranslateHeading ("Time"), LVCFMT_LEFT, 120);
  m_ctlList.InsertColumn(2, TranslateHeading ("Send"), LVCFMT_LEFT, 120);
  m_ctlList.InsertColumn(3, TranslateHeading ("Group"), LVCFMT_LEFT, 50);
 
  int iItem = 0;
  POSITION pos;  
  CString strName;

  for (pos = m_doc->m_TimerMap.GetStartPosition(); pos; )
    {
    CTimer * t;
    m_doc->m_TimerMap.GetNextAssoc (pos, strName, t);  

    if (t->bTemporary)
      continue;   // ignore temporary ones

    if (m_ctlList.InsertItem (iItem, t->strLabel) != -1)
      {
      CString strTimer;

      if (t->iType == CTimer::eInterval)
        strTimer = TFormat ("Every %02i:%02i:%04.2f", 
                            t->iEveryHour, 
                            t->iEveryMinute,
                            t->fEverySecond);
      else
        strTimer = TFormat ("At %02i:%02i:%04.2f", 
                            t->iAtHour, 
                            t->iAtMinute,
                            t->fAtSecond);

  	  m_ctlList.SetItemText(iItem, 1, strTimer); 
  	  m_ctlList.SetItemText(iItem, 2, t->strContents); 
  	  m_ctlList.SetItemText(iItem, 3, t->strGroup); 
      m_ctlList.SetItemData (iItem, (unsigned long) t);
      m_ctlList.SetItemState (iItem, LVIS_SELECTED, LVIS_SELECTED);
      }
    iItem++;   

    }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPluginWizardPage5::OnSelectAll() 
{
  for (int iItem = 0; iItem < m_ctlList.GetItemCount (); iItem++)
     m_ctlList.SetItemState (iItem, LVIS_SELECTED, LVIS_SELECTED);
	
}

void CPluginWizardPage5::OnSelectNone() 
{
  for (int iItem = 0; iItem < m_ctlList.GetItemCount (); iItem++)
     m_ctlList.SetItemState (iItem, 0, LVIS_SELECTED);
	
}


void CPluginWizardPage5::OnOK() 
{
  POSITION pos;  
  CString strName;

  // first deselect everything
  for (pos = m_doc->m_TimerMap.GetStartPosition(); pos; )
    {
    CTimer * t;
    m_doc->m_TimerMap.GetNextAssoc (pos, strName, t); 
    t->bSelected = false;
    }
  
  for (int nItem = -1;
        (nItem = m_ctlList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
    {
    CTimer * t = (CTimer *) m_ctlList.GetItemData (nItem);
    // find in Timer array (in case it doesn't exist any more) and select it
    for (pos = m_doc->m_TimerMap.GetStartPosition(); pos; )
      {
      CTimer * t2;
      m_doc->m_TimerMap.GetNextAssoc (pos, strName, t2); 
        if (t == t2)
          {
          t->bSelected = true;
          break;
          }
      }
    }   // end of each list item

	CPropertyPage::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CPluginWizardPage6 property page

CPluginWizardPage6::CPluginWizardPage6() : CPropertyPage(CPluginWizardPage6::IDD)
{
	//{{AFX_DATA_INIT(CPluginWizardPage6)
	m_bSaveState = FALSE;
	//}}AFX_DATA_INIT
}

CPluginWizardPage6::~CPluginWizardPage6()
{
}

void CPluginWizardPage6::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPluginWizardPage6)
	DDX_Control(pDX, IDC_VARIABLE_LIST, m_ctlList);
	DDX_Check(pDX, IDC_RETAIN_STATE, m_bSaveState);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPluginWizardPage6, CPropertyPage)
	//{{AFX_MSG_MAP(CPluginWizardPage6)
	ON_BN_CLICKED(IDC_SELECT_ALL, OnSelectAll)
	ON_BN_CLICKED(IDC_SELECT_NONE, OnSelectNone)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CPluginWizardPage6::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
  m_ctlList.SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, 0, 
                        m_ctlList.SendMessage (LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) 
                        | LVS_EX_FULLROWSELECT 
                        | (App.m_bShowGridLinesInListViews ? LVS_EX_GRIDLINES : 0) );

  m_ctlList.InsertColumn(0, TranslateHeading ("Name"), LVCFMT_LEFT, 150);
  m_ctlList.InsertColumn(1, TranslateHeading ("Contents"), LVCFMT_LEFT, 200);
 
  int iItem = 0;
  POSITION pos;  
  CString strName;

  for (pos = m_doc->m_VariableMap.GetStartPosition(); pos; )
    {
    CVariable * v;
    m_doc->m_VariableMap.GetNextAssoc (pos, strName, v);  

    if (m_ctlList.InsertItem (iItem, v->strLabel) != -1)
      {
  	  m_ctlList.SetItemText(iItem, 1, v->strContents); 
      m_ctlList.SetItemData (iItem, (unsigned long) v);
      m_ctlList.SetItemState (iItem, LVIS_SELECTED, LVIS_SELECTED);
      }
    iItem++;   

    }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPluginWizardPage6::OnSelectAll() 
{
  for (int iItem = 0; iItem < m_ctlList.GetItemCount (); iItem++)
     m_ctlList.SetItemState (iItem, LVIS_SELECTED, LVIS_SELECTED);
	
}

void CPluginWizardPage6::OnSelectNone() 
{
  for (int iItem = 0; iItem < m_ctlList.GetItemCount (); iItem++)
     m_ctlList.SetItemState (iItem, 0, LVIS_SELECTED);
	
}


void CPluginWizardPage6::OnOK() 
{
  POSITION pos;  
  CString strName;

  // first deselect everything
  for (pos = m_doc->m_VariableMap.GetStartPosition(); pos; )
    {
    CVariable * v;
    m_doc->m_VariableMap.GetNextAssoc (pos, strName, v); 
    v->bSelected = false;
    }
  
  for (int nItem = -1;
        (nItem = m_ctlList.GetNextItem(nItem, LVNI_SELECTED)) != -1;)
    {
    CVariable * v = (CVariable *) m_ctlList.GetItemData (nItem);
    // find in Variable array (in case it doesn't exist any more) and select it
    for (pos = m_doc->m_VariableMap.GetStartPosition(); pos; )
      {
      CVariable * v2;
      m_doc->m_VariableMap.GetNextAssoc (pos, strName, v2); 
        if (v == v2)
          {
          v->bSelected = true;
          break;
          }
      }
    }   // end of each list item

	CPropertyPage::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CPluginWizardPage7 property page

CPluginWizardPage7::CPluginWizardPage7() : CPropertyPage(CPluginWizardPage7::IDD)
{
	//{{AFX_DATA_INIT(CPluginWizardPage7)
	m_bStandardConstants = FALSE;
	m_strScript = _T("");
	//}}AFX_DATA_INIT
}

CPluginWizardPage7::~CPluginWizardPage7()
{
}

void CPluginWizardPage7::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPluginWizardPage7)
	DDX_Control(pDX, IDC_SCRIPT, m_ctlScript);
	DDX_Control(pDX, IDC_LANGUAGE, m_ctlLanguage);
	DDX_Check(pDX, IDC_INCLUDE_CONSTANTS, m_bStandardConstants);
	DDX_Text(pDX, IDC_SCRIPT, m_strScript);
	//}}AFX_DATA_MAP

  if (pDX->m_bSaveAndValidate)
    {
    // description cannot have ]]> in it
    if (m_strScript.Find ("]]>") != -1)
      {
      ::TMessageBox ("Script may not contain the sequence \"]]>\"", MB_ICONSTOP);
      DDX_Text(pDX, IDC_SCRIPT, m_strScript);
      pDX->Fail();
      }

    } // end of saving and validating


}


BEGIN_MESSAGE_MAP(CPluginWizardPage7, CPropertyPage)
	//{{AFX_MSG_MAP(CPluginWizardPage7)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



BOOL CPluginWizardPage7::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
int nItem = -1;

  if (m_strLanguage.CompareNoCase ("vbscript") == 0)
    nItem = 0;
  else
  if (m_strLanguage.CompareNoCase ("jscript") == 0)
    nItem = 1;
  else
  if (m_strLanguage.CompareNoCase ("perlscript") == 0)
    nItem = 2;
#ifdef USE_PYTHON
  else
  if (m_strLanguage.CompareNoCase ("python") == 0)
    nItem = 3;
#endif
#ifdef USE_TCL
  else
  if (m_strLanguage.CompareNoCase ("tclscript") == 0)
    nItem = 4;
#endif
#ifdef USE_LUA
  else
  if (m_strLanguage.CompareNoCase ("lua") == 0)
    nItem = 5;
#endif
#ifdef USE_PHP
  else
  if (m_strLanguage.CompareNoCase ("phpscript") == 0)
    nItem = 6;
#endif
#ifdef USE_RUBY
  else
  if (m_strLanguage.CompareNoCase ("rubyscript") == 0)
    nItem = 7;
#endif

  m_ctlLanguage.SetCurSel(nItem);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPluginWizardPage7::OnOK() 
{
int nItem = m_ctlLanguage.GetCurSel ();

  switch (nItem)
    {
    case SCRIPT_VBSCRIPT:   m_strLanguage = "VBscript"; break;
    case SCRIPT_JSCRIPT:   m_strLanguage = "JScript"; break;
    case SCRIPT_PERL:   m_strLanguage = "PerlScript"; break;
#ifdef USE_PYTHON
    case SCRIPT_PYTHON:   m_strLanguage = "Python"; break;
#endif
#ifdef USE_TCL
    case SCRIPT_TCL:   m_strLanguage = "TclScript"; break;
#endif
#ifdef USE_LUA
    case SCRIPT_LUA:   m_strLanguage = "Lua"; break;
#endif
#ifdef USE_PHP
    case SCRIPT_PHP:   m_strLanguage = "PhpScript"; break;
#endif
#ifdef USE_RUBY
    case SCRIPT_RUBY:   m_strLanguage = "RubyScript"; break;
#endif
    default:  m_strLanguage = ""; break;
    }   // end of switch
	
	CPropertyPage::OnOK();
}

void CPluginWizardPage7::OnEdit() 
{
CEditMultiLine dlg;

  dlg.m_strText = GetText (m_ctlScript);

  dlg.m_strTitle = "Edit plugin script";

  dlg.m_bScript = true;

  if (m_doc->GetScriptEngine () && m_doc->GetScriptEngine ()->L)
    dlg.m_bLua = true;

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlScript.SetWindowText (dlg.m_strText);
	
}

/////////////////////////////////////////////////////////////////////////////
// CPluginWizardPage8 property page


CPluginWizardPage8::CPluginWizardPage8() : CPropertyPage(CPluginWizardPage8::IDD)
{
	//{{AFX_DATA_INIT(CPluginWizardPage8)
	m_strComments = _T("");
	//}}AFX_DATA_INIT
}

CPluginWizardPage8::~CPluginWizardPage8()
{
}

void CPluginWizardPage8::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPluginWizardPage8)
	DDX_Control(pDX, IDC_COMMENTS, m_ctlComments);
	DDX_Text(pDX, IDC_COMMENTS, m_strComments);
	//}}AFX_DATA_MAP

  if (pDX->m_bSaveAndValidate)
    {
    // description cannot have ]]> in it
    if (m_strComments.Find ("--") != -1)
      {
      ::TMessageBox ("Comments may not contain the sequence \"--\"", MB_ICONSTOP);
      DDX_Text(pDX, IDC_COMMENTS, m_strComments);
      pDX->Fail();
      }

    } // end of saving and validating

}


BEGIN_MESSAGE_MAP(CPluginWizardPage8, CPropertyPage)
	//{{AFX_MSG_MAP(CPluginWizardPage8)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPluginWizardPage8 message handlers

void CPluginWizardPage8::OnEdit() 
{
CEditMultiLine dlg;

  dlg.m_strText = GetText (m_ctlComments);

  dlg.m_strTitle = "Edit plugin comments";

  if (dlg.DoModal () != IDOK)
      return;

  m_ctlComments.SetWindowText (dlg.m_strText);
	
}



