#include "gtest/gtest.h"
#include "parser.h"
#include "iterator.h"

TEST(json_nextTest, not_skipping_space)
{
    json_parser *parser;
    char        *test_string = (char *) "\"Hello World \"";

    parser = (json_parser *) calloc(1, sizeof(json_parser));
    parser->buffer = strdup(test_string);
    parser->buffer_sz = strlen(test_string) + 1;
    parser->skip_space = false;

    for (; *test_string; test_string++)
    {
        ASSERT_EQ(*test_string, json_next(parser));
    }
    
    ASSERT_EQ('\0', json_next(parser));
}


TEST(json_nextTest, skipping_space)
{
    json_parser *parser;
    char        *test_string = (char *) "\"Hello World \"";
    char        *exp_string = (char *) "\"HelloWorld\"";

    parser = (json_parser *) calloc(1, sizeof(json_parser));
    parser->buffer = strdup(test_string);
    parser->buffer_sz = strlen(test_string) + 1;
    parser->skip_space = true;

    for (; *exp_string; exp_string++)
    {
        ASSERT_EQ(*exp_string, json_next(parser));
    }
    
    ASSERT_EQ('\0', json_next(parser));
}


TEST(is_string_matched, basic)
{
    json_parser *parser;
    char        *test_string = (char *) "nulltruefalse";

    parser = (json_parser *) calloc(1, sizeof(json_parser));
    parser->buffer = strdup(test_string);
    parser->buffer_sz = strlen(test_string) + 1;

    ASSERT_EQ(true, is_string_matched(parser, (char *) "null"));
    ASSERT_EQ(true, is_string_matched(parser, (char *) "true"));
    ASSERT_EQ(false, is_string_matched(parser, (char *) "false!"));
}
