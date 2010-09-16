// ColourPickerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "world_prefs\ButtonColour.h"
#include "ColourPickerDlg.h"
#include "..\Color.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static COLORREF iLastColour = 0;      // black is default

enum {
    eColourName,
    eColourHue,
    eColourSaturation,
    eColourLuminance,
    eRed,
    eGreen,
    eBlue
    };

// sort type - see enum above
static int iSort = eColourHue;

/////////////////////////////////////////////////////////////////////////////
// CColourPickerDlg dialog


CColourPickerDlg::CColourPickerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CColourPickerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CColourPickerDlg)
	//}}AFX_DATA_INIT

  m_iColour = 0;
  m_bPickColour = false;

  m_clipboardColour = RGB (0, 0, 0);
}


void CColourPickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColourPickerDlg)
	DDX_Control(pDX, IDC_COLOUR_NAME, m_ctlColourName);
	DDX_Control(pDX, IDC_VB_NAME, m_ctlVBname);
	DDX_Control(pDX, IDC_MXP_NAME, m_ctlMXPname);
	DDX_Control(pDX, IDC_LUA_NAME, m_ctlLuaName);
	DDX_Control(pDX, IDC_JSCRIPT_NAME, m_ctlJscriptName);
	DDX_Control(pDX, IDC_BLUE_SLIDER, m_ctlBlueSlider);
	DDX_Control(pDX, IDC_GREEN_SLIDER, m_ctlGreenSlider);
	DDX_Control(pDX, IDC_RED_SLIDER, m_ctlRedSlider);
	DDX_Control(pDX, IDOK, m_ctlOKbutton);
	DDX_Control(pDX, IDC_BRIGHTNESS, m_ctlLuminance);
	DDX_Control(pDX, IDC_SATURATION, m_ctlSaturation);
	DDX_Control(pDX, IDC_HUE, m_ctlHue);
	DDX_Control(pDX, IDC_COLOUR_SORT, m_ctlColourSort);
	DDX_Control(pDX, IDC_COLOURLIST, m_ctlColourList);
	DDX_Control(pDX, IDC_BLUE, m_ctlBlue);
	DDX_Control(pDX, IDC_GREEN, m_ctlGreen);
	DDX_Control(pDX, IDC_RED, m_ctlRed);
	DDX_Control(pDX, IDC_COLOUR_SWATCH, m_ctlSwatch);
	//}}AFX_DATA_MAP

  if (!pDX->m_bSaveAndValidate)
    m_ctlColourSort.SetCurSel (iSort);

}


BEGIN_MESSAGE_MAP(CColourPickerDlg, CDialog)
	//{{AFX_MSG_MAP(CColourPickerDlg)
	ON_BN_CLICKED(IDC_SELECT_COLOUR, OnSelectColour)
	ON_BN_CLICKED(IDC_COLOUR_SWATCH, OnColourSwatch)
	ON_NOTIFY(NM_DBLCLK, IDC_COLOURLIST, OnDblclkColourlist)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_COLOURLIST, OnItemchangedColourlist)
	ON_CBN_SELCHANGE(IDC_COLOUR_SORT, OnSelchangeColourSort)
	ON_BN_CLICKED(IDC_PASTE, OnPaste)
	ON_BN_CLICKED(IDC_RANDOM, OnRandom)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_MXP_NAME, OnMxpName)
	ON_BN_CLICKED(IDC_VB_NAME, OnVbName)
	ON_BN_CLICKED(IDC_JSCRIPT_NAME, OnJscriptName)
	ON_BN_CLICKED(IDC_LUA_NAME, OnLuaName)
	ON_BN_CLICKED(IDC_COLOUR_NAME, OnColourName)
	//}}AFX_MSG_MAP
  ON_NOTIFY ( NM_CUSTOMDRAW, IDC_COLOURLIST, OnCustomdrawColourList )
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_UPDATE_COMMAND_UI(IDC_PASTE, OnUpdateNeedClipboard)
  ON_UPDATE_COMMAND_UI(IDC_COLOUR_NAME, OnUpdateNeedName)
END_MESSAGE_MAP()

// CompareFunc

static int CALLBACK CompareFunc ( LPARAM lParam1, 
                                 LPARAM lParam2,
                                 LPARAM lParamSort)
{ 


CColours * pColour1 = (CColours *) lParam1;
CColours * pColour2 = (CColours *) lParam2;

 ASSERT (pColour1 != NULL);
 ASSERT (pColour2 != NULL);

CColor clr1;
CColor clr2;

  clr1.SetColor (pColour1->iColour);
  clr2.SetColor (pColour2->iColour);

int iResult = 0;

  switch (lParamSort)
    {
    case eRed:
      if (GetRValue (pColour1->iColour) < GetRValue (pColour2->iColour))
        iResult = -1;
      else if (GetRValue (pColour1->iColour) > GetRValue (pColour2->iColour))
        iResult = 1;
      else
        lParamSort = eColourHue;
      break;

    case eGreen:
      if (GetGValue (pColour1->iColour) < GetGValue (pColour2->iColour))
        iResult = -1;
      else if (GetGValue (pColour1->iColour) > GetGValue (pColour2->iColour))
        iResult = 1;
      else
        lParamSort = eColourHue;
      break;

    case eBlue:
      if (GetBValue (pColour1->iColour) < GetBValue (pColour2->iColour))
        iResult = -1;
      else if (GetBValue (pColour1->iColour) > GetBValue (pColour2->iColour))
        iResult = 1;
      else
        lParamSort = eColourHue;
      break;

    } // end of switch

// if red/green/blue are the same, then sort on hue.

  switch (lParamSort)
    {
    case eColourName: 
      iResult = pColour1->strName.CompareNoCase (pColour2->strName);
      break;

    case eColourHue:
      {
      double hue1 = clr1.GetHue ();
      double hue2 = clr2.GetHue ();
      if (hue1 < hue2)
        {
        iResult = -1;
        break;
        }
      else if (hue1 > hue2)
        {
        iResult = 1;
        break;
        }
      }
    
    // hue the same - fall through and compare saturation (then luminance)
    case eColourSaturation:
      {
      double saturation1 = clr1.GetSaturation ();
      double saturation2 = clr2.GetSaturation ();
      if (saturation1 < saturation2)
        iResult = -1;
      else if (saturation1 > saturation2)
        iResult = 1;
      else
        { // saturation the same - compare luminance
        double luminance1 = clr1.GetLuminance ();
        double luminance2 = clr2.GetLuminance ();
        if (luminance1 < luminance2)
          iResult = -1;
        else if (luminance1 > luminance2)
          iResult = 1;
        } // end same saturation
      }
      break;

    case eColourLuminance:
      {
      double luminance1 = clr1.GetLuminance ();
      double luminance2 = clr2.GetLuminance ();
      if (luminance1 < luminance2)
        iResult = -1;
      else if (luminance1 > luminance2)
        iResult = 1;
      else
        { // luminance the same - compare saturation
        double saturation1 = clr1.GetSaturation ();
        double saturation2 = clr2.GetSaturation ();
        if (saturation1 < saturation2)
          iResult = -1;
        else if (saturation1 > saturation2)
          iResult = 1;
        } // end same luminance
      }
      break;

    } // end of switch

// if reverse sort wanted, reverse sense of result

//  if (psort_param->reverse)
//    iResult *= -1;

  return iResult;

  } // end of CompareFunc


/////////////////////////////////////////////////////////////////////////////
// CColourPickerDlg message handlers

BOOL CColourPickerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  m_ctlColourList.SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, 0, 
                        m_ctlColourList.SendMessage (LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) 
                        | LVS_EX_FULLROWSELECT 
                        | (App.m_bShowGridLinesInListViews ? LVS_EX_GRIDLINES : 0)  );

  m_ctlColourList.InsertColumn(0, TranslateHeading ("Colour name"), LVCFMT_LEFT, 130);
  // swatch
  m_ctlColourList.InsertColumn(1, "", LVCFMT_LEFT, 91);
 
  int iItem = 0;

  for (POSITION pos = App.m_ColoursMap.GetStartPosition(); pos; iItem++)
    {

    CColours * pColour;
    CString strColourName;

    App.m_ColoursMap.GetNextAssoc (pos, strColourName, pColour);

    // remember RGB code
    if (m_ctlColourList.InsertItem (iItem, strColourName) != -1)
      {
  	  m_ctlColourList.SetItemText(iItem, 1, ""); // swatch
      m_ctlColourList.SetItemData (iItem, (unsigned long) pColour);
      }
    }
	
  // user-supplied colour
  if (m_bPickColour)
    {
    iLastColour = m_iColour;
    m_ctlOKbutton.SetWindowText ("OK");
    }

  m_ctlSwatch.m_colour = iLastColour;

  m_ctlRedSlider.SetRange (0, 255);
  m_ctlGreenSlider.SetRange (0, 255);
  m_ctlBlueSlider.SetRange (0, 255);
  m_ctlRedSlider.SetPageSize (10);
  m_ctlGreenSlider.SetPageSize (10);
  m_ctlBlueSlider.SetPageSize (10);

  // show the name corresponding to the initial colour
  ShowName ();

  m_ctlColourList.SortItems (CompareFunc, iSort); 

  // select the new colour, if possible

  SelectCurrentColour ();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

static COLORREF CustomColours [16]
  = { 0 };

void CColourPickerDlg::OnSelectColour() 
{
CColorDialog dlg (m_ctlSwatch.m_colour, 
                  CC_RGBINIT | CC_FULLOPEN, 
                  this);
  
  dlg.m_cc.lpCustColors = CustomColours;

  if (dlg.DoModal () != IDOK)
    return;

  m_ctlSwatch.m_colour = dlg.GetColor ();
  m_ctlSwatch.RedrawWindow ();
  ShowName ();

  // select the new colour, if possible

  SelectCurrentColour ();
	
}

void CColourPickerDlg::OnDblclkColourlist(NMHDR* pNMHDR, LRESULT* pResult)
{

	*pResult = 0;

  OnOK ();    // accept current selection
	
}


void CColourPickerDlg::ShowName (void)
  {
int iRed    = GetRValue (m_ctlSwatch.m_colour);
int iGreen  = GetGValue (m_ctlSwatch.m_colour);
int iBlue   = GetBValue (m_ctlSwatch.m_colour);

CColor clr;

  clr.SetColor (m_ctlSwatch.m_colour);

double fHue = clr.GetHue ();
double fSaturation = clr.GetSaturation ();
double fLuminance = clr.GetLuminance ();

  CString strName;

// see if we can find colour name in list
  for (POSITION pos = App.m_ColoursMap.GetStartPosition(); pos; )
    {

    CColours * pColour;
    CString strColourName;

    App.m_ColoursMap.GetNextAssoc (pos, strColourName, pColour);

    // note - colour might match more than one name
    if (pColour->iColour == m_ctlSwatch.m_colour)
      {
      if (!strName.IsEmpty ())
        strName += ", ";
      strName += strColourName;
      }
    } // end of loop

  m_ctlColourName.SetWindowText (strName);
  
  m_strMXPname = CFormat ("#%02X%02X%02X",
                         iRed,
                         iGreen,
                         iBlue);

  m_ctlMXPname.SetWindowText (m_strMXPname);

  m_ctlVBname.SetWindowText (CFormat ("&&h%02X%02X%02X",
         iBlue,
         iGreen,
         iRed));

  m_ctlJscriptName.SetWindowText (CFormat ("0x%02X%02X%02X",
         iBlue,
         iGreen,
         iRed));

  m_ctlLuaName.SetWindowText (CFormat ("%ld", m_ctlSwatch.m_colour));

  m_iColour = iLastColour = m_ctlSwatch.m_colour;

  m_ctlRed.SetWindowText    (CFormat ("%i", iRed));
  m_ctlGreen.SetWindowText  (CFormat ("%i", iGreen));
  m_ctlBlue.SetWindowText   (CFormat ("%i", iBlue));

  m_ctlHue.SetWindowText    (CFormat ("Hue: %5.1f", fHue));
  m_ctlSaturation.SetWindowText  (TFormat ("Saturation: %5.3f", fSaturation));
  m_ctlLuminance.SetWindowText   (TFormat ("Luminance: %5.3f", fLuminance));

  m_ctlRedSlider.SetPos (iRed);
  m_ctlGreenSlider.SetPos (iGreen);
  m_ctlBlueSlider.SetPos (iBlue);

  }   // end of CColourPickerDlg::ShowName

void CColourPickerDlg::OnColourSwatch() 
{
OnSelectColour ();	
}

void CColourPickerDlg::OnItemchangedColourlist(NMHDR* pNMHDR, LRESULT* pResult) 
{

int iSel = m_ctlColourList.GetNextItem(-1, LVNI_SELECTED);

  if (iSel == -1)
    return;   // no item selected

  CColours * pColour = (CColours * ) m_ctlColourList.GetItemData (iSel);
  m_ctlSwatch.m_colour = pColour->iColour;
  m_ctlSwatch.RedrawWindow ();
  ShowName ();
  
	*pResult = 0;
}

void CColourPickerDlg::OnSelchangeColourSort() 
{
int iSel = m_ctlColourSort.GetCurSel ();

  if (iSel == CB_ERR)
    return;

  iSort = iSel;

  m_ctlColourList.SortItems (CompareFunc, iSort); 

  SelectCurrentColour ();
  
}  // end of CColourPickerDlg::OnSelchangeColourSort


void CColourPickerDlg::SelectCurrentColour (void)
  {
  for (int i = 0; i < m_ctlColourList.GetItemCount (); i++)
    {

    CColours * pColour = (CColours * ) m_ctlColourList.GetItemData (i);
    if (pColour->iColour == m_ctlSwatch.m_colour)
      {
      m_ctlColourList.SetItemState (i, 
                                    LVIS_FOCUSED | LVIS_SELECTED,
                                    LVIS_FOCUSED | LVIS_SELECTED);
      m_ctlColourList.EnsureVisible (i, false);
      }
    else
      m_ctlColourList.SetItemState (i, 0, LVIS_FOCUSED | LVIS_SELECTED);

    }

  } // end of CColourPickerDlg::SelectCurrentColour


void CColourPickerDlg::OnCustomdrawColourList ( NMHDR* pNMHDR, LRESULT* pResult )

  {
static COLORREF crNormal = 0xFF000000;

NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );
//NMCUSTOMDRAW*   pNMCD = reinterpret_cast<NMCUSTOMDRAW*>( pNMHDR );

  // Take the default processing unless we set this to something else below.
  *pResult = CDRF_DODEFAULT;

  // First thing - check the draw stage. If it's the control's prepaint
  // stage, then tell Windows we want messages for every item.

  if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
      {
      *pResult = CDRF_NOTIFYITEMDRAW;
      }

  else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
      {
      // This is the notification message for an item.  We'll request
      // notifications before each subitem's prepaint stage.

      *pResult = CDRF_NOTIFYSUBITEMDRAW;
      }
  

  else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM)  == pLVCD->nmcd.dwDrawStage )
      {

      int    nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );

      if (nItem == 0 && pLVCD->iSubItem == 0)
         crNormal = pLVCD->clrTextBk;

      if ( 1 == pLVCD->iSubItem )
          {
          int    nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );

          CColours * pColour = (CColours * ) m_ctlColourList.GetItemData (nItem);
          CBrush b (pColour->iColour);

          // the rectangle in the pLVCD->nmcd is wrong!!
          CRect rc;
          m_ctlColourList.GetSubItemRect (nItem, pLVCD->iSubItem, LVIR_BOUNDS, rc);

      		FillRect(pLVCD->nmcd.hdc, &rc, b);

          if (pLVCD->nmcd.uItemState & ODS_FOCUS)
            DrawFocusRect (pLVCD->nmcd.hdc, &rc); 

          *pResult = CDRF_SKIPDEFAULT;

          }
       else
         pLVCD->clrTextBk = crNormal;

      }

  }

void CColourPickerDlg::OnPaste() 
{
  m_ctlSwatch.m_colour = m_clipboardColour;
  m_ctlSwatch.RedrawWindow ();
  ShowName ();

  // select the new colour, if possible

  SelectCurrentColour ();
	
}



void CColourPickerDlg::OnUpdateNeedClipboard(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (GetClipboardColour (m_clipboardColour));
  } // end of CColourPickerDlg::OnUpdateNeedClipboard


void CColourPickerDlg::OnUpdateNeedName(CCmdUI* pCmdUI)
  {
  pCmdUI->Enable (!GetText (m_ctlColourName).IsEmpty ());
  } // end of CColourPickerDlg::OnUpdateNeedName


LRESULT CColourPickerDlg::OnKickIdle(WPARAM, LPARAM)
  {
  UpdateDialogControls (AfxGetApp()->m_pMainWnd, false);
  return 0;
  } // end of CColourPickerDlg::OnKickIdle

void CColourPickerDlg::OnRandom() 
{
  m_ctlSwatch.m_colour  = (COLORREF) (genrand () * (double) 0x1000000);
	
  m_ctlSwatch.RedrawWindow ();
  ShowName ();

  // select the new colour, if possible

  SelectCurrentColour ();

}


void CColourPickerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
  CSliderCtrl * pSlider = (CSliderCtrl *) pScrollBar;
	
  if (pSlider == &m_ctlRedSlider)
    {

    }

  m_ctlSwatch.m_colour = RGB (m_ctlRedSlider.GetPos (),
                              m_ctlGreenSlider.GetPos (),
                              m_ctlBlueSlider.GetPos ());
  m_ctlSwatch.RedrawWindow ();
  ShowName ();

  // select the new colour, if possible

  SelectCurrentColour ();

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CColourPickerDlg::OnMxpName() 
{
putontoclipboard (GetText (m_ctlMXPname));	
OnOK ();    // accept current selection
}

void CColourPickerDlg::OnVbName() 
{
// we had to put two && at the start so discard the first one
putontoclipboard (GetText (m_ctlVBname).Mid (1));	
OnOK ();    // accept current selection
}

void CColourPickerDlg::OnJscriptName() 
{
putontoclipboard (GetText (m_ctlJscriptName));	
OnOK ();    // accept current selection
}

void CColourPickerDlg::OnLuaName() 
{
putontoclipboard (GetText (m_ctlLuaName));	
OnOK ();    // accept current selection
}

void CColourPickerDlg::OnColourName() 
{
putontoclipboard (GetText (m_ctlColourName));	
OnOK ();    // accept current selection
}

