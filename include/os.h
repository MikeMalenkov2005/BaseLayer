#ifndef OS_H
#define OS_H

#include <defines.h>

void* OS_MemoryReserve(UZ size);
void  OS_MemoryCommit(void* memory, UZ size);
void  OS_MemoryDecommit(void* memory, UZ size);
void  OS_MemoryRelease(void* memory, UZ size);

#endif
