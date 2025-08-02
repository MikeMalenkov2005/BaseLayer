#include <os.h>

#include <sys/mman.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

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

OS_Library OS_LibraryLoad(STR path)
{
  return (OS_Library)dlopen(nt.str, RTLD_LAZY);
}

OS_LibraryFunc *OS_LibraryGetFunction(OS_Library lib, const char *name)
{
  return (OS_LibraryFunc*)dlsym((void*)lib, name);
}

void OS_LibraryFree(OS_Library lib)
{
  dlclose((void*)lib);
}

OS_Thread OS_ThreadCreate(OS_ThreadFunc *start, void *param)
{
  OS_Thread thread;
  if (pthread_create((void*)&thread, nullptr, (void*)start, param)) thread = 0;
  return thread;
}

bool OS_ThreadJoin(OS_Thread thread, U32 *result)
{
  void *value;
  if (pthread_join((pthread_t)thread, &value)) return false;
  *result = (U32)value;
  return true;
}

static bool OS_NetActive = false;

bool OS_NetStartup()
{
  return (OS_NetActive = true);
}

void OS_NetCleanup()
{
  OS_NetActive = false;
}
