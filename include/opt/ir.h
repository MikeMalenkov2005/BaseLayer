#ifndef OPT_IR_H
#define OPT_IR_H

#include <base_layer.h>

/*     THE IR BYTE CODE OPERATES ON A VIRTUAL STACK AND VARIABLES             */
/*     ACTUAL IMPLEMENTATION MAY INCLUDE REGISTERS AS THE TOP OF THE STACK    */
/*     VARIABLES CAN BE LOCAL OR GLOBAL (L/G IN THE OPCODES)                  */
/*     VALUES PRODUCED BY THE IR OPERATIONS ARE PUSHED ON THE STACK           */
/*     VALUES CONSUMED BY THE IR OPERATIONS ARE POPED OFF THE STACK           */
/*     THE TOPMOST VALUE ON THE STACK IS THE LEFTMOST ARGUMENT                */
/*     THUS VALUES MUST BE PUSHED IN THE RTL ORDER BEFORE THE OPERATION       */
/*     A VARIABLE USED BY THE OPERATION ARE ENCODED RIGHT AFTER THE OPCODE    */
/*     THE TYPE USED IN THE OPERATION IS ENCODED AS A PART OF THE OPCODE      */

#define IR_INVALID_INDEX      MAX_U32
#define IR_VALUE_INDEX_MASK   ((U32)(IR_INVALID_INDEX >> 2))
#define IR_VALUE_CLASS_SHIFT  30
#define IR_VALUE_CLASS_MASK   ((U32)(IR_INVALID_INDEX << IR_VALUE_CLASS_SHIFT))

typedef enum IR_ValueClass
{
  IRV_SINGLE,
  IRV_STRING,
  IRV_FUNCTION,
  IRV_INVALID,
} IR_ValueClass;

#define IR_GetValueClass(index) ((IR_ValueClass)((U32)(index) >> IR_VALUE_CLASS_SHIFT))

typedef enum IR_OpCode
{
  IR_INVL, /* INVALID OPERATION */

  IR_LDGA, /* LOAD GLOBAL ADDRESS                         ( -> PTR)           */
  IR_LDGV, /* LOAD GLOBAL VALUE                           ( -> VAL)           */
  IR_STGV, /* STORE GLOBAL VALUE                          (VAL -> VAL)        */

  IR_LDPA, /* LOAD PARAMETER ADDRESS                      ( -> PTR)           */
  IR_LDPV, /* LOAD PARAMETER VALUE                        ( -> VAL)           */
  IR_STPV, /* STORE PARAMETER VALUE                       (VAL -> VAL)        */

  IR_LDLA, /* LOAD LOCAL ADDRESS                          ( -> PTR)           */
  IR_LDLV, /* LOAD LOCAL VALUE                            ( -> VAL)           */
  IR_STLV, /* STORE LOCAL VALUE                           (VAL -> VAL)        */

  IR_INBA, /* INCREMENT AT ADDRESS BEFORE LOADING         (PTR -> VAL)        */
  IR_DEBA, /* DECREMENT AT ADDRESS BEFORE LOADING         (PTR -> VAL)        */
  IR_INAA, /* INCREMENT AT ADDRESS AFTER LOADING          (PTR -> VAL)        */
  IR_DEAA, /* DECREMENT AT ADDRESS AFTER LOADING          (PTR -> VAL)        */

  IR_INBG, /* INCREMENT AT GLOBAL BEFORE LOADING          ( -> VAL)           */
  IR_DEBG, /* DECREMENT AT GLOBAL BEFORE LOADING          ( -> VAL)           */
  IR_INAG, /* INCREMENT AT GLOBAL AFTER LOADING           ( -> VAL)           */
  IR_DEAG, /* DECREMENT AT GLOBAL AFTER LOADING           ( -> VAL)           */

  IR_INBP, /* INCREMENT AT PARAMETER BEFORE LOADING       ( -> VAL)           */
  IR_DEBP, /* DECREMENT AT PARAMETER BEFORE LOADING       ( -> VAL)           */
  IR_INAP, /* INCREMENT AT PARAMETER AFTER LOADING        ( -> VAL)           */
  IR_DEAP, /* DECREMENT AT PARAMETER AFTER LOADING        ( -> VAL)           */

  IR_INBL, /* INCREMENT AT LOCAL BEFORE LOADING           ( -> VAL)           */
  IR_DEBL, /* DECREMENT AT LOCAL BEFORE LOADING           ( -> VAL)           */
  IR_INAL, /* INCREMENT AT LOCAL AFTER LOADING            ( -> VAL)           */
  IR_DEAL, /* DECREMENT AT LOCAL AFTER LOADING            ( -> VAL)           */

  IR_MKLV, /* MAKE LOCAL VARIABLES (COUNT IS SPECIFIED)   ( -> )              */
  IR_RMLV, /* REMOVE LOCAL VARIABLES (COUNT IS SPECIFIED) ( -> )              */

  IR_POP,  /* DISCARD A VALUE                             (VAL -> )           */
  IR_PUSH, /* CREATE A VALUE                              ( -> VAL)           */
  IR_LDS,  /* LOAD STRING ADDRESS                         ( -> PTR)           */
  IR_LDA,  /* LOAD FROM ADDRESS                           (PTR -> VAL)        */
  IR_STA,  /* STORE AT ADDRESS                            (VAL PTR -> VAL)    */

  IR_RETN, /* RETURN NOTHING                              ( -> RET)           */
  IR_RETV, /* RETURN VALUE                                (VAL -> RET)        */

  IR_ADD,  /* ADDITION                                    (VAL VAL -> VAL)    */
  IR_SUB,  /* SUBTRACTION                                 (VAL VAL -> VAL)    */
  IR_MUL,  /* MULTIPLICATION                              (VAL VAL -> VAL)    */
  IR_DIV,  /* DIVISION                                    (VAL VAL -> VAL)    */
  IR_MOD,  /* MODULO                                      (VAL VAL -> VAL)    */
  IR_NEG,  /* NEGATE                                      (VAL -> VAL)        */

  IR_AND,  /* BITWISE AND                                 (VAL VAL -> VAL)    */
  IR_XOR,  /* BITWISE XOR                                 (VAL VAL -> VAL)    */
  IR_OR,   /* BITWISE OR                                  (VAL VAL -> VAL)    */
  IR_NOT,  /* BITWISE NOT                                 (VAL -> VAL)        */

  IR_LAND, /* LOGICAL AND                                 (VAL VAL -> BOOL)   */
  IR_LOR,  /* LOGICAL OR                                  (VAL VAL -> BOOL)   */
  IR_LNOT, /* LOGICAL NOT                                 (VAL VAL -> BOOL)   */

  IR_LST,  /* LESS THEN                                   (VAL VAL -> BOOL)   */
  IR_LEQ,  /* LESS OR EQUAL                               (VAL VAL -> BOOL)   */
  IR_GRT,  /* GREATER THEN                                (VAL VAL -> BOOL)   */
  IR_GEQ,  /* GREATER OR EQUAL                            (VAL VAL -> BOOL)   */
  IR_EQU,  /* EQUAL TO                                    (VAL VAL -> BOOL)   */
  IR_NEQ,  /* NOT EQUAL                                   (VAL VAL -> BOOL)   */

  IR_IGEQ, /* GREATER OR EQUAL                            (VAL VAL -> BOOL)   */

  IR_AIDX, /* INDEX AN ADDRESS                            (PTR VAL -> PTR)    */
  IR_CALL, /* CALL AN ADDRESS (DATA IS # OF ARGS)         (PTR -> CALL)       */

  IR_ASB,  /* CONVERT TO BOOLEAN                          (VAL -> BOOL)       */
  IR_ASF,  /* CONVERT TO 32-BIT FLOAT                     (VAL -> F32)        */
  IR_ASD,  /* CONVERT TO 64-BIT FLOAT                     (VAL -> F64)        */
  IR_ASI,  /* CONVERT TO BOOLEAN                          (VAL -> INT)        */
  IR_IRSZ, /* CHANGE INTEGER BIT WIDTH                    (VAL -> VAL)        */

  IR_JMP,  /* JUMP                                        ( -> JMP)           */
  IR_JZ,   /* JUMP IF ZERO                                (VAL -> ?JMP)       */
  IR_JNZ,  /* JUMP IF NOT ZERO                            (VAL -> ?JMP)       */
} IR_OpCode;

typedef enum IR_Type
{
  IRT_NONE,
  IRT_BOOL,
  IRT_PTR,

  IRT_S8,
  IRT_S16,
  IRT_S32,
  IRT_S64,
  IRT_SZ,

  IRT_U8,
  IRT_U16,
  IRT_U32,
  IRT_U64,
  IRT_UZ,

  IRT_F32,
  IRT_F64,
} IR_Type;

typedef union IRI
{
  struct { U16 code, type; } op;
  U32 data;
} IRI;

DS_ArrayDefine(IRI);

#define IRI_Data(val) ((IRI) { .data = (val) })
#define IRI_Op(c, m)  ((IRI) { .op = { .code = (c), .mode = (m) } })

typedef union IR_Single
{
  PTR ptr;
  F64 f64;
  F32 f32;
  SZ  sz;
  S64 s64;
  S32 s32;
  S16 s16;
  S8  s8;
  UZ  uz;
  U64 u64;
  U32 u32;
  U16 u16;
  U8  u8;
  B8  b;
} IR_Single;

DS_VectorDefine(IR_Single);

typedef struct IR_Global
{
  U32 name;   /* NAME INDEX                         */
  U32 value;  /* VALUE INDEX OR INVALID IF EXTERNAL */
} IR_Global;

DS_VectorDefine(IR_Global);

typedef struct IR_Function
{
  U32 args; /* PARAMETER COUNT OR MAX_U32 IF VARIADIC */
  U32 body; /* IRI BLOCK INDEX OR INVALID IF EXTERNAL */
} IR_Function;

DS_VectorDefine(IR_Function);

typedef DS_Array(IRI) IRI_Block;

DS_VectorDefine(IRI_Block);

typedef struct IR
{
  MEM *mem;
  DS_Vector(STR) names;
  DS_Vector(STR) strings;
  DS_Vector(IR_Single) singles;
  DS_Vector(IR_Function) functions;
  DS_Vector(IR_Global) globals;
  DS_Vector(IRI_Block) blocks;
} IR;

IR IR_Init(MEM *mem);
void IR_Free(IR *ir);

U32 IR_AddName(IR *ir, STR name);
STR IR_GetName(IR *ir, U32 index);
U32 IR_GetNameIndex(IR *ir, STR name);

U32 IR_AddString(IR *ir, STR string);
STR IR_GetString(IR *ir, U32 index);

U32 IR_AddSingle(IR *ir, IR_Single single);
IR_Single IR_GetSingle(IR *ir, U32 index);

U32 IR_AddFunction(IR *ir, U32 args, IRI_Block *body);
IR_Function IR_GetFunction(IR *ir, U32 index);
IRI_Block IR_GetBlock(IR *ir, U32 index);

U32 IR_AddGlobal(IR *ir, U32 name, U32 value);
IR_Global IR_GetGlobal(IR *ir, U32 index);

#endif
