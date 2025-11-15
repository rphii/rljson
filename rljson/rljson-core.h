#ifndef RLJSON_CORE_H

#include <rlc/err.h>
#include <rlso.h>

#define JSON_DEPTH_MAX  4096

typedef enum {
    //JSON_NONE,
    /* keep below */
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOL,
    JSON_NULL,
} Json_List;

typedef struct Json_Parse_Value {
    union {
        So s;
        bool b;
    };
    Json_List id;
} Json_Parse_Value;

typedef struct Json_Parse_Settings {
    bool verbose;
} Json_Parse_Settings;

typedef void *(*Json_Parse_Callback)(void **user, Json_Parse_Value key, Json_Parse_Value *val);

typedef struct Json_Parse {
    So head;
    Json_Parse_Value key;
    size_t depth;
    Json_Parse_Callback callback;
    Json_Parse_Settings settings;
    void *user;
} Json_Parse;

So json_parse_value_str(Json_Parse_Value v);

ErrDecl json_parse_valid(So input);
ErrDecl json_parse_valid_ext(So input, Json_Parse *parse);

#define ERR_json_parse(...) "failed parsing json (invalid input)"
ErrDecl json_parse(So input, Json_Parse_Callback callback, void *user);
ErrDecl json_parse_ext(So input, Json_Parse_Callback callback, void *user, Json_Parse *parse);

void json_fix_so(So *out, So json_str); /* modifies the existing string; no additional memory allocation */
void json_parse_value_print(Json_Parse_Value *val);

#define RLJSON_CORE_H
#endif

