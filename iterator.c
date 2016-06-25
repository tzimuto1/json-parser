/* ITERATOR */

#include "parser.h"

/*
 * Advance forward until the next unconsumed character is no stace except when
 * parsing strings
 */

static void skip_space(json_parser *parser)
{
    if (!parser->parsing_string)
    {
        while (parser->buffer_idx < parser->buffer_sz)
        {
            char c = parser->buffer[parser->buffer_idx];
            if (!CHAR_IS_WHITESPACE(c))
                return;
            parser->buffer_idx++;
        }
    }
}


/*
 * Return the next unconsumed character. Only if the unconsumed character in space
 * in certain situations
 */

char json_peek(json_parser *parser)
{
    skip_space(parser);
    return (parser->buffer_idx < parser->buffer_sz)
        ? parser->buffer[parser->buffer_idx] : '\0';
}


/*
 * Return the next unconsumed character and advance
 */

char json_next(json_parser *parser)
{
    skip_space(parser);
    return (parser->buffer_idx < parser->buffer_sz) 
        ? parser->buffer[parser->buffer_idx++] : '\0';
}

bool is_string_matched(json_parser *parser, char *str)
{
    for (; *str; str++)
    {
        if (*str != json_next(parser))
            return false;
    }
    return true;
}
