// logdlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLogDlg dialog

class CLogDlg : public CDialog
{
// Construction
public:
	CLogDlg(CWnd* pParent = NULL);   // standard constructor


  CMUSHclientDoc * m_doc;

// Dialog Data
	//{{AFX_DATA(CLogDlg)
	enum { IDD = IDD_LINE_COUNT };
	long	m_lines;
	BOOL	m_bAppendToLogFile;
	BOOL	m_bWriteWorldName;
	CString	m_strPreamble;
	BOOL	m_bLogNotes;
	BOOL	m_bLogInput;
	BOOL	m_bLogOutput;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLogDlg)
	afx_msg void OnHelpbutton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
