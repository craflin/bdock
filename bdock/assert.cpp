
#include "stdafx.h"

#ifdef DEBUG

#include <cstdio>
#include <cstdarg>

void Assert::trace(const char* format, ...)
{
  char data[320];
  va_list ap;
  va_start (ap, format);
  int length;
#ifdef _MSC_VER
  length = vsprintf_s(data, sizeof(data) - 1, format, ap);
#else
  length = ::vsnprintf(data, sizeof(data) - 2, format, ap);
  if(length < 0)
    length = sizeof(data) - 2;
#endif
  va_end (ap);
  data[length++] = '\n';
  data[length] = '\0';
  fputs(data, stderr);
  fflush(stderr);
}

#endif
