#if !defined(AFX_LUACHOOSELIST_H__6E9EEA74_586E_11DA_9963_00008C012785__INCLUDED_)
#define AFX_LUACHOOSELIST_H__6E9EEA74_586E_11DA_9963_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LuaChooseList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLuaChooseList dialog

class CLuaChooseList : public CDialog
{
// Construction
public:
	CLuaChooseList(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLuaChooseList)
	enum { IDD = IDD_LUA_CHOOSE_LIST };
	CListCtrl	m_ctlListBox;
	CString	m_strMessage;
	//}}AFX_DATA

  CString m_strTitle;

  vector<CKeyValuePair> m_data;

  int m_iDefault;   // default key item (vector number)

  int m_iResult;    // result item (vector number)


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLuaChooseList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLuaChooseList)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblclkChooseList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LUACHOOSELIST_H__6E9EEA74_586E_11DA_9963_00008C012785__INCLUDED_)
