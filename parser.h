#ifndef PARSER_H
#define PARSER_H

#include "json.h"

typedef enum json_error
{
    // the input string should be defined
    JSON_ERROR_UNDEFINED_INPUT,
    JSON_ERROR_IVALID_JSON, // quite generic
    JSON_ERROR_UNBALANCED_QOUTE,
    JSON_ERROR_UNBALANCED_BRACE,
    JSON_ERROR_UNBALANCED_SQUARE_BRACKET
    JSON_ERROR_MISSING_OBJ_COLON,
    JSON_ERROR_INVALID_NUM_FORMAT,
    ERROR_MEMORY,
} json_error;

/* the parser output returned to the user */
typedef struct 
{
    json *root;
    int   error;
} json_output;

/* parser object */
typedef struct json_parser 
{
    // buffer specific fields
    char         *buffer;
    int           buffer_sz;
    int           buffer_idx;

    json_output  *output;
    int           parsing_string;
    int           error;
} json_parser;


json_output *json_parse(const char *json_string);

#endif // PARSER_H

