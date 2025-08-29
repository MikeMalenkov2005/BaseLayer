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

MEM_Smart MEM_SmartInit(MEM *parent)
{
  MEM_Smart smart = { parent };
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


#endif
