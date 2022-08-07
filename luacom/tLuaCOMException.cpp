// tLuaCOMException.cpp: implementation of the tLuaCOMException class.
//
//////////////////////////////////////////////////////////////////////

//#include <iostream.h>
#include <string.h>
#include <stdio.h>

#include "tLuaCOMException.h"
#include "tUtil.h"

char const * const tLuaCOMException::messages[] =  
     {
       "Internal Error",
       "Parameter(s) out of range",
       "Type conversion error",
       "COM error",
       "COM exception",
       "Unsupported feature required",
       "Windows error",
       "LuaCOM error",
       "Memory allocation error"
     };


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

tLuaCOMException::tLuaCOMException(Errors p_code, const char *p_file, int p_line,
                                   const char *p_usermessage)
{
  code = p_code;

  file = tStringBuffer(p_file);
  line = p_line;
  usermessage = tStringBuffer(p_usermessage);

  tUtil::log("luacom", getMessage());
}

tLuaCOMException::~tLuaCOMException()
{
}



tStringBuffer tLuaCOMException::getMessage()
{
  char string[5000];
  char error_position[1000];


  strncpy(string, messages[code], sizeof(string)-1);

  if(file != NULL)
  {
    sprintf(error_position, ":(%s,%d)", (const char*)file, line);
    strcat(string, error_position);
  }
  
  strncat(string, ":", sizeof(string) - strlen(string) - 1);


  if(usermessage != NULL)
  {
    strncat(string, usermessage, sizeof(string) - strlen(string) - 1);
  }

  return tStringBuffer(string);
}

tStringBuffer tLuaCOMException::GetErrorMessage(DWORD errorcode)
{
  return tUtil::GetErrorMessage(errorcode);
}
