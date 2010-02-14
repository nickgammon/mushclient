// InsertUnicodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\mushclient.h"
#include "InsertUnicodeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInsertUnicodeDlg dialog


CInsertUnicodeDlg::CInsertUnicodeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInsertUnicodeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInsertUnicodeDlg)
	m_strCharacter = _T("");
	m_bHex = FALSE;
	//}}AFX_DATA_INIT
}


void CInsertUnicodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInsertUnicodeDlg)
	DDX_Text(pDX, IDC_UNICODE_CHARACTER, m_strCharacter);
	DDX_Check(pDX, IDC_UNICODE_HEX, m_bHex);
	//}}AFX_DATA_MAP

  if (pDX->m_bSaveAndValidate)
    {
    m_strCharacter.TrimLeft ();
    m_strCharacter.TrimRight ();

    if(m_strCharacter.IsEmpty ())
      {
      ::TMessageBox ("Unicode character code cannot be blank.");
      DDX_Text(pDX, IDC_UNICODE_CHARACTER, m_strCharacter);
      pDX->Fail();
      }     // end of code being blank

    if(m_strCharacter.GetLength () > 10)
      {
      ::TMessageBox ("Unicode character code too long.");
      DDX_Text(pDX, IDC_UNICODE_CHARACTER, m_strCharacter);
      pDX->Fail();
      }     // end of code being too long to convert

    const char * p = m_strCharacter;
    __int64 i = 0; // converted number

    if (m_bHex)
      {
      for ( ; *p; p++)
        {

        if (!isxdigit (*p))
          {
          ::UMessageBox (TFormat ("Bad hex character: '%c'.", *p));
          DDX_Text(pDX, IDC_UNICODE_CHARACTER, m_strCharacter);
          pDX->Fail();
          }

        int iNewDigit = toupper (*p);
        if (iNewDigit >= 'A')
          iNewDigit -= 7;

    	  i = (i << 4) + iNewDigit - '0';
        } // end of building up sequence
      }  // in hex
    else
      {  // decimal

      for ( ; *p; p++)
        if (!isdigit (*p))
          {
          ::UMessageBox (TFormat ("Bad decimal character: '%c'.", *p));
          DDX_Text(pDX, IDC_UNICODE_CHARACTER, m_strCharacter);
          pDX->Fail();
          }

      i = _atoi64 (m_strCharacter);
      }

    if (i < 0 || i > LONG_MAX)
        {
        ::UMessageBox (TFormat ("Unicode character %I64i too large - must be in range 0 to 2147483647 "
                                  "(hex 0 to 7FFFFFFF).", i));
        DDX_Text(pDX, IDC_UNICODE_CHARACTER, m_strCharacter);
        pDX->Fail();
        }


    m_iCode = i;

    }   // end of saving

}


BEGIN_MESSAGE_MAP(CInsertUnicodeDlg, CDialog)
	//{{AFX_MSG_MAP(CInsertUnicodeDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInsertUnicodeDlg message handlers
