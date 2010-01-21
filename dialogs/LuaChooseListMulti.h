#if !defined(AFX_LUACHOOSELISTMULTI_H__43B68B3B_226F_11DB_9979_00008C012785__INCLUDED_)
#define AFX_LUACHOOSELISTMULTI_H__43B68B3B_226F_11DB_9979_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LuaChooseListMulti.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLuaChooseListMulti dialog

class CLuaChooseListMulti : public CDialog
{
// Construction
public:
	CLuaChooseListMulti(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLuaChooseListMulti)
	enum { IDD = IDD_LUA_CHOOSE_LIST_MULTI };
	CListCtrl	m_ctlListBox;
	CString	m_strMessage;
	//}}AFX_DATA

  CString m_strTitle;

  vector<CKeyValuePair> m_data;

  set<int> m_iDefaults;   // default key items (vector number)

  set<int> m_iResults;    // result items (vector number)

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLuaChooseListMulti)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLuaChooseListMulti)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblclkChooseList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LUACHOOSELISTMULTI_H__43B68B3B_226F_11DB_9979_00008C012785__INCLUDED_)
