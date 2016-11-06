#include "gtest/gtest.h"

extern "C" {
    #include "iterator.h"
}


// TODO: we need a fixture class so that we do not repeat some of the code e.g
// json_output_destroy

/* ========== OBJECT METHODS ========== */

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

TEST(json_object_put_numberTest, basic_update)
{
    int          rv;
    double       test_num = 3.14;
    const char  *test_key = "value";
    const char  *json_str = "{\"name\": \"pi\"}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;
    json        *js = NULL;

    js = json_object_get(object, test_key);
    ASSERT_TRUE(js == NULL);

    rv = json_object_put_number(object, test_key, test_num);
    ASSERT_EQ(API_SUCCESS, rv);

    js = json_object_get(object, test_key);
    ASSERT_TRUE(JSON_IS_NUMBER(js));
    ASSERT_EQ(test_num, js->num_val);

    json_output_destroy(output);
}

TEST(json_object_put_numberTest, existing_value)
{
    int          rv;
    double       test_num = 3.14;
    const char  *test_key = "value";
    const char  *json_str = "{\"name\": \"pi\", \"value\":\"3.1\"}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;
    json        *js = NULL;

    js = json_object_get(object, test_key);
    ASSERT_TRUE(JSON_IS_STRING(js));
    ASSERT_EQ(2, json_get_size(object));

    rv = json_object_put_number(object, test_key, test_num);
    ASSERT_EQ(API_SUCCESS, rv);

    js = json_object_get(object, test_key);
    ASSERT_TRUE(JSON_IS_NUMBER(js));
    ASSERT_EQ(test_num, js->num_val);
    ASSERT_EQ(2, json_get_size(object));

    json_output_destroy(output);
}

TEST(json_object_put_booleanTest, basic_update)
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
    ASSERT_EQ(API_SUCCESS, rv);

    EXPECT_TRUE(json_object_has_key(object, test_key));
    EXPECT_TRUE(json_object_has_boolean(object, test_bool));

    json_output_destroy(output);
}

TEST(json_object_put_stringTest, basic_update)
{
    int          rv;
    const char  *json_str = "{\"value\": \"Hello\"}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;

    rv = json_object_put_string(object, "value", NULL);
    ASSERT_EQ(API_FAILURE, rv);

    rv = json_object_put_string(object, "value", "Hello World!");
    ASSERT_EQ(API_SUCCESS, rv);
    ASSERT_TRUE(json_object_has_string(object, "Hello World!"));

    rv = json_object_put_string(object, "value2", "Executing...");
    ASSERT_EQ(API_SUCCESS, rv);
    ASSERT_TRUE(json_object_has_string(object, "Executing..."));

    json_output_destroy(output);
}

TEST(json_object_remove_memberTest, object)
{
    const char  *json_str = "{\"value\": 0, \"dup_val\": 1, \"dup_val\": 2}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;

    json_object_remove_member(object, "val");
    ASSERT_EQ(3, json_get_size(object));

    json_object_remove_member(object, "value");
    ASSERT_FALSE(json_object_has_key(object, "value"));
    ASSERT_EQ(2, json_get_size(object));

    json_object_remove_member(object, "dup_val");
    ASSERT_FALSE(json_object_has_key(object, "dup_val"));
    ASSERT_EQ(0, json_get_size(object));

    json_output_destroy(output);
}

TEST(json_object_getTest, basic)
{
    const char  *json_str = "{\"pi\": 3.14, \"e\": {\"is_rational\": false}}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;
    json        *js = NULL;

    js = json_object_get(object, "pi");
    ASSERT_TRUE(JSON_IS_NUMBER(js));
    ASSERT_EQ(3.14, js->num_val);

    js = json_object_get(json_object_get(object, "e"), "is_rational");
    ASSERT_TRUE(JSON_IS_BOOLEAN(js));
    ASSERT_EQ(false, js->bool_val);

    ASSERT_TRUE(NULL == json_object_get(object, "i"));

    json_output_destroy(output);
}

TEST(json_object_get_allTest, basic)
{
    const char  *json_str = "{\"pi\": 3.14, \"e\": 2.72, \"g\": 9.81}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;
    json        **all_js = NULL;

    all_js = json_object_get_all(object);
    ASSERT_TRUE(all_js != NULL);

    ASSERT_EQ(3.14, all_js[0]->num_val);
    ASSERT_EQ(2.72, all_js[1]->num_val);
    ASSERT_EQ(9.81, all_js[2]->num_val);
    ASSERT_TRUE(NULL == all_js[3]);

    free(all_js);
    json_output_destroy(output);
}

TEST(json_object_get_numberTest, basic)
{
    double      number = 0;
    const char  *json_str = "{\"pi\": 3.14, \"b\": true}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;     

    ASSERT_EQ(API_FAILURE, json_object_get_number(object, "pi", NULL));

    ASSERT_EQ(API_SUCCESS, json_object_get_number(object, "pi", &number));
    ASSERT_EQ(3.14, number);

    ASSERT_EQ(API_FAILURE, json_object_get_number(object, "b", &number));
    
    json_output_destroy(output);
}

TEST(json_object_get_booleanTest, basic)
{
    bool         bool_val = false;
    const char  *json_str = "{\"pi\": 3.14, \"b\": true}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;     

    ASSERT_EQ(API_FAILURE, json_object_get_boolean(object, "b", NULL));

    ASSERT_EQ(API_SUCCESS, json_object_get_boolean(object, "b", &bool_val));
    ASSERT_EQ(true, bool_val);

    ASSERT_EQ(API_FAILURE, json_object_get_boolean(object, "pi", &bool_val));
    
    json_output_destroy(output);
}

TEST(json_object_get_stringTest, basic)
{
    char        *str_val = NULL;
    const char  *json_str = "{\"str\": \"Hello World\", \"b\": true}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;     

    ASSERT_EQ(API_FAILURE, json_object_get_string(object, "b", NULL));

    ASSERT_EQ(API_SUCCESS, json_object_get_string(object, "str", &str_val));
    ASSERT_STREQ("Hello World", str_val);

    ASSERT_EQ(API_FAILURE, json_object_get_string(object, "b", &str_val));
    
    json_output_destroy(output);
}

TEST(json_obj_iterTest, basic)
{
    const char *json_str = "{\"a\": \"Hello World\", \"b\": true, \"c\":3.14}";
    json_output *output = json_parse(json_str);
    json        *object = output->root;
    json_obj_iter it;
    obj_pair    *pair = NULL;
    json        *value = NULL;

    /*
      we will 'unroll' the loop below
      for (pair = json_obj_next(it); pair != json_obj_end(it); pair = json_obj_next(it))
      {}
    */

    it = json_obj_iter_init(object);

    pair = json_obj_next(&it);
    ASSERT_TRUE(pair != json_obj_end(&it));
    value = pair->value;
    EXPECT_STREQ("a", (char *)pair->key);
    EXPECT_TRUE(json_is_equal2string(value, "Hello World"));

    pair = json_obj_next(&it);
    ASSERT_TRUE(pair != json_obj_end(&it));
    value = pair->value;
    EXPECT_STREQ("b", (char *) pair->key);
    EXPECT_TRUE(json_is_equal2boolean(value, true));

    pair = json_obj_next(&it);
    ASSERT_TRUE(pair != json_obj_end(&it));
    value = pair->value;
    EXPECT_STREQ("c", (char *) pair->key);
    EXPECT_TRUE(json_is_equal2number(value, 3.14));

    pair = json_obj_next(&it);
    ASSERT_TRUE(pair == json_obj_end(&it));

    json_output_destroy(output);
}

/* ========== ARRAY METHODS ========== */

TEST(json_array_has_numberTest, simple_array)
{
    const char  *json_str = "[3.14]";
    json_output *output = json_parse(json_str);
    json        *object = output->root;

    EXPECT_TRUE(json_array_has_number(object, 3.14));
    EXPECT_FALSE(json_array_has_number(object, 3));

    json_output_destroy(output);
}

TEST(json_array_has_booleanTest, simple_array)
{
    const char  *json_str = "[false]";
    json_output *output = json_parse(json_str);
    json        *array = output->root;

    EXPECT_TRUE(json_array_has_boolean(array, false));
    EXPECT_FALSE(json_array_has_boolean(array, true));

    json_output_destroy(output);
}

TEST(json_array_has_stringTest, simple_array)
{
    const char  *json_str = "[\"string\", \"hello world\"]";
    json_output *output = json_parse(json_str);
    json        *array = output->root;

    EXPECT_TRUE(json_array_has_string(array, "hello world"));
    EXPECT_FALSE(json_array_has_string(array, "hello worlds"));
    EXPECT_FALSE(json_array_has_string(array, ""));
    EXPECT_FALSE(json_array_has_string(array, NULL));

    json_output_destroy(output);
}

TEST(json_array_getTest, basic)
{
    const char  *json_str = "[3.14, {\"is_rational\": false}, \"true\"]";
    json_output *output = json_parse(json_str);
    json        *array = output->root;
    json        *js = NULL;

    // out of bounds check
    js = json_array_get(array, -1);
    ASSERT_TRUE(NULL == js);

    js = json_array_get(array, json_get_size(array) + 1);
    ASSERT_TRUE(NULL == js);

    // rest of tests
    js = json_array_get(array, 0);
    ASSERT_TRUE(JSON_IS_NUMBER(js));
    ASSERT_EQ(3.14, js->num_val);

    js = json_array_get(array, 1);
    ASSERT_TRUE(JSON_IS_OBJECT(js));
    js = json_object_get(js, "is_rational");
    ASSERT_FALSE(NULL == js);
    ASSERT_EQ(false, js->bool_val);

    js = json_array_get(array, 2);
    ASSERT_TRUE(JSON_IS_STRING(js));
    ASSERT_STREQ("true", (char *) js->string_val);

    json_output_destroy(output);
}

TEST(json_array_get_numberTest, basic)
{
    double      number = 0;
    const char  *json_str = "[3.14, true]";
    json_output *output = json_parse(json_str);
    json        *array = output->root;     

    ASSERT_EQ(API_FAILURE, json_array_get_number(array, 0, NULL));

    ASSERT_EQ(API_SUCCESS, json_array_get_number(array, 0, &number));
    ASSERT_EQ(3.14, number);

    ASSERT_EQ(API_FAILURE, json_array_get_number(array, 1, &number));
    
    json_output_destroy(output);
}

TEST(json_array_get_booleanTest, basic)
{
    bool        bool_val = false;
    const char  *json_str = "[3.14, true]";
    json_output *output = json_parse(json_str);
    json        *array = output->root;     

    ASSERT_EQ(API_FAILURE, json_array_get_boolean(array, 0, NULL));

    ASSERT_EQ(API_SUCCESS, json_array_get_boolean(array, 1, &bool_val));
    ASSERT_EQ(true, bool_val);

    ASSERT_EQ(API_FAILURE, json_array_get_boolean(array, 0, &bool_val));
    
    json_output_destroy(output);
}

TEST(json_array_get_stringTest, basic)
{
    char        *str_val = NULL;
    const char  *json_str = "[\"Hello World\", true]";
    json_output *output = json_parse(json_str);
    json        *array = output->root;     

    ASSERT_EQ(API_FAILURE, json_array_get_string(array, 0, NULL));

    ASSERT_EQ(API_SUCCESS, json_array_get_string(array, 0, &str_val));
    ASSERT_STREQ("Hello World", str_val);

    ASSERT_EQ(API_FAILURE, json_array_get_string(array, 1, &str_val));
    
    json_output_destroy(output);
}

TEST(json_array_add_numberTest, basic)
{
    int          rv;
    const char  *json_str = "[\"Hello World\", 1, true]";
    json_output *output = json_parse(json_str);
    json        *array = output->root;

    // replace another number
    rv = json_array_add_number(array, 1, 3.14);
    ASSERT_EQ(API_SUCCESS, rv);
    ASSERT_EQ(JSON_TYPE_NUMBER, array->elements[1]->type);
    ASSERT_EQ(3.14, array->elements[1]->num_val);

    // replacing a non number
    rv = json_array_add_number(array, 2, 6.28);
    ASSERT_EQ(API_SUCCESS, rv);
    ASSERT_EQ(JSON_TYPE_NUMBER, array->elements[2]->type);
    ASSERT_EQ(6.28, array->elements[2]->num_val);

    json_output_destroy(output);
}

TEST(json_array_add_booleanTest, basic)
{
    int          rv;
    const char  *json_str = "[\"Hello World\", 1, true]";
    json_output *output = json_parse(json_str);
    json        *array = output->root;

    rv = json_array_add_boolean(array, 1, false);
    ASSERT_EQ(API_SUCCESS, rv);
    ASSERT_EQ(JSON_TYPE_BOOLEAN, array->elements[2]->type);
    ASSERT_EQ(true, array->elements[2]->bool_val);

    json_output_destroy(output);
}

TEST(json_array_add_stringTest, basic)
{
    int          rv;
    const char  *str_val = "Hello Universe";
    const char  *json_str = "[\"Hello World\", 1, true]";
    json_output *output = json_parse(json_str);
    json        *array = output->root;

    rv = json_array_add_string(array, 1, str_val);
    ASSERT_EQ(API_SUCCESS, rv);
    ASSERT_EQ(JSON_TYPE_STRING, array->elements[1]->type);
    ASSERT_STREQ(str_val, (char *) array->elements[1]->string_val);
    ASSERT_EQ(strlen(str_val), json_get_size(array->elements[1]));

    json_output_destroy(output);
}

TEST(json_array_index_of_numberTest, basic)
{
    int          rv;
    const char  *json_str = "[\"Hello World\", 3.14, true]";
    json_output *output = json_parse(json_str);
    json        *array = output->root;

    rv = json_array_index_of_number(array, 3);
    ASSERT_EQ(-1, rv);

    rv = json_array_index_of_number(array, 3.14);
    ASSERT_EQ(1, rv);

    json_output_destroy(output);
}

TEST(json_array_index_of_booleanTest, basic)
{
    int          rv;
    const char  *json_str = "[\"Hello World\", 3.14, true]";
    json_output *output = json_parse(json_str);
    json        *array = output->root;

    rv = json_array_index_of_boolean(array, true);
    ASSERT_EQ(2, rv);

    rv = json_array_index_of_boolean(array, false);
    ASSERT_EQ(-1, rv);

    json_output_destroy(output);
}

TEST(json_array_index_of_stringTest, basic)
{
    int          rv;
    const char  *json_str = "[\"Hello World\", 3.14, true]";
    json_output *output = json_parse(json_str);
    json        *array = output->root;

    rv = json_array_index_of_string(array, "Hello World");
    ASSERT_EQ(0, rv);

    rv = json_array_index_of_string(array, "Hello Universe");
    ASSERT_EQ(-1, rv);

    json_output_destroy(output);
}

TEST(json_array_append_numberTest, basic)
{
    int          test_num = 4;
    const char  *json_str = "[\"Hello World\", 3.14, true]";
    json_output *output = json_parse(json_str);
    json        *array = output->root;
    json        *js;

    json_array_append_number(array, test_num);

    js = array->elements[array->cnt - 1];
    ASSERT_EQ(JSON_TYPE_NUMBER, js->type);
    ASSERT_EQ(test_num, js->num_val);

    json_output_destroy(output);
}

TEST(json_array_append_booleanTest, basic)
{
    bool         test_bool = true;
    const char  *json_str = "[\"Hello World\", 3.14]";
    json_output *output = json_parse(json_str);
    json        *array = output->root;
    json        *js;

    json_array_append_boolean(array, test_bool);

    js = array->elements[array->cnt - 1];
    ASSERT_EQ(JSON_TYPE_BOOLEAN, js->type);
    ASSERT_EQ(test_bool, js->bool_val);

    json_output_destroy(output);
}

TEST(json_array_append_stringTest, basic)
{
    const char  *test_str = "Hello Universe";
    const char  *json_str = "[\"Hello World\", 3.14]";
    json_output *output = json_parse(json_str);
    json        *array = output->root;
    json        *js;

    json_array_append_string(array, test_str);

    js = array->elements[array->cnt - 1];
    ASSERT_EQ(JSON_TYPE_STRING, js->type);
    ASSERT_STREQ(test_str, (char *) js->string_val);
    ASSERT_EQ(strlen(test_str), strlen((char *) js->string_val));

    json_output_destroy(output);
}

TEST(json_array_remove_atTest, basic)
{
    const char  *json_str = "[\"Hello World\", 3.14, true]";
    json_output *output = json_parse(json_str);
    json        *array = output->root;

    // out of bounds
    json_array_remove_at(array, -1);
    ASSERT_EQ(3, json_get_size(array));

    json_array_remove_at(array, 3);
    ASSERT_EQ(3, json_get_size(array));

    // within bounds
    ASSERT_TRUE(json_array_has_number(array, 3.14));
    json_array_remove_at(array, 1);
    ASSERT_FALSE(json_array_has_number(array, 3.14));
    ASSERT_EQ(2, json_get_size(array));

    ASSERT_TRUE(json_array_has_boolean(array, true));
    json_array_remove_at(array, 1);
    ASSERT_FALSE(json_array_has_boolean(array, true));
    ASSERT_EQ(1, json_get_size(array));

    ASSERT_TRUE(json_array_has_string(array, "Hello World"));
    json_array_remove_at(array, 0);
    ASSERT_FALSE(json_array_has_string(array, "Hello World"));
    ASSERT_TRUE(json_is_empty(array));

    json_output_destroy(output);
}

/* 
 This calls a set of functions that utilizes the "hidden" json_array_remove_element
 function
*/
TEST(json_array_remove_elementTest, basic)
{
    /*
    Among other tests, we should test that we remove the first matching item not
    all of them.
    */

    const char  *json_str = "[\"Hello World\", 3.14, true, 3.14]";
    json_output *output = json_parse(json_str);
    json        *array = output->root;

    ASSERT_EQ(1, json_array_index_of_number(array, 3.14));
    json_array_remove_number(array, 3.14);
    ASSERT_EQ(2, json_array_index_of_number(array, 3.14));
    ASSERT_EQ(3, json_get_size(array));

    json_array_remove_number(array, 3.14);
    ASSERT_FALSE(json_array_has_number(array, 3.14));
    ASSERT_EQ(2, json_get_size(array));

    ASSERT_TRUE(json_array_has_boolean(array, true));
    json_array_remove_boolean(array, true);
    ASSERT_FALSE(json_array_has_boolean(array, true));
    ASSERT_EQ(1, json_get_size(array));

    ASSERT_TRUE(json_array_has_string(array, "Hello World"));
    json_array_remove_string(array, "Hello World");
    ASSERT_FALSE(json_array_has_string(array, "Hello World"));
    ASSERT_TRUE(json_is_empty(array));

    json_output_destroy(output);
}

/* ========== PRINTING METHODS ========== */

// simple array
// simple object
// nested array
// nested object

typedef struct {
    const char *input_str;
    const char *exp_output_str;
    int  indent;
} json_data;

class Json2StringTest : public ::testing::TestWithParam<json_data> {

};


TEST_P(Json2StringTest, all_tests)
{
    char        *output_str = NULL;
    json_data   jd = GetParam();
    json_output *output = json_parse(jd.input_str);

    ASSERT_NE((json_output *) NULL, output);

    if (jd.input_str[0])
    {
        ASSERT_NE((json *) NULL, output->root);

        output_str = json2string(output->root, jd.indent);
        ASSERT_STREQ(jd.exp_output_str, output_str);
    }
    else
    {
        ASSERT_EQ(NULL, output->root);
    }

    free(output_str);
    json_output_destroy(output);
}

INSTANTIATE_TEST_CASE_P(json2stringTests,
    Json2StringTest,
    ::testing::Values(
        // empty string
        json_data{ "", "", 0 },
        // lone primitives
        json_data{ "1.23", "1.230000", 0 },
        json_data{ "true", "true", 0 },
        json_data{ "null", "null", 0 },
        json_data{ "\"string\"", "\"string\"", 0 },
        // empty array
        json_data{ "[]", "[]", 0 },
        // simple array with integers
        json_data{ "[1,2,3.14]", "[1.000000,2.000000,3.140000]", 0 },
        // simple array with booleans
        json_data{ "[true,false]", "[true,false]", 0 },
        // simple array with null
        json_data{ "[null]", "[null]", 0 },
        // simple array with strings
        json_data{ "[\"Hello\",\"World\",\"\"]", "[\"Hello\",\"World\",\"\"]", 0 },
        // simple array with complex string A"\/\b\f\n\r\tA
        json_data{ "[\"A\\\"\\\\/\\b\\f\\n\\r\\tA\"]", 
            "[\"A\\\"\\\\\\/\\b\\f\\n\\r\\tA\"]", 0 },
        // empty object
        json_data{ "{}", "{}", 0 },
        // object with primitives
        json_data{ "{\"a\":1,\"b\":true,\"c\":null,\"d\":\"string\"}", 
            "{\"a\":1.000000,\"b\":true,\"c\":null,\"d\":\"string\"}", 0 },
        // nested json values
        json_data{ "{\"a\":1,\"b\":{\"c\":[1,2,3,{\"d\":\"d\"}]}}", 
            "{\"a\":1.000000,\"b\":{\"c\":[1.000000,2.000000,3.000000,{\"d\":\"d\"}]}}", 0 },
        // json with unicode
        json_data{ "[\"TSON \u00a9\"]", "[\"TSON ©\"]", 0},
        // indentation tests
        json_data{ "[]", "[]", 1},
        json_data{ "{}", "{}", 4},
        json_data{ "[1,2,3.14]", "[1.000000,2.000000,3.140000]", -1 },
        json_data{ "[1,2,3.14]", "[\n 1.000000,\n 2.000000,\n 3.140000\n]", 1 },
        json_data{ "[1,2,3.14]", "[\n    1.000000,\n    2.000000,\n    3.140000\n]", 4 },
        json_data{ "{\"a\":1,\"b\":true}", 
            "{\n \"a\": 1.000000,\n \"b\": true\n}", 1 },
        json_data{ "[1,{\"k0\":[2,{\"k1\":3},4]},5]", 
            "[\n"
            " 1.000000,\n"
            " {\n"
            "  \"k0\": [\n"
            "   2.000000,\n"
            "   {\n"
            "    \"k1\": 3.000000\n"
            "   },\n"
            "   4.000000\n"
            "  ]\n"
            " },\n"
            " 5.000000\n"
            "]", 1}
        ));

