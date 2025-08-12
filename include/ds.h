#ifndef DS_H
#define DS_H

#include <str.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                 ARRAY TYPES                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define DS_Array(T) Glue(DS_Array_, T)

#define DS_ArrayDefine(T)   \
typedef struct DS_Array(T)  \
{                           \
  T *data;                  \
  UZ size;                  \
} DS_Array(T)

#define DS_ArrayAllocate(T, arena, size) \
  ((DS_Array(T)) { MEM_ArenaAllocate(arena, sizeof(T) * (size)), (size) })

DS_ArrayDefine(S8);
DS_ArrayDefine(S16);
DS_ArrayDefine(S32);
DS_ArrayDefine(S64);
DS_ArrayDefine(SZ);
DS_ArrayDefine(SP);

DS_ArrayDefine(U8);
DS_ArrayDefine(U16);
DS_ArrayDefine(U32);
DS_ArrayDefine(U64);
DS_ArrayDefine(UZ);
DS_ArrayDefine(UP);

DS_ArrayDefine(B8);
DS_ArrayDefine(B16);
DS_ArrayDefine(B32);
DS_ArrayDefine(B64);

DS_ArrayDefine(F32);
DS_ArrayDefine(F64);

DS_ArrayDefine(PTR);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                 LINKED LIST                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct DS_ListNode
{
  struct DS_ListNode *next;
  struct DS_ListNode *prev;
  U8 value[0];
} DS_ListNode;

typedef struct DS_List
{
  DS_ListNode *first;
  DS_ListNode *last;
  UZ size;
} DS_List;

#define DS_ListInit() ((DS_List) { null })

DS_ListNode *DS_ListGetNode(DS_List *list, UZ index);
DS_ListNode *DS_ListRemoveNode(DS_List *list, UZ index);
void DS_ListInsertNode(DS_List *list, UZ index, DS_ListNode *node);

void DS_ListAppendNode(DS_List *list, DS_ListNode *node);
void DS_ListPrependNode(DS_List *list, DS_ListNode *node);

DS_ListNode *DS_ListNodeAllocate(MEM_Arena *arena, UZ size);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                 BINARY TREE                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct DS_BinaryTree
{
  struct DS_BinaryTree *left;
  struct DS_BinaryTree *right;
  U8 data[0];
} DS_BinaryTree;

DS_BinaryTree *DS_BinaryTreeAllocate(MEM_Arena *arena, UZ size);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                               NON-BINARY TREE                                *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct DS_Tree
{
  struct { struct DS_Tree *next, *prev; } siblings;
  struct { struct DS_Tree *first, *last; UZ size; } children;
  U8 data[0];
} DS_Tree;

DS_Tree *DS_TreeAllocate(MEM_Arena *arena, UZ size);

DS_Tree *DS_TreeGetChild(DS_Tree *tree, UZ index);
DS_Tree *DS_TreeRemoveChild(DS_Tree *tree, UZ index);
void DS_TreeInsertChild(DS_Tree *tree, UZ index, DS_Tree *child);

void DS_TreeAppendChild(DS_Tree *tree, DS_Tree *child);
void DS_TreePrependChild(DS_Tree *tree, DS_Tree *child);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                STR HASH MAP                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct DS_MapEntry
{
  struct { struct DS_MapEntry *next, *prev; } collisions;
  STR key;
  U64 hash;
  U8 value[0];
} DS_MapEntry;

typedef struct DS_Map
{
  DS_Array(PTR) buckets;
  UZ size;
} DS_Map;

DS_Map DS_MapAllocate(MEM_Arena *arena, UZ buckets);

DS_MapEntry *DS_MapGetEntry(DS_Map *map, STR key);
DS_MapEntry *DS_MapRemoveEntry(DS_Map *map, STR key);
DS_MapEntry *DS_MapPutEntry(DS_Map *map, DS_MapEntry *entry);

DS_MapEntry *DS_MapEntryAllocate(MEM_Arena *arena, STR key, UZ size);

#endif
