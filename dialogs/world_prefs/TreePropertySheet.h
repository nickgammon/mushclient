#if !defined(AFX_TREEPROPERTYSHEET_H__B95F414E_0487_11D3_81D8_E47B2B99925E__INCLUDED_)
#define AFX_TREEPROPERTYSHEET_H__B95F414E_0487_11D3_81D8_E47B2B99925E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TreePropertySheet.h : Header-File
//

/////////////////////////////////////////////////////////////////////////////
// CTreePropertySheet

#include <afxtempl.h>

enum	{
	tps_above=0,
	tps_below=1,
	tps_horz_left=0,
	tps_horz_center=1,
	tps_horz_right=2,
	tps_vert_top=0,
	tps_vert_center=4,
	tps_vert_bottom=8,
	tps_horz_mask=3,
	tps_vert_mask=12,
	tps_item_branch=0,
	tps_item_node=1,
	tps_item_endbranch=2,
	tps_item_type_mask=3,
	tps_item_collapsed=0,
	tps_item_expanded=4,
	tps_capborder_none=0,
	tps_capborder_line=1,
	tps_capborder_flow=2
};

class	CMemDC	: public CDC {
public:
	CBitmap m_MemBmp,*m_pOldBmp;
	int		nWidth,nHeight;
	int		nCurWidth, nCurHeight;
public:
	CMemDC(int nWidth,int nHeight) : CDC()
	{
		CDC		*tempDC=CWnd::GetDesktopWindow()->GetDC();

		CreateCompatibleDC(tempDC);
		m_MemBmp.CreateCompatibleBitmap(tempDC,nWidth,nHeight);
		m_pOldBmp=SelectObject(&m_MemBmp);
		this->nWidth	=nWidth;
		this->nHeight	=nHeight;
		nCurWidth		=nWidth;
		nCurHeight		=nHeight;
		CWnd::GetDesktopWindow()->ReleaseDC(tempDC);

        FillSolidRect(CRect(0,0,nWidth,nHeight),RGB(192,192,192));
	}
	CMemDC(CBitmap *bmp) : CDC()
	{
		CDC		*tempDC=CWnd::GetDesktopWindow()->GetDC();

		CreateCompatibleDC(tempDC);
		m_pOldBmp	=SelectObject(bmp);

		CWnd::GetDesktopWindow()->ReleaseDC(tempDC);
	}
	~CMemDC()
	{
		SelectObject(m_pOldBmp);
	}
    static  CBitmap *CreateBitmap(int nWidth,int nHeight)
    {
        CBitmap *pBitmap;

		CDC		*tempDC=CWnd::GetDesktopWindow()->GetDC();
        pBitmap =new CBitmap();
        pBitmap->CreateCompatibleBitmap(tempDC,nWidth,nHeight);
		CWnd::GetDesktopWindow()->ReleaseDC(tempDC);
        return pBitmap;
    }
    void    FitInto(int nWidth,int nHeight)
    {
        if(this->nWidth<nWidth || this->nHeight<nHeight) {
            SelectObject(m_pOldBmp);
            m_MemBmp.DeleteObject();

            CDC		*tempDC=CWnd::GetDesktopWindow()->GetDC();
            m_MemBmp.CreateCompatibleBitmap(tempDC,nWidth,nHeight);
		    CWnd::GetDesktopWindow()->ReleaseDC(tempDC);
            SelectObject(&m_MemBmp);

            this->nWidth    =nWidth;
            this->nHeight   =nHeight;
			FillSolidRect(CRect(0,0,nWidth,nHeight),RGB(192,192,192));
        }
		nCurWidth		=nWidth;
		nCurHeight		=nHeight;
    }
};

class CTreePropertySheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CTreePropertySheet)

// Subclasses
protected:
	class	cControlPos	{
	public:
		int				row,col;
		int				nWidth,nHeight;
		int				nZOrder;

		int				nArrange;
		int				nVertArrange;

		CWnd			*pWnd;
		UINT			nID;
		int				nTWidth,nTHeight;
		CString			sCaption;

		bool			bAutoDelete;

		CRect			rcFinalPos;

		cControlPos()
		{
			row=col=0;
			nWidth=nHeight=0;
			nZOrder=0;
			nArrange=nVertArrange=0;
			pWnd=NULL;
			nID=0;
			nTWidth=nTHeight=0;
			bAutoDelete=false;
			rcFinalPos=CRect(0,0,0,0);
		};
	};

	class	cPropPage {
	public:
		int				nType;
		CPropertyPage	*pPage;
		CString			sCaption;
		CString			sCaptionDescr;
	};

	class	cRowInfo {
	public:
		int		nHeight;
		int		nExtraTop;
		int		nExtraBottom;
	};

public:
	enum	{
		ID_TREECTRL=0x1ffe
	};

// Constructor/Destructor
public:
	CTreePropertySheet(CWnd* pParentWnd, UINT iSelectPage=0);
	CTreePropertySheet(int nTreeWidth, UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CTreePropertySheet(int nTreeWidth=100, LPCTSTR pszCaption="Tree property sheet", CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:
	int					m_nSpaceTop;
	int					m_nSpaceBottom;
	int					m_nSpaceLeft;
	int					m_nSpaceRight;
	int					m_nSpaceMid;
	int					m_nCaptionHeight;
	int					m_nCaptionBorder;
	bool				m_bSpecialCaption;
	COLORREF			m_clrCaptionLeft;
	COLORREF			m_clrCaptionRight;
	COLORREF			m_clrCaptionTextLeft;
	COLORREF			m_clrCaptionTextRight;
	COLORREF			m_clrCaptionBorderTopLeft;
	COLORREF			m_clrCaptionBorderBottomRight;

  int         m_iWantedPage;  // NJG

	bool				m_bDisableStandardButtons;

	CFont				*m_pCaptionFont;
	bool				m_bDeleteCaptionFont;
	CFont				*m_pCaptionDescrFont;
	bool				m_bDeleteCaptionDescrFont;

	bool				m_bDragging;
	CPoint				m_lButtonDownPoint;

// Protected Attributes
protected:
	int					m_nTreeWidth;

	CArray<cRowInfo,cRowInfo&>
						m_anRowHeight_TopPane;

	CArray<cRowInfo,cRowInfo&>
						m_anRowHeight_BottomPane;

	CArray<cPropPage,cPropPage&>
						m_acPropPages;

	CArray<cControlPos,cControlPos&>
						m_acControlPos;

	CArray<CFont*,CFont*>
						m_apFonts;

	CTreeCtrl			m_cTreeCtrl;

	CRect				m_rcCaptionBar;
	CMemDC				m_cCaptionBarDC;
	CMemDC				m_cCaptionDC;

	bool				m_bCaptionBarInvalidated;

// Operations
public:
	void				InsertExtraControl(
							int		nVertArrange,
							int		zOrder,
							CWnd*	pWnd,
							int		row,int col,
							int		nWidth,int nHeight,
							int		nArrange,
							bool	bAutoDelete);
	void				InsertExtraSpace(
							int		nVertArrange,
							int		row,int col,
							int		nWidth=-1,
							int		nHeight=0);
/*	void				AddPage(
							const CString& sCaption,
							int nType,
							UINT nIDTemplate,
							const CString& sCaptionDescr,
							CPropertyPage *pDlg=NULL);*/
	void				AddPage(
							int nType,
							CPropertyPage* pPage,
							LPCTSTR lpszCaptionDescr=_T(""));

	void				SetExtraRowSpace(int nVertArrange,int nRow,int nSpaceTop,int nSpaceBottom);
	void				SetCaptionFont(LPCTSTR lpszFacename,int nHeight,bool bNoInvalidate=false);
	void				SetCaptionDescrFont(LPCTSTR lpszFacename,int nHeight,bool bNoInvalidate=false);

	void				SetSimple(bool bSpecialCaption);

	CFont				*GetFont(const CString& sFacename,int nHeight);

  void        PageHasChanged (CPropertyPage* pPage);  // NJG - they have changed pages

// Protected operations
protected:
	COLORREF			_interpolate(COLORREF clrLeft, COLORREF clrRight, int nStep, int nSteps);
	void				RenderCaptionBar();

	void				AlignControls();
	int					GetHeight(CArray<cRowInfo,cRowInfo&>& pane);
	int					GetWidth(int nVertArrange, int nRow);
	void				AssignFillerSpace(int nVertArrange, int nRow, int nSpace);
	int					CountFillers(int nVertArrange,int nRow);
	int					CountColumns(int nVertArrange,int nRow);
	cControlPos*		GetControl(int nVertArrange,int nRow,int nCol);

	bool				HidePpgButtons();
	void				HideTabControl();
	void				AddTreeView();
	void				AddPaneSpace(int nWidth, int nTopPane,int nBottomPane);
	void				PlaceControl(cControlPos* pCtrl);

	//{{AFX_VIRTUAL(CTreePropertySheet)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTreePropertySheet();

protected:
	//{{AFX_MSG(CTreePropertySheet)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	afx_msg	void	OnSelChanged(NMHDR* pNotifyStruct, LRESULT* pResult);
  afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}

#endif // AFX_TREEPROPERTYSHEET_H__B95F414E_0487_11D3_81D8_E47B2B99925E__INCLUDED_
