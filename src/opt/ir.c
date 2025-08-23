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
    DS_VectorClear(ir->mem, &ir->strings);
    DS_VectorClear(ir->mem, &ir->values);
    DS_VectorClear(ir->mem, &ir->globals);
    DS_VectorClear(ir->mem, &ir->functions);
    ir->mem = nullptr;
  }
}

U32 IR_AddString(IR *ir, STR string)
{
  if (ir->strings.size >= IR_VALUE_INDEX_MASK) return IR_INVALID_INDEX;
  U32 index = 0;
  while (index < (U32)ir->strings.size && !STR_Equals(ir->strings.data[index], string)) ++index;
  if (index == (U32)ir->strings.size) DS_VectorAppend(ir->mem, &ir->strings, string);
  if (index == (U32)ir->strings.size) return IR_INVALID_INDEX;
  return (index | IR_VALUE_STRING_BIT);
}

U32 IR_AddValue(IR *ir, IR_Value value)
{
  if (ir->values.size >= IR_VALUE_INDEX_MASK) return IR_INVALID_INDEX;
  U32 index = 0;
  while (index < (U32)ir->values.size && ir->values.data[index].u64 != value.u64) ++index;
  if (index == (U32)ir->values.size) DS_VectorAppend(ir->mem, &ir->values, value);
  if (index == (U32)ir->values.size) return IR_INVALID_INDEX;
  return index;
}

U32 IR_AddGlobal(IR *ir, STR name, U32 value)
{
  if (ir->globals.size >= IR_INVALID_INDEX || ir->names.size >= IR_INVALID_INDEX) return IR_INVALID_INDEX;
  U32 index = IR_GetNameIndex(ir, name);
  if (index != IR_INVALID_INDEX) return IR_INVALID_INDEX;
  index = (U32)ir->names.size;
  DS_VectorAppend(ir->mem, &ir->names, name);
  if (index == (U32)ir->names.size) return IR_INVALID_INDEX;
  IR_Global global = { index, value };
  index = (U32)ir->globals.size;
  DS_VectorAppend(ir->mem, &ir->globals, global);
  if (index == (U32)ir->globals.size) return (--ir->names.size, IR_INVALID_INDEX);
  return index;
}

U32 IR_AddFunction(IR *ir, STR name, U32 args, IRI_Block *body)
{
  if (ir->functions.size >= IR_INVALID_INDEX || ir->names.size >= IR_INVALID_INDEX) return IR_INVALID_INDEX;
  U32 index = IR_GetNameIndex(ir, name);
  if (index != IR_INVALID_INDEX) return IR_INVALID_INDEX;
  index = (U32)ir->names.size;
  DS_VectorAppend(ir->mem, &ir->names, name);
  if (index == (U32)ir->names.size) return IR_INVALID_INDEX;
  IR_Function function = { index, args, IR_INVALID_INDEX };
  if (body)
  {
    if (ir->blocks.size >= IR_INVALID_INDEX) return IR_INVALID_INDEX;
    index = (U32)ir->blocks.size;
    DS_VectorAppend(ir->mem, &ir->blocks, *body);
    if (index == (U32)ir->blocks.size) return IR_INVALID_INDEX;
    function.body = index;
  }
  index = (U32)ir->functions.size;
  DS_VectorAppend(ir->mem, &ir->functions, function);
  if (index == (U32)ir->functions.size) return (--ir->names.size, IR_INVALID_INDEX);
  return index;
}

STR IR_GetName(IR *ir, U32 index)
{
  return index < ir->names.size ? ir->names.data[index] : (STR) { nullptr };
}

U32 IR_GetNameIndex(IR *ir, STR name)
{
  for (U32 i = 0; i < (U32)ir->names.size; ++i)
  {
    if (STR_Equals(ir->names.data[i], name)) return i;
  }
  return IR_INVALID_INDEX;
}

STR IR_GetString(IR *ir, U32 index)
{
  index = IR_IsStringIndex(index) ? (index & IR_VALUE_INDEX_MASK) : IR_INVALID_INDEX;
  return  index < ir->strings.size ? ir->strings.data[index] : (STR) { nullptr };
}

IR_Value IR_GetValue(IR *ir, U32 index)
{
  index = IR_IsValueIndex(index) ? (index & IR_VALUE_INDEX_MASK) : IR_INVALID_INDEX;
  return  index < ir->values.size ? ir->values.data[index] : (IR_Value) { .u64 = 0 };
}

IR_Global *IR_GetGlobal(IR *ir, U32 index)
{
  return index < ir->globals.size ? &ir->globals.data[index] : nullptr;
}

IR_Function *IR_GetFunction(IR *ir, U32 index)
{
  return index < ir->functions.size ? &ir->functions.data[index] : nullptr;
}
