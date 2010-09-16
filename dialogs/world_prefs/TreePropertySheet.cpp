// TreePropertySheet.cpp: Implementation file
//

#include "stdafx.h"
#include "TreePropertySheet.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SPACELEFT	  5
#define SPACEMID		5
#define SPACERIGHT	5
#define SPACETOP		5
#define SPACEBOTTOM	15
#define CAPTIONHEIGHT 20

/////////////////////////////////////////////////////////////////////////////
// CTreePropertySheet

IMPLEMENT_DYNAMIC(CTreePropertySheet, CPropertySheet)

CTreePropertySheet::CTreePropertySheet(int nTreeWidth, UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
:	CPropertySheet(nIDCaption, pParentWnd, iSelectPage),
	m_cCaptionBarDC(1,1),
	m_cCaptionDC(1,1)
{
  m_iWantedPage = iSelectPage;
	m_nTreeWidth	=nTreeWidth;
	m_nSpaceLeft	=   SPACELEFT;
	m_nSpaceMid		=   SPACEMID;
	m_nSpaceRight	=   SPACERIGHT;
	m_nSpaceTop		=   SPACETOP;
	m_nSpaceBottom =  SPACEBOTTOM;
	m_nCaptionHeight= CAPTIONHEIGHT;
	m_bCaptionBarInvalidated=true;

	m_clrCaptionLeft=RGB(128,128,128);
	m_clrCaptionRight=RGB(128,128,128);

	m_clrCaptionTextLeft=RGB(255,255,255);
	m_clrCaptionTextRight=RGB(255,255,255);

	m_nCaptionBorder	=tps_capborder_line;

	m_clrCaptionBorderTopLeft=RGB(255,255,255);
	m_clrCaptionBorderBottomRight=RGB(64,64,64);

	m_pCaptionFont	=NULL;
	m_pCaptionDescrFont=NULL;

	m_bDeleteCaptionFont=false;
	m_bDeleteCaptionDescrFont=false;

	m_bSpecialCaption=false;
	m_bDragging=false;

	m_bDisableStandardButtons=false;
}

CTreePropertySheet::CTreePropertySheet(int nTreeWidth, LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
:	CPropertySheet(pszCaption, pParentWnd, iSelectPage),
	m_cCaptionBarDC(1,1),
	m_cCaptionDC(1,1)
{
  m_iWantedPage = iSelectPage;
	m_nTreeWidth	=nTreeWidth;
	m_nSpaceLeft	=   SPACELEFT;
	m_nSpaceMid		=   SPACEMID;
	m_nSpaceRight	=   SPACERIGHT;
	m_nSpaceTop		=   SPACETOP;
	m_nSpaceBottom =  SPACEBOTTOM;
	m_nCaptionHeight= CAPTIONHEIGHT;
	m_bCaptionBarInvalidated=true;

	m_clrCaptionLeft=RGB(128,128,128);
	m_clrCaptionRight=RGB(128,128,128);

	m_clrCaptionTextLeft=RGB(255,255,255);
	m_clrCaptionTextRight=RGB(255,255,255);

	m_nCaptionBorder	=tps_capborder_line;

	m_clrCaptionBorderTopLeft=RGB(255,255,255);
	m_clrCaptionBorderBottomRight=RGB(64,64,64);
  
	m_pCaptionFont	=NULL;
	m_pCaptionDescrFont=NULL;

	m_bDeleteCaptionFont=false;
	m_bDeleteCaptionDescrFont=false;

	m_bSpecialCaption=false;
	m_bDragging=false;

	m_bDisableStandardButtons=false;
}

CTreePropertySheet::CTreePropertySheet(CWnd* pParentWnd, UINT iSelectPage)
:	CPropertySheet( _T(""), pParentWnd, iSelectPage),
	m_cCaptionBarDC(1,1),
	m_cCaptionDC(1,1)
{
  m_iWantedPage = iSelectPage;
	m_nTreeWidth	=170;
	m_nSpaceLeft	=   SPACELEFT;
	m_nSpaceMid		=   SPACEMID;
	m_nSpaceRight	=   SPACERIGHT;
	m_nSpaceTop		=   SPACETOP;
	m_nSpaceBottom =  SPACEBOTTOM;
	m_nCaptionHeight= CAPTIONHEIGHT;
	m_bCaptionBarInvalidated=true;

	m_clrCaptionLeft=RGB(128,128,128);
	m_clrCaptionRight=RGB(128,128,128);

	m_clrCaptionTextLeft=RGB(255,255,255);
	m_clrCaptionTextRight=RGB(255,255,255);

	m_nCaptionBorder	=tps_capborder_line;

	m_clrCaptionBorderTopLeft=RGB(255,255,255);
	m_clrCaptionBorderBottomRight=RGB(64,64,64);


	m_pCaptionFont	=NULL;
	m_pCaptionDescrFont=NULL;

	m_bDeleteCaptionFont=false;
	m_bDeleteCaptionDescrFont=false;

	m_bSpecialCaption=true;
	m_bDragging=false;

	m_bDisableStandardButtons=false;
}

CTreePropertySheet::~CTreePropertySheet()
{
	if(m_bDeleteCaptionFont && m_pCaptionFont!=NULL)
		delete m_pCaptionFont;

	if(m_bDeleteCaptionDescrFont && m_pCaptionDescrFont!=NULL)
		delete m_pCaptionDescrFont;

	int		nControl,nControls=m_acControlPos.GetSize();

	for(nControl=0;nControl<nControls;nControl++) {
		cControlPos&	cur=m_acControlPos[nControl];

		if(cur.bAutoDelete && cur.pWnd!=NULL)
			delete cur.pWnd;
	}

	int		nFont,nFonts=m_apFonts.GetSize();

	for(nFont=0;nFont<nFonts;nFont++)
		delete m_apFonts[nFont];
}


int CTreePropertySheet::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{

  EnableStackedTabs (FALSE);

	if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}

BEGIN_MESSAGE_MAP(CTreePropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CTreePropertySheet)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_NOTIFY(TVN_SELCHANGED,ID_TREECTRL,OnSelChanged)
	ON_NOTIFY(TVN_SELCHANGEDW,ID_TREECTRL,OnSelChanged)
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Notification handlers

/*--------------------------------------------------------------------
FUNCTION:	OnSelChanged(NMHDR* pNotifyStruct, LRESULT* pResult);
RETURNS:		
PURPOSE:		
--------------------------------------------------------------------*/
void	 CTreePropertySheet::OnSelChanged(
				NMHDR* pNotifyStruct, 
				LRESULT* pResult)
{
	NMTREEVIEW	*pNotify=(NMTREEVIEW*)pNotifyStruct;

	(*pResult)	=0;
	
	int		dwPage=m_cTreeCtrl.GetItemData(pNotify->itemNew.hItem);
	LockWindowUpdate();
  if (GetPageIndex(GetActivePage()) != dwPage)    // NJG
	  SetActivePage(dwPage);
	UnlockWindowUpdate();

	// Prevent losing the focus when invoked by keyboard
	if(pNotify->action==TVC_BYKEYBOARD)
		m_cTreeCtrl.SetFocus();

	InvalidateRect(&m_rcCaptionBar,FALSE);
}

void CTreePropertySheet::OnPaint() 
{
	CPaintDC	dc(this); // device context for painting
	TEXTMETRIC	tm;
	
	RenderCaptionBar();

	m_cCaptionDC.FitInto(m_rcCaptionBar.Width(),
						 m_rcCaptionBar.Height());

	m_cCaptionDC.BitBlt(0,0,m_rcCaptionBar.Width(),m_rcCaptionBar.Height(),
						&m_cCaptionBarDC,0,0,SRCCOPY);

	CSize		extent;
	CFont		*pOldFont;
	int			nPage=GetActiveIndex();
	cPropPage&	curPage=m_acPropPages[nPage];

	pOldFont	=m_cCaptionDC.GetCurrentFont();

	m_cCaptionDC.SelectObject(m_pCaptionFont);
	m_cCaptionDC.GetTextMetrics(&tm);
	m_cCaptionDC.SetTextColor(m_clrCaptionTextLeft);
	m_cCaptionDC.SetBkMode(TRANSPARENT);
	m_cCaptionDC.TextOut(5,m_nCaptionHeight-tm.tmAscent-tm.tmDescent-2,
						 curPage.sCaption);

	m_cCaptionDC.SelectObject(m_pCaptionDescrFont);
	m_cCaptionDC.GetTextMetrics(&tm);
	m_cCaptionDC.SetTextColor(m_clrCaptionTextRight);
	m_cCaptionDC.SetBkMode(TRANSPARENT);
	extent		=m_cCaptionDC.GetTextExtent(curPage.sCaptionDescr);
	m_cCaptionDC.TextOut(m_rcCaptionBar.Width()-4-extent.cx,m_nCaptionHeight-tm.tmAscent-tm.tmDescent-4,
						 curPage.sCaptionDescr);
					
	dc.BitBlt(	m_rcCaptionBar.left,
				m_rcCaptionBar.top,
				m_rcCaptionBar.Width(),
				m_rcCaptionBar.Height(),
				&m_cCaptionDC,0,0,
				SRCCOPY);
}

BOOL CTreePropertySheet::OnInitDialog() 
{
	CPropertySheet::OnInitDialog();
	
	if(m_bSpecialCaption) {
		CRect	rcWindow;

		ModifyStyle(WS_CAPTION,0,0);
		ModifyStyle(WS_SYSMENU,0,0);
		
		GetWindowRect(&rcWindow);
		rcWindow.bottom-=GetSystemMetrics(SM_CYCAPTION);
		MoveWindow(&rcWindow,FALSE);
	}

	if(m_bDisableStandardButtons)
		HidePpgButtons();

	HideTabControl();
	AddTreeView();
	AlignControls();

	int			nPage,nPages=m_acPropPages.GetSize();
	CArray<HTREEITEM,HTREEITEM>	stack;

	for(nPage=0;nPage<nPages;nPage++) {
		cPropPage&	page=m_acPropPages[nPage];
		HTREEITEM	parent=stack.GetSize()?stack[stack.GetSize()-1]:TVI_ROOT;
		HTREEITEM	cur=NULL;
		
		if(page.sCaption.GetLength()==0)
			GetPage(nPage)->GetWindowText(page.sCaption);

		cur			=m_cTreeCtrl.InsertItem(page.sCaption,parent);
		m_cTreeCtrl.SetItemData(cur,nPage);

		int			nType=page.nType&tps_item_type_mask;

		if(nType==tps_item_branch)
			stack.Add(cur);

		if(nType==tps_item_endbranch)
			stack.SetSize(stack.GetSize()-1);

		if(page.nType&tps_item_expanded)
			m_cTreeCtrl.SetItemState(cur,
									 TVIS_EXPANDED,
									 TVIS_EXPANDED);

    if (nPage == m_iWantedPage)
       m_cTreeCtrl.Select (cur, TVGN_CARET);

	}

	if(m_pCaptionFont==NULL)
		SetCaptionFont("MS Sans Serif",18,TRUE);
	if(m_pCaptionDescrFont==NULL)
		SetCaptionDescrFont("MS Sans Serif",18,TRUE);

  // remove apply button
  CWnd* pTemp;
  pTemp = GetDlgItem(ID_APPLY_NOW);
  if (pTemp)
      pTemp->ShowWindow (SW_HIDE);

	return TRUE;  // return TRUE unless you set the focus to a control
}

// Support for dragging window around if 
// m_bSpecialCaption==true
void CTreePropertySheet::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(m_bSpecialCaption) {
		if(m_rcCaptionBar.PtInRect(point)) {
			m_bDragging			=true;
			m_lButtonDownPoint	=point;
			ClientToScreen(&m_lButtonDownPoint);
			SetCapture();
		}
	}
	
	CPropertySheet::OnLButtonDown(nFlags, point);
}

void CTreePropertySheet::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if(m_bSpecialCaption) {
		m_bDragging=false;
		ReleaseCapture();
	}
	
	CPropertySheet::OnLButtonUp(nFlags, point);
}

void CTreePropertySheet::OnMouseMove(UINT nFlags, CPoint point) 
{
	if(m_bSpecialCaption && nFlags&MK_LBUTTON) {
		if(!m_bDragging)
			OnLButtonDown(nFlags,point);

		if(m_bDragging) {
			CRect			rcWindow;
			CPoint			pnt2=point;
			int				dx,dy;
			ClientToScreen(&pnt2);

			dx				=pnt2.x-m_lButtonDownPoint.x;
			dy				=pnt2.y-m_lButtonDownPoint.y;
			m_lButtonDownPoint=pnt2;

			GetWindowRect(&rcWindow);
			rcWindow.OffsetRect(dx,dy);
			MoveWindow(&rcWindow);
		}
	}
	
	CPropertySheet::OnMouseMove(nFlags, point);
}

//////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////

/*--------------------------------------------------------------------
FUNCTION:	SetCaptionDescrFont(LPCTSTR lpszFacename,int nHeight,bool bNoInvalidate=false);
RETURNS:	<void>
PURPOSE:	Sets the font of the caption description field
--------------------------------------------------------------------*/
void		CTreePropertySheet::SetCaptionDescrFont(
					LPCTSTR lpszFacename,
					int nHeight,
					bool bNoInvalidate)
{
	if(m_bDeleteCaptionDescrFont && m_pCaptionDescrFont!=NULL)
		delete m_pCaptionDescrFont;

	m_bDeleteCaptionDescrFont=false;
	m_pCaptionDescrFont		=GetFont(lpszFacename,nHeight);

	if(!bNoInvalidate)
		InvalidateRect(m_rcCaptionBar,FALSE);
}

/*--------------------------------------------------------------------
FUNCTION:	SetCaptionFont(LPCTSTR lpszFacename,int nHeight,bool bNoInvalidate=false);
RETURNS:	<void>
PURPOSE:	Sets the font of the caption bar
--------------------------------------------------------------------*/
void		CTreePropertySheet::SetCaptionFont(
					LPCTSTR lpszFacename,
					int nHeight,
					bool bNoInvalidate)
{
	if(m_bDeleteCaptionFont && m_pCaptionFont!=NULL)
		delete m_pCaptionFont;

	m_bDeleteCaptionFont	=false;
	m_pCaptionFont			=GetFont(lpszFacename,nHeight);

	if(!bNoInvalidate)
		InvalidateRect(m_rcCaptionBar,FALSE);
}

/*--------------------------------------------------------------------
FUNCTION:	SetSimple();
RETURNS:	<void>
PURPOSE:	Prepares the sheet to some 'normal' look
--------------------------------------------------------------------*/
void		CTreePropertySheet::SetSimple(bool bSpecialCaption)
{
	CStatic *info=new CStatic();

	info->Create("Please select from below:",WS_CHILD,CRect(0,0,200,16),GetDesktopWindow());

	CFont *tempF=GetFont("MS Sans Serif",18);
	info->SetFont(tempF);

	if(bSpecialCaption) {
		m_clrCaptionLeft	=RGB(0,0,128);
		m_clrCaptionRight	=RGB(0,0,128);
	}	else
	{
		m_clrCaptionLeft	=RGB(128,128,128);
		m_clrCaptionRight	=RGB(128,128,128);
		m_clrCaptionBorderTopLeft=RGB(64,64,64);
		m_clrCaptionBorderBottomRight=RGB(255,255,255);
	}

	m_clrCaptionTextLeft=RGB(255,255,255);
	m_clrCaptionTextRight=RGB(255,255,255);

	m_nCaptionHeight	=28;
	m_bSpecialCaption	=bSpecialCaption;

	InsertExtraControl(tps_above,3,info,0,0,216,0,tps_horz_right|tps_vert_center,TRUE);
	SetExtraRowSpace(tps_above,0,8,0);
}


/*--------------------------------------------------------------------
FUNCTION:	AddPage()
RETURNS:	<void>
PURPOSE:	Inserts a page into the sheet.
			For more information, see the help files.
--------------------------------------------------------------------*/
void		CTreePropertySheet::AddPage(
				int nType,
				CPropertyPage* pPage,
				LPCTSTR lpszCaptionDescr)
{
	cPropPage	page;

	pPage->m_psp.dwFlags|=PSP_PREMATURE;

	page.nType	=nType;
	page.pPage	=pPage;
//	page.sCaption=_T("");   // wine
//	page.sCaptionDescr=lpszCaptionDescr; // wine
	page.sCaption=lpszCaptionDescr;     // NJG
	page.sCaptionDescr=_T("");          // NJG

	m_acPropPages.Add(page);

	CPropertySheet::AddPage(pPage);
}

/*--------------------------------------------------------------------
FUNCTION:	InsertExtraSpace()
RETURNS:	<void>
PURPOSE:	Insert an empty space into the sheet
--------------------------------------------------------------------*/
void		CTreePropertySheet::InsertExtraSpace(
				int		nVertArrange,
				int		row,int col,
				int		nWidth,
				int		nHeight)
{
	if(nVertArrange==tps_above) {
		m_anRowHeight_TopPane.SetSize(max(m_anRowHeight_TopPane.GetSize(),row+1));
		m_anRowHeight_TopPane[row].nHeight
					=max(m_anRowHeight_TopPane[row].nHeight,nHeight);
	}	else
	{
		m_anRowHeight_BottomPane.SetSize(max(m_anRowHeight_BottomPane.GetSize(),row+1));
		m_anRowHeight_BottomPane[row].nHeight
					=max(m_anRowHeight_BottomPane[row].nHeight,nHeight);
	}

	cControlPos		pos;

	pos.row			=row;
	pos.col			=col;
	pos.nWidth		=nWidth;
	pos.nHeight		=nHeight;
	pos.nArrange	=0;
	pos.nVertArrange=nVertArrange;
	pos.pWnd		=NULL;
	pos.bAutoDelete	=false;
	pos.rcFinalPos	=CRect(0,0,0,0);

	m_acControlPos.Add(pos);
}

/*--------------------------------------------------------------------
FUNCTION:	InsertExtraControl()
RETURNS:	<void>
PURPOSE:	Insert an extra control into the sheet.
--------------------------------------------------------------------*/
void		CTreePropertySheet::InsertExtraControl(
				int		nVertArrange,
				int		zOrder,
				CWnd*	pWnd,
				int		row,int col,
				int		nWidth,int nHeight,
				int		nArrange,
				bool	bAutoDelete)
{
	cControlPos		pos;
	pos.nID			=0;

	if(nHeight==0) {
		if(pos.nID!=0)
			nHeight=pos.nTHeight;
		else {
			CRect	rcTemp;
			pWnd->GetWindowRect(&rcTemp);
			nHeight=rcTemp.Height();
		}
	}

	pos.row			=row;
	pos.col			=col;
	pos.nWidth		=nWidth;
	pos.nHeight		=nHeight;
	pos.nArrange	=nArrange;
	pos.nVertArrange=nVertArrange;
	pos.pWnd		=pWnd;
	pos.bAutoDelete	=bAutoDelete;
	pos.rcFinalPos	=CRect(0,0,0,0);

	m_acControlPos.Add(pos);

	if(nVertArrange==tps_above) {
		m_anRowHeight_TopPane.SetSize(max(m_anRowHeight_TopPane.GetSize(),row+1));
		m_anRowHeight_TopPane[row].nHeight
					=max(m_anRowHeight_TopPane[row].nHeight,nHeight);
	}	else
	{
		m_anRowHeight_BottomPane.SetSize(max(m_anRowHeight_BottomPane.GetSize(),row+1));
		m_anRowHeight_BottomPane[row].nHeight
					=max(m_anRowHeight_BottomPane[row].nHeight,nHeight);
	}

}

/*--------------------------------------------------------------------
FUNCTION:	SetExtraRowSpace(int nVertArrange,int nRow,int nSpaceTop,int nSpaceBottom);
RETURNS:	<void>
PURPOSE:	Reserves extra space for each row
--------------------------------------------------------------------*/
void		CTreePropertySheet::SetExtraRowSpace(
					int nVertArrange,
					int nRow,
					int nSpaceTop,int nSpaceBottom)
{
	CArray<cRowInfo,cRowInfo&>&	
						pane=(nVertArrange==tps_above)?
								m_anRowHeight_TopPane:
								m_anRowHeight_BottomPane;

	pane.SetSize(max(pane.GetSize(),nRow+1));

	pane[nRow].nExtraTop	=nSpaceTop;
	pane[nRow].nExtraBottom	=nSpaceBottom;
}

//////////////////////////////////////////////////////////////////////
// Protected operations
//////////////////////////////////////////////////////////////////////

/*--------------------------------------------------------------------
FUNCTION:	HidePpgButtons();
RETURNS:	<bool>
PURPOSE:	Hides the default property sheet buttons
--------------------------------------------------------------------*/
bool		CTreePropertySheet::HidePpgButtons()
{
	m_acControlPos.GetSize();

	CRect	rcTemp,rcWindow;
	GetDlgItem(IDOK)->GetWindowRect(&rcTemp);
	GetWindowRect(&rcWindow);
	rcWindow.bottom	=rcTemp.top;
	MoveWindow(&rcWindow,FALSE);

  /* for(int nControl=0;nControl<nControls;nControl++) {
		cControlPos&	cur=m_acControlPos[nControl];
		CRect			rcTemp;

		if(cur.pWnd==NULL && cur.nID!=0) {
			cur.pWnd	=GetDlgItem(cur.nID);
			cur.pWnd->SetWindowPos(
				NULL,
				0,0,
				cur.nTWidth,cur.nTHeight,
				SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
		}
		
		if(cur.pWnd!=NULL) {

			cur.pWnd->GetWindowRect(&rcTemp);

			if(cur.nWidth==0)
				cur.nWidth=rcTemp.Width();

			if(cur.nHeight==0)
				cur.nHeight=rcTemp.Height();
		}
	}  */

	return true;
}

/*--------------------------------------------------------------------
FUNCTION:	GetFont(const CString& sFacename,int nHeight);
RETURNS:	<CFont*>
PURPOSE:	Allocates a font and returns a pointer to it.
			All fonts allocated by this function are deleted
			by the destructor.
--------------------------------------------------------------------*/
CFont		*CTreePropertySheet::GetFont(
					const CString& sFacename,
					int nHeight)
{
	CFont	*temp;
	temp	=new CFont();
	temp->CreateFont( nHeight,0,
					  0,0,FW_NORMAL,
					  FALSE,FALSE,FALSE,
					  MUSHCLIENT_FONT_CHARSET,
					  OUT_DEFAULT_PRECIS,
					  CLIP_DEFAULT_PRECIS,
					  DEFAULT_QUALITY,
					  FF_DONTCARE|DEFAULT_PITCH,
					  sFacename);
	m_apFonts.Add(temp);
	return temp;
}

/*--------------------------------------------------------------------
FUNCTION:	RenderCaptionBar();
RETURNS:	<void>
PURPOSE:	Paints the caption bar into a memory dc.
--------------------------------------------------------------------*/
void		CTreePropertySheet::RenderCaptionBar()
{
	if(!m_bCaptionBarInvalidated)
		return;

	m_bCaptionBarInvalidated=false;

	CSize	m_szCaptionBar=m_rcCaptionBar.Size();

	// Adjust memory DCs' size
	m_cCaptionBarDC.FitInto(m_szCaptionBar.cx,m_szCaptionBar.cy);

	int			x;
	CPen		*pOldPen,cNullPen;
	CBrush		*pOldBrush;

	cNullPen.CreateStockObject(NULL_PEN);
	pOldPen		=m_cCaptionBarDC.SelectObject(&cNullPen);
	pOldBrush	=m_cCaptionBarDC.GetCurrentBrush();

	for(x=0;x<m_szCaptionBar.cx;x++) {
		COLORREF	clrCurrent=_interpolate(
									m_clrCaptionLeft,
									m_clrCaptionRight,
									x,
									m_szCaptionBar.cx);

		CBrush		cBrush(clrCurrent);

		m_cCaptionBarDC.SelectObject(&cBrush);
		m_cCaptionBarDC.Rectangle(x,0,x+2,m_szCaptionBar.cy+1);

		m_cCaptionBarDC.SelectObject(pOldBrush);
	}

	if(m_nCaptionBorder==tps_capborder_line) {
		m_cCaptionBarDC.Draw3dRect( 0,0,m_szCaptionBar.cx,m_szCaptionBar.cy,
									m_clrCaptionBorderTopLeft,
									m_clrCaptionBorderBottomRight);
	}

	m_cCaptionBarDC.SelectObject(pOldPen);
}

/*--------------------------------------------------------------------
FUNCTION:	_interpolate()
RETURNS:	COLORREF
PURPOSE:	Interpolates between two given colors
--------------------------------------------------------------------*/
COLORREF	CTreePropertySheet::_interpolate(
				COLORREF clrLeft, 
				COLORREF clrRight, 
				int nStep, 
				int nSteps)
{
	DWORD	r0,g0,b0;
	DWORD	r1,g1,b1;
	DWORD	rc,gc,bc;

	r0		=(DWORD)(clrLeft&0xFFL);
	g0		=(DWORD)(clrLeft&0xFF00L)>>8;
	b0		=(DWORD)(clrLeft&0xFF0000L)>>16;
	r1		=(DWORD)(clrRight&0xFFL);
	g1		=(DWORD)(clrRight&0xFF00L)>>8;
	b1		=(DWORD)(clrRight&0xFF0000L)>>16;
	rc		=((r1*nStep)+(r0*(nSteps-nStep)))/(nSteps);
	gc		=((g1*nStep)+(g0*(nSteps-nStep)))/(nSteps);
	bc		=((b1*nStep)+(b0*(nSteps-nStep)))/(nSteps);

	return RGB(rc,gc,bc);
}

/*--------------------------------------------------------------------
FUNCTION:	PlaceControl(cControlPos* pCtrl);
RETURNS:	<void>
PURPOSE:	Internally places the given control
--------------------------------------------------------------------*/
void		CTreePropertySheet::PlaceControl(cControlPos* pCtrl)
{
/*	if(pCtrl->pWnd==NULL && pCtrl->nID!=0)
		pCtrl->pWnd	=GetDlgItem(pCtrl->nID);
*/
	if(pCtrl->pWnd!=NULL) {
		CRect	rcItem;
		CSize	szItem;

		pCtrl->pWnd->SetParent(this);
		pCtrl->pWnd->GetClientRect(&rcItem);

		szItem	=rcItem.Size();

		int		nHorzArr=(pCtrl->nArrange&tps_horz_mask);
		int		nVertArr=(pCtrl->nArrange&tps_vert_mask);

		if( nHorzArr==tps_horz_left) {
			rcItem.left	=pCtrl->rcFinalPos.left;
			rcItem.right=rcItem.left+szItem.cx;
		}
		if( nHorzArr==tps_horz_center) {
			rcItem.left	=(pCtrl->rcFinalPos.left+
						  pCtrl->rcFinalPos.right-szItem.cx)/2;
			rcItem.right=rcItem.left+szItem.cx;
		}
		if( nHorzArr==tps_horz_right) {
			rcItem.left	=pCtrl->rcFinalPos.right-szItem.cx;
			rcItem.right=rcItem.left+szItem.cx;
		}

		if( nVertArr==tps_vert_top) {
			rcItem.top	=pCtrl->rcFinalPos.top;
			rcItem.bottom=rcItem.top+szItem.cy;
		}
		if( nVertArr==tps_vert_center) {
			rcItem.top	=(pCtrl->rcFinalPos.top+
						  pCtrl->rcFinalPos.bottom-szItem.cy)/2;
			rcItem.bottom=rcItem.top+szItem.cy;
		}
		if( nVertArr==tps_vert_top) {
			rcItem.top	=pCtrl->rcFinalPos.bottom-szItem.cy;
			rcItem.bottom=rcItem.top+szItem.cy;
		}

		pCtrl->pWnd->MoveWindow(&rcItem,FALSE);
		pCtrl->pWnd->SetFont(CWnd::GetFont(),TRUE);
	}
}

/*--------------------------------------------------------------------
FUNCTION:	AddPaneSpace(int nWidth, int nTopPane,int nBottomPane);
RETURNS:	<void>
PURPOSE:	Reserves space for the extra control panes.
			Internally used.
--------------------------------------------------------------------*/
void		CTreePropertySheet::AddPaneSpace(int nWidth, int nTopPane,int nBottomPane)
{
	CRect	rcClient;
	CRect	rcWindow;
	GetClientRect(&rcClient);
	GetWindowRect(&rcWindow);

	int		nAddSpaceX=(nWidth-rcClient.Width());
	int		nAddSpaceY=nTopPane+nBottomPane;

	int		nLeft=nAddSpaceX/2;
	int		nRight=(nAddSpaceX-nLeft);
	int		nTop=nAddSpaceY/2;
	int		nBottom=(nAddSpaceY-nTop);

	rcWindow.left-=nLeft;
	rcWindow.right+=nRight;
	rcWindow.top-=nTop;
	rcWindow.bottom+=nBottom;
	MoveWindow(&rcWindow,FALSE);

	ScrollWindow(nLeft,nTopPane);

	// Move only the bottoms down by 'nTop'
	CWnd*	pWndChild=GetWindow(GW_CHILD);
	TCHAR	szClass[256];

	while(pWndChild!=NULL) {
		::GetClassName(	pWndChild->GetSafeHwnd(),
						szClass,256);

		if(stricmp(szClass,"Button")==0) {
			CRect		rcButton;

			pWndChild->GetClientRect(&rcButton);
			pWndChild->MapWindowPoints(this,&rcButton);
			rcButton.OffsetRect(0,nBottomPane);
			pWndChild->MoveWindow(&rcButton,FALSE);

			if(m_bDisableStandardButtons) {
				pWndChild->EnableWindow(FALSE);
				pWndChild->ModifyStyle(BS_DEFPUSHBUTTON,0,0);
			}
		}

		pWndChild	=pWndChild->GetWindow(GW_HWNDNEXT);
	}
}

/*--------------------------------------------------------------------
FUNCTION:	AddTreeView();
RETURNS:	<void>
PURPOSE:	Adds the tree view control to the standard
			property sheet.
--------------------------------------------------------------------*/
void		CTreePropertySheet::AddTreeView()
{
	// Insert the space to the left
	int		nAddSpace=m_nTreeWidth+m_nSpaceLeft;
	int		nLeft=nAddSpace/2;
	int		nRight=(nAddSpace-nLeft);
	int		nTopOfTreeView;
	CRect	rcWindow,rcPage,rcTab;

	GetWindowRect(&rcWindow);
	rcWindow.left-=nLeft;
	rcWindow.right+=nRight;
	MoveWindow(&rcWindow,FALSE);

	GetPage(0)->GetWindowRect(&rcPage);
	ScreenToClient(&rcPage);
	GetTabControl()->GetWindowRect(&rcTab);
	ScreenToClient(&rcTab);

	ScrollWindow(nAddSpace,0);

	if(m_bSpecialCaption)
		nTopOfTreeView	=rcPage.top;
	else
		nTopOfTreeView	=rcPage.top-m_nCaptionHeight;

 
  // added in version 3.50 for Wine - otherwise tree view was in wrong place
  nTopOfTreeView = 5;

  // NJG - added  TVS_SHOWSELALWAYS

  
	m_cTreeCtrl.CWnd::CreateEx(WS_EX_CLIENTEDGE,
							 WC_TREEVIEW, 
							 NULL, 
							 WS_CHILD|WS_VISIBLE|WS_TABSTOP|TVS_SHOWSELALWAYS|
							 TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS, 
							 m_nSpaceLeft,nTopOfTreeView,m_nTreeWidth,rcTab.bottom-nTopOfTreeView+5,
							 GetSafeHwnd(), 
							 (HMENU)ID_TREECTRL);

  
  BOOL bIsUnicode;


#ifdef UNICODE
  bIsUnicode = TreeView_SetUnicodeFormat (m_cTreeCtrl.m_hWnd, 1);
#else
  bIsUnicode = TreeView_SetUnicodeFormat (m_cTreeCtrl.m_hWnd, 0);
#endif


}

/*--------------------------------------------------------------------
FUNCTION:	HideTabControl();
RETURNS:	<void>
PURPOSE:	Hides the tab control of the property sheet.
--------------------------------------------------------------------*/
void		CTreePropertySheet::HideTabControl()
{
	CRect	rcClient,rcTab,rcPage,rcWindow;

	GetClientRect(&rcClient);
	GetTabControl()->GetWindowRect(&rcTab);
	GetPage(0)->GetClientRect(&rcPage);
	GetPage(0)->MapWindowPoints(this,&rcPage);
	GetWindowRect(&rcWindow);
	ScreenToClient(&rcTab);

  // NJG - to fix Wine bug (with stuff off on RH side)
//	ScrollWindow(	m_nSpaceMid-rcPage.left,
//					m_nSpaceTop-rcPage.top+m_nCaptionHeight);
	rcWindow.right+=m_nSpaceMid-rcPage.left-(rcClient.Width()-rcTab.right)+m_nSpaceRight;
	rcWindow.bottom+=m_nSpaceTop-rcPage.top+m_nCaptionHeight;

	GetTabControl()->ShowWindow(SW_HIDE);

  // NJG - to fix Wine bug (with stuff off on RH side)

//	MoveWindow(&rcWindow,FALSE);
}

/*--------------------------------------------------------------------
FUNCTION:	AssignFillerSpace()
RETURNS:	<void>
PURPOSE:	Assigns each space filler control
			(width==-1) the remaining space of each row.
--------------------------------------------------------------------*/
void		CTreePropertySheet::AssignFillerSpace(
				int nVertArrange, 
				int nRow, 
				int nSpace)
{
	int		i,nSize=m_acControlPos.GetSize();
	int		nFillers=CountFillers(nVertArrange,nRow);

	if(nFillers==0)
		return;

	int		nItemSpace=nSpace/nFillers;
	int		nSpaceLeft=nSpace;
	int		nCurFiller=0;

	for(i=0;i<nSize;i++) {
		cControlPos&	cur=m_acControlPos[i];

		if(	!(cur.nVertArrange==nVertArrange &&
			cur.row==nRow))
			continue;

		if( cur.nWidth!=-1)
			continue;

		nCurFiller++;
		if(nCurFiller==nFillers)
			cur.nWidth	=nSpaceLeft;
		else
			cur.nWidth	=nItemSpace;

		nSpaceLeft	-=cur.nWidth;
	}
}

/*--------------------------------------------------------------------
FUNCTION:	CountFillers(int nVertArrange,int nRow);
RETURNS:	<int>
PURPOSE:	Counts the number of space filler controls
			for the given row.
--------------------------------------------------------------------*/
int			CTreePropertySheet::CountFillers(
				int nVertArrange,
				int nRow)
{
	int		i,nSize=m_acControlPos.GetSize();
	int		nCount=0;

	for(i=0;i<nSize;i++) {
		cControlPos&	cur=m_acControlPos[i];

		if(	!(cur.nVertArrange==nVertArrange &&
			cur.row==nRow))
			continue;

		if( cur.nWidth==-1)
			nCount++;
	}

	return nCount;
}

/*--------------------------------------------------------------------
FUNCTION:	CountColumns(int nVertArrange,int nCol);
RETURNS:	<int>
PURPOSE:	Counts the number of columns in one row.
--------------------------------------------------------------------*/
int			CTreePropertySheet::CountColumns(int nVertArrange,int nRow)
{
	int		i,nSize=m_acControlPos.GetSize();
	int		nCount=0;

	for(i=0;i<nSize;i++) {
		cControlPos&	cur=m_acControlPos[i];

		if(	!(cur.nVertArrange==nVertArrange &&
			cur.row==nRow))
			continue;

		nCount++;
	}

	return nCount;
}

/*--------------------------------------------------------------------
FUNCTION:	GetControl(int nVertArrange,int nRow,int nCol);
RETURNS:	<cControlPos*>
PURPOSE:	Returns the control description class for the
			given row/column.
--------------------------------------------------------------------*/
CTreePropertySheet::cControlPos* CTreePropertySheet::GetControl(
				int nVertArrange,
				int nRow,
				int nCol)
{
	int		i,nSize=m_acControlPos.GetSize();

	for(i=0;i<nSize;i++) {
		cControlPos&	cur=m_acControlPos[i];

		if(	!(cur.nVertArrange==nVertArrange &&
			cur.row==nRow))
			continue;

		if( cur.col==nCol)
			return &cur;
	}

	return NULL;
}

/*--------------------------------------------------------------------
FUNCTION:	GetWidth(int nVertArrange, int nRow);
RETURNS:	The width of the given row
PURPOSE:	Calculates the width of the given row.
			Drops items with the width '-1'.
			ASSERTs if the width <-1!!!
--------------------------------------------------------------------*/
int			CTreePropertySheet::GetWidth(
				int nVertArrange, 
				int nRow)
{
	int		i,nSize=m_acControlPos.GetSize();
	int		nWidth=0;

	for(i=0;i<nSize;i++) {
		cControlPos&	cur=m_acControlPos[i];

		if(	!(cur.nVertArrange==nVertArrange &&
			cur.row==nRow))
			continue;

		if( cur.nWidth==-1)
			continue;

		ASSERT(cur.nWidth>=0);

		nWidth	+=cur.nWidth;
	}

	return nWidth;
}

/*--------------------------------------------------------------------
FUNCTION:	GetHeight(CArray<cRowInfo,cRowInfo&>& pane)
RETURNS:	The sum of the values of the array 'pane'
PURPOSE:	Calculates the height of the given pane
--------------------------------------------------------------------*/
int			CTreePropertySheet::GetHeight(
				CArray<cRowInfo,cRowInfo&>& pane)
{
	int		i,nSize=pane.GetSize();
	int		nSum=0;

	for(i=0;i<nSize;i++)
        nSum+=pane[i].nHeight+pane[i].nExtraBottom+pane[i].nExtraTop;
	return nSum;
}

/*--------------------------------------------------------------------
FUNCTION:	AlignControls();
RETURNS:	<void>
PURPOSE:	Performs the alignment of the extra controls.
--------------------------------------------------------------------*/
void		CTreePropertySheet::AlignControls()
{
	int			nTotalHeight_TopPane=GetHeight(m_anRowHeight_TopPane);
	int			nTotalHeight_BottomPane=GetHeight(m_anRowHeight_BottomPane);
	int			nVertArrange;
	int			nMaxWidth=0;
	CRect		rcClient;

	GetClientRect(&rcClient);

	// Determine the maximum width of all rows
	for(nVertArrange=tps_above;nVertArrange<=tps_below;nVertArrange++) {
		int					nRow,nRows;
		CArray<cRowInfo,cRowInfo&>&	
							pane=(nVertArrange==tps_above)?
									m_anRowHeight_TopPane:
									m_anRowHeight_BottomPane;
		
		nRows	=pane.GetSize();

		for(nRow=0;nRow<nRows;nRow++) {
			int				nWidth;
			nWidth			=GetWidth(nVertArrange,nRow);
			nMaxWidth		=max(nWidth,nMaxWidth);
		}
	}

	//	Get the maximum of the actual dialog width and 'nMaxWidth'
	//	(to get the overall width)
	nMaxWidth	=max(rcClient.Width(),nMaxWidth);

	//	Align the controls' width (to make those with 'width' is -1
	//	equally spaced)
	for(nVertArrange=tps_above;nVertArrange<=tps_below;nVertArrange++) {
		int					nRow,nRows;
		CArray<cRowInfo,cRowInfo&>&	
							pane=(nVertArrange==tps_above)?
									m_anRowHeight_TopPane:
									m_anRowHeight_BottomPane;
		
		nRows	=pane.GetSize();

		for(nRow=0;nRow<nRows;nRow++) {
			int				nWidth;
			nWidth			=GetWidth(nVertArrange,nRow);

			AssignFillerSpace(nVertArrange,nRow,nMaxWidth-nWidth);
		}
	}

	// Resize the dialog
	AddPaneSpace(nMaxWidth, nTotalHeight_TopPane, nTotalHeight_BottomPane);

	// Calculate each controls' position
	CRect		rcPage;
	GetPage(0)->GetWindowRect(&rcPage);
	ScreenToClient(&rcPage);

	int	nCurXPos;
	int	nCurYPos;
	int	nTopPos;

	if(m_bSpecialCaption)
		nTopPos	=m_nCaptionHeight;
	else
		nTopPos	=0;

	for(nVertArrange=tps_above;nVertArrange<=tps_below;nVertArrange++) {
		int					nRow,nRows;
		CArray<cRowInfo,cRowInfo&>&	
							pane=(nVertArrange==tps_above)?
									m_anRowHeight_TopPane:
									m_anRowHeight_BottomPane;
		
		nRows	=pane.GetSize();
		if(nVertArrange==tps_above)
			nCurYPos=nTopPos;
		else
			nCurYPos=rcPage.bottom+m_nSpaceBottom;

		for(nRow=0;nRow<nRows;nRow++) {
			int				nCol,nCols;

			nCols			=CountColumns(nVertArrange,nRow);
			nCurXPos		=0;
			for(nCol=0;nCol<nCols;nCol++) {
				cControlPos	*pCur=GetControl(nVertArrange,nRow,nCol);
				
				pCur->rcFinalPos.left	=nCurXPos;
				pCur->rcFinalPos.right	=nCurXPos+pCur->nWidth;
				pCur->rcFinalPos.top	=nCurYPos+pane[nRow].nExtraTop;
				pCur->rcFinalPos.bottom	=nCurYPos+pane[nRow].nHeight+pane[nRow].nExtraTop;

				PlaceControl(pCur);

				nCurXPos	+=pCur->nWidth;
			}

            nCurYPos        +=pane[nRow].nHeight+pane[nRow].nExtraBottom+pane[nRow].nExtraTop;
		}
	}

	if(m_bSpecialCaption) {
		CRect		rcTemp;
		GetWindowRect(&rcTemp);

		m_rcCaptionBar.top	=0;
		m_rcCaptionBar.left	=0;
		m_rcCaptionBar.bottom=m_nCaptionHeight;
		m_rcCaptionBar.right=rcTemp.Width();
	}	else
	{
		m_rcCaptionBar.left		=m_nSpaceLeft+m_nTreeWidth+m_nSpaceMid;
		m_rcCaptionBar.top		=m_nSpaceTop+nTotalHeight_TopPane;
		m_rcCaptionBar.right	=rcPage.right;
		m_rcCaptionBar.bottom	=m_nSpaceTop+nTotalHeight_TopPane+m_nCaptionHeight;
	}
}


/*
    NJG - This wanky stuff here was added to make sure the treeview control
    was updated correctly after the user used CTRL+TAB or CTRL+PAGEUP to change
    pages. Without it the page would change, but the control wouldn't.


  */
void CTreePropertySheet::PageHasChanged (CPropertyPage* pNewPage)
  {

  if (m_cTreeCtrl.m_hWnd == NULL)
    return;

CPropertyPage * pCurrentPage = GetActivePage ();

  if (pCurrentPage != pNewPage)
  	for (DWORD nPage=0; nPage < (DWORD) m_acPropPages.GetSize(); nPage++) 
      {
      if (m_acPropPages[nPage].pPage == pNewPage)
        {
        HTREEITEM hdlItem = m_cTreeCtrl.GetNextItem (NULL, TVGN_ROOT);

        while (true)
          {

          // find its page number
          DWORD	dwPage = m_cTreeCtrl.GetItemData (hdlItem);
          if (dwPage == nPage)
            {
            m_cTreeCtrl.Select (hdlItem, TVGN_CARET);
            break;    
            }         // end of found the page

          // first look for children of the current item
          if (m_cTreeCtrl.ItemHasChildren (hdlItem))
            hdlItem = m_cTreeCtrl.GetChildItem (hdlItem);
          else
            {
            while (true)
              {
              HTREEITEM hdlCurrentItem = hdlItem;

            // next look for siblings of the current item
              hdlItem = m_cTreeCtrl.GetNextItem (hdlItem, TVGN_NEXT);
              if (hdlItem)
                break;    // found one - exit loop
              // no siblings - get parent - and then next sibling
              if (!hdlItem)
                {
                hdlItem = m_cTreeCtrl.GetParentItem (hdlCurrentItem);
                if (!hdlItem)
                  break;    // no parent - must be at end of tree control
                } // end of no getting parent
              } // end of finding a sibling somewhere
            }

          // end of tree control - start at root (wrap around), or change areas
          if (!hdlItem)
            return;   // not found

          } // end of searching for item

        break;
        }
      }   // end of searching all pages

  }   // end of correct tree item not being displayed


LRESULT CTreePropertySheet::OnKickIdle(WPARAM, LPARAM)
  {
  GetActivePage ()->SendMessage (WM_KICKIDLE, 0, 0);
  return 0;
  } // end of CGlobalPrefsSheet::OnKickIdle
