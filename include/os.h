#ifndef OS_H
#define OS_H

#include <str.h>

void* OS_MemoryReserve(UZ size);
void  OS_MemoryCommit(void* memory, UZ size);
void  OS_MemoryDecommit(void* memory, UZ size);
void  OS_MemoryRelease(void* memory, UZ size);

STR OS_GetExecutablePath(MEM_Arena *arena);

#endif
