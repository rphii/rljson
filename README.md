# rljson

Immediate json parser ([`rljson-core`](rljson/rljson-core.c)) and auto parser ([`rljson-auto`](rljson/rljson-auto.c)).

## projects using `rljson`:

- [`rphii/c-whvn`](https://github.com/rphii/c-whvn) wallhaven cli interface
- [`rphii/rljson`](https://github.com/rphii/rljson) _(duh)_ see [`rljson-auto.c`](rljson/rljson-auto.c) (<90 loc json parser, quite verbose, it could be less)

## implementing immediate json parser code

noteworthy:

- using `array_???` functions / `ABORT` macros from `rphii/rlc`, general helper library
- using `so_???` functions from `rphii/rlso`, string library

### 0) reference object

see [`examples/readme.json`](examples/readme.json)

```json
{
    "id": 123,
    "name": "Sir. README",
    "icon": "\uD83D\uDDFF",
    "activities": [
        {
            "activity": "code",
            "icon": "\uD83D\uDCBB",
            "when": "nightly"
        },
        {
            "activity": "sleep",
            "icon": "\uD83D\uDECC",
            "when": "daily"
        }
    ]
}
```

### 1) create the equivalent structures in code

```c
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
    Readme_Activity *activities; // array
} Readme;
```

### 2) load file into string

```c
    Readme readme = {0}; // to be parsed object
    So content = SO; // empty string object
    So path = so("readme.json"); // string object from cstring

    if(so_file_read(path, &content)) {
        ABORT("failed reading file");
    }

    json_parse(content, parse_readme, &readme);
```

### 3) constructing functions

#### 3.1) getting top-level information

```c
void *parse_readme(void **user, Json_Parse_Value key, Json_Parse_Value *val) {

    printf("---%s---", __func__);
    printf("\nKEY: ");
    json_parse_value_print(&key);
    printf("\nVAL: ");
    json_parse_value_print(val);
    printf("\n");

    return 0;
}
```

compile, run, output:

```sh
$ gcc examples/readme.c rljson/*.c -lrlc -lrlso -o examples/readme.out && ./examples/readme.out
---parse_readme---
KEY: [object] id
VAL: [number] 123
---parse_readme---
KEY: [object] name
VAL: [string] Sir. README
---parse_readme---
KEY: [object] icon
VAL: [string] \uD83D\uDDFF
---parse_readme---
KEY: [object] activities
VAL:
```

#### 3.2) parse `.id`

```c
void *parse_readme(void **user, Json_Parse_Value key, Json_Parse_Value *val) {

    Readme *readme = *(Readme **)user;

    if(key.id == JSON_OBJECT) {
        if(!so_cmp(key.s, so("id"))) {
            if(so_as_uint(val->s, &readme->id, 0))
                ABORT("cannot parse to uint: %.*s", SO_F(val->s));
        }
    }

    return 0;
}
```

#### 3.3) parse `.name`

```c
void *parse_readme(void **user, Json_Parse_Value key, Json_Parse_Value *val) {

    Readme *readme = *(Readme **)user;

    if(key.id == JSON_OBJECT) {
        if(!so_cmp(key.s, so("name"))) {
            json_fix_so(val->s, &readme->name);
        }
    }

    return 0;
}
```

#### 3.4) parse `.icon`

```c
void *parse_readme(void **user, Json_Parse_Value key, Json_Parse_Value *val) {

    Readme *readme = *(Readme **)user;

    if(key.id == JSON_OBJECT) {
        if(!so_cmp(key.s, so("icon"))) {
            json_fix_so(val->s, &readme->icon);
        }
    }

    return 0;
}
```

#### 3.5) prepare to parse `.activities`

create a new function to parse activities, again, printing the KEY and VAL:

```c
void *parse_activities(void **user, Json_Parse_Value key, Json_Parse_Value *val) {

    printf("---%s---", __func__);
    printf("\nKEY: ");
    json_parse_value_print(&key);
    printf("\nVAL: ");
    json_parse_value_print(val);
    printf("\n");

    return 0;
}
```

now, in `parse_readme` we return this new function, and point the user to `Readme_Activities` (`.activities`):

```c
void *parse_readme(void **user, Json_Parse_Value key, Json_Parse_Value *val) {

    Readme *readme = *(Readme **)user;

    if(key.id == JSON_OBJECT) {
        if(!so_cmp(key.s, so("activities"))) {
            *user = &readme->activities;
            return parse_activities;
        }
    }

    return 0;
}
```

### 4) combining all previous

```c
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
```

lets also print the structure we get as a last thing in `main`:

```c
    printf("person.id: %u\n", readme.id);
    printf("person.name: %.*s\n", SO_F(readme.name));
    printf("person.icon: %.*s\n", SO_F(readme.icon));
    // the array:
    printf("person.activities.length: %zu\n", array_len(readme.activities));
    for(size_t i = 0; i < array_len(readme.activities); ++i) {
        Readme_Activity activity = array_at(readme.activities, i);
        printf("person.activities[%zu].icon: %.*s\n", i, SO_F(activity.icon));
        printf("person.activities[%zu].when: %u\n", i, activity.when);
    }
```

compile, run, output:

```sh
$ gcc examples/readme.c rljson/*.c -lrlc -lrlso -o examples/readme.out && ./examples/readme.out
---parse_readme---
KEY: [object] id
VAL: [number] 123
---parse_readme---
KEY: [object] name
VAL: [string] Sir. README
---parse_readme---
KEY: [object] icon
VAL: [string] \uD83D\uDDFF
---parse_readme---
KEY: [object] activities
VAL: 
---parse_activities---
KEY: [array] activities
VAL: 
---parse_activities---
KEY: [array] activities
VAL: 
person.id: 123
person.name: Sir. README
person.icon: 🗿
person.activities.length: 0
```

we see the `id`, `name` and `icon` are already okay. let's expand parsing the activities!

#### 4.1) add `parse_activity`

now when we `parse_activities`, what this actually indicates to us, is that we need a new array item (see previous/above output!)

so, let's `array_push` onto our `activities`. pay attention to the pointer-ing!

```c
void *parse_activity(void **user, Json_Parse_Value key, Json_Parse_Value *val) {

#if VERBOSE_INFO
    printf("---%s---", __func__);
    printf("\nKEY: ");
    json_parse_value_print(&key);
    printf("\nVAL: ");
    json_parse_value_print(val);
    printf("\n");
#endif

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
```

compile, run, output:

```sh
$ gcc examples/readme.c rljson/*.c -lrlc -lrlso -o examples/readme.out && ./examples/readme.out
---parse_activities---
KEY: [array] activities
VAL: 
---parse_activity---
KEY: [object] activity
VAL: [string] code
---parse_activity---
KEY: [object] icon
VAL: [string] \uD83D\uDCBB
---parse_activity---
KEY: [object] when
VAL: [string] nightly
---parse_activities---
KEY: [array] activities
VAL: 
---parse_activity---
KEY: [object] activity
VAL: [string] sleep
---parse_activity---
KEY: [object] icon
VAL: [string] \uD83D\uDECC
---parse_activity---
KEY: [object] when
VAL: [string] daily
```

we're getting there, let's finish this up once and for all in the next section

#### 4.2) parse activity

```c
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
```

### 5) done

compile, run, output:

```sh
$ gcc examples/readme.c rljson/*.c -lrlc -lrlso -o examples/readme.out && ./examples/readme.out
person.id: 123
person.name: Sir. README
person.icon: 🗿
person.activities.length: 2
person.activities[0].icon: 💻
person.activities[0].when: 2
person.activities[1].icon: 🛌
person.activities[1].when: 1
```

very nice!

### 6) expand

now you can easily expand upon this, say if this structure is a sub-structure of another json?

maybe your original structure now is in arrays like this:

```json
[
    {
        "id": 123,
        "name": "Sir. README",
        "icon": "\uD83D\uDDFF",
        "activities": [
            {
                "activity": "code",
                "icon": "\uD83D\uDCBB",
                "when": "nightly"
            },
            {
                "activity": "sleep",
                "icon": "\uD83D\uDECC",
                "when": "daily"
            }
        ]
    },
    {
        "id": 123,
        "name": "hacker",
        "icon": "\uD83D\uDCA3",
        "activities": [
            {
                "activity": "hack",
                "icon": "\u306F\u304B",
                "when": "daily"
            },
            {
                "activity": "hack",
                "icon": "🌇",
                "when": "nightly"
            }
        ]
    }
]
```

if we enable verbose output again, and read the new [`readme-expanded.json`](examples/readme-expanded.json):

```c
    // ...main...
    so_path_join(&path, so_get_dir(so_l(argv[0])), so("readme-expanded.json")); // so_l() -> runtime length determination, so() compile time length determination
```

compile and run:

```sh
$ gcc examples/readme.c rljson/*.c -lrlc -lrlso -o examples/readme.out && ./examples/readme.out
---parse_readme---
KEY: [array] 
VAL: 
[ABORT] examples/readme.c:103:parse_readme (end of trace) invalid readme structure, unexpected: 1
```

which is to be expected. however, notice that we now get the KEY == array. lets add a new function!

#### expand main

```c
    // ...main...
    Readme *readmes = 0; // to be parsed object
    if(json_parse(content, parse_readmes, &readmes))
        ABORT("invalid json: %.*s", SO_F(content));

    printf("readme.length: %zu\n", array_len(readmes));
    for(size_t i = 0; i < array_len(readmes); ++i) {
        printf("readme[%zu]\n", i);
        Readme readme = array_at(readmes, i);
        readme_print(readme);
    }
```

#### parse readmes

```c
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
```

#### done

compile, run:

```sh
$ gcc examples/readme.c rljson/*.c -lrlc -lrlso -o examples/readme.out && ./examples/readme.out
readme.length: 2
readme[0]
person.id: 123
person.name: Sir. README
person.icon: 🗿
person.activities.length: 2
person.activities[0].icon: 💻
person.activities[0].when: 2
person.activities[1].icon: 🛌
person.activities[1].when: 1
readme[1]
person.id: 123
person.name: hacker
person.icon: 💣
person.activities.length: 2
person.activities[0].icon: はか
person.activities[0].when: 1
person.activities[1].icon: 🌇
person.activities[1].when: 2
```

## auto parsing json

```c
int main(int argc, char **argv) {

    so_path_join(&path, so_get_dir(so_l(argv[0])), so("readme-expanded.json"));

    if(so_file_read(path, &content))
        ABORT("failed reading file: %.*s", SO_F(path));

    So out = SO;
    json_auto_fmt(&out, json_auto, 0);
    so_println(out);
}
```

compile, run, output:

```sh
$ gcc examples/readme.c rljson/*.c -lrlc -lrlso -o examples/readme.out && ./examples/readme.out 
[
  {
    "id": 123,
    "name": "Sir. README",
    "icon": "\uD83D\uDDFF",
    "activities": [
      {
        "activity": "code",
        "icon": "\uD83D\uDCBB",
        "when": "nightly"
      },
      {
        "activity": "sleep",
        "icon": "\uD83D\uDECC",
        "when": "daily"
      }
    ]
  },
  {
    "id": 123,
    "name": "hacker",
    "icon": "\uD83D\uDCA3",
    "activities": [
      {
        "activity": "hack",
        "icon": "\u306F\u304B",
        "when": "daily"
      },
      {
        "activity": "hack",
        "icon": "🌇",
        "when": "nightly"
      }
    ]
  }
]
```

