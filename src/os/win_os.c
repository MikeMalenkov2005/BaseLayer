#include <os.h>

#include <Windows.h>

void* OS_MemoryReserve(UZ size)
{
  return VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
}

void  OS_MemoryCommit(void* memory, UZ size)
{
  VirtualAlloc(memory, size, MEM_COMMIT, PAGE_READWRITE);
}

void  OS_MemoryDecommit(void* memory, UZ size)
{
  VirtualFree(memory, size, MEM_DECOMMIT);
}

void  OS_MemoryRelease(void* memory, UZ size)
{
  VirtualFree(memory, 0, MEM_RELEASE);
}

STR OS_GetExecutablePath(MEM_Arena *arena)
{
  STR result = { 0 };
  for (U16 *buffer = malloc(KiB(4)); buffer; buffer = (free(buffer), nullptr))
  {
    U32 size = GetModuleFileNameW(NULL, buffer, KiB(2));
    if (size) result = STR_From_STR16(arena, (STR16) { .str = buffer, .size = size });
  }
  return result;
}
