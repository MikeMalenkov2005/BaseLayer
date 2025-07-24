#ifndef MEM_H
#define MEM_H

#include <defines.h>

c_linkage_begin

#define MEM_AlignDown(x, n) ((x) - ((x) % (n)))
#define MEM_AlignUp(x, n) MEM_AlignDown((x)+(n)-1, n)

#define MEM_FastAlignDown(x, n) ((x) & ~((n)-1))
#define MEM_FastAlignUp(x, n) MEM_FastAlignDown((x)+(n)-1, n)

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
void *MEM_ArenaAllocateZero(MEM_Arena *arena, UZ size);
void MEM_ArenaDeallocate(MEM_Arena *arena, void *memory);
void MEM_ArenaDeallocateTo(MEM_Arena *arena, UZ position);
void MEM_ArenaDeallocateSize(MEM_Arena *arena, UZ size);

#define MEM_ArenaAllocateArraySized(arena, count, size) \
  MEM_ArenaAllocateZero(arena, (count) * (size))
#define MEM_ArenaAllocateArrayTyped(arena, count, type) \
  MEM_ArenaAllocateArraySized(arena, count, sizeof(type))

typedef struct MEM_ArenaLevel
{
  MEM_Arena *arena;
  UZ position;
} MEM_ArenaLevel;

MEM_ArenaLevel MEM_ArenaLevelOpen(MEM_Arena *arena);
void MEM_ArenaLevelClosee(MEM_ArenaLevel level);

#define MEM_HEAP_DEFAULT_SIZE GiB(1)
#define MEM_HEAP_COMMIT_SIZE  KiB(8)

typedef struct MEM_Heap
{
  U8 *memory;
  UZ size;
  UZ commited;
  UZ head;
} MEM_Heap;

MEM_Heap MEM_HeapInit(UZ size);
void MEM_HeapClear(MEM_Heap *heap);
void MEM_HeapFree(MEM_Heap *heap);

void *MEM_HeapAllocate(MEM_Heap *heap, UZ size);
void *MEM_HeapAllocateZero(MEM_Heap *heap, UZ size);
void *MEM_HeapReallocate(MEM_Heap *heap, void *memory, UZ size);
void MEM_HeapDeallocate(MEM_Heap *heap, void *memory);

#define MEM_HeapAllocateArraySized(heap, count, size) \
  MEM_HeapAllocateZero(arena, (count) * (size))
#define MEM_HeapAllocateArrayTyped(heap, count, type) \
  MEM_HeapAllocateArraySized(arena, count, sizeof(type))

c_linkage_end

#endif
