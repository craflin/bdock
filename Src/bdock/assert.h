
#ifndef Assert_H
#define Assert_H

#ifdef _DEBUG

class Assert
{
public:
  static void trace(const char* format, ...);
};

#include <cassert>
#define ASSERT(e) assert(e)
#define VERIFY(e) ASSERT(e)
#define TRACE(...) Assert::trace(__VA_ARGS__)

#else
#define ASSERT(e)
#define VERIFY(e) e
#define TRACE(...)
#endif

#endif //!Assert_H