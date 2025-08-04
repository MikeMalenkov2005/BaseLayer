#ifndef DS_H
#define DS_H

#include <defines.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                 ARRAY TYPES                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define DS_Array(T) Glue(DS_Array_, T)

#define DS_ArrayDefine(T)   \
typedef struct DS_Array(T)  \
{                           \
  T data;                   \
  UZ size;                  \
} DS_Array(T)

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

DS_ListNode *DS_ListGetNode(DS_List *list, UZ index);
DS_ListNode *DS_ListRemoveNode(DS_List *list, UZ index);
void DS_ListInsertNode(DS_List *list, UZ index, DS_ListNode *node);

void DS_ListAppendNode(DS_List *list, DS_ListNode *node);
void DS_ListPrependNode(DS_List *list, DS_ListNode *node);

#endif
