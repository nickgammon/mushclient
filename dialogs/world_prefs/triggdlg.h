// triggdlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTriggerDlg dialog

class CTriggerDlg : public CDialog
{
	DECLARE_DYNAMIC(CTriggerDlg)

    // Construction
public:
	CTriggerDlg(CWnd* pParent = NULL);   // standard constructor

  ~CTriggerDlg () { delete m_font; delete m_font2; };

// Dialog Data
	//{{AFX_DATA(CTriggerDlg)
	enum { IDD = IDD_EDIT_TRIGGER };
	CStatic	m_ctlVariableLabel;
	CEdit	m_ctlLinesToMatch;
	CButton	m_ctlMultiLine;
	CEdit	m_ctlVariable;
	CComboBox	m_ctlSendTo;
	CEdit	m_ctlTriggerContents;
	CEdit	m_ctlTrigger;
	CColourComboBox	m_ctlTriggerColour;
	CButton	m_ctlRegexp;
	CColourButton	m_ctlTriggerSwatch2;
	CColourButton	m_ctlTriggerSwatch;
	BOOL	m_ignore_case;
	CString	m_trigger;
	CString	m_contents;
	int		m_colour;
	CString	m_sound_pathname;
	BOOL	m_omit_from_log;
	BOOL	m_bEnabled;
	BOOL	m_bOmitFromOutput;
	BOOL	m_bKeepEvaluating;
	CString	m_strLabel;
	int		m_iClipboardArg;
	CString	m_strProcedure;
	BOOL	m_bRegexp;
	BOOL	m_bRepeat;
	short	m_iSequence;
	BOOL	m_bBold;
	BOOL	m_bItalic;
	BOOL	m_bUnderline;
	BOOL	m_bSoundIfInactive;
	int		m_iMatchTextColour;
	int		m_iMatchBackColour;
	int		m_iMatchBold;
	int		m_iMatchItalic;
	int		m_iMatchInverse;
	int		m_iSendTo;
	BOOL	m_bExpandVariables;
	BOOL	m_bTemporary;
	BOOL	m_bLowercaseWildcard;
	CString	m_strGroup;
	CString	m_strVariable;
	CString	m_strIncluded;
	BOOL	m_bMultiLine;
	short	m_iLinesToMatch;
	int		m_iColourChangeType;
	CString	m_strRegexpError;
	BOOL	m_bOneShot;
	//}}AFX_DATA

  CTriggerMap * m_pTriggerMap;
  CTrigger * m_current_trigger;

  // copied from page14
	BOOL	m_bShowBold;
	BOOL	m_bShowItalic;
	BOOL	m_bShowUnderline;

  bool  m_bUTF_8;
  bool  m_bLua;

  CFont * m_font;
  CFont * m_font2;

  CMUSHclientDoc * m_pDoc;

  // custom (user-defined) colours for triggers etc.
  COLORREF m_customtext [MAX_CUSTOM];
  COLORREF m_customback [MAX_CUSTOM];
  CString m_strCustomName [MAX_CUSTOM];
  COLORREF m_iOtherForeground;
  COLORREF m_iOtherBackground;


  void FixSwatch (CColourButton & m_ctlSwatch, 
                  CColourButton & m_ctlSwatch2,
                  int iNewColour);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTriggerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTriggerDlg)
	afx_msg void OnBrowseSounds();
	afx_msg void OnNoSound();
	afx_msg void OnTestSound();
	afx_msg void OnHelpbutton();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeTriggerColour();
	afx_msg void OnEdit();
	afx_msg void OnConvertToRegexp();
	afx_msg void OnEditSend();
	afx_msg void OnSelchangeSendTo();
	afx_msg void OnTriggerSwatch();
	afx_msg void OnTriggerSwatch2();
	afx_msg void OnMultiLine();
	//}}AFX_MSG
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
  afx_msg void OnUpdateOK(CCmdUI* pCmdUI);
  afx_msg void OnUpdateInvocationCount(CCmdUI* pCmdUI);
  afx_msg void OnUpdateMatchCount(CCmdUI* pCmdUI);
  afx_msg void OnUpdateTimeTaken(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedSound(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedRegexp(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedRegexpButNotMultiline(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedMultiline(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedNoMultiline(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedNoRegexp(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedBold(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedItalic(CCmdUI* pCmdUI);
  afx_msg void OnUpdateNeedUnderline(CCmdUI* pCmdUI);
  afx_msg void OnUpdateCheckComboColour(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
