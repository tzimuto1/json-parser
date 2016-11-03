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

#define JSON_HAS_TYPE(js, t)        (js && (js->type == t))
#define JSON_IS_NONE(js)            JSON_HAS_TYPE(js, JSON_TYPE_NONE)
#define JSON_IS_OBJECT(js)          JSON_HAS_TYPE(js, JSON_TYPE_OBJECT)
#define JSON_IS_ARRAY(js)           JSON_HAS_TYPE(js, JSON_TYPE_ARRAY)
#define JSON_IS_STRING(js)          JSON_HAS_TYPE(js, JSON_TYPE_STRING)
#define JSON_IS_NUMBER(js)          JSON_HAS_TYPE(js, JSON_TYPE_NUMBER)
#define JSON_IS_BOOLEAN(js)         JSON_HAS_TYPE(js, JSON_TYPE_BOOLEAN)
#define JSON_IS_NULL(js)            JSON_HAS_TYPE(js, JSON_TYPE_NULL)
#define JSON_IS_COMPLEX(js)         (JSON_IS_OBJECT(js) \
                                     || JSON_IS_ARRAY(js) \
                                     || JSON_IS_NULL(js))

#define IS_PRIMITIVE_TYPE(t)        ((t) == JSON_TYPE_STRING \
                                     || (t) == JSON_TYPE_NUMBER \
                                     || (t) == JSON_TYPE_BOOLEAN)


#define JSON_HAS_SIZE(js)           ((js->type == JSON_TYPE_STRING) \
                                     || (js->type == JSON_TYPE_ARRAY) \
                                     || (js->type == JSON_TYPE_OBJECT))
#define IDX_WITHIN_BOUNDS(js, idx)  ((idx) > -1 && (idx) < (js)->cnt)


typedef struct JSON json;
typedef struct json_object_pair obj_pair;

/* the key-value pair for JSON objects */
struct json_object_pair {
    unsigned char *key;
    json          *value;
};

/* json value object */
struct JSON
{
    json_type type;
    int       cnt;     // used for objects and arrays
    int       alloced; // used for objects and arrays
    union {
        obj_pair  **members; // used for objects
        json      **elements; // used for arrays
        double      num_val; // used for numbers
        bool        bool_val; // used for boolean
        unsigned char *string_val; // used for strings    
    };
};

typedef struct json_obj_iter {
    json *obj;
    int   idx;
} json_obj_iter;

// TODO consolidate string not found and number not found
typedef enum api_error
{
    API_ERROR_NONE,
    API_ERROR_NOT_OBJECT,
    API_ERROR_NOT_ARRAY,
    API_ERROR_KEY_NOT_FOUND,
    API_ERROR_KEY_INVALID,
    API_ERROR_NOT_FOUND,
    API_ERROR_VALUE_INVALID,
    API_ERROR_INPUT_INVALID,
} api_error;


/* json2string handling */
typedef struct {
    int  cnt;
    int  alloced;
    char *string;
} string_buf;

/* generic APIs */
json *json_create(json_type type);
json *json_full_create(json_type type, const void *val);
#define JSON_OBJECT_CREATE()  json_create(JSON_TYPE_OBJECT)
#define JSON_ARRAY_CREATE()   json_create(JSON_TYPE_ARRAY)
#define JSON_STRING_CREATE()  json_create(JSON_TYPE_STRING)
#define JSON_NUMBER_CREATE()  json_create(JSON_TYPE_NUMBER)
#define JSON_BOOLEAN_CREATE() json_create(JSON_TYPE_BOOLEAN)
#define JSON_NULL_CREATE()    json_create(JSON_TYPE_NULL)

bool  json_is_empty(json *js);
void  json_destroy(json *js);
int   json_get_size(json *js);
bool  json_is_equal2number(json *js, double number);
bool  json_is_equal2boolean(json *js, bool bool_val);
bool  json_is_equal2string(json *js, const char *string);
char *json2string(json *js, int indent);

/* object APIs */
struct json_obj_iter json_obj_iter_init(json *object);
obj_pair *json_obj_next(json_obj_iter *it);
#define json_obj_end(it)  _json_obj_end()
obj_pair *_json_obj_end();

bool    json_object_has_key(json *object, const char *key);
bool    json_object_has_number(json *object, double number);
bool    json_object_has_boolean(json *object, bool bool_val);
bool    json_object_has_string(json *object, const char *string);

json    *json_object_get(json *object, const char *key);
json    **json_object_get_all(json *object);
api_error json_object_get_number(json *object, const char *key, double *number);
api_error json_object_get_boolean(json *object, const char *key, bool *bool_val);

int     json_object_put_number(json *object, const char *key, double number);
int     json_object_put_boolean(json *object, const char *key, bool bool_val);
int     json_object_put_string(json *object, const char *key, const char *str_val);
int     json_object_put_complex_value(json *object, const char *key, json *value);

void    json_object_remove_member(json *object, const char *key);
api_error json_object_get_string(json *object, const char *key, char **str_val);

/* array APIs */
bool    json_array_has_number(json *array, double number);
bool    json_array_has_boolean(json *array, bool bool_val);
bool    json_array_has_string(json *array, const char *string);

json      *json_array_get(json *array, int idx);
api_error json_array_get_number(json *array, int idx, double *number);
api_error json_array_get_boolean(json *array, int idx, bool *bool_val);
api_error json_array_get_string(json *array, int idx, char **str_val);
json      **json_array_get_elements(json *array);

int     json_array_index_of_number(json *array, double number);
int     json_array_index_of_boolean(json *array, bool bool_val);
int     json_array_index_of_string(json *array, const char *str_val);

int     json_array_add_number(json *array, int idx, double number);
int     json_array_add_boolean(json *array, int idx, bool bool_val);
int     json_array_add_string(json *array, int idx, const char *str_val);

void    json_array_append_number(json *array, double number);
void    json_array_append_boolean(json *array, bool bool_val);
void    json_array_append_string(json *array, const char *str_val);

void    json_array_remove_at(json *array, int idx);
void    json_array_remove_number(json *array, double number);
void    json_array_remove_boolean(json *array, bool bool_val);
void    json_array_remove_string(json *array, const char *str_val);

#endif // JSON_H
