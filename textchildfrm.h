// Textchildfrm.h : interface of the CTextChildFrame class
//
/////////////////////////////////////////////////////////////////////////////

class CTextChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CTextChildFrame)
public:
	CTextChildFrame();

  CTextDocument * m_pDoc;

// Attributes
protected:
  
// Operations
public:

  virtual void OnUpdateFrameMenu(BOOL bActivate, CWnd* pActivateWnd, HMENU hMenuAlt);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextChildFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTextChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CTextChildFrame)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
