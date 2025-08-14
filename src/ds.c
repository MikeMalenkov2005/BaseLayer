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
