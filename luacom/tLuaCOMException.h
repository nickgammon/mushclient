// tLuaCOMException.h: interface for the tLuaCOMException class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TLUACOMEXCEPTION_H__26509908_AFD8_11D4_B882_0000B45D7541__INCLUDED_)
#define AFX_TLUACOMEXCEPTION_H__26509908_AFD8_11D4_B882_0000B45D7541__INCLUDED_

#include <windows.h>
#include "tStringBuffer.h"

class tLuaCOMException  
{
public:
  void getDebugMessage(void);
  tStringBuffer getMessage(void);
  enum Errors {INTERNAL_ERROR, PARAMETER_OUT_OF_RANGE,
    TYPECONV_ERROR, COM_ERROR, COM_EXCEPTION, UNSUPPORTED_FEATURE,
    WINDOWS_ERROR, LUACOM_ERROR, MALLOC_ERROR
  };

  static tStringBuffer GetErrorMessage(DWORD errorcode);

  tLuaCOMException(Errors p_code, const char *p_file, int p_line, const char *usermessage = NULL);
  virtual ~tLuaCOMException();

  Errors code;
  tStringBuffer file;
  int line;
  tStringBuffer usermessage;

protected:
  static char const * const messages[];
};

#define LUACOM_EXCEPTION(x) throw tLuaCOMException(tLuaCOMException::x, \
  __FILE__, __LINE__)

#define LUACOM_EXCEPTION_MSG(x,y) throw tLuaCOMException(tLuaCOMException::x, \
  __FILE__, __LINE__, y)


#define CHECK(x,y) ((x) ? (void) 0 : LUACOM_EXCEPTION(y))

#define TYPECONV_ERROR(x) throw tLuaCOMException(tLuaCOMException::TYPECONV_ERROR, \
  __FILE__, __LINE__, x)

#define COM_ERROR(x) throw tLuaCOMException(tLuaCOMException::COM_ERROR, \
__FILE__, __LINE__, ((x) == NULL ? "Unknown error" : (x)))

#define COM_EXCEPTION(x) throw tLuaCOMException(tLuaCOMException::COM_EXCEPTION, \
  __FILE__, __LINE__, x)

#define CHECKPARAM(x) ((x) ? (void) 0 : LUACOM_EXCEPTION(PARAMETER_OUT_OF_RANGE))

#define CHECKPARAM_MSG(x,y) ((x) ? (void) 0 : LUACOM_EXCEPTION_MSG( \
  PARAMETER_OUT_OF_RANGE, y))

#define CHECKPRECOND(x) ((x) ? (void) 0 : LUACOM_EXCEPTION(INTERNAL_ERROR))

#define CHECKPOSCOND(x) CHECKPRECOND(x)

#define CHECKFEATURE(x, y) ((x) ? (void) 0 : tLuaCOMException( \
        tLuaCOMException::UNSUPPORTED_FEATURE, __FILE__, __LINE__, y))

#define INTERNAL_ERROR() LUACOM_EXCEPTION(INTERNAL_ERROR)

#define WINCHECK(x) ((x) ? (void) 0 : LUACOM_EXCEPTION(WINDOWS_ERROR))

#define LUACOM_ERROR(x) (throw tLuaCOMException( \
        tLuaCOMException::LUACOM_ERROR, __FILE__, __LINE__, x))

#define CHK_LCOM_ERR(x, y) ((x) ? (void) 0 : LUACOM_ERROR(y))

#define CHKMALLOC(x) ((x) ? (void) 0 : LUACOM_EXCEPTION(MALLOC_ERROR))

inline void chk_com_code(HRESULT hr, const char * filename, int linenum) {
  if (hr != S_OK)
    throw tLuaCOMException(tLuaCOMException::COM_ERROR,
             filename, linenum, tLuaCOMException::GetErrorMessage(hr));
}
#define CHK_COM_CODE(hr) chk_com_code(hr, __FILE__, __LINE__)


#endif // !defined(AFX_TLUACOMEXCEPTION_H__26509908_AFD8_11D4_B882_0000B45D7541__INCLUDED_)
