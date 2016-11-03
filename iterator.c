/* ITERATOR */

#include "parser.h"
#include "iterator.h"

#define CHAR_IS_WHITESPACE(c)    ((c) == '\t' || (c) == '\n' || \
                                  (c) == '\r' || (c) == ' ')

/*
 * Advance forward until the next unconsumed character is no stace except when
 * parsing strings
 */
static void skip_space(json_parser *parser)
{
    if (parser->skip_space)
    {
        while (parser->buffer_idx < parser->buffer_sz)
        {
            uint8_t c = parser->buffer[parser->buffer_idx];
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
int32_t json_peek(json_parser *parser)
{
    skip_space(parser);

    utf8proc_int32_t code_point = 0;
    utf8proc_ssize_t read;
    
    if (parser->buffer_idx >= parser->buffer_sz || parser->error)
    {
        return 0;
    }

    read = utf8proc_iterate(parser->buffer + parser->buffer_idx,
        BYTES_PER_UNICODE_CHAR, &code_point);

    if (read < 0) 
    {
        parser->error = read;
    }

    return code_point;
}


/*
 * Return the next unconsumed character and advance
 */
int32_t json_next(json_parser *parser)
{
    skip_space(parser);

    utf8proc_int32_t code_point = 0;
    utf8proc_ssize_t read;

    if (parser->buffer_idx >= parser->buffer_sz || parser->error)
    {
        return 0;
    }

    read = utf8proc_iterate(parser->buffer + parser->buffer_idx, 
        BYTES_PER_UNICODE_CHAR, &code_point);

    if (read < 0) 
    {
        parser->error = read;
    }
    else 
    {
        parser->buffer_idx += read;
    }

    return code_point;
}

bool is_string_matched(json_parser *parser, const unsigned char *str)
{
    /* Only works with ASCII strings */

    bool matched = true;
    parser->skip_space = false;
    for (; *str; str++)
    {
        if (*str != json_next(parser))
        {
            matched = false;
            break;
        }
    }
    parser->skip_space = true;
    return matched;
}


ssize_t utf8encode(int32_t code_point, uint8_t *dst)
{
    return utf8proc_encode_char(code_point, dst);
}

bool utf8is_codepoint_valid(int32_t code_point)
{
    return utf8proc_codepoint_valid(code_point);
}
