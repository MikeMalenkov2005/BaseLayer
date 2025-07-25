#include "unix_os.c"

#include <unistd.h>

U32 OS_GetExecutablePath(char *buffer, U32 size)
{
  SZ count = readlink("/proc/self/exe", buffer, size);
  if (count < 0) return 0;
  buffer[Min((U32)count, size - 1)] = 0;
  return (U32)count;
}
