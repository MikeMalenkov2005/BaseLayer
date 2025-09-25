#include <os.h>

#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sched.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

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

void OS_Exit(int code)
{
  _exit(code);
}

OS_File OS_FileOpen(STR path, U32 flags)
{
  int oflags = 0;
  switch (flags & OS_FILE_OPEN_READ_WRITE)
  {
  case OS_FILE_OPEN_READ:
    oflags = O_RDONLY;
    break;
  case OS_FILE_OPEN_WRITE:
    oflags = O_WRONLY;
    break;
  case OS_FILE_OPEN_READ_WRITE:
    oflags = O_RDWR;
    break;
  }
  if (flags & OS_FILE_OPEN_CREATE) oflags |= O_CREAT;
  if ((flags & OS_FILE_OPEN_TRUNCATE) && (flags & OS_FILE_OPEN_WRITE)) oflags |= O_TRUNC;
  int file = open((char*)path.str, oflags, 0666);
  return (file < 0 ? null : ((OS_File)file + 1));
}

void OS_FileClose(OS_File file)
{
  close((int)(file - 1));
}

U64 OS_FileTell(OS_File file)
{
#if defined(OS_LINUX) && defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS < 64
  off64_t result = lseek64((int)(file - 1), 0, SEEK_CUR);
#else
  off_t result = lseek((int)(file - 1), 0, SEEK_CUR);
#endif
  if (result == -1) return MAX_U64;
  return (U64)result;
}

U64 OS_FileSeek(OS_File file, S64 offset, OS_FileSeekMode mode)
{
#if defined(OS_LINUX) && defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS < 64
  off64_t result = lseek64((int)(file - 1), (off64_t)offset, mode);
#else
  off_t result = lseek((int)(file - 1), (off_t)offset, mode);
#endif
  if (result == -1) return MAX_U64;
  return (U64)result;
}

U64 OS_FileSize(OS_File file)
{
  struct stat st;
  if (fstat((int)(file - 1), &st) == -1) return 0;
  return (U64)st.st_size;
}

UZ OS_FileRead(OS_File file, STR buffer)
{
  SZ bytes = read((int)(file - 1), buffer.str, buffer.size);
  if (bytes == -1) return 0;
  return (UZ)bytes;
}

UZ OS_FileWrite(OS_File file, STR data)
{
  SZ result = write((int)(file - 1), data.str, data.size);
  return (result == -1 ? 0 : (UZ)result);
}

bool OS_FileExists(STR path)
{
  return (access((char*)path.str, F_OK) == 0);
}

bool OS_FileRename(STR src, STR dst)
{
  return (rename((char*)src.str, (char*)dst.str) == 0);
}

bool OS_FileDelete(STR path)
{
  return (remove((char*)path.str) != -1);
}

bool OS_FileCreateDir(STR path)
{
  return (mkdir((char*)path.str, 0777) != -1);
}

bool OS_FileDeleteDir(STR path)
{
  return (rmdir((char*)path.str) != -1);
}

OS_Library OS_LibraryLoad(STR path)
{
  return (OS_Library)dlopen((char*)path.str, RTLD_LAZY);
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
  pthread_t thread;
  if (pthread_create(&thread, nullptr, (void*)start, param)) return null;
  return (OS_Thread)thread + 1;
}

bool OS_ThreadJoin(OS_Thread thread, U32 *result)
{
  void *value;
  if (pthread_join((pthread_t)(thread - 1), &value)) return false;
  if (result) *result = (U32)(UP)value;
  return true;
}

void OS_ThreadExit(U32 code)
{
  pthread_exit((void*)(UP)code);
}

bool OS_ThreadYield()
{
  return !sched_yield();
}

OS_ThreadKey OS_ThreadKeyInit()
{
  pthread_key_t key;
  if (pthread_key_create(&key, nullptr)) return null;
  return (OS_ThreadKey)key + 1;
}

void OS_ThreadKeyFree(OS_ThreadKey key)
{
  pthread_key_delete((pthread_key_t)(key - 1));
}

void *OS_ThreadKeyGet(OS_ThreadKey key)
{
  return pthread_getspecific((pthread_key_t)(key - 1));
}

void OS_ThreadKeySet(OS_ThreadKey key, void *value)
{
  pthread_setspecific((pthread_key_t)(key - 1), value);
}

OS_Mutex OS_MutexInit()
{
  pthread_mutex_t *mutex = malloc(sizeof(*mutex));
  if (mutex && pthread_mutex_init(mutex, nullptr))
  {
    free(mutex);
    mutex = nullptr;
  }
  return (OS_Mutex)mutex;
}

bool OS_MutexFree(OS_Mutex mutex)
{
  if (pthread_mutex_destroy((pthread_mutex_t *)mutex))
  {
    free((void*)mutex);
    return true;
  }
  return false;
}

bool OS_MutexLock(OS_Mutex mutex)
{
  return !pthread_mutex_lock((pthread_mutex_t *)mutex);
}

bool OS_MutexUnlock(OS_Mutex mutex)
{
  return !pthread_mutex_unlock((pthread_mutex_t *)mutex);
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

OS_NetAddress OS_NetAddressResolve(const char *node, const char *service)
{
  struct addrinfo *info;
  OS_NetAddress result = { .type = OS_NET_TYPE_NULL };
  if (!getaddrinfo(node, service, nullptr, &info))
  {
    switch (info->ai_addr->sa_family)
    {
    case AF_INET:
      result.type = OS_NET_TYPE_IPv4;
      result.port = ntohs(((struct sockaddr_in*)info->ai_addr)->sin_port);
      MemoryCopy(result.ipv4.addr, &((struct sockaddr_in*)info->ai_addr)->sin_addr, sizeof(result.ipv4.addr));
      break;
    case AF_INET6:
      result.type = OS_NET_TYPE_IPv6;
      result.port = ntohs(((struct sockaddr_in6*)info->ai_addr)->sin6_port);
      result.ipv6.flow = ((struct sockaddr_in6*)info->ai_addr)->sin6_flowinfo;
      result.ipv6.scope = ((struct sockaddr_in6*)info->ai_addr)->sin6_scope_id;
      MemoryCopy(result.ipv6.addr, &((struct sockaddr_in6*)info->ai_addr)->sin6_addr, sizeof(result.ipv6.addr));
      break;
    }
    freeaddrinfo(info);
  }
  return result;
}

OS_NetSocket OS_NetOpenDatagramSocket(OS_NetType type)
{
  int sock = -1;
  switch (type)
  {
  case OS_NET_TYPE_IPv4:
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    break;
  case OS_NET_TYPE_IPv6:
    sock = socket(AF_INET6, SOCK_DGRAM, 0);
    break;
  };
  if (sock == -1) return null;
  return (OS_NetSocket)sock + 1;
}

OS_NetSocket OS_NetOpenServer(OS_NetAddress *address, int backlog)
{
  int sock = -1;
  switch (address->type)
  {
  case OS_NET_TYPE_IPv4:
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) != -1)
    {
      struct sockaddr_in addr =
      {
        .sin_family = AF_INET,
        .sin_port = htons(address->port),
      };
      MemoryCopy(&addr.sin_addr, address->ipv4.addr, sizeof(address->ipv4.addr));
      if (bind(sock, (PTR)&addr, sizeof(addr))) return (close(sock), null);
    }
    else return null;
    break;
  case OS_NET_TYPE_IPv6:
    if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) != -1)
    {
      struct sockaddr_in6 addr = {
        .sin6_family = AF_INET6,
        .sin6_port = htons(address->port),
        .sin6_flowinfo = address->ipv6.flow,
        .sin6_scope_id = address->ipv6.scope,
      };
      MemoryCopy(&addr.sin6_addr, address->ipv6.addr, sizeof(address->ipv6.addr));
      if (bind(sock, (PTR)&addr, sizeof(addr))) return (close(sock), null);
    }
    else return null;
    break;
  }
  if (listen(sock, backlog)) return (close(sock), null);
  return (OS_NetSocket)sock + 1;
}

OS_NetSocket OS_NetConnect(OS_NetAddress *address)
{
  int sock = -1;
  switch (address->type)
  {
  case OS_NET_TYPE_IPv4:
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) != -1)
    {
      struct sockaddr_in addr =
      {
        .sin_family = AF_INET,
        .sin_port = htons(address->port),
      };
      MemoryCopy(&addr.sin_addr, address->ipv4.addr, sizeof(address->ipv4.addr));
      if (connect(sock, (PTR)&addr, sizeof(addr))) return (close(sock), null);
    }
    else return null;
    break;
  case OS_NET_TYPE_IPv6:
    if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) != -1)
    {
      struct sockaddr_in6 addr = {
        .sin6_family = AF_INET6,
        .sin6_port = htons(address->port),
        .sin6_flowinfo = address->ipv6.flow,
        .sin6_scope_id = address->ipv6.scope,
      };
      MemoryCopy(&addr.sin6_addr, address->ipv6.addr, sizeof(address->ipv6.addr));
      if (connect(sock, (PTR)&addr, sizeof(addr))) return (close(sock), null);
    }
    else return null;
    break;
  }
  return (OS_NetSocket)sock + 1;
}

OS_NetSocket OS_NetAccept(OS_NetSocket server, OS_NetAddress *address)
{
  U8 addr[64] = { 0 };
  UZ addrlen = sizeof(addr);
  int sock = accept((int)(server - 1), (struct sockaddr*)addr, &addrlen);
  switch (((struct sockaddr*)addr)->sa_family)
  {
  case AF_INET:
    address->type = OS_NET_TYPE_IPv4;
    address->port = ntohs(((struct sockaddr_in*)addr)->sin_port);
    MemoryCopy(address->ipv4.addr, &((struct sockaddr_in*)addr)->sin_addr, sizeof(address->ipv4.addr));
    break;
  case AF_INET6:
    address->type = OS_NET_TYPE_IPv6;
    address->port = ntohs(((struct sockaddr_in6*)addr)->sin6_port);
    address->ipv6.flow = ((struct sockaddr_in6*)addr)->sin6_flowinfo;
    address->ipv6.scope = ((struct sockaddr_in6*)addr)->sin6_scope_id;
    MemoryCopy(address->ipv6.addr, &((struct sockaddr_in6*)addr)->sin6_addr, sizeof(address->ipv6.addr));
    break;
  }
  return (OS_NetSocket)sock + 1;
}

void OS_NetClose(OS_NetSocket socket)
{
  close((int)(socket - 1));
}

SZ OS_NetSend(OS_NetSocket socket, const void *data, UZ size)
{
  return (SZ)send((int)(socket - 1), data, (size_t)size, 0);
}

SZ OS_NetSendTo(OS_NetSocket socket, const void *data, UZ size, OS_NetAddress *address)
{
  switch (address->type)
  {
  case OS_NET_TYPE_IPv4:
    {
      struct sockaddr_in addr =
      {
        .sin_family = AF_INET,
        .sin_port = htons(address->port),
      };
      MemoryCopy(&addr.sin_addr, address->ipv4.addr, sizeof(address->ipv4.addr));
      return (SZ)sendto((int)(socket - 1), data, (size_t)size, 0, (PTR)&addr, sizeof(addr));
    }
    break;
  case OS_NET_TYPE_IPv6:
    {
      struct sockaddr_in6 addr = {
        .sin6_family = AF_INET6,
        .sin6_port = htons(address->port),
        .sin6_flowinfo = address->ipv6.flow,
        .sin6_scope_id = address->ipv6.scope,
      };
      MemoryCopy(&addr.sin6_addr, address->ipv6.addr, sizeof(address->ipv6.addr));
      return (SZ)sendto((int)(socket - 1), data, (size_t)size, 0, (PTR)&addr, sizeof(addr));
    }
    break;
  }
  return -1;
}

SZ OS_NetReceive(OS_NetSocket socket, void *buffer, UZ size)
{
  return (SZ)recv((int)(socket - 1), buffer, (size_t)size, 0);
}

SZ OS_NetReceiveFrom(OS_NetSocket socket, void *buffer, UZ size, OS_NetAddress *address)
{
  U8 addr[64] = { 0 };
  UZ addrlen = sizeof(addr);
  SZ result = (SZ)recvfrom((int)(socket - 1), buffer, (size_t)size, 0, (PTR)addr, (void*)&addrlen);
  switch (((struct sockaddr*)addr)->sa_family)
  {
  case AF_INET:
    address->type = OS_NET_TYPE_IPv4;
    address->port = ntohs(((struct sockaddr_in*)addr)->sin_port);
    MemoryCopy(address->ipv4.addr, &((struct sockaddr_in*)addr)->sin_addr, sizeof(address->ipv4.addr));
    break;
  case AF_INET6:
    address->type = OS_NET_TYPE_IPv6;
    address->port = ntohs(((struct sockaddr_in6*)addr)->sin6_port);
    address->ipv6.flow = ((struct sockaddr_in6*)addr)->sin6_flowinfo;
    address->ipv6.scope = ((struct sockaddr_in6*)addr)->sin6_scope_id;
    MemoryCopy(address->ipv6.addr, &((struct sockaddr_in6*)addr)->sin6_addr, sizeof(address->ipv6.addr));
    break;
  }
  return result;
}
