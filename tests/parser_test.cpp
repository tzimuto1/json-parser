#include "gtest/gtest.h"
#include "parser.h"

// TODO we need a way to get rid of the repeated call to json_output_destroy

/*
 * List of tests
 * INCORRECT STARTING INPUT
 *  - [DONE]NULL input
 *  - [DONE]empty input
 *  - [DONE]primitives: number, boolean, string, null
 * NUMBER TESTS
 *  - [DONE]
 * STRING TESTS
 *  - simple string
 *  - strings with control characters
 * ARRAY
 *  - [DONE]empty array
 *  - [DONE]simple array with number, string, null, true, false
 *  - [DONE]complex array with nested arrays
 *  - [DONE]incorrect array: missing square bracket
 *  - [DONE]incorrect array: missing comma
 *  - [DONE]incorrect array: trailing input
 *  - incorrect array: recursion depth exceeded
 * OBJECT
 *  - [DONE]empty object
 *  - [DONE]simple object
 *  - [DONE]nested object
 *  - [DONE]incorrect object: missing brace
 *  - [DONE]incorrect array: missing comm
 *  - [DONE]incorrect object: key not a string
 *  - [DONE]incorrect object: trailing input
 *  - incorrect object: recursion depth exceeded
 * HYBRID
 *  - [DONE]nested
 *  - e.g incorrect array followed by invalid json input
 *  - nested incorrect: recursion depth exceeded
 */


/* BASIC */
TEST(parserTest, null_input)
{
    const char  *json_str = NULL;
    json_output *output = json_parse(json_str);

    ASSERT_NE((json_output *) NULL, output); // TODO find why we need the cast
    ASSERT_EQ(NULL, output->root);
    ASSERT_EQ(0, output->error);

    json_output_destroy(output);
}


TEST(parserTest, empty_input)
{
    const char  *json_str = "    ";
    json_output *output = json_parse(json_str);

    ASSERT_NE((json_output *) NULL, output);
    ASSERT_EQ(NULL, output->root);
    ASSERT_EQ(JSON_ERROR_NONE, output->error);

    json_output_destroy(output);
}


TEST(parserTest, general_primitives)
{
    int          i = 0;
    const char  *primitives[] = { "3.14", "true", "null", "\"json\"" };
    json         results[] = {
        { JSON_TYPE_NUMBER, 0, 0, { .num_val = 3.14} },
        { JSON_TYPE_BOOLEAN, 0, 0, { .bool_val = true } },
        { JSON_TYPE_NULL, 0, 0, { .string_val = NULL } },
        { JSON_TYPE_STRING, 0, 0, { .string_val = (char *)"json" } },
    };
    int          num_prims = sizeof(primitives) / sizeof(char *);
    json_output *output = NULL;

    for ( i = 0; i < num_prims; i++)
    {
        json res = results[i];

        output = json_parse(primitives[i]);
        ASSERT_TRUE(NULL != output->root);
        ASSERT_EQ(JSON_ERROR_NONE, output->error);

        switch (res.type)
        {
            case JSON_TYPE_NUMBER:
                ASSERT_TRUE(json_is_equal2number(output->root, res.num_val));
                break;
            case JSON_TYPE_BOOLEAN:
                ASSERT_TRUE(json_is_equal2boolean(output->root, res.bool_val));
                break;
            case JSON_TYPE_NULL:
                ASSERT_TRUE(JSON_IS_NULL(output->root));
                break;
            case JSON_TYPE_STRING:
                ASSERT_TRUE(json_is_equal2string(output->root, res.string_val));
                break;
            default:
                ASSERT_TRUE(false);
                break;
        }

        json_output_destroy(output);
    } 
}


/* NUMBERS */
typedef struct num_info
{
    double      num;
    const char *num_arr;
    json_error  error;
} num_info;

class NumberTest : public ::testing::TestWithParam<num_info> {

};

TEST_P(NumberTest, numbers)
{
    json_output *output = NULL;
    num_info     info = GetParam();

    output = json_parse(info.num_arr);
    ASSERT_EQ(info.error, output->error);

    if (output->error == JSON_ERROR_NONE)
    {
        ASSERT_EQ(JSON_TYPE_NUMBER, output->root->elements[0]->type);
        ASSERT_EQ(info.num, output->root->elements[0]->num_val);
    }

    json_output_destroy(output);
}

INSTANTIATE_TEST_CASE_P(parserTests,
    NumberTest,
    ::testing::Values(
        num_info{ 0,    "[0]",       JSON_ERROR_NONE },
        num_info{ 3,    "[3]",       JSON_ERROR_NONE },
        num_info{ -3,   "[-3]",      JSON_ERROR_NONE },
        num_info{ 0.1,  "[0.1]",     JSON_ERROR_NONE },
        num_info{ 3.1,  "[3.1]",     JSON_ERROR_NONE },
        num_info{ 300,  "[3e2]",     JSON_ERROR_NONE },
        num_info{ 3000, "[3e+3]",    JSON_ERROR_NONE },
        num_info{ 30,   "[300E-1]",  JSON_ERROR_NONE },
        num_info{ -30,  "[-300E-1]", JSON_ERROR_NONE },
        num_info{ -300, "[-300E-0]", JSON_ERROR_NONE },
        num_info{ 0,    "[03]",      JSON_ERROR_INVALID_NUM_FORMAT },
        num_info{ 0,    "[.1]",      JSON_ERROR_INVALID_JSON },
        num_info{ 0,    "[3.]",      JSON_ERROR_INVALID_NUM_FORMAT },
        num_info{ 0,    "[3. 1]",    JSON_ERROR_INVALID_NUM_FORMAT },
        num_info{ 0,    "[3e]",      JSON_ERROR_INVALID_NUM_FORMAT }
        ));


/* STRINGS */
typedef struct str_info
{
    const char *str;
    const char *str_arr;
    json_error  error;
} str_info;

class StringTest : public ::testing::TestWithParam<str_info> {

};

TEST_P(StringTest, strings)
{
    json_output *output = NULL;
    str_info     info = GetParam();

    output = json_parse(info.str_arr);
    ASSERT_EQ(info.error, output->error);

    if (output->error == JSON_ERROR_NONE)
    {
        ASSERT_EQ(JSON_TYPE_STRING, output->root->elements[0]->type);
        ASSERT_STREQ(info.str, output->root->elements[0]->string_val);
    }

    json_output_destroy(output);
}
 
INSTANTIATE_TEST_CASE_P(parserTests,
    StringTest,
    ::testing::Values(
        // basic string
        str_info{ "hello, world", "[\"hello, world\"]", JSON_ERROR_NONE },
        // string with escaped character: " \ / b f n r t
        str_info{ "\"\\/\b\f\n\r\t", "[\"\\\"\\\\\\/\\b\\f\\n\\r\\t\"]", JSON_ERROR_NONE },
        // string without a cloging quote
        str_info{ NULL, "[\"hello, world]", JSON_ERROR_UNBALANCED_QUOTE },
        // string with invalid escape sequence
        str_info{ NULL, "[\"\\u\"]", JSON_ERROR_INVALID_ESCAPE_SEQUENCE },
        str_info{ NULL, "[\"\\m\"]", JSON_ERROR_INVALID_ESCAPE_SEQUENCE },
        // string with a control character
        str_info{ NULL, "[\"\0\"]", JSON_ERROR_UNBALANCED_QUOTE },
        str_info{ NULL, "[\"\037\"]", JSON_ERROR_STRING_HAS_CONTROL_CHAR },
        str_info{ NULL, "[\"\177\"]", JSON_ERROR_STRING_HAS_CONTROL_CHAR }
        ));


/* ARRAYS */
TEST(parserTest, empty_array)
{
    const char  *json_str = "[]";
    json_output *output = json_parse(json_str);
    json        *root = output->root;

    ASSERT_TRUE(NULL != root);
    ASSERT_EQ(JSON_TYPE_ARRAY, root->type);
    ASSERT_EQ(0, root->cnt);
    ASSERT_EQ(NULL, root->elements);

    json_output_destroy(output);
}


TEST(parserTest, simple_array)
{
    const char  *json_str = "[1, 3.14, false, \"hello world\"]";
    json_output *output = json_parse(json_str);
    json        *root = output->root;

    ASSERT_TRUE(NULL != root);
    ASSERT_EQ(JSON_TYPE_ARRAY, root->type);
    ASSERT_EQ(4, root->cnt);
    ASSERT_TRUE(NULL != root->elements);

    ASSERT_EQ(JSON_TYPE_NUMBER, root->elements[0]->type);
    ASSERT_EQ(1, root->elements[0]->num_val);

    ASSERT_EQ(JSON_TYPE_NUMBER, root->elements[1]->type);
    ASSERT_EQ(3.14, root->elements[1]->num_val);

    ASSERT_EQ(JSON_TYPE_BOOLEAN, root->elements[2]->type);
    ASSERT_EQ(false, root->elements[2]->bool_val);

    ASSERT_EQ(JSON_TYPE_STRING, root->elements[3]->type);
    ASSERT_STREQ("hello world", root->elements[3]->string_val);

    json_output_destroy(output);
}


TEST(parserTest, nested_array)
{
    const char  *json_str = " [1, [2, [3]], 4] ";
    json_output *output = json_parse(json_str);
    json        *root = output->root;

    ASSERT_EQ(JSON_TYPE_ARRAY, root->type);
    ASSERT_EQ(3, root->cnt);
    
    ASSERT_EQ(JSON_TYPE_NUMBER, root->elements[0]->type);
    
    ASSERT_EQ(JSON_TYPE_ARRAY, root->elements[1]->type);
    ASSERT_EQ(2, root->elements[1]->cnt);
    
    ASSERT_EQ(JSON_TYPE_NUMBER, root->elements[1]->elements[0]->type);

    ASSERT_EQ(JSON_TYPE_ARRAY, root->elements[1]->elements[1]->type);
    ASSERT_EQ(1, root->elements[1]->elements[1]->cnt);

    ASSERT_EQ(JSON_TYPE_NUMBER, root->elements[1]->elements[1]->elements[0]->type);
    
    ASSERT_EQ(JSON_TYPE_NUMBER, root->elements[2]->type);

    json_output_destroy(output);
}


TEST(parserTest, missing_square_bracket_array)
{
    const char  *json_str = "[1, 2, 3";
    json_output *output = json_parse(json_str);
    json        *root = output->root;

    ASSERT_EQ(NULL, root);
    ASSERT_EQ(JSON_ERROR_UNBALANCED_SQUARE_BRACKET, output->error);

    json_output_destroy(output);
}


TEST(parserTest, missing_comma_array)
{
    const char  *json_str = "[1 2, 3";
    json_output *output = json_parse(json_str);
    json        *root = output->root;

    ASSERT_EQ(NULL, root);
    ASSERT_EQ(JSON_ERROR_UNBALANCED_SQUARE_BRACKET, output->error);

    json_output_destroy(output);
}


TEST(parserTest, extra_input)
{
    const char  *json_str = "[1, 2][]";
    json_output *output = json_parse(json_str);
    json        *root = output->root;

    ASSERT_EQ(NULL, root);
    ASSERT_EQ(JSON_ERROR_INVALID_JSON, output->error);

    json_output_destroy(output);
}


TEST(parserTest, array_recursion_depth_exceeded)
{
    const char json_str[] = \
        "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
        "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
        "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
        "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
        "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
        "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
        "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
        "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
        "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
        "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
        "[[[[[[[[[[[[[";
    
    ASSERT_EQ(JSON_PARSER_MAX_DEPTH + 1, strlen(json_str));
    json_output *output = json_parse(json_str);
    json        *root = output->root;

    ASSERT_EQ(NULL, root);
    ASSERT_EQ(JSON_ERROR_PARSER_MAX_DEPTH_EXCEEDED, output->error);

    json_output_destroy(output);
}

/* OBJECTS */
TEST(parserTest, empty_object)
{
    const char  *json_str = "{}";
    json_output *output = json_parse(json_str);
    json        *root = output->root;

    ASSERT_TRUE(NULL != root);
    ASSERT_EQ(JSON_TYPE_OBJECT, root->type);
    ASSERT_EQ(0, root->cnt);
    ASSERT_EQ(NULL, root->members);

    json_output_destroy(output);
}


TEST(parserTest, simple_object)
{
    const char  *json_str = "{\"number\":3.14,\"string\":\"pi\","
                            "\"boolean\":false,\"null\":null}";
    json_output *output = json_parse(json_str);
    json        *root = output->root;

    ASSERT_TRUE(NULL != root);
    ASSERT_EQ(JSON_TYPE_OBJECT, root->type);
    ASSERT_EQ(4, root->cnt);
    ASSERT_TRUE(NULL != root->members);

    ASSERT_STREQ("number", root->members[0]->key);
    ASSERT_EQ(JSON_TYPE_NUMBER, root->members[0]->value->type);
    ASSERT_EQ(3.14, root->members[0]->value->num_val);

    ASSERT_STREQ("string", root->members[1]->key);
    ASSERT_EQ(JSON_TYPE_STRING, root->members[1]->value->type);
    ASSERT_STREQ("pi", root->members[1]->value->string_val);

    ASSERT_STREQ("boolean", root->members[2]->key);
    ASSERT_EQ(JSON_TYPE_BOOLEAN, root->members[2]->value->type);
    ASSERT_EQ(false, root->members[2]->value->bool_val);

    ASSERT_STREQ("null", root->members[3]->key);
    ASSERT_EQ(JSON_TYPE_NULL, root->members[3]->value->type);

    json_output_destroy(output);
}


TEST(parserTest, nested_object)
{
    const char  *json_str = "{\"k0\":1,\"k1\":{\"k2\":2, \"k3\":true}, \"k4\":{}}";
    json_output *output = json_parse(json_str);
    json        *root = output->root;

    ASSERT_EQ(JSON_TYPE_OBJECT, root->type);
    ASSERT_EQ(3, root->cnt);

    ASSERT_EQ(JSON_TYPE_NUMBER, root->members[0]->value->type);

    ASSERT_EQ(JSON_TYPE_OBJECT, root->members[1]->value->type);
    ASSERT_EQ(2, root->members[1]->value->cnt);

    ASSERT_EQ(JSON_TYPE_NUMBER, root->members[1]->value->members[0]->value->type);

    ASSERT_EQ(JSON_TYPE_BOOLEAN, root->members[1]->value->members[1]->value->type);

    ASSERT_EQ(JSON_TYPE_OBJECT, root->members[2]->value->type);
    ASSERT_EQ(0, root->members[2]->value->cnt);

    json_output_destroy(output);
}


TEST(parserTest, missing_brace_object)
{
    const char  *json_str = "{\"1\":1";
    json_output *output = json_parse(json_str);
    json        *root = output->root;

    ASSERT_EQ(NULL, root);
    ASSERT_EQ(JSON_ERROR_UNBALANCED_BRACE, output->error);

    json_output_destroy(output);
}


TEST(parserTest, missing_comma_object)
{
    const char  *json_str = "{\"1\":1 \"2\":2}";
    json_output *output = json_parse(json_str);
    json        *root = output->root;

    ASSERT_EQ(NULL, root);
    ASSERT_EQ(JSON_ERROR_UNBALANCED_BRACE, output->error);

    json_output_destroy(output);
}


TEST(parserTest, non_string_key_object)
{
    const char  *json_str = "{\"1\":1, 2:2}";
    json_output *output = json_parse(json_str);
    json        *root = output->root;

    ASSERT_EQ(NULL, root);
    ASSERT_EQ(JSON_ERROR_INVALID_JSON, output->error);

    json_output_destroy(output);
}


TEST(parserTest, extra_input_object)
{
    const char  *json_str = "{\"1\":2}{}";
    json_output *output = json_parse(json_str);
    json        *root = output->root;

    ASSERT_EQ(NULL, root);
    ASSERT_EQ(JSON_ERROR_INVALID_JSON, output->error);

    json_output_destroy(output);
}


TEST(parserTest, object_recursion_depth_exceeded)
{
    const char json_str[] = \
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":"
        "{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":{\"\":";
    
    ASSERT_EQ(JSON_PARSER_MAX_DEPTH * 4 + 4, strlen(json_str));
    json_output *output = json_parse(json_str);
    json        *root = output->root;

    ASSERT_EQ(NULL, root);
    ASSERT_EQ(JSON_ERROR_PARSER_MAX_DEPTH_EXCEEDED, output->error);

    json_output_destroy(output);
}

/* HYBRID TESTS */
TEST(parserTest, nested_arrays_object1)
{
    // TODO: the deference chain disaster in this func is a strong motivation
    // to write accessor functions or even macros
    const char  *json_str = "[1, {\"k0\": [2, {\"k1\":3, \"k2\":4}, 5]},6]";
    json_output *output = json_parse(json_str);
    json        *root = output->root;

    ASSERT_EQ(JSON_TYPE_ARRAY, root->type);
    ASSERT_EQ(3, root->cnt);

    ASSERT_EQ(JSON_TYPE_NUMBER, root->elements[0]->type);

    ASSERT_EQ(JSON_TYPE_OBJECT, root->elements[1]->type);
    ASSERT_EQ(1, root->elements[1]->cnt);

    ASSERT_EQ(JSON_TYPE_ARRAY, root->elements[1]->members[0]->value->type);
    ASSERT_EQ(3, root->elements[1]->members[0]->value->cnt);

    ASSERT_EQ(JSON_TYPE_NUMBER, root->elements[1]->members[0]->value->elements[0]->type);

    ASSERT_EQ(JSON_TYPE_OBJECT, root->elements[1]->members[0]->value->elements[1]->type);
    ASSERT_EQ(2, root->elements[1]->members[0]->value->elements[1]->cnt);

    ASSERT_EQ(JSON_TYPE_NUMBER, root->elements[1]->members[0]->value->elements[1]->members[0]->value->type);

    ASSERT_EQ(JSON_TYPE_NUMBER, root->elements[1]->members[0]->value->elements[1]->members[1]->value->type);

    ASSERT_EQ(JSON_TYPE_NUMBER, root->elements[1]->members[0]->value->elements[2]->type);

    ASSERT_EQ(JSON_TYPE_NUMBER, root->elements[2]->type);

    json_output_destroy(output);
}


TEST(parserTest, nested_recursion_depth_exceeded)
{
    const char json_str[] = \
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":"
        "[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":[{\"\":";
    
    ASSERT_EQ(5 * JSON_PARSER_MAX_DEPTH / 2 + 5, strlen(json_str));
    json_output *output = json_parse(json_str);
    json        *root = output->root;

    ASSERT_EQ(NULL, root);
    ASSERT_EQ(JSON_ERROR_PARSER_MAX_DEPTH_EXCEEDED, output->error);

    json_output_destroy(output);
}