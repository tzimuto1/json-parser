#include "gtest/gtest.h"
#include "parser.h"
#include "json.h"

TEST(json_object_has_keyTest, empty_object)
{
    const char  *json_str = "{}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;

    EXPECT_FALSE(json_object_has_key(object, ""));
    EXPECT_FALSE(json_object_has_key(object, "key"));

    json_output_destroy(output);
}


TEST(json_object_has_keyTest, simple_object)
{
    const char  *json_str = "{\"number\": 1}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;

    EXPECT_TRUE(json_object_has_key(object, "number"));

    json_output_destroy(output);
}


TEST(json_object_has_numberTest, simple_object)
{
    const char  *json_str = "{\"number\": 3.14}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;

    EXPECT_TRUE(json_object_has_number(object, 3.14));
    EXPECT_FALSE(json_object_has_number(object, 3));

    json_output_destroy(output);
}


TEST(json_object_has_booleanTest, simple_object)
{
    const char  *json_str = "{\"is true?\": false}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;

    EXPECT_TRUE(json_object_has_boolean(object, false));
    EXPECT_FALSE(json_object_has_boolean(object, true));

    json_output_destroy(output);
}


TEST(json_object_has_stringTest, simple_object)
{
    const char  *json_str = "{\"string\": \"hello world\"}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;

    EXPECT_TRUE(json_object_has_string(object, "hello world"));
    EXPECT_FALSE(json_object_has_string(object, "hello worlds"));
    EXPECT_FALSE(json_object_has_string(object, ""));
    EXPECT_FALSE(json_object_has_string(object, NULL));

    json_output_destroy(output);
}


TEST(json_object_put_number, basic_update)
{
    int          rv;
    double       test_num = 3.14;
    const char  *test_key = "value";
    const char  *json_str = "{\"name\": \"pi\"}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;

    ASSERT_FALSE(json_object_has_key(object, test_key));
    ASSERT_FALSE(json_object_has_number(object, test_num));

    rv = json_object_put_number(object, test_key, test_num);
    ASSERT_EQ(API_ERROR_NONE, rv);

    EXPECT_TRUE(json_object_has_key(object, test_key));
    EXPECT_TRUE(json_object_has_number(object, test_num));

    json_output_destroy(output);
}


TEST(json_object_put_number, existing_value)
{
    int          rv;
    double       test_num = 3.14;
    const char  *test_key = "value";
    const char  *json_str = "{\"name\": \"pi\", \"value\":3.1}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;

    ASSERT_TRUE(json_object_has_key(object, test_key));
    ASSERT_TRUE(json_object_has_number(object, 3.1));
    ASSERT_EQ(2, object->cnt);

    rv = json_object_put_number(object, test_key, test_num);
    ASSERT_EQ(API_ERROR_NONE, rv);

    EXPECT_TRUE(json_object_has_key(object, test_key));
    EXPECT_TRUE(json_object_has_number(object, test_num));
    ASSERT_EQ(2, object->cnt);

    json_output_destroy(output);
}


TEST(json_object_put_boolean, basic_update)
{
    int          rv;
    bool         test_bool = true;
    const char  *test_key = "value";
    const char  *json_str = "{}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;

    ASSERT_FALSE(json_object_has_key(object, test_key));
    ASSERT_FALSE(json_object_has_boolean(object, test_bool));

    rv = json_object_put_boolean(object, test_key, test_bool);
    ASSERT_EQ(API_ERROR_NONE, rv);

    EXPECT_TRUE(json_object_has_key(object, test_key));
    EXPECT_TRUE(json_object_has_boolean(object, test_bool));

    json_output_destroy(output);
}


TEST(json_object_put_string, basic_update)
{
    int          rv;
    const char  *json_str = "{\"value\": \"Hello\"}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;

    rv = json_object_put_string(object, "value", NULL);
    ASSERT_EQ(API_ERROR_VALUE_INVALID, rv);

    rv = json_object_put_string(object, "value", "Hello World!");
    ASSERT_EQ(API_ERROR_NONE, rv);
    ASSERT_TRUE(json_object_has_string(object, "Hello World!"));

    rv = json_object_put_string(object, "value2", "Executing...");
    ASSERT_EQ(API_ERROR_NONE, rv);
    ASSERT_TRUE(json_object_has_string(object, "Executing..."));

    json_output_destroy(output);
}


TEST(json_object_remove_member, object)
{
    const char  *json_str = "{\"value\": 0, \"dup_val\": 1, \"dup_val\": 2}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;

    json_object_remove_member(object, "val");
    ASSERT_EQ(3, object->cnt);

    json_object_remove_member(object, "value");
    ASSERT_FALSE(json_object_has_key(object, "value"));
    ASSERT_EQ(2, object->cnt);

    json_object_remove_member(object, "dup_val");
    ASSERT_FALSE(json_object_has_key(object, "dup_val"));
    ASSERT_EQ(0, object->cnt);

    json_output_destroy(output);
}

