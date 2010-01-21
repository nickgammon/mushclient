#pragma once

#include "ButtonColour.h"
#include "genpropertypage.h"
#include "StatLink.h"

class CPrefsP3;
class CPrefsP5;
class CPrefsP14;

/////////////////////////////////////////////////////////////////////////////
// CPrefsP1 dialog

class CPrefsP1 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP1)

// Construction
public:
	CPrefsP1();
	~CPrefsP1();

  CMUSHclientDoc * m_doc;
  int m_page_number;

	CStaticLink	m_MudListLink;
	CStaticLink	m_BugReportLink;

  CString m_strProxyUserName;
  CString m_strProxyPassword;

// Dialog Data
	//{{AFX_DATA(CPrefsP1)
	enum { IDD = IDD_PREFS_P1 };
	long	m_port;
	CString	m_mush_name;
	CString	m_server;
	BOOL	m_bSaveWorldAutomatically;
	CString	m_strMudListAddress;
	CString	m_strBugReportAddress;
	int		m_iSocksProcessing;
	CString	m_strProxyServerName;
	long	m_iProxyServerPort;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP1)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsP1)
	virtual BOOL OnInitDialog();
	afx_msg void OnProxyServerPassword();
	afx_msg void OnClearCached();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateCanClearCache(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CPrefsP2 dialog

class CPrefsP2 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP2)

// Construction
public:
	CPrefsP2();
	~CPrefsP2();

  CMUSHclientDoc * m_doc;
  int m_page_number;

// Dialog Data
	//{{AFX_DATA(CPrefsP2)
	enum { IDD = IDD_PREFS_P2 };
	CString	m_sound_pathname;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP2)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsP2)
	afx_msg void OnBrowseSounds();
	afx_msg void OnTestSound();
	afx_msg void OnNoSound();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedSound(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CPrefsP3 dialog

class CPrefsP3 : public CPropertyPage
{

friend class CPrefsP5;

	DECLARE_DYNCREATE(CPrefsP3)

// Construction
public:
	CPrefsP3();
	~CPrefsP3();

  CMUSHclientDoc * m_doc;
  int m_page_number;
  CPrefsP5 * m_prefsp5;

  int m_RH_click_count;   // for Easter Egg
  CTime m_RH_last_click;

// Dialog Data
	//{{AFX_DATA(CPrefsP3)
	enum { IDD = IDD_PREFS_P3 };
	CEdit	m_ctlCustomName9;
	CEdit	m_ctlCustomName8;
	CEdit	m_ctlCustomName7;
	CEdit	m_ctlCustomName6;
	CEdit	m_ctlCustomName5;
	CEdit	m_ctlCustomName4;
	CEdit	m_ctlCustomName3;
	CEdit	m_ctlCustomName2;
	CEdit	m_ctlCustomName16;
	CEdit	m_ctlCustomName15;
	CEdit	m_ctlCustomName14;
	CEdit	m_ctlCustomName13;
	CEdit	m_ctlCustomName12;
	CEdit	m_ctlCustomName11;
	CEdit	m_ctlCustomName10;
	CEdit	m_ctlCustomName1;
	CColourButton	m_ctlCustom9;
	CColourButton	m_ctlCustom8;
	CColourButton	m_ctlCustom7;
	CColourButton	m_ctlCustom6;
	CColourButton	m_ctlCustom5;
	CColourButton	m_ctlCustom4;
	CColourButton	m_ctlCustom32;
	CColourButton	m_ctlCustom31;
	CColourButton	m_ctlCustom30;
	CColourButton	m_ctlCustom3;
	CColourButton	m_ctlCustom29;
	CColourButton	m_ctlCustom28;
	CColourButton	m_ctlCustom27;
	CColourButton	m_ctlCustom26;
	CColourButton	m_ctlCustom25;
	CColourButton	m_ctlCustom24;
	CColourButton	m_ctlCustom23;
	CColourButton	m_ctlCustom22;
	CColourButton	m_ctlCustom21;
	CColourButton	m_ctlCustom20;
	CColourButton	m_ctlCustom2;
	CColourButton	m_ctlCustom19;
	CColourButton	m_ctlCustom18;
	CColourButton	m_ctlCustom17;
	CColourButton	m_ctlCustom16;
	CColourButton	m_ctlCustom15;
	CColourButton	m_ctlCustom14;
	CColourButton	m_ctlCustom13;
	CColourButton	m_ctlCustom12;
	CColourButton	m_ctlCustom11;
	CColourButton	m_ctlCustom10;
	CColourButton	m_ctlCustom1;
	CColourButton	m_ctlYellowSwatchBold;
	CColourButton	m_ctlYellowSwatch;
	CColourButton	m_ctlWhiteSwatchBold;
	CColourButton	m_ctlWhiteSwatch;
	CColourButton	m_ctlRedSwatchBold;
	CColourButton	m_ctlRedSwatch;
	CColourButton	m_ctlMagentaSwatchBold;
	CColourButton	m_ctlMagentaSwatch;
	CColourButton	m_ctlGreenSwatchBold;
	CColourButton	m_ctlGreenSwatch;
	CColourButton	m_ctlCyanSwatchBold;
	CColourButton	m_ctlCyanSwatch;
	CColourButton	m_ctlBlueSwatchBold;
	CColourButton	m_ctlBlueSwatch;
	CColourButton	m_ctlBlackSwatchBold;
	CColourButton	m_ctlBlackSwatch;
	//}}AFX_DATA

  // custom (user-defined) colours for triggers etc.
  COLORREF m_customtext [MAX_CUSTOM];
  COLORREF m_customback [MAX_CUSTOM];
  CString m_strCustomName [MAX_CUSTOM];

  void amendcolour (CColourButton & whichcolour);

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP3)
	public:
	virtual BOOL OnSetActive();
	virtual void OnOK();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

  void LoadSwatches (void);
  void SaveSwatches (void);

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsP3)
	afx_msg void OnSwatchCustom1();
	afx_msg void OnSwatchCustom2();
	afx_msg void OnSwatchCustom10();
	afx_msg void OnSwatchCustom11();
	afx_msg void OnSwatchCustom12();
	afx_msg void OnSwatchCustom13();
	afx_msg void OnSwatchCustom14();
	afx_msg void OnSwatchCustom15();
	afx_msg void OnSwatchCustom16();
	afx_msg void OnSwatchCustom17();
	afx_msg void OnSwatchCustom18();
	afx_msg void OnSwatchCustom19();
	afx_msg void OnSwatchCustom20();
	afx_msg void OnSwatchCustom21();
	afx_msg void OnSwatchCustom22();
	afx_msg void OnSwatchCustom23();
	afx_msg void OnSwatchCustom24();
	afx_msg void OnSwatchCustom25();
	afx_msg void OnSwatchCustom26();
	afx_msg void OnSwatchCustom27();
	afx_msg void OnSwatchCustom28();
	afx_msg void OnSwatchCustom29();
	afx_msg void OnSwatchCustom3();
	afx_msg void OnSwatchCustom30();
	afx_msg void OnSwatchCustom31();
	afx_msg void OnSwatchCustom32();
	afx_msg void OnSwatchCustom4();
	afx_msg void OnSwatchCustom5();
	afx_msg void OnSwatchCustom6();
	afx_msg void OnSwatchCustom7();
	afx_msg void OnSwatchCustom8();
	afx_msg void OnSwatchCustom9();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	virtual BOOL OnInitDialog();
	afx_msg void OnInvert();
	afx_msg void OnAllLighter();
	afx_msg void OnAllDarker();
	afx_msg void OnAllMoreColour();
	afx_msg void OnAllLessColour();
	afx_msg void OnDefaults();
	afx_msg void OnSwap();
	afx_msg void OnRandom();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNotUsingDefaults(CCmdUI* pCmdUI);
DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CPrefsP4 dialog

class CPrefsP4 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP4)

// Construction
public:
	CPrefsP4();
	~CPrefsP4();

  CMUSHclientDoc * m_doc;
  int m_page_number;

// Dialog Data
	//{{AFX_DATA(CPrefsP4)
	enum { IDD = IDD_PREFS_P4 };
	CEdit	m_ctlPostamble;
	CEdit	m_ctlPreamble;
	CButton	m_ctlLogHTML;
	CEdit	m_ctlLogFileName;
	BOOL	m_log_input;
	BOOL	m_bLogNotes;
	CString	m_strLogFilePreamble;
	CString	m_strAutoLogFileName;
	BOOL	m_bWriteWorldNameToLog;
	CString	m_strLogLinePreambleOutput;
	CString	m_strLogLinePreambleInput;
	CString	m_strLogLinePreambleNotes;
	CString	m_strLogFilePostamble;
	CString	m_strLogLinePostambleOutput;
	CString	m_strLogLinePostambleInput;
	CString	m_strLogLinePostambleNotes;
	BOOL	m_bLogOutput;
	BOOL	m_bLogHTML;
	BOOL	m_bLogInColour;
	BOOL	m_bLogRaw;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP4)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsP4)
	afx_msg void OnBrowse();
	afx_msg void OnStandardPreamble();
	afx_msg void OnEditPreamble();
	afx_msg void OnEditPostamble();
	afx_msg void OnSubsitutionHelp();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedHTML(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CPrefsP5 dialog

class CPrefsP5 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP5)

// Construction
public:
	CPrefsP5();
	~CPrefsP5();

  CMUSHclientDoc * m_doc;
  int m_page_number;

  CPrefsP3 * m_prefsp3;

  COLORREF m_normalcolour [8];
  COLORREF m_boldcolour [8];

// Dialog Data
	//{{AFX_DATA(CPrefsP5)
	enum { IDD = IDD_PREFS_P5 };
	CButton	m_ctlUseDefaultColours;
	CColourButton	m_ctlYellowSwatchBold;
	CColourButton	m_ctlYellowSwatch;
	CColourButton	m_ctlWhiteSwatchBold;
	CColourButton	m_ctlWhiteSwatch;
	CColourButton	m_ctlRedSwatchBold;
	CColourButton	m_ctlRedSwatch;
	CColourButton	m_ctlMagentaSwatchBold;
	CColourButton	m_ctlMagentaSwatch;
	CColourButton	m_ctlGreenSwatchBold;
	CColourButton	m_ctlGreenSwatch;
	CColourButton	m_ctlCyanSwatchBold;
	CColourButton	m_ctlCyanSwatch;
	CColourButton	m_ctlBlueSwatchBold;
	CColourButton	m_ctlBlueSwatch;
	CColourButton	m_ctlBlackSwatchBold;
	CColourButton	m_ctlBlackSwatch;
	BOOL	m_bUseDefaultColours;
	BOOL	m_bCustom16isDefaultColour;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP5)
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

  void amendcolour (CColourButton & whichcolour);
  void LoadSwatches (void);
  void SaveSwatches (void);

  // Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsP5)
	afx_msg void OnSaveColours();
	afx_msg void OnLoadColours();
	virtual BOOL OnInitDialog();
	afx_msg void OnAnsiColours();
	afx_msg void OnBlackSwatch();
	afx_msg void OnRedSwatch();
	afx_msg void OnGreenSwatch();
	afx_msg void OnYellowSwatch();
	afx_msg void OnBlueSwatch();
	afx_msg void OnMagentaSwatch();
	afx_msg void OnCyanSwatch();
	afx_msg void OnWhiteSwatch();
	afx_msg void OnBlackSwatchBold();
	afx_msg void OnRedSwatchBold();
	afx_msg void OnGreenSwatchBold();
	afx_msg void OnYellowSwatchBold();
	afx_msg void OnBlueSwatchBold();
	afx_msg void OnMagentaSwatchBold();
	afx_msg void OnCyanSwatchBold();
	afx_msg void OnWhiteSwatchBold();
	afx_msg void OnSwap();
	afx_msg void OnNormalLighter();
	afx_msg void OnNormalDarker();
	afx_msg void OnBoldLighter();
	afx_msg void OnBoldDarker();
	afx_msg void OnAllLighter();
	afx_msg void OnAllDarker();
	afx_msg void OnAllMoreColour();
	afx_msg void OnAllLessColour();
	afx_msg void OnCopyToCustom();
	afx_msg void OnInvert();
	afx_msg void OnRandom();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNotUsingDefaults(CCmdUI* pCmdUI);
  afx_msg void OnUpdateHaveDefaults(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CPrefsP6 dialog

class CPrefsP6 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP6)

// Construction
public:
	CPrefsP6();
	~CPrefsP6();

  CMUSHclientDoc * m_doc;
  int m_page_number;
  CFindInfo * m_pMacrosFindInfo;

// Dialog Data
	//{{AFX_DATA(CPrefsP6)
	enum { IDD = IDD_PREFS_P6 };
	CButton	m_ctlUseDefaultMacros;
	CListCtrl	m_ctlMacroList;
	BOOL	m_bUseDefaultMacros;
	//}}AFX_DATA

    enum 
    { eColumnName,
      eColumnText,
      eColumnAction,
      eColumnCount      // this must be last!
    };

  // for sorting the list

  int m_last_col;
  BOOL m_reverse;

  CString m_macros [MACRO_COUNT];                // for function keys
  CString m_macro_type_alpha [MACRO_COUNT];      // "Replace", "Send", or "Add"
  unsigned short m_macro_type [MACRO_COUNT];     // actual macro type

  void load_list (void);

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP6)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  static int CALLBACK CompareFunc ( LPARAM lParam1, 
                                    LPARAM lParam2,
                                    LPARAM lParamSort);
  
// for finding

  static void InitiateSearch (const CObject * pObject,
                              CFindInfo & FindInfo);
 
  static bool GetNextLine (const CObject * pObject,
                           CFindInfo & FindInfo, 
                           CString & strLine);

  void DoFind (bool bAgain);
  
  // Generated message map functions
	//{{AFX_MSG(CPrefsP6)
	afx_msg void OnEditMacro();
	afx_msg void OnDblclkMacrosList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickMacrosList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLoadMacro();
	afx_msg void OnSaveMacro();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnFind();
	afx_msg void OnFindNext();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedSelection(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedEntries(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNotUsingDefaults(CCmdUI* pCmdUI);
  afx_msg void OnUpdateHaveDefaults(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CPrefsP7 dialog

class CPrefsP7 : public CGenPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP7)

// Construction
public:
	CPrefsP7();
	~CPrefsP7();

// Dialog Data
	//{{AFX_DATA(CPrefsP7)
	enum { IDD = IDD_PREFS_P7 };
	CStatic	m_ctlSummary;
	CButton	m_ctlFilter;
	CButton	m_ctlUseDefaultAliases;
	CListCtrl	m_ctlAliasList;
	BOOL	m_enable_aliases;
	BOOL	m_bUseDefaultAliases;
	//}}AFX_DATA

  enum 
    { eColumnAlias,
      eColumnSequence,
      eColumnContents,
      eColumnLabel,
      eColumnGroup,
      eColumnCount      // this must be last!
    };

  // ================== start of virtual functions =======================

  // dialog management - initialise, load, unload, check if changed, get name
  virtual void InitDialog (CDialog * pDlg);          // sets up for new dialog
  virtual void LoadDialog (CDialog * pDlg,             // move from item to dialog
                           CObject * pItem) ; 
  virtual void UnloadDialog (CDialog * pDlg,             // move from dialog to item
    CObject * pItem); 
  virtual bool CheckIfChanged (CDialog * pDlg,           // has the user changed anything?
                               CObject * pItem) const;
  virtual CString GetObjectName (CDialog * pDlg) const;    // gets name from dialog

  // object setup - create a new one, set its modification number, set its dispatch ID
  virtual CObject * MakeNewObject (void);            // create new item, e.g. trigger
  virtual void SetModificationNumber (CObject * pItem, 
                                      __int64 m_nUpdateNumber);   // set modification number
  virtual void SetDispatchID (CObject * pItem, const DISPID dispid);         // set script dispatch ID
  virtual void SetInternalName (CObject * pItem, const CString strName);    // name after creation

  // get info about the object
  virtual __int64 GetModificationNumber (CObject * pItem) const;   // get modification number
  virtual CString GetScriptName (CObject * pItem) const;    // get script subroutine name
  virtual CString GetLabel (CObject * pItem) const;    // get label name

  // list management - add the item to the list control
  virtual int AddItem (CObject * pItem,                  // add one item to the list control
                       const int nItemNumber,
                       const BOOL nItem);

  virtual bool CheckIfIncluded (CObject * pItem);
  virtual bool CheckIfExecuting (CObject * pItem);

  virtual bool GetFilterFlag ();            // is filtering enabled?
  virtual CString GetFilterScript ();       // get the filter script

  // ================== end of virtual functions =======================

  static int CompareObjects     (const int iColumn, 
                                 const CObject * item1, 
                                 const CObject * item2);

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP7)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  // Generated message map functions
	//{{AFX_MSG(CPrefsP7)
	afx_msg void OnAddAlias();
	afx_msg void OnChangeAlias();
	afx_msg void OnDeleteAlias();
	afx_msg void OnDblclkAliasesList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickAliasesList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLoadAlias();
	afx_msg void OnSaveAlias();
	virtual BOOL OnInitDialog();
	afx_msg void OnCopy();
	afx_msg void OnPaste();
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	afx_msg void OnFilter();
	afx_msg void OnEditFilter();
	//}}AFX_MSG
  afx_msg void OnUpdateNotUsingDefaults(CCmdUI* pCmdUI);
  afx_msg void OnUpdateHaveDefaults(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedSelection(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedOneSelection(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedXMLClipboard(CCmdUI* pCmdUI);
  afx_msg void OnUpdateCanSequence(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CPrefsP8 dialog

class CPrefsP8 : public CGenPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP8)

// Construction
public:
	CPrefsP8();
	~CPrefsP8();

// Dialog Data
	//{{AFX_DATA(CPrefsP8)
	enum { IDD = IDD_PREFS_P8 };
	CStatic	m_ctlSummary;
	CButton	m_ctlFilter;
	CButton	m_ctlUseDefaultTriggers;
	CListCtrl	m_ctlTriggerList;
	BOOL	m_enable_triggers;
	BOOL	m_enable_trigger_sounds;
	BOOL	m_bUseDefaultTriggers;
	BOOL	m_bFilter;
	//}}AFX_DATA

  enum 
    { 
      eColumnTrigger,
      eColumnSequence,
      eColumnContents,
      eColumnLabel,
      eColumnGroup,
      eColumnCount      // this must be last!
    };

  CPrefsP3 * m_prefsp3;
  CPrefsP14 * m_prefsp14;

//  CImageList m_imglIcons;

  // ================== start of virtual functions =======================

  // dialog management - initialise, load, unload, check if changed, get name
  virtual void InitDialog (CDialog * pDlg);          // sets up for new dialog
  virtual void LoadDialog (CDialog * pDlg,             // move from item to dialog
                           CObject * pItem); 
  virtual void UnloadDialog (CDialog * pDlg,             // move from dialog to item
    CObject * pItem); 
  virtual bool CheckIfChanged (CDialog * pDlg,           // has the user changed anything?
                               CObject * pItem) const;
  virtual CString GetObjectName (CDialog * pDlg) const;    // gets name from dialog

  // object setup - create a new one, set its modification number, set its dispatch ID
  virtual CObject * MakeNewObject (void);            // create new item, e.g. trigger
  virtual void SetModificationNumber (CObject * pItem, 
                                      __int64 m_nUpdateNumber);   // set modification number
  virtual void SetDispatchID (CObject * pItem, const DISPID dispid);         // set script dispatch ID
  virtual void SetInternalName (CObject * pItem, const CString strName);    // name after creation

  // get info about the object
  virtual __int64 GetModificationNumber (CObject * pItem) const;   // get modification number
  virtual CString GetScriptName (CObject * pItem) const;    // get script subroutine name
  virtual CString GetLabel (CObject * pItem) const;    // get label name

  // list management - add the item to the list control
  virtual int AddItem (CObject * pItem,                  // add one item to the list control
                       const int nItemNumber,
                       const BOOL nItem);

  virtual bool CheckIfIncluded (CObject * pItem);
  virtual bool CheckIfExecuting (CObject * pItem);

  virtual bool GetFilterFlag ();            // is filtering enabled?
  virtual CString GetFilterScript ();       // get the filter script

  // ================== end of virtual functions =======================

  static int CompareObjects     (const int iColumn, 
                                 const CObject * item1, 
                                 const CObject * item2);

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP8)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  
  // Generated message map functions
	//{{AFX_MSG(CPrefsP8)
	afx_msg void OnAddTrigger();
	afx_msg void OnChangeTrigger();
	afx_msg void OnDeleteTrigger();
	afx_msg void OnDblclkTriggersList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickTriggersList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLoadTrigger();
	afx_msg void OnSaveTrigger();
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	afx_msg void OnCopy();
	afx_msg void OnPaste();
	afx_msg void OnEditFilter();
	afx_msg void OnFilter();
	//}}AFX_MSG
  afx_msg void OnUpdateCanSequence(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNotUsingDefaults(CCmdUI* pCmdUI);
  afx_msg void OnUpdateHaveDefaults(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedSelection(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedOneSelection(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedXMLClipboard(CCmdUI* pCmdUI);
  DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CPrefsP9 dialog

class CPrefsP9 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP9)

// Construction
public:
	CPrefsP9();
	~CPrefsP9();

  CMUSHclientDoc * m_doc;
  int m_page_number;

  COLORREF m_input_text_colour;
  COLORREF m_input_background_colour;

  LONG    m_input_font_height;               
  BYTE    m_input_font_italic;            
  LONG    m_input_font_weight;  
  DWORD   m_input_font_charset;
  CString m_strTabCompletionDefaults;
  int   m_iTabCompletionLines;

	BOOL	m_bConfirmBeforeReplacingTyping;
	BOOL	m_bEscapeDeletesInput;
	BOOL	m_bArrowsChangeHistory;
	BOOL	m_bAltArrowRecallsPartial;
	BOOL	m_bArrowRecallsPartial;
	BOOL	m_bDoubleClickInserts;
	BOOL	m_bDoubleClickSends;
	BOOL	m_bSaveDeletedCommand;
	BOOL	m_bArrowKeysWrap;
  BOOL  m_bCtrlZGoesToEndOfBuffer;    
  BOOL  m_bCtrlPGoesToPreviousCommand;
  BOOL  m_bCtrlNGoesToNextCommand; 
  BOOL  m_bTabCompletionSpace;
                                      
  CPrefsP3 * m_prefsp3;

// Dialog Data
	//{{AFX_DATA(CPrefsP9)
	enum { IDD = IDD_PREFS_P9 };
	CStatic	m_ctlInputStyle;
	CButton	m_ctlUseDefaultInputFont;
	CColourComboBox	m_ctlInputColour;
	CColourButton	m_ctlInputSwatch2;
	CStatic	m_ctlFontName;
	CColourButton	m_ctlBackgroundSwatch;
	CColourButton	m_ctlTextSwatch;
	CColourButton	m_ctlInputSwatch;
	CString	m_command_stack_character;
	BOOL	m_enable_command_stacking;
	BOOL	m_enable_speed_walk;
	CString	m_speed_walk_prefix;
	BOOL	m_echo_input;
	CString	m_input_font_name;
	int		m_nHistoryLines;
	BOOL	m_bAutoRepeat;
	BOOL	m_bTranslateGerman;
	BOOL	m_bSpellCheckOnSend;
	CString	m_strSpeedWalkFiller;
	BOOL	m_bLowerCaseTabCompletion;
	int		m_echo_colour;
	BOOL	m_bUseDefaultInputFont;
	CString	m_strInputStyle;
	BOOL	m_bTranslateBackslashSequences;
	BOOL	m_bKeepCommandsOnSameLine;
	short	m_iSpeedWalkDelay;
	BOOL	m_bNoEchoOff;
	BOOL	m_bEnableSpamPrevention;
	int		m_iSpamLineCount;
	CString	m_strSpamMessage;
	//}}AFX_DATA

  void FixSwatch (CColourButton & m_ctlSwatch, 
                  CColourButton & m_ctlSwatch2, 
                  const int iNewColour);

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP9)
	public:
	virtual void OnOK();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsP9)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeEchoColour();
	afx_msg void OnInputFont();
	afx_msg void OnTabCompletionDefaults();
	afx_msg void OnTextSwatch();
	afx_msg void OnBackgroundSwatch();
	afx_msg void OnKeyboardPrefs();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedSpellCheck(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNotUsingDefaults(CCmdUI* pCmdUI);
  afx_msg void OnUpdateHaveDefaults(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CPrefsP10 dialog

class CPrefsP10 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP10)

// Construction
public:
	CPrefsP10();
	~CPrefsP10();

// Dialog Data
	//{{AFX_DATA(CPrefsP10)
	enum { IDD = IDD_PREFS_P10 };
	CString	m_file_postamble;
	CString	m_file_preamble;
	CString	m_line_postamble;
	CString	m_line_preamble;
	BOOL	m_bCommentedSoftcode;
	long	m_iLineDelay;
	BOOL	m_bConfirmOnSend;
	BOOL	m_bEcho;
	long	m_nFileDelayPerLines;
	//}}AFX_DATA

  CMUSHclientDoc * m_doc;
  int m_page_number;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP10)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsP10)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CPrefsP11 dialog

class CPrefsP11 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP11)

// Construction
public:
	CPrefsP11();
	~CPrefsP11();

// Dialog Data
	//{{AFX_DATA(CPrefsP11)
	enum { IDD = IDD_PREFS_P11 };
	CEdit	m_ctlNotes;
	CString	m_notes;
	//}}AFX_DATA


  CMUSHclientDoc * m_doc;
  int m_page_number;
  CFindInfo * m_pNotesFindInfo;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP11)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

// for finding

  void DoFind (bool bAgain);


	// Generated message map functions
	//{{AFX_MSG(CPrefsP11)
	afx_msg void OnLoadNotes();
	afx_msg void OnSaveNotes();
	afx_msg void OnSetfocusNotes();
	afx_msg void OnFind();
	afx_msg void OnFindNext();
	afx_msg void OnEdit();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedText(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CPrefsP12 dialog

class CPrefsP12 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP12)

// Construction
public:
	CPrefsP12();
	~CPrefsP12();

// Dialog Data
	//{{AFX_DATA(CPrefsP12)
	enum { IDD = IDD_PREFS_P12 };
	CButton	m_ctlControl;
	BOOL	m_keypad_enable;
	CString	m_keypad_0;
	CString	m_keypad_1;
	CString	m_keypad_2;
	CString	m_keypad_3;
	CString	m_keypad_4;
	CString	m_keypad_5;
	CString	m_keypad_6;
	CString	m_keypad_7;
	CString	m_keypad_8;
	CString	m_keypad_9;
	CString	m_keypad_dash;
	CString	m_keypad_dot;
	CString	m_keypad_plus;
	CString	m_keypad_slash;
	CString	m_keypad_star;
	BOOL	m_bControl;
	//}}AFX_DATA

  CString m_keypad [eKeypad_Max_Items];

  CMUSHclientDoc * m_doc;
  int m_page_number;

  bool m_bFirstTime;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP12)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsP12)
	afx_msg void OnControlKey();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CPrefsP13 dialog

class CPrefsP13 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP13)

// Construction
public:
	CPrefsP13();
	~CPrefsP13();

// Dialog Data
	//{{AFX_DATA(CPrefsP13)
	enum { IDD = IDD_PREFS_P13 };
	CString	m_paste_postamble;
	CString	m_paste_preamble;
	CString	m_pasteline_postamble;
	CString	m_pasteline_preamble;
	BOOL	m_bConfirmOnPaste;
	BOOL	m_bCommentedSoftcode;
	long	m_iLineDelay;
	BOOL	m_bEcho;
	long	m_nPasteDelayPerLines;
	//}}AFX_DATA

  CMUSHclientDoc * m_doc;
  int m_page_number;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP13)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsP13)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CPrefsP14 dialog

class CPrefsP14 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP14)

// Construction
public:
	CPrefsP14();
	~CPrefsP14();

  CMUSHclientDoc * m_doc;
  int m_page_number;

  LONG    m_font_height;               
  LONG    m_font_weight;   
  DWORD   m_font_charset;

// Dialog Data
	//{{AFX_DATA(CPrefsP14)
	enum { IDD = IDD_PREFS_P14 };
	CButton	m_ctlCopySelectionToClipboard;
	CStatic	m_ctlOutputStyle;
	CButton	m_ctlUseDefaultOutputFont;
	CEdit	m_ctlWrapColumn;
	CStatic	m_ctlFontName;
	BOOL	m_enable_beeps;
	CString	m_font_name;
	BOOL	m_indent_paras;
	BOOL	m_wrap_output;
	long	m_nLines;
	UINT	m_nWrapColumn;
	BOOL	m_bLineInformation;
	BOOL	m_bStartPaused;
	BOOL	m_bShowBold;
	int		m_iPixelOffset;
	BOOL	m_bAutoFreeze;
	BOOL	m_bDisableCompression;
	BOOL	m_bFlashIcon;
	BOOL	m_bShowItalic;
	BOOL	m_bShowUnderline;
	BOOL	m_bUseDefaultOutputFont;
	CString	m_strOutputStyle;
	BOOL	m_bUnpauseOnSend;
	BOOL	m_bAlternativeInverse;
	BOOL	m_bAutoWrapWindowWidth;
	CString	m_strBeepSound;
	BOOL	m_bNAWS;
	CString	m_strTerminalIdentification;
	BOOL	m_bShowConnectDisconnect;
	BOOL	m_bCopySelectionToClipboard;
	BOOL	m_bAutoCopyInHTML;
	short	m_iLineSpacing;
	BOOL	m_bUTF_8;
	BOOL	m_bCarriageReturnClearsLine;
	CString	m_sound_pathname;
	BOOL	m_bConvertGAtoNewline;
	//}}AFX_DATA

  long m_nOldLines;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP14)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsP14)
	afx_msg void OnChooseFont();
	virtual BOOL OnInitDialog();
	afx_msg void OnAdjustWidth();
	afx_msg void OnBrowseSounds();
	afx_msg void OnTestSound();
	afx_msg void OnAdjustToWidth();
	afx_msg void OnBrowseSounds2();
	afx_msg void OnTestSound2();
	afx_msg void OnNoSound();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNotUsingDefaults(CCmdUI* pCmdUI);
  afx_msg void OnUpdateHaveDefaults(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedSound(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedActivitySound(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedCopyToClipboard(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CPrefsP15 dialog

class CPrefsP15 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP15)

// Construction
public:
	CPrefsP15();
	~CPrefsP15();

  CMUSHclientDoc * m_doc;
  int m_page_number;

// Dialog Data
	//{{AFX_DATA(CPrefsP15)
	enum { IDD = IDD_PREFS_P15 };
	CString	m_strBufferLines;
	CString	m_strConnectionDuration;
	CString	m_strConnectionTime;
	CString	m_strAliases;
	CString	m_strTriggers;
	CString	m_strCompressionRatio;
	CString	m_strBytesSent;
	CString	m_strBytesReceived;
	CString	m_strTimers;
	CString	m_strTriggerTimeTaken;
	CString	m_strIpAddress;
	CString	m_strMXPinbuiltElements;
	CString	m_strMXPinbuiltEntities;
	CString	m_strMXPentitiesReceived;
	CString	m_strMXPerrors;
	CString	m_strMXPmudElements;
	CString	m_strMXPmudEntities;
	CString	m_strMXPtagsReceived;
	CString	m_strMXPunclosedTags;
	CString	m_strCompressedIn;
	CString	m_strCompressedOut;
	CString	m_strTimeTakenCompressing;
	CString	m_strMXPactionsCached;
	CString	m_strMXPreferenceCount;
	//}}AFX_DATA


  void CalculateMemoryUsage ();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP15)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsP15)
	virtual BOOL OnInitDialog();
	afx_msg void OnCalculateMemory();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CPrefsP16 dialog

class CPrefsP16 : public CGenPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP16)

// Construction
public:
	CPrefsP16();
	~CPrefsP16();

  enum 
    { eColumnType,
      eColumnWhen,
      eColumnContents,
      eColumnLabel,
      eColumnGroup,
      eColumnNext,
      eColumnCount      // this must be last!
    };

  // ================== start of virtual functions =======================

  // dialog management - initialise, load, unload, check if changed, get name
  virtual void InitDialog (CDialog * pDlg);          // sets up for new dialog
  virtual void LoadDialog (CDialog * pDlg,             // move from item to dialog
                           CObject * pItem) ; 
  virtual void UnloadDialog (CDialog * pDlg,             // move from dialog to item
    CObject * pItem); 
  virtual bool CheckIfChanged (CDialog * pDlg,           // has the user changed anything?
                               CObject * pItem) const;
  virtual CString GetObjectName (CDialog * pDlg) const;    // gets name from dialog

  // object setup - create a new one, set its modification number, set its dispatch ID
  virtual CObject * MakeNewObject (void);            // create new item, e.g. trigger
  virtual void SetModificationNumber (CObject * pItem, 
                                      __int64 m_nUpdateNumber);   // set modification number
  virtual void SetDispatchID (CObject * pItem, const DISPID dispid);         // set script dispatch ID
  virtual void SetInternalName (CObject * pItem, const CString strName);    // name after creation

  // get info about the object
  virtual __int64 GetModificationNumber (CObject * pItem) const;   // get modification number
  virtual CString GetScriptName (CObject * pItem) const;    // get script subroutine name
  virtual CString GetLabel (CObject * pItem) const;    // get label name

  // list management - add the item to the list control
  virtual int AddItem (CObject * pItem,                  // add one item to the list control
                       const int nItemNumber,
                       const BOOL nItem);

  virtual bool CheckIfIncluded (CObject * pItem);
  virtual bool CheckIfExecuting (CObject * pItem);

  virtual bool GetFilterFlag ();            // is filtering enabled?
  virtual CString GetFilterScript ();       // get the filter script

  // ================== end of virtual functions =======================

  static int CompareObjects     (const int iColumn, 
                                 const CObject * item1, 
                                 const CObject * item2);
// Dialog Data
	//{{AFX_DATA(CPrefsP16)
	enum { IDD = IDD_PREFS_P16 };
	CStatic	m_ctlSummary;
	CButton	m_ctlFilter;
	CButton	m_ctlUseDefaultTimers;
	CListCtrl	m_ctlTimerList;
	BOOL	m_bEnableTimers;
	BOOL	m_bUseDefaultTimers;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP16)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  // Generated message map functions
	//{{AFX_MSG(CPrefsP16)
	afx_msg void OnAddTimer();
	afx_msg void OnChangeTimer();
	afx_msg void OnDeleteTimer();
	afx_msg void OnLoadTimers();
	afx_msg void OnResetTimers();
	afx_msg void OnSaveTimers();
	afx_msg void OnDblclkTimersList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickTimersList(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnEnableTimers();
	afx_msg void OnCopy();
	afx_msg void OnPaste();
	afx_msg void OnFilter();
	afx_msg void OnEditFilter();
	//}}AFX_MSG
  afx_msg void OnUpdateNeedSelection(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedOneSelection(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNotUsingDefaults(CCmdUI* pCmdUI);
  afx_msg void OnUpdateHaveDefaults(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedXMLClipboard(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CPrefsP17 dialog

class CPrefsP17 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP17)

// Construction
public:
	CPrefsP17();
	~CPrefsP17();

// Dialog Data
	//{{AFX_DATA(CPrefsP17)
	enum { IDD = IDD_PREFS_P17 };
	CButton	m_ctlRegister;
	CButton	m_ctlEditScriptWithNotepad;
	CButton	m_ctlChooseEditor;
	CColourComboBox	m_ctlTextColour;
	CColourButton	m_ctlTextSwatch2;
	CColourButton	m_ctlTextSwatch;
	CComboBox	m_ctlLanguage;
	CString	m_strWorldClose;
	CString	m_strWorldConnect;
	CString	m_strWorldDisconnect;
	CString	m_strScriptFilename;
	BOOL	m_bEnableScripts;
	int		m_iNoteTextColour;
	CString	m_strScriptPrefix;
	CString	m_strWorldOpen;
	CString	m_strScriptEditor;
	int		m_iReloadOption;
	CString	m_strWorldGetFocus;
	CString	m_strWorldLoseFocus;
	BOOL	m_bEditScriptWithNotepad;
	CString	m_strExecutionTime;
	CString	m_strWorldSave;
	BOOL	m_bWarnIfScriptingInactive;
	CString	m_strIsActive;
	BOOL	m_bScriptErrorsToOutputWindow;
	//}}AFX_DATA

  CMUSHclientDoc * m_doc;
  int m_page_number;

  CString m_strLanguage;

  CPrefsP3 * m_prefsp3;

	CString	m_strOnMXP_Start;
	CString	m_strOnMXP_Stop;
	CString	m_strOnMXP_OpenTag;
	CString	m_strOnMXP_CloseTag;
	CString	m_strOnMXP_SetVariable;
	CString	m_strOnMXP_Error;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP17)
	public:
	virtual BOOL OnSetActive();
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsP17)
	afx_msg void OnBrowse();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeTextColour();
	afx_msg void OnRegister();
	afx_msg void OnEditScript();
	afx_msg void OnChooseEditor();
	afx_msg void OnUseNotepad();
	afx_msg void OnMxpScripts();
	afx_msg void OnNewScript();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateNeedScriptFile(CCmdUI* pCmdUI);
  afx_msg void OnUpdateEnableRegisterButton(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

  void FixSwatch (CColourButton & m_ctlSwatch,
                  CColourButton & m_ctlSwatch2, 
                  int iNewColour);

  BOOL ScriptBrowser (BOOL bLoad);

};
/////////////////////////////////////////////////////////////////////////////
// CPrefsP18 dialog

class CPrefsP18 : public CGenPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP18)

// Construction
public:
	CPrefsP18();
	~CPrefsP18();

// Dialog Data
	//{{AFX_DATA(CPrefsP18)
	enum { IDD = IDD_PREFS_P18 };
	CStatic	m_ctlSummary;
	CButton	m_ctlFilter;
	CListCtrl	m_ctlVariableList;
	//}}AFX_DATA

  enum 
    { eColumnName,
      eColumnContents,
      eColumnCount      // this must be last!
    };

  // ================== start of virtual functions =======================

  // dialog management - initialise, load, unload, check if changed, get name
  virtual void InitDialog (CDialog * pDlg);          // sets up for new dialog
  virtual void LoadDialog (CDialog * pDlg,             // move from item to dialog
                           CObject * pItem) ; 
  virtual void UnloadDialog (CDialog * pDlg,             // move from dialog to item
    CObject * pItem); 
  virtual bool CheckIfChanged (CDialog * pDlg,           // has the user changed anything?
                               CObject * pItem) const;
  virtual CString GetObjectName (CDialog * pDlg) const;    // gets name from dialog

  // object setup - create a new one, set its modification number, set its dispatch ID
  virtual CObject * MakeNewObject (void);            // create new item, e.g. trigger
  virtual void SetModificationNumber (CObject * pItem, 
                                      __int64 m_nUpdateNumber);   // set modification number
  virtual void SetDispatchID (CObject * pItem, const DISPID dispid);         // set script dispatch ID
  virtual void SetInternalName (CObject * pItem, const CString strName);    // name after creation

  // get info about the object
  virtual __int64 GetModificationNumber (CObject * pItem) const;   // get modification number
  virtual CString GetScriptName (CObject * pItem) const;    // get script subroutine name
  virtual CString GetLabel (CObject * pItem) const;    // get label name

  // list management - add the item to the list control
  virtual int AddItem (CObject * pItem,                  // add one item to the list control
                       const int nItemNumber,
                       const BOOL nItem);

  virtual bool CheckIfIncluded (CObject * pItem) { return false; }
  virtual bool CheckIfExecuting (CObject * pItem) { return false; }

  virtual bool GetFilterFlag ();            // is filtering enabled?
  virtual CString GetFilterScript ();       // get the filter script

  // ================== end of virtual functions =======================

  static int CompareObjects     (const int iColumn, 
                                 const CObject * item1, 
                                 const CObject * item2);

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP18)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:


  // Generated message map functions
	//{{AFX_MSG(CPrefsP18)
	afx_msg void OnAddVariable();
	afx_msg void OnChangeVariable();
	afx_msg void OnDeleteVariable();
	virtual BOOL OnInitDialog();
	afx_msg void OnColumnclickVariablesList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFind();
	afx_msg void OnFindNext();
	afx_msg void OnDblclkVariablesList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLoadVariables();
	afx_msg void OnSaveVariables();
	afx_msg void OnCopy();
	afx_msg void OnPaste();
	afx_msg void OnFilter();
	afx_msg void OnEditFilter();
	//}}AFX_MSG
  afx_msg void OnUpdateNeedXMLClipboard(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedOneSelection(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CPrefsP19 dialog

class CPrefsP19 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP19)

// Construction
public:
	CPrefsP19();
	~CPrefsP19();

// Dialog Data
	//{{AFX_DATA(CPrefsP19)
	enum { IDD = IDD_PREFS_P19 };
	CString	m_strAutoSayString;
	BOOL	m_bEnableAutoSay;
	BOOL	m_bExcludeMacros;
	BOOL	m_bExcludeNonAlpha;
	CString	m_strOverridePrefix;
	BOOL	m_bReEvaluateAutoSay;
	//}}AFX_DATA

  CMUSHclientDoc * m_doc;
  int m_page_number;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP19)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsP19)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CPrefsP20 dialog

class CPrefsP20 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP20)

// Construction
public:
	CPrefsP20();
	~CPrefsP20();

// Dialog Data
	//{{AFX_DATA(CPrefsP20)
	enum { IDD = IDD_PREFS_P20 };
	BOOL	m_bBold0;
	BOOL	m_bBold1;
	BOOL	m_bBold10;
	BOOL	m_bBold11;
	BOOL	m_bBold12;
	BOOL	m_bBold13;
	BOOL	m_bBold14;
	BOOL	m_bBold15;
	BOOL	m_bBold2;
	BOOL	m_bBold3;
	BOOL	m_bBold4;
	BOOL	m_bBold5;
	BOOL	m_bBold6;
	BOOL	m_bBold7;
	BOOL	m_bBold8;
	BOOL	m_bBold9;
	BOOL	m_bItalic0;
	BOOL	m_bItalic1;
	BOOL	m_bItalic10;
	BOOL	m_bItalic11;
	BOOL	m_bItalic12;
	BOOL	m_bItalic13;
	BOOL	m_bItalic14;
	BOOL	m_bItalic15;
	BOOL	m_bItalic2;
	BOOL	m_bItalic3;
	BOOL	m_bItalic4;
	BOOL	m_bItalic5;
	BOOL	m_bItalic6;
	BOOL	m_bItalic7;
	BOOL	m_bItalic8;
	BOOL	m_bItalic9;
	BOOL	m_bUnderline0;
	BOOL	m_bUnderline1;
	BOOL	m_bUnderline10;
	BOOL	m_bUnderline11;
	BOOL	m_bUnderline12;
	BOOL	m_bUnderline13;
	BOOL	m_bUnderline14;
	BOOL	m_bUnderline15;
	BOOL	m_bUnderline2;
	BOOL	m_bUnderline3;
	BOOL	m_bUnderline4;
	BOOL	m_bUnderline5;
	BOOL	m_bUnderline6;
	BOOL	m_bUnderline7;
	BOOL	m_bUnderline8;
	BOOL	m_bUnderline9;
	//}}AFX_DATA

  CMUSHclientDoc * m_doc;
  int m_page_number;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP20)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsP20)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CPrefsP21 dialog

class CPrefsP21 : public CPropertyPage
{
// Construction
public:
	CPrefsP21();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsP21)
	enum { IDD = IDD_PREFS_P21 };
	CEdit	m_ctlConnectText;
	CStatic	m_ctlLineCount;
	CString	m_name;
	CString	m_connect_text;
	CString	m_password;
	int		m_connect_now;
	//}}AFX_DATA

  CMUSHclientDoc * m_doc;
  int m_page_number;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP21)
  public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsP21)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateLineCount(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CPrefsP0 dialog

class CPrefsP0 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP0)

// Construction
public:
	CPrefsP0();
	~CPrefsP0();


  CMUSHclientDoc * m_doc;
  int m_page_number;

// Dialog Data
	//{{AFX_DATA(CPrefsP0)
	enum { IDD = IDD_PREFS_P0 };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP0)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsP0)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CPrefsP22 dialog

class CPrefsP22 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP22)

// Construction
public:
	CPrefsP22();
	~CPrefsP22();

// Dialog Data
	//{{AFX_DATA(CPrefsP22)
	enum { IDD = IDD_PREFS_P22 };
	CColourButton	m_ctlHyperlinkSwatch;
	int		m_iUseMXP;
	CString	m_strMXPactive;
	int		m_iMXPdebugLevel;
	BOOL	m_bPueblo;
	BOOL	m_bEchoHyperlinkInOutputWindow;
	BOOL	m_bHyperlinkAddsToCommandHistory;
	BOOL	m_bUseCustomLinkColour;
	BOOL	m_bMudCanChangeLinkColour;
	BOOL	m_bUnderlineHyperlinks;
	BOOL	m_bMudCanRemoveUnderline;
	BOOL	m_bIgnoreMXPcolourChanges;
	BOOL	m_bSendMXP_AFK_Response;
	BOOL	m_bMudCanChangeOptions;
	//}}AFX_DATA

  CMUSHclientDoc * m_doc;
  int m_page_number;

  COLORREF m_iHyperlinkColour;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP22)
	public:
	virtual BOOL OnSetActive();
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsP22)
	virtual BOOL OnInitDialog();
	afx_msg void OnHyperlinkSwatch();
	afx_msg void OnResetTags();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
/////////////////////////////////////////////////////////////////////////////
// CPrefsP23 dialog

class CPrefsP23 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrefsP23)

// Construction
public:
	CPrefsP23();
	~CPrefsP23();

// Dialog Data
	//{{AFX_DATA(CPrefsP23)
	enum { IDD = IDD_PREFS_P23 };
	CColourButton	m_ctlTextSwatch;
	CColourButton	m_ctlBackSwatch;
	CString	m_strOurChatName;
	BOOL	m_bAutoAllowSnooping;
	BOOL	m_bAcceptIncomingChatConnections;
	long	m_IncomingChatPort;
	BOOL	m_bValidateIncomingCalls;
	BOOL	m_bIgnoreChatColours;
	CString	m_strChatMessagePrefix;
	long	m_iMaxChatLinesPerMessage;
	long	m_iMaxChatBytesPerMessage;
	CString	m_strChatSaveDirectory;
	BOOL	m_bAutoAllowFiles;
	//}}AFX_DATA

  CMUSHclientDoc * m_doc;
  int m_page_number;

  COLORREF m_iTextColour;
  COLORREF m_iBackColour;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsP23)
	public:
	virtual void OnOK();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsP23)
	afx_msg void OnChatTextSwatch();
	afx_msg void OnChatBackSwatch();
	afx_msg void OnFileDirBrowse();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
