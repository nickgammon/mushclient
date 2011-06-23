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
m_commandHistoryItems = NULL;
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


CString strNoMatches = "(no matches)";

extern tInternalFunctionsTable InternalFunctionsTable [1];

void CCompleteWordDlg::ReloadList ()
  {

  m_ctlFunctions.ResetContent ();

  if (m_commandHistoryItems)
    {
    strNoMatches = "(no matching commands)";
    if (m_commandHistoryItems->empty ())
      {
      m_ctlFunctions.AddString (strNoMatches);
      m_ctlFunctions.EnableWindow (FALSE);
      return;
      }

    string sLastOne;

    for (vector<string>::const_iterator it = m_commandHistoryItems->begin ();
         it != m_commandHistoryItems->end ();
         it++)
           {
            m_ctlFunctions.AddString  (it->c_str ());
            sLastOne = *it;
           }

//    m_ctlFunctions.SelectString (-1, sLastOne.c_str ());

    return;
    }

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
    for (int i = 0; InternalFunctionsTable [i].sFunction [0]; i++)
      {
      strFunction = InternalFunctionsTable [i].sFunction;
      strFunction.MakeLower ();
      strFunction = strFunction.Left (iFilterLength);

      if (m_strFilter == strFunction)
        {
        m_ctlFunctions.AddString  (InternalFunctionsTable [i].sFunction);
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

  if (!m_strFilter.IsEmpty ())
    m_ctlFunctions.SelectString (-1, m_strFilter);
  else if (!m_strDefault.IsEmpty ())
    m_ctlFunctions.SelectString (-1, m_strDefault);

  if (iCount == 0)
    {
    m_ctlFunctions.AddString (strNoMatches);
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
  if (m_strResult == strNoMatches)
    m_strResult.Empty ();
  else
    {
    for (int i = 0; InternalFunctionsTable [i].sFunction [0]; i++)
      {
      if (m_strResult == InternalFunctionsTable [i].sFunction)
        {
        m_strArgs = InternalFunctionsTable [i].sArgs;
        break;
        } // end if function found
      }  // end for
    }  // not "no matches"

	CDialog::OnOK();
}
