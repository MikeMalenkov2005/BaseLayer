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
    DS_VectorClear(ir->mem, &ir->singles);
    DS_VectorClear(ir->mem, &ir->functions);
    DS_VectorClear(ir->mem, &ir->globals);
    DS_VectorClear(ir->mem, &ir->blocks);
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

U32 IR_GetNameIndex(IR *ir, STR name)
{
  for (U32 index = 0; index < ir->names.size; ++index)
  {
    if (STR_Equals(name, ir->names.data[index])) return index;
  }
  return IR_INVALID_INDEX;
}

U32 IR_AddString(IR *ir, STR string)
{
  if (ir->strings.size >= IR_VALUE_INDEX_MASK) return IR_INVALID_INDEX;
  U32 index = 0;
  while (index < (U32)ir->strings.size && !STR_Equals(string, ir->strings.data[index])) ++index;
  if (index == (U32)ir->strings.size) DS_VectorAppend(ir->mem, &ir->strings, string);
  if (index == (U32)ir->strings.size) return IR_INVALID_INDEX;
  return index | (IRV_STRING << IR_VALUE_CLASS_SHIFT);
}

STR IR_GetString(IR *ir, U32 index)
{
  index = IR_GetValueClass(index) == IRV_STRING ? index & IR_VALUE_INDEX_MASK : IR_INVALID_INDEX;
  return index < ir->strings.size ? ir->strings.data[index] : (STR) { nullptr };
}

U32 IR_AddSingle(IR *ir, IR_Single single)
{
  if (ir->singles.size >= IR_VALUE_INDEX_MASK) return IR_INVALID_INDEX;
  U32 index = 0;
  while (index < (U32)ir->singles.size && single.u64 != ir->singles.data[index].u64) ++index;
  if (index == (U32)ir->singles.size) DS_VectorAppend(ir->mem, &ir->singles, single);
  if (index == (U32)ir->singles.size) return IR_INVALID_INDEX;
  return index | (IRV_SINGLE << IR_VALUE_CLASS_SHIFT);
}

IR_Single IR_GetSingle(IR *ir, U32 index)
{
  index = IR_GetValueClass(index) == IRV_SINGLE ? index & IR_VALUE_INDEX_MASK : IR_INVALID_INDEX;
  return index < ir->singles.size ? ir->singles.data[index] : (IR_Single) { .u64 = 0 };
}

U32 IR_AddFunction(IR *ir, U32 args, IRI_Block *body)
{
  if (ir->functions.size >= IR_VALUE_INDEX_MASK) return IR_INVALID_INDEX;
  IR_Function function = { args, IR_INVALID_INDEX };
  if (body)
  {
    if (ir->blocks.size >= IR_INVALID_INDEX) return IR_INVALID_INDEX;
    U32 index = (U32)ir->blocks.size;
    DS_VectorAppend(ir->mem, &ir->blocks, *body);
    if (index == (U32)ir->blocks.size) return IR_INVALID_INDEX;
  }
  U32 index = (U32)ir->functions.size;
  DS_VectorAppend(ir->mem, &ir->functions, function);
  if (index == (U32)ir->functions.size) return IR_INVALID_INDEX;
  return index | (IRV_FUNCTION << IR_VALUE_CLASS_SHIFT);
}

IR_Function IR_GetFunction(IR *ir, U32 index)
{
  index = IR_GetValueClass(index) == IRV_FUNCTION ? index & IR_VALUE_INDEX_MASK : IR_INVALID_INDEX;
  return index < ir->functions.size ? ir->functions.data[index] : (IR_Function) { IR_INVALID_INDEX, IR_INVALID_INDEX };
}

IRI_Block IR_GetBlock(IR *ir, U32 index)
{
  return index < ir->blocks.size ? ir->blocks.data[index] : (IRI_Block) { nullptr };
}

U32 IR_AddGlobal(IR *ir, U32 name, U32 value)
{
  if (ir->globals.size >= IR_INVALID_INDEX) return IR_INVALID_INDEX;
  IR_Global global = { name, value };
  U32 index = (U32)ir->globals.size;
  DS_VectorAppend(ir->mem, &ir->globals, global);
  if (index == (U32)ir->globals.size) return IR_INVALID_INDEX;
  return index;
}

IR_Global IR_GetGlobal(IR *ir, U32 index)
{
  return index < ir->globals.size ? ir->globals.data[index] : (IR_Global) { IR_INVALID_INDEX, IR_INVALID_INDEX };
}

