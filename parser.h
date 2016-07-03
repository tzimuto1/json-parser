#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "json.h"

typedef enum json_error
{
    // the input string should be defined
    JSON_ERROR_NONE,
    JSON_ERROR_INVALID_JSON, // quite generic
    JSON_ERROR_UNBALANCED_BRACE,
    JSON_ERROR_MISSING_OBJ_COLON,
    JSON_ERROR_INVALID_STRING, // generic string error (unidentified maybe)
    JSON_ERROR_UNBALANCED_SQUARE_BRACKET,
    JSON_ERROR_UNBALANCED_QUOTE,
    JSON_ERROR_INVALID_ESCAPE_SEQUENCE,
    JSON_ERROR_STRING_HAS_CONTROL_CHAR,
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
    bool          skip_space; // TODO don't like the design for this
    int           error;
} json_parser;


json_output *json_parse(const char *json_string);
void         json_output_destroy(json_output *jo);

#endif // PARSER_H

