// PluginWizard.h : header file
//

#ifndef __PLUGINWIZARD_H__
#define __PLUGINWIZARD_H__

class CPluginWizardPage2;

/////////////////////////////////////////////////////////////////////////////
// CPluginWizardPage1 dialog

class CPluginWizardPage1 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPluginWizardPage1)

// Construction
public:
	CPluginWizardPage1();
	~CPluginWizardPage1();

// Dialog Data
	//{{AFX_DATA(CPluginWizardPage1)
	enum { IDD = IDD_PLUGINWIZARDPAGE1 };
	CString	m_strAuthor;
	CString	m_strDateWritten;
	CString	m_strPurpose;
	CString	m_strVersion;
	CString	m_strName;
	CString	m_strID;
	double	m_fRequires;
	BOOL	m_bRemoveItems;
	//}}AFX_DATA

  CMUSHclientDoc * m_doc;           // which document this belongs to

  CString m_strFilename;    // output file name

  CPluginWizardPage2 * m_pPage2;  // for mucking around with help name

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPluginWizardPage1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPluginWizardPage1)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CPluginWizardPage2 dialog

class CPluginWizardPage2 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPluginWizardPage2)

// Construction
public:
	CPluginWizardPage2();
	~CPluginWizardPage2();

// Dialog Data
	//{{AFX_DATA(CPluginWizardPage2)
	enum { IDD = IDD_PLUGINWIZARDPAGE2 };
	CButton	m_ctlGenerateHelp;
	CEdit	m_ctlDescription;
	CString	m_strDescription;
	BOOL	m_bGenerateHelp;
	CString	m_strHelpAlias;
	//}}AFX_DATA

  CMUSHclientDoc * m_doc;           // which document this belongs to

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPluginWizardPage2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPluginWizardPage2)
	afx_msg void OnEdit();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedDescription(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CPluginWizardPage3 dialog

class CPluginWizardPage3 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPluginWizardPage3)

// Construction
public:
	CPluginWizardPage3();
	~CPluginWizardPage3();

// Dialog Data
	//{{AFX_DATA(CPluginWizardPage3)
	enum { IDD = IDD_PLUGINWIZARDPAGE3 };
	CListCtrl	m_ctlList;
	//}}AFX_DATA

  CMUSHclientDoc * m_doc;           // which document this belongs to

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPluginWizardPage3)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPluginWizardPage3)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectAll();
	afx_msg void OnSelectNone();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CPluginWizardPage4 dialog

class CPluginWizardPage4 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPluginWizardPage4)

// Construction
public:
	CPluginWizardPage4();
	~CPluginWizardPage4();

// Dialog Data
	//{{AFX_DATA(CPluginWizardPage4)
	enum { IDD = IDD_PLUGINWIZARDPAGE4 };
	CListCtrl	m_ctlList;
	//}}AFX_DATA

  CMUSHclientDoc * m_doc;           // which document this belongs to

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPluginWizardPage4)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPluginWizardPage4)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectAll();
	afx_msg void OnSelectNone();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CPluginWizardPage5 dialog

class CPluginWizardPage5 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPluginWizardPage5)

// Construction
public:
	CPluginWizardPage5();
	~CPluginWizardPage5();

// Dialog Data
	//{{AFX_DATA(CPluginWizardPage5)
	enum { IDD = IDD_PLUGINWIZARDPAGE5 };
	CListCtrl	m_ctlList;
	//}}AFX_DATA

  CMUSHclientDoc * m_doc;           // which document this belongs to

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPluginWizardPage5)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPluginWizardPage5)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectAll();
	afx_msg void OnSelectNone();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CPluginWizardPage6 dialog

class CPluginWizardPage6 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPluginWizardPage6)

// Construction
public:
	CPluginWizardPage6();
	~CPluginWizardPage6();

// Dialog Data
	//{{AFX_DATA(CPluginWizardPage6)
	enum { IDD = IDD_PLUGINWIZARDPAGE6 };
	CListCtrl	m_ctlList;
	BOOL	m_bSaveState;
	//}}AFX_DATA


  CMUSHclientDoc * m_doc;           // which document this belongs to

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPluginWizardPage6)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPluginWizardPage6)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectAll();
	afx_msg void OnSelectNone();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CPluginWizardPage7 dialog

class CPluginWizardPage7 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPluginWizardPage7)

// Construction
public:
	CPluginWizardPage7();
	~CPluginWizardPage7();

// Dialog Data
	//{{AFX_DATA(CPluginWizardPage7)
	enum { IDD = IDD_PLUGINWIZARDPAGE7 };
	CEdit	m_ctlScript;
	CComboBox	m_ctlLanguage;
	BOOL	m_bStandardConstants;
	CString	m_strScript;
	//}}AFX_DATA

  CMUSHclientDoc * m_doc;           // which document this belongs to

  CString m_strLanguage;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPluginWizardPage7)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPluginWizardPage7)
	virtual BOOL OnInitDialog();
	afx_msg void OnEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};



#endif // __PLUGINWIZARD_H__
/////////////////////////////////////////////////////////////////////////////
// CPluginWizardPage8 dialog

class CPluginWizardPage8 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPluginWizardPage8)

// Construction
public:
	CPluginWizardPage8();
	~CPluginWizardPage8();

// Dialog Data
	//{{AFX_DATA(CPluginWizardPage8)
	enum { IDD = IDD_PLUGINWIZARDPAGE8 };
	CEdit	m_ctlComments;
	CString	m_strComments;
	//}}AFX_DATA

  CMUSHclientDoc * m_doc;           // which document this belongs to

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPluginWizardPage8)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPluginWizardPage8)
	afx_msg void OnEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
