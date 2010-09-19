#include "stdafx.h"
#include "..\MUSHclient.h"
// CG: This file added by 'Tip of the Day' component.

#include <winreg.h>
#include <sys\stat.h>
#include <sys\types.h>

#include "TipDlg.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTipDlg dialog

#define MAX_BUFLEN 1000

static const TCHAR szIntFilePos[] = _T("Tip_FilePos");
static const TCHAR szTimeStamp[] = _T("Tip_TimeStamp");
static const TCHAR szIntStartup[] = _T("Tip_StartUp");

CTipDlg::CTipDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_TIP, pParent)
{
char fullfilename[MAX_PATH];
char filename [MAX_PATH];
char * p;

  // look for tips.txt in the same directory as the executable file
  if (GetModuleFileName (NULL, fullfilename, sizeof (fullfilename)))
   {

// remove last part of file name to get working directory

    strcpy (filename, fullfilename);

    p = strrchr (filename, '\\');
    if (p)
      *p = 0;

    strcat (filename, "\\tips.txt");
    }
  else
    strcpy (filename, "tips.txt");

	//{{AFX_DATA_INIT(CTipDlg)
	m_bStartup = TRUE;
	//}}AFX_DATA_INIT

	// We need to find out what the startup and file position parameters are
	// If startup does not exist, we assume that the Tips on startup is checked TRUE.
	m_bStartup = !App.db_get_int("control", szIntStartup, 0);
	UINT iFilePos = App.db_get_int("control", szIntFilePos, 0);

	// Now try to open the tips file
	m_pStream = fopen(filename, "r");
	if (m_pStream == NULL) 
	{
		m_strTip.LoadString(CG_IDS_FILE_ABSENT);
		return;
	} 

	// If the timestamp in the INI file is different from the timestamp of
	// the tips file, then we know that the tips file has been modified
	// Reset the file position to 0 and write the latest timestamp to the
	// ini file
	struct _stat buf;
	_fstat(_fileno(m_pStream), &buf);
	CString strCurrentTime = ctime(&buf.st_ctime);
	strCurrentTime.TrimRight();
	CString strStoredTime = 
		App.db_get_string("control", szTimeStamp, "");
	if (strCurrentTime != strStoredTime) 
	{
		iFilePos = 0;
		App.db_write_string("control", szTimeStamp, strCurrentTime);
	}

	if (fseek(m_pStream, iFilePos, SEEK_SET) != 0) 
	{
		AfxMessageBox(CG_IDP_FILE_CORRUPT);
	}
	else 
	{
		GetNextTipString(m_strTip);
	}
}

CTipDlg::~CTipDlg()
{
	// This destructor is executed whether the user had pressed the escape key
	// or clicked on the close button. If the user had pressed the escape key,
	// it is still required to update the filepos in the ini file with the 
	// latest position so that we don't repeat the tips! 
    
	// But make sure the tips file existed in the first place....
	if (m_pStream != NULL) 
	{
		App.db_write_int("control", szIntFilePos, ftell(m_pStream));
		fclose(m_pStream);
	}
}
        
void CTipDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTipDlg)
	DDX_Check(pDX, IDC_STARTUP, m_bStartup);
	DDX_Text(pDX, IDC_TIPSTRING, m_strTip);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTipDlg, CDialog)
	//{{AFX_MSG_MAP(CTipDlg)
	ON_BN_CLICKED(IDC_NEXTTIP, OnNextTip)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTipDlg message handlers

void CTipDlg::OnNextTip()
{
	GetNextTipString(m_strTip);
	UpdateData(FALSE);
}

void CTipDlg::GetNextTipString(CString& strNext)
{
	LPTSTR lpsz = strNext.GetBuffer(MAX_BUFLEN);

	// This routine identifies the next string that needs to be
	// read from the tips file
	BOOL bStop = FALSE;
	while (!bStop) 
	{
		if (_fgetts(lpsz, MAX_BUFLEN, m_pStream) == NULL) 
		{
			// We have either reached EOF or encountered some problem
			// In both cases reset the pointer to the beginning of the file
			// This behavior is same as VC++ Tips file
			if (fseek(m_pStream, 0, SEEK_SET) != 0) 
				AfxMessageBox(CG_IDP_FILE_CORRUPT);
		} 
		else 
		{
			if (*lpsz != ' ' && *lpsz != '\t' && 
				*lpsz != '\n' && *lpsz != ';') 
			{
				// There should be no space at the beginning of the tip
				// This behavior is same as VC++ Tips file
				// Comment lines are ignored and they start with a semicolon
				bStop = TRUE;
			}
		}
	}
	strNext.ReleaseBuffer();
}

HBRUSH CTipDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd->GetDlgCtrlID() == IDC_TIPSTRING)
		return (HBRUSH)GetStockObject(WHITE_BRUSH);

	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CTipDlg::OnOK()
{
	CDialog::OnOK();
	
    // Update the startup information stored in the INI file
	App.db_write_int ("control", szIntStartup, !m_bStartup);
}

BOOL CTipDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// If Tips file does not exist then disable NextTip
	if (m_pStream == NULL)
		GetDlgItem(IDC_NEXTTIP)->EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CTipDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	// Get paint area for the big static control
	CWnd* pStatic = GetDlgItem(IDC_BULB);
	CRect rect;
	pStatic->GetWindowRect(&rect);
	ScreenToClient(&rect);

	// Paint the background white.
	CBrush brush;
	brush.CreateStockObject(WHITE_BRUSH);
	dc.FillRect(rect, &brush);

	// Load bitmap and get dimensions of the bitmap
	CBitmap bmp;
	bmp.LoadBitmap(IDB_LIGHTBULB);
	BITMAP bmpInfo;
	bmp.GetBitmap(&bmpInfo);

	// Draw bitmap in top corner and validate only top portion of window
	CDC dcTmp;
	dcTmp.CreateCompatibleDC(&dc);
	dcTmp.SelectObject(&bmp);
	rect.bottom = bmpInfo.bmHeight + rect.top;
	dc.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), 
		&dcTmp, 0, 0, SRCCOPY);

	// Draw out "Did you know..." message next to the bitmap
	CString strMessage;
	strMessage.LoadString(CG_IDS_DIDYOUKNOW);
	rect.left += bmpInfo.bmWidth;
	dc.DrawText(strMessage, rect, DT_VCENTER | DT_SINGLELINE);

	// Do not call CDialog::OnPaint() for painting messages
}

