#include <opt/json.h>

#include <math.h>

#define JSON_IsWhiteSpace(c) ((c) == ' ' || (c) == '\n' || (c) == '\r' || (c) == '\t')
#define JSON_IsDigit(c) ((c) >= '0' && (c) <= '9')

#define JSON_IsHex(c) (((c) >= '0' && (c) <= '9') || ((c) >= 'A' && (c) <= 'F') || ((c) >= 'a' && (c) <= 'f'))

STR JSON_ReadNextToken(JSON *json)
{
  STR token = { null };
  while (json->position < json->string.size && JSON_IsWhiteSpace(json->string.str[json->position])) ++json->position;
  if (json->position < json->string.size) switch (json->string.str[json->position])
  {
  case '\"':
    token.size = json->position;
    token.str = json->string.str + token.size;
    for (bool escape = true; json->position < json->string.size && (escape || token.str[json->position] != '\"'); ++json->position)
    {
      if (escape) switch (token.str[json->position])
      {
      case 'u':
        if (
          ++json->position < json->string.size &&
          JSON_IsHex(json->string.str[json->position]) &&
          ++json->position < json->string.size &&
          JSON_IsHex(json->string.str[json->position]) &&
          ++json->position < json->string.size &&
          JSON_IsHex(json->string.str[json->position]) &&
          ++json->position < json->string.size &&
          JSON_IsHex(json->string.str[json->position])
        ) break;
      default:
        json->position = token.size;
        token.str = nullptr;
        token.size = 0;
      case '\"':
      case '\\':
      case '/':
      case 'b':
      case 't':
      case 'n':
      case 'f':
      case 'r':
      }
      escape = token.str[json->position] == '\\';
    }
    if (json->position == json->string.size)
    {
      json->position = token.size;
      token.str = nullptr;
      token.size = 0;
    }
    else token.size = ++json->position - token.size;
    break;
  case '{':
  case '}':
  case '[':
  case ']':
  case ',':
  case ':':
    token.str = json->string.str + json->position++;
    token.size = 1;
    break;
  case '-':
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    token.size = json->position;
    token.str = json->string.str + token.size;
    if (token.str[0] == '-' && ++json->position == json->string.size)
    {
      json->position = token.size;
      token.str = nullptr;
      token.size = 0;
      break;
    }
    if (json->string.str[json->position++]) while (json->position < json->string.size && JSON_IsDigit(json->string.str[json->position])) ++json->position;
    if (json->position < json->string.size && json->string.str[json->position] == '.')
    {
      if (++json->position == json->string.size)
      {
        json->position = token.size;
        token.str = nullptr;
        token.size = 0;
        break;
      }
      while (json->position < json->string.size && JSON_IsDigit(json->string.str[json->position])) ++json->position;
    }
    if (json->position < json->string.size && (json->string.str[json->position] & 0xDF) == 'E')
    {
      if (++json->position == json->string.size)
      {
        json->position = token.size;
        token.str = nullptr;
        token.size = 0;
        break;
      }
      if ((json->string.str[json->position] == '+' || json->string.str[json->position] == '-') && ++json->position == json->string.size)
      {
        json->position = token.size;
        token.str = nullptr;
        token.size = 0;
        break;
      }
      while (json->position < json->string.size && JSON_IsDigit(json->string.str[json->position])) ++json->position;
    }
    token.size = json->position - token.size;
    break;
  default:
    token.size = json->position;
    token.str = json->string.str + token.size;
    while (json->position < json->string.size && !JSON_IsWhiteSpace(json->string.str[json->position])) ++json->position;
    token.size = json->position - token.size;
    break;
  }
  return token;
}

bool JSON_ExpectByteToken(JSON *json, U8 byte)
{
  UZ start = json->position;
  STR token = JSON_ReadNextToken(json);
  if (token.size != 1 || token.str[0] != byte)
  {
    json->position = start;
    return false;
  }
  return true;
}

bool JSON_SwitchStateAfterValue(JSON *json)
{
  switch (json->state)
  {
  case JSON_ARRAY_BEGIN:
    json->state = JSON_ARRAY;
    break;
  case JSON_OBJECT_BEGIN:
  case JSON_OBJECT:
    json->state = JSON_OBJECT_VALUE;
    break;
  case JSON_OBJECT_VALUE:
    json->state = JSON_OBJECT;
    break;
  }
  return true;
}

bool JSON_ReadValueBegin(JSON *json)
{
  switch (json->state)
  {
  case JSON_ARRAY:
  case JSON_OBJECT:
    return JSON_ExpectByteToken(json, ',');
  }
  return true;
}

bool JSON_ReadStringValue(JSON *json, MEM *mem, STR *value)
{
  if (JSON_ReadValueBegin(json))
  {
    UZ start = json->position;
    STR token = JSON_ReadNextToken(json);
    if (token.size > 1 && token.str[0] == '\"' && token.str[token.size - 1] == '\"')
    {
      *value = STR_Allocate(mem, token.size - 2);
      if (value->str)
      {
        value->size = 0;
        for (UZ i = 1; i < token.size - 1; ++i)
        {
          if (token.str[i] == '\\') switch (token.str[++i])
          {
          case '\"':
          case '\\':
          case '/':
            value->str[value->size++] = token.str[i];
            break;
          case 'b':
            value->str[value->size++] = '\b';
            break;
          case 't':
            value->str[value->size++] = '\t';
            break;
          case 'n':
            value->str[value->size++] = '\n';
            break;
          case 'f':
            value->str[value->size++] = '\f';
            break;
          case 'r':
            value->str[value->size++] = '\r';
            break;
          case 'u':
            for (U16 utf16[2] = { 0 };;)
            {
              for (U8 j = 4; j; --j)
              {
                U8 byte = token.str[++i];
                utf16[0] = (utf16[0] << 4) | (byte <= '9' ? byte - '0' : (byte <= 'F' ? byte - 'A' : byte - 'a') + 10);
              }
              if ((utf16[0] & 0xF800) == 0xD800)
              {
                if (token.str[++i] != '\\' || token.str[++i] != 'u')
                {
                  MEM_Deallocate(mem, value->str);
                  json->position = start;
                  return false;
                }
                for (U8 j = 4; j; --j)
                {
                  U8 byte = token.str[++i];
                  utf16[1] = (utf16[1] << 4) | (byte <= '9' ? byte - '0' : (byte <= 'F' ? byte - 'A' : byte - 'a') + 10);
                }
              }
              S32 codepoint = UTF16_DecodeFirst((STR16) { utf16, 2 });
              if (codepoint < 0)
              {
                MEM_Deallocate(mem, value->str);
                json->position = start;
                return false;
              }
              value->size += UTF8_Encode(value->str + value->size, 8, codepoint);
              break;
            }
            break;
          }
        }
        return JSON_SwitchStateAfterValue(json);
      }
    }
    json->position = start;
  }
  return false;
}

bool JSON_ReadNumberValue(JSON *json, F64 *value)
{
  if (JSON_ReadValueBegin(json))
  {
    UZ start = json->position;
    STR token = JSON_ReadNextToken(json);
    if (token.size && (JSON_IsDigit(token.str[0]) || token.str[0] == '-'))
    {
      UZ i = 0;
      F64 s = 1;
      *value = 0;
      if (token.str[i] == '-') s = -++i;
      while (i < token.size && JSON_IsDigit(token.str[i]))
      {
        *value = *value * 10 + token.str[i++] - '0';
      }
      if (i < token.size && token.str[i] == '.')
      {
        F64 d = 10;
        while (++i < token.size && JSON_IsDigit(token.str[i]))
        {
          *value = *value + (token.str[i] - '0') / d;
        }
      }
      *value *= s;
      if (i < token.size && (token.str[i] & 0xDF) == 'E')
      {
        F64 e = 0;
        if (++i < token.size)
        {
          s = token.str[i] == '-' ? -1 : 1;
          if (token.str[i] == '+' || token.str[i] == '-') ++i;
        }
        while (i < token.size && JSON_IsDigit(token.str[i]))
        {
          e = e * 10 + token.str[i++] - '0';
        }
        *value *= pow(10, e);
      }
      return JSON_SwitchStateAfterValue(json);
    }
    json->position = start;
  }
  return false;
}

bool JSON_ReadBooleanValue(JSON *json, bool *value)
{
  if (JSON_ReadValueBegin(json))
  {
    UZ start = json->position;
    STR token = JSON_ReadNextToken(json);
    if (STR_Equals(token, STR_Static("true")))
    {
      *value = true;
      return JSON_SwitchStateAfterValue(json);
    }
    if (STR_Equals(token, STR_Static("false")))
    {
      *value = false;
      return JSON_SwitchStateAfterValue(json);
    }
    json->position = start;
  }
  return false;
}

bool JSON_ReadNullValue(JSON *json)
{
  if (JSON_ReadValueBegin(json))
  {
    UZ start = json->position;
    STR token = JSON_ReadNextToken(json);
    if (STR_Equals(token, STR_Static("null")))
    {
      return JSON_SwitchStateAfterValue(json);
    }
    json->position = start;
  }
  return false;
}

JSON_State JSON_ReadArrayBegin(JSON *json)
{
  if (JSON_ReadValueBegin(json) && JSON_ExpectByteToken(json, '[') && JSON_SwitchStateAfterValue(json))
  {
    JSON_State state = json->state;
    json->state = JSON_ARRAY_BEGIN;
    return state;
  }
  return JSON_ERROR;
}

bool JSON_ReadArrayEnd(JSON *json, JSON_State state)
{
  switch (json->state)
  {
  case JSON_ARRAY_BEGIN:
  case JSON_ARRAY:
    if (JSON_ExpectByteToken(json, ']'))
    {
      json->state = state;
      return true;
    }
  }
  return false;
}

JSON_State JSON_ReadObjectBegin(JSON *json)
{
  if (JSON_ReadValueBegin(json) && JSON_ExpectByteToken(json, '{') && JSON_SwitchStateAfterValue(json))
  {
    JSON_State state = json->state;
    json->state = JSON_OBJECT_BEGIN;
    return state;
  }
  return JSON_ERROR;
}

bool JSON_ReadObjectEnd(JSON *json, JSON_State state)
{
  switch (json->state)
  {
  case JSON_OBJECT_BEGIN:
  case JSON_OBJECT:
    if (JSON_ExpectByteToken(json, '}'))
    {
      json->state = state;
      return true;
    }
  }
  return false;
}

bool JSON_ReadObjectKey(JSON *json, MEM *mem, STR *key)
{
  UZ start = json->position;
  JSON_State state = json->state;
  switch (state)
  {
  case JSON_OBJECT_BEGIN:
  case JSON_OBJECT:
    if (JSON_ReadStringValue(json, mem, key))
    {
      if (JSON_ExpectByteToken(json, ':')) return true;
      MEM_Deallocate(mem, key->str);
    }
  }
  json->position = start;
  json->state = state;
  return false;
}

bool JSON_WriteByte(JSON *json, U8 byte)
{
  if (json->position < json->string.size)
  {
    json->string.str[json->position++] = byte;
    return true;
  }
  return false;
}

bool JSON_WriteString(JSON *json, STR string)
{
  bool result = true;
  for (UZ i = 0; result && i < string.size; ++i)
  {
    result = JSON_WriteByte(json, string.str[i]);
  }
  return result;
}

bool JSON_WriteValueBegin(JSON *json)
{
  switch (json->state)
  {
  case JSON_ARRAY:
  case JSON_OBJECT:
    return JSON_WriteByte(json, ',');
  }
  return true;
}

bool JSON_WriteStringValue(JSON *json, STR value)
{
  bool result = JSON_WriteValueBegin(json) && JSON_WriteByte(json, '\"');
  for (UZ i = 0; result && i < value.size; ++i)
  {
    static const U8 hex[] = "0123456789abcdef";
    static const U8 specials[] = "btnvfr";
    U8 byte = value.str[i];
    if (byte == '\"' || byte == '\\')
    {
      result = JSON_WriteByte(json, '\\') && JSON_WriteByte(json, byte);
    }
    else if (byte >= '\b' && byte <= '\r' && byte != '\v')
    {
      result = JSON_WriteByte(json, '\\') && JSON_WriteByte(json, specials[byte - '\b']);
    }
    else if (byte >= ' ' && byte <= '~')
    {
      result = JSON_WriteByte(json, byte);
    }
    else
    {
      S32 codepoint = UTF8_DecodeFirst((STR) { value.str + i, value.size - i });
      if (codepoint >= 0 && codepoint <= 0xFFFF)
      {
        if (codepoint > 0x7F) ++i;
        if (codepoint > 0x7FF) ++i;
        if (codepoint > 0xFFFF)
        {
          ++i;
          U16 utf16[2];
          UTF16_Encode(utf16, 2, codepoint);
          for (U8 j = 4; result && j; --j)
          {
            result = JSON_WriteByte(json, hex[(utf16[0] >> (i << 2)) & 15]);
          }
          for (U8 j = 4; result && j; --j)
          {
            result = JSON_WriteByte(json, hex[(utf16[1] >> (i << 2)) & 15]);
          }
        }
        else for (U8 j = 4; result && j; --j)
        {
          result = JSON_WriteByte(json, hex[(codepoint >> (i << 2)) & 15]);
        }
      }
      else result = false;
    }
  }
  return result && JSON_WriteByte(json, '\"') && JSON_SwitchStateAfterValue(json);
}

bool JSON_WriteInteger(JSON *json, U64 value)
{
  bool result = true;
  UZ start = json->position;
  do (result = JSON_WriteByte(json, value % 10 + '0')), value /= 10; while (result && value);
  for (UZ i = 0; i < ((json->position - result) >> 1); ++i)
  {
    U8 byte = json->string.str[start + i];
    json->string.str[start + i] = json->string.str[json->position - i - 1];
  }
  return result && JSON_SwitchStateAfterValue(json);
}

bool JSON_WriteNumberValue(JSON *json, F64 value, U32 precision)
{
  if (!JSON_WriteValueBegin(json)) return false;
  if (value < 0)
  {
    if (!JSON_WriteByte(json, '-')) return false;
    value = -value;
  }
  if (!JSON_WriteInteger(json, (U64)value)) return false;
  if (precision)
  {
    if (!JSON_WriteByte(json, '.')) return false;
    value -= (U64)value;
    value *= pow(10, precision);
    if (!JSON_WriteInteger(json, (U64)value)) return false;
  }
  return JSON_SwitchStateAfterValue(json);
}

bool JSON_WriteBooleanValue(JSON *json, bool value)
{
  return JSON_WriteValueBegin(json)
    && JSON_WriteString(json, value ? STR_Static("true") : STR_Static("false"));
}

bool JSON_WriteNullValue(JSON *json)
{
  return JSON_WriteValueBegin(json)
    && JSON_WriteString(json, STR_Static("null"));
}

JSON_State JSON_WriteArrayBegin(JSON *json)
{
  if (JSON_WriteValueBegin(json) && JSON_WriteByte(json, '[') && JSON_SwitchStateAfterValue(json))
  {
    JSON_State state = json->state;
    json->state = JSON_ARRAY_BEGIN;
    return state;
  }
  return JSON_ERROR;
}

bool JSON_WriteArrayEnd(JSON *json, JSON_State state)
{
  switch (json->state)
  {
  case JSON_ARRAY_BEGIN:
  case JSON_ARRAY:
    if (JSON_WriteByte(json, ']'))
    {
      json->state = state;
      return true;
    }
  }
  return false;
}

JSON_State JSON_WriteObjectBegin(JSON *json)
{
  if (JSON_WriteValueBegin(json) && JSON_WriteByte(json, '{') && JSON_SwitchStateAfterValue(json))
  {
    JSON_State state = json->state;
    json->state = JSON_OBJECT_BEGIN;
    return state;
  }
  return JSON_ERROR;
}

bool JSON_WriteObjectEnd(JSON *json, JSON_State state)
{
  switch (json->state)
  {
  case JSON_OBJECT_BEGIN:
  case JSON_OBJECT:
    if (JSON_WriteByte(json, '}'))
    {
      json->state = state;
      return true;
    }
  }
  return false;
}

bool JSON_WriteObjectKey(JSON *json, STR key)
{
  switch (json->state)
  {
  case JSON_OBJECT_BEGIN:
  case JSON_OBJECT:
    return JSON_WriteStringValue(json, key) && JSON_WriteByte(json, ':');
  }
  return false;
}
