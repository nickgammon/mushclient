// ActivityView.h : header file
//
#include <afxcview.h>

/////////////////////////////////////////////////////////////////////////////
// CActivityView view

class CActivityView : public CListView
{
protected:
	CActivityView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CActivityView)

// Attributes
public:

protected:

  BOOL m_bUpdateLockout;
  BOOL m_bOnTop;

  enum 
  { eColumnSeq,
    eColumnMush,
    eColumnNew,
    eColumnLines,
    eColumnStatus,
    eColumnSince,
    eColumnDuration,
    eColumnCount      // this must be last!
  };

  // for sorting the list

  int m_last_col;
  BOOL m_reverse;

  CImageList m_imglIcons;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CActivityView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CActivityView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

  CMUSHclientDoc * GetSelectedWorld (void);

  static int CALLBACK CompareFunc ( LPARAM lParam1, 
                                    LPARAM lParam2,
                                    LPARAM lParamSort);

  // Generated message map functions
protected:
	afx_msg void OnContextMenu(CWnd*, CPoint point);
	//{{AFX_MSG(CActivityView)
	afx_msg void OnDestroy();
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReturn(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPopupConfigureworld();
	afx_msg void OnPopupSwitchtoworld();
	afx_msg void OnUpdatePopupConfigureworld(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnConnectionConnect();
	afx_msg void OnConnectionDisconnect();
	afx_msg void OnUpdateNeedSelection(CCmdUI* pCmdUI);
	afx_msg void OnUpdateConnectionConnect(CCmdUI* pCmdUI);
	afx_msg void OnUpdateConnectionDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnPopupFileClose();
	afx_msg void OnPopupFileSave();
	afx_msg void OnPopupSaveworlddetailsas();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
