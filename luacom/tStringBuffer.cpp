// tStringBuffer.cpp: implementation of the tStringBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include <string.h>

#include "tStringBuffer.h"

void tStringBuffer::Init()
{
  size = 0;
  buffer = NULL;
}

void tStringBuffer::Reset()
{
  if(buffer != NULL)
    delete[] buffer;
  Init();
}

tStringBuffer::tStringBuffer()
{
  Init();
}
 
tStringBuffer::tStringBuffer(const char* source)
{
  Init();
  copyToBuffer(source);
}

tStringBuffer::tStringBuffer(const char* source, size_t length)
{
	Init();
	copyToBuffer(source, length);
}

tStringBuffer::tStringBuffer(const tStringBuffer& copy)
{
  Init();
  copyToBuffer(copy.buffer, copy.size);
}

tStringBuffer& tStringBuffer::operator=(const tStringBuffer& other)
{
  Reset();
  copyToBuffer(other.buffer, other.size);
  return *this;
}

tStringBuffer::operator const char *()
{
  return buffer;
}

tStringBuffer::~tStringBuffer()
{
  Reset();
}

void tStringBuffer::copyToBuffer(const char * source)
{
  Reset();
  if(source)
  {
    size = strlen(source) + 1;
    buffer = new char[size];
    strncpy(buffer, source, size);
  }
}

void tStringBuffer::copyToBuffer(const char *source, size_t length)
{
  Reset();
  if(source && length)
  {
    size = length;
    buffer = new char[size];
    memcpy(buffer, source, length);
  }
}

const char * tStringBuffer::getBuffer()
{
  return buffer;
}
const size_t tStringBuffer::getSize()
{
	return size;
}
