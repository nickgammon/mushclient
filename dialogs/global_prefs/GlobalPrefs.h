// CGlobalPrefs.h : header file
//

#ifndef __CGlobalPREFS_H__
#define __CGlobalPREFS_H__

#include "..\world_prefs\ButtonColour.h"

class CMUSHclientDoc;

/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP1 dialog

class CGlobalPrefsP1 : public CPropertyPage
{
	DECLARE_DYNCREATE(CGlobalPrefsP1)

// Construction
public:
	CGlobalPrefsP1();
	~CGlobalPrefsP1();

// Dialog Data
	//{{AFX_DATA(CGlobalPrefsP1)
	enum { IDD = IDD_GLOBAL_PREFSP1 };
	CStatic	m_ctlSelectedWorld;
	CString	m_strDefaultWorldFileDirectory;
	//}}AFX_DATA

  CMUSHclientDoc * m_pCurrentDoc;

  CString m_strWorldList;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGlobalPrefsP1)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGlobalPrefsP1)
	afx_msg void OnNewStartupWorld();
	afx_msg void OnRemoveStartupWorld();
	virtual BOOL OnInitDialog();
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	afx_msg void OnDefaultDirectory();
	afx_msg void OnAddCurrentWorld();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedSelection(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedCurrentWorld(CCmdUI* pCmdUI);
  afx_msg void OnUpdateSelectedWorld(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP2 dialog

class CGlobalPrefsP2 : public CPropertyPage
{
	DECLARE_DYNCREATE(CGlobalPrefsP2)

// Construction
public:
	CGlobalPrefsP2();
	~CGlobalPrefsP2();

// Dialog Data
	//{{AFX_DATA(CGlobalPrefsP2)
	enum { IDD = IDD_GLOBAL_PREFSP2 };
	BOOL	m_bAutoConnectWorlds;
	BOOL	m_bReconnectOnLinkFailure;
	BOOL	m_bOpenWorldsMaximised;
	BOOL	m_bNotifyOnDisconnect;
	BOOL	m_bEnableSpellCheck;
	CString	m_strWordDelimiters;
	CString	m_strWordDelimitersDblClick;
	BOOL	m_bAutoExpand;
	BOOL	m_bColourGradient;
	BOOL	m_bBleedBackground;
	BOOL	m_bFixedFontForEditing;
	BOOL	m_bSmootherScrolling;
	BOOL	m_bAllTypingToCommandWindow;
	BOOL	m_bDisableKeyboardMenuActivation;
	BOOL	m_bSmoothScrolling;
	BOOL	m_bRegexpMatchEmpty;
	BOOL	m_bNotifyIfCannotConnect;
	BOOL	m_bShowGridLinesInListViews;
	BOOL	m_bF1macro;
	BOOL	m_bTriggerRemoveCheck;
	int		m_iWindowTabsStyle;
	BOOL	m_bFlatToolbars;
	BOOL	m_bErrorNotificationToOutputWindow;
	CString	m_strLocale;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGlobalPrefsP2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGlobalPrefsP2)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP3 dialog

class CGlobalPrefsP3 : public CPropertyPage
{
	DECLARE_DYNCREATE(CGlobalPrefsP3)

// Construction
public:
	CGlobalPrefsP3();
	~CGlobalPrefsP3();

// Dialog Data
	//{{AFX_DATA(CGlobalPrefsP3)
	enum { IDD = IDD_GLOBAL_PREFSP3 };
	BOOL	m_bConfirmBeforeClosingMushclient;
	BOOL	m_bConfirmBeforeClosingWorld;
	BOOL	m_bConfirmBeforeClosingMXPdebug;
	BOOL	m_bConfirmBeforeSavingVariables;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGlobalPrefsP3)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGlobalPrefsP3)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP4 dialog

class CGlobalPrefsP4 : public CPropertyPage
{
	DECLARE_DYNCREATE(CGlobalPrefsP4)

// Construction
public:
	CGlobalPrefsP4();
	~CGlobalPrefsP4();

// Dialog Data
	//{{AFX_DATA(CGlobalPrefsP4)
	enum { IDD = IDD_GLOBAL_PREFSP4 };
	CStatic	m_ctlFontName;
	CString	m_strPrinterFont;
	UINT	m_nPrinterLeftMargin;
	UINT	m_nPrinterTopMargin;
	UINT	m_nPrinterLinesPerPage;
	//}}AFX_DATA

  unsigned int m_nPrinterFontSize;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGlobalPrefsP4)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGlobalPrefsP4)
	afx_msg void OnFont();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP5 dialog

class CGlobalPrefsP5 : public CPropertyPage
{
	DECLARE_DYNCREATE(CGlobalPrefsP5)

// Construction
public:
	CGlobalPrefsP5();
	~CGlobalPrefsP5();

// Dialog Data
	//{{AFX_DATA(CGlobalPrefsP5)
	enum { IDD = IDD_GLOBAL_PREFSP5 };
	BOOL	m_bAutoLogWorld;
	BOOL	m_bAppendToLogFiles;
	CString	m_strDefaultLogFileDirectory;
	BOOL	m_bConfirmLogFileClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGlobalPrefsP5)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGlobalPrefsP5)
	afx_msg void OnDefaultDirectory();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP6 dialog

class CGlobalPrefsP6 : public CPropertyPage
{
	DECLARE_DYNCREATE(CGlobalPrefsP6)

// Construction
public:
	CGlobalPrefsP6();
	~CGlobalPrefsP6();

// Dialog Data
	//{{AFX_DATA(CGlobalPrefsP6)
	enum { IDD = IDD_GLOBAL_PREFSP6 };
	UINT	m_nTimerInterval;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGlobalPrefsP6)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGlobalPrefsP6)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP7 dialog

class CGlobalPrefsP7 : public CPropertyPage
{
	DECLARE_DYNCREATE(CGlobalPrefsP7)

// Construction
public:
	CGlobalPrefsP7();
	~CGlobalPrefsP7();

// Dialog Data
	//{{AFX_DATA(CGlobalPrefsP7)
	enum { IDD = IDD_GLOBAL_PREFSP7 };
	BOOL	m_bOpenActivityWindow;
	UINT	m_nActivityWindowRefreshInterval;
	int		m_nActivityWindowRefreshType;
	int		m_iActivityButtonBarStyle;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGlobalPrefsP7)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGlobalPrefsP7)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP9 dialog

class CGlobalPrefsP9 : public CPropertyPage
{
	DECLARE_DYNCREATE(CGlobalPrefsP9)

// Construction
public:
	CGlobalPrefsP9();
	~CGlobalPrefsP9();

  int m_iDefaultInputFontHeight;
  int m_iDefaultInputFontWeight;
  BOOL m_iDefaultInputFontItalic;
  DWORD m_iDefaultOutputFontCharset;
  DWORD m_iDefaultInputFontCharset;

  int m_iDefaultOutputFontHeight;

// Dialog Data
	//{{AFX_DATA(CGlobalPrefsP9)
	enum { IDD = IDD_GLOBAL_PREFSP9 };
	CEdit	m_ctlInputStyle;
	CEdit	m_ctlOutputStyle;
	CEdit	m_ctlDefaultInputFont;
	CEdit	m_ctlDefaultOutputFont;
	CEdit	m_ctlDefaultTimersFile;
	CEdit	m_ctlDefaultMacrosFile;
	CEdit	m_ctlDefaultAliasesFile;
	CEdit	m_ctlDefaultTriggersFile;
	CEdit	m_ctlDefaultColoursFile;
	CString	m_strDefaultColoursFile;
	CString	m_strDefaultTriggersFile;
	CString	m_strDefaultAliasesFile;
	CString	m_strDefaultMacrosFile;
	CString	m_strDefaultTimersFile;
	CString	m_strDefaultOutputFont;
	CString	m_strDefaultInputFont;
	CString	m_strInputStyle;
	CString	m_strOutputStyle;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGlobalPrefsP9)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

  bool GeneralBrowse (const int set_type,
                      CString & strPathName);

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGlobalPrefsP9)
	afx_msg void OnBrowseColours();
	afx_msg void OnBrowseTriggers();
	afx_msg void OnBrowseAliases();
	afx_msg void OnBrowseMacros();
	afx_msg void OnBrowseTimers();
	afx_msg void OnSelectOutputFont();
	afx_msg void OnSelectInputFont();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP10 dialog

class CGlobalPrefsP10 : public CPropertyPage
{
	DECLARE_DYNCREATE(CGlobalPrefsP10)

// Construction
public:
	CGlobalPrefsP10();
	~CGlobalPrefsP10();

// Dialog Data
	//{{AFX_DATA(CGlobalPrefsP10)
	enum { IDD = IDD_GLOBAL_PREFSP10 };
	CColourButton	m_ctlBackgroundSwatch;
	CColourButton	m_ctlTextSwatch;
	CButton	m_ctlEscapeDoubleQuotes;
	CButton	m_ctlEscapeSingleQuotes;
	CButton	m_ctlDoubleQuotes;
	CButton	m_ctlSingleQuotes;
	CButton	m_ctlPercentEscapes;
	CButton	m_ctlBackslashEscapes;
	CButton	m_ctlNestBraces;
	CEdit	m_ctlNotepadStyle;
	CEdit	m_ctlNotepadFont;
	CString	m_strNotepadFont;
	CString	m_strNotepadStyle;
	BOOL	m_bNotepadWordWrap;
	CString	m_strNotepadQuoteString;
	//}}AFX_DATA

  unsigned int m_nParenMatchFlags;

  COLORREF m_cNotepadTextColour;
  COLORREF m_cNotepadBackColour;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGlobalPrefsP10)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGlobalPrefsP10)
	afx_msg void OnTextSwatch();
	afx_msg void OnBackgroundSwatch();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedSingleQuotes(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedDoubleQuotes(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP11 dialog

class CGlobalPrefsP11 : public CPropertyPage
{
	DECLARE_DYNCREATE(CGlobalPrefsP11)

// Construction
public:
	CGlobalPrefsP11();
	~CGlobalPrefsP11();

// Dialog Data
	//{{AFX_DATA(CGlobalPrefsP11)
	enum { IDD = IDD_GLOBAL_PREFSP11 };
	CStatic	m_ctlFileName;
	int		m_iShowIconPlace;
	CString	m_strTrayIconFileName;
	int		m_iIconType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGlobalPrefsP11)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGlobalPrefsP11)
	afx_msg void OnChooseIconFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP12 dialog

class CGlobalPrefsP12 : public CPropertyPage
{
	DECLARE_DYNCREATE(CGlobalPrefsP12)

// Construction
public:
	CGlobalPrefsP12();
	~CGlobalPrefsP12();

// Dialog Data
	//{{AFX_DATA(CGlobalPrefsP12)
	enum { IDD = IDD_GLOBAL_PREFSP12 };
	CStatic	m_ctlSelectedPlugin;
	CString	m_strPluginsDirectory;
	//}}AFX_DATA

  CString m_strPluginList;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGlobalPrefsP12)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGlobalPrefsP12)
	afx_msg void OnPluginsDirectory();
	afx_msg void OnNewPlugin();
	afx_msg void OnRemovePlugin();
	virtual BOOL OnInitDialog();
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedSelection(CCmdUI* pCmdUI);
  afx_msg void OnUpdateSelectedPlugin(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CGlobalPrefsP13 dialog

class CGlobalPrefsP13 : public CPropertyPage
{
	DECLARE_DYNCREATE(CGlobalPrefsP13)

// Construction
public:
	CGlobalPrefsP13();
	~CGlobalPrefsP13();

// Dialog Data
	//{{AFX_DATA(CGlobalPrefsP13)
	enum { IDD = IDD_GLOBAL_PREFSP13 };
	CEdit	m_ctlScript;
	CString	m_strLuaScript;
	BOOL	m_bEnablePackageLibrary;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGlobalPrefsP13)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGlobalPrefsP13)
	afx_msg void OnEdit();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

};
#endif // __CGlobalPREFS_H__
