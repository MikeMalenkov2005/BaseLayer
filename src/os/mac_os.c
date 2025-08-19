#include "unix_os.c"

#include <mach-o/dyld.h>

STR OS_GetExecutablePath(MEM *mem)
{
  U32 size;
  _NSGetExecutablePath(NULL, &size);
  STR result = STR_Allocate(mem, size);
  if (result.str) _NSGetExecutablePath(result.str, &size);
  return result;
}
