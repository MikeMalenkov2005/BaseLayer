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

#endif
