#ifndef MEM_H
#define MEM_H

#include <defines.h>

c_linkage_begin

#define MEM_AlignDown(x, n) ((x) - ((x) % (n)))
#define MEM_AlignUp(x, n) MEM_AlignDown((x)+(n)-1, n)

#define MEM_FastAlignDown(x, n) ((x) & ~((n)-1))
#define MEM_FastAlignUp(x, n) MEM_FastAlignDown((x)+(n)-1, n)

#define MEM_ARENA_ALIGNMENT     16
#define MEM_ARENA_DEFAULT_SIZE  GiB(1)
#define MEM_ARENA_COMMIT_SIZE   KiB(8)

typedef struct MEM_Arena
{
  U8 *memory;
  UZ size;
  UZ commited;
  UZ allocated;
} MEM_Arena;

MEM_Arena MEM_ArenaInit(UZ size);
void MEM_ArenaClear(MEM_Arena *arena);
void MEM_ArenaFree(MEM_Arena *arena);

void *MEM_ArenaAllocate(MEM_Arena *arena, UZ size);
void *MEM_ArenaReallocate(MEM_Arena *arena, void *memory, UZ size);
void MEM_ArenaDeallocate(MEM_Arena *arena, void *memory);
void MEM_ArenaDeallocateTo(MEM_Arena *arena, UZ position);
void MEM_ArenaDeallocateSize(MEM_Arena *arena, UZ size);

typedef struct MEM_ArenaLevel
{
  MEM_Arena *arena;
  UZ position;
} MEM_ArenaLevel;

MEM_ArenaLevel MEM_ArenaLevelInit(MEM_Arena *arena);
void MEM_ArenaLevelFree(MEM_ArenaLevel level);

typedef void *MEM_AllocateCallback(PTR data, UZ size);
typedef void *MEM_ReallocateCallback(PTR data, void *memory, UZ size);
typedef void MEM_DeallocateCallback(PTR data, void *memory);

typedef struct MEM
{
  MEM_AllocateCallback *allocate;
  MEM_ReallocateCallback *reallocate;
  MEM_DeallocateCallback *deallocate;
  PTR data;
} MEM;

void *MEM_Allocate(MEM *mem, UZ size);
void *MEM_Reallocate(MEM *mem, void *memory, UZ size);
void MEM_Deallocate(MEM *mem, void *memory);

void *MEM_AllocateZero(MEM *mem, UZ size);
#define MEM_AllocateArraySized(mem, count, size) \
  MEM_AllocateZero(mem, (count) * (size))
#define MEM_AllocateArrayTyped(mem, count, type) \
  MEM_AllocateArraySized(mem, count, sizeof(type))

#define MEM_FromArena(arena) ((MEM) { (PTR)MEM_ArenaAllocate, (PTR)MEM_ArenaReallocate, (PTR)MEM_ArenaDeallocate, (arena) })

c_linkage_end

#endif
