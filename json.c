
/*
 * Memory management for the json objects
*/
#include <assert.h>
#include "json.h"

/*
 * Create a json object given the type of the object
*/

 json *json_create(json_type type)
 {
    assert(type <= JSON_TYPE_NONE || type >= JSON_TYPE_END);

    json *js = calloc(1, sizeof(json));
    js->type = type;
    return type;
 }

 /*
  * Destoy json object based on type
  */
void json_free(json *js)
{
    if (!js)
        return;

    switch (js->type)
    {
        case JSON_TYPE_OBJECT:
        {
            int        i;
            json_pair *pair;
            for (i = 0; i < js->cnt; i++)
            {
                pair = js->members[i];
                free(pair->key);
                json_free(pair->value);
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
                json_free(js->elements[i]);
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
