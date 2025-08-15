#include <opt/lex.h>
#include <os.h>

LEX LEX_Init(MEM_Arena *arena, STR source)
{
  LEX lex = { null };
  if (source.size && (lex.rules = MEM_ArenaAllocateArrayTyped(arena, 256, LEX_Rule))) lex.source = source;
  return lex;
}

LEX LEX_FromFile(MEM_Arena *arena, STR path)
{
  LEX lex = { null };
  OS_File file = OS_FileOpen(path, OS_FILE_OPEN_CREATE);
  if (file)
  {
    U64 size = OS_FileSize(file);
    if (size && size < MAX_UZ)
    {
      STR source = STR_Allocate(arena, (UZ)size);
      if (source.str)
      {
        UZ bytes = OS_FileRead(file, source);
        if (bytes)
        {
          if (bytes < source.size)
          {
            MEM_ArenaDeallocateSize(arena, source.size - bytes);
            source.str[bytes] = 0;
            source.size = bytes;
          }
          lex = LEX_Init(arena, source);
        }
        else MEM_ArenaDeallocate(arena, source.str);
      }
    }
    OS_FileClose(file);
  }
  return lex;
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
