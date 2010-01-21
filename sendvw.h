// sendvw.h : header file
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1994 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.


// for iHistoryStatus
enum  {
    eAtTop,
    eInMiddle,
    eAtBottom,
  };

class CSendView : public CEditView
{
protected:
	CSendView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSendView)


  friend class CMUSHView;

// Attributes
public:
	BOOL m_bSendRandomMessages;
	CMUSHclientDoc* GetDocument();
	CStringList m_msgList;
  CString m_last_command;
  long m_inputcount;
  POSITION m_HistoryPosition;
  int m_iHistoryStatus;   // see enum above
  CMUSHView * m_topview;
  CChildFrame * m_owner_frame;

  CString m_strChangeFrom;  
  CString m_strChangeTo;

  CBrush * m_backbr;   // background colour brush
  COLORREF m_backcolour;  // current background colour


  bool m_bChanged;    // the user has typed something
  
  CString m_strPartialCommand;  // for Alt+UpArrow

// stuff for finding in the input buffer

  CFindInfo m_HistoryFindInfo;

// Operations
public:

  void SetCommand (LPCTSTR str);
  void SendCommand (const CString strCommand, 
                    const BOOL bSavePrevious, 
                    const BOOL bKeepInHistory = TRUE);
  void SendMacro (int whichone);
  bool CheckTyping (CMUSHclientDoc* pDoc, CString strReplacement);
  void DoFind (bool bAgain);
  bool TabCompleteOneLine (CMUSHclientDoc* pDoc,
                           const int nStartChar,
                           const int nEndChar,
                           const CString & strWord, 
                           const CString & strLine);
  void DoCommandHistory();
  void DoPreviousCommand ();
  void DoNextCommand ();

  void NotifyPluginCommandChanged ();

  void AddToCommandHistory (const CString & strCommand);

  void AdjustCommandWindowSize (void);
  void CheckForSelectionChange (void);
  void CancelSelection (void);
  
  // Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendView)
	public:
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSendView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// for finding

  static void InitiateSearch (const CObject * pObject,
                              CFindInfo & FindInfo);
 
  static bool GetNextLine (const CObject * pObject,
                           CFindInfo & FindInfo, 
                           CString & strLine);

	// Generated message map functions
protected:
	afx_msg void OnContextMenu(CWnd*, CPoint point);
	//{{AFX_MSG(CSendView)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeysNextcommand();
	afx_msg void OnKeysPrevcommand();
	afx_msg void OnTestEnd();
	afx_msg void OnTestPagedown();
	afx_msg void OnTestPageup();
	afx_msg void OnTestStart();
	afx_msg void OnTestLinedown();
	afx_msg void OnTestLineup();
	afx_msg void OnGameDown();
	afx_msg void OnGameEast();
	afx_msg void OnGameExamine();
	afx_msg void OnGameLook();
	afx_msg void OnGameNorth();
	afx_msg void OnGameSouth();
	afx_msg void OnGameUp();
	afx_msg void OnGameWest();
	afx_msg void OnGameSay();
	afx_msg void OnGameWhisper();
	afx_msg void OnGamePagesomeone();
	afx_msg void OnGameCommandhistory();
	afx_msg void OnGameDoing();
	afx_msg void OnGameWho();
	afx_msg void OnGameDrop();
	afx_msg void OnGameTake();
	afx_msg void OnDisplayFreezeoutput();
	afx_msg void OnUpdateDisplayFreezeoutput(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatuslineFreeze(CCmdUI* pCmdUI);
	afx_msg void OnActionsLogout();
	afx_msg void OnActionsQuit();
	afx_msg BOOL OnKeypadCommand(UINT nIDC);
	afx_msg BOOL OnMacroCommand(UINT nIDC);
	afx_msg void OnWindowMinimize();
	afx_msg void OnUpdateWindowMinimize(CCmdUI* pCmdUI);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnChange();
	afx_msg void OnUpdateSessionOpen(CCmdUI* pCmdUI);
	afx_msg void OnDisplayFind();
	afx_msg void OnDisplayFindagain();
	afx_msg void OnFilePrintWorld();
	afx_msg void OnDestroy();
	afx_msg void OnFindAgainBackwards();
	afx_msg void OnUpdateFindAgainBackwards(CCmdUI* pCmdUI);
	afx_msg void OnFindAgainForwards();
	afx_msg void OnUpdateFindAgainForwards(CCmdUI* pCmdUI);
	afx_msg void OnRepeatLastCommand();
	afx_msg void OnUpdateRepeatLastCommand(CCmdUI* pCmdUI);
	afx_msg void OnAltDownarrow();
	afx_msg void OnAltUparrow();
	afx_msg void OnEditSelectAll();
	afx_msg void OnKeysTab();
	afx_msg void OnDisplayClearCommandHistory();
	afx_msg void OnUpdateDisplayClearCommandHistory(CCmdUI* pCmdUI);
	afx_msg void OnProcessEscape();
	afx_msg void OnDisplayGotobookmark();
	afx_msg void OnDisplayBookmarkselection();
	afx_msg void OnInputGlobalchange();
	afx_msg void OnUpdateInputGlobalchange(CCmdUI* pCmdUI);
	afx_msg void OnEditSpellcheck();
	afx_msg void OnUpdateEditSpellcheck(CCmdUI* pCmdUI);
	afx_msg void OnEditGeneratecharactername();
	afx_msg void OnEditNotesworkarea();
	afx_msg void OnDisplayGotoline();
	afx_msg void OnFileCtrlN();
	afx_msg void OnFileCtrlP();
	afx_msg void OnEditCtrlZ();
	afx_msg void OnRepeatLastWord();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnCommandHome();
	afx_msg void OnCommandEnd();
	afx_msg void OnEditGotomatchingbrace();
	afx_msg void OnEditSelecttomatchingbrace();
	afx_msg void OnDisplayHighlightphrase();
	afx_msg void OnDisplayMultilinetrigger();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnGameFunctionslist();
	afx_msg void OnCompleteFunction();
	afx_msg void OnUpdateCompleteFunction(CCmdUI* pCmdUI);
	afx_msg void OnWindowMaximize();
	afx_msg void OnWindowRestore();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	//}}AFX_MSG
  afx_msg void OnMXPMenu(UINT nID);    
  afx_msg void OnAcceleratorCommand(UINT nID);  
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnInitMenuPopup(CMenu* pMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateNeedSel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNeedOutputSel(CCmdUI* pCmdUI);
	afx_msg void OnEditCopyashtml();
  
	afx_msg void OnDisplayTextattributes();

	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in sendvw.cpp
inline CMUSHclientDoc* CSendView::GetDocument()
   { return (CMUSHclientDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
