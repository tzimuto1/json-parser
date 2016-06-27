
/* 
 * Parse string to a json representation. The grammar used is as follows:

json := object | array
object = {} | { members }
members = pair | (pair , members)
pair = string : value

array = [] | [ elements ]
elements = (value) | (value , elements)
value = string | number | object | array | true | false | null

 * Source: json.org
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "parser.h"
#include "json.h"
#include "iterator.h"
#include "utils.h"


/* static function declarations */

static json *parse_object(json_parser *);
static json *parse_array(json_parser *);
static json *parse_string(json_parser *);
static json *parse_number(json_parser *);
static json *parse_boolean(json_parser *, bool);
static json *parse_null(json_parser *);
static obj_pair *parse_pair(json_parser *);
static char *parse_object_key(json_parser *);
static json *parse_value(json_parser *);

static void  json_parser_init(json_parser *parser, const char *json_string);
static void  json_parser_destroy(json_parser *);
static json_output *json_output_new();
void         json_output_destroy(json_output *jo);

static int   escaped_char2actual(char c);


/* UTILITIES */
#define parse_true(p)            parse_boolean(p, true)
#define parse_false(p)           parse_boolean(p, false)

#define CHAR2NUM(c)              ((c) - '0')
#define IS_CONTROL_CHAR(c)       ((c) < 32 ||  (c) == 127)

static int escaped_char2actual(char c)
{
    switch (c)
    {
        case '"':
        case '\\':
        case '/':
            break;
        case 'b':
            c = '\b';
            break;
        case 'f':
            c = '\f';
            break;
        case 'n':
            c = '\n';
            break;
        case 'r':
            c = '\r';
            break;
        case 't':
            c = '\t';
            break;
        case 'u': // TODO not currently supported
        default:
            c = -1;
            break;
    }
    return -1;
}


/* PARSER */


/*
 * Parse a value
 * grammar:
 *  value = string | number | object | array | true | false | null
*/
static json *parse_value(json_parser *parser)
{
    LOGFUNC();
    // The error treatment will be done by the corresponding parser_* functions

    json *value = NULL;
    char  c = json_peek(parser);

    switch (c)
    {
        case '{':
            value = parse_object(parser);
            break;
        case '[':
            value = parse_array(parser);
            break;
        case '"':
            value = parse_string(parser);
            break;
        case 't':
            value = parse_true(parser);
            break;
        case 'f':
            value = parse_false(parser);
            break;
        case 'n':
            value = parse_null(parser);
            break;
        default:
            if (c == '-' || isdigit(c))
                value = parse_number(parser);
            else
                parser->error = JSON_ERROR_INVALID_JSON;
            break;      
    }

    return value;
}


/*
 * Parse a null value
 * grammar:
 *  null = null (that's so meta!)
*/
static json *parse_null(json_parser *parser)
{
    LOGFUNC();
    if (is_string_matched(parser, "null"))
    {
        json *null_obj = json_create(JSON_TYPE_BOOLEAN);
        return null_obj;
    }

    parser->error = JSON_ERROR_INVALID_JSON;
    return NULL;
}


/*
 * Parse a boolean of value bool_val
 * grammar:
 *  boolean = true | false
*/

static json *parse_boolean(json_parser *parser, bool bool_val)
{
    LOGFUNC();
    char *bool_str = (bool_val) ? "true" : "false";
    json *bool_obj = NULL;
    if (is_string_matched(parser, bool_str))
    {
        bool_obj = json_create(JSON_TYPE_BOOLEAN);
        bool_obj->bool_val = bool_val;
        return bool_obj;
    }

    parser->error = JSON_ERROR_INVALID_JSON;
    return NULL;
}


/*
 * Parse an integer e.g 3.14E-314, 3e+10, 0.300
 * grammar 
    (adapted from https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/JSON):
 * number          = - positive_number | positive_number
 * positive_number = decimal_number | decimal_number . digits |
                     decimal_number . digits exponent_part | decimal_number exponent_part
 * decimal_number  = 0 | one-nine digits
 * exponent_part   = e exponent | E exponent
 * exponent        = digits | + digits | - digits
 * digits          = digit | digits digit
 * digit           = 0->9
 * one-nine        = 1->9
 *
 * A more simplified approach representation that abracts the grammar details is as follows:
 * [<sign_part>] (no_leading_zero)<integer_part> [<decimal_point> <fraction_part>] 
 * [<E/e_symbol> [<sign_part>] <exponent_part>]
 */
static json *parse_number(json_parser *parser)
{
    LOGFUNC();
    char    c;
    int     base_sign = 1;
    int     exp_sign = 1;
    double  num_value = 0;
    double  exponent = 0;
    json   *number = json_create(JSON_TYPE_NUMBER);

    /* process sign of number */
    if (json_peek(parser) == '-')
    {
        base_sign = -1;
        json_next(parser);
    }

    parser->skip_space = false;

    /* process integer part */
    if (!isdigit(json_peek(parser)))
    {
        parser->error = JSON_ERROR_INVALID_NUM_FORMAT;
        goto ERROR;
    }

    c = json_next(parser);

    // make sure that there are no leading zeros
    if (c == '0' && json_peek(parser) != '.')
    {
        parser->error = JSON_ERROR_INVALID_NUM_FORMAT;
        goto ERROR;
    }

    num_value = num_value * 10  + CHAR2NUM(c);

    while (isdigit(json_peek(parser)))
    {
        c = json_next(parser);
        num_value = num_value * 10  + CHAR2NUM(c);
    }

    /* process fraction part */
    if (json_peek(parser) == '.')
    {
        json_next(parser);

        double coeff = .1;
        while (isdigit(json_peek(parser)))
        {
            c = json_next(parser);
            num_value = num_value + coeff * CHAR2NUM(c);
            coeff *= .1;
        }
    }

    /* process the exponent part */
    c = json_peek(parser);
    if (c == 'E' || c == 'e') {
        json_next(parser);

        c = json_peek(parser);
        // deal with the sign and consume it
        if (c == '-')
        {
            json_next(parser);
            exp_sign = -1;
        }
        else if (c == '+')
        {
            json_next(parser);
            exp_sign = 1; // unnecessary, really, just for uniformity sake
        }

        if (!isdigit(json_peek(parser)))
        {
            parser->error = JSON_ERROR_INVALID_NUM_FORMAT;
            goto ERROR;
        }

        while (isdigit(json_peek(parser)))
        {
            c = json_next(parser);
            exponent = exponent * 10 + CHAR2NUM(c);
        }
    }

    parser->skip_space = true;
    /* now wrap up everything */
    number->num_val = num_value * base_sign * pow(10, exp_sign * exponent);
    return number;

ERROR:
    json_destroy(number);
    return NULL;
}


/*
 * parse a string
 * grammar:
 *  string = "" | "chars"
 *  chars  = (char) | (char chars)
 *  char   = any unicode except " or \ or control character
 *           |
 *           \" | \\ | \/ | \b | \f | \n | \r | \t | \u four-hex-digits
 *  control_character = charatcer from 0-32 and 127
 * TODO currently supposet 0-127
m*/
static json *parse_string(json_parser *parser)
{
    LOGFUNC();
    json *string = NULL;
    if (json_next(parser) == '"')
    {
        char c;

        parser->skip_space = false;
        string = json_create(JSON_TYPE_STRING);

        if (json_peek(parser) == '"')
        {
            json_next(parser);
            parser->skip_space = true;
            return string;
        }

        while ((c = json_next(parser)) != '"' && !IS_CONTROL_CHAR(c))
        {
            if (string->cnt >= string->alloced - 1 )
            {
                string->alloced += 10;
                string->string_val = (char *) realloc(string->string_val, 
                    sizeof(char) * string->alloced);
            }

            if (c == '\\')
            {
                if ((c = escaped_char2actual(json_next(parser))) < 0)
                    goto ERROR;
            }

            string->string_val[string->cnt++] = c;
        }

        string->string_val[string->cnt++] = '\0'; // is null termination necessary?

        if (c != '"')
        {
            parser->error = JSON_ERROR_UNBALANCED_QUOTE;
            goto ERROR;
        }
        
        parser->skip_space = true;
        return string;
    }
    else 
    {
        parser->error = JSON_ERROR_INVALID_JSON;
    }

ERROR:
    json_destroy(string);
    return NULL;
}


/*
 * parser an array
 * grammar:
 *  array = [] | [ value (, value)... ]
 */
static json *parse_array(json_parser *parser)
{
    LOGFUNC();
    json *array = NULL;
    if (json_next(parser) == '[')
    {
        json *value = NULL;
        char  c;

        array = json_create(JSON_TYPE_ARRAY);

        if (json_peek(parser) == ']')
        {
            json_next(parser);
            return array;
        }

        do {
            if (array->cnt == array->alloced)
            {
                array->alloced += 10;
                array->elements = (json **) realloc(array->elements, 
                    sizeof(json *) * array->alloced);
            }

            if (!(value = parse_value(parser)))
                goto ERROR;

            array->elements[array->cnt++] = value;
        } while ((c = json_next(parser)) == ',');

        if (c != ']')
        {
            parser->error = JSON_ERROR_UNBALANCED_SQUARE_BRACKET;
            goto ERROR;
        }
        
        return array;
    }
    else
    {
        parser->error = JSON_ERROR_INVALID_JSON;
    }

ERROR:
    json_destroy(array);
    return NULL;
}

// helper to parse_pair
static char *parse_object_key(json_parser *parser)
{
    LOGFUNC();
    char *key    = NULL;
    json *string = parse_string(parser);

    if (string)
        key = strdup(string->string_val);

    json_destroy(string);
    return key;
}

/*
 * parse an object pair
 * grammar:
 *   pair = string : value
 */
static obj_pair *parse_pair(json_parser *parser)
{
    LOGFUNC();
    obj_pair *pair = NULL;
    json     *value = NULL;
    char     *key = NULL;

    pair = (obj_pair *) calloc(1, sizeof(obj_pair));
    
    if (!(key = parse_object_key(parser)))
        goto ERROR;

    if (json_next(parser) == ':')
    {
        if (!(value = parse_value(parser)))
            goto ERROR;

        pair->key = key;
        pair->value = value;

        return pair;
    }
    else
    {
        parser->error = JSON_ERROR_INVALID_JSON;
    }   

ERROR:
    free(pair);
    free(key);
    json_destroy(value);
    return NULL;
}


/*
 * parse an object
 * grammar:
 *  object = {} | { pair (, pair) }
 */
static json *parse_object(json_parser *parser)
{
    LOGFUNC();
    json *object = NULL;
    if (json_next(parser) == '{')
    {
        obj_pair *pair = NULL;
        char      c;

        object = json_create(JSON_TYPE_OBJECT);

        if (json_peek(parser) == '}')
        {
            json_next(parser);
            return object;
        }

        do {
            if (object->cnt == object->alloced)
            {
                object->alloced += 10;
                object->members = (obj_pair **) realloc(object->members, 
                    sizeof(obj_pair *) * object->alloced);
            }

            if (!(pair = parse_pair(parser)))
                goto ERROR;

            object->members[object->cnt++] = pair;
        } while ((c = json_next(parser)) == ',');

        if (c != '}')
        {
            parser->error = JSON_ERROR_UNBALANCED_BRACE;
            goto ERROR;
        }

        return object;
    }
    else
    {
        parser->error = JSON_ERROR_INVALID_JSON;
    }

ERROR:
    json_destroy(object);
    return NULL;
}

/*
* Parse json_string to a json tree
* GRAMMAR: json := object | array
*/
json_output *json_parse(const char *json_string)
{
    json_parser   parser;
    json_output  *output;

    output = json_output_new();

    if (!json_string)
        return output;

    //PRINTLN("Parsing string: %s", json_string);

    json_parser_init(&parser, json_string);

    switch (json_peek(&parser))
    {
        case '{':
            output->root  = parse_object(&parser);
            output->error = parser.error;
            break;
        case '[':
            output->root  = parse_array(&parser);
            output->error = parser.error;
            break;
        default:
            output->error = JSON_ERROR_INVALID_JSON;
            break;
    }
    json_parser_destroy(&parser);
    return output;
}


static void json_parser_init(json_parser *parser, const char *json_string)
{
    parser->buffer = strdup(json_string);
    parser->buffer_sz = strlen(json_string) + 1;
    parser->buffer_idx = 0;
    parser->skip_space = true;
    parser->error = 0;
}


static void json_parser_destroy(json_parser *parser)
{
    free(parser->buffer);
}


static json_output *json_output_new()
{
    json_output *output;

    output = (json_output *) calloc(1, sizeof(json_output));
    output->root  = NULL;
    output->error = 0;
    return output;
}


void json_output_destroy(json_output *jo)
{
    json_destroy(jo->root);
    free(jo);
}

#ifndef TEST_MODE
int main(int argc, char *argv[])
{
    LOGFUNC();
    json_output *jo = json_parse("{\"msg\":\"Hello, World\"}");
    if (jo->error)
    {
        printf("Error happened : %d\n", jo->error);
    }
    else 
    {
        json *r = jo->root;
        if (!r)
        {
            printf("NULL ROOT\n");
        }
        else
        {
            printf("JSON VALUE OF TYPE %d\n", r->type);
        }
    }
    return 0;
}
#endif // TEST_MODE
