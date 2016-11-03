#ifndef ITERATOR_H
#define ITERATOR_H

#include "utf8proc/utf8proc.h"
#include "parser.h"

#define BYTES_PER_UNICODE_CHAR    4

int32_t json_peek(json_parser *parser);
int32_t json_next(json_parser *parser);
bool    is_string_matched(json_parser *parser, const uint8_t *str);
ssize_t utf8encode(int32_t code_point, uint8_t *dst);
bool    utf8is_codepoint_valid(int32_t code_point);

#endif // ITERATOR_H
