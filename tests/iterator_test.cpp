#include "gtest/gtest.h"

extern "C" {
    #include "iterator.h"
}

void *memdup(const void *src, size_t n)
{
    void *dest = malloc(n);
    memcpy(dest, src, n);
    return dest;
}

TEST(json_nextTest, not_skipping_space)
{
    json_parser   *parser;
    unsigned char *test_string = (unsigned char *) "\"Hello World \"";
    size_t         ts_len = strlen((char *)test_string);

    parser = (json_parser *) calloc(1, sizeof(json_parser));

    parser->buffer = (unsigned char *) memdup(test_string, ts_len + 1);
    parser->buffer_sz = ts_len + 1;
    parser->skip_space = false;

    for (; *test_string; test_string++)
    {
        ASSERT_EQ(*test_string, json_next(parser));
    }
    
    // successive calls should return null code point
    ASSERT_EQ('\0', json_next(parser));
    ASSERT_EQ('\0', json_next(parser));

    free(parser->buffer);
    free(parser);
}


TEST(json_nextTest, skipping_space)
{
    json_parser   *parser;
    unsigned char *test_string = (unsigned char *) "\"Hello World \"";
    unsigned char *exp_string = (unsigned char *) "\"HelloWorld\"";
    size_t         ts_len = strlen((char *)test_string);

    parser = (json_parser *) calloc(1, sizeof(json_parser));
    parser->buffer = (unsigned char *) memdup(test_string, ts_len + 1);
    parser->buffer_sz = ts_len + 1;
    parser->skip_space = true;

    for (; *exp_string; exp_string++)
    {
        ASSERT_EQ(*exp_string, json_next(parser));
    }
    
    ASSERT_EQ('\0', json_next(parser));
    ASSERT_EQ('\0', json_next(parser));

    free(parser->buffer);
    free(parser);
}


TEST(is_string_matched, basic)
{
    json_parser    *parser;
    unsigned char  *test_string = (unsigned char *) "nulltruefalse";
    size_t          ts_len = strlen((char *) test_string);

    parser = (json_parser *) calloc(1, sizeof(json_parser));
    parser->buffer = (unsigned char *) memdup(test_string, ts_len + 1);
    parser->buffer_sz = ts_len + 1;

    ASSERT_EQ(true, is_string_matched(parser, (unsigned char *) "null"));
    ASSERT_EQ(true, is_string_matched(parser, (unsigned char *) "true"));
    ASSERT_EQ(false, is_string_matched(parser, (unsigned char *) "false!"));

    free(parser->buffer);
    free(parser);
}
