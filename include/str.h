#ifndef STR_H
#define STR_H

#include <mem.h>

#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                             ANSI & UTF-8 STRINGS                             *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct STR
{
  U8 *str;
  UZ size;
} STR;

#define STR_Static(s) ((STR) { .str = (U8*)(s), .size = sizeof(s) - 1 })
#define STR_Make(s) ((STR) { .str = (U8*)(s), .size = strlen(s) })

STR STR_Allocate(MEM_Arena *arena, UZ size);
STR STR_Copy(MEM_Arena *arena, STR other);
STR STR_Cat(MEM_Arena *arena, STR left, STR right);
STR STR_Replace(MEM_Arena *arena, STR string, STR substring, STR replacement);

UZ STR_Count(STR string, STR substring);
UZ STR_FindFirst(STR string, STR substring, UZ offset);
UZ STR_FindLast(STR string, STR substring, UZ offset);

U32 STR_Hash(STR string);
U64 STR_Hash64(STR string);
bool STR_Equals(STR left, STR right);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                UTF-16 STRINGS                                *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct STR16
{
  U16 *str;
  UZ size;
} STR16;

#define STR16_Static(s) ((STR16) { .str = (s), .size = sizeof(s) / 2 - 1 })
STR16 STR16_Make(U16 *s);

STR16 STR16_Allocate(MEM_Arena *arena, UZ size);

STR16 STR16_From_STR(MEM_Arena *arena, STR string);
STR STR_From_STR16(MEM_Arena *arena, STR16 string);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                      UTF-8 & UTF-16 ENCODING & DECODING                      *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

S32 UTF8_DecodeFirst(STR string);
UZ UTF8_Encode(U8 *buffer, UZ capacity, S32 codepoint);
UZ UTF8_GetLength(STR string);

S32 UTF16_DecodeFirst(STR16 string);
UZ UTF16_Encode(U16 *buffer, UZ capacity, S32 codepoint);
UZ UTF16_GetLength(STR16 string);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                             UMBRA STYLE STRINGS                              *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct USTR
{
  U32 size;
  union { U32 u; U8 s[4]; } prefix;
  union { U64 u; U8 s[8]; U8 *p; } data;
} USTR;

USTR USTR_Init(STR string);

#define USTR_Static(s) USTR_Init(STR_Static(s))
#define USTR_Make(s) USTR_Init(STR_Make(s))

USTR USTR_From_STR(MEM_Arena *arena, STR string);
STR STR_From_USTR(MEM_Arena *arena, USTR string);

U32 USTR_Hash(USTR string);
U64 USTR_Hash64(USTR string);
bool USTR_Equals(USTR left, USTR right);

#endif
