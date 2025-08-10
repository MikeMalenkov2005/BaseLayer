#ifndef OS_H
#define OS_H

#include <str.h>

#ifdef OS_WIN
#define OSAPI __stdcall
#else
#define OSAPI
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                   MEMORY                                     *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void* OS_MemoryReserve(UZ size);
void  OS_MemoryCommit(void* memory, UZ size);
void  OS_MemoryDecommit(void* memory, UZ size);
void  OS_MemoryRelease(void* memory, UZ size);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                    INFO                                      *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

STR OS_GetExecutablePath(MEM_Arena *arena);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                  FILE API                                    *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef UP OS_File;

typedef enum OS_FileOpenFlags
{
  OS_FILE_OPEN_READ         = 1,
  OS_FILE_OPEN_WRITE        = 2,
  OS_FILE_OPEN_READ_WRITE   = 3,
  OS_FILE_OPEN_CREATE       = 4,
  OS_FILE_OPEN_TRUNCATE     = 8,
} OS_FileOpenFlags;

typedef enum OS_FileSeekMode
{
  OS_FILE_SEEK_SET,
  OS_FILE_SEEK_CUR,
  OS_FILE_SEEK_END,
} OS_FileSeekMode;

OS_File OS_FileOpen(STR path, U32 flags);
void OS_FileClose(OS_File file);

U64 OS_FileTell(OS_File file);
U64 OS_FileSeek(OS_File file, S64 offset, OS_FileSeekMode mode);

U64 OS_FileSize(OS_File file);

UZ OS_FileRead(OS_File file, STR buffer);
UZ OS_FileWrite(OS_File file, STR data);

bool OS_FileExists(STR path);
bool OS_FileRename(STR src, STR dst);

bool OS_FileDelete(STR path);

bool OS_FileCreateDir(STR path);
bool OS_FileDeleteDir(STR path);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                              DYNAMIC LINKING                                 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef UP OS_Library;
typedef void OSAPI OS_LibraryFunc(void);

OS_Library OS_LibraryLoad(STR path);
OS_LibraryFunc *OS_LibraryGetFunction(OS_Library lib, const char *name);
void OS_LibraryFree(OS_Library lib);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                               MULTITHREADING                                 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef UP OS_Thread;
typedef U32 OSAPI OS_ThreadFunc(void *);

OS_Thread OS_ThreadCreate(OS_ThreadFunc *start, void *param);
bool OS_ThreadJoin(OS_Thread thread, U32 *result);

void OS_ThreadExit(U32 code);

typedef UP OS_ThreadKey;

OS_ThreadKey OS_ThreadKeyInit();
void OS_ThreadKeyFree(OS_ThreadKey key);
void *OS_ThreadKeyGet(OS_ThreadKey key);
void OS_ThreadKeySet(OS_ThreadKey key, void *value);

typedef UP OS_Mutex;

OS_Mutex OS_MutexInit();
bool OS_MutexFree(OS_Mutex mutex);
bool OS_MutexLock(OS_Mutex mutex);
bool OS_MutexUnlock(OS_Mutex mutex);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                 NETWORKING                                   *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool OS_NetStartup();
void OS_NetCleanup();

typedef struct OS_NetAddressIPv4
{
  U8 addr[4];
} OS_NetAddressIPv4;

typedef struct OS_NetAddressIPv6
{
  U32 flow;
  U8 addr[16];
  U32 scope;
} OS_NetAddressIPv6;

typedef struct OS_NetAddress
{
  U16 type;
  U16 port;
  union
  {
    OS_NetAddressIPv4 ipv4;
    OS_NetAddressIPv6 ipv6;
  };
} OS_NetAddress;

typedef enum OS_NetType
{
  OS_NET_TYPE_NULL,
  OS_NET_TYPE_IPv4,
  OS_NET_TYPE_IPv6,
} OS_NetType;

OS_NetAddress OS_NetAddressResolve(const char *node, const char *service);

typedef UP OS_NetSocket;

OS_NetSocket OS_NetOpenDatagramSocket(OS_NetType type);
OS_NetSocket OS_NetOpenServer(OS_NetAddress *address, int backlog);
OS_NetSocket OS_NetConnect(OS_NetAddress *address);

void OS_NetClose(OS_NetSocket socket);

SZ OS_NetSend(OS_NetSocket socket, const void *data, UZ size);
SZ OS_NetSendTo(OS_NetSocket socket, const void *data, UZ size, OS_NetAddress *address);

SZ OS_NetReceive(OS_NetSocket socket, void *buffer, UZ size);
SZ OS_NetReceiveFrom(OS_NetSocket socket, void *buffer, UZ size, OS_NetAddress *address);

#endif
