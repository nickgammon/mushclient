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

#pragma warning (disable : 4800)  // forcing value to bool 'true' or 'false' (performance warning)

/////////////////////////////////////////////////////////////////////////////
// CFunctionListDlg dialog


CFunctionListDlg::CFunctionListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFunctionListDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFunctionListDlg)
	m_strFilter = _T("");
	//}}AFX_DATA_INIT
  m_bFunctions = false;
  m_bNoSort = false;
  m_L = NULL;
  m_iFilterIndex = LUA_NOREF;
  m_iFilterPrepIndex = LUA_NOREF;
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


// see Josuttis p499/500
bool nocase_compare (char c1, char c2)
  {
  return toupper (c1) == toupper (c2);
  }


BOOL CFunctionListDlg::ReloadList ()
  {

  m_ctlFunctions.DeleteAllItems ();

  m_strFilter.TrimLeft ();
  m_strFilter.TrimRight ();

  string sFilter (m_strFilter);

  // call filter "prep" function (eg. do FTS3 database lookup based on the wanted filter)

  if (m_L && m_iFilterPrepIndex != LUA_NOREF)
    {

    lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_iFilterPrepIndex);

    // Lua filter:  function f (filter)  ... end

    // what they have currently typed
    lua_pushlstring (m_L, sFilter.c_str (), sFilter.size ());

    // call the function: arg1: filter field, arg2: key, arg3: value
    if (lua_pcall (m_L, 1, 0, 0))   // call with 3 args and 0 results
      {
      LuaError (m_L);    // note that this clears the stack, so we won't call it again
      lua_settop (m_L, 0);   // clear stack, just in case LuaError changes behaviour
      }   // end of error

    }  // end of Lua filter prep function available


  // filter based on a partial match on what is in the filter box
  // (eg. "chat" would find all chat functions)

  int nItem = 0;
  int nKeynum = 0;
  for (vector<CKeyValuePair>::const_iterator it = m_data.begin ();
       it != m_data.end ();
       it++, nKeynum++)
    {
    CKeyValuePair kv = *it;
    string sValue = kv.sValue_;
    bool bWanted = false;

   if (m_L && m_iFilterIndex != LUA_NOREF)
      {

      lua_rawgeti (m_L, LUA_REGISTRYINDEX, m_iFilterIndex);

      // Lua filter:  function f (filter, key, value)  ... end

      // what they have currently typed
      lua_pushlstring (m_L, sFilter.c_str (), sFilter.size ());

      // push number or string which is the key
      if (kv.bNumber_)
        lua_pushnumber (m_L, kv.iKey_);
      else
        lua_pushlstring (m_L, kv.sKey_.c_str (), kv.sKey_.size ());

      // push value
      lua_pushlstring (m_L, kv.sValue_.c_str (), kv.sValue_.size ());

      // call the function: arg1: filter field, arg2: key, arg3: value
      if (lua_pcall (m_L, 3, 1, 0))   // call with 3 args and 1 result
        {
        LuaError (m_L);    // note that this clears the stack, so we won't call it again
        lua_settop (m_L, 0);   // clear stack, just in case LuaError changes behaviour
        bWanted = false;
        }   // end of error
      else
        {
        bWanted = lua_toboolean (m_L, -1);
        lua_pop (m_L, 1);  // pop result
        }  // end of no error

      }  // end of Lua filter function available
    else
      // no Lua function, just do a substring compare
      bWanted = sFilter.empty () || search (sValue.begin (), sValue.end (),
                                    sFilter.begin (), sFilter.end (),
                                    nocase_compare) != sValue.end ();

    if (bWanted)
      {
      int iPos = m_ctlFunctions.InsertItem (nItem, sValue.c_str ());
      if (iPos != -1)
        m_ctlFunctions.SetItemData (iPos, nKeynum); // sorting changes the position

      // select the exact match, if any (so, if they highlight world.Note then it is selected)

      if (sValue == sFilter)
        m_ctlFunctions.SetItemState (nItem, 
                                      LVIS_FOCUSED | LVIS_SELECTED,
                                      LVIS_FOCUSED | LVIS_SELECTED);
      nItem++;

      }  // end of wanted in list
   }  // end of for loop


  // if the filtering results in a single item, select it
  if (nItem == 1)
    {
    m_ctlFunctions.SetItemState (0, 
                                  LVIS_FOCUSED | LVIS_SELECTED,
                                  LVIS_FOCUSED | LVIS_SELECTED);
    return FALSE;
    }

  // if no filter value, put focus there so they can type one in
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

  SetWindowText (m_strTitle);	

  // hide buttons if we are not showing functions
  if (!m_bFunctions)
    {
    GetDlgItem(IDC_LUA_FUNCTIONS)->ShowWindow (SW_HIDE);
    GetDlgItem(IDC_COPY_NAME)->ShowWindow (SW_HIDE);
    }

  if (m_bNoSort)
    {
    m_ctlFunctions.ModifyStyle (LVS_SORTASCENDING, 0);
    m_ctlFunctions.ModifyStyle (LVS_SORTDESCENDING, 0);
    }

	return ReloadList ();  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CFunctionListDlg::OnOK() 
{
  int iWhich = m_ctlFunctions.GetNextItem(-1, LVNI_SELECTED);


  if (iWhich != -1)
    {
    int nKeynum = m_ctlFunctions.GetItemData (iWhich);
    m_result = m_data [nKeynum];
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
  m_result.sValue_ = "DOC_lua";
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


