// CompleteWordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "CompleteWordDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern set<string> LuaFunctionsSet;
extern set<string> LuaTablesSet;

/////////////////////////////////////////////////////////////////////////////
// CCompleteWordDlg dialog


CCompleteWordDlg::CCompleteWordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCompleteWordDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCompleteWordDlg)
	//}}AFX_DATA_INIT

m_bLua = false;
m_bFunctions = true;
m_extraItems = NULL;
}


void CCompleteWordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCompleteWordDlg)
	DDX_Control(pDX, IDC_WORD_LIST, m_ctlFunctions);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCompleteWordDlg, CDialog)
	//{{AFX_MSG_MAP(CCompleteWordDlg)
	ON_LBN_DBLCLK(IDC_WORD_LIST, OnDblclkWordList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCompleteWordDlg message handlers


static const char * sNoMatches = "(no matches)";

extern const char * sFunctions [1];

void CCompleteWordDlg::ReloadList ()
  {

  m_ctlFunctions.ResetContent ();

  m_strFilter.MakeLower ();
  m_strFilter.TrimLeft ();
  m_strFilter.TrimRight ();

  // filter based on a partial match on what is in the filter box
  // (eg. "chat" would find all chat functions)

  CString strFunction;
  int iFilterLength = m_strFilter.GetLength ();

  int iCount = 0;

  if (m_bFunctions)
    {
    for (int i = 0; sFunctions [i] [0]; i++)
      {
      strFunction = sFunctions [i];
      strFunction.MakeLower ();
      strFunction = strFunction.Left (iFilterLength);

      if (m_strFilter == strFunction)
        {
        m_ctlFunctions.AddString  (sFunctions [i]);
        iCount++;
        }

      }
      
    // add Lua functions
    if (m_bLua)
      {
      set<string>::const_iterator it;

      for (it = LuaFunctionsSet.begin ();
           it != LuaFunctionsSet.end (); 
           it++)

         {
          strFunction = it->c_str ();
          strFunction.MakeLower ();
          strFunction = strFunction.Left (iFilterLength);

          if (m_strFilter == strFunction)
            {
            m_ctlFunctions.AddString  (it->c_str ());
            iCount++;
            }
         }   // end of doing each Lua function

      for (it = LuaTablesSet.begin ();
           it != LuaTablesSet.end (); 
           it++)

         {
          strFunction = it->c_str ();
          strFunction.MakeLower ();
          strFunction = strFunction.Left (iFilterLength);

          if (m_strFilter == strFunction)
            {
            m_ctlFunctions.AddString  (it->c_str ());
            iCount++;
            }
         }   // end of doing each Lua function

      }  // if Lua wanted
    } // end of functions wanted
    
  if (m_extraItems)
    {
    set<string>::const_iterator it;

    for (it = m_extraItems->begin ();
         it != m_extraItems->end (); 
         it++)

       {
        strFunction = it->c_str ();
        strFunction.MakeLower ();
        strFunction = strFunction.Left (iFilterLength);

        if (m_strFilter == strFunction)
          {
          m_ctlFunctions.AddString  (it->c_str ());
          iCount++;
          }
       }   // end of doing each Lua function
    }

  // select the exact match, if any (so, if they highlight world.Note then it is selected)

  m_ctlFunctions.SelectString (-1, m_strFilter);

  if (iCount == 0)
    {
    m_ctlFunctions.AddString (sNoMatches);
    m_ctlFunctions.EnableWindow (FALSE);
    }

  } // end of CFunctionListDlg::ReloadList

BOOL CCompleteWordDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  ReloadList ();
	
  int iHeight;
  int iWidth;

  GetButtonSize (*this, iHeight, iWidth);

  SetWindowPos (NULL, m_pt.x, m_pt.y - (iHeight / 2), 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCompleteWordDlg::OnDblclkWordList() 
{
  OnOK ();	
	
}

void CCompleteWordDlg::OnOK() 
{
  int iWhich = m_ctlFunctions.GetCurSel ();

  if (iWhich != LB_ERR )
    m_ctlFunctions.GetText (iWhich, m_strResult);

  // can't return our "no matches" string
  if (m_strResult == sNoMatches)
    m_strResult.Empty ();

	CDialog::OnOK();
}
