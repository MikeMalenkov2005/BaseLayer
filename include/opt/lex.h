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
  MEM *mem;
  LEX_Rule *rules;
  STR source;
  UZ position;
  U32 line;
  U16 column;
} LEX;

typedef U16 LEX_RuleCallback(LEX *lex, PTR data);

LEX LEX_Init(MEM *mem, STR source);
void LEX_Free(LEX *lex);

void LEX_SetRuleForByte(LEX *lex, LEX_RuleCallback *callback, PTR data, U8 byte);
void LEX_SetRuleForRange(LEX *lex, LEX_RuleCallback *callback, PTR data, U8 first, U8 last);

U8 LEX_CurrentByte(LEX *lex);
void LEX_Increment(LEX *lex);

LEX_Token LEX_NextToken(LEX *lex);

#define LEX_ForEach(N, lex) \
  for (LEX_Token N = LEX_NextToken(lex); N.id; N = LEX_NextToken(lex))

typedef U16 LEX_DefaultKeyWordCallback(STR word);

typedef struct LEX_DefaultStringSettings
{
  U16 token;
  U8 escape;
} LEX_DefaultStringSettings;

U16 LEX_DefaultKeyWordRule(LEX *lex, LEX_DefaultKeyWordCallback *callback);
U16 LEX_DefaultStringRule(LEX *lex, LEX_DefaultStringSettings *settings);

#endif
