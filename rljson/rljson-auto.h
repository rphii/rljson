#ifndef RLJSON_AUTO_H

#include "rljson-core.h"

typedef enum {
    JSON_AUTO_VALUE_NULL,
    JSON_AUTO_VALUE_BOOL,
    JSON_AUTO_VALUE_SIZE,
    JSON_AUTO_VALUE_DOUBLE,
    JSON_AUTO_VALUE_STRING,
    JSON_AUTO_VALUE_OBJECT,
    JSON_AUTO_VALUE_ARRAY,
} Json_Auto_Value_List ;

typedef struct Json_Auto_Value {
    union {
        bool b;
        size_t z;
        double f;
        So so;
        struct Json_Auto_Value *arr;
        struct Json_Auto_Key_Value *dict;
    };
    Json_Auto_Value_List id;
} Json_Auto_Value;

typedef struct Json_Auto_Key_Value {
    So key;
    Json_Auto_Value val;
} Json_Auto_Key_Value;

typedef struct Json_Auto_Fmt {
    bool pretty;
    int spaces;
    int tabs;
} Json_Auto_Fmt;

ErrDecl json_auto_parse(So input, Json_Auto_Value *out);
ErrDecl json_auto_parse_ext(So input, Json_Auto_Value *out, Json_Parse_Settings *settings);
void json_auto_print(Json_Auto_Value autojson, Json_Auto_Fmt *fmt);
void json_auto_fmt(So *out, Json_Auto_Value autojson, Json_Auto_Fmt *fmt);
void json_auto_free(Json_Auto_Value *autojson);

#define RLJSON_AUTO_H
#endif // RLJSON_AUTO_H

