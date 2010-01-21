#pragma once

/////////////////////////////////////////////////////////////////////////////
// CFormat

struct CFormat : CString
  {
  CFormat (LPCTSTR lpszFormat, ...);
  CFormat (UINT nFormatID, ...);    
  CFormat (LPCTSTR lpszFormat, va_list argList);    
  CFormat (UINT nFormatID, va_list argList);    
  };

