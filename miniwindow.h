// for miniwindows

// fonts
typedef map<string, CFont *> FontMap;
typedef FontMap::iterator FontMapIterator;

// images
typedef map<string, CBitmap *> ImageMap;
typedef ImageMap::iterator ImageMapIterator;


// hotspots
class CHotspot
  {

  public:

  CHotspot () : m_Cursor (0),  m_Flags (0), m_DragFlags (0),
                m_dispid_MouseOver        (DISPID_UNKNOWN),
                m_dispid_CancelMouseOver  (DISPID_UNKNOWN),
                m_dispid_MouseDown        (DISPID_UNKNOWN),
                m_dispid_CancelMouseDown  (DISPID_UNKNOWN),
                m_dispid_MouseUp          (DISPID_UNKNOWN),
                m_dispid_MoveCallback     (DISPID_UNKNOWN),
                m_dispid_ReleaseCallback  (DISPID_UNKNOWN),
                m_dispid_ScrollwheelCallback (DISPID_UNKNOWN)
                {}  // constructor

  CRect  m_rect;           // where it is

  string m_sMouseOver;       // function to call on mouseover
  string m_sCancelMouseOver; // function to call when mouse moves away or is clicked

  string m_sMouseDown;       // mouse down here  (might cancel mouseover first)
  string m_sCancelMouseDown; // they let go somewhere else
  string m_sMouseUp;         // mouse up following a mouse-down in this hotspot

  string m_sTooltipText;     // tooltip text

  long   m_Cursor;           // what cursor to show
  long   m_Flags;            // flags

  string m_sMoveCallback;    // callback when mouse moves
  string m_sReleaseCallback; // callback when mouse released
  long   m_DragFlags;        // drag-and-drop flags

  string m_sScrollwheelCallback; // mouse-wheel (scroll wheel) moved over hotspot

  // dispids for calling functions from NOT in a plugin (ignored in a plugin)
  DISPID m_dispid_MouseOver;       // function to call on mouseover
  DISPID m_dispid_CancelMouseOver; // function to call when mouse moves away or is clicked
  DISPID m_dispid_MouseDown;       // mouse down here  (might cancel mouseover first)
  DISPID m_dispid_CancelMouseDown; // they let go somewhere else
  DISPID m_dispid_MouseUp;         // mouse up following a mouse-down in this hotspot
  DISPID m_dispid_MoveCallback;    // callback when mouse moves
  DISPID m_dispid_ReleaseCallback; // callback when mouse released
  DISPID m_dispid_ScrollwheelCallback; // callback when scroll wheel moved

  };   // end of class  CStringValuePair


typedef map<string, CHotspot *> HotspotMap;
typedef HotspotMap::iterator HotspotMapIterator;

// flags

#define MINIWINDOW_DRAW_UNDERNEATH 0x01        // draw underneath scrolling text
#define MINIWINDOW_ABSOLUTE_LOCATION 0x02      // use m_iLeft, m_iTop to position 
#define MINIWINDOW_TRANSPARENT 0x04            // draw transparently, using m_iBackgroundColour 
#define MINIWINDOW_IGNORE_MOUSE 0x08           // ignore mouse-down, mouse-over etc. 
#define MINIWINDOW_KEEP_HOTSPOTS 0x10          // retain existing hotspots

class CMiniWindow
  {
  private:

    CDC dc;                   // our offscreen device context
  	CBitmap *   m_oldBitmap;  // bitmap originally found in CMyMemDC
    CBitmap *   m_Bitmap;     // where it all happens
    FontMap     m_Fonts;      // all the fonts they want
    ImageMap    m_Images;     // other images they may want to blt onto the window

    long    m_iWidth;   
    long    m_iHeight;

    CPoint  m_Location;   // left, top position (if absolute position)
    short   m_iPosition;  // where to position it
    long    m_iFlags;     // see defines above
    COLORREF m_iBackgroundColour;

    bool    m_bShow;  // true = draw it


  public:
    CMiniWindow ();   // constructor

    ~CMiniWindow ();  // destructor


  CRect   m_rect;   // where we actually put it
  bool m_bTemporarilyHide;   // no room right now
  HotspotMap  m_Hotspots;   // where we can click with the mouse

  // where we last did things:
  CPoint m_last_mouseposition;
  long   m_last_mouse_update;

  CPoint m_client_mouseposition;

  CTime   m_tDateInstalled;   // date installed

  string  m_sMouseOverHotspot;    // last hotspot we moused over
  string  m_sMouseDownHotspot;    // last hotspot we mouse clicked in
  long    m_FlagsOnMouseDown;     // which mouse-down we got
  long    m_ZOrder;               // Z-order. If zero, use name order. Lower is drawn earlier.
  bool    m_bExecutingScript;      // so windows don't delete themselves

  string  m_sCallbackPlugin;      // plugin we are using
  string  m_sCreatingPlugin;      // plugin that created the miniwindow

  const long GetWidth ()      const { return m_iWidth; }
  const long GetHeight ()     const { return m_iHeight; }
  const short GetPosition ()  const { return m_iPosition; }
  const bool GetShow ()       const { return m_bShow; }
  const long GetFlags ()      const { return m_iFlags; }
  const CPoint GetLocation () const { return m_Location; }
  const COLORREF GetBackgroundColour () const { return m_iBackgroundColour; }
  const FontMap & GetFonts () const { return m_Fonts; }
  const ImageMap & GetImages () const { return m_Images; }

  void  Create (long Left, long Top, long Width, long Height,
                short Position, long Flags, 
                COLORREF iBackgroundColour);

  void  Show (bool bShow);
  
  CDC*  GetDC (void) { return &dc; }

  long FixRight (const long Right);
  long FixBottom (const long Bottom);

  void Info (long InfoType, VARIANT & vaResult);

  long  RectOp (short Action, long Left, 
                long Top, long Right, long Bottom, 
                long Colour1, long Colour2);

  long  CircleOp (short Action, 
                  long Left, long Top, long Right, long Bottom, 
                  long PenColour, long PenStyle, long PenWidth, 
                  long BrushColour, long BrushStyle,
                  long Extra1, long Extra2, long Extra3, long Extra4);

  long Font (LPCTSTR FontId, LPCTSTR FontName, 
             double Size, 
             BOOL Bold, BOOL Italic, BOOL Underline, BOOL Strikeout, 
             short Charset, short PitchAndFamily);

  void FontInfo (LPCTSTR FontId, long InfoType, VARIANT & vaResult);
  void FontList (VARIANT & vaResult);

  long Text (LPCTSTR FontId,  // which previously-created font
             LPCTSTR Text,   // what to say
             long Left, long Top, long Right, long Bottom, // where to put it
             long Colour,       // colour to show it in
             BOOL Unicode);      // true if UTF8

  long TextWidth (LPCTSTR FontId,  // which previously-created font
                   LPCTSTR Text,   // what to measure
                   BOOL Unicode);  // true if UTF8

  long Line (long x1, long y1, long x2, long y2, 
             long PenColour, long PenStyle, long PenWidth);
    
  long Arc (long Left, long Top, long Right, long Bottom, 
            long x1, long y1, 
            long x2, long y2, 
            long PenColour, long PenStyle, long PenWidth);

  long LoadImage (LPCTSTR ImageId, LPCTSTR FileName);
  long LoadPngImage (LPCTSTR ImageId, LPCTSTR FileName);

  long Write (LPCTSTR FileName);
  long WritePng (LPCTSTR FileName, const BITMAPINFO * bmi, unsigned char * pData);

  long DrawImage(LPCTSTR ImageId, 
                 long Left, long Top, long Right, long Bottom, 
                 short Mode,
                 long SrcLeft, long SrcTop, long SrcRight, long SrcBottom);
  
  void ImageList(VARIANT & vaResult);

  void ImageInfo(LPCTSTR ImageId, long InfoType, VARIANT & vaResult);

  long Bezier(LPCTSTR Points, long PenColour, long PenStyle, long PenWidth);

  long Polygon(LPCTSTR Points, 
               long PenColour, short PenStyle, long PenWidth, 
               long BrushColour, long BrushStyle, 
               BOOL Close,
               BOOL Winding);

  long Position(long Left, long Top, 
                short Position, 
                long Flags); 


  long AddHotspot(CMUSHclientDoc * pDoc,
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
                   long Flags);

  long DeleteHotspot(LPCTSTR HotspotId);
  void HotspotList(VARIANT & vaResult);
  long DeleteAllHotspots();

  void HotspotInfo(LPCTSTR HotspotId, 
                  long InfoType, VARIANT & vaResult);

  long ImageOp(short Action, 
              long Left, long Top, long Right, long Bottom, 
              long PenColour, long PenStyle, long PenWidth, 
              long BrushColour, LPCTSTR ImageId, 
              long EllipseWidth, long EllipseHeight);

  long CreateImage(LPCTSTR ImageId, 
                   long Row1, long Row2, long Row3, long Row4, long Row5, long Row6, long Row7, long Row8);

  long BlendImage(LPCTSTR ImageId, 
                  long Left, long Top, long Right, long Bottom, 
                  short Mode, double Opacity, 
                  long SrcLeft, long SrcTop, long SrcRight, long SrcBottom);

  long ImageFromWindow(LPCTSTR ImageId, CMiniWindow * pSrcWindow);

  long Gradient(long Left, long Top, long Right, long Bottom, 
                      long StartColour, long EndColour, 
                      short Mode); 

  long Filter(long Left, long Top, long Right, long Bottom, 
                    short Operation, double Options);

  long SetPixel(long x, long y, long Colour);

  long GetPixel(long x, long y);

  long MergeImageAlpha(LPCTSTR ImageId, LPCTSTR MaskId, 
                      long Left, long Top, long Right, long Bottom, 
                      short Mode, double Opacity, 
                      long SrcLeft, long SrcTop, long SrcRight, long SrcBottom);

  CString Menu(long Left, long Top, LPCTSTR Items, CMUSHView* pView);


  long DragHandler(CMUSHclientDoc * pDoc, 
                   LPCTSTR HotspotId, 
                   string sPluginID,
                   LPCTSTR MoveCallback, 
                   LPCTSTR ReleaseCallback, 
                   long Flags);

  long ScrollwheelHandler(CMUSHclientDoc * pDoc, 
                         LPCTSTR HotspotId, 
                         string sPluginID,
                         LPCTSTR MoveCallback);

  long HotspotTooltip(LPCTSTR HotspotId, 
                  LPCTSTR TooltipText);

  long LoadImageMemory(LPCTSTR ImageId, 
                       unsigned char * Buffer, 
                       const size_t Length,
                       const bool bAlpha);


  long DrawImageAlpha(LPCTSTR ImageId, 
                      long Left, long Top, long Right, long Bottom, 
                      double Opacity, 
                      long SrcLeft, long SrcTop); 

  long GetImageAlpha(LPCTSTR ImageId, 
                      long Left, long Top, long Right, long Bottom, 
                      long SrcLeft, long SrcTop); 

  long Resize(long Width, long Height, long BackgroundColour);

  long MoveHotspot(LPCTSTR HotspotId, long Left, long Top, long Right, long Bottom);

  long TransformImage(LPCTSTR ImageId, float Left, float Top, short Mode, float Mxx, float Mxy, float Myx, float Myy);

  long SetZOrder(long Order);

  };


typedef map<string, CMiniWindow *> MiniWindowMap;

typedef MiniWindowMap::iterator MiniWindowMapIterator;

typedef vector<pair<string, CMiniWindow *> > MiniWindowVector;

typedef MiniWindowVector::iterator MiniWindowVectorIterator;
typedef MiniWindowVector::reverse_iterator MiniWindowVectorReverseIterator;
