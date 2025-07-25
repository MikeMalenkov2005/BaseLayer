#include "unix_os.c"

#include <mach-o/dyld.h>

U32 OS_GetExecutablePath(char *buffer, U32 size)
{
  U32 count = 0;
  _NSGetExecutablePath(NULL, &count);
  if (count > size) return 0;
  if (_NSGetExecutablePath(buffer, &count)) return 0;
  buffer[--count] = 0;
  return count;
}
