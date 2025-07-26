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
      if (arena->allocated < arena->size)
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

void *MEM_ArenaAllocateZero(MEM_Arena *arena, UZ size)
{
  void *memory = MEM_ArenaAllocate(arena, size);
  return memory ? MemoryZero(memory, size) : memory;
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

MEM_ArenaLevel MEM_ArenaLevelOpen(MEM_Arena *arena)
{
  return (MEM_ArenaLevel) { .arena = arena, .position = arena->allocated };
}

void MEM_ArenaLevelClosee(MEM_ArenaLevel level)
{
  MEM_ArenaDeallocateTo(level.arena, level.position);
}
