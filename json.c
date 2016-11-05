
/*
 * JSON "object" methods
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include "json.h"

/* static function declarations */
static bool json_is_equal(json *js, const void *val, json_type type);
static void json_shallow_copy(json *js, const void *val_ptr, json_type type);
static int  _json2string(json *js, string_buf *buf, int indent);
static void string_buf_append(string_buf *buf, const char *fmt, ...);
static unsigned char *string2escaped_string(const unsigned char *str);

static void pair_destroy(obj_pair *pair);
static int  json_object_generic_get(json *object, const char *key, void *val_ptr, json_type type);
static bool json_object_has_value(json *object, const void *val, json_type type);
static int  json_object_generic_put(json *object, const char *key, const void *val, json_type type);

static int  json_array_index_of(json *array, const void *val, json_type type);
static int  json_array_generic_get(json *array, int idx, void *val_ptr, json_type type);
static int  json_array_generic_add(json *array, int idx, json_type type, const void *val);
static void json_array_append(json *array, const void *val, json_type type);
static void json_array_remove_element(json *array, const void *elem, json_type type);
static bool json_array_has_value(json *array, const void *val, json_type type);

/* ========== GENERIC METHODS ========== */

static bool json_is_equal(json *js, const void *val, json_type type)
{
    if (!JSON_HAS_TYPE(js, type) || !val)
    {
        return false;
    }

    switch (type)
    {
        case JSON_TYPE_NUMBER:
            return (js->num_val == (*(double *) val));
        case JSON_TYPE_BOOLEAN:
            return (js->bool_val == (*(bool *) val));
        case JSON_TYPE_STRING:
            return (strncmp((char *)js->string_val, 
                (char *) val, strlen((char *) js->string_val) + 1) == 0);
        default: // not currently supporting arrays and objects
            return false;
    }
}

/*
 * Determine if json value has given number
 */
bool json_is_equal2number(json *js, double number)
{
    return json_is_equal(js, &number, JSON_TYPE_NUMBER);
}

/*
 * Determine if json value has given boolean
 */
bool json_is_equal2boolean(json *js, bool bool_val)
{
    return json_is_equal(js, &bool_val, JSON_TYPE_BOOLEAN);
}

/*
 * Determine if json value has given string
 */
bool json_is_equal2string(json *js, const char *string)
{
    return json_is_equal(js, string, JSON_TYPE_STRING);
}


/*
 * Saves the contents of js in val
 */
static void json_shallow_copy(json *js, const void *val_ptr, json_type type)
{
    if (!JSON_HAS_TYPE(js, type) || !val_ptr)
    {
        return;
    }

    switch (type)
    {
        case JSON_TYPE_NUMBER:
            *(double *) val_ptr = js->num_val;
            break;
        case JSON_TYPE_BOOLEAN:
            *(bool *) val_ptr = js->bool_val;
            break;
        case JSON_TYPE_STRING: // no string duplication, hence shallow copying
            *(unsigned char **) val_ptr = js->string_val;
            break;
        default:
            break; 
    }
}

/*
 * Create a json object given the type of the object
 */
json *json_create(json_type type)
{
    assert(type > JSON_TYPE_NONE || type < JSON_TYPE_END);

    json *js = (json *) calloc(1, sizeof(json));
    js->type = type;

    if (type == JSON_TYPE_STRING)
    {
        js->string_val = (unsigned char *) calloc(1, sizeof(unsigned char));
        js->alloced = 1;
    }

    return js;
}

/*****************************************************************************/

/*
 * Create a json "object" give a type and value
 */
json *json_full_create(json_type type, const void *val)
{
    json *js = NULL;
    js = json_create(type);

    switch (type)
    {
        case JSON_TYPE_NUMBER:
            js->num_val = *(double *) val;
            break;
        case JSON_TYPE_BOOLEAN:
            js->bool_val = *(bool *) val;
            break;
        case JSON_TYPE_STRING:
            js->string_val = (unsigned char *) strdup((char *) val);
            js->cnt = strlen((char *) js->string_val) + 1;
            break;
        default:
            break;
    }
    return js;
}

/*****************************************************************************/

/*
 * Return the size of json "object"
 */
int json_get_size(json *js)
{
    if (!js || !JSON_HAS_SIZE(js))
    {
        return -1;
    }
    return js->cnt;
}

/*****************************************************************************/

/*
 * Return true if value is empty (makes sense for objects and arrays)
 */
bool json_is_empty(json *js)
{
    return json_get_size(js) <= 0;
}

/*****************************************************************************/

/*
 * Destroy a pair
 */
static void pair_destroy(obj_pair *pair)
{
    free(pair->key);
    json_destroy(pair->value);
    free(pair);
}

/*
 * Destoy json object based on type
 */
void json_destroy(json *js)
{
    if (!js)
        return;

    switch (js->type)
    {
        case JSON_TYPE_OBJECT:
        {
            int i;
            for (i = 0; i < js->cnt; i++)
            {
                pair_destroy(js->members[i]);
            }
            free(js->members);
            free(js);
            break;
        }

        case JSON_TYPE_ARRAY:
        {
            int i;
            for (i = 0; i < js->cnt; i++)
                json_destroy(js->elements[i]);
            free(js->elements);
            free(js);
            break;
        }

        case JSON_TYPE_STRING:
            free(js->string_val);
            free(js);
            break;
        default:
            free(js);
            break;
    }
}

/*****************************************************************************/

static unsigned char *string2escaped_string(const unsigned char *str)
{
    int            str_len = strlen((char *)str);
    unsigned char *escaped_str = NULL;
    unsigned char *e_str = NULL;

    if (!str)
    {
        return NULL;
    }

    escaped_str = (unsigned char *) malloc(sizeof(unsigned char) * (2 * str_len + 1));
    e_str = escaped_str;

    for ( ; *str; str++)
    {
        switch (*str)
        {
            case '"':
            case '\\':
            case '/':
                *e_str++ = '\\';
                *e_str++ = *str;
                break;
            case '\b':
                *e_str++ = '\\';
                *e_str++ = 'b';
                break;
            case '\f':
                *e_str++ = '\\';
                *e_str++ = 'f';
                break;
            case '\n':
                *e_str++ = '\\';
                *e_str++ = 'n';
                break;
            case '\r':
                *e_str++ = '\\';
                *e_str++ = 'r';
                break;
            case '\t':
                *e_str++ = '\\';
                *e_str++ = 't';
                break;
            default:
                *e_str++ = *str;
                break;
        }
    }
    *e_str = '\0';
    return escaped_str;
}

static void string_buf_append(string_buf *buf, const char *fmt, ...)
{
    /*
     * Part of this function was adapted from man 3 printf
     */
    int     size = 0;
    va_list ap;

    /* Determine required size */
    va_start(ap, fmt);
    size = vsnprintf(NULL, size, fmt, ap);
    va_end(ap);

    if (size < 0)
    {
        return;
    }

    // increase buffer if we will leave no space for the null character
    if (buf->cnt + size > (buf->alloced - 1))
    {
        buf->alloced = buf->cnt + size + 64; // add 64 more bytes
        buf->string = (char *) realloc(buf->string, sizeof(char) * buf->alloced);
    }

    // write to buffer
    va_start(ap, fmt);
    vsnprintf(buf->string + buf->cnt, size + 1, fmt, ap);
    va_end(ap);

    buf->cnt += size;

}

static int _json2string(json *js, string_buf *buf, int indent)
{
    int ret = API_SUCCESS;

    if (!js)
    {
        return API_FAILURE;
    }

    switch (js->type)
    {
        case JSON_TYPE_OBJECT:
        {
            json_obj_iter it;
            obj_pair *pair = NULL;
            unsigned char     *escaped_key = NULL;      

            string_buf_append(buf, "%c", '{');

            it = json_obj_iter_init(js);
            for (pair = json_obj_next(&it); pair != json_obj_end(&it); pair = json_obj_next(&it))
            {
                if (!(escaped_key = string2escaped_string(pair->key)))
                {
                    return API_FAILURE;
                }

                string_buf_append(buf, "\"%s\":", escaped_key);
                free(escaped_key);

                if (_json2string(pair->value, buf, indent) != API_SUCCESS)
                {
                    return API_FAILURE;
                }
                string_buf_append(buf, "%s", ", ");
            }

            if (json_get_size(js) > 0)
            {
                buf->cnt -= 2; // remove ", "
            }
            string_buf_append(buf, "%c", '}');
            break;
        }
        case JSON_TYPE_ARRAY:
        {
            int i = 0;

            string_buf_append(buf, "%c", '[');

            for (i = 0; i < json_get_size(js); i++)
            {
                if (_json2string(js->elements[i], buf, indent) != API_SUCCESS)
                {
                    return API_FAILURE;
                }
                string_buf_append(buf, "%s", ", ");
            }

            if (json_get_size(js) > 0)
            {
                buf->cnt -= 2; // remove ", "
            }
            string_buf_append(buf, "%c", ']');
            break;
        }
        case JSON_TYPE_STRING:
        {
            unsigned char *escaped_str = NULL;
            if (!(escaped_str = string2escaped_string(js->string_val)))
            {
                return API_FAILURE;
            }
            string_buf_append(buf, "\"%s\"", escaped_str);
            free(escaped_str);
            break;
        }
        case JSON_TYPE_NUMBER:
            string_buf_append(buf, "%f", js->num_val);
            break;
        case JSON_TYPE_BOOLEAN:
            string_buf_append(buf, "%s", js->bool_val ? "true" : "false");
            break;
        case JSON_TYPE_NULL:
            string_buf_append(buf, "%s", "null");
            break;
        default:
            ret = API_FAILURE;
            break;
    }

    return ret;
}

/*
 * Convert js to it's string representation with indentation of indent
 */
char *json2string(json *js, int indent)
{
    string_buf buf;
    buf.cnt = 0;
    buf.alloced = 256;
    buf.string = (char *) malloc(sizeof(char) * buf.alloced);

    if (_json2string(js, &buf, indent) != API_SUCCESS)
    {
        free(buf.string);
        return NULL;
    }
    
    buf.string[buf.cnt++] = '\0';
    return buf.string;
}

/* ========== OBJECT METHODS ========== */


json_obj_iter json_obj_iter_init(json *object)
{
    json_obj_iter it = { .obj = NULL, .idx = 0 };
    if (JSON_IS_OBJECT(object))
    {
        it.obj = object;
    }
    return it;
}

obj_pair *json_obj_next(json_obj_iter *it)
{
    json *object = it->obj;
    if (!JSON_IS_OBJECT(object) 
        || !IDX_WITHIN_BOUNDS(object, it->idx))
    {
        return NULL;
    }
    return object->members[it->idx++];
}

obj_pair *_json_obj_end()
{
    return NULL;
}

/*
 * Return true if json object has the key
 */
bool json_object_has_key(json *object, const char *key)
{
    int i = 0;

    if (!JSON_IS_OBJECT(object) || !key)
        return false;

    for (i = 0; i < object->cnt; i++)
    {
        if (strcmp(key, (char *) object->members[i]->key) == 0)
            return true;
    }

    return false;
}


static bool json_object_has_value(json *object, const void *val, json_type type)
{
    int i = 0;

    if (!JSON_IS_OBJECT(object))
        return false;

    for (i = 0; i < object->cnt; i++)
    {
        if (json_is_equal(object->members[i]->value, val, type))
        {
            return true;
        }
    }

    return false;
}
/*
 * Return true if json object has the number value
 */
bool json_object_has_number(json *object, double number)
{
    return json_object_has_value(object, &number, JSON_TYPE_NUMBER);
}


/*
 * Return true if json object has the boolean value
 */
bool json_object_has_boolean(json *object, bool bool_val)
{
    return json_object_has_value(object, &bool_val, JSON_TYPE_BOOLEAN);
}


/*
 * Return true if json object has the string value
 */
bool json_object_has_string(json *object, const char *string)
{
    return json_object_has_value(object, string, JSON_TYPE_STRING);
}


/*
 * Get the object value corresponding to given key
 */
json *json_object_get(json *object, const char *key)
{
    int   i = 0;

    if (!JSON_IS_OBJECT(object) || !key)
        return NULL;

    for (i = 0; i < object->cnt; i++)
    {
        if (strcmp(key, (char *) object->members[i]->key) == 0)
            return object->members[i]->value;
    }

    return NULL;
}


/*
 * Get all the values in the object
 * Memory is allocated to store the values and should be deallocated by the callee.
 * The last entry in the array is null and can be useful during iteration.
 */
json **json_object_get_all(json *object)
{
    int    i = 0;
    json **values = NULL;

    if (!JSON_IS_OBJECT(object))
    {
        return NULL;
    }

    values = (json **) calloc(object->cnt + 1, sizeof(json *));

    for (i = 0; i < object->cnt; i++)
    {
        values[i] = object->members[i]->value;
    }

    return values;
}


static int json_object_generic_get(json *object, const char *key, void *val_ptr, json_type type)
{
    int i = 0;

    if (!JSON_IS_OBJECT(object) || !key || !val_ptr)
    {
        return API_FAILURE;
    }

    for (i = 0; i < object->cnt; i++)
    {
        if (JSON_HAS_TYPE(object->members[i]->value, type) 
            && (strcmp(key, (char *) object->members[i]->key) == 0))
        {
            json_shallow_copy(object->members[i]->value, val_ptr, type);
            return API_SUCCESS;
        }
    }

    return API_FAILURE;
}

/*
 * Get the number corresponding to given key
 */
int json_object_get_number(json *object, const char *key, double *number)
{
    return json_object_generic_get(object, key, number, JSON_TYPE_NUMBER);
}

/*
 * Get the boolean corresponding to given key
 */
int json_object_get_boolean(json *object, const char *key, bool *bool_val)
{
    return json_object_generic_get(object, key, bool_val, JSON_TYPE_BOOLEAN);
}


/*
 * Get the string corresponding to given key
 */
int json_object_get_string(json *object, const char *key, char **str_val)
{
    return json_object_generic_get(object, key, str_val, JSON_TYPE_STRING);
}


static int json_object_generic_put(json *object, const char *key, const void *val, json_type type)
{
    int       i = 0;
    obj_pair *pair = NULL;
    
    if (!JSON_IS_OBJECT(object) || !key || !val)
    {
        return API_FAILURE;
    }

    for (i = 0; i < object->cnt; i++)
    {
        if (strcmp(key, (char *) object->members[i]->key) == 0)
        {
            json_destroy(object->members[i]->value);
            object->members[i]->value = json_full_create(type, val);
            return API_SUCCESS;
        }
    }

    pair = (obj_pair *) calloc(1, sizeof(obj_pair));
    pair->key = (unsigned char *) strdup(key);

    if (IS_PRIMITIVE_TYPE(type))
    {
        pair->value = json_full_create(type, val);
    }
    else
    {
        pair->value = (json *) val;
    }
    

    if (object->cnt == object->alloced)
    {
        object->alloced += 10;
        object->members = (obj_pair **) realloc(object->members, 
            sizeof(obj_pair *) * object->alloced);
    }

    object->members[object->cnt++] = pair;
    return API_SUCCESS;
}

/*
 * Put a number in an object
 */
int json_object_put_number(json *object, const char *key, double number)
{
    return json_object_generic_put(object, key, &number, JSON_TYPE_NUMBER);
}


/*
 * Put a boolean in an object
 */
int json_object_put_boolean(json *object, const char *key, bool bool_val)
{
    return json_object_generic_put(object, key, &bool_val, JSON_TYPE_BOOLEAN);
}


/*
 * Map key to the value str_val
 */
int json_object_put_string(json *object, const char *key, const char *str_val)
{
    return json_object_generic_put(object, key, str_val, JSON_TYPE_STRING);
}

/*
 * Map key to to complex type: TODO not tested
 */
int json_object_put_complex_value(json *object, const char *key, json *value)
{
    if (!JSON_IS_COMPLEX(value))
    {
        return API_FAILURE;
    }
    return json_object_generic_put(object, key, value, value->type);
}

/*
 * Remove the pair(s) with given key
 */
void json_object_remove_member(json *object, const char *key)
{
    int i = 0;
    int j = 0;
    int num_rem = 0;  

    if (!JSON_IS_OBJECT(object) || !key)
        return;

    for (i = 0; i < object->cnt; i++)
    {
        if (strcmp(key, (char *)object->members[i]->key) == 0)
        {
            pair_destroy(object->members[i]);

            // shift items to the left
            for (j = i + 1; j < object->cnt; j++)
            {
                object->members[j - 1] = object->members[j];
            }

            num_rem++;
        }
    }

    object->cnt -= num_rem;
    object->alloced -= num_rem;
}


/* ========== ARRAY METHODS ========== */

static bool json_array_has_value(json *array, const void *val, json_type type)
{
    int i = 0;

    if (!JSON_IS_ARRAY(array))
        return false;

    for (i = 0; i < array->cnt; i++)
    {
        if (json_is_equal(array->elements[i], val, type))
        {
            return true;
        }
    }
    return false;
}

/*
 * Return true if json array has the number value
 */
bool json_array_has_number(json *array, double number)
{
    return json_array_has_value(array, &number, JSON_TYPE_NUMBER);
}


/*
 * Return true if json array has the boolean value
 */
bool json_array_has_boolean(json *array, bool bool_val)
{
   return json_array_has_value(array, &bool_val, JSON_TYPE_BOOLEAN);
}


/*
 * Return true if json array has the string value
 */
bool json_array_has_string(json *array, const char *string)
{
    return json_array_has_value(array, string, JSON_TYPE_STRING);
}


/*
 * Get the array value corresponding to given index
 */
json *json_array_get(json *array, int idx)
{
    if (JSON_IS_ARRAY(array) 
        && IDX_WITHIN_BOUNDS(array, idx))
    {
        return array->elements[idx];
    }
    return NULL;
}


static int json_array_generic_get(json *array, int idx, void *val_ptr, json_type type)
{
    if (!JSON_IS_ARRAY(array) 
        || !IDX_WITHIN_BOUNDS(array, idx)
        || !val_ptr)
    {
        return API_FAILURE;
    }

    if (JSON_HAS_TYPE(array->elements[idx], type))
    {
        json_shallow_copy(array->elements[idx], val_ptr, type);
        return API_SUCCESS;
    }
    return API_FAILURE;
}

/*
 * Get the number corresponding to given index
 */
int json_array_get_number(json *array, int idx, double *number)
{
    return json_array_generic_get(array, idx, number, JSON_TYPE_NUMBER);
}


/*
 * Get the boolean corresponding to given index
 */
int json_array_get_boolean(json *array, int idx, bool *bool_val)
{
    return json_array_generic_get(array, idx, bool_val, JSON_TYPE_BOOLEAN);
}


/*
 * Get the string corresponding to given index
 */
int json_array_get_string(json *array, int idx, char **str_val)
{
    return json_array_generic_get(array, idx, str_val, JSON_TYPE_STRING);
}

/*
 * Return real array of elements making the json array
 */
 json **json_array_get_elements(json *array)
 {
    if (!JSON_IS_ARRAY(array))
    {
        return NULL;
    }
    return array->elements;
 }

static int json_array_index_of(json *array, const void *val, json_type type)
{
    int i = 0;

    if (!JSON_IS_ARRAY(array))
    {
        return -1;
    }

    for (i = 0; i < array->cnt; i++)
    {
        if (json_is_equal(array->elements[i], val, type))
        {
            return i;
        }
    }

    return -1;
}

/*
 * Get first index of given number in array
 */
int json_array_index_of_number(json *array, double number)
{
    return json_array_index_of(array, &number, JSON_TYPE_NUMBER);
}

/*
 * Get first index of given boolean in array
 */
int json_array_index_of_boolean(json *array, bool bool_val)
{
    return json_array_index_of(array, &bool_val, JSON_TYPE_BOOLEAN);
}

/*
 * Get first index of given string in array
 */
int json_array_index_of_string(json *array, const char *str_val)
{
    return json_array_index_of(array, str_val, JSON_TYPE_STRING);
}

/*
 * Put a value in an array
 */
static int json_array_generic_add(json *array, int idx, json_type type, const void *val)
{
    if (!JSON_IS_ARRAY(array) 
        || !IDX_WITHIN_BOUNDS(array, idx))
    {
        return API_FAILURE;
    }

    assert(type > JSON_TYPE_NONE || type < JSON_TYPE_END);

    // destroy original element and create a new one;
    json_destroy(array->elements[idx]); // TODO can we reuse memory instead of deallocating?
    array->elements[idx] = json_full_create(type, val);

    return API_SUCCESS;
}

/*
 * Put a number at given index in an array
 */
int json_array_add_number(json *array, int idx, double number)
{
    return json_array_generic_add(array, idx, JSON_TYPE_NUMBER, &number);
}

/*
 * Put a boolean at given index in an array
 */
int json_array_add_boolean(json *array, int idx, bool bool_val)
{
    return json_array_generic_add(array, idx, JSON_TYPE_BOOLEAN, &bool_val);
}

/*
 * Put a string at given index in an array
 */
int json_array_add_string(json *array, int idx, const char *str_val)
{
    return json_array_generic_add(array, idx, JSON_TYPE_STRING, str_val);
}

static void json_array_append(json *array, const void *val, json_type type)
{
    json *js = NULL;

    assert(NULL == js);

    if (!JSON_IS_ARRAY(array) 
        || !IS_PRIMITIVE_TYPE(type))
    {
        return;
    }

    js = json_full_create(type, val);
    if (array->cnt == array->alloced)
    {
        array->alloced += 10;
        array->elements = (json **) realloc(array->elements, 
            sizeof(json *) * array->alloced);
    }

    array->elements[array->cnt++] = js;
}

/*
 * Append a number to a given array
 */
void json_array_append_number(json *array, double number)
{
    json_array_append(array, &number, JSON_TYPE_NUMBER);
}

/*
 * Append a boolean to a given array
 */
void json_array_append_boolean(json *array, bool bool_val)
{
    json_array_append(array, &bool_val, JSON_TYPE_BOOLEAN);
}

/*
 * Append a string to a given array
 */
void json_array_append_string(json *array, const char *str_val)
{
    json_array_append(array, str_val, JSON_TYPE_STRING);
}

/*
 * Remove the element at the given index
 */
void json_array_remove_at(json *array, int idx)
{
    int   i = 0;
    json *js;

    if (!JSON_IS_ARRAY(array) 
        || !IDX_WITHIN_BOUNDS(array, idx))
    {
        return;
    }

    // destroy element
    js = array->elements[idx];
    json_destroy(js);

    // shift elements
    for (i = idx + 1; i < array->cnt; i++)
    {
        array->elements[i - 1] = array->elements[i];
    }

    array->cnt--;
    array->alloced--;
}

static void json_array_remove_element(json *array, const void *elem, json_type type)
{
    // more efficient to remove element in one loop, but delegation is neat:)
    int idx = json_array_index_of(array, elem, type);

    if (idx < 0)
    {
        return;
    }

    json_array_remove_at(array, idx);
}

/*
 * Remove the first occurence of a number
 */
void json_array_remove_number(json *array, double number)
{
    json_array_remove_element(array, &number, JSON_TYPE_NUMBER);
}

/*
 * Remove the first occurence of a boolean
 */
void json_array_remove_boolean(json *array, bool bool_val)
{
    json_array_remove_element(array, &bool_val, JSON_TYPE_BOOLEAN);
}

/*
 * Remove the first occurence of a string
 */
void json_array_remove_string(json *array, const char *str_val)
{
    json_array_remove_element(array, str_val, JSON_TYPE_STRING);
}
