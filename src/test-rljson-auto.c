#include "rljson-auto.h"
#include <unistd.h>

int main(int argc, char **argv) {
    Json_Auto_Value out = {0};
    So content = SO;
    So json = SO;
    Json_Parse jp = {0};
    for(int i = 1; i < argc; ++i) {
        so_clear(&json);
        so_clear(&content);
        so_file_read(so_l(argv[i]), &content);
        //if(json_parse_valid_ext(content, &jp)) {
        //    printf("invalid json (from '%.*s' @ %zu, string length: %zu)\n", SO_F(so_l(argv[i])), so_it0(jp.head)-so_it0(content), so_len(content));
        //    usleep(2e6);
        //    printf("%.*s\n", SO_F(content));
        //} else {
            //printff("valid json.");
            json_auto_parse(content, &out);
            //json_auto_print(out, 0);
            json_auto_fmt(&json, out, 0);
            so_println(json);
            json_auto_free(&out);
        //}
    }
    so_free(&content);
    so_free(&json);
    return 0;
}

