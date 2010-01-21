// mainfrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MyStatusBar.h"
#include "MDITabs.h"

class CMUSHclientDoc;

#define TRAY_FIRST_MENU 11000
#define TRAY_MENU_COUNT 50   // number of world menu items we support

#define ACCELERATOR_FIRST_COMMAND 12000
#define ACCELERATOR_COUNT 1000   // number of accelerator items we support

class CMyToolBar : public CToolBar
{
public:
	void DrawGripper(CDC* pDC, const CRect& rect);
	void EraseNonClient();
	virtual void DoPaint(CDC* pDC);

protected:

	//{{AFX_MSG(CMyToolBar)
	afx_msg void OnNcPaint();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};


class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

	BOOL OnDynamicTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);

// Attributes
public:

   int m_nPauseItem;

   UINT m_wDeviceID;

   bool m_bFullScreen;      // are we in full screen mode?
   WINDOWPLACEMENT m_wpPrev;  // where was the window before full screen mode?
   BOOL m_bMaximized;       // were MDI child windows maximized?

   bool m_bFlashingWindow;  // true if we flashed the window

   NOTIFYICONDATA m_niData; 

   CMDITabs m_wndMDITabs;   // for tabs

   CTime m_timeFlashed;     // when we flashed the icon last
   int m_iTabsCount;

// Operations
public:

   void SetStatusMessage(const CString& msg);
   void SetStatusMessageNow(const CString& msg);
   void SetStatusNormal (void);
   void ReturnStatusRect (CRect & rect);
   void ReturnToolbarRect(CRect & rect);
   void DelayDebugStatus(const CString& msg);

   void FixUpTitleBar (void);
   void OnUpdateBtnWorlds (int iWorld, CCmdUI* pCmdUI); 
   void CancelSound (void);
   bool PlaySoundFile (const CString & strSound);
   void ShowMCIError (const DWORD dwCode, const CString & strSound);
   bool IsFullScreen () { return m_bFullScreen; };
   void AddTrayIcon (void);
   void LeftTrayClick (void);
   void RightTrayClick (void);

   void OpenAndConnectToWorldsInStartupList (const bool bConnect);

   virtual void OnUpdateFrameMenu(HMENU hMenuAlt);
   virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

   void DoFileOpen (void);
   void DoFixMenus(CCmdUI* pCmdUI);

   void DockControlBarNextTo(CControlBar* pBar,
                             CControlBar* pTargetBar);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:

	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CMyStatusBar  m_wndStatusBar;

  UINT m_timer;
  UINT m_ticktimer;

	CMyToolBar    m_wndToolBar;
	CMyToolBar    m_wndGameToolBar;
	CMyToolBar    m_wndActivityToolBar;

  CDialogBar  m_wndInfoBar;

  CHARFORMAT  m_defaultInfoBarFormat;

protected:  // control bar embedded members

  bool        m_bActive;  // is app active or not?

// Generated message map functions
protected:
	afx_msg void OnContextMenu(CWnd*, CPoint point);
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnDisplayActivitylist();
	afx_msg BOOL OnWorldSwitch(UINT nIDC);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnUpdateStatuslineFreeze(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatuslineMushname(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatuslineLines(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatuslineLog(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatuslineTime(CCmdUI* pCmdUI);
	afx_msg void OnHelpTipoftheday();
	afx_msg void OnUpdateDisplayActivitylist(CCmdUI* pCmdUI);
	afx_msg void OnFilePreferences();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnConnectionAutoconnect();
	afx_msg void OnUpdateConnectionAutoconnect(CCmdUI* pCmdUI);
	afx_msg void OnConnectionReconnectondisconnect();
	afx_msg void OnUpdateConnectionReconnectondisconnect(CCmdUI* pCmdUI);
	afx_msg void OnHelpContents();
	afx_msg void OnUpdateBtnWorldsWorld0(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBtnWorldsWorld1(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBtnWorldsWorld2(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBtnWorldsWorld3(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBtnWorldsWorld4(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBtnWorldsWorld5(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBtnWorldsWorld6(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBtnWorldsWorld7(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBtnWorldsWorld8(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBtnWorldsWorld9(CCmdUI* pCmdUI);
	afx_msg void OnWebPage();
	afx_msg void OnViewResetToolbars();
	afx_msg void OnHelpMudlists();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnDisplayStopsoundplaying();
	afx_msg void OnUpdateDisplayStopsoundplaying(CCmdUI* pCmdUI);
	afx_msg void OnHelpBugreportsuggestion();
	afx_msg void OnViewAlwaysontop();
	afx_msg void OnUpdateViewAlwaysontop(CCmdUI* pCmdUI);
	afx_msg void OnGameSendtoallworlds();
	afx_msg void OnEditReloadnamesfile();
	afx_msg void OnViewFullscreenmode();
	afx_msg void OnUpdateViewFullscreenmode(CCmdUI* pCmdUI);
	afx_msg void OnEditNotesworkarea();
	afx_msg void OnWindowCloseallnotepadwindows();
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenworldsinstartuplist();
	afx_msg void OnUpdateFileOpenworldsinstartuplist(CCmdUI* pCmdUI);
	afx_msg void OnFileWinsock();
	afx_msg void OnConnectionConnecttoallopenworlds();
	afx_msg void OnConnectionConnecttoworldsinstartuplist();
	afx_msg void OnUpdateConnectionConnecttoworldsinstartuplist(CCmdUI* pCmdUI);
	afx_msg void OnHelpForum();
	afx_msg void OnHelpFunctionswebpage();
	afx_msg void OnHelpDocumentationwebpage();
	afx_msg void OnHelpRegularexpressionswebpage();
	afx_msg void OnHelpPluginslist();
	afx_msg void OnEditGenerateuniqueid();
	afx_msg void OnGameFunctionslist();
	afx_msg void OnEditConvertclipboardforumcodes();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	//}}AFX_MSG
  afx_msg LRESULT OnMCINotify(WPARAM, LPARAM);
	afx_msg void OnF1Test();
	afx_msg void OnUpdateInfoBar(CCmdUI* pCmdUI);
  afx_msg void OnTrayMenu(UINT nID);    
	afx_msg void OnFixMenus(CCmdUI* pCmdUI);
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

CMUSHclientDoc * FindWorld (const UINT nIDC);

