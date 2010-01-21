#if !defined(AFX_TEXTDOCUMENT_H__D2527D3F_0F59_11D4_BB0A_0080AD7972EF__INCLUDED_)
#define AFX_TEXTDOCUMENT_H__D2527D3F_0F59_11D4_BB0A_0080AD7972EF__INCLUDED_

/*
Copyright (C) 2000 Nick Gammon.

*/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextDocument.h : header file
//

// enums for  m_iSaveOnChange
enum 
  {
  eNotepadSaveDefault,     // 0
  eNotepadSaveAlways,      // 1
  eNotepadSaveNever,       // 2
  };

/////////////////////////////////////////////////////////////////////////////
// CTextDocument document

class CTextDocument : public CDocument
{
protected:
	CTextDocument();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTextDocument)

// Attributes
public:

	HANDLE	m_pThread;			// Notification thread
	CEvent  m_eventFileChanged;		// file changed thread event
  bool m_bInFileChanged;
  CTime m_timeFileMod;

  CString m_strTitle;
  CMUSHclientDoc * m_pRelatedWorld; // which world it belongs to, if any

   __int64 m_iUniqueDocumentNumber;    // to confirm we have the right doc
   CString m_strFontName;        // window font
   LONG     m_iFontSize;
   LONG     m_iFontWeight;
   DWORD    m_iFontCharset;
   bool     m_bFontItalic;
   bool     m_bFontUnderline;
   bool     m_bFontStrikeout;
   COLORREF m_textColour;        // window colour
   COLORREF m_backColour;
   CString  m_strSearchString;   // if recall, what did they recall?
   CString  m_strRecallLinePreamble;   // what line preamble did they want?
   bool m_bMatchCase;
   bool m_bRegexp;          // and other params
   bool m_bCommands;
   bool m_bOutput;
   bool m_bNotes;
   int  m_iSaveOnChange;    // whether to save if contents changed, see enum above
   int  m_iLines;
   int  m_iNotepadType;    // "type" of notepad window - see enum in doc.h

   BOOL m_bReadOnly;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextDocument)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	virtual void OnCloseDocument();
	virtual BOOL SaveModified();
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTextDocument();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


  CMUSHclientDoc * FindWorld (void);
  void OnFileChanged(void);
  virtual BOOL DoSave (LPCTSTR lpszPathName, BOOL bReplace = TRUE);
  void CreateMonitoringThread(const char * sName);
  void SetTheFont (void);
  void SetReadOnly (BOOL bReadOnly);

	// Generated message map functions
protected:
	//{{AFX_MSG(CTextDocument)
	afx_msg void OnFileOpen();
	afx_msg void OnUpdateStatuslineMushname(CCmdUI* pCmdUI);
	afx_msg void OnFileNew();
	afx_msg void OnEditConvertclipboardforumcodes();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTDOCUMENT_H__D2527D3F_0F59_11D4_BB0A_0080AD7972EF__INCLUDED_)
