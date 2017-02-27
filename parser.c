
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

#include "json.h"
#include "parser.h"
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
static unsigned char *parse_object_key(json_parser *);
static json *parse_value(json_parser *);

static void  json_parser_init(json_parser *parser, const char *json_string);
static void  json_parser_destroy(json_parser *);
static json_output *json_output_new();
static void  arr_realloc(json *);
void         json_output_destroy(json_output *jo);

static int32_t escaped_chars2actual(json_parser *parser);
static int32_t char2hex(int32_t c);


/* UTILITIES */
#define parse_true(p)            parse_boolean(p, true)
#define parse_false(p)           parse_boolean(p, false)

#define CHAR2NUM(c)              ((c) - '0')
#define IS_CONTROL_CHAR(c)       ((c) < 32)

/*
 The macro sets the error if it has not been set. In some cases we just set a 
 generic error, JSON_ERROR_INVALID_JSON is we can't determine the error.
*/
#define SET_PARSER_ERROR(p, e)   p->error = p->error ? p->error : e


static int32_t char2hex(int32_t c)
{
    if (isdigit(c))
    {
        return CHAR2NUM(c);
    }

    switch (c)
    {
        case 'A': case 'a':
            c = 0xA;
            break;
        case 'B': case 'b':
            c = 0xB;
            break;
        case 'C': case 'c':
            c = 0xC;
            break;
        case 'D': case 'd':
            c = 0xD;
            break;
        case 'E': case 'e':
            c = 0xE;
            break;
        case 'F': case 'f':
            c = 0xF;
            break;
        default:
            c = -1;
            break;
    }
    return c;
}

static int32_t escaped_chars2actual(json_parser *parser)
{
    int32_t c = json_next(parser);

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
        case 'u':
        {
            int     i;
            int32_t c2 = 0;

            for (i = 0; i < 4; i++)
            {
                c = char2hex(json_next(parser));

                if (c < 0)
                {
                    SET_PARSER_ERROR(parser, 
                        JSON_ERROR_INVALID_UNICODE_ESCAPE_SEQUENCE);
                    return -1;
                }

                c2 += (c << (12 - 4 * i));
            }

            if (!utf8is_codepoint_valid(c2))
            {
                SET_PARSER_ERROR(parser, 
                    JSON_ERROR_INVALID_UNICODE_ESCAPE_SEQUENCE);
                return -1;
            }

            c = c2;
        }
            break;
        default:
            SET_PARSER_ERROR(parser, JSON_ERROR_INVALID_ESCAPE_SEQUENCE);
            return -1;
    }

    return c;
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

    json    *value = NULL;
    int32_t  c = json_peek(parser);

    parser->depth++;
    if (parser->depth > JSON_PARSER_MAX_DEPTH)
    {
        SET_PARSER_ERROR(parser, JSON_ERROR_PARSER_MAX_DEPTH_EXCEEDED);
        return NULL;
    }

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
        case '\0':
            value = NULL;
            break;
        default:
            if (c == '-' || isdigit(c))
                value = parse_number(parser);
            else
                SET_PARSER_ERROR(parser, JSON_ERROR_INVALID_JSON);
            break;      
    }

    parser->depth--;
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
    if (is_string_matched(parser, (unsigned char *) "null"))
    {
        json *null_obj = json_create(JSON_TYPE_NULL);
        return null_obj;
    }

    SET_PARSER_ERROR(parser, JSON_ERROR_INVALID_JSON);
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
    const char *bool_str = (bool_val) ? "true" : "false";
    json *bool_obj = NULL;
    if (is_string_matched(parser, (unsigned char *)bool_str))
    {
        bool_obj = json_create(JSON_TYPE_BOOLEAN);
        bool_obj->bool_val = bool_val;
        return bool_obj;
    }

    SET_PARSER_ERROR(parser, JSON_ERROR_INVALID_JSON);
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
    int32_t c;
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
        SET_PARSER_ERROR(parser, JSON_ERROR_INVALID_NUM_FORMAT);
        goto ERROR;
    }

    c = json_next(parser);

    // make sure that there are no leading zeros
    if (c == '0' && isdigit(json_peek(parser)))
    {
        SET_PARSER_ERROR(parser, JSON_ERROR_INVALID_NUM_FORMAT);
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

        // decimal point MUST be followed by a digit
        if (!isdigit(json_peek(parser)))
        {
            SET_PARSER_ERROR(parser, JSON_ERROR_INVALID_NUM_FORMAT);
            goto ERROR;
        }

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
            SET_PARSER_ERROR(parser, JSON_ERROR_INVALID_NUM_FORMAT);
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
 * TODO currently supports 0-127
m*/
static json *parse_string(json_parser *parser)
{
    LOGFUNC();
    json *string = NULL;
    if (json_next(parser) == '"')
    {
        int32_t c;

        parser->skip_space = false;
        string = json_create(JSON_TYPE_STRING);

        while ((c = json_next(parser)) != '"' && c != -1 && !IS_CONTROL_CHAR(c))
        {
            arr_realloc(string);

            if (c == '\\')
            {
                if ((c = escaped_chars2actual(parser)) < 0)
                {
                    SET_PARSER_ERROR(parser, JSON_ERROR_INVALID_JSON);
                    goto ERROR;
                }
            }

            string->cnt += utf8encode(c, string->string_val + string->cnt);
        }

        if (c != '"')
        {
            if (c == '\0')
                SET_PARSER_ERROR(parser, JSON_ERROR_UNBALANCED_QUOTE);
            else if (IS_CONTROL_CHAR(c))
                SET_PARSER_ERROR(parser, JSON_ERROR_STRING_HAS_CONTROL_CHAR);
            else if (c == -1)
            { /* do nothing */ }
            else
                SET_PARSER_ERROR(parser, JSON_ERROR_INVALID_STRING);
            goto ERROR;
        }
        
        string->string_val[string->cnt] = '\0';
        parser->skip_space = true;
        return string;
    }
    else 
    {
        SET_PARSER_ERROR(parser, JSON_ERROR_INVALID_JSON);
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
        json    *value = NULL;
        int32_t  c;

        array = json_create(JSON_TYPE_ARRAY);

        if (json_peek(parser) == ']')
        {
            json_next(parser);
            return array;
        }

        if (json_peek(parser) == '\0')
        {
            SET_PARSER_ERROR(parser, JSON_ERROR_UNBALANCED_SQUARE_BRACKET);
            goto ERROR;
        }

        do {
            arr_realloc(array);

            if (!(value = parse_value(parser)))
            {
                SET_PARSER_ERROR(parser, JSON_ERROR_INVALID_JSON);
                goto ERROR;
            }

            array->elements[array->cnt++] = value;
        } while ((c = json_next(parser)) == ',');

        if (c != ']')
        {
            SET_PARSER_ERROR(parser, JSON_ERROR_UNBALANCED_SQUARE_BRACKET);
            goto ERROR;
        }
        
        return array;
    }
    else
    {
        SET_PARSER_ERROR(parser, JSON_ERROR_INVALID_JSON);
    }

ERROR:
    json_destroy(array);
    return NULL;
}

// helper to parse_pair
static unsigned char *parse_object_key(json_parser *parser)
{
    LOGFUNC();
    unsigned char *key = NULL;
    json *string = parse_string(parser);

    if (string)
    {
        key = (unsigned char *) strdup((char *) string->string_val);
    }

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
    unsigned char *key = NULL;

    pair = (obj_pair *) calloc(1, sizeof(obj_pair));

    if (!(key = parse_object_key(parser)))
        goto ERROR;

    if (json_next(parser) == ':')
    {
        if (!(value = parse_value(parser)))
        {
            SET_PARSER_ERROR(parser, JSON_ERROR_INVALID_JSON);
            goto ERROR;
        }

        pair->key = key;
        pair->value = value;

        return pair;
    }
    else
    {
        SET_PARSER_ERROR(parser, JSON_ERROR_INVALID_JSON);
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
        int32_t   c;

        object = json_create(JSON_TYPE_OBJECT);

        if (json_peek(parser) == '}')
        {
            json_next(parser);
            return object;
        }

        do {
            arr_realloc(object);

            if (!(pair = parse_pair(parser)))
            {
                SET_PARSER_ERROR(parser, JSON_ERROR_INVALID_JSON);
                goto ERROR;
            }

            object->members[object->cnt++] = pair;
        } while ((c = json_next(parser)) == ',');

        if (c != '}')
        {
            SET_PARSER_ERROR(parser, JSON_ERROR_UNBALANCED_BRACE);
            goto ERROR;
        }

        return object;
    }
    else
    {
        SET_PARSER_ERROR(parser, JSON_ERROR_INVALID_JSON);
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
    LOGFUNC();
    json_parser   parser;
    json_output  *output;

    output = json_output_new();

    // error on null string since it's not valid JSON
    if (!json_string)
    {
        output->error = JSON_ERROR_EMPTY_INPUT;
        return output;
    }

    json_parser_init(&parser, json_string);

    // error on 'empty' input since it's not valid JSON
    if (json_peek(&parser) == '\0')
    {
        output->error = JSON_ERROR_EMPTY_INPUT;
        return output;
    }

    output->root = parse_value(&parser);
    output->error = parser.error;
    output->buffer_idx = parser.buffer_idx;

    // unhandled error cases
    if (output->error == JSON_ERROR_NONE)
    {
        if (json_peek(&parser) != '\0' || !output->root)
        {
            json_destroy(output->root);
            output->root = NULL;
            output->error = JSON_ERROR_INVALID_JSON;
        }
    }
    else
    {
        json_destroy(output->root);
        output->root = NULL;
    }

    json_parser_destroy(&parser);
    return output;
}


static void json_parser_init(json_parser *parser, const char *json_string)
{
    parser->buffer = (unsigned char *) strdup(json_string);
    parser->buffer_sz = strlen(json_string) + 1;
    parser->buffer_idx = 0;
    parser->skip_space = true;
    parser->error = 0;
    parser->depth = 0;
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
    output->buffer_idx = 0;
    return output;
}


/*
 * Reallocate the C arrays used within the json structures 
 */
static void arr_realloc(json *js)
{
    /*
    For strings we should have enough space for terminal byte and 4 unicode bytes
    */
    if ((JSON_IS_STRING(js) 
        && (js->cnt > js->alloced - BYTES_PER_UNICODE_CHAR - 1)) 
        || (!JSON_IS_STRING(js) 
            && (js->cnt == js->alloced)))
    {
        js->alloced += 10;

        switch (js->type)
        {
            case JSON_TYPE_OBJECT:
                js->members = (obj_pair **) realloc(js->members, 
                    sizeof(obj_pair *) * js->alloced);
                break;
            case JSON_TYPE_ARRAY:
                js->elements = (json **) realloc(js->elements, 
                    sizeof(json *) * js->alloced);
                break;
            case JSON_TYPE_STRING:
                js->string_val = (unsigned char *) realloc(js->string_val, 
                    sizeof(unsigned char) * js->alloced);
            default:
                break;
        }
    }
}

/*
 * Feee memory within json_output
 */
void json_output_destroy(json_output *jo)
{
    json_destroy(jo->root);
    free(jo);
}

/*
 * Return true if a parsing error happened
 */
bool json_parser_found_error(json_output *jo)
{
    return jo->error != 0;
}

int json_parser_get_error_loc(json_output *jo)
{
    return jo->error ? jo->buffer_idx : 0;
}

const char *json_parser_get_error(json_output *jo)
{
    if (jo->error < 0)
    {
        return utf8proc_errmsg(jo->error);
    }

    switch (jo->error) 
    {
        case JSON_ERROR_NONE:
            return "No error";
        case JSON_ERROR_EMPTY_INPUT:
            return "Empty or null input";
        case JSON_ERROR_INVALID_JSON:
            return "JSON is invalid";
        case JSON_ERROR_UNBALANCED_BRACE:
            return "Unbalanced brace";
        case JSON_ERROR_MISSING_OBJ_COLON:
            return "Missing object colon";
        case JSON_ERROR_INVALID_STRING:
            return "Invalid string";
        case JSON_ERROR_UNBALANCED_SQUARE_BRACKET:
            return "Unbalanced square bracket";
        case JSON_ERROR_UNBALANCED_QUOTE:
            return "Unbalanced string quote";
        case JSON_ERROR_INVALID_ESCAPE_SEQUENCE:
            return "Invalid escape sequence";
        case JSON_ERROR_INVALID_UNICODE_ESCAPE_SEQUENCE:
            return "Invalid unicode escape sequence";
        case JSON_ERROR_STRING_HAS_CONTROL_CHAR:
            return "String has unescaped control character";
        case JSON_ERROR_INVALID_NUM_FORMAT:
            return "Invalid number format";
        case JSON_ERROR_PARSER_MAX_DEPTH_EXCEEDED:
            return "Parser max depth exceeded";
        case JSON_ERROR_ILLEGAL_CHARACTER:
            return "Illegal character encountered";
        default:
            return "Unknown error happened in the parser";
    }
}
