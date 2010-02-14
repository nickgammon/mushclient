// FunctionListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "FunctionListDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern set<string> LuaFunctionsSet;

/////////////////////////////////////////////////////////////////////////////
// CFunctionListDlg dialog


CFunctionListDlg::CFunctionListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFunctionListDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFunctionListDlg)
	m_strFilter = _T("");
	//}}AFX_DATA_INIT
  m_bLua = false;
}


void CFunctionListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFunctionListDlg)
	DDX_Control(pDX, IDC_FILTER, m_ctlFilter);
	DDX_Control(pDX, IDC_FUNCTIONS_LIST, m_ctlFunctions);
	DDX_Text(pDX, IDC_FILTER, m_strFilter);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFunctionListDlg, CDialog)
	//{{AFX_MSG_MAP(CFunctionListDlg)
	ON_NOTIFY(NM_DBLCLK, IDC_FUNCTIONS_LIST, OnDblclkFunctionsList)
	ON_BN_CLICKED(IDC_LUA_FUNCTIONS, OnLuaFunctions)
	ON_BN_CLICKED(IDC_COPY_NAME, OnCopyName)
	ON_EN_CHANGE(IDC_FILTER, OnChangeFilter)
	//}}AFX_MSG_MAP
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_COPY_NAME, OnUpdateNeedSelection)
  ON_UPDATE_COMMAND_UI(IDOK, OnUpdateNeedSelection)

END_MESSAGE_MAP()

extern const char * sFunctions [1];

BOOL CFunctionListDlg::ReloadList ()
  {

  m_ctlFunctions.DeleteAllItems ();

  m_strFilter.MakeLower ();
  m_strFilter.TrimLeft ();
  m_strFilter.TrimRight ();

  // filter based on a partial match on what is in the filter box
  // (eg. "chat" would find all chat functions)

  CString strFunction;

  int nItem = 0;

  for (int i = 0; sFunctions [i] [0]; i++)
    {
    strFunction = sFunctions [i];
    strFunction.MakeLower ();

    if (m_strFilter.IsEmpty () || strFunction.Find (m_strFilter) != -1)
      {
      m_ctlFunctions.InsertItem (nItem, sFunctions [i]);

      // select the exact match, if any (so, if they highlight world.Note then it is selected)

      if (strFunction == m_strFilter)
        m_ctlFunctions.SetItemState (nItem, 
                                      LVIS_FOCUSED | LVIS_SELECTED,
                                      LVIS_FOCUSED | LVIS_SELECTED);
      nItem++;

      }
    }

  // add Lua functions
  if (m_bLua)
    {
    for (set<string>::const_iterator it = LuaFunctionsSet.begin ();
         it != LuaFunctionsSet.end (); 
         it++)

       {
        strFunction = it->c_str ();
        strFunction.MakeLower ();

        if (m_strFilter.IsEmpty () || strFunction.Find (m_strFilter) != -1)
          {
          m_ctlFunctions.InsertItem (nItem, it->c_str ());

          // select the exact match, if any (so, if they highlight world.Note then it is selected)

          if (strFunction == m_strFilter)
            m_ctlFunctions.SetItemState (nItem, 
                                          LVIS_FOCUSED | LVIS_SELECTED,
                                          LVIS_FOCUSED | LVIS_SELECTED);
          nItem++;

          }
           
       }   // end of doing each Lua function
    }


  // if the filtering results in a single item, select it
  if (nItem == 1)
    {
    m_ctlFunctions.SetItemState (0, 
                                  LVIS_FOCUSED | LVIS_SELECTED,
                                  LVIS_FOCUSED | LVIS_SELECTED);
    return FALSE;
    }

  if (m_strFilter.IsEmpty ())
    {
    m_ctlFilter.SetFocus ();
    return FALSE;
    }

  return TRUE;

  } // end of CFunctionListDlg::ReloadList

/////////////////////////////////////////////////////////////////////////////
// CFunctionListDlg message handlers

BOOL CFunctionListDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	return ReloadList ();  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CFunctionListDlg::OnOK() 
{
  int iWhich = m_ctlFunctions.GetNextItem(-1, LVNI_SELECTED);


  if (iWhich != -1)
    {
    string sFunctionName = (LPCTSTR) m_ctlFunctions.GetItemText (iWhich, 0);

    // might be Lua function
    if (LuaFunctionsSet.find (sFunctionName) != LuaFunctionsSet.end ())
      m_strResult = CFormat ("LUA_%s", sFunctionName.c_str ());
    else
      m_strResult = CFormat ("FNC_%s", sFunctionName.c_str ());

  	CDialog::OnOK();
    }
	
}

void CFunctionListDlg::OnDblclkFunctionsList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnOK ();	
	
	*pResult = 0;
}

void CFunctionListDlg::OnLuaFunctions() 
{
  m_strResult = "DOC_lua";
  CDialog::OnOK();
}

void CFunctionListDlg::OnCopyName() 
{

  int iWhich = m_ctlFunctions.GetNextItem(-1, LVNI_SELECTED);

  if (iWhich != -1)
    {
    putontoclipboard (m_ctlFunctions.GetItemText (iWhich, 0));
    CDialog::OnOK();
	  }
  else
    ::TMessageBox ("No function selected");
}

void CFunctionListDlg::OnChangeFilter() 
{
  m_strFilter = GetText (m_ctlFilter);
	
  ReloadList ();

}


LRESULT CFunctionListDlg::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CFunctionListDlg::OnKickIdle

void CFunctionListDlg::OnUpdateNeedSelection(CCmdUI* pCmdUI)
{
  int iWhich = m_ctlFunctions.GetNextItem(-1, LVNI_SELECTED);
	pCmdUI->Enable(iWhich != -1);
}  // end of CFunctionListDlg::OnUpdateNeedSelection

