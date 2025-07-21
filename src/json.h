#ifndef RPHIIC_JSON_H

#include <rphii/err.h>
#include <rphii/so.h>

#define JSON_DEPTH_MAX  4096

typedef struct JsonParse JsonParse;

typedef enum {
    //JSON_NONE,
    /* keep below */
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOL,
    JSON_NULL,
} JsonList;

typedef struct JsonParseValue {
    union {
        So s;
        bool b;
    };
    JsonList id;
} JsonParseValue;

typedef struct JsonParseSettings {
    bool verbose;
} JsonParseSettings;

typedef void *(*JsonParseCallback)(void **user, JsonParseValue key, JsonParseValue *val);

typedef struct JsonParse {
    So_Ref head;
    JsonParseValue key;
    size_t depth;
    JsonParseCallback callback;
    JsonParseSettings settings;
    void *user;
} JsonParse;

So json_parse_value_str(JsonParseValue v);

ErrDecl json_parse_valid(So input);

#define ERR_json_parse(...) "failed parsing json (invalid input)"
ErrDecl json_parse(So input, JsonParseCallback callback, void *user);

void json_fix_so(So *out, So json_str); /* modifies the existing string; no additional memory allocation */

#define RPHIIC_JSON_H
#endif

