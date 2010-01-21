#include "stdafx.h"
#include "resource.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/*

Custom DDX/DDV processing
=========================

This file provides the extra processing which enables the "minimum length" for
CStrings in the Class Wizard. 


If you have to rebuild the winforms.CLW file, then you must insert the following
lines in the first section of that file (ie. in the [General Info] section).
They are inserted at the end of the section.


; ClassWizard DDX information for custom DDX functions
ExtraDDXCount=2
ExtraDDX1=E;;String with min and max;CString;;MinMaxString;CString with a minimum and maximum length;MinMaxString;M&inimum length;d;Ma&ximum length;d
ExtraDDX2=M;;String with min and max;CString;;MinMaxCBString;CString with a minimum and maximum length;MinMaxCBString;M&inimum length;d;Ma&ximum length;d


*/
void AFXAPI DDV_MinMaxString(CDataExchange* pDX, CString& value, int nMinChars, int nMaxChars)
{

  value.TrimLeft ();
  value.TrimRight ();

	if (pDX->m_bSaveAndValidate && value.GetLength() < nMinChars)
	{
		TCHAR szT[32];
    if (nMinChars == 1)
		  TMessageBox("This field may not be blank", MB_ICONEXCLAMATION);
    else
      {
		  wsprintf(szT, _T("%d"), nMinChars);
		  CString prompt;
		  AfxFormatString1(prompt, IDS_MIN_STRING_SIZE, szT);
		  AfxMessageBox(prompt, MB_ICONEXCLAMATION, AFX_IDP_PARSE_STRING_SIZE);
   		prompt.Empty(); // exception prep
      }
		pDX->Fail();
	}
 
// now test maximum length

  DDV_MaxChars (pDX, value, nMaxChars);

}

void AFXAPI DDX_MinMaxString(CDataExchange* pDX, int nIDC, CString& value)
  {
  DDX_Text(pDX, nIDC, value);
  }

void AFXAPI DDV_MinMaxCBString(CDataExchange* pDX, CString& value, int nMinChars, int nMaxChars)
{
  DDV_MinMaxString (pDX, value, nMinChars, nMaxChars);
}

void AFXAPI DDX_MinMaxCBString(CDataExchange* pDX, int nIDC, CString& value)
  {
  DDX_CBString(pDX, nIDC, value);
  }

