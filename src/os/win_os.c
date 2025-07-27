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
    U32 size = GetModuleFileNameW(nullptr, buffer, KiB(2));
    if (size) result = STR_From_STR16(arena, (STR16) { .str = buffer, .size = size });
  }
  return result;
}

OS_Library OS_LibraryLoad(STR path)
{
  MEM_Arena arena = MEM_ArenaInit((path.size + 1) << 1);
  STR16 pathw = STR16_From_STR(&arena, path);
  OS_Library lib = (OS_Library)LoadLibraryW(pathw.str);
  MEM_ArenaFree(&arena);
  return lib;
}

OS_LibraryFunc *OS_LibraryGetFunction(OS_Library lib, const char *name)
{
  return (OS_LibraryFunc*)GetProcAddress((HMODULE)lib, name);
}

void OS_LibraryFree(OS_Library lib)
{
  FreeLibrary((HMODULE)lib);
}

OS_Thread OS_ThreadCreate(OS_ThreadFunc *start, void *param)
{
  return (OS_Thread)CreateThread(nullptr, 0, start, param, 0, nullptr);
}

bool OS_ThreadJoin(OS_Thread thread, U32 *result)
{
  return !WaitForSingleObject((HANDLE)thread, INFINITE) && GetExitCodeThread((HANDLE)thread, result);
}
