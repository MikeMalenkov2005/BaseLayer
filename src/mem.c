#include <mem.h>
#include <os.h>

MEM_Arena MEM_ArenaInit(size_t size)
{
  MEM_Arena arena;
  arena.size = size ? MEM_FastAlignUp(size, MEM_ARENA_ALIGNMENT)
                    : (UZ)MEM_ARENA_DEFAULT_SIZE;
  arena.memory = OS_MemoryReserve(arena.size);
  arena.commited = 0;
  arena.allocated = 0;
  return arena;
}

void MEM_ArenaClear(MEM_Arena *arena)
{
  MEM_ArenaDeallocateSize(arena, arena->allocated);
}

void MEM_ArenaFree(MEM_Arena *arena)
{
  OS_MemoryRelease(arena->memory, arena->size);
  MemoryZeroStruct(arena);
}

void *MEM_ArenaAllocate(MEM_Arena *arena, UZ size)
{
  void *memory = nullptr;
  if (arena->memory)
  {
    size = MEM_FastAlignUp(size, MEM_ARENA_ALIGNMENT);
    arena->allocated = MEM_FastAlignUp(arena->allocated, MEM_ARENA_ALIGNMENT);
    memory = arena->memory + arena->allocated;
    arena->allocated += size;
    if (arena->allocated > arena->commited)
    {
      if (arena->allocated <= arena->size)
      {
        UZ commit = MEM_FastAlignUp(size, (UZ)MEM_ARENA_COMMIT_SIZE);
        commit = Min(commit, arena->size - arena->commited);
        OS_MemoryCommit(arena->memory + arena->commited, commit);
        arena->commited += commit;
      }
      else
      {
        arena->allocated -= size;
        memory = nullptr;
      }
    }
  }
  return memory;
}

void *MEM_ArenaReallocate(MEM_Arena *arena, void *memory, UZ size)
{
  if (!memory) return MEM_ArenaAllocate(arena, size);
  UZ position = (UZ)((UP)memory - (UP)arena->memory);
  if (position >= arena->allocated) return nullptr;
  UZ current = arena->allocated - position;
  if (current < size)
  {
    size -= current;
    UZ offset = MEM_FastAlignUp(arena->allocated, MEM_ARENA_ALIGNMENT) - arena->allocated;
    if (offset < size)
    {
      if (!MEM_ArenaAllocate(arena, size - offset)) return nullptr;
    }
    else arena->allocated += size;
  }
  else MEM_ArenaDeallocateSize(arena, current - size);
  return memory;
}

void MEM_ArenaDeallocate(MEM_Arena *arena, void *memory)
{
  MEM_ArenaDeallocateTo(arena, (UZ)((U8*)memory - arena->memory));
}

void MEM_ArenaDeallocateTo(MEM_Arena *arena, UZ position)
{
  if (position < arena->allocated)
  {
    MEM_ArenaDeallocateSize(arena, arena->allocated - position);
  }
}

void MEM_ArenaDeallocateSize(MEM_Arena *arena, UZ size)
{
  arena->allocated -= Min(arena->allocated, size);
}

MEM_ArenaLevel MEM_ArenaLevelInit(MEM_Arena *arena)
{
  return (MEM_ArenaLevel) { .arena = arena, .position = arena->allocated };
}

void MEM_ArenaLevelFree(MEM_ArenaLevel level)
{
  MEM_ArenaDeallocateTo(level.arena, level.position);
}

typedef struct MEM_HeapBlock
{
  struct MEM_HeapBlock *next;
  struct MEM_HeapBlock *prev;
  UZ size;
  UZ free;
} MEM_HeapBlock;

static MEM_HeapBlock *MEM_HeapFindFirstFreeBlock(MEM_Heap *heap, UZ size)
{
  MEM_HeapBlock *block = heap->commited ? (PTR)heap->memory : nullptr;
  while (block && block->size < size && !block->free) block = block->next;
  return block;
}

static void MEM_HeapDefragment(MEM_Heap *heap, MEM_HeapBlock *block)
{
  if (block->free)
  {
    while (block != (PTR)heap->memory && block->prev->free) block = block->prev;
    while (block->next && block->next->free)
    {
      block->size += block->next->size + sizeof *block;
      block->next = block->next->next;
      if (!block->next) block->prev = block;
      else block->next->prev = block;
    }
  }
}

MEM_Heap MEM_HeapInit(UZ size)
{
  MEM_Heap heap;
  heap.size = size  ? MEM_FastAlignUp(size, MEM_HEAP_ALIGNMENT)
                    : (UZ)MEM_HEAP_DEFAULT_SIZE;
  heap.memory = OS_MemoryReserve(heap.size);
  heap.commited = 0;
  return heap;
}

void MEM_HeapClear(MEM_Heap *heap)
{
  if (heap && heap->memory && heap->commited)
  {
    ((MEM_HeapBlock*)heap->memory)->next = nullptr;
    ((MEM_HeapBlock*)heap->memory)->prev = (PTR)heap->memory;
    ((MEM_HeapBlock*)heap->memory)->size = heap->commited;
    ((MEM_HeapBlock*)heap->memory)->free = true;
  }
}

void MEM_HeapFree(MEM_Heap *heap)
{
  OS_MemoryRelease(heap->memory, heap->size);
  MemoryZeroStruct(heap);
}

void *MEM_HeapAllocate(MEM_Heap *heap, UZ size)
{
  MEM_HeapBlock *block;
  if (!heap || !heap->memory) return nullptr;
  if (!(size = MEM_FastAlignUp(size, MEM_HEAP_ALIGNMENT))) return nullptr;
  if (!(block = MEM_HeapFindFirstFreeBlock(heap, size)))
  {
    if (heap->commited && ((MEM_HeapBlock*)heap->memory)->prev->free)
    {
      block = ((MEM_HeapBlock*)heap->memory)->prev;
      UZ commit = MEM_FastAlignUp(size - block->size, (UZ)MEM_HEAP_COMMIT_SIZE);
      commit = Min(commit, heap->size - heap->commited);
      if (commit < size - block->size) return nullptr;
      OS_MemoryCommit(heap->memory + heap->commited, commit);
      heap->commited += commit;
      block->size += commit;
    }
    else
    {
      block = (PTR)(heap->memory + heap->commited);
      UZ commit = MEM_FastAlignUp(size, (UZ)MEM_HEAP_COMMIT_SIZE);
      commit = Min(commit, heap->size - heap->commited);
      if (commit < size) return nullptr;
      OS_MemoryCommit(heap->memory + heap->commited, commit);
      if (heap->commited)
      {
        block->prev = ((MEM_HeapBlock*)heap->memory)->prev;
        block->prev->next = ((MEM_HeapBlock*)heap->memory)->prev = block;
      }
      else block->prev = block;
      heap->commited += commit;
      block->next = nullptr;
      block->size = commit;
      block->free = true;
    }
  }
  if (block->size > size + sizeof *block)
  {
    MEM_HeapBlock *second = (PTR)((U8*)(block + 1) + size);
    second->prev = block;
    second->next = block->next;
    second->size = block->size - size - sizeof *block;
    second->free = true;
    block->next = second;
    block->size = size;
    if (block == (PTR)heap->memory) block->prev = second;
  }
  block->free = false;
  return block + 1;
}

void *MEM_HeapReallocate(MEM_Heap *heap, void *memory, UZ size)
{
  if (!(size = MEM_FastAlignUp(size, MEM_HEAP_ALIGNMENT))) return nullptr;
  if (!memory) return MEM_HeapAllocate(heap, size);

  if (heap && heap->memory && (UP)memory - (UP)heap->memory < heap->commited)
  {
    MEM_HeapBlock *block = (MEM_HeapBlock*)memory - 1;
    if (memory && !((UP)memory & (MEM_HEAP_ALIGNMENT - 1)) && block->size && !(block->size & (MEM_HEAP_ALIGNMENT - 1)) && !block->free)
    {
      if ((memory = MEM_HeapAllocate(heap, size)))
      {
        block->free = true;
        memory = MemoryCopy(memory, block + 1, size);
        MEM_HeapDefragment(heap, block);
        return memory;
      }
    }
  }
  return nullptr;
}

void MEM_HeapDeallocate(MEM_Heap *heap, void *memory)
{
  if (heap && heap->memory && (UP)memory - (UP)heap->memory < heap->commited)
  {
    MEM_HeapBlock *block = (MEM_HeapBlock*)memory - 1;
    if (memory && !((UP)memory & (MEM_HEAP_ALIGNMENT - 1)) && block->size && !(block->size & (MEM_HEAP_ALIGNMENT - 1)) && !block->free)
    {
      block->free = true;
      MEM_HeapDefragment(heap, block);
    }
  }
}

void *MEM_Allocate(MEM *mem, UZ size)
{
  return mem->allocate(mem->data, size);
}

void *MEM_Reallocate(MEM *mem, void *memory, UZ size)
{
  return mem->reallocate(mem->data, memory, size);
}

void MEM_Deallocate(MEM *mem, void *memory)
{
  mem->deallocate(mem->data, memory);
}

void *MEM_AllocateZero(MEM *mem, UZ size)
{
  void *memory = MEM_Allocate(mem, size);
  return memory ? MemoryZero(memory, size) : memory;
}

#ifndef BASE_LAYER_MEM_NO_DEFAULT

#include <stdlib.h>

void *MEM_DefaultAllocate(PTR ignored, UZ size)
{
  (void)ignored;
  return malloc(size);
}

void *MEM_DefaultReallocate(PTR ignored, void *memory, UZ size)
{
  (void)ignored;
  return realloc(memory, size);
}

void MEM_DefaultDeallocate(PTR ignored, void *memory)
{
  (void)ignored;
  free(memory);
}

#endif

MEM_Smart MEM_SmartInit(MEM *parent)
{
  MEM_Smart smart = { parent, nullptr, 0 };
  return smart;
}

void MEM_SmartClear(MEM_Smart *smart)
{
  for (UZ i = 0; i < smart->capacity; ++i)
  {
    if (smart->owned[i]) MEM_Deallocate(smart->parent, smart->owned[i]);
  }
}

void MEM_SmartFree(MEM_Smart *smart)
{
  MEM_SmartClear(smart);
  if (smart->owned) MEM_Deallocate(smart->parent, smart->owned);
  MemoryZeroStruct(smart);
}

UZ MEM_SmartGetIndex(MEM_Smart *smart, PTR block)
{
  UZ index = 0;
  while (index < smart->capacity && smart->owned[index] != block) ++index;
  if (!block && index == smart->capacity && smart->capacity < MEM_SMART_MAX_OWNED)
  {
    UZ capacity = smart->capacity + MEM_SMART_INCREMENT;
    PTR *owned = MEM_Reallocate(smart->parent, smart->owned, sizeof(PTR) * capacity);
    if (owned)
    {
      smart->owned = owned;
      smart->capacity = capacity;
      MemoryZeroTyped((smart->owned + index), MEM_SMART_INCREMENT);
    }
  }
  return index;
}

void *MEM_SmartAllocate(MEM_Smart *smart, UZ size)
{
  return MEM_SmartReallocate(smart, nullptr, size);
}

void *MEM_SmartReallocate(MEM_Smart *smart, void *memory, UZ size)
{
  UZ index = MEM_SmartGetIndex(smart, memory);
  if (index >= smart->capacity) return nullptr;
  memory = MEM_Reallocate(smart->parent, memory, size);
  if (memory) smart->owned[index] = memory;
  return memory;
}

void MEM_SmartDeallocate(MEM_Smart *smart, void *memory)
{
  if (memory)
  {
    UZ index = MEM_SmartGetIndex(smart, memory);
    if (index < smart->capacity)
    {
      smart->owned[index] = nullptr;
      MEM_Deallocate(smart->parent, memory);
    }
  }
}
