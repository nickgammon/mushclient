#ifdef PANE

#if !defined(AFX_PANEVIEW_H__9568B17A_AC73_11D8_9939_00008C012785__INCLUDED_)
#define AFX_PANEVIEW_H__9568B17A_AC73_11D8_9939_00008C012785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PaneView.h : header file
//

#include "paneline.h"

#define PANE_SHOWBOLD       0x0001
#define PANE_SHOWITALIC     0x0002
#define PANE_SHOWUNDERLINE  0x0004

/////////////////////////////////////////////////////////////////////////////
// CPaneView view

class CPaneView : public CView
{
protected:
	CPaneView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPaneView)

// Attributes
public:

  string  m_sName;    // pane name
  string  m_sTitle;   // window title

  long    m_iLeft;  
  long    m_iTop;  
  long    m_iWidth;
  long    m_iHeight;
  long    m_iFlags;      // see above

  bool m_bShowBold;
  bool m_bShowItalic;   
  bool m_bShowUnderline;

  COLORREF m_cText;
  COLORREF m_cBack;

  CString m_strFont;    // font name

  int m_nHeight;        // font size in points

  CFont * m_font [8];   // all 8 possible combinations of bold, italic etc.
  deque <CPaneLine *> m_lines;        // scrollback buffer

  unsigned int m_iLineHeight;      // vertical size of each line
  unsigned int m_iFontWidth;       // average character width
  unsigned int m_iMaxLines;        // max lines in m_lines
  unsigned int m_iMaxWidth;        // max width of output window
  bool         m_bUTF8;            // true if we are using UTF-8 here


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPaneView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPaneView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:

  CPaneLine * GetLastLine (void)
    {
    if (m_lines.empty ())
      return NULL;
    else
      return m_lines.back ();
    };  // end of GetLastLine

  void MarkLastLineNewLine (void)
    {
    if (!m_lines.empty ())
      GetLastLine ()->SetNewline ();
    };
  
  int GetLineHeight (void) { return m_iLineHeight; };
  int GetMaxLines (void) { return m_iMaxLines; };
  int GetMaxWidth (void) { return m_iMaxWidth; };


  CPaneLine * AddLine ();

  void AddText (const string sText,         // what to add
                const COLORREF & cText,     // text colour
                const COLORREF & cBack,     // background colour
                const int iStyle,           // style (eg. italic)
                const bool bNewLine = false); // end with newline?
               
  void Clear ();

	// Generated message map functions
	//{{AFX_MSG(CPaneView)
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnGotoEnd();
	afx_msg void OnLinedown();
	afx_msg void OnLineup();
	afx_msg void OnPagedown();
	afx_msg void OnPageup();
	afx_msg void OnGotoStart();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PANEVIEW_H__9568B17A_AC73_11D8_9939_00008C012785__INCLUDED_)
#endif // PANE