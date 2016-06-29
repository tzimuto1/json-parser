#ifndef ITERATOR_H
#define ITERATOR_H

#include "parser.h"

char json_peek(json_parser *parser);
char json_next(json_parser *parser);
bool is_string_matched(json_parser *parser, const char *str);

#endif // ITERATOR_H
