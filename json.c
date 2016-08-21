
/*
 * JSON "object" methods
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "json.h"

/* static function declarations */
static bool json_is_equal(json *js, const void *val, json_type type);
static void pair_destroy(obj_pair *pair);
static int  json_array_index_of(json *array, const void *val, json_type type);
static int  json_array_generic_add(json *array, int idx, json_type type, const void *val);
static void json_array_append(json *array, const void *val, json_type type);
static void json_array_remove_element(json *array, const void *elem, json_type type);

/* ========== GENERIC METHODS ========== */

/*
 * Determine if a json value has the given value
 */
static bool json_is_equal(json *js, const void *val, json_type type)
{
    if (!JSON_HAS_TYPE(js, type))
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


/*
 * Return true if json object has the number value
 */
bool json_object_has_number(json *object, double number)
{
    int i = 0;

    if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT))
        return false;

    for (i = 0; i < object->cnt; i++)
    {
        if (JSON_HAS_TYPE(object->members[i]->value, JSON_TYPE_NUMBER))
        {
            if (object->members[i]->value->num_val == number)
                return true;
        }
    }

    return false;
}


/*
 * Return true if json object has the boolean value
 */
bool json_object_has_boolean(json *object, bool bool_val)
{
    int i = 0;

    if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT))
        return false;

    for (i = 0; i < object->cnt; i++)
    {
        if (JSON_HAS_TYPE(object->members[i]->value, JSON_TYPE_BOOLEAN))
        {
            if (object->members[i]->value->bool_val == bool_val)
                return true;
        }
    }

    return false;
}


/*
 * Return true if json object has the string value
 */
bool json_object_has_string(json *object, const char *string)
{
    int i = 0;

    if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT) || !string)
        return false;

    for (i = 0; i < object->cnt; i++)
    {
        if (JSON_HAS_TYPE(object->members[i]->value, JSON_TYPE_STRING))
        {
            if (strcmp(string, object->members[i]->value->string_val) == 0)
                return true;
        }
    }

    return false;
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


/*
 * Get the number corresponding to given key
 */
api_error json_object_get_number(json *object, const char *key, double *number)
{
    int i = 0;

    if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT) || !key || !number)
    {
        return API_ERROR_INPUT_INVALID;
    }

    for (i = 0; i < object->cnt; i++)
    {
        if (JSON_HAS_TYPE(object->members[i]->value, JSON_TYPE_NUMBER) 
            && (strcmp(key, object->members[i]->key) == 0))
        {
            *number = object->members[i]->value->num_val;
            return API_ERROR_NONE;
        }
    }

    return API_ERROR_NOT_FOUND;
}

/*
 * Get the boolean corresponding to given key
 */
api_error json_object_get_boolean(json *object, const char *key, bool *bool_val)
{
    int i = 0;

    if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT) || !key || !bool_val)
    {
        return API_ERROR_INPUT_INVALID;
    }

    for (i = 0; i < object->cnt; i++)
    {
        if (JSON_HAS_TYPE(object->members[i]->value, JSON_TYPE_BOOLEAN) 
            && (strcmp(key, object->members[i]->key) == 0))
        {
            *bool_val = object->members[i]->value->bool_val;
            return API_ERROR_NONE;
        }
    }

    return API_ERROR_NOT_FOUND;
}


/*
 * Get the string corresponding to given key
 */
api_error json_object_get_string(json *object, const char *key, char **str_val)
{
    int i = 0;

    if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT) || !key || !str_val)
    {
        return API_ERROR_INPUT_INVALID;
    }

    for (i = 0; i < object->cnt; i++)
    {
        if (JSON_HAS_TYPE(object->members[i]->value, JSON_TYPE_STRING) 
            && (strcmp(key, object->members[i]->key) == 0))
        {
            *str_val = object->members[i]->value->string_val;
            return API_ERROR_NONE;
        }
    }

    return API_ERROR_NOT_FOUND;
}


/*
 * Put a number in an object
 */
  // TODO: should call a generic function
int json_object_put_number(json *object, const char *key, double number)
{
    int       i = 0;
    obj_pair *pair = NULL;

    if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT))
        return API_ERROR_NOT_OBJECT; // should be input invalid maybe

    for (i = 0; i < object->cnt; i++)
    {
        if (strcmp(key, object->members[i]->key) == 0)
        {
            object->members[i]->value->num_val = number;
            return API_ERROR_NONE;
        }
    }

    pair = (obj_pair *) calloc(1, sizeof(obj_pair));
    pair->key = strdup(key);
    pair->value = json_create(JSON_TYPE_NUMBER);
    pair->value->num_val = number;

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
 * Put a boolean in an object
 */
 // TODO: should call a generic function
int json_object_put_boolean(json *object, const char *key, bool bool_val)
{
    int       i = 0;
    obj_pair *pair = NULL;

    if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT))
        return API_ERROR_NOT_OBJECT;

    if (!key)
        return API_ERROR_KEY_INVALID;

    for (i = 0; i < object->cnt; i++)
    {
        if (strcmp(key, object->members[i]->key) == 0)
        {
            object->members[i]->value->bool_val = bool_val;
            return API_ERROR_NONE;
        }
    }

    pair = (obj_pair *) calloc(1, sizeof(obj_pair));
    pair->key = strdup(key);
    pair->value = json_create(JSON_TYPE_BOOLEAN);
    pair->value->bool_val = bool_val;

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
 * Map key to the value str_val
 */
int json_object_put_string(json *object, const char *key, const char *str_val)
{
    int       i = 0;
    obj_pair *pair = NULL;

    if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT))
        return API_ERROR_NOT_OBJECT;

    if (!key)
        return API_ERROR_KEY_INVALID;

    if (!str_val)
    {
        return API_ERROR_VALUE_INVALID;
    }

    for (i = 0; i < object->cnt; i++)
    {
        if (strcmp(key, object->members[i]->key) == 0)
        {
            free(object->members[i]->value->string_val);
            object->members[i]->value->string_val = strdup(str_val);
            return API_ERROR_NONE;
        }
    }

    // create a new pair
    pair = (obj_pair *) calloc(1, sizeof(obj_pair));
    pair->key = strdup(key);
    pair->value = json_create(JSON_TYPE_STRING);
    pair->value->string_val = strdup(str_val);
    pair->value->cnt = strlen(str_val) + 1;

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
 * Remove the pair(s) with given key
 * TODO: bug, reduce the number of alloced items too
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
}


/* ========== ARRAY METHODS ========== */

/*
 * Return true if json array has the number value
 */
bool json_array_has_number(json *array, double number)
{
    int i = 0;

    if (!JSON_HAS_TYPE(array, JSON_TYPE_ARRAY))
        return false;

    for (i = 0; i < array->cnt; i++)
    {
        if (JSON_HAS_TYPE(array->elements[i], JSON_TYPE_NUMBER))
        {
            if (array->elements[i]->num_val == number)
                return true;
        }
    }

    return false;
}


/*
 * Return true if json array has the boolean value
 */
bool json_array_has_boolean(json *array, bool bool_val)
{
    int i = 0;

    if (!JSON_HAS_TYPE(array, JSON_TYPE_ARRAY))
        return false;

    for (i = 0; i < array->cnt; i++)
    {
        if (JSON_HAS_TYPE(array->elements[i], JSON_TYPE_BOOLEAN))
        {
            if (array->elements[i]->bool_val == bool_val)
                return true;
        }
    }

    return false;
}


/*
 * Return true if json array has the string value
 */
bool json_array_has_string(json *array, const char *string)
{
    int i = 0;

    if (!JSON_HAS_TYPE(array, JSON_TYPE_ARRAY) || !string)
        return false;

    for (i = 0; i < array->cnt; i++)
    {
        if (JSON_HAS_TYPE(array->elements[i], JSON_TYPE_STRING))
        {
            if (strcmp(string, array->elements[i]->string_val) == 0)
                return true;
        }
    }

    return false;
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


/*
 * Get the number corresponding to given index
 */
api_error json_array_get_number(json *array, int idx, double *number)
{
    if (!JSON_HAS_TYPE(array, JSON_TYPE_ARRAY) 
        || !IDX_WITHIN_BOUNDS(array, idx)
        || !number)
    {
        return API_ERROR_INPUT_INVALID;
    }

    if (JSON_HAS_TYPE(array->elements[idx], JSON_TYPE_NUMBER))
    {
        *number = array->elements[idx]->num_val;
        return API_ERROR_NONE;
    }
    return API_ERROR_NOT_FOUND;
}


/*
 * Get the boolean corresponding to given index
 */
api_error json_array_get_boolean(json *array, int idx, bool *bool_val)
{
    if (!JSON_HAS_TYPE(array, JSON_TYPE_ARRAY) 
        || !IDX_WITHIN_BOUNDS(array, idx)
        || !bool_val)
    {
        return API_ERROR_INPUT_INVALID;
    }

    if (JSON_HAS_TYPE(array->elements[idx], JSON_TYPE_BOOLEAN))
    {
        *bool_val = array->elements[idx]->bool_val;
        return API_ERROR_NONE;
    }
    return API_ERROR_NOT_FOUND;
}


/*
 * Get the string corresponding to given index
 */
api_error json_array_get_string(json *array, int idx, char **str_val)
{
    if (!JSON_HAS_TYPE(array, JSON_TYPE_ARRAY) 
        || !IDX_WITHIN_BOUNDS(array, idx)
        || !str_val)
    {
        return API_ERROR_INPUT_INVALID;
    }

    if (JSON_HAS_TYPE(array->elements[idx], JSON_TYPE_STRING))
    {
        *str_val = array->elements[idx]->string_val;
        return API_ERROR_NONE;
    }
    return API_ERROR_NOT_FOUND;
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
    json *elem = NULL;
    if (!JSON_HAS_TYPE(array, JSON_TYPE_ARRAY) 
        || !IDX_WITHIN_BOUNDS(array, idx))
    {
        return API_ERROR_INPUT_INVALID;
    }

    assert(type > JSON_TYPE_NONE || type < JSON_TYPE_END);

    // destroy original element and create a new one
    elem = array->elements[idx];
    json_destroy(elem); // TODO can we reuse memory instead of deallocating?
    elem = json_create(type);

    switch (type) 
    {
        case JSON_TYPE_NUMBER:
            elem->num_val = *(double *) val;
            break;
        case JSON_TYPE_BOOLEAN:
            elem->bool_val = *(bool *) val;
            break;
        case JSON_TYPE_STRING:
            elem->string_val = strdup((char *) val);
            elem->cnt = strlen(elem->string_val) + 1;
            break;
        default: // suppress warning
            break;
    }

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

    if (!JSON_HAS_TYPE(array, JSON_TYPE_ARRAY) 
        || !IS_PRIMITIVE_TYPE(type))
    {
        return;
    }

    // create value
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

    // reallocate memory
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
