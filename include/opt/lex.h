#ifndef OPT_LEX_H
#define OPT_LEX_H

#include <str.h>

#define LEX_EOF 0

typedef struct LEX_Token
{
  STR source;
  U32 line;
  U16 column;
  U16 id;
} LEX_Token;

typedef struct LEX_Rule
{
  PTR callback;
  PTR data;
} LEX_Rule;

typedef struct LEX
{
  LEX_Rule *rules;
  STR source;
  UZ position;
  U32 line;
  U16 column;
} LEX;

typedef U16 LEX_RuleCallback(LEX *lex, PTR data);

LEX LEX_Init(MEM *mem, STR source);
LEX LEX_FromFile(MEM *mem, STR path);

void LEX_SetRuleForByte(LEX *lex, LEX_RuleCallback *callback, PTR data, U8 byte);
void LEX_SetRuleForRange(LEX *lex, LEX_RuleCallback *callback, PTR data, U8 first, U8 last);

U8 LEX_CurrentByte(LEX *lex);
void LEX_Increment(LEX *lex);

LEX_Token LEX_NextToken(LEX *lex);

#define LEX_ForEach(N, lex) \
  for (LEX_Token N = LEX_NextToken(lex); N.id; N = LEX_NextToken(lex))

#endif
