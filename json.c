
/*
 * JSON "object" methods
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "json.h"

/* ========== BASIC METHODS ========== */

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
            int        i;
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
 * Return true if json object has the number value
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

// /*
//  * Return all they keys in the object
//  */
// char **json_object_get_all_keys(json *object)
// {
//     int    i = 0;
//     int    alloced = 10;
//     char **keys = NULL;

//     if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT))
//         return NULL;

//     keys = (char **) calloc(alloced, sizeof(char *));

//     for (i = 0; i < object->cnt; i++)
//     {
//         if (i == alloced)
//         {
//             alloced += 10;
//             keys = (char **) realloc(keys, alloced * sizeof(char *));
//         }

//         keys[i] = object->members[i]->key;
//     }
//     return keys;
// }


// /*
//  * Get the size of the object
//  */
// int json_object_get_size(json *object)
// {
//     if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT))
//         return 0;

//     return object->cnt;
// }


// /*
//  * Determine if an object is empty or not
//  */
// bool json_object_is_empty(json *object)
// {
//     return json_object_get_size(object) == 0;
// }


/*
 * Put a number in an object
 */
  // TODO: should call a generic function
int json_object_put_number(json *object, const char *key, double number)
{
    int       i = 0;
    obj_pair *pair = NULL;

    if (!JSON_HAS_TYPE(object, JSON_TYPE_OBJECT))
        return API_ERROR_NOT_OBJECT;

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

