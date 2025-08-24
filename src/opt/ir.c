#include <opt/ir.h>

IR IR_Init(MEM *mem)
{
  IR ir = { mem };
  return ir;
}

void IR_Free(IR *ir)
{
  if (ir->mem)
  {
    DS_VectorClear(ir->mem, &ir->names);
    DS_VectorClear(ir->mem, &ir->globals);
    DS_VectorClear(ir->mem, &ir->functions);
    DS_VectorClear(ir->mem, &ir->arrays);
    DS_VectorClear(ir->mem, &ir->values);
    ir->mem = nullptr;
  }
}

U32 IR_AddName(IR *ir, STR name)
{
  if (ir->names.size >= IR_INVALID_INDEX) return IR_INVALID_INDEX;
  U32 index = 0;
  while (index < (U32)ir->names.size && !STR_Equals(name, ir->names.data[index])) ++index;
  if (index == (U32)ir->names.size) DS_VectorAppend(ir->mem, &ir->names, name);
  if (index == (U32)ir->names.size) return IR_INVALID_INDEX;
  return index;
}

STR IR_GetName(IR *ir, U32 index)
{
  return index < ir->names.size ? ir->names.data[index] : (STR) { nullptr };
}

U32 IR_FindName(IR *ir, STR name)
{
  for (U32 index = 0; index < ir->names.size; ++index)
  {
    if (STR_Equals(name, ir->names.data[index])) return index;
  }
  return IR_INVALID_INDEX;
}

U32 IR_AddGlobal(IR *ir, U32 name, U32 value, UZ size)
{
  IR_Global global = { name, value, size };
  if (ir->globals.size >= IR_INVALID_INDEX) return IR_INVALID_INDEX;
  if (IR_FindFunction(ir, name) != IR_INVALID_INDEX) return IR_INVALID_INDEX; /* TODO: FIND OUT IF IT IS GOOD. */
  U32 index = IR_FindGlobal(ir, name);
  if (index == IR_INVALID_INDEX)
  {
    index = (U32)ir->globals.size;
    DS_VectorAppend(ir->mem, &ir->globals, global);
  }
  else ir->globals.data[index] = global;
  if (index == (U32)ir->globals.size) return IR_INVALID_INDEX;
  return index;
}

IR_Global IR_GetGlobal(IR *ir, U32 index)
{
  return index < ir->globals.size ? ir->globals.data[index] : (IR_Global) { IR_INVALID_INDEX, IR_INVALID_INDEX };
}

U32 IR_FindGlobal(IR *ir, U32 name)
{
  for (U32 index = 0; index < ir->globals.size; ++index)
  {
    if (ir->globals.data[index].name == name) return index;
  }
  return IR_INVALID_INDEX;
}

U32 IR_AddFunction(IR *ir, U32 name, U32 args, DS_Array(IRI) body)
{
  IR_Function function = { name, args, body };
  if (ir->functions.size >= IR_INVALID_INDEX) return IR_INVALID_INDEX;
  if (IR_FindGlobal(ir, name) != IR_INVALID_INDEX) return IR_INVALID_INDEX; /* TODO: FIND OUT IF IT IS GOOD. */
  U32 index = IR_FindFunction(ir, name);
  if (index == IR_INVALID_INDEX)
  {
    index = (U32)ir->functions.size;
    DS_VectorAppend(ir->mem, &ir->functions, function);
  }
  else ir->functions.data[index] = function;
  if (index == (U32)ir->functions.size) return IR_INVALID_INDEX;
  return index;
}

IR_Function IR_GetFunction(IR *ir, U32 index)
{
  return index < ir->functions.size ? ir->functions.data[index] : (IR_Function) { IR_INVALID_INDEX };
}

U32 IR_FindFunction(IR *ir, U32 name)
{
  for (U32 index = 0; index < ir->functions.size; ++index)
  {
    if (ir->functions.data[index].name == name) return index;
  }
  return IR_INVALID_INDEX;
}

U32 IR_AddArray(IR *ir, STR array)
{
  if (ir->arrays.size >= IR_VALUE_INDEX_MASK) return IR_INVALID_INDEX;
  U32 index = IR_FindArray(ir, array);
  if (index == IR_INVALID_INDEX)
  {
    index = (U32)ir->arrays.size;
    DS_VectorAppend(ir->mem, &ir->arrays, array);
  }
  if (index == (U32)ir->arrays.size) return IR_INVALID_INDEX;
  return index;
}

STR IR_GetArray(IR *ir, U32 index)
{
  index = index & IR_VALUE_ARRAY_BIT ? index & IR_VALUE_INDEX_MASK : IR_INVALID_INDEX;
  return index < ir->arrays.size ? ir->arrays.data[index] : (STR) { null };
}

U32 IR_FindArray(IR *ir, STR array)
{
  for (U32 index = 0; index < ir->arrays.size; ++index)
  {
    if (STR_Equals(ir->arrays.data[index], array)) return index | IR_VALUE_ARRAY_BIT;
  }
  return IR_INVALID_INDEX;
}

U32 IR_AddValue(IR *ir, U64 value)
{
  if (ir->values.size >= IR_VALUE_INDEX_MASK) return IR_INVALID_INDEX;
  U32 index = IR_FindValue(ir, value);
  if (index == IR_INVALID_INDEX)
  {
    index = (U32)ir->values.size;
    DS_VectorAppend(ir->mem, &ir->values, value);
  }
  if (index == (U32)ir->values.size) return IR_INVALID_INDEX;
  return index;
}

U64 IR_GetValue(IR *ir, U32 index)
{
  index = index & IR_VALUE_ARRAY_BIT ? IR_INVALID_INDEX : index & IR_VALUE_INDEX_MASK;
  return index < ir->values.size ? ir->values.data[index] : null;
}

U32 IR_FindValue(IR *ir, U64 value)
{
  for (U32 index = 0; index < ir->values.size; ++index)
  {
    if (ir->values.data[index] == value) return index;
  }
  return IR_INVALID_INDEX;
}

