// tStringBuffer.cpp: implementation of the tStringBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include <string.h>

#include "tStringBuffer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

tStringBuffer::tStringBuffer()
{
  size = 0;
  buffer = NULL;
}

tStringBuffer::~tStringBuffer()
{
  if(buffer!=NULL)
	  delete[] buffer;
}

void tStringBuffer::copyToBuffer(char * source)
{
  long new_size = strlen(source) + 1;

  if(new_size > size)
  {
    if(buffer != NULL)
      delete[] buffer;

    size = new_size;
    buffer = new char[size];
  }

  strncpy(buffer, source, new_size);
}

const char * tStringBuffer::getBuffer()
{
  return buffer;
}
