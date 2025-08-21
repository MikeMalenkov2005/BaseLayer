#include <utl.h>

STR UTL_LoadFile(MEM *mem, STR path)
{
  STR result = { null };
  OS_File file = OS_FileOpen(path, OS_FILE_OPEN_READ);
  if (file)
  {
    U64 size = OS_FileSize(file);
    if (size < MAX_UZ)
    {
      result = STR_Allocate(mem, (UZ)size);
      if (result.str)
      {
        UZ bytes = OS_FileRead(file, result);
        U8 *data = MEM_Reallocate(mem, result.str, bytes + 1);
        if (data) result.str = data;
        result.str[bytes] = 0;
        result.size = bytes;
      }
    }
    OS_FileClose(file);
  }
  return result;
}
