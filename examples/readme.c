#include "../rljson.h"

#define VERBOSE_INFO    0
#define EXPANDED        1
#define AUTO            1

typedef enum {
    README_FREQUENCY_NONE,
    README_FREQUENCY_DAILY,
    README_FREQUENCY_NIGHTLY,
} Readme_When_List;

typedef struct Readme_Activity {
    So icon;
    Readme_When_List when;
} Readme_Activity;

typedef struct Readme {
    unsigned int id;
    So name;
    So icon;
    Readme_Activity *activities;
} Readme;

void *parse_activity(void **user, Json_Parse_Value key, Json_Parse_Value *val) {

#if VERBOSE_INFO
    printf("---%s---", __func__);
    printf("\nKEY: ");
    json_parse_value_print(&key);
    printf("\nVAL: ");
    json_parse_value_print(val);
    printf("\n");
#endif

    Readme_Activity *activity = *(Readme_Activity **)user;

    if(key.id == JSON_OBJECT) {
        if(!val) {
            ABORT("expected value, have none:");
        }
        if(!so_cmp(key.s, so("icon"))) {
            json_fix_so(val->s, &activity->icon);
        } else if(!so_cmp(key.s, so("when"))) {
            if(!so_cmp(val->s, so("daily"))) {
                activity->when = README_FREQUENCY_DAILY;
            } else if(!so_cmp(val->s, so("nightly"))) {
                activity->when = README_FREQUENCY_NIGHTLY;
            } else {
                ABORT("unknown 'when' value: %.*s", SO_F(val->s));
            }
        }
    } else {
        ABORT("invalid readme structure, unexpected: %u", key.id);
    }

    return 0;
}

void *parse_activities(void **user, Json_Parse_Value key, Json_Parse_Value *val) {

#if VERBOSE_INFO
    printf("---%s---", __func__);
    printf("\nKEY: ");
    json_parse_value_print(&key);
    printf("\nVAL: ");
    json_parse_value_print(val);
    printf("\n");
#endif

    Readme_Activity **activities = *(Readme_Activity ***)user;

    if(key.id == JSON_ARRAY) {
        array_push(*activities, (Readme_Activity){0});
        *user = array_itE(*activities);
        return parse_activity;
    } else {
        ABORT("unexpected id: %u", key.id);
    }
}

void *parse_readme(void **user, Json_Parse_Value key, Json_Parse_Value *val) {

#if VERBOSE_INFO
    printf("---%s---", __func__);
    printf("\nKEY: ");
    json_parse_value_print(&key);
    printf("\nVAL: ");
    json_parse_value_print(val);
    printf("\n");
#endif

    Readme *readme = *(Readme **)user;

    if(key.id == JSON_OBJECT) {
        if(!so_cmp(key.s, so("id"))) {
            if(so_as_uint(val->s, &readme->id, 0))
                ABORT("cannot parse to uint: %.*s", SO_F(val->s));
        } else if(!so_cmp(key.s, so("name"))) {
            json_fix_so(val->s, &readme->name);
        } else if(!so_cmp(key.s, so("icon"))) {
            json_fix_so(val->s, &readme->icon);
        } else if(!so_cmp(key.s, so("activities"))) {
            *user = &readme->activities;
            return parse_activities;
        } else {
            ABORT("invalid object: %.*s", SO_F(key.s));
        }
    } else {
        ABORT("invalid readme structure, unexpected: %u", key.id);
    }

    return 0;
}

void *parse_readmes(void **user, Json_Parse_Value key, Json_Parse_Value *val) {

#if VERBOSE_INFO
    printf("---%s---", __func__);
    printf("\nKEY: ");
    json_parse_value_print(&key);
    printf("\nVAL: ");
    json_parse_value_print(val);
    printf("\n");
#endif

    Readme **readmes = *(Readme ***)user;

    if(key.id == JSON_ARRAY) {
        array_push(*readmes, (Readme){0});
        *user = array_itE(*readmes);
        return parse_readme;
    } else {
        ABORT("unexpected id: %u", key.id);
    }
    return 0;
}

void readme_print(Readme readme) {
    printf("person.id: %u\n", readme.id);
    printf("person.name: %.*s\n", SO_F(readme.name));
    printf("person.icon: %.*s\n", SO_F(readme.icon));
    printf("person.activities.length: %zu\n", array_len(readme.activities));
    for(size_t i = 0; i < array_len(readme.activities); ++i) {
        Readme_Activity activity = array_at(readme.activities, i);
        printf("person.activities[%zu].icon: %.*s\n", i, SO_F(activity.icon));
        printf("person.activities[%zu].when: %u\n", i, activity.when);
    }
}

int main(int argc, char **argv) {

    So content = SO, path = SO; // empty string objects

#if !(EXPANDED)

    so_path_join(&path, so_get_dir(so_l(argv[0])), so("readme.json")); // so_l() -> runtime length determination, so() compile time length determination

    Readme readme = {0}; // to be parsed object
    if(so_file_read(path, &content))
        ABORT("failed reading file: %.*s", SO_F(path));

    if(json_parse(content, parse_readme, &readme))
        ABORT("invalid json: %.*s", SO_F(content));

    readme_print(readme);

#else

    so_path_join(&path, so_get_dir(so_l(argv[0])), so("readme-expanded.json")); // so_l() -> runtime length determination, so() compile time length determination

    if(so_file_read(path, &content))
        ABORT("failed reading file: %.*s", SO_F(path));

#if AUTO

    Json_Auto_Value json_auto = {0};
    if(json_auto_parse(content, &json_auto))
        ABORT("invalid json");

    So out = SO;
    json_auto_fmt(&out, json_auto, 0);
    so_println(out);

#else

    Readme *readmes = 0; // to be parsed object
    if(json_parse(content, parse_readmes, &readmes))
        ABORT("invalid json: %.*s", SO_F(content));

    printf("readme.length: %zu\n", array_len(readmes));
    for(size_t i = 0; i < array_len(readmes); ++i) {
        printf("readme[%zu]\n", i);
        Readme readme = array_at(readmes, i);
        readme_print(readme);
    }

#endif

#endif
}

