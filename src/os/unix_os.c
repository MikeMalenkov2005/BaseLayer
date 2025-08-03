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
  if (sock == -1) return OS_NET_SOCKET_INVALID;
  return (OS_NetSocket)sock;
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
      if (bind(sock, &addr, sizeof(addr))) return (close(sock), OS_NET_SOCKET_INVALID);
    }
    else return OS_NET_SOCKET_INVALID;
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
      if (bind(sock, &addr, sizeof(addr))) return (close(sock), OS_NET_SOCKET_INVALID);
    }
    else return OS_NET_SOCKET_INVALID;
    break;
  }
  if (listen(sock, backlog)) return (close(sock), OS_NET_SOCKET_INVALID);
  return (OS_NetSocket)sock;
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
      if (connect(sock, &addr, sizeof(addr))) return (close(sock), OS_NET_SOCKET_INVALID);
    }
    else return OS_NET_SOCKET_INVALID;
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
      if (connect(sock, &addr, sizeof(addr))) return (close(sock), OS_NET_SOCKET_INVALID);
    }
    else return OS_NET_SOCKET_INVALID;
    break;
  }
  return (OS_NetSocket)sock;
}

void OS_NetClose(OS_NetSocket socket)
{
  close((int)socket);
}

SZ OS_NetSend(OS_NetSocket socket, const void *data, UZ size)
{
  return (SZ)send((int)socket, data, (size_t)size, 0);
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
      return (SZ)sendto((int)socket, data, (size_t)size, 0, &addr, sizeof(addr));
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
      return (SZ)sendto((int)socket, data, (size_t)size, 0, &addr, sizeof(addr));
    }
    break;
  }
  return -1;
}

SZ OS_NetReceive(OS_NetSocket socket, void *buffer, UZ size)
{
  return (SZ)recv((int)socket, buffer, (size_t)size, 0);
}

SZ OS_NetReceiveFrom(OS_NetSocket socket, void *buffer, UZ size, OS_NetAddress *address)
{
  U8 addr[64] = { 0 };
  SZ result = (SZ)sendto((int)socket, buffer, (size_t)size, 0, &addr, sizeof(addr));
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
