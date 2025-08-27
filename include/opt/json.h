#ifndef OPT_JSON_H
#define OPT_JSON_H

#include <str.h>

typedef enum JSON_State
{
  JSON_ERROR,
  JSON_ROOT,
  JSON_ARRAY_BEGIN,
  JSON_ARRAY,
  JSON_OBJECT_BEGIN,
  JSON_OBJECT,
  JSON_OBJECT_VALUE,
} JSON_State;

typedef struct JSON
{
  STR string;
  UZ position;
  JSON_State state;
} JSON;

#define JSON_Init(string) ((JSON) { (string), 0, JSON_ROOT })

#define JSON_Line Glue(__json_, __LINE__)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                  JSON READ                                   *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool JSON_ReadStringValue(JSON *json, MEM *mem, STR *value);

bool JSON_ReadNumberValue(JSON *json, F64 *value);

bool JSON_ReadBooleanValue(JSON *json, bool *value);

bool JSON_ReadNullValue(JSON *json);

JSON_State JSON_ReadArrayBegin(JSON *json);
bool JSON_ReadArrayEnd(JSON *json, JSON_State state);

#define JSON_ReadArray(json, success) for (         \
  JSON_State JSON_Line = JSON_ReadArrayBegin(json); \
  (success) = false,                                \
  JSON_Line != JSON_ERROR;                          \
  (success) = JSON_ReadArrayEnd(json, JSON_Line),   \
  JSON_Line = JSON_ERROR                            \
)

JSON_State JSON_ReadObjectBegin(JSON *json);
bool JSON_ReadObjectEnd(JSON *json, JSON_State state);

#define JSON_ReadObject(json, success) for (          \
  JSON_State JSON_Line = JSON_ReadObjectBegin(json);  \
  (success) = false,                                  \
  JSON_Line != JSON_ERROR;                            \
  (success) = JSON_ReadObjectEnd(json, JSON_Line),    \
  JSON_Line = JSON_ERROR                              \
)

bool JSON_ReadObjectKey(JSON *json, MEM *mem, STR *key);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                  JSON WRITE                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool JSON_WriteStringValue(JSON *json, STR value);

bool JSON_WriteNumberValue(JSON *json, F64 value, U32 precision);

bool JSON_WriteBooleanValue(JSON *json, bool value);

bool JSON_WriteNullValue(JSON *json);

JSON_State JSON_WriteArrayBegin(JSON *json);
bool JSON_WriteArrayEnd(JSON *json, JSON_State state);

#define JSON_WriteArray(json, success) for (          \
  JSON_State JSON_Line = JSON_WriteArrayBegin(json);  \
  (success) = false,                                  \
  JSON_Line != JSON_ERROR;                            \
  (success) = JSON_WriteArrayEnd(json, JSON_Line),    \
  JSON_Line = JSON_ERROR                              \
)

JSON_State JSON_WriteObjectBegin(JSON *json);
bool JSON_WriteObjectEnd(JSON *json, JSON_State state);

#define JSON_WriteObject(json, success) for (         \
  JSON_State JSON_Line = JSON_WriteObjectBegin(json); \
  (success) = false,                                  \
  JSON_Line != JSON_ERROR;                            \
  (success) = JSON_WriteObjectEnd(json, JSON_Line),   \
  JSON_Line = JSON_ERROR                              \
)

bool JSON_WriteObjectKey(JSON *json, STR key);

#endif
