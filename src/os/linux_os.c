#include <os.h>

#include <sys/mman.h>

void* OS_MemoryReserve(UZ size)
{
  return mmap(nullptr, size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
}

void  OS_MemoryCommit(void* memory, UZ size)
{
  mprotect(memory, size, PROT_READ | PROT_WRITE);
}

void  OS_MemoryDecommit(void* memory, UZ size)
{
  mprotect(memory, size, PROT_NONE);
}

void  OS_MemoryRelease(void* memory, UZ size)
{
  munmap(memory, size);
}
