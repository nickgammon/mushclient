// activitychildfrm.h : interface of the CActivityChildFrame class
//
/////////////////////////////////////////////////////////////////////////////

class CActivityChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CActivityChildFrame)
public:
	CActivityChildFrame();

// Attributes
protected:
  
// Operations
public:

  virtual void OnUpdateFrameMenu(BOOL bActivate, CWnd* pActivateWnd, HMENU hMenuAlt);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CActivityChildFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CActivityChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CActivityChildFrame)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
