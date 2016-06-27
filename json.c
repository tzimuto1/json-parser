
/*
 * Memory management for the json objects
*/

#include <stdlib.h>
#include <assert.h>
#include "json.h"

/*
 * Create a json object given the type of the object
*/

json *json_create(json_type type)
{
    assert(type > JSON_TYPE_NONE || type < JSON_TYPE_END);

    json *js = calloc(1, sizeof(json));
    js->type = type;
    return js;
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
            obj_pair  *pair;
            for (i = 0; i < js->cnt; i++)
            {
                pair = js->members[i];
                free(pair->key);
                json_destroy(pair->value);
                free(pair);
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
