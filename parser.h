#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "json.h"

#define JSON_PARSER_MAX_DEPTH      512

typedef enum json_error
{
    // the input string should be defined
    JSON_ERROR_NONE,
    JSON_ERROR_EMPTY_INPUT,
    JSON_ERROR_INVALID_JSON, // quite generic
    JSON_ERROR_UNBALANCED_BRACE,
    JSON_ERROR_MISSING_OBJ_COLON,
    JSON_ERROR_INVALID_STRING, // generic string error (unidentified maybe)
    JSON_ERROR_UNBALANCED_SQUARE_BRACKET,
    JSON_ERROR_UNBALANCED_QUOTE,
    JSON_ERROR_INVALID_ESCAPE_SEQUENCE,
    JSON_ERROR_INVALID_UNICODE_ESCAPE_SEQUENCE,
    JSON_ERROR_STRING_HAS_CONTROL_CHAR,
    JSON_ERROR_INVALID_NUM_FORMAT,
    JSON_ERROR_PARSER_MAX_DEPTH_EXCEEDED,
    JSON_ERROR_ILLEGAL_CHARACTER,
    ERROR_MEMORY,
} json_error;

/* parser object */
typedef struct json_parser 
{
    // buffer specific fields
    unsigned char *buffer;
    int           buffer_sz;
    int           buffer_idx;

    json_output  *output;
    bool          skip_space; // TODO don't like the design for this
    int           error;
    int           depth;
} json_parser;

#endif // PARSER_H

