#include <ds.h>

typedef struct __DS_ListNode
{
  struct __DS_ListNode *next;
  struct __DS_ListNode *prev;
} __DS_ListNode;

PTR DS_ListGetNode(DS_List *list, UZ index)
{
  if (!list || index >= list->size) return nullptr;
  for (__DS_ListNode *node = list->first; node; node = node->next)
  {
    if (index--) return node;
  }
  return nullptr;
}

PTR DS_ListRemoveNode(DS_List *list, UZ index)
{
  __DS_ListNode *node = DS_ListGetNode(list, index);
  if (node)
  {
    if (node->next) node->next->prev = node->prev;
    else list->last = node->prev;

    if (node->prev) node->prev->next = node->next;
    else list->first = node->next;

    node->next = nullptr;
    node->prev = nullptr;
    list->size--;
  }
  return node;
}

void DS_ListInsertNode(DS_List *list, UZ index, PTR _node)
{
  __DS_ListNode *node = _node;
  if (!list || !~list->size || index > list->size || !node || node->next || node->prev) return;
  if (index == list->size)
  {
    if (node->prev = list->last) node->prev->next = node;
    else list->first = node;
    list->last = node;
    list->size++;
  }
  else for (__DS_ListNode *next = DS_ListGetNode(list, index); next; next = nullptr)
  {
    if (node->prev = next->prev) node->prev->next = node;
    else list->first = node;
    node->next = next;
    next->prev = node;
    list->size++;
  }
}

void DS_ListAppendNode(DS_List *list, PTR node)
{
  if (list) DS_ListInsertNode(list, list->size, node);
}

void DS_ListPrependNode(DS_List *list, PTR node)
{
  DS_ListInsertNode(list, 0, node);
}

DS_BitField DS_BitFieldAllocate(MEM_Arena *arena, UZ width)
{
  DS_BitField field = { MEM_ArenaAllocateZero(arena, (width + 7) >> 3) };
  if (field.data) field.width = width;
  return field;
}

bool DS_BitFieldGet(DS_BitField bits, UZ index)
{
  if (index >= bits.width) return 0;
  UZ offset = index >> 3;
  UZ shift = index & 7;
  return ((bits.data[offset] >> shift) & 1);
}

bool DS_BitFieldFlip(DS_BitField bits, UZ index)
{
  if (index >= bits.width) return 0;
  UZ offset = index >> 3;
  UZ shift = index & 7;
  return (((bits.data[offset] ^= (U8)(1 << shift)) >> shift) & 1);
}

void DS_BitFieldSet(DS_BitField bits, UZ index)
{
  if (index >= bits.width) return;
  UZ offset = index >> 3;
  UZ shift = index & 7;
  bits.data[offset] |= (U8)(1 << shift);
}

void DS_BitFieldClear(DS_BitField bits, UZ index)
{
  if (index >= bits.width) return;
  UZ offset = index >> 3;
  UZ shift = index & 7;
  bits.data[offset] &= ~(U8)(1 << shift);
}
