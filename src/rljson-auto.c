#include "rljson-auto.h"

void *json_auto_parse_string(void **user, Json_Parse_Value key, Json_Parse_Value *val) {
    Json_Auto_Value *autoval = *(Json_Auto_Value **)user;
    autoval->so = key.s;
    autoval->id = JSON_AUTO_VALUE_STRING;
    return 0;
}

void *json_auto_parse_bool(void **user, Json_Parse_Value key, Json_Parse_Value *val) {
    Json_Auto_Value *autoval = *(Json_Auto_Value **)user;
    autoval->b = key.b;
    autoval->id = JSON_AUTO_VALUE_BOOL;
    return 0;
}

void *json_auto_parse_null(void **user, Json_Parse_Value key, Json_Parse_Value *val) {
    Json_Auto_Value *autoval = *(Json_Auto_Value **)user;
    autoval->id = JSON_AUTO_VALUE_NULL;
    return 0;
}

void *json_auto_parse_number(void **user, Json_Parse_Value key, Json_Parse_Value *val) {
    Json_Auto_Value *autoval = *(Json_Auto_Value **)user;
    double d = 0;
    size_t z = 0;
    if(!so_as_size(key.s, &z, 0)) {
        autoval->z = z;
        autoval->id = JSON_AUTO_VALUE_SIZE;
    } else if(!so_as_double(key.s, &d)) {
        autoval->f = d;
        autoval->id = JSON_AUTO_VALUE_DOUBLE;
    } else {
        autoval->so = key.s;
        autoval->id = JSON_AUTO_VALUE_STRING;
    }
    return 0;
}

void *json_auto_parse_value(void **user, Json_Parse_Value key, Json_Parse_Value *val);

void *json_auto_parse_array(void **user, Json_Parse_Value key, Json_Parse_Value *val) {
    Json_Auto_Value *autoval = *(Json_Auto_Value **)user;
    autoval->id = JSON_AUTO_VALUE_ARRAY;
    array_push(autoval->arr, (Json_Auto_Value){0});
    Json_Auto_Value *subuser = array_itE(autoval->arr);
    if(val) {
        return json_auto_parse_value((void **)&subuser, *val, 0);
    } else {
        *user = subuser;
        return json_auto_parse_value;
    }
}

void *json_auto_parse_object(void **user, Json_Parse_Value key, Json_Parse_Value *val) {
    Json_Auto_Value *autoval = *(Json_Auto_Value **)user;
    autoval->id = JSON_AUTO_VALUE_OBJECT;
    array_push(autoval->dict, (Json_Auto_Key_Value){0});
    Json_Auto_Key_Value *subkv = array_itE(autoval->dict);
    subkv->key = key.s;
    Json_Auto_Value *subuser = &subkv->val;
    if(val) {
        return json_auto_parse_value((void **)&subuser, *val, 0);
    } else {
        *user = subuser;
        return json_auto_parse_value;
    }
}

void *json_auto_parse_value(void **user, Json_Parse_Value key, Json_Parse_Value *val) {
    Json_Auto_Value *autoval = *(Json_Auto_Value **)user;
    switch(key.id) {
        case JSON_NUMBER: json_auto_parse_number(user, key, val); break;
        case JSON_STRING: json_auto_parse_string(user, key, val); break;
        case JSON_BOOL: json_auto_parse_bool(user, key, val); break;
        case JSON_NULL: json_auto_parse_null(user, key, val); break;
        case JSON_ARRAY: return json_auto_parse_array(user, key, val);
        case JSON_OBJECT: return json_auto_parse_object(user, key, val);
        default: break;
    }
    return 0;
}

ErrDecl json_auto_parse(So input, Json_Auto_Value *out) {
    ASSERT_ARG(out);
    json_parse(input, json_auto_parse_value, out);
    return 0;
}

void json_auto_fmt_spacing(So *out, Json_Auto_Fmt *fmt, int nest) {
    if(!fmt->pretty) return;
    if(fmt->tabs) {
        for(int i = 0; i < nest; ++i) {
            for(int j = 0; j < fmt->tabs; ++j) {
                so_push(out, '\t');
            }
        }
    } else {
        so_fmt(out, "%*s", fmt->spaces * nest, "");
    }
}

void json_auto_print_spacing(Json_Auto_Fmt *fmt, int nest) {
    if(!fmt->pretty) return;
    if(fmt->tabs) {
        for(int i = 0; i < nest; ++i) {
            for(int j = 0; j < fmt->tabs; ++j) {
                printf("\t");
            }
        }
    } else {
        printf("%*s", fmt->spaces * nest, "");
    }
}

void json_auto_print_ext(Json_Auto_Value autojson, Json_Auto_Fmt *fmt, int nest) {
    switch(autojson.id) {
        case JSON_AUTO_VALUE_ARRAY: {
            printf("[");
            if(fmt->pretty) printf("\n");
            for(size_t i = 0; i < array_len(autojson.arr); ++i) {
                Json_Auto_Value sub = array_at(autojson.arr, i);
                if(i) {
                    printf(",");
                    if(fmt->pretty) printf("\n");
                }
                json_auto_print_spacing(fmt, nest + 1);
                json_auto_print_ext(sub, fmt, nest + 1);
            }
            if(fmt->pretty) printf("\n");
            json_auto_print_spacing(fmt, nest);
            printf("]");
        } break;
        case JSON_AUTO_VALUE_BOOL: {
            printf("%s", autojson.b ? "true" : "false");
        } break;
        case JSON_AUTO_VALUE_DOUBLE: {
            printf("%f", autojson.f);
        } break;
        case JSON_AUTO_VALUE_NULL: {
            printf("null");
        } break;
        case JSON_AUTO_VALUE_OBJECT: {
            printf("{");
            if(fmt->pretty) printf("\n");
            for(size_t i = 0; i < array_len(autojson.dict); ++i) {
                Json_Auto_Key_Value sub = array_at(autojson.dict, i);
                if(i) {
                    printf(",");
                    if(fmt->pretty) printf("\n");
                }
                json_auto_print_spacing(fmt, nest + 1);
                printf("\"%.*s\":", SO_F(sub.key)); // TODO need to escape "
                if(fmt->pretty) printf(" ");
                json_auto_print_ext(sub.val, fmt, nest + 1);
            }
            if(fmt->pretty) printf("\n");
            json_auto_print_spacing(fmt, nest);
            printf("}");
        } break;
        case JSON_AUTO_VALUE_SIZE: {
            printf("%zu", autojson.z);
        } break;
        case JSON_AUTO_VALUE_STRING: {
            printf("\"%.*s\"", SO_F(autojson.so)); // TODO need to escape "
        } break;
        default: ABORT(ERR_UNREACHABLE("invalid switch: %u"), autojson.id);
    }
}

void json_auto_fmt_ext(So *out, Json_Auto_Value autojson, Json_Auto_Fmt *fmt, int nest) {
    switch(autojson.id) {
        case JSON_AUTO_VALUE_ARRAY: {
            so_push(out, '[');
            if(fmt->pretty) so_push(out, '\n');
            for(size_t i = 0; i < array_len(autojson.arr); ++i) {
                Json_Auto_Value sub = array_at(autojson.arr, i);
                if(i) {
                    so_push(out, ',');
                    if(fmt->pretty) so_push(out, '\n');
                }
                json_auto_fmt_spacing(out, fmt, nest + 1);
                json_auto_fmt_ext(out, sub, fmt, nest + 1);
            }
            if(fmt->pretty) so_push(out, '\n');
            json_auto_fmt_spacing(out, fmt, nest);
            so_push(out, ']');
        } break;
        case JSON_AUTO_VALUE_BOOL: {
            so_fmt(out, "%s", autojson.b ? "true" : "false");
        } break;
        case JSON_AUTO_VALUE_DOUBLE: {
            so_fmt(out, "%f", autojson.f);
        } break;
        case JSON_AUTO_VALUE_NULL: {
            so_fmt(out, "null");
        } break;
        case JSON_AUTO_VALUE_OBJECT: {
            so_push(out, '{');
            if(fmt->pretty) so_push(out, '\n');
            for(size_t i = 0; i < array_len(autojson.dict); ++i) {
                Json_Auto_Key_Value sub = array_at(autojson.dict, i);
                if(i) {
                    so_push(out, ',');
                    if(fmt->pretty) so_push(out, '\n');
                }
                json_auto_fmt_spacing(out, fmt, nest + 1);
                so_fmt(out, "\"%.*s\":", SO_F(sub.key)); // TODO need to escape "
                if(fmt->pretty) so_push(out, ' ');
                json_auto_fmt_ext(out, sub.val, fmt, nest + 1);
            }
            if(fmt->pretty) so_push(out, '\n');
            json_auto_fmt_spacing(out, fmt, nest);
            so_push(out, '}');
        } break;
        case JSON_AUTO_VALUE_SIZE: {
            so_fmt(out, "%zu", autojson.z);
        } break;
        case JSON_AUTO_VALUE_STRING: {
            so_fmt(out, "\"%.*s\"", SO_F(autojson.so)); // TODO need to escape "
        } break;
        default: ABORT(ERR_UNREACHABLE("invalid switch: %u"), autojson.id);
    }
}

void json_auto_print(Json_Auto_Value autojson, Json_Auto_Fmt *fmt) {
     Json_Auto_Fmt d = (Json_Auto_Fmt){
        .spaces = 2,
        .pretty = true,
    };
     if(!fmt) fmt = &d;
    json_auto_print_ext(autojson, fmt, 0);
    if(fmt->pretty) {
        printf("\n");
    }
}

void json_auto_fmt(So *out, Json_Auto_Value autojson, Json_Auto_Fmt *fmt) {
     Json_Auto_Fmt d = (Json_Auto_Fmt){
        .spaces = 2,
        .pretty = true,
    };
     if(!fmt) fmt = &d;
    json_auto_fmt_ext(out, autojson, fmt, 0);
    if(fmt->pretty) {
        so_push(out, '\n');
    }
}

void json_auto_free_kv(Json_Auto_Key_Value *autojson) {
    so_free(&autojson->key);
    json_auto_free(&autojson->val);
}

void json_auto_free(Json_Auto_Value *autojson) {
    if(!autojson) return;
    switch(autojson->id) {
        case JSON_AUTO_VALUE_ARRAY: {
            array_free_set(autojson->arr, json_auto_free);
            array_free(autojson->arr);
        } break;
        case JSON_AUTO_VALUE_OBJECT: {
            array_free_set(autojson->dict, json_auto_free_kv);
            array_free(autojson->dict);
        } break;
        case JSON_AUTO_VALUE_STRING: {
            so_free(&autojson->so);
        } break;
        /* never ever have to free anything */
        case JSON_AUTO_VALUE_DOUBLE:
        case JSON_AUTO_VALUE_BOOL:
        case JSON_AUTO_VALUE_NULL:
        case JSON_AUTO_VALUE_SIZE: break;
        default: ABORT(ERR_UNREACHABLE("invalid switch: %u"), autojson->id);
    }
    memset(autojson, 0, sizeof(*autojson));
}

