#ifndef DS_H
#define DS_H

#include <str.h>

#define X_FOR_BASE_TYPES  \
X(S8);                    \
X(S16);                   \
X(S32);                   \
X(S64);                   \
X(SZ);                    \
X(SP);                    \
X(U8);                    \
X(U16);                   \
X(U32);                   \
X(U64);                   \
X(UZ);                    \
X(UP);                    \
X(B8);                    \
X(B16);                   \
X(B32);                   \
X(B64);                   \
X(F32);                   \
X(F64);                   \
X(PTR);

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

#define X DS_ArrayDefine
X_FOR_BASE_TYPES
#undef X

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                 LINKED LIST                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define DS_ListNode(T) Glue(DS_ListNode_, T)

#define DS_ListNodeDefine(T)  \
typedef struct DS_ListNode(T) \
{                             \
  PTR next;                   \
  PTR prev;                   \
  T value;                    \
} DS_ListNode(T)

#define X DS_ListNodeDefine
X_FOR_BASE_TYPES
#undef X

typedef struct DS_List
{
  PTR first;
  PTR last;
  UZ size;
} DS_List;

#define DS_ListInit() ((DS_List) { null })

PTR DS_ListGetNode(DS_List *list, UZ index);
PTR DS_ListRemoveNode(DS_List *list, UZ index);
void DS_ListInsertNode(DS_List *list, UZ index, PTR _node);

void DS_ListAppendNode(DS_List *list, PTR node);
void DS_ListPrependNode(DS_List *list, PTR node);

#define DS_ListNodeAllocate(T, a) ((DS_ListNode(T)*)MEM_ArenaAllocateZero(a, sizeof(DS_ListNode(T))))

#define DS_ListForEach(T, N, l) for (DS_ListNode(T) N = (l)->first; N; N = N->next)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                 BINARY TREE                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define DS_BinaryTree(T) Glue(DS_BinaryTree_, T)

#define DS_BinaryTreeDefine(T)    \
typedef struct DS_BinaryTree(T)   \
{                                 \
  struct DS_BinaryTree(T) *left;  \
  struct DS_BinaryTree(T) *right; \
  T value;                        \
} DS_BinaryTree(T)

#define X DS_BinaryTreeDefine
X_FOR_BASE_TYPES
#undef X

#define DS_BinaryTreeAllocate(T, a) ((DS_BinaryTree(T)*)MEM_ArenaAllocateZero(a, sizeof(DS_BinaryTree(T))))

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                               NON-BINARY TREE                                *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define DS_Tree(T) Glue(DS_Tree_, T)

#define DS_TreeDefine(T)  \
typedef struct DS_Tree(T) \
{                         \
  PTR siblings[2];        \
  DS_List children;       \
  T value;                \
} DS_Tree(T)

#define X DS_TreeDefine
X_FOR_BASE_TYPES
#undef X

#define DS_TreeAllocate(T, a) ((DS_Tree(T)*)MEM_ArenaAllocateZero(a, sizeof(DS_Tree(T))))

#define DS_TreeGetChild(tree, index) DS_ListGetNode((tree)->children, index)
#define DS_TreeRemoveChild(tree, index) DS_ListRemoveNode((tree)->children, index)
#define DS_TreeInsertChild(tree, index, child) DS_ListInsertNode((tree)->children, index, child)

#define DS_TreeAppendChild(tree, child) DS_ListAppendNode((tree)->children, child)
#define DS_TreePrependChild(tree, child) DS_ListPrependNode((tree)->children, child)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                  BIT-FIELD                                   *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct DS_BitField
{
  U8 *data;
  UZ width;
} DS_BitField;

DS_BitField DS_BitFieldAllocate(MEM_Arena *arena, UZ width);

bool DS_BitFieldGet(DS_BitField bits, UZ index);
bool DS_BitFieldFlip(DS_BitField bits, UZ index);
void DS_BitFieldSet(DS_BitField bits, UZ index);
void DS_BitFieldClear(DS_BitField bits, UZ index);

#define DS_BitFieldSetValue(bits, index, value) (value ? DS_BitFieldSet(bits, index) : DS_BitFieldClear(bits, index))

#endif
