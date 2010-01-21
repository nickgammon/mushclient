// genprint.h - stuff for generalised printing

extern BOOL bAborted;

/////////////////////////////////////////////////////////////////////////////
// Printing Dialog

class CMyPrintingDialog : public CDialog
{
public:

	//{{AFX_DATA(CPrintingDialog)
	enum { IDD = AFX_IDD_PRINTDLG };
	//}}AFX_DATA
	CMyPrintingDialog::CMyPrintingDialog(CWnd* pParent)
		{
			Create(CMyPrintingDialog::IDD, pParent);      // modeless !
      bAborted = FALSE;
    }
	virtual ~CMyPrintingDialog() { }
	virtual BOOL OnInitDialog();
	virtual void OnCancel();

};

BOOL CALLBACK PrintingAbortProc(HDC, int);

// --------------------------------------------------------------------------------

// Our printer control block

#define FONT_NORMAL 0x00
#define FONT_ITALIC 0x01
#define FONT_UNDERLINE 0x02
#define FONT_BOLD 0x04

// Use the above defines to get to any font, e.g. bold-italic would be FONT_BOLD | FONT_ITALIC

typedef struct
  {
  CPrintDialog * pd;    // printer dialog info
  CMyPrintingDialog * dlgPrintStatus;    // printing dialog info
  HDC hDC;           // print device context
  DOCINFO di;  // document info (document name, print file name)
  HWND hwnd;      // copy of the view's window handle
  char docname [256]; // name of document being printed
  BOOL initialised;   // true if we opened the printer OK
  BOOL ok;          // are we still printing OK?
  BOOL cancelled;   // did they cancel the print job?
  int init_left,    // initial left position
      init_top;     // initial top position
  int left,         // current left position
      top;          // current top position
  WORD current_page;  // current page number
  WORD pages_printed;
  long lines_printed;

  TEXTMETRIC  tm;
  int logpelsX,
      logpelsY,
      offsetX,
      offsetY;
  HFONT font [8];  // see defines above
  HFONT * current_font;
  int	m_nLineSpacing;

  int left_margin;
  int top_margin;
  int lines_per_page;
  int point_size;
  int printer_spacing;
  char printer_font [256];

  } t_print_control_block;


BOOL print_start_document (t_print_control_block & pcb,
                            const CString docname,
                            const WORD first_page,
                            const WORD last_page,
                            const int left_margin,
                            const int top_margin,
                            const int lines_per_page,
                            const int point_size,
                            const int printer_spacing,
                            const CString & printer_font,
                            const BOOL bSelection);
BOOL print_start_page (t_print_control_block & pcb);
BOOL print_printline (t_print_control_block & pcb, int skip, const char * theline, ...);
BOOL print_end_page (t_print_control_block & pcb);
BOOL print_end_document (t_print_control_block & pcb);

// font changing stuff

void print_font (t_print_control_block & pcb, const short font_type);

