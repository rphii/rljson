#include <stdlib.h>
#include <stdbool.h>
#include <rphii/so-uc.h>
#include <rphii/so-as.h>
#include "json.h"

bool json_parse_value(JsonParse *p, JsonParseValue *v);

So json_parse_value_str(JsonParseValue v) {
    switch(v.id) {
        case JSON_STRING:
        case JSON_OBJECT: //return so("OBJECT");
        case JSON_NUMBER: return v.s;
        case JSON_BOOL: return v.b ? so("true") : so("false");
        case JSON_ARRAY: return so("ARRAY");
        case JSON_NULL: return so("null");
    }
    return so("(null)");
}

/* return true on match */
bool json_parse_ch(JsonParse *p, char c) {
    ASSERT_ARG(p);
    if(!p->head.len) return false;
    bool result = (bool)(*p->head.str == c);
    if(result) {
        ++p->head.str;
        --p->head.len;
    }
    return result;
}

/* return true on match */
bool json_parse_any(JsonParse *p, char *s) {
    ASSERT_ARG(p);
    if(!p->head.len) return false;
    char *result = strchr(s, *p->head.str);
    if(result && *result) {
        ++p->head.str;
        --p->head.len;
        return true;
    }
    return false;
}

void json_parse_ws(JsonParse *p) {
    ASSERT_ARG(p);
    while(json_parse_any(p, " \t\v\n")) {}
}

/* return true on successful parse */
bool json_parse_string(JsonParse *p, So *val) {
    ASSERT_ARG(p);
    ASSERT_ARG(val);
    JsonParse q = *p;
    if(!json_parse_ch(&q, '"')) goto invalid;
    int escape = 0;
    while(q.head.len) {
        if(escape < 0) {
            escape = 0;
            switch(*q.head.str) {
                case '"' : break;
                case '\\': break;
                case '/' : break;
                case 'b' : break;
                case 'f' : break;
                case 'n' : break;
                case 'r' : break;
                case 't' : break;
                case 'u' : escape += 4; break;
                default  : goto invalid;
            }
            ++q.head.str;
            --q.head.len;
        } else if(escape) {
            if(!json_parse_any(&q, "0123456789abcdefABCDEF")) goto invalid;
            --escape;
        } else if(json_parse_ch(&q, '\\')) {
            escape = -1;
        } else if(json_parse_ch(&q, '"')) {
            *val = so_ll(p->head.str + 1, q.head.str - p->head.str - 2);
            p->head = q.head;
            if(p->key.id != JSON_ARRAY && p->key.id != JSON_OBJECT) {
                if(p->settings.verbose) printf("%*s[string] '%.*s' : '%.*s'\n", (int)p->depth, "", SO_F(json_parse_value_str(p->key)), SO_F(*val));
            }
            return true;
        } else if(*q.head.str >= ' ') {
            ++q.head.str;
            --q.head.len;
        } else {
            So_Uc_Point u8p = {0};
            if(so_uc_point(so_ll(q.head.str, q.head.len), &u8p)) {
                goto invalid;
            }
            q.head.str += u8p.bytes;
            q.head.len -= u8p.bytes;
        }
    }
invalid:
    if(p->settings.verbose) {
        printf("%*s[invalid string] %.*s", (int)p->depth, "", (int)p->head.len, p->head.str);
    }
    return false;
}

bool json_parse_bool(JsonParse *p, bool *val) {
    ASSERT_ARG(p);
    ASSERT_ARG(val);
    JsonParse q = *p;
    if(json_parse_ch(&q, 't')) {
        if(!json_parse_ch(&q, 'r')) return false;
        if(!json_parse_ch(&q, 'u')) return false;
        if(!json_parse_ch(&q, 'e')) return false;
        *val = true;
        if(p->key.id != JSON_ARRAY && p->key.id != JSON_OBJECT) {
            if(p->settings.verbose) printf("%*s[bool] '%.*s' : true\n", (int)p->depth, "", SO_F(json_parse_value_str(p->key)));
        }
        p->head = q.head;
        return true;
    }
    if(json_parse_ch(&q, 'f')) {
        if(!json_parse_ch(&q, 'a')) return false;
        if(!json_parse_ch(&q, 'l')) return false;
        if(!json_parse_ch(&q, 's')) return false;
        if(!json_parse_ch(&q, 'e')) return false;
        *val = false;
        if(p->key.id != JSON_ARRAY && p->key.id != JSON_OBJECT) {
            if(p->settings.verbose) printf("%*s[bool] '%.*s' : false\n", (int)p->depth, "", SO_F(json_parse_value_str(p->key)));
        }
        p->head = q.head;
        return true;
    }
    return false;
}


#define JSON_DIGITS  "0123456789"
#define JSON_DIGIT1  "123456789"

bool json_parse_number(JsonParse *p, So *val) {
    ASSERT_ARG(p);
    ASSERT_ARG(val);
    JsonParse q = *p;
    json_parse_ch(&q, '-');
    if(json_parse_ch(&q, '0')) {
    } else if(json_parse_any(&q, JSON_DIGIT1)) {
        while(json_parse_any(&q, JSON_DIGITS)) {}
    } else {
        return false;
    }
    if(json_parse_ch(&q, '.')) {
        if(!json_parse_any(&q, JSON_DIGITS)) return false;
        while(json_parse_any(&q, JSON_DIGITS)) {}
    }
    if(json_parse_any(&q, "eE")) {
        json_parse_any(&q, "+-");
        if(!json_parse_any(&q, JSON_DIGITS)) return false;
        while(json_parse_any(&q, JSON_DIGITS)) {}
    }
    So result = so_ll(p->head.str, q.head.str - p->head.str);
    size_t len = so_len(result);
    if(len) {
        *val = result;
        if(p->key.id != JSON_ARRAY && p->key.id != JSON_OBJECT) {
            if(p->settings.verbose) printf("%*s[number] '%.*s' : '%.*s'\n", (int)p->depth, "", SO_F(json_parse_value_str(p->key)), SO_F(*val));
        }
        p->head = q.head;
    }
    return (bool)len;
}

bool json_parse_null(JsonParse *p) {
    ASSERT_ARG(p);
    JsonParse q = *p;
    if(json_parse_ch(&q, 'n')) {
        if(!json_parse_ch(&q, 'u')) return false;
        if(!json_parse_ch(&q, 'l')) return false;
        if(!json_parse_ch(&q, 'l')) return false;
        p->head = q.head;
        if(p->key.id != JSON_ARRAY && p->key.id != JSON_OBJECT) {
            if(p->settings.verbose) printf("%*s[null] '%.*s'\n", (int)p->depth, "", SO_F(json_parse_value_str(p->key)));
        }
        return true;
    }
    return false;
}

bool json_parse_array(JsonParse *p) {
    ASSERT_ARG(p);
    JsonParseValue v = {0};
    JsonParse q = *p;
    if(!json_parse_ch(&q, '[')) return false;
    q.key.id = JSON_ARRAY;
    ++q.depth;
    if(q.depth >= JSON_DEPTH_MAX) return false;
    if(p->depth) {
        if(p->settings.verbose) printf("%*s[array enter -> '%.*s']\n", (int)p->depth, "", SO_F(json_parse_value_str(p->key)));
        if(p->callback) q.callback = p->callback(&q.user, p->key, 0);
    }
    json_parse_ws(&q);
    bool first = true;
    do {
        if(!json_parse_value(&q, &v)) {
            if(first) break;
            else return false;
        }
        if(q.callback && v.id != JSON_OBJECT && v.id != JSON_ARRAY) {
            if(p->settings.verbose) printf("%*s[array] '%.*s' <- '%.*s'\n", (int)q.depth, "", SO_F(json_parse_value_str(v)), SO_F(json_parse_value_str(p->key)));
            void *user = q.user; //p->user;
            q.callback(&user, q.key, &v);
        }
        if(!json_parse_ch(&q, ',')) break;
        first = false;
    } while(q.head.len);
    if(!json_parse_ch(&q, ']')) return false;
    p->head = q.head;
    if(p->depth) {
        if(p->settings.verbose) printf("%*s[array exit <- '%.*s']\n", (int)p->depth, "", SO_F(json_parse_value_str(p->key)));
    }
    return true;
}

bool json_parse_object(JsonParse *p) {
    ASSERT_ARG(p);
    JsonParse q = *p;
    if(!json_parse_ch(&q, '{')) return false;
    q.key.id = JSON_OBJECT;
    ++q.depth;
    if(q.depth >= JSON_DEPTH_MAX) return false;
    if(p->depth) {
        if(p->settings.verbose) printf("%*s[object enter -> '%.*s']\n", (int)p->depth, "", SO_F(json_parse_value_str(p->key)));
        if(p->callback) q.callback = p->callback(&q.user, p->key, 0);
    }
    json_parse_ws(&q);
    JsonParseValue k = { .id = JSON_OBJECT };
    JsonParseValue v = {0};
    bool first = true;
    do {
        json_parse_ws(&q);
        if(!json_parse_string(&q, &k.s)) {
            if(first) break;
            else return false;
        }
        json_parse_ws(&q);
        if(!json_parse_ch(&q, ':')) return false;
        q.key = k;
        if(!json_parse_value(&q, &v)) return false;
        /* key-value pair found */
        if(v.id != JSON_OBJECT && v.id != JSON_ARRAY) {
            if(p->settings.verbose) printf("%*s[object] '%.*s' : '%.*s' %u <- '%.*s'\n", (int)q.depth, "", SO_F(json_parse_value_str(k)), SO_F(json_parse_value_str(v)), v.id, SO_F(json_parse_value_str(p->key)));
            //q.user = p->user;
            void *user = q.user;
            if(q.callback) q.callback(&user, q.key, &v);
        }
        /* next */
        if(!json_parse_ch(&q, ',')) break;
        first = false;
    } while(q.head.len);
    if(!json_parse_ch(&q, '}')) return false;
    p->head = q.head;
    if(p->depth) {
        if(p->settings.verbose) printf("%*s[object exit <- '%.*s']\n", (int)p->depth, "", SO_F(json_parse_value_str(p->key)));
    }
    return true;
}

bool json_parse_value(JsonParse *p, JsonParseValue *v) {
    JsonParse q = *p;
    json_parse_ws(&q);
    if(json_parse_object(&q)) { v->id = JSON_OBJECT; goto valid; }
    if(json_parse_array(&q)) { v->id = JSON_ARRAY; goto valid; }
    /* atomic */
    if(json_parse_string(&q, &v->s))  { v->id = JSON_STRING; goto valid; }
    if(json_parse_number(&q, &v->s)) { v->id = JSON_NUMBER; goto valid; }
    if(json_parse_bool(&q, &v->b)) { v->id = JSON_BOOL; goto valid; }
    if(json_parse_null(&q)) { v->id = JSON_NULL; goto valid; }
    return false;
valid:
    json_parse_ws(&q);
    p->head = q.head;
    return true;
}

ErrDecl json_parse_valid(So input) {
    JsonParseValue v = {0};
    JsonParse q = {
        .head = so_ref(input),
        .settings.verbose = false,
    };
    if(json_parse_value(&q, &v)) goto valid;
    return -1;
valid:
    json_parse_ws(&q);
    return q.head.len;
}

ErrDecl json_parse(So input, JsonParseCallback callback, void *user) {
    if(json_parse_valid(input)) goto invalid;
    JsonParseValue v = {0};
    JsonParse q = {
        .head = so_ref(input),
        .callback = callback,
        .user = user,
        .settings.verbose = false,
    };
    json_parse_ws(&q);
    if(json_parse_value(&q, &v)) goto valid;
invalid:
    return -1;
valid:
    json_parse_ws(&q);
    return q.head.len;
}

void json_fix_so(So *out, So json_str) {
    So_Ref ref = so_ref(json_str);
    int escape = 0;
    size_t begin = 0;
    size_t j = 0;
    for(size_t i = 0; i < ref.len; ++i) {
        char c = ref.str[i];
        if(!escape && c == '\\') {
            escape = -1;
        } else if(escape < 0) {
            escape = 0;
            switch(c) {
                case 'b':  *so_it(json_str, j++) = '\b'; break;
                case '"':  *so_it(json_str, j++) = '\"'; break;
                case '\'': *so_it(json_str, j++) = '\''; break;
                case '/':  *so_it(json_str, j++) = '/';  break;
                case 'f':  *so_it(json_str, j++) = '\f'; break;
                case 'n':  *so_it(json_str, j++) = '\n'; break;
                case 'r':  *so_it(json_str, j++) = '\r'; break;
                case 't':  *so_it(json_str, j++) = '\t'; break;
                case 'u': escape = 4; break;
                default: break; /* TODO what do ? should never reach this */
            }
        } else if(escape > 0) {
            --escape;
            if(escape == 3) begin = i;
            if(escape == 0) {
                ASSERT(4 == i - begin + 1, "expect to have 4 characters...");
                So num = so_ll(so_it(json_str, begin), 4);
                size_t z = 0;
                if(!so_as_size(num, &z, 16)) {
                    So_Uc_Point u8p = { .val = z };
                    So u8 = {0};
                    if(!so_uc_fmt_point(&u8, &u8p)) {
                        So_Ref p = so_ref(u8);
                        ASSERT(p.len <= 3, "expect to have 3 or less characters (0xFFFF max)...");
                        for(size_t k = 0; k < p.len; ++k) {
                            *so_it(json_str, j++) = p.str[k];
                        }
                    }
                }
            }
        } else {
            *so_it(json_str, j++) = c;
        }
    }
    ASSERT(j < so_len(json_str), "we only want to shrink the string!");
    so_resize(&json_str, j);
    *out = json_str;
}

