// methods_miniwindows.cpp

// Methods for miniwindows - main implementation in miniwindows.cpp

#include "stdafx.h"
#include "..\..\MUSHclient.h"
#include "..\..\doc.h"
#include "..\..\MUSHview.h"
#include "..\..\blending.h"
#include "..\..\color.h"
#include "..\errors.h"

// Implements:

//    BlendPixel
//    FilterPixel
//    WindowAddHotspot
//    WindowArc
//    WindowBezier
//    WindowBlendImage
//    WindowCircleOp
//    WindowCreate
//    WindowCreateImage
//    WindowDelete
//    WindowDeleteAllHotspots
//    WindowDeleteHotspot
//    WindowDragHandler
//    WindowDrawImage
//    WindowDrawImageAlpha
//    WindowFilter
//    WindowFont
//    WindowFontInfo
//    WindowFontList
//    WindowGetImageAlpha
//    WindowGetPixel
//    WindowGradient
//    WindowHotspotInfo
//    WindowHotspotList
//    WindowHotspotTooltip
//    WindowImageFromWindow
//    WindowImageInfo
//    WindowImageList
//    WindowImageOp
//    WindowInfo
//    WindowLine
//    WindowList
//    WindowLoadImage
//    WindowMenu
//    WindowMergeImageAlpha
//    WindowMoveHotspot
//    WindowPolygon
//    WindowPosition
//    WindowRectOp
//    WindowResize
//    WindowScrollwheelHandler
//    WindowSetPixel
//    WindowSetZOrder
//    WindowShow
//    WindowText
//    WindowTextWidth
//    WindowTransformImage
//    WindowWrite


/* positions: 

  0 = stretch to output view size 
  1 = stretch with aspect ratio

  2 = stretch to owner size 
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
long CMUSHclientDoc::WindowCreate(LPCTSTR Name, 
                                  long Left, long Top, 
                                  long Width, long Height, 
                                  short Position, 
                                  long Flags, 
                                  long BackgroundColour) 
  {

  if (strlen (Name) == 0) 
    return eNoNameSpecified;    // empty name throws out various things

  if (Width < 0 || Height < 0)
    return eBadParameter;

  MiniWindowMapIterator it = m_MiniWindows.find (Name);

  CMiniWindow * pMiniWindow = NULL;

  if (it == m_MiniWindows.end ())
    {
    pMiniWindow = new CMiniWindow ();
    m_MiniWindows [Name] = pMiniWindow;
    }
  else
    pMiniWindow = it->second;

  pMiniWindow->Create (Left, Top, Width, Height,
                       Position, Flags, 
                       BackgroundColour);

  pMiniWindow->m_sCreatingPlugin.erase ();
  if (m_CurrentPlugin)
    pMiniWindow->m_sCreatingPlugin = m_CurrentPlugin->m_strID;

  SortWindows ();  // need to re-make sorted list in Z-order
  UpdateAllViews (NULL);

	return eOK;
}     // end of CMUSHclientDoc::WindowCreate

// set/clear the show flag so the window becomes visible
long CMUSHclientDoc::WindowShow(LPCTSTR Name, BOOL Show) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  it->second->Show (Show);

  UpdateAllViews (NULL);

	return eOK;
}     // end of CMUSHclientDoc::WindowShow

/*

------------------------------------------------------------------------------------
See: http://www.functionx.com/win32/Lesson11.htm

  for some nice examples of what they look like
------------------------------------------------------------------------------------

  */

// various rectangle operations
long CMUSHclientDoc::WindowRectOp(LPCTSTR Name, short Action, 
                                  long Left, long Top, long Right, long Bottom, 
                                  long Colour1, long Colour2) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->RectOp (Action, 
                             Left, Top, Right, Bottom, 
                             Colour1, Colour2);

}   // end of CMUSHclientDoc::WindowRectOp


// various circle/ellipse/pie operations
long CMUSHclientDoc::WindowCircleOp(LPCTSTR Name, short Action, 
                                    long Left, long Top, long Right, long Bottom, 
                                    long PenColour, long PenStyle, long PenWidth, 
                                    long BrushColour, long BrushStyle, 
                                    long Extra1, long Extra2, long Extra3, long Extra4) 
{
  if (strlen (Name) == 0) 
    return eNoNameSpecified;    // empty name throws out various things

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->CircleOp (Action, 
                               Left, Top, Right, Bottom, 
                               PenColour, PenStyle, PenWidth, 
                               BrushColour, BrushStyle,
                               Extra1, Extra2, Extra3, Extra4);

}     // end of CMUSHclientDoc::WindowCircleOp 


// add a font to our map of fonts by name (eg. "inventory")
long CMUSHclientDoc::WindowFont(LPCTSTR Name,        // which window
                                LPCTSTR FontId,      // eg. Inventory
                                LPCTSTR FontName,    // eg. Courier New
                                double Size,         // eg. 9 pt
                                BOOL Bold, BOOL Italic, BOOL Underline, BOOL Strikeout, 
                                short Charset, 
                                short PitchAndFamily) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Font (FontId, FontName,
                           Size, 
                           Bold, Italic, Underline, Strikeout, 
                           Charset, PitchAndFamily);
}    // end of CMUSHclientDoc::WindowFont

// output text, ordinary or UTF8 - returns length of text
long CMUSHclientDoc::WindowText(LPCTSTR Name,      // which window
                                LPCTSTR FontId,    // eg. Inventory
                                LPCTSTR Text,      // what to say
                                long Left, long Top, long Right, long Bottom,   // where to put it
                                long Colour,       // colour to show it in
                                BOOL Unicode)      // true if UTF8
{
  if (strlen (Name) == 0) 
    return -1;    // empty name throws out various things

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return -1;

  return it->second->Text (FontId, Text,  Left,  Top,  Right,  Bottom, Colour, Unicode );
}    // end of CMUSHclientDoc::WindowText 

// measure text, ordinary or UTF8
long CMUSHclientDoc::WindowTextWidth(LPCTSTR Name, LPCTSTR FontId, LPCTSTR Text, BOOL Unicode) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return -1;

  return it->second->TextWidth (FontId, Text, Unicode );
}     // end of CMUSHclientDoc::WindowTextWidth

// return info about the select font
VARIANT CMUSHclientDoc::WindowFontInfo(LPCTSTR Name, LPCTSTR FontId, long InfoType) 
{

	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;  

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return vaResult;

  it->second->FontInfo (FontId, InfoType, vaResult);
	return vaResult;
}    // end of CMUSHclientDoc::WindowFontInfo

// draws a straight line
long CMUSHclientDoc::WindowLine(LPCTSTR Name, 
                                long x1, long y1, long x2, long y2, 
                                long PenColour, long PenStyle, long PenWidth) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Line (x1, y1, x2, y2, PenColour, PenStyle, PenWidth);
}


// draws an arc
long CMUSHclientDoc::WindowArc(LPCTSTR Name, 
                               long Left, long Top, long Right, long Bottom, 
                               long x1, long y1, 
                               long x2, long y2, 
                               long PenColour, long PenStyle, long PenWidth) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Arc (Left, Top, Right, Bottom, x1, y1, x2, y2, PenColour, PenStyle, PenWidth);
}      // end of CMUSHclientDoc::WindowArc


// return list of windows we have made
VARIANT CMUSHclientDoc::WindowList() 
{
  COleSafeArray sa;   // for array list

  long iCount = 0;
  
  // put the arrays into the array
  if (!m_MiniWindows.empty ())    // cannot create empty dimension
    {
    sa.CreateOneDim (VT_VARIANT, m_MiniWindows.size ());

    for (MiniWindowMapIterator it = m_MiniWindows.begin (); 
         it != m_MiniWindows.end ();
         it++)
           {
            // the array must be a bloody array of variants, or VBscript kicks up
            COleVariant v (it->first.c_str ());
            sa.PutElement (&iCount, &v);
            iCount++;
           }

    } // end of having at least one

	return sa.Detach ();
}    // end of CMUSHclientDoc::WindowList

// return list of fonts created for this window
VARIANT CMUSHclientDoc::WindowFontList(LPCTSTR Name) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;  

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return vaResult;

  it->second->FontList (vaResult);
	return vaResult;
}    // end of CMUSHclientDoc::WindowFontList

// information about a window
VARIANT CMUSHclientDoc::WindowInfo(LPCTSTR Name, long InfoType) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;  

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return vaResult;

  it->second->Info (InfoType, vaResult);
	return vaResult;
}   // end of CMUSHclientDoc::WindowInfo

// load an image from disk
long CMUSHclientDoc::WindowLoadImage(LPCTSTR Name, 
                                     LPCTSTR ImageId, 
                                     LPCTSTR FileName) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->LoadImage (ImageId, FileName);
}   // end of  CMUSHclientDoc::WindowLoadImage

// load an image from memory
long CMUSHclientDoc::WindowLoadImageMemory(LPCTSTR Name, 
                                           LPCTSTR ImageId, 
                                           unsigned char * Buffer, 
                                           const size_t Length,
                                           const bool bAlpha) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->LoadImageMemory (ImageId, Buffer, Length, bAlpha);
}   // end of  CMUSHclientDoc::WindowLoadImageMemory


long CMUSHclientDoc::WindowWrite(LPCTSTR Name, LPCTSTR FileName) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Write (FileName);
}   // CMUSHclientDoc::WindowWrite


// draw an image
long CMUSHclientDoc::WindowDrawImage(LPCTSTR Name, 
                                     LPCTSTR ImageId, 
                                     long Left, long Top, long Right, long Bottom, 
                                     short Mode,
                                     long SrcLeft, long SrcTop, long SrcRight, long SrcBottom) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->DrawImage (ImageId, Left, Top, Right, Bottom, Mode, 
                                SrcLeft,   SrcTop,   SrcRight,   SrcBottom);
}     // end of CMUSHclientDoc::WindowDrawImage

// list loaded images
VARIANT CMUSHclientDoc::WindowImageList(LPCTSTR Name) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;  

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return vaResult;

  it->second->ImageList (vaResult);
	return vaResult;
}   // end of CMUSHclientDoc::WindowImageList


// info about an image
VARIANT CMUSHclientDoc::WindowImageInfo(LPCTSTR Name, LPCTSTR ImageId, long InfoType) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;  

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return vaResult;

  it->second->ImageInfo (ImageId, InfoType, vaResult);
	return vaResult;
}  // end of CMUSHclientDoc::WindowImageInfo



// draw a bezier curve
long CMUSHclientDoc::WindowBezier(LPCTSTR Name, 
                                  LPCTSTR Points, 
                                  long PenColour, long PenStyle, long PenWidth) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Bezier (Points, PenColour, PenStyle, PenWidth);
}  // end of CMUSHclientDoc::WindowBezier


// draw an open or closed polygon
long CMUSHclientDoc::WindowPolygon(LPCTSTR Name, 
                                   LPCTSTR Points, 
                                   long PenColour, long PenStyle, long PenWidth, 
                                   long BrushColour, long BrushStyle,
                                   BOOL Close,       // close the polygon
                                   BOOL Winding)     // ALTERNATE or WINDING fill
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Polygon (Points, PenColour, PenStyle, PenWidth, BrushColour, BrushStyle, Close, Winding);
}  // end of CMUSHclientDoc::WindowPolygon


// move an existing window
long CMUSHclientDoc::WindowPosition(LPCTSTR Name, 
                                    long Left, long Top, 
                                    short Position, 
                                    long Flags) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  long status = it->second->Position (Left, Top, Position, Flags);

  if (status == eOK)
    UpdateAllViews (NULL);

  return status;

}    // end of CMUSHclientDoc::WindowPosition


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

  */


// add a hotspot for handling mouse-over, mouse up/down events
long CMUSHclientDoc::WindowAddHotspot(LPCTSTR Name, 
                                     LPCTSTR HotspotId, 
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
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

static bool bInWindowAddHotspot = false;

  // don't recurse into infinite loops
  if (bInWindowAddHotspot)
    return eItemInUse;

  bInWindowAddHotspot = true;

  string sPluginID;

  if (m_CurrentPlugin)                            
    sPluginID = m_CurrentPlugin->m_strID;

  long status;

  status = it->second->AddHotspot (this, 
                               HotspotId, 
                               sPluginID,
                               Left, Top, Right, Bottom, 
                               MouseOver, 
                               CancelMouseOver, 
                               MouseDown, 
                               CancelMouseDown, 
                               MouseUp,
                               TooltipText,
                               Cursor,
                               Flags);

  // in mouse was over hotspot when it was created, do a "mouse move" to detect this
  if (status == eOK)
    {
    for(POSITION pos=GetFirstViewPosition();pos!=NULL;)
	    {
	    CView* pView = GetNextView(pos);
	    
	    if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
        ((CMUSHView*)pView)->Mouse_Move_MiniWindow (this, m_lastMousePosition);
      } // end of looping through views
    } // end of added hotspot OK

  bInWindowAddHotspot = false;

  return status;

}  // end of CMUSHclientDoc::WindowHotspot

// remove a previously-installed hotspot
long CMUSHclientDoc::WindowDeleteHotspot(LPCTSTR Name, LPCTSTR HotspotId) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->DeleteHotspot (HotspotId);
}   // end of CMUSHclientDoc::WindowDeleteHotspot

// return list of all hotspots in this miniwindow
VARIANT CMUSHclientDoc::WindowHotspotList(LPCTSTR Name) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;  

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return vaResult;

  it->second->HotspotList (vaResult);
	return vaResult;
}    // end of CMUSHclientDoc::WindowHotspotList

// delete all hotspots
long CMUSHclientDoc::WindowDeleteAllHotspots(LPCTSTR Name) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->DeleteAllHotspots ();
}    // end of CMUSHclientDoc::WindowDeleteAllHotspots


// get information about a hotspot
VARIANT CMUSHclientDoc::WindowHotspotInfo(LPCTSTR Name, LPCTSTR HotspotId, long InfoType) 
{
	VARIANT vaResult;
	VariantInit(&vaResult);

  vaResult.vt = VT_EMPTY;  

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return vaResult;

  it->second->HotspotInfo (HotspotId, InfoType, vaResult);
	return vaResult;
}      // end of CMUSHclientDoc::WindowHotspotInfo


long CMUSHclientDoc::WindowImageOp(LPCTSTR Name, short Action, 
                                   long Left, long Top, long Right, long Bottom, 
                                   long PenColour, long PenStyle, long PenWidth,
                                   long BrushColour,  // 0-bits are drawn in this colour
                                   LPCTSTR ImageId, 
                                   long EllipseWidth, long EllipseHeight) 
  {
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->ImageOp (Action, Left, Top, Right, Bottom, 
                              PenColour, PenStyle, PenWidth, 
                              BrushColour, ImageId, 
                              EllipseWidth, EllipseHeight);
}    // end of CMUSHclientDoc::WindowImageOp

long CMUSHclientDoc::WindowCreateImage(LPCTSTR Name, LPCTSTR ImageId, 
                                       long Row1, long Row2, long Row3, long Row4, long Row5, long Row6, long Row7, long Row8) 
{

  if (strlen (Name) == 0) 
    return eNoNameSpecified;    // empty name throws out various things

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->CreateImage (ImageId, Row1, Row2, Row3, Row4, Row5, Row6, Row7, Row8);
}     // end of CMUSHclientDoc::WindowCreateImage


long CMUSHclientDoc::WindowBlendImage(LPCTSTR Name, LPCTSTR ImageId, 
                                      long Left, long Top, long Right, long Bottom, 
                                      short Mode, double Opacity, 
                                      long SrcLeft, long SrcTop, long SrcRight, long SrcBottom) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->BlendImage (ImageId, Left, Top, Right, Bottom, Mode, Opacity, SrcLeft, SrcTop, SrcRight, SrcBottom);
}   // end of CMUSHclientDoc::WindowBlendImage


long CMUSHclientDoc::WindowImageFromWindow(LPCTSTR Name, LPCTSTR ImageId, LPCTSTR SourceWindow) 
  {
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  CMiniWindow * pDestWindow = it->second;

  it = m_MiniWindows.find (SourceWindow);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  CMiniWindow * pSrcWindow = it->second;

  return pDestWindow->ImageFromWindow (ImageId, pSrcWindow);
}  // end of CMUSHclientDoc::WindowImageFromWindow



long CMUSHclientDoc::WindowGradient(LPCTSTR Name, 
                                    long Left, long Top, long Right, long Bottom, 
                                    long StartColour, long EndColour, 
                                    short Mode) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Gradient (Left, Top, Right, Bottom, 
                                    StartColour, EndColour, 
                                    Mode);
}    // end of CMUSHclientDoc::WindowGradient

  
long CMUSHclientDoc::WindowFilter(LPCTSTR Name, long Left, long Top, long Right, long Bottom, 
                                  short Operation, double Options) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Filter (Left, Top, Right, Bottom, 
                                    Operation, Options);
}    // CMUSHclientDoc::WindowFilter
                                       

long CMUSHclientDoc::WindowSetPixel(LPCTSTR Name, long x, long y, long Colour) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->SetPixel (x, y, Colour);
}   // end of CMUSHclientDoc::WindowSetPixel


long CMUSHclientDoc::WindowSetZOrder(LPCTSTR Name, long Order) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  long status = it->second->SetZOrder (Order);

  SortWindows ();   // need to re-make sorted list in Z-order

  return status;   // now we can return status

}   // end of CMUSHclientDoc::WindowSetZOrder


long CMUSHclientDoc::WindowGetPixel(LPCTSTR Name, long x, long y) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return -2;

  return it->second->GetPixel (x, y);
}     // end of CMUSHclientDoc::WindowGetPixel



// see also: CMiniWindow::Filter

long CMUSHclientDoc::FilterPixel(long Pixel, short Operation, double Options) 
{
long r = GetRValue (Pixel),
     g = GetGValue (Pixel),
     b = GetBValue (Pixel);

   switch (Operation)
     {
     case 1:        // Noise
       {
       double threshold = Options / 100.0;
       r += (128 - genrand () * 256) * threshold;
       g += (128 - genrand () * 256) * threshold;
       b += (128 - genrand () * 256) * threshold;
       break;
       }

     case 2:    // MonoNoise
       {
       double threshold = Options / 100.0;
       long j = (128 - genrand () * 256) * threshold;
       r += j;
       g += j;
       b += j;
       break;
       }


     case  7: // Brightness     
        {
        r += Options;
        g += Options;
        b += Options;
        break;
        }


     case  8: // Contrast        
        {
        double c;

        c = r - 128;   // center on zero
        c *= Options;  // multiply by contrast
        r = c + 128;   // put back

        c = g - 128;   // center on zero
        c *= Options;  // multiply by contrast
        g = c + 128;   // put back

        c = b - 128;   // center on zero
        c *= Options;  // multiply by contrast
        b = c + 128;   // put back

        break;
        }


     case  9: // Gamma         
        {
        double c;

        if (Options < 0.0)
          Options = 0.0;

        c = ( (double) r) / 255.0;  // normalize it
        c = pow (c, Options);
        r = c * 255.0;

        c = ( (double) g) / 255.0;  // normalize it
        c = pow (c, Options);
        g = c * 255.0;

        c = ( (double) b) / 255.0;  // normalize it
        c = pow (c, Options);
        b = c * 255.0;

        break;
        }

     case  10: // ColourBrightness - red     
        {
        r += Options;
        break;
        }

     case  11: // ColourContrast - red        
        {
        double c;

        c = r - 128;   // center on zero
        c *= Options;  // multiply by contrast
        r = c + 128;   // put back

        break;
        }

     case  12: // ColourGamma - red         
        {
        double c;

        if (Options < 0.0)
          Options = 0.0;

        c = ( (double) r) / 255.0;  // normalize it
        c = pow (c, Options);
        r = c * 255.0;

        break;
        }

     case  13: // ColourBrightness - green     
        {
        g += Options;
        break;
        }

     case  14: // ColourContrast - green        
        {
        double c;

        c = g - 128;   // center on zero
        c *= Options;  // multiply by contrast
        g = c + 128;   // put back

        break;
        }

     case  15: // ColourGamma - green         
        {
        double c;

        if (Options < 0.0)
          Options = 0.0;

        c = ( (double) g) / 255.0;  // normalize it
        c = pow (c, Options);
        g = c * 255.0;

        break;
        }

     case  16: // ColourBrightness - blue     
        {
        b += Options;
        break;
        }

     case  17: // ColourContrast - blue        
        {
        double c;

        c = b - 128;   // center on zero
        c *= Options;  // multiply by contrast
        b = c + 128;   // put back

        break;
        }

     case  18: // ColourGamma - blue         
        {
        double c;

        if (Options < 0.0)
          Options = 0.0;

        c = ( (double) b) / 255.0;  // normalize it
        c = pow (c, Options);
        b = c * 255.0;

        break;
        }

     case 19: // MakeGreyscale  - linear
       {
        double c;
        c =  r + g + b;
        c /= 3;
        r = c;
        g = c;
        b = c;
        break;
       }

     case 20: // MakeGreyscale  - perceptual
       {
        double c;
        c = b * 0.11 + // blue   (perceptual)
            g * 0.59 + // green
            r * 0.30;  // red
        c /= 3;
        r = c;
        g = c;
        b = c;
        break;
       }

     case  21: // Brightness - multiplicative    
        {
        r *= Options;
        g *= Options;
        b *= Options;
        break;
        }

     case  22: // Brightness - multiplicative - red    
        {
        r *= Options;
        break;
        }

     case  23: // Brightness - multiplicative - green    
        {
        g *= Options;
        break;
        }

     case  24: // Brightness - multiplicative - blue    
        {
        b *= Options;
        break;
        }


     case 27:  // Average (of 1 pixel is itself)
       break;

     default: return -1;

     } // end of switch


	return RGB (CLAMP (r), CLAMP (g), CLAMP (b));
}   // end of CMUSHclientDoc::FilterPixel

#define Blend_It(Op) \
   do \
      if (Opacity < 1.0) \
        {  \
          r =  Blend_Opacity (rA, rB, Op, Opacity);\
          g =  Blend_Opacity (gA, gB, Op, Opacity);\
          b =  Blend_Opacity (bA, bB, Op, Opacity);\
        }  \
      else\
        {  \
          r = Op (rA, rB);\
          g = Op (gA, gB);\
          b = Op (bA, bB);\
        }  \
   while (false)


#define Colour_Op(fR,fG,fB) \
   do \
     {  \
      if (Opacity < 1.0) \
          {  \
          r = Simple_Opacity (rB, fR, Opacity); \
          g = Simple_Opacity (gB, fG, Opacity); \
          b = Simple_Opacity (bB, fB, Opacity); \
          }\
      else\
          {\
          r = fR;  \
          g = fG;  \
          b = fB;  \
          }\
       }\
   while (false)


// see also: CMiniWindow::BlendImage

long CMUSHclientDoc::BlendPixel(long Blend, long Base, short Mode, double Opacity) 
{
long rA = GetRValue (Blend),
     gA = GetGValue (Blend),
     bA = GetBValue (Blend);
long rB = GetRValue (Base),
     gB = GetGValue (Base),
     bB = GetBValue (Base);

long r, g, b;

  if (Opacity < 0.0 || Opacity > 1.0)
    return -2;

  // precompute cos table
  static unsigned char cos_table [256];
  static bool bTableComputed = false;

  if (!bTableComputed)
    {
    double pi_div255 = 3.1415926535898 / 255.0;
    long i;
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
    case  1: Blend_It (Blend_Normal);     break;
    case  2: Blend_It (Blend_Average);    break;
    case  3: Blend_It (Blend_Interpolate);  break;

    case  4:  // dissolve - randomly choose pixels based on opacity
      {
       double rnd = genrand ();

        r = (rnd < Opacity) ? rA : rB;
        g = (rnd < Opacity) ? gA : gB;
        b = (rnd < Opacity) ? bA : bB;
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
          CColor cA, cB;    // A = blend, B = base

          cA.SetRGB (rA, gA, bA);
          cB.SetRGB (rB, gB, bB);

          cB.SetHue (cA.GetHue ());  // hue of blend image, others from base image
          r = Simple_Opacity (rB, cB.GetRed (), Opacity); 
          g = Simple_Opacity (gB, cB.GetGreen (), Opacity); 
          b = Simple_Opacity (bB, cB.GetBlue (), Opacity); 
       }
      break;

    case 61:           // Saturation mode
        {
          CColor cA, cB;    // A = blend, B = base

          cA.SetRGB (rA, gA, bA);
          cB.SetRGB (rB, gB, bB);

          cB.SetSaturation (cA.GetSaturation ());  // saturation of blend image, others from base image
          r = Simple_Opacity (rB, cB.GetRed (), Opacity); 
          g = Simple_Opacity (gB, cB.GetGreen (), Opacity); 
          b = Simple_Opacity (bB, cB.GetBlue (), Opacity); 
       }
      break;

    case 62:           // Colour mode
      {
          CColor cA, cB;    // A = blend, B = base

          cA.SetRGB (rA, gA, bA);
          cB.SetRGB (rB, gB, bB);

          cB.SetHue (cA.GetHue ());  // hue of blend image, 
          cB.SetSaturation (cA.GetSaturation ());  // saturation of blend image, luminance from base image
          r = Simple_Opacity (rB, cB.GetRed (), Opacity); 
          g = Simple_Opacity (gB, cB.GetGreen (), Opacity); 
          b = Simple_Opacity (bB, cB.GetBlue (), Opacity); 
       }
      break;


    case 63:           // Luminance mode
      {
          CColor cA, cB;    // A = blend, B = base

          cA.SetRGB (rA, gA, bA);
          cB.SetRGB (rB, gB, bB);

          cB.SetLuminance (cA.GetLuminance ());  // luminance from blend image, others from base image

          r = Simple_Opacity (rB, cB.GetRed (), Opacity); 
          g = Simple_Opacity (gB, cB.GetGreen (), Opacity); 
          b = Simple_Opacity (bB, cB.GetBlue (), Opacity); 

       }
      break;

    case 64:           // HSL (hue to red, saturation to green, luminance to blue)
        {
          CColor cA, cB;    // A = blend, B = base
          cA.SetRGB (rA, gA, bA);

          r = Simple_Opacity (rB, cB.GetRed (), Opacity); 
          g = Simple_Opacity (gB, cB.GetGreen (), Opacity); 
          b = Simple_Opacity (bB, cB.GetBlue (), Opacity); 

         }
      break;

     default: return -1;

     } // end of switch


	return RGB (CLAMP (r), CLAMP (g), CLAMP (b));
} // end of CMUSHclientDoc::BlendPixel


long CMUSHclientDoc::WindowMergeImageAlpha(LPCTSTR Name, 
                                           LPCTSTR ImageId, LPCTSTR MaskId, 
                                           long Left, long Top, long Right, long Bottom, 
                                           short Mode, 
                                           double Opacity, 
                                           long SrcLeft, long SrcTop, long SrcRight, long SrcBottom) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->MergeImageAlpha (ImageId, MaskId, Left, Top, Right, Bottom, Mode, Opacity, SrcLeft, SrcTop, SrcRight, SrcBottom);
}    // end of CMUSHclientDoc::WindowMergeImageAlpha


long CMUSHclientDoc::WindowDelete(LPCTSTR Name) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  if (it->second->m_bExecutingScript)
    return eItemInUse;

  delete it->second;

  m_MiniWindows.erase (it);

  SortWindows ();   // need to re-make sorted list in Z-order

  UpdateAllViews (NULL);

	return eOK;
}    // end of CMUSHclientDoc::WindowDelete


BSTR CMUSHclientDoc::WindowMenu(LPCTSTR Name, long Left, long Top, LPCTSTR Items) 
{

  CString strResult;

  MiniWindowMapIterator it = m_MiniWindows.find (Name);

	CView* pView = NULL;
  
  for(POSITION pos = GetFirstViewPosition(); pos != NULL; )
	  {
	  pView = GetNextView(pos);

	  if (pView->IsKindOf(RUNTIME_CLASS(CMUSHView)))
      break;

    }

  if (pView && it != m_MiniWindows.end ())
    strResult = it->second->Menu (Left, Top, Items, (CMUSHView *) pView);

	return strResult.AllocSysString();

}   // end of CMUSHclientDoc::WindowMenu



long CMUSHclientDoc::WindowHotspotTooltip(LPCTSTR Name, LPCTSTR HotspotId, LPCTSTR TooltipText) 
{

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return eNoSuchWindow;

  return it->second->HotspotTooltip (HotspotId, TooltipText);
}    // end of CMUSHclientDoc::WindowHotspotTooltip

long CMUSHclientDoc::WindowDrawImageAlpha(LPCTSTR Name, LPCTSTR ImageId, long Left, long Top, long Right, long Bottom, double Opacity, long SrcLeft, long SrcTop) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->DrawImageAlpha (ImageId, Left, Top, Right, Bottom, Opacity, 
                                    SrcLeft,   SrcTop);

}  // end of CMUSHclientDoc::WindowDrawImageAlpha

long CMUSHclientDoc::WindowGetImageAlpha(LPCTSTR Name, LPCTSTR ImageId, long Left, long Top, long Right, long Bottom, long SrcLeft, long SrcTop) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->GetImageAlpha (ImageId, Left, Top, Right, Bottom, 
                                    SrcLeft,   SrcTop);

}   // end of CMUSHclientDoc::WindowGetImageAlpha



long CMUSHclientDoc::WindowResize(LPCTSTR Name, long Width, long Height, long BackgroundColour) 
{

  if (Width < 0 || Height < 0)
    return eBadParameter;

  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->Resize (Width, Height, BackgroundColour);
}  // end of CMUSHclientDoc::WindowResize
     

long CMUSHclientDoc::WindowMoveHotspot(LPCTSTR Name, LPCTSTR HotspotId, long Left, long Top, long Right, long Bottom) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
  	return eNoSuchWindow;

  return it->second->MoveHotspot (HotspotId, Left, Top, Right, Bottom);
}   // end of CMUSHclientDoc::WindowMoveHotspot


long CMUSHclientDoc::WindowTransformImage(LPCTSTR Name, LPCTSTR ImageId, float Left, float Top, short Mode, float Mxx, float Mxy, float Myx, float Myy) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  return it->second->TransformImage (ImageId, Left, Top, Mode, Mxx, Mxy, Myx, Myy);
}  // end of CMUSHclientDoc::WindowTransformImage



long CMUSHclientDoc::WindowDragHandler(LPCTSTR Name, LPCTSTR HotspotId, LPCTSTR MoveCallback, LPCTSTR ReleaseCallback, long Flags) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  string sPluginID;

  if (m_CurrentPlugin)                            
    sPluginID = m_CurrentPlugin->m_strID;

  return it->second->DragHandler (this, HotspotId, sPluginID, MoveCallback, ReleaseCallback, Flags);
}   // end of CMUSHclientDoc::WindowDragHandler


long CMUSHclientDoc::WindowScrollwheelHandler(LPCTSTR Name, LPCTSTR HotspotId, LPCTSTR MoveCallback) 
{
  MiniWindowMapIterator it = m_MiniWindows.find (Name);
    
  if (it == m_MiniWindows.end ())
    return eNoSuchWindow;

  string sPluginID;

  if (m_CurrentPlugin)                            
    sPluginID = m_CurrentPlugin->m_strID;

  return it->second->ScrollwheelHandler (this, HotspotId, sPluginID, MoveCallback);
}   // end of CMUSHclientDoc::WindowScrollwheelHandler

