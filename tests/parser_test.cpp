#include "gtest/gtest.h"
#include "parser.h"

/*
 * List of tests
 * INCORRECT STARTING INPUT
 *  - [DONE]NULL input
 *  - [empty input
 *  - [DONE]primitives: number, boolean, string
 * ARRAY
 *  - [DONE]empty array
 *  - [DONE]simple array with number, string, null, true, false
 *  - [DONE]complex array with nested arrays
 *  - [DONE]incorrect array: missing square bracket
 *  - [DONE]incorrect array: missing comma
 * OBJECT
 *  - [DONE]empty object
 *  - [DONE]simple object
 *  - [DONE]nested object
 *  - [DONE]incorrect object: missing brace
 *  - [DONE]incorrect array: missing comm
 *  - [DONE]incorrect object: key not a string
 * HYBRID
 *  - [DONE]nested
 *  - complex strings that have control characters
 *  - e.g incorrect array followed by invalid json input
 */


TEST(parserTest, null_input)
{
    const char  *json_str = NULL;
    json_output *output = json_parse(json_str);

    ASSERT_NE((json_output *) NULL, output); // TODO find why we need the cast
    ASSERT_EQ(NULL, output->root);
    ASSERT_EQ(0, output->error);

    json_output_destroy(output);
}


TEST(parserTest, primitives)
{
    int          i = 0;
    const char  *primitives[] = { "3.14", "true", "false", "\"json\"" };
    int          num_prims = sizeof(primitives) / sizeof(char *);
    json_output *output = NULL;

    for ( i = 0; i < num_prims; i++)
    {
        output = json_parse(primitives[i]);

        ASSERT_EQ(NULL, output->root);
        ASSERT_EQ(JSON_ERROR_INVALID_JSON, output->error);

        json_output_destroy(output);
    } 
}


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
