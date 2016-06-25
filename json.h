#ifndef JSON_H
#define JSON_H

#include <stdbool.h>

typedef enum json_type
{
    JSON_TYPE_NONE,
    JSON_TYPE_OBJECT,
    JSON_TYPE_ARRAY,
    JSON_TYPE_STRING,
    JSON_TYPE_NUMBER,
    JSON_TYPE_BOOLEAN,
    JSON_TYPE_NULL,
    JSON_TYPE_END,
} json_type;

/* the key-value pair for JSON objects */
typedef struct json_object_pair {
    const char  *key;
    struct json *value;
} obj_pair;

/* json value object */
typedef struct
{
    json_type type;
    int       cnt;     // used for objects and arrays
    int       alloced; // used for objects and arrays
    union {
        struct obj_pair **members; // used for objects
        struct json **elements; // used for arrays
        double num_val; // used for numbers
        bool   bool_val; // used for boolean
        char  *string_val; // used for strings    
    };
} json;

#endif // JSON_H
