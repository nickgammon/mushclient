// Exceptions.cpp - our special exception classes
//

#include "stdafx.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////
// CSystemException


IMPLEMENT_DYNAMIC(CSystemException, CException)

CSystemException::CSystemException(DWORD dwError)
{
	m_dwError = dwError;
}

BOOL CSystemException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext /*=NULL*/)
{
	ASSERT(lpszError != NULL && AfxIsValidString(lpszError, nMaxError));

	if (pnHelpContext != NULL)
		*pnHelpContext = 0;

	LPTSTR	lpBuffer;
	BOOL	bRet = FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					m_dwError,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
					(LPTSTR) &lpBuffer, 
					0, 
					NULL);

	if (bRet == FALSE)
		*lpszError = '\0';
	else
	{
		lstrcpyn(lpszError, lpBuffer, nMaxError);
		LocalFree(lpBuffer);
	}
	return bRet;
}

void ThrowSystemException(DWORD dwError /*=0*/)
{
	if (dwError == 0)
		dwError = ::GetLastError();
	TRACE1("Throw system error exception, %lu\n", dwError);
	throw new CSystemException(dwError);
}

/////////////////////////////////////////////////////////////////////////////
// CErrorException

IMPLEMENT_DYNAMIC(CErrorException, CException)

// constructor - accepts a "printf" string.

// example:  throw new CErrorException ("An error code %i occurred", iError);

CErrorException::CErrorException(LPCTSTR lpszFormat, ...)
{
	va_list argList;
	va_start(argList, lpszFormat);
	m_strError = CFormat (lpszFormat, argList);
	va_end(argList);
}

// constructor - accepts a "printf" string from a resource ID

// example:  throw new CErrorException (IDS_ERROR_OCCURRED, iError);

CErrorException::CErrorException(UINT nFormatID,  ...)
{
	CString	strFormat;
	VERIFY(strFormat.LoadString(nFormatID) != 0);

  va_list argList;
	va_start(argList, nFormatID);
	m_strError = CFormat (strFormat, argList);
	va_end(argList);
}

// returns the error message

BOOL CErrorException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext /*=NULL*/)
{
	ASSERT(lpszError != NULL && AfxIsValidString(lpszError, nMaxError));

	if (pnHelpContext != NULL)
		*pnHelpContext = 0;

	lstrcpyn(lpszError, m_strError, nMaxError);
	return TRUE;
}

// helper function to throw an error exception for you

void ThrowErrorException(UINT nFormatID, ...)
{
	CString	strFormat;
	VERIFY(strFormat.LoadString(nFormatID) != 0);

	va_list argList;
	va_start(argList, nFormatID);
	CString strText = CFormat (strFormat, argList);
	va_end(argList);
	
	TRACE1("Throw error exception, %s\n", (LPCTSTR) strText);
	throw new CErrorException(strText);
}

// helper function to throw an error exception for you

void ThrowErrorException(LPCTSTR lpszFormat, ...)
{
	ASSERT(AfxIsValidString(lpszFormat, FALSE));

	va_list argList;
	va_start(argList, lpszFormat);
	CString strText = CFormat (lpszFormat, argList);
	va_end(argList);

	TRACE1("Throw error exception, %s\n", (LPCTSTR) strText);
	throw new CErrorException("%s", strText);
}
