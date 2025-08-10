#include <os.h>

#define WIN32_LEAN_AND_MEAN
#include <stdlib.h>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

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

OS_File OS_FileOpen(STR path, U32 flags)
{
  MEM_Arena arena = MEM_ArenaInit((path.size + 1) << 1);
  STR16 pathw = STR16_From_STR(&arena, path);
  U32 access = 0;
  if (flags & OS_FILE_OPEN_READ) access |= GENERIC_READ;
  if (flags & OS_FILE_OPEN_WRITE) access |= GENERIC_WRITE;
  U32 create = OPEN_EXISTING;
  if (flags & OS_FILE_OPEN_CREATE)
  {
    create = OPEN_ALWAYS;
    if ((flags & OS_FILE_OPEN_TRUNCATE) && (flags & OS_FILE_OPEN_WRITE))
    {
      create = CREATE_ALWAYS;
    }
  }
  else if ((flags & OS_FILE_OPEN_TRUNCATE) && (flags & OS_FILE_OPEN_WRITE))
  {
    create = TRUNCATE_EXISTING;
  }
  HANDLE file = CreateFileW(pathw.str, access, null, nullptr, create, FILE_ATTRIBUTE_NORMAL, nullptr);
  MEM_ArenaFree(&arena);
  return (OS_File)file;
}

void OS_FileClose(OS_File file)
{
  CloseHandle((HANDLE)file);
}

U64 OS_FileTell(OS_File file)
{
  U32 high = 0;
  U32 low = SetFilePointer((HANDLE)file, 0, &high, FILE_CURRENT);
  if (low == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) return MAX_U64;
  return (((U64)high << 32) | low);
}

U64 OS_FileSeek(OS_File file, S64 offset, OS_FileSeekMode mode)
{
  U32 high = (U32)(offset >> 32);
  U32 low = SetFilePointer((HANDLE)file, (S32)offset, &high, mode);
  return (((U64)high << 32) | low);
}

U64 OS_FileSize(OS_File file)
{
  U32 high = 0;
  U32 low = GetFileSize((HANDLE)file, &high);
  if (low == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) return ~(U64)0;
  return (((U64)high << 32) | low);
}

UZ OS_FileRead(OS_File file, STR buffer)
{
  UZ bytes = 0;
  while (bytes < buffer.size)
  {
    UZ remain = (buffer.size - bytes);
    U32 toread = (U32)remain;
    if (remain > MAX_U32) toread = MAX_U32;
    U32 count = 0;
    if (!ReadFile((HANDLE)file, buffer.str + bytes, toread, &count, nullptr)) return 0;
    bytes += count;
    if (count < toread) break;
  }
  return bytes;
}

UZ OS_FileWrite(OS_File file, STR data)
{
  UZ result = 0;
  while (result < data.size)
  {
    UZ remain = (data.size - result);
    U32 towrite = (U32)remain;
    if (remain > MAX_U32) towrite = MAX_U32;
    U32 count = 0;
    if (!WriteFile((HANDLE)file, data.str + result, towrite, &count, nullptr)) break;
    result += count;
    if (count < towrite) break;
  }
  return result;
}

bool OS_FileExists(STR path)
{
  MEM_Arena arena = MEM_ArenaInit((path.size + 1) << 1);
  STR16 pathw = STR16_From_STR(&arena, path);
  U32 attributes = GetFileAttributesW(pathw.str);
  MEM_ArenaFree(&arena);
  return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

bool OS_FileRename(STR src, STR dst)
{
  MEM_Arena arena = MEM_ArenaInit((src.size + dst.size + 2) << 1);
  STR16 srcw = STR16_From_STR(&arena, src);
  STR16 dstw = STR16_From_STR(&arena, dst);
  bool result = MoveFileW(srcw.str, dstw.str);
  MEM_ArenaFree(&arena);
  return result;
}

bool OS_FileDelete(STR path)
{
  MEM_Arena arena = MEM_ArenaInit((path.size + 1) << 1);
  STR16 pathw = STR16_From_STR(&arena, path);
  bool result = DeleteFileW(pathw.str);
  MEM_ArenaFree(&arena);
  return result;
}

bool OS_FileCreateDir(STR path)
{
  MEM_Arena arena = MEM_ArenaInit((path.size + 1) << 1);
  STR16 pathw = STR16_From_STR(&arena, path);
  bool result = CreateDirectoryW(pathw.str, nullptr);
  MEM_ArenaFree(&arena);
  return result;
}

bool OS_FileDeleteDir(STR path)
{
  MEM_Arena arena = MEM_ArenaInit((path.size + 1) << 1);
  STR16 pathw = STR16_From_STR(&arena, path);
  bool result = RemoveDirectoryW(pathw.str);
  MEM_ArenaFree(&arena);
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

void OS_ThreadExit(U32 code)
{
  ExitThread(code);
}

OS_ThreadKey OS_ThreadKeyInit()
{
  U32 key = TlsAlloc();
  if (key == TLS_OUT_OF_INDEXES) return null;
  return (OS_ThreadKey)(key + 1);
}

void OS_ThreadKeyFree(OS_ThreadKey key)
{
  TlsFree((U32)(key - 1));
}

void *OS_ThreadKeyGet(OS_ThreadKey key)
{
  return TlsGetValue((U32)(key - 1));
}

void OS_ThreadKeySet(OS_ThreadKey key, void *value)
{
  TlsSetValue((U32)(key - 1), value);
}

OS_Mutex OS_MutexInit()
{
  return (OS_Mutex)CreateMutex(nullptr, false, nullptr);
}

bool OS_MutexFree(OS_Mutex mutex)
{
  return CloseHandle((HANDLE)mutex);
}

bool OS_MutexLock(OS_Mutex mutex)
{
  return WaitForSingleObject((HANDLE)mutex, INFINITE) == WAIT_OBJECT_0;
}

bool OS_MutexUnlock(OS_Mutex mutex)
{
  return ReleaseMutex((HANDLE)mutex);
}

static bool OS_NetActive = false;
static WSADATA OS_NetInfo = { 0 };

bool OS_NetStartup()
{
  return OS_NetActive || (OS_NetActive = WSAStartup(MAKEWORD(2, 2), &OS_NetInfo));
}

void OS_NetCleanup()
{
  if (OS_NetActive)
  {
    WSACleanup();
    OS_NetActive = false;
  }
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
  SOCKET sock = INVALID_SOCKET;
  switch (type)
  {
  case OS_NET_TYPE_IPv4:
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    break;
  case OS_NET_TYPE_IPv6:
    sock = socket(AF_INET6, SOCK_DGRAM, 0);
    break;
  };
  if (sock == INVALID_SOCKET) return null;
  return (OS_NetSocket)sock + 1;
}

OS_NetSocket OS_NetOpenServer(OS_NetAddress *address, int backlog)
{
  SOCKET sock = INVALID_SOCKET;
  switch (address->type)
  {
  case OS_NET_TYPE_IPv4:
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) != INVALID_SOCKET)
    {
      struct sockaddr_in addr =
      {
        .sin_family = AF_INET,
        .sin_port = htons(address->port),
      };
      MemoryCopy(&addr.sin_addr, address->ipv4.addr, sizeof(address->ipv4.addr));
      if (bind(sock, &addr, sizeof(addr))) return (closesocket(sock), null);
    }
    else return null;
    break;
  case OS_NET_TYPE_IPv6:
    if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) != INVALID_SOCKET)
    {
      struct sockaddr_in6 addr = {
        .sin6_family = AF_INET6,
        .sin6_port = htons(address->port),
        .sin6_flowinfo = address->ipv6.flow,
        .sin6_scope_id = address->ipv6.scope,
      };
      MemoryCopy(&addr.sin6_addr, address->ipv6.addr, sizeof(address->ipv6.addr));
      if (bind(sock, &addr, sizeof(addr))) return (closesocket(sock), null);
    }
    else return null;
    break;
  }
  if (listen(sock, backlog)) return (closesocket(sock), null);
  return (OS_NetSocket)sock + 1;
}

OS_NetSocket OS_NetConnect(OS_NetAddress *address)
{
  SOCKET sock = INVALID_SOCKET;
  switch (address->type)
  {
  case OS_NET_TYPE_IPv4:
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) != INVALID_SOCKET)
    {
      struct sockaddr_in addr =
      {
        .sin_family = AF_INET,
        .sin_port = htons(address->port),
      };
      MemoryCopy(&addr.sin_addr, address->ipv4.addr, sizeof(address->ipv4.addr));
      if (connect(sock, &addr, sizeof(addr))) return (closesocket(sock), null);
    }
    else return null;
    break;
  case OS_NET_TYPE_IPv6:
    if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) != INVALID_SOCKET)
    {
      struct sockaddr_in6 addr = {
        .sin6_family = AF_INET6,
        .sin6_port = htons(address->port),
        .sin6_flowinfo = address->ipv6.flow,
        .sin6_scope_id = address->ipv6.scope,
      };
      MemoryCopy(&addr.sin6_addr, address->ipv6.addr, sizeof(address->ipv6.addr));
      if (connect(sock, &addr, sizeof(addr))) return (closesocket(sock), null);
    }
    else return null;
    break;
  }
  return (OS_NetSocket)sock + 1;
}

void OS_NetClose(OS_NetSocket socket)
{
  closesocket((SOCKET)(socket - 1));
}

SZ OS_NetSend(OS_NetSocket socket, const void *data, UZ size)
{
  return (SZ)send((SOCKET)(socket - 1), data, (int)size, 0);
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
      return (SZ)sendto((SOCKET)(socket - 1), data, (int)size, 0, &addr, sizeof(addr));
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
      return (SZ)sendto((SOCKET)(socket - 1), data, (int)size, 0, &addr, sizeof(addr));
    }
    break;
  }
  return -1;
}

SZ OS_NetReceive(OS_NetSocket socket, void *buffer, UZ size)
{
  return (SZ)recv((SOCKET)(socket - 1), buffer, (int)size, 0);
}

SZ OS_NetReceiveFrom(OS_NetSocket socket, void *buffer, UZ size, OS_NetAddress *address)
{
  U8 addr[64] = { 0 };
  UZ addrlen = sizeof(addr);
  SZ result = (SZ)recvfrom((SOCKET)(socket - 1), buffer, (int)size, 0, addr, &addrlen);
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
