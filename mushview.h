// MUSHview.h : interface of the CMUSHView class
//
/////////////////////////////////////////////////////////////////////////////

#define PALETTE 0x02000000    // makes RGB calls palletized (See PALETTERGB)

#define MXP_FIRST_MENU 10000
#define MXP_MENU_COUNT 100   // number of MXP menu items we support

class CMUSHView : public CView
{

  friend class CSendView;

// statuses for "pause" status line entry

  public:

  enum { ePauseUninitialised, eNotPaused, ePaused, eMore, eClosed };

  protected: // create from serialization only
	CMUSHView();
	DECLARE_DYNCREATE(CMUSHView)

// Attributes
public:
	CMUSHclientDoc* GetDocument();

  int m_DefaultWidth,
      m_DefaultHeight;

  int m_selstart_line,   // start of current selection
      m_selstart_col,
      m_selend_line,     // end of current selection
      m_selend_col,
      m_pin_line,   // this is where they started a selection (with the initial mouse down)
      m_pin_col;

  int m_iPauseStatus;

  BOOL m_freeze;

  bool m_bAtBufferEnd;    // true if we should be showing the last line

  CPoint m_last_mouse_position;

  CSendView * m_bottomview;
  CChildFrame * m_owner_frame;

  int m_last_line_drawn;

  long m_last_line;

 	CToolTipCtrl m_ToolTip;
  long m_nLastToolTipLine;
  long m_nLastToolTipColumn;

  CPoint m_scroll_position;
  CPoint m_scroll_limit;

  string m_sPreviousMiniWindow;

  BOOL m_mousedover;

// Operations
public:

void sizewindow (void);

void addedstuff (void);

int mouse_still_down (void);

bool get_selection (CRgn & rgn);    // true if empty region

void display_text (CDC* pDC, 
                   const CMUSHclientDoc* pDoc, 
                   const long line,
                   const CLine * pLine, 
                   const int col, 
                   const int len, 
                   const BOOL selected,
                   long & pixel,
                   const bool bBackground);


void Blit_Bitmap (CDC* pDestDC, CDC* pSrcDC, 
                  const long iWidth, const long iHeight, 
                  const short iMode,
                  CRect & rect,
                  const bool bUseRect = false,
                  const bool bTransparent = false,
                  const COLORREF TransparentColour = 0);

void Send_Mouse_Event_To_Plugin (DISPID iDispatchID,
                                 const string m_sPluginID, 
                                 const string sRoutineName, 
                                 const string HotspotId,
                                 long Flags = 0,
                                 bool dont_modify_flags = false);

CMiniWindow * Mouse_Over_Miniwindow (CMUSHclientDoc* pDoc, 
                                     CPoint point, 
                                     string & sHotspotId, 
                                     CHotspot * &pHotspot, 
                                     string & sMiniwindowId);

bool Mouse_Move_MiniWindow (CMUSHclientDoc* pDoc, CPoint point);
bool Mouse_Down_MiniWindow (CMUSHclientDoc* pDoc, CPoint point, long flags);
bool Mouse_Up_MiniWindow (CMUSHclientDoc* pDoc, CPoint point, long flags);
bool Mouse_Tooltip_MiniWindow (CMUSHclientDoc* pDoc, CPoint point, NMHDR * pNMHDR);
bool Mouse_Wheel_MiniWindow (CMUSHclientDoc* pDoc, CPoint point, long delta);

void Calculate_MiniWindow_Rectangles (CMUSHclientDoc* pDoc, const bool bUnderneath);

RECT GetTextRectangle (const bool bIncludeBorder = false);
bool HaveTextRectangle ();
long GetOutputWindowHeight () ;
long GetOutputWindowWidth () ;
void GetTextRect (RECT * rect);

void did_jump (void);

void doStart (void);
void doEnd (void);
void doPageup (void);
void doPagedown (void);
void doLineup (void);
void doLinedown (void);
void FixupTitle (void);
void doBookmark (void);
void doGotoBookmark (void);
void doGotoLine (void);
void EnsureSelectionVisible (void);
void SelectLine (const int iLine);
void AliasMenu (CPoint point);

void DrawImage (CDC* pDC, CBitmap & bitmap, const short iMode);

// we add this to simulate a CScrollView
void ScrollToPosition( POINT pt, const bool bSmooth );
CPoint GetScrollPosition( ) const { return m_scroll_position; }; 
void SetScrollSizes (SIZE sizeTotal, 
                     const SIZE& sizePage, 
                     const SIZE& sizeLine);

bool calculate_line_and_column (const CPoint & point, 
                                CClientDC & dc, 
                                int & line, 
                                int & col,
                                const bool bHalfWay = true);
long calculate_width (const int & line, const int len, CMUSHclientDoc* pDoc, CClientDC & dc);
void extend_selection (const int line, const int col);
void PrintWorld (void);
void RemoveToolTip (void);
bool GetSelection(CFile & f);
bool GetSelection(CString & s);
void SendMacro (int whichone);
void SelectionChanged (void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMUSHView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMUSHView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

  void  OnUpdateStatuslineFreeze_helper(CCmdUI* pCmdUI);
	BOOL OnToolTipNeedText(UINT id, NMHDR * pNMHDR, LRESULT * pResult);

protected:

// Generated message map functions
protected:
	afx_msg void OnContextMenu(CWnd*, CPoint point);
	//{{AFX_MSG(CMUSHView)
	afx_msg void OnTestEnd();
	afx_msg void OnTestPagedown();
	afx_msg void OnTestPageup();
	afx_msg void OnTestStart();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTestLinedown();
	afx_msg void OnTestLineup();
	afx_msg void OnEditCopy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeysActivatecommandview();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDisplayFreezeoutput();
	afx_msg void OnUpdateDisplayFreezeoutput(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatuslineFreeze(CCmdUI* pCmdUI);
	afx_msg void OnWindowMinimize();
	afx_msg void OnUpdateWindowMinimize(CCmdUI* pCmdUI);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDisplayFind();
	afx_msg void OnDisplayFindagain();
	afx_msg void OnFilePrintWorld();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnFileSaveselection();
	afx_msg void OnFindAgainBackwards();
	afx_msg void OnUpdateFindAgainBackwards(CCmdUI* pCmdUI);
	afx_msg void OnFindAgainForwards();
	afx_msg void OnUpdateFindAgainForwards(CCmdUI* pCmdUI);
	afx_msg void OnDisplayGotourl();
	afx_msg void OnUpdateNeedSelection(CCmdUI* pCmdUI);
	afx_msg void OnDisplaySendmailto();
	afx_msg void OnEditSelectAll();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnDisplayGotobookmark();
	afx_msg void OnDisplayBookmarkselection();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnAltUparrow();
	afx_msg void OnAltDownarrow();
	afx_msg void OnDisplayTextattributes();
	afx_msg void OnEditNotesworkarea();
	afx_msg void OnDisplayRecalltext();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDisplayGotoline();
	afx_msg void OnGameCommandhistory();
	afx_msg void OnFileCtrlN();
	afx_msg void OnFileCtrlP();
	afx_msg void OnEditCtrlZ();
	afx_msg void OnEditCopyashtml();
	afx_msg void OnDisplayHighlightphrase();
	afx_msg void OnDisplayMultilinetrigger();
	afx_msg void OnWindowMaximize();
	afx_msg void OnWindowRestore();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg BOOL OnMacroCommand(UINT nIDC);
	afx_msg BOOL OnKeypadCommand(UINT nIDC);
  afx_msg void OnMXPMenu(UINT nID);    
  afx_msg void OnAcceleratorCommand(UINT nID);    
	afx_msg void OnUpdateNeedSelectionForCopy(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in view.cpp
inline CMUSHclientDoc* CMUSHView::GetDocument()
   { return (CMUSHclientDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////


//////////////////
// Class to save and restore the device context
//

class CDCExcursion {
protected:
   int savedcontext;
   HDC savedhandle;
public:
   CDCExcursion(const HDC hdc)   { savedcontext = ::SaveDC (hdc); savedhandle = hdc; }
   ~CDCExcursion()  { ::RestoreDC(savedhandle, savedcontext); }
};

