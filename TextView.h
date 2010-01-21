#if !defined(AFX_TEXTVIEW_H__D2527D40_0F59_11D4_BB0A_0080AD7972EF__INCLUDED_)
#define AFX_TEXTVIEW_H__D2527D40_0F59_11D4_BB0A_0080AD7972EF__INCLUDED_

/*
Copyright (C) 2000 Nick Gammon.

*/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextView view

class CTextView : public CEditView
{
protected:
	CTextView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTextView)

  lua_State           * L;                  // Lua state

  CBrush * m_backbr;
  COLORREF m_backcolour;

// Attributes
public:

  CFont * m_font;
  bool m_bInsertMode;

// Operations
public:

  bool GetSelection (CString & strSelection);
  bool SetText(const char * sText);
  void SetTheFont (void);

  void ReplaceAndReselect (const bool bAll, const CString & strText);

// Overrides

  virtual void SerializeRaw(CArchive& ar);

  void OpenLuaDelayed ();
  void OpenLua ();
  bool ReplaceBlock (const char * source,
                     const int sourcelen,
                     CString & result,
                     int & count);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CTextView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CTextView)
	afx_msg void OnEditGoto();
	afx_msg void OnEscape();
	afx_msg void OnViewMaximize();
	afx_msg void OnUpdateViewMaximize(CCmdUI* pCmdUI);
	afx_msg void OnConvertUnixtodos();
	afx_msg void OnConvertDostounix();
	afx_msg void OnConvertMactodos();
	afx_msg void OnConvertDostomac();
	afx_msg void OnConvertRemoveendoflines();
	afx_msg void OnConvertQuotelines();
	afx_msg void OnEditInsertdatetime();
	afx_msg void OnEditWordcount();
	afx_msg void OnConvertUppercase();
	afx_msg void OnConvertLowercase();
	afx_msg void OnConvertRemoveextrablanks();
	afx_msg void OnEditSpellcheck();
	afx_msg void OnUpdateEditSpellcheck(CCmdUI* pCmdUI);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnEditGotomatchingbrace();
	afx_msg void OnEditSelecttomatchingbrace();
	afx_msg void OnEditSendtoworld();
	afx_msg void OnUpdateEditSendtoworld(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatuslineTime(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatuslineLines(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatuslineFreeze(CCmdUI* pCmdUI);
	afx_msg void OnEditRefreshrecalleddata();
	afx_msg void OnUpdateEditRefreshrecalleddata(CCmdUI* pCmdUI);
	afx_msg void OnEditSendtocommandwindow();
	afx_msg void OnUpdateEditSendtocommandwindow(CCmdUI* pCmdUI);
	afx_msg void OnEditFliptonotepad();
	afx_msg void OnUpdateEditFliptonotepad(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatuslineLog(CCmdUI* pCmdUI);
	afx_msg void OnEditInsert();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEditAsciiart();
	afx_msg void OnConvertQuoteforumcodes();
	afx_msg void OnConvertBase64encode();
	afx_msg void OnConvertBase64decode();
	afx_msg void OnConvertConverthtmlspecial();
	afx_msg void OnUnconvertConverthtmlspecial();
	afx_msg void OnEditSendtoimmediate();
	afx_msg void OnUpdateEditSendtoimmediate(CCmdUI* pCmdUI);
	afx_msg void OnGameFunctionslist();
	afx_msg void OnSearchGlobalreplace();
	afx_msg void OnCompleteFunction();
	afx_msg void OnUpdateCompleteFunction(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTVIEW_H__D2527D40_0F59_11D4_BB0A_0080AD7972EF__INCLUDED_)
