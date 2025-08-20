#ifndef RC_H
#define RC_H

#include <mem.h>
#include <os.h>

#define __RC(T) Glue(__RC_, T)
#define RC(T) Glue(RC_, T)

#define RC_Define(T)    \
typedef struct __RC(T)  \
{                       \
  MEM *mem;             \
  UZ count;             \
  T value;              \
} __RC(T), *RC(T)

PTR __RC_Init(MEM *mem, UZ size);
#define RC_Init(T, mem) __RC_Init(mem, sizeof(__RC(T)))

PTR RC_Clone(PTR _rc);
void RC_Free(PTR _rc);

#define __ARC(T) Glue(__ARC_, T)
#define ARC(T) Glue(ARC_, T)

#define ARC_Define(T)    \
typedef struct __ARC(T)  \
{                       \
  MEM *mem;             \
  OS_Mutex mutex;       \
  UZ count;             \
  T value;              \
} __ARC(T), *ARC(T)

PTR __ARC_Init(MEM *mem, UZ size);
#define ARC_Init(T, mem) __ARC_Init(mem, sizeof(__RC(T)))

PTR ARC_Clone(PTR _arc);
void ARC_Free(PTR _arc);

#endif
