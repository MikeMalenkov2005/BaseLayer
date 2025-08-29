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

#ifndef BASE_LAYER_MEM_NO_DEFAULT

void *MEM_DefaultAllocate(PTR ignored, UZ size);
void *MEM_DefaultReallocate(PTR ignored, void *memory, UZ size);
void MEM_DefaultDeallocate(PTR ignored, void *memory);

#define MEM_Default() ((MEM) { (PTR)MEM_DefaultAllocate, (PTR)MEM_DefaultReallocate, (PTR)MEM_DefaultDeallocate, nullptr })

#endif

#define MEM_FromArena(arena) ((MEM) { (PTR)MEM_ArenaAllocate, (PTR)MEM_ArenaReallocate, (PTR)MEM_ArenaDeallocate, (arena) })

#define MEM_From_MEM(mem) ((MEM) { (PTR)MEM_Allocate, (PTR)MEM_Reallocate, (PTR)MEM_Deallocate, (mem) })

#define MEM_SMART_INCREMENT 16
#define MEM_SMART_MAX_OWNED MEM_FastAlignDown(MAX_UZ / sizeof(UZ), MEM_SMART_INCREMENT)

typedef struct MEM_Smart
{
  MEM *parent;
  PTR *owned;
  UZ capacity;
} MEM_Smart;

MEM_Smart MEM_SmartInit(MEM *parent);
void MEM_SmartClear(MEM_Smart *smart);
void MEM_SmartFree(MEM_Smart *smart);

void *MEM_SmartAllocate(MEM_Smart *smart, UZ size);
void *MEM_SmartReallocate(MEM_Smart *smart, void *memory, UZ size);
void MEM_SmartDeallocate(MEM_Smart *smart, void *memory);

#define MEM_FromSmart(smart) ((MEM) { (PTR)MEM_SmartAllocate, (PTR)MEM_SmartReallocate, (PTR)MEM_SmartDeallocate, (smart) })

c_linkage_end

#endif
