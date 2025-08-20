#include <rc.h>

typedef struct __Generic_RC
{
  MEM *mem;
  UZ count;
} __Generic_RC, *Generic_RC;

PTR __RC_Init(MEM *mem, UZ size)
{
  Generic_RC rc = MEM_AllocateZero(mem, sizeof(__Generic_RC) + size);
  if (rc)
  {
    rc->mem = mem;
    rc->count = 1;
  }
  return rc;
}

PTR RC_Clone(PTR _rc)
{
  Generic_RC rc = _rc;
  if (rc) ++rc->count;
  return rc;
}

void RC_Free(PTR _rc)
{
  Generic_RC rc = _rc;
  if (rc && !--rc->count) MEM_Deallocate(rc->mem, rc);
}

typedef struct __Generic_ARC
{
  MEM *mem;
  OS_Mutex mutex;
  UZ count;
} __Generic_ARC, *Generic_ARC;

PTR __ARC_Init(MEM *mem, UZ size)
{
  Generic_ARC arc = MEM_AllocateZero(mem, sizeof(__Generic_ARC) + size);
  if (arc)
  {
    arc->mem = mem;
    arc->count = 1;
    arc->mutex = OS_MutexInit();
    if (!arc->mutex)
    {
      MEM_Deallocate(mem, arc);
      arc = nullptr;
    }
  }
  return arc;
}

PTR ARC_Clone(PTR _arc)
{
  Generic_ARC arc = _arc;
  if (arc && OS_MutexLock(arc->mutex))
  {
    ++arc->count;
    OS_MutexUnlock(arc->mutex);
    return arc;
  }
  return nullptr;
}

void ARC_Free(PTR _arc)
{
  Generic_ARC arc = _arc;
  if (arc && OS_MutexLock(arc->mutex))
  {
    if (!--arc->count)
    {
      OS_MutexFree(arc->mutex);
      MEM_Deallocate(arc->mem, arc);
    }
    else OS_MutexUnlock(arc->mutex);
  }
}
