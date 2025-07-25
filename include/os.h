#ifndef OS_H
#define OS_H

#include <defines.h>

void* OS_MemoryReserve(UZ size);
void  OS_MemoryCommit(void* memory, UZ size);
void  OS_MemoryDecommit(void* memory, UZ size);
void  OS_MemoryRelease(void* memory, UZ size);

/* @returns 0 on error, `size` on truncation, string length excluding null terminator on success. */
U32 OS_GetExecutablePath(char *buffer, U32 size);

#endif
