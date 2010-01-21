
// genprint.cpp - generalised printing

#include "stdafx.h"
#include <afxext.h>

#include "genprint.h"
#include "MUSHclient.h"
#include "doc.h"
#include "mainfrm.h"
  
#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
  
BOOL bAborted = FALSE;

/* +++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*                                                     */
/*       print_start_document                          */
/*                                                     */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++ */


// initiates a print job
   
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
                            const BOOL bSelection)
                            
  {
int err;
DWORD flags;
int i;

  ZeroMemory (&pcb, sizeof pcb);

  pcb.hwnd = Frame.GetSafeHwnd ();

  pcb.left_margin      = left_margin;
  pcb.top_margin       = top_margin; 
  pcb.lines_per_page   = lines_per_page;
  pcb.point_size       = point_size; 
  pcb.printer_spacing  = printer_spacing;
  strcpy (pcb.printer_font, printer_font);
  
  //
  // Initialize a PRINTDLG struct and call PrintDlg to allow user to
  //   specify various printing options...
  //

  flags = PD_RETURNDC;
  if (last_page == 0)
    flags |= PD_NOPAGENUMS;
  if (!bSelection)
     flags |= PD_NOSELECTION;
  else
     flags |= PD_SELECTION;
											   
// create new print dialog

  pcb.pd = new CPrintDialog (FALSE, flags, &Frame);

  pcb.pd->m_pd.nMinPage = pcb.pd->m_pd.nFromPage = first_page;
  pcb.pd->m_pd.nMaxPage = pcb.pd->m_pd.nToPage 	 = last_page;

  if (AfxGetApp ()->DoPrintDialog (pcb.pd) != IDOK)
   	{
	  delete pcb.pd;
    return TRUE;
	  }

  if (last_page == 0)
    {
    pcb.pd->m_pd.nFromPage  = 1;
    pcb.pd->m_pd.nToPage = 0xFFFF;
    } // end of no page range specified

// set up the DOCINFO field
  
  strcpy (pcb.docname, docname);
  pcb.di.lpszDocName = pcb.docname;
  pcb.di.lpszOutput = NULL;
  pcb.di.cbSize = sizeof (pcb.di);


// get a copy of the DC so we don't have to call GetPrinterDC all the time

  pcb.hDC = pcb.pd->GetPrinterDC ();

// set up an abort procedure so they can cancel the printing

  if (SetAbortProc (pcb.hDC, PrintingAbortProc) == SP_ERROR)
    {
  	delete pcb.pd;
    return TRUE;
    };

// start the document

  err = StartDoc  (pcb.hDC, &pcb.di);

  if (err == SP_ERROR)
    {
  	delete pcb.pd;
    pcb.pd = NULL;
    return TRUE;
    }

// disable main window while printing & init printing status dialog

  Frame.EnableWindow (FALSE);

// set up a progress dialog

  pcb.dlgPrintStatus = new CMyPrintingDialog (&Frame);

  pcb.dlgPrintStatus->SetDlgItemText(AFX_IDC_PRINT_DOCNAME, pcb.docname);

  pcb.dlgPrintStatus->SetDlgItemText(AFX_IDC_PRINT_PRINTERNAME, pcb.pd->GetDeviceName());

  CString strTemp;
  CString strPortName = pcb.pd->GetPortName();
  int nFormatID = AFX_IDS_PRINTONPORT;

  AfxFormatString1(strTemp, nFormatID, strPortName);
  pcb.dlgPrintStatus->SetDlgItemText(AFX_IDC_PRINT_PORTNAME, strTemp);

  pcb.dlgPrintStatus->ShowWindow(SW_SHOW);
  pcb.dlgPrintStatus->UpdateWindow();

  pcb.initialised = TRUE;
  pcb.current_page = 0;
  pcb.pages_printed = 0;
  pcb.lines_printed = 0;

  pcb.logpelsX = GetDeviceCaps (pcb.hDC, LOGPIXELSX);
  pcb.logpelsY = GetDeviceCaps (pcb.hDC, LOGPIXELSY);
  pcb.offsetX = GetDeviceCaps (pcb.hDC, PHYSICALOFFSETX);
  pcb.offsetY = GetDeviceCaps (pcb.hDC, PHYSICALOFFSETY);

// calculate initial top and left positions

double left = pcb.left_margin / 25.4 * pcb.logpelsX;       // 25.4 mm to an inch
double top = pcb.top_margin / 25.4 * pcb.logpelsY;

// adjust for non-printable area, and save as start of this page

  pcb.init_left = (long) left - pcb.offsetX; 
  pcb.init_top = (long) top - pcb.offsetY;     

// make sure not negative (i.e. off edge of printable area)

  if (pcb.init_left < 0)
    pcb.init_left = 0;

  if (pcb.init_top < 0)
    pcb.init_top = 0;

// calculate font height in device coordinates

double height = pcb.point_size / 72.0 * double (pcb.logpelsY);

LOGFONT lf;
                                                                        
  lf.lfHeight         =  (long) height;	// logical height of font    
  lf.lfWidth          =  0;	// logical average character width         
  lf.lfEscapement     =  0;	// angle of escapement                     
  lf.lfOrientation    =  0;	// base-line orientation angle             
  lf.lfWeight         =  FW_NORMAL	;	// font weight                   
  lf.lfItalic         =  FALSE;	// italic attribute flag               
  lf.lfUnderline      =  FALSE;	// underline attribute flag            
  lf.lfStrikeOut      =  FALSE;	// strikeout attribute flag            
  lf.lfCharSet        =  MUSHCLIENT_FONT_CHARSET;	// character set identifier      
  lf.lfOutPrecision   =  OUT_DEVICE_PRECIS;	// output precision        
  lf.lfClipPrecision  =  CLIP_DEFAULT_PRECIS;	// clipping precision    
  lf.lfQuality        =  DEFAULT_QUALITY;	// output quality            
  lf.lfPitchAndFamily =  MUSHCLIENT_FONT_FAMILY;	// pitch and family              
  strcpy (lf.lfFaceName, pcb.printer_font); 	// address of typeface name string       


// create 8 fonts (all possible combinations of bold, italic and underline)

  for (i = 0; i < 8; i++)
    {
    lf.lfItalic = (i & FONT_ITALIC) != 0;
    lf.lfUnderline = (i & FONT_UNDERLINE) != 0;
    if (i & FONT_BOLD)
      lf.lfWeight         =  FW_BOLD;	                 
    else      
      lf.lfWeight         =  FW_NORMAL;	                  
    
    pcb.font [i] =  CreateFontIndirect (&lf);
    if (!pcb.font [i])
      {
      ::TMessageBox ("Unable to create a font for printing");
      return TRUE;
      }
    }

	// Calc line spacing height
	CFont	fontSpacing;
	lf.lfHeight = -MulDiv(pcb.printer_spacing, pcb.logpelsY, 72);
	lf.lfWeight = FW_NORMAL;
	lf.lfItalic = FALSE;
	lf.lfUnderline = FALSE;
  lf.lfPitchAndFamily =  MUSHCLIENT_FONT_FAMILY;	// pitch and family              
	fontSpacing.CreateFontIndirect(&lf);
	::SelectObject(pcb.hDC, fontSpacing.GetSafeHandle());
	TEXTMETRIC	tmSpacing;
	::GetTextMetrics(pcb.hDC, &tmSpacing);
	pcb.m_nLineSpacing = tmSpacing.tmHeight + tmSpacing.tmExternalLeading;

	// Select normal font     
  	SelectObject (pcb.hDC, pcb.font [FONT_NORMAL]);
  	pcb.current_font = &pcb.font [FONT_NORMAL];

  GetTextMetrics (pcb.hDC, &pcb.tm);
  pcb.ok = TRUE;

  bAborted = FALSE;

  return FALSE; // OK exit
  }   // end of print_start_document


/* +++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*                                                     */
/*       print_start_page                              */
/*                                                     */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++ */


// initiates a print page
   
BOOL print_start_page (t_print_control_block & pcb)
  {
int err;

  if (!pcb.ok)
    return TRUE;
  
  if (!PrintingAbortProc(pcb.hDC, 0))
    {
    pcb.ok = FALSE;
    pcb.cancelled = TRUE;
    return TRUE;
    }

// count pages, if out of page range, don't print

  pcb.current_page++;
                                
  if (pcb.pd->PrintRange () && 
      (pcb.current_page < pcb.pd->GetFromPage () || pcb.current_page > pcb.pd->GetToPage ()))
    return FALSE;

// update page number in dialogue

  TCHAR szBuf[80];
  CString strTemp;

  VERIFY(strTemp.LoadString(AFX_IDS_PRINTPAGENUM));

  wsprintf(szBuf, strTemp, pcb.current_page);
  pcb.dlgPrintStatus->SetDlgItemText(AFX_IDC_PRINT_PAGENUM, szBuf);

  err = StartPage (pcb.hDC);

  if (err <= 0)
    {
    ::TMessageBox ("Error occurred starting a new page");
    pcb.ok = FALSE;
    return TRUE;
    }

// reset the font to the current one (some printer drivers seem to change it)

  SelectObject (pcb.hDC, *(pcb.current_font));

// initialise position on page

  pcb.left = pcb.init_left;
  pcb.top = pcb.init_top;

  pcb.pages_printed++;

  return FALSE;
  } // end of print_start_page

/* +++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*                                                     */
/*       print_printline                               */
/*                                                     */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++ */

static char printline_buff [MAX_LINE_WIDTH];

BOOL print_printline (t_print_control_block & pcb, int skip, const char * theline, ...)
  {
va_list arglist;

  if (!pcb.ok)
    return TRUE;

// if out of page range, don't print

  if (pcb.pd->PrintRange () && pcb.current_page < pcb.pd->GetFromPage ())
    return FALSE;

// if past last page, return TRUE so we stop reading the file

  if (pcb.pd->PrintRange () && pcb.current_page > pcb.pd->GetToPage ())
    return TRUE;

/* print the message as if it was a PRINTF type message */

  va_start (arglist, theline);
  _vsnprintf (printline_buff, sizeof (printline_buff), theline, arglist);
  va_end (arglist);

  TextOut (pcb.hDC, pcb.left, pcb.top, printline_buff, strlen (printline_buff));

// if this is a new line, count lines, and move down to the next one

  if (skip)
    {  
    pcb.left = pcb.init_left;   // back to left margin
  	pcb.top += pcb.m_nLineSpacing * skip;
    pcb.lines_printed++;
    }   // end of starting a new line
  else
    {

// same line, find width of this piece of text and add to the left pixel position

    SIZE size;

    GetTextExtentPoint32 (pcb.hDC, printline_buff, strlen (printline_buff), &size); 
    pcb.left += size.cx;
    }   // end of not starting a new line

  return FALSE;
  } // end of print_printline

/* +++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*                                                     */
/*       print_end_page                                */
/*                                                     */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++ */


// ends a print page
   
BOOL print_end_page (t_print_control_block & pcb)
  {
int err;

  if (!pcb.ok)
    return TRUE;

// if out of page range, don't print

  if (pcb.pd->PrintRange () && 
      (pcb.current_page < pcb.pd->GetFromPage () || pcb.current_page > pcb.pd->GetToPage ()))
    return FALSE;

  err = EndPage (pcb.hDC);

  if (err <= 0)
    {
    ::TMessageBox ("Error occurred starting a new page");
    pcb.ok = FALSE;
    return TRUE;
    }

  return FALSE;
  } // end of print_end_page

/* +++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*                                                     */
/*       print_end_document                            */
/*                                                     */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++ */


BOOL print_end_document (t_print_control_block & pcb)
  {
int err;
int i;

  if (pcb.initialised)
    {
    err = EndDoc (pcb.hDC);

    if (err <= 0)
      ::TMessageBox ("Error occurred closing printer");

    } // end of having started the document

// delete our fonts and device contexts etc.

  for (i = 0; i < 8; i++)
    if (pcb.font [i])
      DeleteObject (pcb.font [i]);
      
  if (pcb.hDC)
    DeleteDC  (pcb.hDC);

  pcb.dlgPrintStatus->DestroyWindow ();
  delete pcb.dlgPrintStatus;
  
  delete pcb.pd;
  
  Frame.EnableWindow (TRUE);   
  Frame.SetFocus ();           // so keyboard input works

  pcb.ok = FALSE;
  return FALSE; // OK exit
  } // end of print_end_document


/* +++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*                                                     */
/*       print_font                                    */
/*                                                     */
/* +++++++++++++++++++++++++++++++++++++++++++++++++++ */

// changes the printout to the specified font
/*
Possible combinations of font_type are:

    FONT_NORMAL  
    FONT_ITALIC    
    FONT_UNDERLINE
    FONT_BOLD      

*/

void print_font (t_print_control_block & pcb, const short font_type)
  {

  if (pcb.ok && font_type >= 0 && font_type <= 7)
    {
    SelectObject (pcb.hDC, pcb.font [font_type]);
    pcb.current_font = &pcb.font [font_type];
    }

  } // end of print_font


BOOL CALLBACK PrintingAbortProc(HDC, int)
{


	MSG msg;
	while (!bAborted &&
		::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE))
	{
		if (!AfxGetThread()->PumpMessage())
			return FALSE;   // terminate if WM_QUIT received
	}
	return !bAborted;
}

BOOL CMyPrintingDialog::OnInitDialog()
{
	SetWindowText(AfxGetAppName());
	CenterWindow();
	return CDialog::OnInitDialog();
}

void CMyPrintingDialog::OnCancel()
{
	bAborted = TRUE;  // flag that user aborted print
	CDialog::OnCancel();
}
