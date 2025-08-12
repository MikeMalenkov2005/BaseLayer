#include <ds.h>

DS_ListNode *DS_ListGetNode(DS_List *list, UZ index)
{
  if (!list || index >= list->size) return nullptr;
  for (DS_ListNode *node = list->first; node; node = node->next)
  {
    if (index--) return node;
  }
  return nullptr;
}

DS_ListNode *DS_ListRemoveNode(DS_List *list, UZ index)
{
  DS_ListNode *node = DS_ListGetNode(list, index);
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

void DS_ListInsertNode(DS_List *list, UZ index, DS_ListNode *node)
{
  if (!list || !~list->size || index > list->size || !node || node->next || node->prev) return;
  if (index == list->size)
  {
    if (node->prev = list->last) node->prev->next = node;
    else list->first = node;
    list->last = node;
    list->size++;
  }
  else for (DS_ListNode *next = DS_ListGetNode(list, index); next; next = nullptr)
  {
    if (node->prev = next->prev) node->prev->next = node;
    else list->first = node;
    node->next = next;
    next->prev = node;
    list->size++;
  }
}

void DS_ListAppendNode(DS_List *list, DS_ListNode *node)
{
  if (list) DS_ListInsertNode(list, list->size, node);
}

void DS_ListPrependNode(DS_List *list, DS_ListNode *node)
{
  DS_ListInsertNode(list, 0, node);
}

DS_ListNode *DS_ListNodeAllocate(MEM_Arena *arena, UZ size)
{
  DS_ListNode *node = (void*)MEM_ArenaAllocate(arena, sizeof(DS_ListNode) + size);
  return node ? MemoryZeroStruct(node) : node;
}

DS_BinaryTree *DS_BinaryTreeAllocate(MEM_Arena *arena, UZ size)
{
  DS_BinaryTree *tree = (void*)MEM_ArenaAllocate(arena, sizeof(DS_BinaryTree) + size);
  return tree ? MemoryZeroStruct(tree) : tree;
}

DS_Tree *DS_TreeAllocate(MEM_Arena *arena, UZ size)
{
  DS_Tree *tree = (void*)MEM_ArenaAllocate(arena, sizeof(DS_Tree) + size);
  return tree ? MemoryZeroStruct(tree) : tree;
}

DS_Tree *DS_TreeGetChild(DS_Tree *tree, UZ index)
{
  return tree ? (void*)DS_ListGetNode((void*)&tree->children, index) : nullptr;
}

DS_Tree *DS_TreeRemoveChild(DS_Tree *tree, UZ index)
{
  return tree ? (void*)DS_ListRemoveNode((void*)&tree->children, index) : nullptr;
}

void DS_TreeInsertChild(DS_Tree *tree, UZ index, DS_Tree *child)
{
  if (tree && child) DS_ListInsertNode((void*)&tree->children, index, (void*)child);
}

void DS_TreeAppendChild(DS_Tree *tree, DS_Tree *child)
{
  if (tree) DS_TreeInsertChild(tree, tree->children.size, child);
}

void DS_TreePrependChild(DS_Tree *tree, DS_Tree *child)
{
  DS_TreeInsertChild(tree, 0, child);
}

DS_Map DS_MapAllocate(MEM_Arena *arena, UZ buckets)
{
  DS_Map map = { .buckets = DS_ArrayAllocate(PTR, arena, buckets) };
  if (map.buckets.data) MemoryZero(map.buckets.data, map.buckets.size * sizeof(*map.buckets.data));
  return map;
}

DS_MapEntry *DS_MapGetEntry(DS_Map *map, STR key)
{
  if (!map || !map->buckets.size) return nullptr;
  U64 hash = STR_Hash64(key);
  UZ index = (UZ)(hash % (U64)map->buckets.size);
  for (DS_MapEntry *entry = map->buckets.data[index]; entry; entry = entry->collisions.next)
  {
    if (entry->hash == hash && STR_Equals(entry->key, key)) return entry;
  }
  return nullptr;
}

DS_MapEntry *DS_MapRemoveEntry(DS_Map *map, STR key)
{
  DS_MapEntry *entry = DS_MapGetEntry(map, key);
  if (entry)
  {
    if (entry->collisions.prev) entry->collisions.prev->collisions.next = entry->collisions.next;
    if (entry->collisions.next) entry->collisions.next->collisions.prev = entry->collisions.prev;
    MemoryZeroStruct(&entry->collisions);
    --map->size;
  }
  return entry;
}

DS_MapEntry *DS_MapPutEntry(DS_Map *map, DS_MapEntry *entry)
{
  if (!map || !map->buckets.size || !entry) return nullptr;
  DS_MapEntry *prev = DS_MapRemoveEntry(map, entry->key);
  UZ index = (UZ)(entry->hash % (U64)map->buckets.size);
  if (entry->collisions.next = map->buckets.data[index])
  {
    entry->collisions.next->collisions.prev = entry;
  }
  map->buckets.data[index] = entry;
  ++map->size;
  return prev;
}

DS_MapEntry *DS_MapEntryAllocate(MEM_Arena *arena, STR key, UZ size)
{
  DS_MapEntry *entry = (void*)MEM_ArenaAllocate(arena, sizeof(DS_MapEntry) + size);
  if (entry)
  {
    MemoryZeroStruct(&entry->collisions);
    entry->key = key;
    entry->hash = STR_Hash64(key);
  }
  return entry;
}
