
/*
 * JSON "object" methods
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "json.h"

/* static function declarations */
static bool json_is_equal(json *js, const void *val, json_type type);
static void json_shallow_copy(json *js, const void *val_ptr, json_type type);

static void pair_destroy(obj_pair *pair);
static api_error json_object_generic_get(json *object, const char *key, void *val_ptr, json_type type);
static bool json_object_has_value(json *object, const void *val, json_type type);
static int  json_object_generic_put(json *object, const char *key, const void *val, json_type type);

static int  json_array_index_of(json *array, const void *val, json_type type);
static api_error json_array_generic_get(json *array, int idx, void *val_ptr, json_type type);
static int  json_array_generic_add(json *array, int idx, json_type type, const void *val);
static void json_array_append(json *array, const void *val, json_type type);
static void json_array_remove_element(json *array, const void *elem, json_type type);
static bool json_array_has_value(json *array, const void *val, json_type type);

/* ========== GENERIC METHODS ========== */

/*
 * Determine if a json value has the given value
 */
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
            return (strncmp(js->string_val, 
                (char *) val, strlen(js->string_val) + 1) == 0);
        default: // not currently supporting arrays and objects
            return false;
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
    return js;
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
            *(char **) val_ptr = js->string_val;
            break;
        default:
            break; 
    }
}

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
            js->string_val = strdup((char *) val);
            js->cnt = strlen(js->string_val) + 1;
            break;
        default:
            break;
    }
    return js;
}

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

/*
 * Return true if value is empty (makes sense for objects and arrays)
 */
bool json_is_empty(json *js)
{
    return json_get_size(js) <= 0;
}

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

/* ========== OBJECT METHODS ========== */

/*
 * Return true if json object has the key
 */
bool json_object_has_key(json *object, const char *key)
{
    int i = 0;

    if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT) || !key)
        return false;

    for (i = 0; i < object->cnt; i++)
    {
        if (strcmp(key, object->members[i]->key) == 0)
            return true;
    }

    return false;
}


static bool json_object_has_value(json *object, const void *val, json_type type)
{
    int i = 0;

    if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT))
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

    if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT) || !key)
        return NULL;

    for (i = 0; i < object->cnt; i++)
    {
        if (strcmp(key, object->members[i]->key) == 0)
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

    if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT))
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


static api_error json_object_generic_get(json *object, const char *key, void *val_ptr, json_type type)
{
    int i = 0;

    if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT) || !key || !val_ptr)
    {
        return API_ERROR_INPUT_INVALID;
    }

    for (i = 0; i < object->cnt; i++)
    {
        if (JSON_HAS_TYPE(object->members[i]->value, type) 
            && (strcmp(key, object->members[i]->key) == 0))
        {
            json_shallow_copy(object->members[i]->value, val_ptr, type);
            return API_ERROR_NONE;
        }
    }

    return API_ERROR_NOT_FOUND;
}

/*
 * Get the number corresponding to given key
 */
api_error json_object_get_number(json *object, const char *key, double *number)
{
    return json_object_generic_get(object, key, number, JSON_TYPE_NUMBER);
}

/*
 * Get the boolean corresponding to given key
 */
api_error json_object_get_boolean(json *object, const char *key, bool *bool_val)
{
    return json_object_generic_get(object, key, bool_val, JSON_TYPE_BOOLEAN);
}


/*
 * Get the string corresponding to given key
 */
api_error json_object_get_string(json *object, const char *key, char **str_val)
{
    return json_object_generic_get(object, key, str_val, JSON_TYPE_STRING);
}


static int json_object_generic_put(json *object, const char *key, const void *val, json_type type)
{
    int       i = 0;
    obj_pair *pair = NULL;

    // TODO error handling a mess
    if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT))
    {
        return API_ERROR_NOT_OBJECT; // should be input invalid maybe
    }

    if (!key)
    {
        return API_ERROR_KEY_INVALID;
    }

    if (!val)
    {
        return API_ERROR_VALUE_INVALID;
    }

    for (i = 0; i < object->cnt; i++)
    {
        if (strcmp(key, object->members[i]->key) == 0)
        {
            json_destroy(object->members[i]->value);
            object->members[i]->value = json_full_create(type, val);
            return API_ERROR_NONE;
        }
    }

    pair = (obj_pair *) calloc(1, sizeof(obj_pair));
    pair->key = strdup(key);
    pair->value = json_full_create(type, val);

    if (object->cnt == object->alloced)
    {
        object->alloced += 10;
        object->members = (obj_pair **) realloc(object->members, 
            sizeof(obj_pair *) * object->alloced);
    }

    object->members[object->cnt++] = pair;
    return API_ERROR_NONE;
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
 * Remove the pair(s) with given key
 */
void json_object_remove_member(json *object, const char *key)
{
    int i = 0;
    int j = 0;
    int num_rem = 0;  

    if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT) || !key)
        return;

    for (i = 0; i < object->cnt; i++)
    {
        if (strcmp(key, object->members[i]->key) == 0)
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

    if (!JSON_HAS_TYPE(array, JSON_TYPE_ARRAY))
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
    if (JSON_HAS_TYPE(array, JSON_TYPE_ARRAY) 
        && IDX_WITHIN_BOUNDS(array, idx))
    {
        return array->elements[idx];
    }
    return NULL;
}


static api_error json_array_generic_get(json *array, int idx, void *val_ptr, json_type type)
{
    if (!JSON_HAS_TYPE(array, JSON_TYPE_ARRAY) 
        || !IDX_WITHIN_BOUNDS(array, idx)
        || !val_ptr)
    {
        return API_ERROR_INPUT_INVALID;
    }

    if (JSON_HAS_TYPE(array->elements[idx], type))
    {
        json_shallow_copy(array->elements[idx], val_ptr, type);
        return API_ERROR_NONE;
    }
    return API_ERROR_NOT_FOUND;
}

/*
 * Get the number corresponding to given index
 */
api_error json_array_get_number(json *array, int idx, double *number)
{
    return json_array_generic_get(array, idx, number, JSON_TYPE_NUMBER);
}


/*
 * Get the boolean corresponding to given index
 */
api_error json_array_get_boolean(json *array, int idx, bool *bool_val)
{
    return json_array_generic_get(array, idx, bool_val, JSON_TYPE_BOOLEAN);
}


/*
 * Get the string corresponding to given index
 */
api_error json_array_get_string(json *array, int idx, char **str_val)
{
    return json_array_generic_get(array, idx, str_val, JSON_TYPE_STRING);
}


static int json_array_index_of(json *array, const void *val, json_type type)
{
    int i = 0;

    if (!JSON_HAS_TYPE(array, JSON_TYPE_ARRAY))
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
    if (!JSON_HAS_TYPE(array, JSON_TYPE_ARRAY) 
        || !IDX_WITHIN_BOUNDS(array, idx))
    {
        return API_ERROR_INPUT_INVALID;
    }

    assert(type > JSON_TYPE_NONE || type < JSON_TYPE_END);

    // destroy original element and create a new one;
    json_destroy(array->elements[idx]); // TODO can we reuse memory instead of deallocating?
    array->elements[idx] = json_full_create(type, val);

    return API_ERROR_NONE;
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

    if (!JSON_HAS_TYPE(array, JSON_TYPE_ARRAY) 
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

    if (!JSON_HAS_TYPE(array, JSON_TYPE_ARRAY) 
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
