#include <opt/lex.h>
#include <os.h>

LEX LEX_Init(MEM *mem, STR source)
{
  LEX lex = { null };
  if (source.size && (lex.rules = MEM_AllocateArrayTyped(mem, 256, LEX_Rule)))
  {
    lex.mem = mem;
    lex.source = source;
  }
  return lex;
}

void LEX_Free(LEX *lex)
{
  if (lex->rules)
  {
    MEM_Deallocate(lex->mem, lex->rules);
    MemoryZeroStruct(lex);
  }
}

void LEX_SetRuleForByte(LEX *lex, LEX_RuleCallback *callback, PTR data, U8 byte)
{
  if (lex->rules)
  {
    lex->rules[byte].callback = callback;
    lex->rules[byte].data = data;
  }
}

void LEX_SetRuleForRange(LEX *lex, LEX_RuleCallback *callback, PTR data, U8 first, U8 last)
{
  if (lex->rules) for (U16 i = first; i < (U16)last; ++i)
  {
    lex->rules[i].callback = callback;
    lex->rules[i].data = data;
  }
}

U8 LEX_CurrentByte(LEX *lex)
{
  return lex->position < lex->source.size ? lex->source.str[lex->position] : LEX_EOF;
}

void LEX_Increment(LEX *lex)
{
  if (lex->position < lex->source.size)
  {
    U8 byte = lex->source.str[lex->position++];
    if (byte == '\n')
    {
      lex->column = 0;
      ++lex->line;
    }
    else ++lex->column;
  }
}

LEX_Token LEX_NextToken(LEX *lex)
{
  LEX_Token token = { .id = LEX_EOF };
  if (lex->rules)
  {
    U8 byte = LEX_CurrentByte(lex);
    while (!lex->rules[byte].callback && lex->position < lex->source.size)
    {
      LEX_Increment(lex);
      byte = LEX_CurrentByte(lex);
    }
    LEX_RuleCallback *callback = lex->rules[byte].callback;
    UZ start = lex->position;
    token.line = lex->line;
    token.column = lex->column;
    if (callback) token.id = callback(lex, lex->rules[byte].data);
    token.source.str = lex->source.str + start;
    token.source.size = lex->position - start;
  }
  return token;
}

U16 LEX_DefaultKeyWordRule(LEX *lex, LEX_DefaultKeyWordCallback *callback)
{
  STR word = { lex->source.str + lex->position, 0 };
  U8 byte = LEX_CurrentByte(lex);
  while (byte == '_' || (U8)(byte - '0') < 10 || (U8)((byte & 0xDF) - 'A') < 26)
  {
    LEX_Increment(lex);
    byte = LEX_CurrentByte(lex);
    ++word.size;
  }
  return callback(word);
}

U16 LEX_DefaultStringRule(LEX *lex, UP escape)
{
  U8 end = LEX_CurrentByte(lex);
  bool skip = true;
  for (U8 byte = end; byte && (skip || byte != end); byte = LEX_CurrentByte(lex))
  {
    skip = (byte == (U8)escape);
    LEX_Increment(lex);
  }
  LEX_Increment(lex);
  return end;
}

