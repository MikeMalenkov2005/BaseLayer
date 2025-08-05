#include "unix_os.c"

#include <stdlib.h>

STR OS_GetExecutablePath(MEM_Arena *arena)
{
  STR result = { 0 };
  for (U8 *buffer = malloc(KiB(4)); buffer; buffer = (free(buffer), nullptr))
  {
    SZ size = readlink("/proc/self/exe", buffer, KiB(4));
    if (size > 0)
    {
      result = STR_Allocate(arena, size + 1);
      if (result.str) MemoryCopy(result.str, buffer, size);
    }
  }
  return result;
}
