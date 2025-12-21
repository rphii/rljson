#include "../rljson/rljson-auto.h"

int main(int argc, char **argv) {
    if(argc <= 1) ABORT("not given expected test result: 'fail' or 'pass'");
    if(argc <= 2) ABORT("no filename given to test");
    if(argc <= 3) ABORT("not given mode: 'strict' or 'non-strict'");

    int status = 0;
    bool expected = 0;
    So filename = so_l(argv[2]);
    Json_Parse_Settings settings = {0};
    So choice = so_l(argv[1]);
    /* parse fail/pass */
    if(!so_cmp(choice, so("fail"))) {
        expected = 1;
    } else if(!so_cmp(choice, so("pass"))) {
        expected = 0;
    } else {
        ABORT("invalid test result: '%s' (expect 'fail' or 'pass')", argv[2]);
    }
    /* parse strict/non-strict */
    So mode = so_l(argv[3]);
    if(!so_cmp(mode, so("strict"))) {
        settings.strict = true;
    } else if(!so_cmp(mode, so("non-strict"))) {
        settings.strict = false;
    } else {
        ABORT("invalid test result: '%s' (expect 'strict' or 'non-strict')", argv[2]);
    }

    So content = SO;
    if(so_file_read(filename, &content)) ABORT("failed reading file: '%.*s'", SO_F(filename));

    Json_Auto_Value json;
    //bool result = json_parse_valid(content);
    bool result = json_auto_parse_ext(content, &json, &settings);
    if(result != expected) {
        printff(F("INVALID %s!", FG_RD_B) " '%.*s'", result ? "FAIL" : "PASS", SO_F(filename));
        status = 1;
    } else {
        printff(F("SUCCESS %s!", FG_GN_B) " '%.*s'", result ? "FAIL" : "PASS", SO_F(filename));
    }

#define DEBUG_OUTPUT 1
#if DEBUG_OUTPUT
    printf("=vv-content-vv=============================================\n");
    so_println(content);
    printf("=vv-parsed-json-vv=========================================\n");
    json_auto_print(json, 0);
    printf("===========================================================\n");
#endif

    json_auto_free(&json);
    so_free(&content);

    return status;
}

