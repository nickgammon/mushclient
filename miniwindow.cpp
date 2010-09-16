// MiniWindow.cpp : implementation of the CMiniWindow class
//


#include "stdafx.h"
#include "MUSHclient.h"
#include "doc.h"
#include "MUSHview.h"
#include "scripting\errors.h"
#include "color.h"
#include "mainfrm.h"

#define PNG_NO_CONSOLE_IO
#include "png\png.h"

// constructor
CMiniWindow::CMiniWindow ()  :       
          m_oldBitmap (NULL),
          m_Bitmap (NULL),
          m_iWidth (0), m_iHeight (0),
          m_iPosition (0), m_iFlags (0),
          m_iBackgroundColour (0), m_bShow (false),
          m_rect (0, 0, 0, 0),
          m_bTemporarilyHide (false),
          m_last_mouseposition (0, 0),
          m_last_mouse_update (0),
          m_client_mouseposition (0, 0)
  {
  dc.CreateCompatibleDC(NULL);
  dc.SetTextAlign (TA_LEFT | TA_TOP);
  m_tDateInstalled = CTime::GetCurrentTime();  // when miniwindow loaded
  }  // end CMiniWindow::CMiniWindow  (constructor)

CMiniWindow::~CMiniWindow ()  // destructor
  {

  // get rid of old one if any
  if (m_Bitmap)
    {
		dc.SelectObject(m_oldBitmap);    // swap old one back
    m_Bitmap->DeleteObject ();        // delete the one we made
    delete m_Bitmap;
    }


  // delete our fonts
  for (FontMapIterator fit = m_Fonts.begin (); 
       fit != m_Fonts.end ();
       fit++)
         delete fit->second;

  m_Fonts.clear ();

  // delete our images
  for (ImageMapIterator it = m_Images.begin (); 
       it != m_Images.end ();
       it++)
         delete it->second;

  m_Images.clear ();

  // delete our hotspots
  for (HotspotMapIterator hit = m_Hotspots.begin (); 
       hit != m_Hotspots.end ();
       hit++)
         delete hit->second;

  m_Hotspots.clear ();

  }   // end CMiniWindow::~CMiniWindow  (destructor)


// a negative or zero value for the RH side of a rectange is considered offset from the right edge
// eg. -1 is 1 pixel in from right, 0 is the RH edge.
long CMiniWindow::FixRight (const long Right)
  {

  if (Right <= 0) 
    return m_iWidth + Right;

  return Right;
  }  // end CMiniWindow::FixRight

static int BytesPerLine (int nWidth, int nBitsPerPixel)
  {
  return ( (nWidth * nBitsPerPixel + 31) & (~31) ) / 8;
  }

// helper function to make normal/geometric pens
// legacy behaviour is to do what we always did
// however if an endcap or join mask bit is on we juse the ExtCreatePen function
static void MakeAPen (CPen & pen, long PenColour, long PenStyle, long PenWidth) 
  {

  // legacy behaviour
  if ((PenStyle & PS_ENDCAP_MASK) == 0 &&
      (PenStyle & PS_JOIN_MASK) == 0)
    {
    pen.CreatePen (PenStyle, PenWidth, PenColour); 
    }
  else
    {
    LOGBRUSH logbrush;
    logbrush.lbStyle = BS_SOLID;
    logbrush.lbColor = PenColour;
    logbrush.lbHatch = 0;     // not applicable

    pen.Attach (::ExtCreatePen (PenStyle | PS_GEOMETRIC, PenWidth, &logbrush, 0, NULL));
    }
  } // end of MakeAPen

// a negative or zero value for the bottom of a rectange is considered offset from the bottom edge
long CMiniWindow::FixBottom (const long Bottom)
  {

  if (Bottom <= 0) 
    return m_iHeight + Bottom;

  return Bottom;

  }  // end  CMiniWindow::FixBottom


/* positions: 

  0 = strech to output view size 
  1 = stretch with aspect ratio

  2 = strech to owner size 
  3 = stretch with aspect ratio
  
  -- going clockwise here:

  -- top
  4 = top left
  5 = center left-right at top
  6 = top right

  -- rh side
  7 = on right, center top-bottom
  8 = on right, at bottom

  -- bottom
  9 = center left-right at bottom

  -- lh side
  10 = on left, at bottom  
  11 = on left, center top-bottom 

  -- middle
  12 = center all

  13 = tile


  */


// create (or re-create) a mini-window
void CMiniWindow::Create (long Left, long Top, long Width, long Height,
                          short Position, long Flags, 
                          COLORREF BackgroundColour)
  {
 
  m_Location.x           = Left            ;
  m_Location.y           = Top             ;
  m_iWidth               = Width           ;
  m_iHeight              = Height          ;
  m_iPosition            = Position        ;
  m_iFlags               = Flags           ;
  m_iBackgroundColour    = BackgroundColour;

  // get rid of old one if any
  if (m_Bitmap)
    {
		dc.SelectObject(m_oldBitmap);    // swap old one back
    m_Bitmap->DeleteObject ();
    delete m_Bitmap;
    }

  m_Bitmap = new CBitmap;

  //  CreateBitmap with zero-dimensions creates a monochrome bitmap, so force to be at least 1x1
  m_Bitmap->CreateBitmap (MAX (m_iWidth, 1), MAX (m_iHeight, 1), 1, GetDeviceCaps(dc, BITSPIXEL), NULL); 
	m_oldBitmap = dc.SelectObject (m_Bitmap);
	dc.SetWindowOrg(0, 0);

  dc.FillSolidRect (0, 0, m_iWidth, m_iHeight, m_iBackgroundColour);

  m_bShow = false;

  // a newly created window has no hotspots
  if ((Flags & MINIWINDOW_KEEP_HOTSPOTS) == 0)
    DeleteAllHotspots ();

  } // end of MiniWindow::Create


// set/clear the show flag so the window becomes visible
void  CMiniWindow::Show (bool bShow)
  {
  m_bShow = bShow;
  }    // end of CMiniWindow::Show

/*

  Actions:

  1 = FrameRect        ( 1 pixel )
  2 = FillRect
  3 = InvertRect
  4 = 3D Rect    (Colour1 is top and left edge colour, Colour2 is bottom and right edge colour)
  5 = DrawEdge   (draws a 3d-style edge with optional fill)

  Colour1 = style of edge:

  EDGE_RAISED:      // 5     
  EDGE_ETCHED:      // 6     
  EDGE_BUMP:        // 9     
  EDGE_SUNKEN:      // 10    

  Colour2 = where to draw it:

  BF_TOPLEFT      0x3
  BF_TOPRIGHT     0x6
  BF_BOTTOMLEFT   0x9
  BF_BOTTOMRIGHT  0xC
  BF_RECT         0xF

  BF_DIAGONAL     0x0010

  // For diagonal lines, the BF_RECT flags specify the end point of the
  // vector bounded by the rectangle parameter.

  BF_DIAGONAL_ENDTOPLEFT      0x13
  BF_DIAGONAL_ENDTOPRIGHT     0x16
  BF_DIAGONAL_ENDBOTTOMLEFT   0x19
  BF_DIAGONAL_ENDBOTTOMRIGHT  0x1C

  Additional Colour2 flags:

  BF_MIDDLE       0x0800   Fill in the middle 
  BF_SOFT         0x1000   For softer buttons 
  BF_ADJUST       0x2000   Calculate the space left over 
  BF_FLAT         0x4000   For flat rather than 3D borders 
  BF_MONO         0x8000   For monochrome borders 


  6 = Flood Fill Border (fills to border specified by Colour1)

  7 = Flood Fill Surface (fills while on surface specified by Colour1)

*/

// various rectangle operations
long  CMiniWindow::RectOp (short Action, long Left, long Top, long Right, long Bottom, long Colour1, long Colour2)
  {
  switch (Action)

    {
    case 1:       // frame
      {
      CBrush br1;
      br1.CreateSolidBrush (Colour1);    
      dc.FrameRect (CRect (Left, Top, FixRight (Right), FixBottom (Bottom)), &br1);
      break; 
      }

    case 2:       // fill
      {
      CBrush br1;
      br1.CreateSolidBrush (Colour1);    
      dc.FillRect (CRect (Left, Top, FixRight (Right), FixBottom (Bottom)), &br1);
      break; 
      }

    case 3:       // invert
      {
      dc.InvertRect (CRect (Left, Top, FixRight (Right), FixBottom (Bottom)));
      break; 
      }

    case 4:       // 3D rect
      {
      dc.Draw3dRect (CRect (Left, Top, FixRight (Right), FixBottom (Bottom)), Colour1, Colour2);
      break; 
      }

    case 5:       // DrawEdge
      {

      switch (Colour1)   //  nEdge
        {
        // must be one of these 4 flags
        case EDGE_RAISED:      // 5
        case EDGE_ETCHED:      // 6
        case EDGE_BUMP:        // 9
        case EDGE_SUNKEN:      // 10
             break;

        default: return eBadParameter;
        }

      if ((Colour2 & 0xFF) > 0x1F)
        return eBadParameter;

      dc.DrawEdge (CRect (Left, Top, FixRight (Right), FixBottom (Bottom)), Colour1, Colour2);
      break; 
      }
      
    case 6:       // Flood fill border
      {
      dc.FloodFill (Left, Top, Colour1);
      break; 
      }

    case 7:       // Flood fill surface
      {
      dc.ExtFloodFill (Left, Top, Colour1, FLOODFILLSURFACE);
      break; 
      }

    default: return eUnknownOption;

    } // end of switch

  return eOK;

  } // end of CMiniWindow::RectOp 
         
static long ValidatePenStyle (const long PenStyle, const long PenWidth)
  {

  switch (PenStyle & PS_STYLE_MASK)   
    {
      // must be one of these flags
      case PS_SOLID:           // 0  
      case PS_NULL:            // 5   
      case PS_INSIDEFRAME:     // 6
        break;
        
      case PS_DASH:            // 1       /* -------  */     
      case PS_DOT:             // 2       /* .......  */     
      case PS_DASHDOT:         // 3       /* _._._._  */     
      case PS_DASHDOTDOT:      // 4       /* _.._.._  */  
         if (PenWidth > 1) 
           return ePenStyleNotValid;
         break;

    default: return ePenStyleNotValid;
    }

  switch (PenStyle & PS_ENDCAP_MASK)   
    {
      // must be one of these flags
      case PS_ENDCAP_ROUND:   // 0x000  
      case PS_ENDCAP_SQUARE:  // 0x100   
      case PS_ENDCAP_FLAT:    // 0x200
        break;

    default: return ePenStyleNotValid;
    }

  switch (PenStyle & PS_JOIN_MASK)   
    {
      // must be one of these flags
      case PS_JOIN_ROUND:     // 0x0000  
      case PS_JOIN_BEVEL:     // 0x1000   
      case PS_JOIN_MITER:     // 0x2000
        break;

    default: return ePenStyleNotValid;
    }

  return eOK;
  }


long ValidateBrushStyle (const long BrushStyle, 
                          const long PenColour, 
                          const long BrushColour, 
                          CBrush & br)
  {


  LOGBRUSH lb;
  lb.lbColor = PenColour; 

  switch (BrushStyle)   
    {
      // must be one of these flags
      case BS_SOLID:           // 0 
         lb.lbStyle = BS_SOLID;
         lb.lbColor = BrushColour; 
         break;
        
      case BS_NULL:            // 1                          
         lb.lbStyle = BS_NULL;
         break;

      // hatched styles:
      case 2:              
         lb.lbStyle = BS_HATCHED;
         lb.lbHatch = HS_HORIZONTAL;
         break;

      case 3:              
         lb.lbStyle = BS_HATCHED;
         lb.lbHatch = HS_VERTICAL;
         break;

      case 4:             
         lb.lbStyle = BS_HATCHED;
         lb.lbHatch = HS_FDIAGONAL;
         break;

      case 5:              
         lb.lbStyle = BS_HATCHED;
         lb.lbHatch = HS_BDIAGONAL;
         break;

      case 6:              
         lb.lbStyle = BS_HATCHED;
         lb.lbHatch = HS_CROSS;
         break;

      case 7:              
         lb.lbStyle = BS_HATCHED;
         lb.lbHatch = HS_DIAGCROSS;
         break;

         // each byte is a line, so 0xAA would be 10101010   (top line)
         //                         0x55 would be 01010101   (next line)
      case 8:       // fine hatch
        {
        CBitmap bitmap;
        WORD		wBits[] = { 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55 };

        bitmap.CreateBitmap (8, 8, 1, 1, wBits);
        br.CreatePatternBrush (&bitmap);
        return eOK;
        }

      case 9:      // medium hatch
        {
        CBitmap bitmap;
        WORD		wBits[] = { 0x33, 0x33, 0xCC, 0xCC, 0x33, 0x33, 0xCC, 0xCC, };     

        bitmap.CreateBitmap (8, 8, 1, 1, wBits);
        br.CreatePatternBrush (&bitmap);
        return eOK;
        }

      case 10:       // coarse hatch
        {
        CBitmap bitmap;
        WORD		wBits[] = { 0x0F, 0x0F, 0x0F, 0x0F, 0xF0, 0xF0, 0xF0, 0xF0 };

        bitmap.CreateBitmap (8, 8, 1, 1, wBits);
        br.CreatePatternBrush (&bitmap);
        return eOK;
        }

      case 11:       // waves - horizontal
        {
        CBitmap bitmap;
        WORD		wBits[] = { 0xCC, 0x33, 0x00, 0x00, 0xCC, 0x33, 0x00, 0x00  };

        bitmap.CreateBitmap (8, 8, 1, 1, wBits);
        br.CreatePatternBrush (&bitmap);
        return eOK;
        }

      case 12:       // waves - vertical
        {
        CBitmap bitmap;
        WORD		wBits[] = { 0x11, 0x11, 0x22, 0x22, 0x11, 0x11, 0x22, 0x22  };

        bitmap.CreateBitmap (8, 8, 1, 1, wBits);
        br.CreatePatternBrush (&bitmap);
        return eOK;
        }

    default: return eBrushStyleNotValid;
    }

  br.CreateBrushIndirect (&lb);

  return eOK;
  }

/*

  Actions:

  1 = Ellipse
  2 = Rectangle (can have thicker pen style and be filled)
  3 = Round Rectangle
  4 = Chord (a closed figure bounded by the intersection of an ellipse and a line segment)
  5 = Pie (Draws a pie-shaped wedge by drawing an elliptical arc whose center and two endpoints are joined by lines)

  */

// various circle/ellipse/pie operations
long CMiniWindow::CircleOp (short Action, 
                            long Left, long Top, long Right, long Bottom, 
                            long PenColour, long PenStyle, long PenWidth, 
                            long BrushColour, long BrushStyle,
                            long Extra1, long Extra2, long Extra3, long Extra4)
  {
  long iResult = eUnknownOption;

  if (ValidatePenStyle (PenStyle, PenWidth))
    return ePenStyleNotValid;

  // validate and create requested bruch
  CBrush br;

  if (ValidateBrushStyle (BrushStyle, PenColour, BrushColour, br))
    return eBrushStyleNotValid;

  // create requested pen 
  CPen pen;
  MakeAPen (pen, PenColour, PenStyle, PenWidth);
 
  // select into DC
  CPen* oldPen = dc.SelectObject(&pen);
  CBrush* oldBrush = dc.SelectObject(&br);

  if (BrushStyle > 1 && BrushStyle <= 7)
    {
    dc.SetBkColor (BrushColour);      // for hatched brushes this is the background colour
    }
  else
  if (BrushStyle > 7)  // pattern brushes
    {
    dc.SetTextColor (BrushColour);      // for patterned brushes
    dc.SetBkColor (PenColour);      // for hatched brushes and patterned brushes
    }

  if (BrushColour != -1)
    dc.SetBkMode (OPAQUE);
  else
    dc.SetBkMode (TRANSPARENT);

  switch (Action)
                                  
    {
    case 1:       // ellipse
      {
      dc.Ellipse (CRect (Left, Top, FixRight (Right), FixBottom (Bottom)));
      iResult = eOK;
      break; 
      }

    case 2:       // rectangle
      {
      dc.Rectangle (CRect (Left, Top, FixRight (Right), FixBottom (Bottom)));
      iResult = eOK;
      break; 
      }

    case 3:       // round rectangle
      {
      dc.RoundRect (CRect (Left, Top, FixRight (Right), FixBottom (Bottom)), CPoint (Extra1, Extra2));
      iResult = eOK;
      break; 
      }

    case 4:       // chord
      {
      dc.Chord (CRect (Left, Top, FixRight (Right), FixBottom (Bottom)), 
                CPoint (Extra1, Extra2), 
                CPoint (Extra3, Extra4));
      iResult = eOK;
      break; 
      }

    case 5:       // pie
      {
      dc.Pie (CRect (Left, Top, FixRight (Right), FixBottom (Bottom)), 
                CPoint (Extra1, Extra2), 
                CPoint (Extra3, Extra4));
      iResult = eOK;
      break; 
      }

    } // end of switch

  // put things back
  dc.SelectObject (oldPen);
  dc.SelectObject (oldBrush);

  return iResult;

  } // end of CMiniWindow::CircleOp
                    

// add a font to our map of fonts by name (eg. "inventory")
long CMiniWindow::Font (LPCTSTR FontId,        // eg. "inventory"
                        LPCTSTR FontName,      // eg. "Courier New"
                       double Size, 
                       BOOL Bold, BOOL Italic, BOOL Underline, BOOL Strikeout, 
                       short Charset, short PitchAndFamily)
  {

  FontMapIterator it = m_Fonts.find (FontId);

  if (it != m_Fonts.end ())
    {
    delete it->second;         // delete existing font
    m_Fonts.erase (it);
    }

  // no font name, and zero point size requested - delete old font
  if (FontName [0] == 0 && Size == 0.0)
    return eOK;

  CFont * pFont = new CFont;

  int lfHeight = -MulDiv(Size ? (int) Size : 10, dc.GetDeviceCaps(LOGPIXELSY), 72);

  if (pFont->CreateFont (lfHeight, 
				                  0, // int nWidth, 
				                  0, // int nEscapement, 
				                  0, // int nOrientation, 
                          Bold ? FW_BOLD : FW_NORMAL, // int nWeight, 
                          Italic, // BYTE bItalic, 
                          Underline, // BYTE bUnderline, 
                          Strikeout, // BYTE cStrikeOut, 
                          Charset,
                          0, // BYTE nOutPrecision, 
                          0, // BYTE nClipPrecision, 
                          0, // BYTE nQuality, 
                          PitchAndFamily, 
                          FontName) == 0)
    {
    delete pFont;
    return eCannotAddFont;
    }


  m_Fonts [FontId] = pFont;
  return eOK;

  }   // end of CMiniWindow::Font 


static void SetUpVariantLong (VARIANT & tVariant, const long iContents)
  {
  VariantClear (&tVariant);
  tVariant.vt = VT_I4;
  tVariant.lVal = iContents; 
  }   // end of SetUpVariantLong

static void SetUpVariantString (VARIANT & tVariant, const CString & strContents)
  {
  VariantClear (&tVariant);
  tVariant.vt = VT_BSTR;
  tVariant.bstrVal = strContents.AllocSysString (); 
  }   // end of SetUpVariantString

static void SetUpVariantBool (VARIANT & tVariant, const BOOL iContents)
  {
  VariantClear (&tVariant);
  tVariant.vt = VT_BOOL;
  tVariant.boolVal = iContents; 
  }   // end of SetUpVariantBool

static void SetUpVariantDate (VARIANT & tVariant, const COleDateTime iContents)
  {
  VariantClear (&tVariant);
  tVariant.vt = VT_DATE;
  tVariant.date = iContents; 
  }   // end of SetUpVariantDate

// return info about the select font
void CMiniWindow::FontInfo (LPCTSTR FontId, long InfoType, VARIANT & vaResult)
  {
  FontMapIterator it = m_Fonts.find (FontId);

  if (it == m_Fonts.end ())
    return;   // no such font

  CFont* oldFont = dc.SelectObject(it->second);    // select in the requested font
  
  TEXTMETRIC tm;
  dc.GetTextMetrics(&tm);

  CString rString;
  dc.GetTextFace (rString);

  switch (InfoType)
    {

    case  1:  SetUpVariantLong (vaResult, tm.tmHeight);           break;               
    case  2:  SetUpVariantLong (vaResult, tm.tmAscent);           break;               
    case  3:  SetUpVariantLong (vaResult, tm.tmDescent);          break;              
    case  4:  SetUpVariantLong (vaResult, tm.tmInternalLeading);  break;      
    case  5:  SetUpVariantLong (vaResult, tm.tmExternalLeading);  break;      
    case  6:  SetUpVariantLong (vaResult, tm.tmAveCharWidth);     break;         
    case  7:  SetUpVariantLong (vaResult, tm.tmMaxCharWidth);     break;         
    case  8:  SetUpVariantLong (vaResult, tm.tmWeight);           break;               
    case  9:  SetUpVariantLong (vaResult, tm.tmOverhang);         break;             
    case 10:  SetUpVariantLong (vaResult, tm.tmDigitizedAspectX); break;    
    case 11:  SetUpVariantLong (vaResult, tm.tmDigitizedAspectY); break;    
    case 12:  SetUpVariantLong (vaResult, tm.tmFirstChar);        break;           
    case 13:  SetUpVariantLong (vaResult, tm.tmLastChar);         break;            
    case 14:  SetUpVariantLong (vaResult, tm.tmDefaultChar);      break;         
    case 15:  SetUpVariantLong (vaResult, tm.tmBreakChar);        break;           
    case 16:  SetUpVariantLong (vaResult, tm.tmItalic);           break;              
    case 17:  SetUpVariantLong (vaResult, tm.tmUnderlined);       break;          
    case 18:  SetUpVariantLong (vaResult, tm.tmStruckOut);        break;           
    case 19:  SetUpVariantLong (vaResult, tm.tmPitchAndFamily);   break;      
    case 20:  SetUpVariantLong (vaResult, tm.tmCharSet);          break;             
    case 21:  SetUpVariantString (vaResult, rString);             break;             

    default:
      vaResult.vt = VT_NULL;
      break;

    } // end of switch

  dc.SelectObject(oldFont);
  }  // end of CMiniWindow::FontInfo

// return list of fonts we installed
void CMiniWindow::FontList (VARIANT & vaResult)
  {
  COleSafeArray sa;   // for array list

  long iCount = 0;
  
  // put the arrays into the array
  if (!m_Fonts.empty ())    // cannot create empty dimension
    {
    sa.CreateOneDim (VT_VARIANT, m_Fonts.size ());

    for (FontMapIterator it = m_Fonts.begin (); 
         it != m_Fonts.end ();
         it++)
           {
            // the array must be a bloody array of variants, or VBscript kicks up
            COleVariant v (it->first.c_str ());
            sa.PutElement (&iCount, &v);
            iCount++;
           }

    } // end of having at least one

	vaResult = sa.Detach ();


  } // end of  CMiniWindow::FontList



//helper function to calculate length of UTF8 string
static long CalculateUTF8length (LPCTSTR Text, size_t length)
  {
 
  int iBad = _pcre_valid_utf8 ((unsigned char  *) Text, length);
  if (iBad >= 0)
    return -1;

    // string is OK, calculate its length

  int i = 0;   // length

  // this algorithm assumes the UTF-8 is OK, based on the earlier check

  for (register const unsigned char *p = (const unsigned char *) Text ; 
       length-- > 0; 
       i++)
    {          
    register int ab;    // additional bytes
    register int c = *p++;  // this byte

    if (c < 128)
      continue;     // zero additional bytes

    ab = _pcre_utf8_table4 [c & 0x3f];  /* Number of additional bytes */

    length -= ab;  // we know string is valid already, so just skip the additional bytes (ab)
    p += ab;

    }

  return i;

  }   // end of CalculateUTF8length

// output text, ordinary or UTF8 - returns length of text
long CMiniWindow::Text (LPCTSTR FontId,  // which previously-created font
                        LPCTSTR Text,    // what to say
                        long Left, long Top, long Right, long Bottom, // where to say it
                        long Colour,       // colour to show it in
                        BOOL Unicode)      // true if UTF8

  {
  FontMapIterator it = m_Fonts.find (FontId);

  if (it == m_Fonts.end ())
    return -2;

  size_t length = strlen (Text);
  long utf8_length = 0;

  // give up if no text
  if (length <= 0)
    return 0;

  // quick sanity check on our UTF8 stuff
  if (Unicode)
    {
    utf8_length = CalculateUTF8length (Text, length);
    if (utf8_length < 0)
      return -3;    // ohno!
    }


  CFont* oldFont = dc.SelectObject(it->second);    // select in the requested font

  CSize textsize;

  dc.SetTextColor (Colour);  
  dc.SetBkMode (TRANSPARENT);

  if (Unicode)
    {
    vector<WCHAR> v (utf8_length);    // get correct size vector
    int iUnicodeCharacters = MultiByteToWideChar (CP_UTF8, 0, 
                              Text, length,            // input
                              &v [0], utf8_length);    // output

    ExtTextOutW (dc.m_hDC, Left, Top, ETO_CLIPPED, CRect (Left, Top, FixRight (Right), FixBottom (Bottom) ),
                  &v [0], iUnicodeCharacters, NULL);

    // now calculate width of Unicode pixels
    GetTextExtentPoint32W(
        dc.m_hDC,             // handle to device context
        &v [0],               // pointer to text string
        iUnicodeCharacters,   // number of characters in string
        &textsize             // pointer to structure for string size
      );

    }
  else
    {
    dc.ExtTextOut (Left, Top, ETO_CLIPPED, CRect (Left, Top, FixRight (Right), FixBottom (Bottom)), 
                  Text, length, NULL);
    textsize = dc.GetTextExtent (Text, length);

    }


  dc.SelectObject(oldFont);

  return min (textsize.cx, FixRight (Right) - Left);  // if clipped, length is width of rectangle
  } // end of CMiniWindow::Text 


// measure text, ordinary or UTF8
long CMiniWindow::TextWidth (LPCTSTR FontId,  // which previously-created font
                             LPCTSTR Text,    // what to measure
                             BOOL Unicode)    // true if UTF8
  {

  FontMapIterator it = m_Fonts.find (FontId);

  if (it == m_Fonts.end ())
    return -2;

  size_t length = strlen (Text);
  long utf8_length = 0;

  // give up if no text
  if (length <= 0)
    return 0;

  // quick sanity check on our UTF8 stuff
  if (Unicode)
    {
    utf8_length = CalculateUTF8length (Text, length);
    if (utf8_length < 0)
      return -3;    // ohno!
    }


  CFont* oldFont = dc.SelectObject(it->second);    // select in the requested font

  CSize textsize;

  if (Unicode)
    {
    vector<WCHAR> v (utf8_length);    // get correct size vector
    int iUnicodeCharacters = MultiByteToWideChar (CP_UTF8, 0,     
                              Text, length,           // input
                              &v [0], utf8_length);   // output

    // now calculate width of Unicode pixels
    GetTextExtentPoint32W(
        dc.m_hDC,           // handle to device context
        &v [0],             // pointer to text string
        iUnicodeCharacters, // number of characters in string
        &textsize           // pointer to structure for string size
      );

    }
  else
    textsize = dc.GetTextExtent (Text, length);


  dc.SelectObject(oldFont);

  return textsize.cx;

  }  // end of  CMiniWindow::TextWidth


// draws a straight line
long CMiniWindow::Line (long x1, long y1, long x2, long y2, 
                        long PenColour, long PenStyle, long PenWidth)
  {

  if (ValidatePenStyle (PenStyle, PenWidth))
    return ePenStyleNotValid;

  dc.SetBkMode (TRANSPARENT);

  // create requested pen
  CPen pen;
  MakeAPen (pen, PenColour, PenStyle, PenWidth);

  CPen* oldPen = dc.SelectObject(&pen);

  dc.MoveTo (x1, y1);
  dc.LineTo (x2, y2);    // note NOT: FixRight (x2), FixBottom (y2) (as at version 4.38) 

  // put things back
  dc.SelectObject (oldPen);

  return eOK;

  }  // end of CMiniWindow::Line


// draws an arc
long CMiniWindow::Arc (long Left, long Top, long Right, long Bottom, 
                        long x1, long y1, 
                        long x2, long y2, 
                        long PenColour, long PenStyle, long PenWidth)
  {

  if (ValidatePenStyle (PenStyle, PenWidth))
    return ePenStyleNotValid;

  dc.SetBkMode (TRANSPARENT);

  // create requested pen
  CPen pen;
  MakeAPen (pen, PenColour, PenStyle, PenWidth);

  CPen* oldPen = dc.SelectObject(&pen);

  dc.Arc(Left, Top, FixRight (Right), FixBottom (Bottom), 
          x1, y1, // from
          FixRight (x2), FixBottom (y2)); // to

  // put things back
  dc.SelectObject (oldPen);

  return eOK;

  }  // end of CMiniWindow::Arc


// return info about the window
void CMiniWindow::Info (long InfoType, VARIANT & vaResult)
  {

  switch (InfoType)
    {
    case  1:  SetUpVariantLong    (vaResult, m_Location.x);        break; // left 
    case  2:  SetUpVariantLong    (vaResult, m_Location.y);        break; // top
    case  3:  SetUpVariantLong    (vaResult, m_iWidth);            break; // width
    case  4:  SetUpVariantLong    (vaResult, m_iHeight);           break; // height
    case  5:  SetUpVariantBool    (vaResult, m_bShow);             break; // show flag
    case  6:  SetUpVariantBool    (vaResult, m_bTemporarilyHide);  break; // is it hidden right now?
    case  7:  SetUpVariantLong    (vaResult, m_iPosition);         break; // layout mode
    case  8:  SetUpVariantLong    (vaResult, m_iFlags);            break; // flags
    case  9:  SetUpVariantLong    (vaResult, m_iBackgroundColour); break; // background colour

    case 10:  SetUpVariantLong    (vaResult, m_rect.left);         break; // where it is right now
    case 11:  SetUpVariantLong    (vaResult, m_rect.top);          break; //       "
    case 12:  SetUpVariantLong    (vaResult, m_rect.right);        break; //       "
    case 13:  SetUpVariantLong    (vaResult, m_rect.bottom);       break; //       "

    case 14:  SetUpVariantLong    (vaResult, m_last_mouseposition.x);  break; //  last mouse x position
    case 15:  SetUpVariantLong    (vaResult, m_last_mouseposition.y);  break; //  last mouse y position

    case 16:  SetUpVariantLong    (vaResult, m_last_mouse_update);  break;    //  last position update count

    case 17:  SetUpVariantLong    (vaResult, m_client_mouseposition.x); break; //  last client mouse x position
    case 18:  SetUpVariantLong    (vaResult, m_client_mouseposition.y); break; //  last client mouse y position

    case 19:  SetUpVariantString  (vaResult, m_sMouseOverHotspot.c_str ()); break; // mouse-over hotspot
    case 20:  SetUpVariantString  (vaResult, m_sMouseDownHotspot.c_str ()); break; // mouse-down hotspot
    case 21:  SetUpVariantDate    (vaResult, COleDateTime (m_tDateInstalled.GetTime ()));  break;

    default:
      vaResult.vt = VT_NULL;
      break;

    } // end of switch

  }  // end of CMiniWindow::Info


// loads an image file, ready for drawing into window
long CMiniWindow::LoadImage (LPCTSTR ImageId, LPCTSTR FileName)
  {
  ImageMapIterator it = m_Images.find (ImageId);

  if (it != m_Images.end ())
    {
    delete it->second;         // delete existing image
    m_Images.erase (it);
    }

  CString strFileName = FileName;

  strFileName.TrimLeft ();
  strFileName.TrimRight ();

  // no file name means get rid of image
  if (strFileName.IsEmpty ())
    return eOK;

  // have to be long enough to have x.bmp
  if (strFileName.GetLength () < 5)
    return eBadParameter;

  // handle PNG files separately
  if (strFileName.Right (4).CompareNoCase (".png") == 0)
    return LoadPngImage (ImageId, FileName);

  // must be .bmp or .png file
  if (strFileName.Right (4).CompareNoCase (".bmp") != 0)
    return eBadParameter;


  HBITMAP hBmp = (HBITMAP)::LoadImage(
                  NULL,
                  FileName,
                  IMAGE_BITMAP,
                  0,
                  0,
                  LR_LOADFROMFILE|LR_CREATEDIBSECTION
                  );

   if (hBmp) 
     {
      CBitmap * pImage = new CBitmap;
      pImage->Attach (hBmp);
      m_Images [ImageId] = pImage;
    	return eOK;
     }  // end of having a bitmap loaded


   if (GetLastError () == 2)
     return eFileNotFound;

   return eUnableToLoadImage;


  } // end of CMiniWindow::LoadImage


// loads an image from memory, ready for drawing into window
long CMiniWindow::LoadImageMemory(LPCTSTR ImageId, 
                                 unsigned char * Buffer, 
                                 const size_t Length,
                                 const bool bAlpha)
  {
  ImageMapIterator it = m_Images.find (ImageId);

  if (it != m_Images.end ())
    {
    delete it->second;         // delete existing image
    m_Images.erase (it);
    }

  HBITMAP hbmp;
  
  long result =  LoadPngMemory (Buffer, Length, hbmp, bAlpha);

  if (result != eOK) 
    return result;

  // make bitmap to add to images list
  CBitmap * pImage = new CBitmap;
  pImage->Attach (hbmp);
  m_Images [ImageId] = pImage;

  return eOK;

  } // end of CMiniWindow::LoadImageMemory


static void user_error_fn(png_structp png_ptr,
        png_const_charp error_msg)
  {
//  AfxMessageBox (error_msg);
  }

static void user_warning_fn(png_structp png_ptr,
    png_const_charp warning_msg)
  {
//  AfxMessageBox (warning_msg);

  }


long CMiniWindow::LoadPngImage (LPCTSTR ImageId, LPCTSTR FileName)
  {

  HBITMAP hbmp;
  
  long result =  LoadPng (FileName, hbmp);

  if (result != eOK) 
    return result;

  // make bitmap to add to images list
  CBitmap * pImage = new CBitmap;
  pImage->Attach (hbmp);
  m_Images [ImageId] = pImage;

  return eOK;
  } // end of CMiniWindow::LoadPngImage



// saves an image file
long CMiniWindow::Write (LPCTSTR FileName)
  {

  CString strFileName = FileName;

  strFileName.TrimLeft ();
  strFileName.TrimRight ();

  if (strFileName.IsEmpty ())
    return eNoNameSpecified;

  // have to be long enough to have x.bmp
  if (strFileName.GetLength () < 5)
    return eBadParameter;

  // must be .bmp or .png file
  if (strFileName.Right (4).CompareNoCase (".bmp") != 0 && 
      strFileName.Right (4).CompareNoCase (".png") != 0)
    return eBadParameter;

  // get window data
  CDC gDC;
  gDC.CreateCompatibleDC(&dc);
  CBitmap gbmp;

  BITMAPINFO bmi;
  ZeroMemory (&bmi, sizeof bmi);

  bmi.bmiHeader.biSize = sizeof bmi;
  bmi.bmiHeader.biWidth =          m_iWidth;       
  bmi.bmiHeader.biHeight =         m_iHeight;
  bmi.bmiHeader.biPlanes =         1;
  bmi.bmiHeader.biBitCount =       24;
  bmi.bmiHeader.biCompression =    BI_RGB;
  bmi.bmiHeader.biSizeImage =      m_iHeight * BytesPerLine (m_iWidth, 24);

  unsigned char * pA = NULL;

  HBITMAP hbmG = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**) &pA, NULL, 0);

  HBITMAP hOldAbmp = (HBITMAP) SelectObject(gDC.m_hDC, hbmG);

  // copy image from miniwindow to bitmap
  gDC.BitBlt (0, 0, m_iWidth, m_iHeight, &dc, 0, 0, SRCCOPY);  

  long iReturn = eOK;

    // must be .bmp or .png file
  if (strFileName.Right (4).CompareNoCase (".png") == 0)
    iReturn = WritePng (FileName, &bmi, pA);
  else
    {     // write BMP file

    // create requested file
	  CFile file;
	  if( !file.Open (FileName, CFile::modeWrite | CFile::modeCreate))
		  return eCouldNotOpenFile;

    BITMAPFILEHEADER	hdr;
    ZeroMemory (&hdr, sizeof hdr);

	  // Fill in the fields of the file header 
	  hdr.bfType		= ((WORD) ('M' << 8) | 'B');	// is always "BM"
	  hdr.bfOffBits	= sizeof hdr  + sizeof bmi.bmiHeader;
	  hdr.bfSize		=  hdr.bfOffBits + bmi.bmiHeader.biSizeImage;
 
    try
      { 
	    // Write the file header 
	    file.Write( &hdr, sizeof hdr);
  
	    // Write the bitmap info header 
	    file.Write( &bmi.bmiHeader, sizeof bmi.bmiHeader);

	    // Write the bits 
	    file.Write( pA,  bmi.bmiHeader.biSizeImage);
      }
    catch (CFileException * e)
      {
      e->Delete ();
      iReturn = eLogFileBadWrite;
      } // end of catching a file exception

    }   // end of writing BMP file

  // finished with bitmap
  SelectObject(gDC.m_hDC, hOldAbmp);
  DeleteObject (hbmG);

  return iReturn;

  } // end of CMiniWindow::Write

long CMiniWindow::WritePng (LPCTSTR FileName, const BITMAPINFO * bmi, unsigned char * pData)
  {
  // open file
  FILE *fp = fopen(FileName, "wb");
  if (!fp)
      return (eCouldNotOpenFile);

  // create PNG structure
  png_structp png_ptr = png_create_write_struct
       (PNG_LIBPNG_VER_STRING, NULL,
        user_error_fn, user_warning_fn);

  if (!png_ptr)
    {
    fclose (fp);
    return eLogFileBadWrite;
    }

  // create info structure
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    {
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    fclose (fp);
    return eLogFileBadWrite;
    }
  

  // if png fails it will longjmp back to here, so we destroy the structure,
  // close the file, and wrap up
  if (setjmp(png_jmpbuf(png_ptr)))
    {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose (fp);
    return eLogFileBadWrite;
    }

  // initialize IO
  png_init_io (png_ptr, fp);

  // tell PNG the file format
  png_set_IHDR  (png_ptr, 
                 info_ptr, 
                 bmi->bmiHeader.biWidth, 
                 bmi->bmiHeader.biHeight,
                 8,        // bits per pixel
                 PNG_COLOR_TYPE_RGB, 
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, 
                 PNG_FILTER_TYPE_DEFAULT);

  // set up pointer to each row of data
  vector<png_byte *> row_pointers;
  row_pointers.resize (bmi->bmiHeader.biHeight);

  long bpl = BytesPerLine (bmi->bmiHeader.biWidth, 24);

  unsigned long row;
  unsigned char * p = pData;

  // have to reverse row order
  for (row = 0; row < info_ptr->height; row++, p += bpl)
     row_pointers [bmi->bmiHeader.biHeight - row - 1] = p;

  // tell png where our pixel data is
  png_set_rows (png_ptr, info_ptr, &row_pointers [0]);

  // write out the image data
  png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_BGR, NULL);

  // wrap up
  png_write_end (png_ptr, info_ptr);
  
  // done with data
  png_destroy_write_struct (&png_ptr, &info_ptr);

  // done with file
  fclose (fp);

  return eOK;
  } // end of CMiniWindow::WritePng


/*

  for monitors with 256 colours or less, consider:

HDRAWDIB hdd = DrawDibOpen(); 
DrawDibDraw(hdd, hPaintDC,0,0,BITMAP_WIDTH,BITMAP_HEIGHT,&m_BitmapInfo,m_pBits,0,0,BITMAP_WIDTH,BITMAP_HEIGHT,DDF_HALFTONE);
DrawDibClose(hdd);

  */


// draw a previously-loaded image into the window
long CMiniWindow::DrawImage(LPCTSTR ImageId, 
               long Left, long Top, long Right, long Bottom, 
               short Mode,
               long SrcLeft, long SrcTop, long SrcRight, long SrcBottom)
  {

  ImageMapIterator it = m_Images.find (ImageId);

  if (it == m_Images.end ())
    return eImageNotInstalled;

  CBitmap * bitmap = it->second;

  dc.SetBkMode (TRANSPARENT);

  dc.SetStretchBltMode (HALFTONE);  // looks better when squashed
  SetBrushOrgEx(dc.m_hDC, 0, 0, NULL);  // as recommended after  SetStretchBltMode

  BITMAP  bi;
  bitmap->GetBitmap(&bi);

  CDC bmDC;
  bmDC.CreateCompatibleDC(&dc);
  CBitmap *pOldbmp = bmDC.SelectObject(bitmap);

  // adjust so that -1 means 1 from right
  if (SrcRight <= 0) 
    SrcRight = bi.bmWidth + SrcRight;

  if (SrcBottom <= 0) 
    SrcBottom =  bi.bmHeight + SrcBottom;

  // calculate size of desired rectangle
  long iWidth = SrcRight - SrcLeft;
  long iHeight = SrcBottom - SrcTop;

  if (iWidth > 0 && iHeight > 0)   // sanity check
    switch (Mode)
      {
      case 1: dc.BitBlt (Left, Top, iWidth, iHeight, &bmDC, SrcLeft, SrcTop, SRCCOPY);  
              break;      // straight copy

      case 2: dc.StretchBlt (Left, Top, FixRight (Right) - Left, FixBottom (Bottom) - Top, &bmDC, 
                             SrcLeft, SrcTop, SrcRight - SrcLeft, SrcBottom - SrcTop, SRCCOPY);
              break;      // stretch

      case 3:        // transparency, och!
        {
        COLORREF crOldBack = dc.SetBkColor (RGB (255, 255, 255));    // white
	      COLORREF crOldText = dc.SetTextColor (RGB (0, 0, 0));        // black
	      CDC dcTrans;   // transparency mask


	      // Create a memory dc for the mask
	      dcTrans.CreateCompatibleDC(&dc);

	      // Create the mask bitmap for the subset of the main image
	      CBitmap bitmapTrans;
	      bitmapTrans.CreateBitmap(iWidth, iHeight, 1, 1, NULL);

	      // Select the mask bitmap into the appropriate dc
	      CBitmap* pOldBitmapTrans = dcTrans.SelectObject(&bitmapTrans);

        // Our transparent pixel will be at 0,0 (top left corner) of original image (not subimage)
        COLORREF crOldBackground = bmDC.SetBkColor (::GetPixel (bmDC, 0, 0));

	      // Build mask based on transparent colour at location 0, 0
	      dcTrans.BitBlt (0, 0, iWidth, iHeight, &bmDC, SrcLeft, SrcTop, SRCCOPY);

	      // Do the work 
	      dc.BitBlt (Left, Top, iWidth, iHeight, &bmDC, SrcLeft, SrcTop, SRCINVERT);
	      dc.BitBlt (Left, Top, iWidth, iHeight, &dcTrans, 0, 0, SRCAND);
	      dc.BitBlt (Left, Top, iWidth, iHeight, &bmDC, SrcLeft, SrcTop, SRCINVERT);

	      // Restore settings
	      dcTrans.SelectObject(pOldBitmapTrans);
	      dc.SetBkColor(crOldBack);
	      dc.SetTextColor(crOldText);
        bmDC.SetBkColor(crOldBackground);
        }
        break;

      default: return eBadParameter;
      } // end of switch

  bmDC.SelectObject(pOldbmp);

  return eOK;
  }  // end of CMiniWindow::DrawImage


// return list of images
void CMiniWindow::ImageList(VARIANT & vaResult)
  {
  COleSafeArray sa;   // for array list

  long iCount = 0;
  
  // put the arrays into the array
  if (!m_Images.empty ())    // cannot create empty dimension
    {
    sa.CreateOneDim (VT_VARIANT, m_Images.size ());

    for (ImageMapIterator it = m_Images.begin (); 
         it != m_Images.end ();
         it++)
           {
            // the array must be a bloody array of variants, or VBscript kicks up
            COleVariant v (it->first.c_str ());
            sa.PutElement (&iCount, &v);
            iCount++;
           }

    } // end of having at least one

	vaResult = sa.Detach ();

  }   // end of CMiniWindow::ImageList



// return info about the selected image
void CMiniWindow::ImageInfo (LPCTSTR ImageId, long InfoType, VARIANT & vaResult)
  {
  ImageMapIterator it = m_Images.find (ImageId);

  if (it == m_Images.end ())
    return;   // no such Image

  CBitmap * bitmap = it->second;

  BITMAP  bi;
  bitmap->GetBitmap(&bi);

  switch (InfoType)
    {

    case  1:  SetUpVariantLong (vaResult, bi.bmType);      break;
    case  2:  SetUpVariantLong (vaResult, bi.bmWidth);     break;
    case  3:  SetUpVariantLong (vaResult, bi.bmHeight);    break;
    case  4:  SetUpVariantLong (vaResult, bi.bmWidthBytes);break;
    case  5:  SetUpVariantLong (vaResult, bi.bmPlanes);    break;
    case  6:  SetUpVariantLong (vaResult, bi.bmBitsPixel); break;

     default:
      vaResult.vt = VT_NULL;
      break;

    } // end of switch

  }  // end of CMiniWindow::ImageInfo


// draw bezier curves
long CMiniWindow::Bezier(LPCTSTR Points, long PenColour, long PenStyle, long PenWidth)
  {

  if (ValidatePenStyle (PenStyle, PenWidth))
    return ePenStyleNotValid;

  vector<string> v;

  StringToVector (Points, v, ",");

  int iCount = v.size ();

  // must have at least a start point (2 items), plus one extra (3 points)
  if (iCount < 8)
    return eInvalidNumberOfPoints;

  // and has to be 2 more than 6 items  (1 more than 3n points)
  if ((iCount % 6) != 2)
    return eInvalidNumberOfPoints;

  iCount = iCount / 2;  // number of points

  vector<POINT> points (iCount);

  int iCurrent = 0;

  for (vector<string>::const_iterator i = v.begin (); i != v.end (); i++)
    {
    if (!IsStringNumber (*i, true))
      return eInvalidPoint;
    points [iCurrent].x = atol (i->c_str ());
    i++;  // we know this is safe becaue of earlier check (we must have pairs of numbers)
    if (!IsStringNumber (*i, true))
      return eInvalidPoint;
    points [iCurrent].y = atol (i->c_str ());
    iCurrent++;  // onto next point
    }

  dc.SetBkMode (TRANSPARENT);

  // create requested pen
  CPen pen;
  MakeAPen (pen, PenColour, PenStyle, PenWidth);
  CPen* oldPen = dc.SelectObject(&pen);

  dc.PolyBezier(&points [0], iCount); 

  // put things back
  dc.SelectObject (oldPen);

  return eOK;


  }   // end of CMiniWindow::Bezier


// draw a polygon (straight lines)
long CMiniWindow::Polygon(LPCTSTR Points, 
                         long PenColour, short PenStyle, long PenWidth, 
                         long BrushColour, long BrushStyle, 
                         BOOL Close,
                         BOOL Winding)
  {

  if (ValidatePenStyle (PenStyle, PenWidth))
    return ePenStyleNotValid;

  // validate and create requested bruch
  CBrush br;

  if (ValidateBrushStyle (BrushStyle, PenColour, BrushColour, br))
    return eBrushStyleNotValid;

  vector<string> v;

  StringToVector (Points, v, ",");

  int iCount = v.size ();

  // must have at least a start point (2 items), plus one extra (1 point)
  if (iCount < 4)
    return eInvalidNumberOfPoints;

  // it has to be divisible by 2
  if ((iCount % 2) != 0)
    return eInvalidNumberOfPoints;

  iCount = iCount / 2;  // number of points

  vector<POINT> points (iCount);

  int iCurrent = 0;

  for (vector<string>::const_iterator i = v.begin (); i != v.end (); i++)
    {
    if (!IsStringNumber (*i, true))
      return eInvalidPoint;
    points [iCurrent].x = atol (i->c_str ());
    i++;  // we know this is safe becaue of earlier check (we must have pairs of numbers)
    if (!IsStringNumber (*i, true))
      return eInvalidPoint;
    points [iCurrent].y = atol (i->c_str ());
    iCurrent++;  // onto next point
    }

  // create requested pen 
  CPen pen;
  MakeAPen (pen, PenColour, PenStyle, PenWidth);

  // select pen and brush into device context
  CPen* oldPen = dc.SelectObject(&pen);
  CBrush* oldBrush = dc.SelectObject(&br);

  dc.SetPolyFillMode (Winding ? WINDING : ALTERNATE);

  if (BrushStyle > 1 && BrushStyle <= 7)
    {
    dc.SetBkColor (BrushColour);      // for hatched brushes this is the background colour
    }
  else
  if (BrushStyle > 7)  // pattern brushes
    {
    dc.SetTextColor (BrushColour);      // for patterned brushes
    dc.SetBkColor (PenColour);      // for hatched brushes and patterned brushes
    }

  if (BrushColour != -1)
    dc.SetBkMode (OPAQUE);
  else
    dc.SetBkMode (TRANSPARENT);

  if (Close)
    dc.Polygon(&points [0], iCount); 
  else
    dc.Polyline(&points [0], iCount); 

  // put things back
  dc.SelectObject (oldPen);
  dc.SelectObject (oldBrush);

  return eOK;

  }   // end of CMiniWindow::Polygon
                           
// reposition window
long CMiniWindow::Position(long Left, long Top, 
                           short Position, 
                           long Flags) 
  {
  m_Location.x           = Left     ;
  m_Location.y           = Top      ;
  m_iPosition            = Position ;
  m_iFlags               = Flags    ;

  return eOK;

  } // end of  CMiniWindow::Position


/*

  Cursor values:

  0:  arrow                                
  1:  hand                                 
  2:  I-beam                               
  3:  + symbol                           
  4:  wait (hour-glass)                    
  5:  up arrow                             
  6:  arrow nw-se                          
  7:  arrow ne-sw                          
  8:  arrow e-w                            
  9:  arrow n-s                            
 10:  arrow - all ways                     
 11:  (X) no, no, I won't do that, but ... 
 12:  help  (? symbol)                     


  Flags:

  0x01 : report all mouseovers, not just when first entering the hotspot

  */

// add a hotspot for handling mouse-over, mouse up/down events
long CMiniWindow::AddHotspot(CMUSHclientDoc * pDoc,
                             LPCTSTR HotspotId, 
                             string sPluginID,
                             long Left, long Top, long Right, long Bottom, 
                             LPCTSTR MouseOver, 
                             LPCTSTR CancelMouseOver, 
                             LPCTSTR MouseDown, 
                             LPCTSTR CancelMouseDown, 
                             LPCTSTR MouseUp, 
                             LPCTSTR TooltipText,
                             long Cursor, 
                             long Flags)
  {

  if (strlen (MouseOver) > 0 && CheckLabel (MouseOver, true))
    return eInvalidObjectLabel;
  if (strlen (CancelMouseOver) > 0 && CheckLabel (CancelMouseOver, true))
    return eInvalidObjectLabel;
  if (strlen (MouseDown) > 0 && CheckLabel (MouseDown, true))
    return eInvalidObjectLabel;
  if (strlen (CancelMouseDown) > 0 && CheckLabel (CancelMouseDown, true))
    return eInvalidObjectLabel;
  if (strlen (MouseUp) > 0 && CheckLabel (MouseUp, true))
    return eInvalidObjectLabel;

  // can't switch plugins here :)
  if (!m_sCallbackPlugin.empty () && m_sCallbackPlugin != sPluginID)
    return eHotspotPluginChanged;

  m_sCallbackPlugin = sPluginID;

  HotspotMapIterator it = m_Hotspots.find (HotspotId);

  if (it != m_Hotspots.end ())
    {
    delete it->second;         // delete existing hotspot
    m_Hotspots.erase (it);
    if (m_sMouseOverHotspot == HotspotId)
      m_sMouseOverHotspot.erase ();

    if (m_sMouseDownHotspot == HotspotId)
      m_sMouseDownHotspot.erase ();

    }

  CHotspot * pHotspot = new CHotspot;

  pHotspot->m_rect              = CRect (Left, Top, FixRight (Right), FixBottom (Bottom));
  pHotspot->m_sMouseOver        = MouseOver;
  pHotspot->m_sCancelMouseOver  = CancelMouseOver;
  pHotspot->m_sMouseDown        = MouseDown;
  pHotspot->m_sCancelMouseDown  = CancelMouseDown;
  pHotspot->m_sMouseUp          = MouseUp;
  pHotspot->m_sTooltipText      = TooltipText;
  pHotspot->m_Cursor            = Cursor;
  pHotspot->m_Flags             = Flags;

  // if not in a plugin, look in main world for hotspot callbacks, and remember the dispatch ID
  if (sPluginID.empty ())
    {
    CString strErrorMessage;

    pHotspot->m_dispid_MouseOver        = pDoc->GetProcedureDispid (MouseOver, "mouse over", "", strErrorMessage);
    pHotspot->m_dispid_CancelMouseOver  = pDoc->GetProcedureDispid (CancelMouseOver, "cancel mouse over", "", strErrorMessage);
    pHotspot->m_dispid_MouseDown        = pDoc->GetProcedureDispid (MouseDown, "mouse down", "", strErrorMessage);
    pHotspot->m_dispid_CancelMouseDown  = pDoc->GetProcedureDispid (CancelMouseDown, "cancel mouse down", "", strErrorMessage);
    pHotspot->m_dispid_MouseUp          = pDoc->GetProcedureDispid (MouseUp, "mouse up", "", strErrorMessage);
    }
    
  m_Hotspots [HotspotId] = pHotspot;

  return eOK;
  }    // end of CMiniWindow::AddHotspot

// remove a previously-installed hotspot
long CMiniWindow::DeleteHotspot(LPCTSTR HotspotId)
  {

  HotspotMapIterator it = m_Hotspots.find (HotspotId);

  if (it == m_Hotspots.end ())
    return eHotspotNotInstalled;   // no such hotspot

  delete it->second;

  m_Hotspots.erase (it);

  if (m_sMouseOverHotspot == HotspotId)
    m_sMouseOverHotspot.erase ();

  if (m_sMouseDownHotspot == HotspotId)
    m_sMouseDownHotspot.erase ();

  if (m_Hotspots.empty ())
    m_sCallbackPlugin.erase ();

  return eOK;
  }    // end of CMiniWindow::DeleteHotspot

// return list of all hotspots in this miniwindow
void CMiniWindow::HotspotList(VARIANT & vaResult)
  {
  COleSafeArray sa;   // for array list

  long iCount = 0;
  
  // put the arrays into the array
  if (!m_Hotspots.empty ())    // cannot create empty dimension
    {
    sa.CreateOneDim (VT_VARIANT, m_Hotspots.size ());

    for (HotspotMapIterator it = m_Hotspots.begin (); 
         it != m_Hotspots.end ();
         it++)
           {
            // the array must be a bloody array of variants, or VBscript kicks up
            COleVariant v (it->first.c_str ());
            sa.PutElement (&iCount, &v);
            iCount++;
           }

    } // end of having at least one

	vaResult = sa.Detach ();

  }    // end of CMiniWindow::HotspotList

// delete all hotspots
long CMiniWindow::DeleteAllHotspots()
  {
  // delete our hotspots
  for (HotspotMapIterator hit = m_Hotspots.begin (); 
       hit != m_Hotspots.end ();
       hit++)
         delete hit->second;

  m_Hotspots.clear ();
  m_sMouseOverHotspot.erase ();
  m_sMouseDownHotspot.erase ();
  m_sCallbackPlugin.erase ();
  return eOK;
  }    // end of CMiniWindow::DeleteAllHotspots

// get information about a hotspot
void CMiniWindow::HotspotInfo(LPCTSTR HotspotId, long InfoType, VARIANT & vaResult)
  {

  HotspotMapIterator it = m_Hotspots.find (HotspotId);

  if (it == m_Hotspots.end ())
    return;   // no such hotspot

  CHotspot * pHotspot = it->second;

  switch (InfoType)
    {

    case  1:  SetUpVariantLong   (vaResult, pHotspot->m_rect.left);                 break; // left 
    case  2:  SetUpVariantLong   (vaResult, pHotspot->m_rect.top);                  break; // top
    case  3:  SetUpVariantLong   (vaResult, pHotspot->m_rect.right);                break; // right
    case  4:  SetUpVariantLong   (vaResult, pHotspot->m_rect.bottom);               break; // bottom
    case  5:  SetUpVariantString (vaResult, pHotspot->m_sMouseOver.c_str ());       break;             
    case  6:  SetUpVariantString (vaResult, pHotspot->m_sCancelMouseOver.c_str ()); break;             
    case  7:  SetUpVariantString (vaResult, pHotspot->m_sMouseDown.c_str ());       break;             
    case  8:  SetUpVariantString (vaResult, pHotspot->m_sCancelMouseDown.c_str ()); break;             
    case  9:  SetUpVariantString (vaResult, pHotspot->m_sMouseUp.c_str ());         break;             
    case 10:  SetUpVariantString (vaResult, pHotspot->m_sTooltipText.c_str ());     break;             
    case 11:  SetUpVariantLong   (vaResult, pHotspot->m_Cursor);                    break; // cursor code
    case 12:  SetUpVariantLong   (vaResult, pHotspot->m_Flags);                     break; // flags
    case 13:  SetUpVariantString (vaResult, pHotspot->m_sMoveCallback.c_str ());    break; // drag-and-drop move callback            
    case 14:  SetUpVariantString (vaResult, pHotspot->m_sReleaseCallback.c_str ()); break; // drag-and-drop release callback            
    case 15:  SetUpVariantLong   (vaResult, pHotspot->m_DragFlags);                 break; // drag-and-drop flags

    default:
      vaResult.vt = VT_NULL;
      break;

    } // end of switch

  }    // end of CMiniWindow::HotspotInfo


long CMiniWindow::ImageOp(short Action, 
                          long Left, long Top, long Right, long Bottom, 
                          long PenColour, long PenStyle, long PenWidth, 
                          long BrushColour, LPCTSTR ImageId, 
                          long EllipseWidth, long EllipseHeight)
  {

  long iResult = eUnknownOption;

  ImageMapIterator it = m_Images.find (ImageId);

  if (it == m_Images.end ())
    return eImageNotInstalled;

  CBitmap * bitmap = it->second;

  dc.SetBkMode (OPAQUE);

  // for monochrome bitmaps, 1-bits will come out in pen colour, and 0-bits will come out in background colour
  dc.SetTextColor (BrushColour);  // for patterned brushes
  dc.SetBkColor (PenColour);      // for hatched brushes and patterned brushes

  if (ValidatePenStyle (PenStyle, PenWidth))
    return ePenStyleNotValid;

  CBrush br;

  br.CreatePatternBrush (bitmap);

  // create requested pen 
  CPen pen;
  MakeAPen (pen, PenColour, PenStyle, PenWidth);
  
  // select into DC
  CPen* oldPen = dc.SelectObject(&pen);
  CBrush* oldBrush = dc.SelectObject(&br);

  switch (Action)
                                  
    {
    case 1:       // ellipse
      {
      dc.Ellipse (CRect (Left, Top, FixRight (Right), FixBottom (Bottom)));
      iResult = eOK;
      break; 
      }

    case 2:       // rectangle
      {
      dc.Rectangle (CRect (Left, Top, FixRight (Right), FixBottom (Bottom)));
      iResult = eOK;
      break; 
      }

    case 3:       // round rectangle
      {                                                                                                                                         
      dc.RoundRect (CRect (Left, Top, FixRight (Right), FixBottom (Bottom)), CPoint (EllipseWidth, EllipseHeight));
      iResult = eOK;
      break; 
      }


    } // end of switch

  // put things back
  dc.SelectObject (oldPen);
  dc.SelectObject (oldBrush);

  return iResult;


  }   // end of  CMiniWindow::ImageOp

long CMiniWindow::CreateImage(LPCTSTR ImageId, WORD Row1, WORD Row2, WORD Row3, WORD Row4, WORD Row5, WORD Row6, WORD Row7, WORD Row8)
  {

  ImageMapIterator it = m_Images.find (ImageId);

  if (it != m_Images.end ())
    {
    delete it->second;         // delete existing image
    m_Images.erase (it);
    }

  CBitmap * pImage = new CBitmap;

  // bottom row comes first in a bitmap
  WORD		wBits[8] = { Row8, Row7, Row6, Row5, Row4, Row3, Row2, Row1 };

  pImage->CreateBitmap (8, 8, 1, 1, wBits);

  m_Images [ImageId] = pImage;
  return eOK;

  }  // end of CMiniWindow::CreateImage

// see: http://www.nathanm.com/photoshop-blending-math/
// and: http://www.pegtop.net/delphi/articles/blendmodes/                   

#define Blend_It(Op) \
   do \
      if (Opacity < 1.0) \
        for (i = 0; i < count; i++) \
          pB [i]     = Blend_Opacity (pA [i], pB [i], Op, Opacity);\
      else\
        for (i = 0; i < count; i ++)\
          pB [i]      = Op (pA [i], pB [i]);\
   while (false)

// we have to do this a row at a time, because there might be a filler at the end of each row
// and thus, a discontinuity which throws out the r/g/b sequence
#define Colour_Op(fR,fG,fB) \
   do \
     {  \
     for (row = 0; row < iHeight; row++)\
      {\
      long base = row * perline;\
      unsigned char rA, gA, bA, rB, gB, bB; \
      if (Opacity < 1.0) \
        for (i = 0; i < perline - 2; ) \
          {\
          bA = pA [base + i];     \
          gA = pA [base + i + 1]; \
          rA = pA [base + i + 2]; \
          bB = pB [base + i];     \
          gB = pB [base + i + 1]; \
          rB = pB [base + i + 2]; \
          pB [base + i] = Simple_Opacity (bB, fB, Opacity); \
          i++;\
          pB [base + i] = Simple_Opacity (gB, fG, Opacity); \
          i++;\
          pB [base + i] = Simple_Opacity (rB, fR, Opacity); \
          i++;\
          }\
      else\
        for (i = 0; i < perline - 2;)\
          {\
          bA = pA [base + i];     \
          gA = pA [base + i + 1]; \
          rA = pA [base + i + 2]; \
          bB = pB [base + i];     \
          gB = pB [base + i + 1]; \
          rB = pB [base + i + 2]; \
          pB [base + i] = fB;  \
          i++;\
          pB [base + i] = fG;  \
          i++;\
          pB [base + i] = fR;  \
          i++;\
          }\
        }\
       }\
   while (false)



// see also: CMUSHclientDoc::BlendPixel

long CMiniWindow::BlendImage(LPCTSTR ImageId, 
                  long Left, long Top, long Right, long Bottom, 
                  short Mode, double Opacity, 
                  long SrcLeft, long SrcTop, long SrcRight, long SrcBottom)
  {

  // constrain to what we actually have
  if (Left < 0)
    Left = 0;
  if (Top < 0)
    Top = 0;
  if (Right > m_iWidth)
    Right = m_iWidth;
  if (Bottom > m_iHeight)
    Bottom = m_iHeight;

  ImageMapIterator it = m_Images.find (ImageId);

  if (it == m_Images.end ())
    return eImageNotInstalled;

  if (Opacity < 0.0 || Opacity > 1.0)
    return eBadParameter;

  CBitmap * bitmap = it->second;

  BITMAP  bi;
  bitmap->GetBitmap(&bi);

  // calculate size of desired rectangle
  long iWidth = FixRight (Right) - Left;
  long iHeight = FixBottom (Bottom) - Top;

  // constrain to what we actually have
  if (SrcLeft < 0)
    SrcLeft = 0;
  if (SrcTop < 0)
    SrcTop = 0;
  if (SrcRight > bi.bmWidth )
    SrcRight = bi.bmWidth ;
  if (SrcBottom > bi.bmHeight)
    SrcBottom = bi.bmHeight;

  // adjust so that -1 means 1 from right
  if (SrcRight <= 0) 
    SrcRight = bi.bmWidth + SrcRight;

  if (SrcBottom <= 0) 
    SrcBottom =  bi.bmHeight + SrcBottom;

  // width and height are the smaller of the two rectangles
  iWidth = min (iWidth, SrcRight - SrcLeft);
  iHeight = min (iHeight, SrcBottom - SrcTop);

  if (iWidth <= 0 || iHeight <= 0)   // sanity check
    return eOK;

  BITMAPINFO bmi;
  ZeroMemory (&bmi, sizeof bmi);

  bmi.bmiHeader.biSize = sizeof bmi;
  bmi.bmiHeader.biWidth =          iWidth;       
  bmi.bmiHeader.biHeight =         iHeight;
  bmi.bmiHeader.biPlanes =         1;
  bmi.bmiHeader.biBitCount =       24;
  bmi.bmiHeader.biCompression =    BI_RGB;
  bmi.bmiHeader.biSizeImage =      iHeight * BytesPerLine (iWidth, 24);

  // upper layer (from image id)
  CDC A_DC;
  A_DC.CreateCompatibleDC(&dc);

  unsigned char * pA = NULL;

  HBITMAP hbmA = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**) &pA, NULL, 0);

  HBITMAP hOldAbmp = (HBITMAP) SelectObject(A_DC.m_hDC, hbmA);

  //copy part from image to upper layer
  CDC bmDC;
  bmDC.CreateCompatibleDC(&dc);
  CBitmap *pOldbmp = bmDC.SelectObject(bitmap);

  A_DC.BitBlt (0, 0, iWidth, iHeight, &bmDC, SrcLeft, SrcTop, SRCCOPY);  
  bmDC.SelectObject(pOldbmp);


  // base image (from miniwindow)

  CDC B_DC;
  B_DC.CreateCompatibleDC(&dc);
  CBitmap B_bmp;

  unsigned char * pB = NULL;

  HBITMAP hbmB = CreateDIBSection(dc.m_hDC, &bmi, DIB_RGB_COLORS, (void**) &pB, NULL, 0);

  HBITMAP hOldBbmp = (HBITMAP) SelectObject(B_DC.m_hDC, hbmB);

  // copy base image from miniwindow to bitmap
  B_DC.BitBlt (0, 0, iWidth, iHeight, &dc, Left, Top, SRCCOPY);  


  // manipulate image here   A = blend, B = base


  long count = bmi.bmiHeader.biSizeImage;
  long perline = BytesPerLine (iWidth, 24);
 
  long i, row;

  // precompute cos table
  static unsigned char cos_table [256];
  static bool bTableComputed = false;

  if (!bTableComputed)
    {
    double pi_div255 = 3.1415926535898 / 255.0;
    for (i = 0; i < 256; i++)
      {
      double a = 64.0 - cos ((double) i * pi_div255) * 64.0;
      cos_table [i] = (uint8) (a + 0.5);  // round
      }
    bTableComputed = true;
    } // table needs computing

  switch (Mode)
    {

    // normal modes
    case  1: Blend_It (Blend_Normal);       break;
    case  2: Blend_It (Blend_Average);      break;
    case  3: Blend_It (Blend_Interpolate);  break;

     case  4:  // dissolve - randomly choose pixels based on opacity
       {

       for (row = 0; row < iHeight; row++)
        {
        long base = row * perline;
        unsigned char rA, gA, bA, rB, gB, bB; 
        for (i = 0; i < perline - 2; ) 
          {
          double rnd = genrand ();
          bA = pA [base + i];     
          gA = pA [base + i + 1]; 
          rA = pA [base + i + 2]; 

          bB = pB [base + i];  
          gB = pB [base + i + 1];
          rB = pB [base + i + 2];

          pB [base + i] = (rnd < Opacity) ? bA : bB;;
          i++;
          pB [base + i] = (rnd < Opacity) ? gA : gB;
          i++;
          pB [base + i] = (rnd < Opacity) ? rA : rB;
          i++;

          }
         }  // end for each row

       }
      break;

    // darkening modes
    case  5: Blend_It (Blend_Darken);     break;
    case  6: Blend_It (Blend_Multiply);   break;
    case  7: Blend_It (Blend_ColorBurn);  break;
    case  8: Blend_It (Blend_LinearBurn); break;
    case  9: Blend_It (Blend_InverseColorBurn);  break;
    case 10: Blend_It (Blend_Subtract);   break;
    
    // lightening modes
    case 11: Blend_It (Blend_Lighten);    break;
    case 12: Blend_It (Blend_Screen);     break;
    case 13: Blend_It (Blend_ColorDodge); break;
    case 14: Blend_It (Blend_LinearDodge);break;
    case 15: Blend_It (Blend_InverseColorDodge);  break;
    case 16: Blend_It (Blend_Add);        break;
      
    // soft/hard light etc.
    case 17: Blend_It (Blend_Overlay);    break;
    case 18: Blend_It (Blend_SoftLight);  break;
    case 19: Blend_It (Blend_HardLight);  break;
    case 20: Blend_It (Blend_VividLight); break;
    case 21: Blend_It (Blend_LinearLight);break;
    case 22: Blend_It (Blend_PinLight);   break;
    case 23: Blend_It (Blend_HardMix);    break;

    // difference modes
    case 24: Blend_It (Blend_Difference); break;
    case 25: Blend_It (Blend_Exclusion);  break;

    // glow modes
    case 26: Blend_It (Blend_Reflect);    break;
    case 27: Blend_It (Blend_Glow);       break;
    case 28: Blend_It (Blend_Freeze);     break;
    case 29: Blend_It (Blend_Heat);       break;
        
    case 30: Blend_It (Blend_Negation);   break;
    case 31: Blend_It (Blend_Phoenix);    break;
    case 32: Blend_It (Blend_Stamp);      break;

    // logical modes
    case 33: Blend_It (Blend_Xor);      break;
    case 34: Blend_It (Blend_And);      break;
    case 35: Blend_It (Blend_Or);       break;

    // the follow modes take one colour from the blend and retain 2 from the base
    case 36: Colour_Op (rA, gB, bB); break;  // red  
    case 37: Colour_Op (rB, gA, bB); break;  // green
    case 38: Colour_Op (rB, gB, bA); break;  // blue 

    // the follow modes take two colours from the blend and retain 1 from the base
    case 39: Colour_Op (rA, gA, bB); break;  // yellow 
    case 40: Colour_Op (rB, gA, bA); break;  // cyan    
    case 41: Colour_Op (rA, gB, bA); break;  // magenta    

    // limit green
    case 42: Colour_Op (rA, (gA > rA) ? rA : gA, bA); break;  // green limited by red    
    case 43: Colour_Op (rA, (gA > bA) ? bA : gA, bA); break;  // green limited by blue    
    case 44: Colour_Op (rA, (gA > ((rA + bA) / 2)) ? ((rA + bA) / 2) : gA, bA); break;  // green limited by average of red and blue    

    // limit blue
    case 45: Colour_Op (rA, gA, (bA > rA) ? rA : bA); break;  // blue limited by red    
    case 46: Colour_Op (rA, gA, (bA > gA) ? gA : bA); break;  // blue limited by green    
    case 47: Colour_Op (rA, gA, (bA > ((rA + gA) / 2)) ? ((rA + gA) / 2) : bA); break; // blue limited by average of red and green    

    // limit red
    case 48: Colour_Op ((rA > gA) ? gA : rA, gA, bA); break;  // red limited by green    
    case 49: Colour_Op ((rA > bA) ? bA : rA, gA, bA); break;  // red limited by blue    
    case 50: Colour_Op ((rA > ((gA + bA) / 2)) ? ((gA + bA) / 2) : rA, gA, bA); break;  // red limited by average of green and blue    

    // select single colour
    case 51: Colour_Op (rA, 0, 0); break;  // red only       (looks red)
    case 52: Colour_Op (0, gA, 0); break;  // green only     (looks green)
    case 53: Colour_Op (0, 0, bA); break;  // blue only      (looks blue)

    // discard single colour
    case 54: Colour_Op (0, gA, bA); break;  // discard red    (looks cyan)
    case 55: Colour_Op (rA, 0, bA); break;  // discard green  (looks magenta)
    case 56: Colour_Op (rA, gA, 0); break;  // discard blue   (looks yellow)

    // one colour to all channels  (ie. it looks grey)
    case 57: Colour_Op (rA, rA, rA); break;  // all red   
    case 58: Colour_Op (gA, gA, gA); break;  // all green   
    case 59: Colour_Op (bA, bA, bA); break;  // all blue   

    case 60:           // Hue mode
      {
       for (row = 0; row < iHeight; row++)
        {
        long base = row * perline;
        unsigned char rA, gA, bA, rB, gB, bB; 
        for (i = 0; i < perline - 2; ) 
          {
          CColor cA, cB;    // A = blend, B = base
          bA = pA [base + i];     
          gA = pA [base + i + 1]; 
          rA = pA [base + i + 2]; 
          bB = pB [base + i];  
          gB = pB [base + i + 1];
          rB = pB [base + i + 2];

          cA.SetRGB (rA, gA, bA);
          cB.SetRGB (rB, gB, bB);

          cB.SetHue (cA.GetHue ());  // hue of blend image, others from base image
          pB [base + i] = Simple_Opacity (bB, cB.GetBlue (), Opacity); 
          i++;
          pB [base + i] = Simple_Opacity (gB, cB.GetGreen (), Opacity); 
          i++;
          pB [base + i] = Simple_Opacity (rB, cB.GetRed (), Opacity); 
          i++;
          }
         }  // end for each row
       }
      break;

    case 61:           // Saturation mode
      {
       for (row = 0; row < iHeight; row++)
        {
        long base = row * perline;
        unsigned char rA, gA, bA, rB, gB, bB; 
        for (i = 0; i < perline - 2; ) 
          {
          CColor cA, cB;    // A = blend, B = base
          bA = pA [base + i];     
          gA = pA [base + i + 1]; 
          rA = pA [base + i + 2]; 
          bB = pB [base + i];  
          gB = pB [base + i + 1];
          rB = pB [base + i + 2];

          cA.SetRGB (rA, gA, bA);
          cB.SetRGB (rB, gB, bB);

          cB.SetSaturation (cA.GetSaturation ());  // saturation of blend image, others from base image
          pB [base + i] = Simple_Opacity (bB, cB.GetBlue (), Opacity); 
          i++;
          pB [base + i] = Simple_Opacity (gB, cB.GetGreen (), Opacity); 
          i++;
          pB [base + i] = Simple_Opacity (rB, cB.GetRed (), Opacity); 
          i++;
          }
         }  // end for each row
       }
      break;

    case 62:           // Colour mode
      {
       for (row = 0; row < iHeight; row++)
        {
        long base = row * perline;
        unsigned char rA, gA, bA, rB, gB, bB; 
        for (i = 0; i < perline - 2; ) 
          {
          CColor cA, cB;    // A = blend, B = base
          bA = pA [base + i];     
          gA = pA [base + i + 1]; 
          rA = pA [base + i + 2]; 
          bB = pB [base + i];  
          gB = pB [base + i + 1];
          rB = pB [base + i + 2];

          cA.SetRGB (rA, gA, bA);
          cB.SetRGB (rB, gB, bB);

          cB.SetHue (cA.GetHue ());  // hue of blend image, 
          cB.SetSaturation (cA.GetSaturation ());  // saturation of blend image, luminance from base image
          pB [base + i] = Simple_Opacity (bB, cB.GetBlue (), Opacity); 
          i++;
          pB [base + i] = Simple_Opacity (gB, cB.GetGreen (), Opacity); 
          i++;
          pB [base + i] = Simple_Opacity (rB, cB.GetRed (), Opacity); 
          i++;
          }
         }  // end for each row
       }
      break;

      /*

    Note: I don't get the same results here as using Photoshop on the same images. Not quite sure why,
    some test code below seems to indicate that for all possible values of RGB, they map to HSL and back
    again without error.

    In the absence of any obvious bug, I am assuming that by plugging the Luminance from one colour into the
    L channel of another, I am creating an out-of-gamut colour, which then cannot be represented properly.

    I can't find, so far, any code that would limit the value to be in-gamut, so for now this Luminance (and
    Colour, Saturation and Hue too, probably) should be regarded as experimental.  - NJG - 2/Aug/2008.

    */

    case 63:           // Luminance mode
      {
       for (row = 0; row < iHeight; row++)
        {
        long base = row * perline;
        unsigned char rA, gA, bA, rB, gB, bB; 
        for (i = 0; i < perline - 2; ) 
          {
          CColor cA, cB;    // A = blend, B = base
          bA = pA [base + i];     
          gA = pA [base + i + 1]; 
          rA = pA [base + i + 2]; 
          bB = pB [base + i];  
          gB = pB [base + i + 1];
          rB = pB [base + i + 2];

          // bit of a fudge here, but what can you do?

          // this is going to brighten the really dark pixels, however at least then they
          // should retain their base colour, rather than be promoted up to something garish

//          if (rB < 12) rB = 12;
//          if (gB < 12) gB = 12;
//          if (bB < 12) bB = 12;

          cA.SetRGB (rA, gA, bA);
          cB.SetRGB (rB, gB, bB);

          cB.SetLuminance (cA.GetLuminance ());  // luminance from blend image, others from base image

          pB [base + i] = Simple_Opacity (bB, cB.GetBlue (), Opacity); 
          i++;
          pB [base + i] = Simple_Opacity (gB, cB.GetGreen (), Opacity); 
          i++;
          pB [base + i] = Simple_Opacity (rB, cB.GetRed (), Opacity); 
          i++;

          }
         }  // end for each row
       }
      break;

    case 64:           // HSL (hue to red, saturation to green, luminance to blue)
      {
       for (row = 0; row < iHeight; row++)
        {
        long base = row * perline;
        unsigned char rA, gA, bA, rB, gB, bB; 
        for (i = 0; i < perline - 2; ) 
          {
          CColor cA, cB;    // A = blend, B = base
          bA = pA [base + i];     
          gA = pA [base + i + 1]; 
          rA = pA [base + i + 2]; 
          bB = pB [base + i];  
          gB = pB [base + i + 1];
          rB = pB [base + i + 2];

          cA.SetRGB (rA, gA, bA);

          pB [base + i] = Simple_Opacity (bB, cA.GetLuminance () * 255, Opacity); 
          i++;
          pB [base + i] = Simple_Opacity (gB, cA.GetSaturation () * 255, Opacity); 
          i++;
          pB [base + i] = Simple_Opacity (rB, (cA.GetHue () / 360.0) * 255, Opacity); 
          i++;

          }
         }  // end for each row
       }
      break;

      /*
    case 65: // testing  conversion to/from RGB to HSL and back again is consistent.
      {
      int r, g, b;
      CColor cA;  

      for (r = 0; r < 256; r ++)
        for (g = 0; g < 256; g ++)
          for (b = 0; b < 256; b ++)
            {
            cA.SetRGB (r, g, b);
            cA.GetLuminance ();  // force change to HSL
            ASSERT (r == cA.GetRed ());
            ASSERT (g == cA.GetGreen ());
            ASSERT (b == cA.GetBlue ());

            } 


      }
    break;

      */

    default: return eUnknownOption;
    }

  // copy result back

  dc.BitBlt (Left, Top, iWidth, iHeight, &B_DC, 0, 0, SRCCOPY);  


  SelectObject(A_DC.m_hDC, hOldAbmp);
  SelectObject(B_DC.m_hDC, hOldBbmp);

  DeleteObject (hbmA);
  DeleteObject (hbmB);

  return eOK;



  }   // end of CMiniWindow::BlendImage


// copies one miniwindow image as a stand-alone image
long CMiniWindow::ImageFromWindow(LPCTSTR ImageId, CMiniWindow * pSrcWindow)
  {


  ImageMapIterator it = m_Images.find (ImageId);

  if (it != m_Images.end ())
    {
    delete it->second;         // delete existing image
    m_Images.erase (it);
    }

  // make new bitmap of appropriate size to hold image from other miniwindow
  CBitmap * pImage = new CBitmap;
  pImage->CreateCompatibleBitmap (&dc, pSrcWindow->m_iWidth, pSrcWindow->m_iHeight);

  // prepare to copy it
  CDC bmDC;
  bmDC.CreateCompatibleDC(&dc);
  CBitmap *pOldbmp = bmDC.SelectObject(pImage);

  // copy into our bitmap
  bmDC.BitBlt (0, 0, pSrcWindow->m_iWidth, pSrcWindow->m_iHeight, &pSrcWindow->dc, 0, 0, SRCCOPY);  

  // done
  bmDC.SelectObject(pOldbmp);

  // save in map
  m_Images [ImageId] = pImage;
  return eOK;

  } // end of CMiniWindow::ImageFromWindow


static void HorizontalLinearGradient (const COLORREF StartColour, 
                                      const COLORREF EndColour, 
                                      unsigned char * pBuffer,
                                      const long iWidth,
                                      const long iHeight)
  {

  if (iWidth < 1)
    return;  // avoid divide by zero

  double rdiff = GetRValue (EndColour) - GetRValue (StartColour),
         gdiff = GetGValue (EndColour) - GetGValue (StartColour),
         bdiff = GetBValue (EndColour) - GetBValue (StartColour);

  double rval = GetRValue (StartColour),
        gval = GetGValue (StartColour),
        bval = GetBValue (StartColour);

  double rinc = rdiff / (double) (iWidth - 1), 
        ginc = gdiff / (double) (iWidth - 1), 
        binc = bdiff / (double) (iWidth - 1);

  long row, col;
  uint8 r = 0, g = 0, b = 0;

  int increment =  BytesPerLine (iWidth, 24);

   // main loop is columns
   for (col = 0; col < iWidth; col++)
     {
     r = (uint8) rval;
     g = (uint8) gval;
     b = (uint8) bval;
     unsigned char * p = pBuffer + col * 3;
     for (row = 0; row < iHeight; row++)
       {
       p [0] = b;
       p [1] = g;
       p [2] = r;
       p += increment;
       }  // end of each row
     rval += rinc;
     gval += ginc;
     bval += binc;                      
     }  // end of each column


  }  // end of HorizontalLinearGradient

static void HorizontalLinearGradientHSL (const CColor StartColour, 
                                      const CColor EndColour, 
                                      unsigned char * pBuffer,
                                      const long iWidth,
                                      const long iHeight)
  {

  if (iWidth < 1)
    return;  // avoid divide by zero

  double starthue = StartColour.GetHue ();
  double endhue = EndColour.GetHue ();
  double startlum = StartColour.GetLuminance ();
  double endlum = EndColour.GetLuminance ();
  double startsat = StartColour.GetSaturation ();
  double endsat = EndColour.GetSaturation ();


  double hinc = (endhue - starthue) / (double) (iWidth - 1), 
         linc = (endlum - startlum) / (double) (iWidth - 1),
         sinc = (endsat - startsat) / (double) (iWidth - 1);  

  long row, col;
  uint8 r = 0, g = 0, b = 0;

  int increment =  BytesPerLine (iWidth, 24);

   // main loop is columns
   for (col = 0; col < iWidth; col++)
     {
     double h = starthue;
     double l = startlum;
     double s = startsat;

     CColor c;
     c.SetHLS (h, l, s);
     r = c.GetRed ();
     g = c.GetGreen ();
     b = c.GetBlue ();

     unsigned char * p = pBuffer + col * 3;
     for (row = 0; row < iHeight; row++)
       {
       p [0] = b;
       p [1] = g;
       p [2] = r;
       p += increment;
       }  // end of each row
     starthue += hinc;
     startlum += linc;                      
     startsat += sinc;
     }  // end of each column


  }  // end of HorizontalLinearGradient

static void VerticalLinearGradient (const COLORREF StartColour, 
                                    const COLORREF EndColour, 
                                      unsigned char * pBuffer,
                                      const long iWidth,
                                      const long iHeight)
  {

  if (iHeight < 1)
    return;  // avoid divide by zero

  double rdiff = GetRValue (EndColour) - GetRValue (StartColour),
         gdiff = GetGValue (EndColour) - GetGValue (StartColour),
         bdiff = GetBValue (EndColour) - GetBValue (StartColour);

  double rval = GetRValue (EndColour),
         gval = GetGValue (EndColour),
         bval = GetBValue (EndColour);

  double rinc = - rdiff / (double) (iHeight - 1), 
         ginc = - gdiff / (double) (iHeight - 1), 
         binc = - bdiff / (double) (iHeight - 1);

  long row, col;
  uint8 r = 0, g = 0, b = 0;

  int increment =  BytesPerLine (iWidth, 24);

   // main loop is rows
   for (row = 0; row < iHeight; row++)
     {
     r = (uint8) rval;
     g = (uint8) gval;
     b = (uint8) bval;
     unsigned char * p = pBuffer + increment * row;
     for (col = 0; col < iWidth; col++)
       {
       p [0] = b;
       p [1] = g;
       p [2] = r;
       p += 3;
       }  // end of each column
     rval += rinc;
     gval += ginc;
     bval += binc;                      
     }  // end of each row
   
  }  // end of VerticalLinearGradient


static void MakeTexture (const COLORREF Multiplier, 
                         unsigned char * pBuffer,
                         const long iWidth,
                         const long iHeight)
  {

  ULONG  rval = GetRValue (Multiplier),
         gval = GetGValue (Multiplier),
         bval = GetBValue (Multiplier);

  ULONG row, col;
  ULONG c;

  int increment =  BytesPerLine (iWidth, 24);

   // main loop is columns
   for (col = 0; col < iWidth; col++)
     {
     unsigned char * p = pBuffer + col * 3;
     for (row = 0; row < iHeight; row++)
       {
       c = col ^ row;
       p [0] = (unsigned char) (c * bval);
       p [1] = (unsigned char) (c * gval);
       p [2] = (unsigned char) (c * rval);
       p += increment;
       }  // end of each row
     }  // end of each column


  }  // end of MakeTexture


// this is a load of old cobblers

#if 0

static void HorizontalLogGradient (const COLORREF StartColour, const double rdiff, const double gdiff, const double bdiff, 
                                      unsigned char * pBuffer,
                                      const long iWidth,
                                      const long iHeight,
                                      const bool rightToLeft)
  {
  double rval = GetRValue (StartColour),
         gval = GetGValue (StartColour),
         bval = GetBValue (StartColour);


  double rfactor = pow (abs (rdiff), (double) 1.0 / (double) iWidth);
  double gfactor = pow (abs (gdiff), (double) 1.0 / (double) iWidth);
  double bfactor = pow (abs (bdiff), (double) 1.0 / (double) iWidth);


  long row, col;
  uint8 r = 0, g = 0, b = 0;

  double rinc = rfactor, 
         ginc = gfactor, 
         binc = bfactor;

  int increment =  BytesPerLine (iWidth, 24);

   // main loop is columns
   for (col = 0; col < iWidth; col++)
     {


     r = (uint8) rval;
     g = (uint8) gval;
     b = (uint8) bval;
     unsigned char * p = pBuffer + col * 3;
     for (row = 0; row < iHeight; row++)
       {
       p [0] = b;
       p [1] = g;
       p [2] = r;
       p += increment;
       }  // end of each row

     rinc *= rfactor;
     ginc *= gfactor;
     binc *= bfactor;

     if (rightToLeft)
       {
       rval = GetRValue (StartColour) - rinc;
       gval = GetGValue (StartColour) - ginc;
       bval = GetBValue (StartColour) - binc;
       }
     else
       {
       rval = GetRValue (StartColour) + rinc;
       gval = GetGValue (StartColour) + ginc;
       bval = GetBValue (StartColour) + binc;
       }
     }  // end of each column


  }  // end of HorizontalLogGradient

#endif // 0

long CMiniWindow::Gradient(long Left, long Top, long Right, long Bottom, 
                      long StartColour, long EndColour, 
                      short Mode)
  {
  // calculate size of desired rectangle
  long iWidth = FixRight (Right) - Left;
  long iHeight = FixBottom (Bottom) - Top;

  if (iWidth <= 0 || iHeight <= 0)   // sanity check
    return eOK;

  // upper layer (from image id)
  CDC gDC;
  gDC.CreateCompatibleDC(&dc);
  CBitmap gbmp;

  BITMAPINFO bmi;
  ZeroMemory (&bmi, sizeof bmi);

  bmi.bmiHeader.biSize = sizeof bmi;
  bmi.bmiHeader.biWidth =          iWidth;       
  bmi.bmiHeader.biHeight =         iHeight;
  bmi.bmiHeader.biPlanes =         1;
  bmi.bmiHeader.biBitCount =       24;
  bmi.bmiHeader.biCompression =    BI_RGB;
  bmi.bmiHeader.biSizeImage =      iHeight * BytesPerLine (iWidth, 24);

  unsigned char * pA = NULL;

  HBITMAP hbmG = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**) &pA, NULL, 0);

  HBITMAP hOldAbmp = (HBITMAP) SelectObject(gDC.m_hDC, hbmG);

  switch (Mode)
    {
    case 1 : // horizontal (left to right)
       HorizontalLinearGradient (StartColour, EndColour, pA, iWidth, iHeight);
       break;  // end of horizontal


    case 2 : // vertical  (top to bottom)
       VerticalLinearGradient (StartColour, EndColour, pA, iWidth, iHeight);
       break;  // end of vertical

    case 3 : // texture 
       MakeTexture (StartColour, pA, iWidth, iHeight);
       break;  // end of texture

    // forget it!

#if 0
    case 4 : // horizontal (left to right)
       HorizontalLinearGradientHSL (CColor (StartColour), CColor (EndColour), pA, iWidth, iHeight);
       break;  // end of horizontal

    case 5 : // horizontal (left to right)
       HorizontalLogGradient (StartColour, rdiff, gdiff, bdiff, pA, iWidth, iHeight, false);
       break;  // end of horizontal

    case 6 : // horizontal (right to left)
       HorizontalLogGradient (EndColour, rdiff, gdiff, bdiff, pA, iWidth, iHeight, true);
       break;  // end of horizontal

#endif // 0

    default: return eUnknownOption;

    } // end of switch


  // copy result back

  dc.BitBlt (Left, Top, iWidth, iHeight, &gDC, 0, 0, SRCCOPY);  


  SelectObject(gDC.m_hDC, hOldAbmp);

  DeleteObject (hbmG);

  return eOK;

  } // end of CMiniWindow::Gradient

static void GeneralFilter (unsigned char * inbuf, long iWidth, long iHeight, 
                            const long iPerLine, 
                            const double Options,
                            const double * iMatrix,
                            const double iDivisor)
  {

  unsigned char * pi;
  long total;

  long row, col, rgb;
  int i;
  int iLastByte = iWidth * 3;
  long window [5];

  if (Options != 2)    // 2 = vertical only
    {
    // horizontal
    // main loop is rows
    for (row = 0; row < iHeight; row++)
     {

     pi = inbuf + iPerLine * row;
   
    
     for (rgb = 0; rgb < 3; rgb++)
       {

       // preload 4 slots in window for edge case  (-2 to +1)
       for (col = 0; col < 4; col++)
          window [col + 1] = pi [min (max (rgb + (col * 3) - 6, 0), iLastByte - 1)];

       for (col = 0; col < iLastByte - 4; col += 3)
         {
         // we add 3 to get to the next pixel in that colour (R, G or B)

         // insert right-most byte into window
       
         window [0] = window [1];
         window [1] = window [2];
         window [2] = window [3];
         window [3] = window [4];
         window [4] = pi [min (max (col + rgb + 6, 0), iLastByte - 1)];

         total = 0;

         for (i = 0; i < 5; i++)
            total +=  window [i] * iMatrix [i];

         pi [col + rgb] = (uint8) (min (max (total / iDivisor, 0), 255));
         }   // end of each column
       }     // end of each colour

     }  // end of each row
    }   // horizontal wanted

  if (Options != 1)    // 1 = horizontal only
  {

  // vertical
    // main loop is columns
    for (col = 0; col < iLastByte; col++)
      {

       // preload 4 slots in window for edge case  (-2 to +1)
       for (row = 0; row < 4; row++)
         {
         long from = min (max (row - 2, 0), iHeight - 1);
         pi = inbuf + col + (from * iPerLine);
         window [row + 1] = *pi;
         }

      for (row = 0; row < iHeight; row++)
       {

       long from = min (max (row + 3, 0), iHeight - 1);
       pi = inbuf + col + (from * iPerLine);

       // insert right-most byte into window
     
       window [0] = window [1];
       window [1] = window [2];
       window [2] = window [3];
       window [3] = window [4];
       window [4] = *pi; 

       total = 0;

       for (i = 0; i < 5; i++)
          total +=  window [i] * iMatrix [i];

       pi = inbuf + col + (row * iPerLine);
       pi [0] = (uint8) (min (max (total / iDivisor, 0), 255));

       }  // end of each row
      }  // end of each column
    }  // vertical wanted

  } // end of Blur


static void Noise (unsigned char * inbuf, long iWidth, long iHeight, long iPerLine, double Options)
  {

  unsigned char * pi = inbuf;
  long count = iPerLine * iHeight;
  long i, c;
  double threshold = Options / 100.0;
  for (i = 0; i < count; i++)
    {
    c = *pi;
    c += (long) ((128 - genrand () * 256) * threshold);
    *pi++  = CLAMP (c);
    }

  } // end of Noise

static void MonoNoise (unsigned char * inbuf, long iWidth, long iHeight, long iPerLine, double Options)
  {

  long i, j, c, row;
  double threshold = Options / 100.0;


  for (row = 0; row < iHeight; row++)
    {
    unsigned char * pi = inbuf + (row * iPerLine);
    for (i = 0; i < iWidth; i ++) 
      {
      j = (128 - genrand () * 256) * threshold;

      c = *pi + j;
      *pi++  = CLAMP (c);
      c = *pi + j;
      *pi++  = CLAMP (c);
      c = *pi + j;
      *pi++  = CLAMP (c);
      }
     }  // end for each row


  } // end of MonoNoise


static void Brightness (unsigned char * inbuf, long iWidth, long iHeight, long iPerLine, double Options)
  {

  unsigned char * pi = inbuf;
  long count = iPerLine * iHeight;
  long i, c;
  for (i = 0; i < count; i++)
    {
    c = *pi;
    c += Options;
    *pi++  = CLAMP (c);
    }

  } // end of Brightness


static void BrightnessMultiply (unsigned char * inbuf, long iWidth, long iHeight, long iPerLine, double Options)
  {

  unsigned char * pi = inbuf;
  long count = iPerLine * iHeight;
  long i, c;
  for (i = 0; i < count; i++)
    {
    c = *pi;
    c *= Options;
    *pi++  = CLAMP (c);
    }

  } // end of BrightnessMultiply

static void Contrast (unsigned char * inbuf, long iWidth, long iHeight, long iPerLine, double Options)
  {

  unsigned char lookup [256];  // lookup table for speed

  unsigned char * pi = inbuf;
  long count = iPerLine * iHeight;
  long i;
  double c;
  
  for (i = 0; i < 256; i++)
    {
    c = i - 128;    // center on zero
    c *= Options;  // multiply by contrast
    c += 128;  // put back
    lookup [i] = CLAMP (c);
    }  // end of for loop

  // now convert image using lookup table
  for (i = 0; i < count; i++)
    *pi++ = lookup [*pi];

  } // end of Contrast

static void Gamma (unsigned char * inbuf, long iWidth, long iHeight, long iPerLine, double Options)
  {

  unsigned char lookup [256];  // lookup table for speed

  unsigned char * pi = inbuf;
  long count = iPerLine * iHeight;
  long i;
  double c;
  if (Options < 0.0)
    Options = 0.0;

  for (i = 0; i < 256; i++)
    {
    c = ( (double) i) / 255.0;  // normalize it
    c = pow (c, Options);
    c *= 255;
    lookup [i] = CLAMP (c);
    }  // end of for loop

  // now convert image using lookup table
  for (i = 0; i < count; i++)
    *pi++ = lookup [*pi];
    
  } // end of Gamma


static void ColourBrightness (unsigned char * inbuf, long iWidth, long iHeight, 
                              long iPerLine, double Options, long iColour)
  {

  long i, row, c;

  for (row = 0; row < iHeight; row++)
    {
    unsigned char * pi = inbuf + (row * iPerLine) + iColour;
    for (i = 0; i < iWidth; i ++) 
      {
      c = *pi;
      c += Options;
      *pi  = CLAMP (c);
      pi += 3;
      }
     }  // end for each row

  } // end of ColourBrightness


static void ColourBrightnessMultiply (unsigned char * inbuf, long iWidth, long iHeight, 
                              long iPerLine, double Options, long iColour)
  {

  long i, row, c;

  for (row = 0; row < iHeight; row++)
    {
    unsigned char * pi = inbuf + (row * iPerLine) + iColour;
    for (i = 0; i < iWidth; i ++) 
      {
      c = *pi;
      c *= Options;
      *pi  = CLAMP (c);
      pi += 3;
      }
     }  // end for each row

  } // end of ColourBrightnessMultiply


static void ColourContrast (unsigned char * inbuf, long iWidth, long iHeight, 
                            long iPerLine, double Options, long iColour)
  {

  unsigned char lookup [256];  // lookup table for speed

  long i, row;
  double c;
  
  for (i = 0; i < 256; i++)
    {
    c = i - 128;    // center on zero
    c *= Options;  // multiply by contrast
    c += 128;  // put back
    lookup [i] = CLAMP (c);
    }  // end of for loop

  // now convert image using lookup table
  for (row = 0; row < iHeight; row++)
    {
    unsigned char * pi = inbuf + (row * iPerLine) + iColour;
    for (i = 0; i < iWidth; i ++) 
      {
      *pi = lookup [*pi];
      pi += 3;
      }
     }  // end for each row

    } // end of ColourContrast

static void ColourGamma (unsigned char * inbuf, long iWidth, long iHeight, 
                         long iPerLine, double Options, long iColour)
  {

  unsigned char lookup [256];  // lookup table for speed

  long i, row;
  double c;

  if (Options < 0.0)
    Options = 0.0;

  for (i = 0; i < 256; i++)
    {
    c = ( (double) i) / 255.0;  // normalize it
    c = pow (c, Options);
    c *= 255;
    lookup [i] = CLAMP (c);
    }  // end of for loop

  // now convert image using lookup table
  for (row = 0; row < iHeight; row++)
    {
    unsigned char * pi = inbuf + (row * iPerLine) + iColour;
    for (i = 0; i < iWidth; i ++) 
      {
      *pi = lookup [*pi];
      pi += 3;
      }
     }  // end for each row
    
  } // end of ColourGamma


// see: http://en.wikipedia.org/wiki/Grayscale

static void MakeGreyscale (unsigned char * inbuf, long iWidth, long iHeight, long iPerLine, double Options, const bool bLinear)
  {

  long i, row;
  double c;


  for (row = 0; row < iHeight; row++)
    {
    unsigned char * pi = inbuf + (row * iPerLine);
    for (i = 0; i < iWidth; i ++) 
      {

      if (bLinear)
        {
        c =  pi [0] + pi [1] + pi [2];
        c /= 3;
        }
      else
        c = pi [0] * 0.11 + // blue   (perceptual)
            pi [1] * 0.59 + // green
            pi [2] * 0.30;  // red

      *pi++  = CLAMP (c);
      *pi++  = CLAMP (c);
      *pi++  = CLAMP (c);
      }
     }  // end for each row


  } // end of MakeGreyscale


static void Average (unsigned char * pBuffer,
                      const long iWidth,
                      const long iHeight)
  {

  long row, col;
  __int64 r = 0, g = 0, b = 0, count = 0;

  int increment =  BytesPerLine (iWidth, 24);

  // find average

  // main loop is columns
  for (col = 0; col < iWidth; col++)
   {
   unsigned char * p = pBuffer + col * 3;
   for (row = 0; row < iHeight; row++)
     {
     b += p [0];
     g += p [1];
     r += p [2];
     count++;
     p += increment;
     }  // end of each row
   }  // end of each column

  r /= count;
  g /= count;
  b /= count;

  // apply average

  // main loop is columns
  for (col = 0; col < iWidth; col++)
   {
   unsigned char * p = pBuffer + col * 3;
   for (row = 0; row < iHeight; row++)
     {
     p [0] = CLAMP (b);
     p [1] = CLAMP (g);
     p [2] = CLAMP (r);
     p += increment;
     }  // end of each row
   }  // end of each column

  }  // end of Average


// see also:  CMUSHclientDoc::FilterPixel
long CMiniWindow::Filter(long Left, long Top, long Right, long Bottom, 
                  short Operation, double Options)
  {

  // constrain to what we actually have
  if (Left < 0)
    Left = 0;
  if (Top < 0)
    Top = 0;
  if (Right > m_iWidth)
    Right = m_iWidth;
  if (Bottom > m_iHeight)
    Bottom = m_iHeight;

  // calculate size of desired rectangle
  long iWidth = FixRight (Right) - Left;
  long iHeight = FixBottom (Bottom) - Top;

  if (iWidth < 1 || iHeight < 1)   // sanity check
    return eOK;

  // upper layer (from image id)
  CDC gDC;
  gDC.CreateCompatibleDC(&dc);
  CBitmap gbmp;

  BITMAPINFO bmi;
  ZeroMemory (&bmi, sizeof bmi);

  bmi.bmiHeader.biSize = sizeof bmi;
  bmi.bmiHeader.biWidth =          iWidth;       
  bmi.bmiHeader.biHeight =         iHeight;
  bmi.bmiHeader.biPlanes =         1;
  bmi.bmiHeader.biBitCount =       24;
  bmi.bmiHeader.biCompression =    BI_RGB;
  bmi.bmiHeader.biSizeImage =      iHeight * BytesPerLine (iWidth, 24);

  unsigned char * pA = NULL;

  HBITMAP hbmG = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**) &pA, NULL, 0);

  HBITMAP hOldAbmp = (HBITMAP) SelectObject(gDC.m_hDC, hbmG);

  // copy base image from miniwindow to bitmap
  gDC.BitBlt (0, 0, iWidth, iHeight, &dc, Left, Top, SRCCOPY);  

  // manipulate image here

   switch (Operation)
     {
     case 1: Noise    (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options); break;
     case 2: MonoNoise(pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options); break;
     case 3: // blur
       {
       double iMatrix [5] = { 1, 1, 1, 1, 1 };
       GeneralFilter     (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, iMatrix, 5); 
       }
       break;
     case 4:  // sharpen
       {
       double iMatrix [5] = { -1, -1, 7, -1, -1 };
       GeneralFilter     (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, iMatrix, 3); 
       }
       break;
     case 5:  // edge detect
       {
       double iMatrix [5] = { 0, 2.5, -6, 2.5, 0 };
       GeneralFilter     (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, iMatrix, 1); 
       }
       break;
     case 6:  // emboss
       {
       double iMatrix [5] = { 1, 2, 1, -1, -2 };
       GeneralFilter     (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, iMatrix, 1); 
       }
       break;
     case  7: Brightness        (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options); break;      // additive brightness
     case  8: Contrast          (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options); break;
     case  9: Gamma             (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options); break;
     case 10: ColourBrightness  (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, 2); break;    // red
     case 11: ColourContrast    (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, 2); break;    // red
     case 12: ColourGamma       (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, 2); break;    // red
     case 13: ColourBrightness  (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, 1); break;    // green
     case 14: ColourContrast    (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, 1); break;    // green
     case 15: ColourGamma       (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, 1); break;    // green
     case 16: ColourBrightness  (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, 0); break;    // blue
     case 17: ColourContrast    (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, 0); break;    // blue
     case 18: ColourGamma       (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, 0); break;    // blue
     case 19: MakeGreyscale     (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, true); break;    // linear
     case 20: MakeGreyscale     (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, false); break;    // perceptual
     case 21: BrightnessMultiply(pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options); break;          // multiplicative brightness
     case 22: ColourBrightnessMultiply(pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, 2); break;   // red
     case 23: ColourBrightnessMultiply(pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, 1); break;   // green
     case 24: ColourBrightnessMultiply(pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, 0); break;   // blue
     case 25: // lesser blur
       {
       double iMatrix [5] = { 0, 1, 1, 1, 0 };
       GeneralFilter     (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, iMatrix, 3); 
       }
       break;
     case 26: // minor blur
       {
       double iMatrix [5] = { 0, 0.5, 1, 0.5, 0 };
       GeneralFilter     (pA, iWidth, iHeight, BytesPerLine (iWidth, 24), Options, iMatrix, 2); 
       }
       break;
     case 27: Average             (pA, iWidth, iHeight); break;



     default: return eUnknownOption;

     }

  // copy result back

  dc.BitBlt (Left, Top, iWidth, iHeight, &gDC, 0, 0, SRCCOPY);  


  SelectObject(gDC.m_hDC, hOldAbmp);

  DeleteObject (hbmG);

  return eOK;

  } // end of CMiniWindow::Filter 



long CMiniWindow::SetPixel(long x, long y, long Colour)
  {
  dc.SetPixelV(x, y, Colour);
  return eOK;
  } // end of CMiniWindow::SetPixel


long CMiniWindow::GetPixel(long x, long y)
  {
  return dc.GetPixel(x, y);
  } // end of CMiniWindow::GetPixel


long CMiniWindow::MergeImageAlpha(LPCTSTR ImageId, LPCTSTR MaskId, 
                                  long Left, long Top, long Right, long Bottom, 
                                  short Mode, double Opacity, 
                                  long SrcLeft, long SrcTop, long SrcRight, long SrcBottom)
  {

  // constrain to what we actually have
  if (Left < 0)
    Left = 0;
  if (Top < 0)
    Top = 0;
  if (Right > m_iWidth)
    Right = m_iWidth;
  if (Bottom > m_iHeight)
    Bottom = m_iHeight;

  if (Opacity < 0.0 || Opacity > 1.0)
    return eBadParameter;

  // image to be merged
  ImageMapIterator it = m_Images.find (ImageId);

  if (it == m_Images.end ())
    return eImageNotInstalled;

  CBitmap * bitmap = it->second;

  // mask image
  it = m_Images.find (MaskId);

  if (it == m_Images.end ())
    return eImageNotInstalled;

  CBitmap * mask_bitmap = it->second;

  BITMAP  bi;
  bitmap->GetBitmap(&bi);
                                                  
  // calculate size of desired rectangle
  long iWidth = FixRight (Right) - Left;
  long iHeight = FixBottom (Bottom) - Top;

  // constrain to what we actually have
  if (SrcLeft < 0)
    SrcLeft = 0;
  if (SrcTop < 0)
    SrcTop = 0;
  if (SrcRight > bi.bmWidth )
    SrcRight = bi.bmWidth ;
  if (SrcBottom > bi.bmHeight)
    SrcBottom = bi.bmHeight;

  // adjust so that -1 means 1 from right
  if (SrcRight <= 0) 
    SrcRight = bi.bmWidth + SrcRight;

  if (SrcBottom <= 0) 
    SrcBottom =  bi.bmHeight + SrcBottom;

  // width and height are the smaller of the two rectangles
  iWidth = min (iWidth, SrcRight - SrcLeft);
  iHeight = min (iHeight, SrcBottom - SrcTop);

  if (iWidth <= 0 || iHeight <= 0)   // sanity check
    return eOK;

  mask_bitmap->GetBitmap(&bi);

  // adjust down in case we don't use whole image
  SrcRight = SrcLeft + iWidth;
  SrcBottom = SrcTop + iHeight;

  // mask must at least be as big as the image we are merging
  if (bi.bmWidth < SrcRight)
    return eBadParameter;
  if (bi.bmHeight < SrcBottom)
    return eBadParameter;


  // merge layer (from image id)
  CDC A_DC;
  A_DC.CreateCompatibleDC(&dc);

  BITMAPINFO bmi;
  ZeroMemory (&bmi, sizeof bmi);

  bmi.bmiHeader.biSize = sizeof bmi;
  bmi.bmiHeader.biWidth =          iWidth;       
  bmi.bmiHeader.biHeight =         iHeight;
  bmi.bmiHeader.biPlanes =         1;
  bmi.bmiHeader.biBitCount =       24;
  bmi.bmiHeader.biCompression =    BI_RGB;
  bmi.bmiHeader.biSizeImage =      iHeight * BytesPerLine (iWidth, 24);

  unsigned char * pA = NULL;

  HBITMAP hbmA = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**) &pA, NULL, 0);

  HBITMAP hOldAbmp = (HBITMAP) SelectObject(A_DC.m_hDC, hbmA);

  CDC bmDC;   // for loading bitmaps into
  bmDC.CreateCompatibleDC(&dc);

  //copy part from image to upper layer
  CBitmap *pOldbmp = bmDC.SelectObject(bitmap);
  A_DC.BitBlt (0, 0, iWidth, iHeight, &bmDC, SrcLeft, SrcTop, SRCCOPY);  
  bmDC.SelectObject(pOldbmp);


  // base image (from miniwindow)

  CDC B_DC;
  B_DC.CreateCompatibleDC(&dc);
  CBitmap B_bmp;

  unsigned char * pB = NULL;

  HBITMAP hbmB = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**) &pB, NULL, 0);

  HBITMAP hOldBbmp = (HBITMAP) SelectObject(B_DC.m_hDC, hbmB);

  // copy base image from miniwindow to bitmap
  B_DC.BitBlt (0, 0, iWidth, iHeight, &dc, Left, Top, SRCCOPY);  


  // mask image 

  CDC M_DC;
  M_DC.CreateCompatibleDC(&dc);
  CBitmap M_bmp;

  unsigned char * pM = NULL;

  HBITMAP hbmM = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**) &pM, NULL, 0);

  HBITMAP hOldMbmp = (HBITMAP) SelectObject(M_DC.m_hDC, hbmM);

  // copy mask image from image to bitmap
  pOldbmp = bmDC.SelectObject(mask_bitmap);
  M_DC.BitBlt (0, 0, iWidth, iHeight, &bmDC, SrcLeft, SrcTop, SRCCOPY);  
  bmDC.SelectObject(pOldbmp);

  long count = bmi.bmiHeader.biSizeImage;
  long perline = BytesPerLine (iWidth, 24);

  long i;

  // do the merge

#define Blend_Mask(A,B,M) ((uint8) ((A * M + B * (255 - M)) / 255) )

  switch (Mode)
    {
    case 0:     // normal
      if (Opacity < 1.0) 
        {
        for (i = 0; i < count; i++) 
          {
          pB [i] = Simple_Opacity (pB [i], Blend_Mask (pA [i], pB [i], pM [i]), Opacity);
          }
        }
      else
        {
        for (i = 0; i < count; i ++)
          pB [i]  = Blend_Mask (pA [i], pB [i], pM [i]);
        }
      break;  // end of mode 0

    case 1:  // transparent on pixel at 0,0
      {
      COLORREF opaque = ::GetPixel (A_DC, 0, 0);

      long row;
     for (row = 0; row < iHeight; row++)
      {
      long base = row * perline;
      unsigned char rA, gA, bA, rB, gB, bB; 
      if (Opacity < 1.0) 
        for (i = 0; i < perline - 2; ) 
          {
          bA = pA [base + i];     
          gA = pA [base + i + 1]; 
          rA = pA [base + i + 2]; 
          bB = pB [base + i];     
          gB = pB [base + i + 1]; 
          rB = pB [base + i + 2]; 

          // if this pixel is the opaque one, take the base pixel instead
          if (RGB (rA, gA, bA) == opaque)
            {
            rA = rB;
            gA = gB;
            bA = bB;
            } 

          pB [base + i] = Simple_Opacity (bB, Blend_Mask (bA, bB, pM [base + i]), Opacity);
          i++;

          pB [base + i] = Simple_Opacity (gB, Blend_Mask (gA, gB, pM [base + i]), Opacity);
          i++;
          
          pB [base + i] = Simple_Opacity (rB, Blend_Mask (rA, rB, pM [base + i]), Opacity);
          i++;
          }
      else
        for (i = 0; i < perline - 2;)
          {
          bA = pA [base + i];     
          gA = pA [base + i + 1]; 
          rA = pA [base + i + 2]; 
          bB = pB [base + i];     
          gB = pB [base + i + 1]; 
          rB = pB [base + i + 2]; 

          // if this pixel is the opaque one, take the base pixel instead
          if (RGB (rA, gA, bA) == opaque)
            {
            rA = rB;
            gA = gB;
            bA = bB;
            }

          pB [base + i] = Blend_Mask (bA, bB, pM [base + i]);
          i++;

          pB [base + i] = Blend_Mask (gA, gB, pM [base + i]);
          i++;
          
          pB [base + i] = Blend_Mask (rA, rB, pM [base + i]);
          i++;
          }   // end Opacity == 1
        }     // end for loop


      }
      break;    // end of mode 1

    default: return eUnknownOption;

    } // end of switch on Mode


  // copy result back

  dc.BitBlt (Left, Top, iWidth, iHeight, &B_DC, 0, 0, SRCCOPY);  


  SelectObject(A_DC.m_hDC, hOldAbmp);
  SelectObject(B_DC.m_hDC, hOldBbmp);
  SelectObject(M_DC.m_hDC, hOldMbmp);

  DeleteObject (hbmA);
  DeleteObject (hbmB);
  DeleteObject (hbmM);

  return eOK;

  } // end of CMiniWindow::MergeImageAlpha

static CString strMXP_menu_item [MXP_MENU_COUNT];

CMenu* GrabAMenu (void)
  {
  CMenu menu;
	VERIFY(menu.LoadMenu(IDR_MXP_MENU));

	CMenu* pPopup = menu.GetSubMenu(0);
  if (pPopup == NULL)
    return NULL;
  menu.Detach ();    // CMenu is temporary on stack
  pPopup->DeleteMenu (0, MF_BYPOSITION);  // get rid of dummy item

  return pPopup;
  }   // end of GrabAMenu

CString CMiniWindow::Menu(long Left, long Top, LPCTSTR Items, CMUSHView* pView)
  {
CString strResult;

  // can't menu if not visible
  if (!m_bShow || m_bTemporarilyHide)
    return strResult;

  // can't if outside window
  if (Left < 0 || Left > m_iWidth ||
      Top < 0 || Top > m_iHeight)
      return strResult;

  // make relative to miniwindow
  Left += m_rect.left;
  Top  += m_rect.top;

  vector<string> v;

  StringToVector (Items, v, "|");

  int iCount = v.size ();

  // must have at least one item
  if (iCount < 1)
    return strResult;

#if 0 // AARRRRRRRRGGGGGGGGGGGHHHHHHHHHHHHHHH!!!!!!!!!!!!!!!!!!!

  // if we are doing this in response to a mouse-down, we have to cancel the
  // mouse-down event now, or things get confused

  if (!pView->m_sPreviousMiniWindow.empty ())
    {
    CMUSHclientDoc* pDoc = pView->GetDocument();

    MiniWindowMapIterator it = pDoc->m_MiniWindows.find (pView->m_sPreviousMiniWindow);
  
    if (it != pDoc->m_MiniWindows.end ())
      {

      CMiniWindow * old_mw = it->second;

      // cancel previous move-down hotspot
      if (!old_mw->m_sMouseDownHotspot.empty ())   // HotspotId was used
        {
        // lookup that HotspotId
        HotspotMapIterator it = old_mw->m_Hotspots.find (old_mw->m_sMouseDownHotspot);

        // call CancelMouseDown for that hotspot, if it exists
        if (it != old_mw->m_Hotspots.end ())
          {
          m_last_mouseup = m_last_mousemove;
          pView->Send_Mouse_Event_To_Plugin (old_mw->m_sCallbackPlugin,
                                      it->second->m_sCancelMouseDown, 
                                      old_mw->m_sMouseDownHotspot);
          }
        old_mw->m_sMouseDownHotspot.erase ();  // no mouse-down right now
        }   // we had previous hotspot

      } // previous window still exists

    pView->m_sPreviousMiniWindow.erase ();  // no longer have a previous window
	  ReleaseCapture();   // Release the mouse capture established at
    	    					    // the beginning of the mouse click.

    } // released mouse in different window

#endif    // AARRRRRRRRGGGGGGGGGGGHHHHHHHHHHHHHHH!!!!!!!!!!!!!!!!!!!



CPoint menupoint (Left, Top);


  list<CMenu*> vPopup;    // nested menus

	CMenu* pPopup = GrabAMenu ();   // top level
  if (pPopup == NULL)
    return strResult;     // doesn't exist, strange
  vPopup.push_back (pPopup);

	CWnd* pWndPopupOwner = (CWnd *) pView;
	while (pWndPopupOwner->GetStyle() & WS_CHILD)
		pWndPopupOwner = pWndPopupOwner->GetParent();

  ClientToScreen(pView->m_hWnd, &menupoint);

  int j = 0;

  for (vector<string>::const_iterator i = v.begin (); i != v.end (); i++)
    {
    CString strItem = i->c_str ();

    if (*i == "-" || *i == "")
      vPopup.back ()->AppendMenu (MF_SEPARATOR, 0, "");
    else if (strItem.Left (1) == ">")    // nested menu
      {
    	CMenu* pNestedMenu = GrabAMenu ();   // nested menu
      if (pNestedMenu)
        {
        vPopup.back ()->AppendMenu (MF_POPUP | MF_ENABLED, (UINT ) pNestedMenu->m_hMenu, strItem.Mid (1));
        vPopup.push_back (pNestedMenu);  // add to menu stack
        }
      }
    else if (strItem.Left (1) == "<")    // un-nest menu
      {
      if (vPopup.size () > 1)
        vPopup.pop_back ();  // drop last item
      }
    else if (strItem.Left (1) == "^")
      vPopup.back ()->AppendMenu (MF_STRING | MF_GRAYED, 0, strItem.Mid (1));
    else
      {
      strMXP_menu_item [j] = strItem;
      vPopup.back ()->AppendMenu (MF_STRING | MF_ENABLED, MXP_FIRST_MENU + j, strItem);
      j++;
      if (j >= MXP_MENU_COUNT)
        break;
      }
    }

  // without this line the auto-enable always set "no items" to active
  Frame.m_bAutoMenuEnable  = FALSE;

	int iResult = vPopup.front ()->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, 
                          menupoint.x, 
                          menupoint.y,
			                    pWndPopupOwner);

  // put things back how they were
  Frame.m_bAutoMenuEnable  = TRUE;

  if (iResult > 0)
    strResult = strMXP_menu_item [iResult - MXP_FIRST_MENU];

  vPopup.front ()->DestroyMenu ();   // clean up

  return  strResult;  // will be empty for errors or cancelled


  }  // end of CMiniWindow::Menu

long CMiniWindow::DragHandler(CMUSHclientDoc * pDoc, LPCTSTR HotspotId, string sPluginID, LPCTSTR MoveCallback, LPCTSTR ReleaseCallback, long Flags)
  {

  if (strlen (MoveCallback) > 0 && CheckLabel (MoveCallback, true))
    return eInvalidObjectLabel;
  if (strlen (ReleaseCallback) > 0 && CheckLabel (ReleaseCallback, true))
    return eInvalidObjectLabel;

  // can't switch plugins here :)
  if (!m_sCallbackPlugin.empty () && m_sCallbackPlugin != sPluginID)
    return eHotspotPluginChanged;

  HotspotMapIterator it = m_Hotspots.find (HotspotId);

  if (it == m_Hotspots.end ())
    return eHotspotNotInstalled;   // no such hotspot

  CHotspot * pHotspot = it->second;

  pHotspot->m_sMoveCallback = MoveCallback;
  pHotspot->m_sReleaseCallback = ReleaseCallback;
  pHotspot->m_DragFlags = Flags;


  // if not in a plugin, look in main world for hotspot callbacks, and remember the dispatch ID
  if (sPluginID.empty ())
    {

    CString strErrorMessage;

    pHotspot->m_dispid_MoveCallback     = pDoc->GetProcedureDispid (MoveCallback, "mouse move", "", strErrorMessage);
    pHotspot->m_dispid_ReleaseCallback  = pDoc->GetProcedureDispid (ReleaseCallback, "mouse release", "", strErrorMessage);
    }

  return eOK;


  } // end of CMiniWindow::DragHandler


// changes hotspot tooltip text
long CMiniWindow::HotspotTooltip(LPCTSTR HotspotId, 
                             LPCTSTR TooltipText)
  {

  HotspotMapIterator it = m_Hotspots.find (HotspotId);

  if (it == m_Hotspots.end ())
    return eHotspotNotInstalled;

  CHotspot * pHotspot = it->second;

  pHotspot->m_sTooltipText      = TooltipText;

  return eOK;
  }    // end of CMiniWindow::HotspotTooltip

long CMiniWindow::DrawImageAlpha(LPCTSTR ImageId, 
                    long Left, long Top, long Right, long Bottom, 
                    double Opacity, 
                    long SrcLeft, long SrcTop)
  {

  // constrain to what we actually have
  if (Left < 0)
    Left = 0;
  if (Top < 0)
    Top = 0;
  if (Right > m_iWidth)
    Right = m_iWidth;
  if (Bottom > m_iHeight)
    Bottom = m_iHeight;

  if (Opacity < 0.0 || Opacity > 1.0)
    return eBadParameter;

  // image to be merged
  ImageMapIterator it = m_Images.find (ImageId);

  if (it == m_Images.end ())
    return eImageNotInstalled;

  CBitmap * bitmap = it->second;


  BITMAP  bi;
  bitmap->GetBitmap(&bi);

  // can't do it unless we have alpha channel
  if (bi.bmBitsPixel != 32)
    return eImageNotInstalled;

  // calculate size of desired rectangle
  long iWidth = FixRight (Right) - Left;
  long iHeight = FixBottom (Bottom) - Top;

  // constrain to what we actually have
  if (SrcLeft < 0)
    SrcLeft = 0;
  if (SrcTop < 0)
    SrcTop = 0;

  if (iWidth <= 0 || iHeight <= 0)   // sanity check
    return eOK;

  // merge layer (from image id)
  CDC A_DC;
  A_DC.CreateCompatibleDC(&dc);

  BITMAPINFO bmi;
  ZeroMemory (&bmi, sizeof bmi);

  bmi.bmiHeader.biSize = sizeof bmi;
  bmi.bmiHeader.biWidth =          iWidth;       
  bmi.bmiHeader.biHeight =         iHeight;
  bmi.bmiHeader.biPlanes =         1;
  bmi.bmiHeader.biBitCount =       32;
  bmi.bmiHeader.biCompression =    BI_RGB;
  bmi.bmiHeader.biSizeImage =      iHeight * BytesPerLine (iWidth, 32);

  unsigned char * pA = NULL;

  HBITMAP hbmA = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**) &pA, NULL, 0);

  HBITMAP hOldAbmp = (HBITMAP) SelectObject(A_DC.m_hDC, hbmA);

  CDC bmDC;   // for loading bitmaps into
  bmDC.CreateCompatibleDC(&dc);

  //copy part from image to upper layer
  CBitmap *pOldbmp = bmDC.SelectObject(bitmap);
  A_DC.BitBlt (0, 0, iWidth, iHeight, &bmDC, SrcLeft, SrcTop, SRCCOPY);  
  bmDC.SelectObject(pOldbmp);


  // base image (from miniwindow)

  CDC B_DC;
  B_DC.CreateCompatibleDC(&dc);
  CBitmap B_bmp;

  unsigned char * pB = NULL;

  HBITMAP hbmB = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**) &pB, NULL, 0);

  HBITMAP hOldBbmp = (HBITMAP) SelectObject(B_DC.m_hDC, hbmB);

  // copy base image from miniwindow to bitmap
  B_DC.BitBlt (0, 0, iWidth, iHeight, &dc, Left, Top, SRCCOPY);  


  long count = bmi.bmiHeader.biSizeImage;

  long i;

  // do the merge

#define Blend_Mask(A,B,M) ((uint8) ((A * M + B * (255 - M)) / 255) )

  if (Opacity < 1.0) 
    {
    for (i = 0; i < count; i += 4) 
      {
      pB [i]     = Simple_Opacity (pB [i], Blend_Mask (pA [i], pB [i], pA [i + 3]), Opacity);
      pB [i + 1] = Simple_Opacity (pB [i + 1], Blend_Mask (pA [i + 1], pB [i + 1], pA [i + 3]), Opacity);
      pB [i + 2] = Simple_Opacity (pB [i + 2], Blend_Mask (pA [i + 2], pB [i + 2], pA [i + 3]), Opacity);
      }
    }
  else
    {
    for (i = 0; i < count; i += 4)
      {
      pB [i]      = Blend_Mask (pA [i], pB [i], pA [i + 3]);
      pB [i + 1]  = Blend_Mask (pA [i + 1], pB [i + 1], pA [i + 3]);
      pB [i + 2]  = Blend_Mask (pA [i + 2], pB [i + 2], pA [i + 3]);
      }
    }


  // copy result back

  dc.BitBlt (Left, Top, iWidth, iHeight, &B_DC, 0, 0, SRCCOPY);  


  SelectObject(A_DC.m_hDC, hOldAbmp);
  SelectObject(B_DC.m_hDC, hOldBbmp);

  DeleteObject (hbmA);
  DeleteObject (hbmB);

  return eOK;


  } // end of  CMiniWindow::DrawImageAlpha

long CMiniWindow::GetImageAlpha(LPCTSTR ImageId, 
                    long Left, long Top, long Right, long Bottom, 
                    long SrcLeft, long SrcTop)
  {

  // constrain to what we actually have
  if (Left < 0)
    Left = 0;
  if (Top < 0)
    Top = 0;
  if (Right > m_iWidth)
    Right = m_iWidth;
  if (Bottom > m_iHeight)
    Bottom = m_iHeight;

  // image to be merged
  ImageMapIterator it = m_Images.find (ImageId);

  if (it == m_Images.end ())
    return eImageNotInstalled;

  CBitmap * bitmap = it->second;


  BITMAP  bi;
  bitmap->GetBitmap(&bi);

  // can't do it unless we have alpha channel
  if (bi.bmBitsPixel != 32)
    return eImageNotInstalled;

  // calculate size of desired rectangle
  long iWidth = FixRight (Right) - Left;
  long iHeight = FixBottom (Bottom) - Top;

  // constrain to what we actually have
  if (SrcLeft < 0)
    SrcLeft = 0;
  if (SrcTop < 0)
    SrcTop = 0;

  if (iWidth <= 0 || iHeight <= 0)   // sanity check
    return eOK;

  // merge layer (from image id)
  CDC A_DC;
  A_DC.CreateCompatibleDC(&dc);

  BITMAPINFO bmi;
  ZeroMemory (&bmi, sizeof bmi);

  bmi.bmiHeader.biSize = sizeof bmi;
  bmi.bmiHeader.biWidth =          iWidth;       
  bmi.bmiHeader.biHeight =         iHeight;
  bmi.bmiHeader.biPlanes =         1;
  bmi.bmiHeader.biBitCount =       32;
  bmi.bmiHeader.biCompression =    BI_RGB;
  bmi.bmiHeader.biSizeImage =      iHeight * BytesPerLine (iWidth, 32);

  unsigned char * pA = NULL;

  HBITMAP hbmA = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**) &pA, NULL, 0);

  HBITMAP hOldAbmp = (HBITMAP) SelectObject(A_DC.m_hDC, hbmA);

  CDC bmDC;   // for loading bitmaps into
  bmDC.CreateCompatibleDC(&dc);

  //copy part from image to upper layer
  CBitmap *pOldbmp = bmDC.SelectObject(bitmap);
  A_DC.BitBlt (0, 0, iWidth, iHeight, &bmDC, SrcLeft, SrcTop, SRCCOPY);  
  bmDC.SelectObject(pOldbmp);


  // base image (from miniwindow)

  CDC B_DC;
  B_DC.CreateCompatibleDC(&dc);
  CBitmap B_bmp;

  unsigned char * pB = NULL;

  HBITMAP hbmB = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**) &pB, NULL, 0);

  HBITMAP hOldBbmp = (HBITMAP) SelectObject(B_DC.m_hDC, hbmB);


  long count = bmi.bmiHeader.biSizeImage;

  long i;

  // copy alpha channel into window

  for (i = 0; i < count; i += 4)
    {
    pB [i]      = pA [i + 3];
    pB [i + 1]  = pA [i + 3];
    pB [i + 2]  = pA [i + 3];
    }


  // copy result back

  dc.BitBlt (Left, Top, iWidth, iHeight, &B_DC, 0, 0, SRCCOPY);  


  SelectObject(A_DC.m_hDC, hOldAbmp);
  SelectObject(B_DC.m_hDC, hOldBbmp);

  DeleteObject (hbmA);
  DeleteObject (hbmB);

  return eOK;


  } // end of  CMiniWindow::GetImageAlpha


long CMiniWindow::ScrollwheelHandler(CMUSHclientDoc * pDoc, LPCTSTR HotspotId, string sPluginID, LPCTSTR MoveCallback)
  {

  if (strlen (MoveCallback) > 0 && CheckLabel (MoveCallback, true))
    return eInvalidObjectLabel;

  // can't switch plugins here :)
  if (!m_sCallbackPlugin.empty () && m_sCallbackPlugin != sPluginID)
    return eHotspotPluginChanged;

  HotspotMapIterator it = m_Hotspots.find (HotspotId);

  if (it == m_Hotspots.end ())
    return eHotspotNotInstalled;   // no such hotspot

  CHotspot * pHotspot = it->second;

  pHotspot->m_sScrollwheelCallback = MoveCallback;


  // if not in a plugin, look in main world for hotspot callbacks, and remember the dispatch ID
  if (sPluginID.empty ())
    {

    CString strErrorMessage;

    pHotspot->m_dispid_ScrollwheelCallback     = pDoc->GetProcedureDispid (MoveCallback, "scroll wheel", "", strErrorMessage);
    }

  return eOK;


  } // end of CMiniWindow::ScrollwheelHandler


// resize a window

long CMiniWindow::Resize(long Width, long Height, long BackgroundColour)
  {

  // no change to size? wow, that was easy ...
  if (Width == m_iWidth && Height == m_iHeight)
    return eOK;

  // remember new width and height

  m_iWidth  = Width ;
  m_iHeight = Height;

  CDC bmDC;   // for loading bitmaps into
  bmDC.CreateCompatibleDC(&dc);

  // select original bitmap out of device context
  dc.SelectObject(m_oldBitmap);

  // save old bitmap for copying from
  CBitmap * previousWindowBitmap = m_Bitmap;

  // select into new device context
  CBitmap * pOldbmp = bmDC.SelectObject (previousWindowBitmap);

  // make new bitmap for different size
  m_Bitmap = new CBitmap;

  //  CreateBitmap with zero-dimensions creates a monochrome bitmap, so force to be at least 1x1
  m_Bitmap->CreateBitmap (MAX (m_iWidth, 1), MAX (m_iHeight, 1), 1, GetDeviceCaps(dc, BITSPIXEL), NULL); 
	m_oldBitmap = dc.SelectObject (m_Bitmap);
	dc.SetWindowOrg(0, 0);

  // fill with requested border colour
  dc.FillSolidRect (0, 0, m_iWidth, m_iHeight, BackgroundColour);

  // copy old contents back
  dc.BitBlt (0, 0, m_iWidth, m_iHeight, &bmDC, 0, 0, SRCCOPY);  
  bmDC.SelectObject(pOldbmp);

  // done with previous bitmap from this miniwindow
  previousWindowBitmap->DeleteObject ();
  delete previousWindowBitmap;

  return eOK;

  } // end of CMiniWindow::Resize



// move a hotspot (maybe the window was resized)

long CMiniWindow::MoveHotspot(LPCTSTR HotspotId, 
                             long Left, long Top, long Right, long Bottom)
  {

  HotspotMapIterator it = m_Hotspots.find (HotspotId);

  if (it == m_Hotspots.end ())
    return eHotspotNotInstalled;

  it->second->m_rect = CRect (Left, Top, FixRight (Right), FixBottom (Bottom));

  return eOK;
  }    // end of CMiniWindow::MoveHotspot



long CMiniWindow::TransformImage(LPCTSTR ImageId, float Left, float Top, short Mode, float Mxx, float Mxy, float Myx, float Myy)
  {

  ImageMapIterator it = m_Images.find (ImageId);

  if (it == m_Images.end ())
    return eImageNotInstalled;

  CBitmap * bitmap = it->second;

  dc.SetBkMode (TRANSPARENT);

  BITMAP  bi;
  bitmap->GetBitmap(&bi);

  CDC bmDC;
  bmDC.CreateCompatibleDC(&dc);
  CBitmap *pOldbmp = bmDC.SelectObject(bitmap);

  long iWidth  = bi.bmWidth;
  long iHeight = bi.bmHeight;

  if (iWidth <= 0 || iHeight <= 0)   // Sanity Claus
    {
    bmDC.SelectObject(pOldbmp);
    return eOK;
    }

  // need advanced mode to do SetWorldTransform successfully
  if (SetGraphicsMode (dc.m_hDC, GM_ADVANCED) == 0)
    {
    bmDC.SelectObject (pOldbmp);
    return eBadParameter;
    }

  // x' = x * Mxx + y * Mxy + Left 
  // y' = x * Myx + y * Myy + Top 

	XFORM xform;
	xform.eM11 = Mxx;       // cosine
	xform.eM21 = Mxy;       // sine
	xform.eM12 = Myx;       // - sine
	xform.eM22 = Myy;       // cosine
	xform.eDx = Left;
	xform.eDy = Top;

	if (SetWorldTransform (dc.m_hDC, &xform) == 0)
    {
    bmDC.SelectObject(pOldbmp);
    return eBadParameter;
    }

  // gives smoother rotations, especially with transparency
  dc.SetStretchBltMode (HALFTONE);  
  SetBrushOrgEx(dc.m_hDC, 0, 0, NULL);  // as recommended after  SetStretchBltMode

  switch (Mode)
    {
    case 1: dc.BitBlt (0, 0, iWidth, iHeight, &bmDC, 0, 0, SRCCOPY);  
            break;      // straight copy

    case 3:        // transparency, nom nom nom!
      {
      COLORREF crOldBack = dc.SetBkColor (RGB (255, 255, 255));    // white
	    COLORREF crOldText = dc.SetTextColor (RGB (0, 0, 0));        // black
	    CDC dcTrans;   // transparency mask


	    // Create a memory dc for the mask
	    dcTrans.CreateCompatibleDC(&dc);

	    // Create the mask bitmap 
	    CBitmap bitmapTrans;
	    bitmapTrans.CreateBitmap(m_iWidth, m_iHeight, 1, 1, NULL);

	    // Select the mask bitmap into the appropriate dc
	    CBitmap* pOldBitmapTrans = dcTrans.SelectObject(&bitmapTrans);

      // Our transparent pixel will be at 0,0 (top left corner) of original image 
      COLORREF crOldBackground = bmDC.SetBkColor (::GetPixel (bmDC, 0, 0));

	    // Build mask based on transparent colour at location 0, 0
	    dcTrans.BitBlt (0, 0, iWidth, iHeight, &bmDC, 0, 0, SRCCOPY);

	    // Do the work 
	    dc.BitBlt (0, 0, iWidth, iHeight, &bmDC,    0, 0, SRCINVERT);
	    dc.BitBlt (0, 0, iWidth, iHeight, &dcTrans, 0, 0, SRCAND);
	    dc.BitBlt (0, 0, iWidth, iHeight, &bmDC,    0, 0, SRCINVERT);

	    // Restore settings
	    dcTrans.SelectObject(pOldBitmapTrans);
	    dc.SetBkColor(crOldBack);
	    dc.SetTextColor(crOldText);
      bmDC.SetBkColor(crOldBackground);
      }
      break;

    default: 
      bmDC.SelectObject(pOldbmp);
      ModifyWorldTransform(dc.m_hDC, &xform, MWT_IDENTITY);
      SetGraphicsMode (dc.m_hDC, GM_COMPATIBLE);
      return eBadParameter;

    } // end of switch

  bmDC.SelectObject(pOldbmp);

  // reset to identity transformation
  ModifyWorldTransform(dc.m_hDC, &xform, MWT_IDENTITY);
  SetGraphicsMode (dc.m_hDC, GM_COMPATIBLE);

  return eOK;
  }   // end of CMiniWindow::TransformImage
