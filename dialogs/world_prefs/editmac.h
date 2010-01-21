// editmac.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditMacro dialog

class CEditMacro : public CDialog
{
// Construction
public:
	CEditMacro(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditMacro)
	enum { IDD = IDD_EDIT_MACRO };
	CEdit	m_ctlSend;
	CString	m_macro;
	int		m_macro_type;
	CString	m_macro_desc;
	//}}AFX_DATA

  CString m_strTitle;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditMacro)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditMacro)
	afx_msg void OnHelpbutton();
	afx_msg void OnEdit();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
