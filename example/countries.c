
/*
 * Parse json and list countries by their income levels
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../parser.h"
#include "../json.h"

static char *read_file(const char *file_name)
{
    int     written = 0;
    int     buf_sz = 100000; // the file is 97.1 KB
    char    *country_data = (char *) malloc(buf_sz * sizeof(char));

    FILE    *fp    = fopen(file_name, "r");
    char    *line  = NULL;
    size_t  n      = 0;
    ssize_t read;

    // read the file data into memory (wish we had a file iterator in iterator.c)
    country_data[0] = '\0';
    while ((read = getline(&line, &n, fp)) != -1)
    {
        strcat(country_data + written, line);
        written += read;
    }

    free(line);
    fclose(fp);
    return country_data;
}

static int country_compare(const void *c1, const void *c2)
{
    json *cp1 = *(json **) c1;
    json *cp2 = *(json **) c2;

    char *income_level1 = NULL;
    char *income_level2 = NULL;

    json_object_get_string(json_object_get(cp1, "incomeLevel"), "id", &income_level1);
    json_object_get_string(json_object_get(cp2, "incomeLevel"), "id", &income_level2);

    return strcmp(income_level1, income_level2);

}

/*
 * Sort the countries by their income levels and the same time create an object of the 
 * following form:
   {
     "INCOME_LEVEL_1": [country1, country2, ...., country1M],
     "INCOME_LEVEL_2": [country21, country22, ..., country2N],
     ...
     "INCOME_LEVEL_X": [countryX1, countryX2, ..., countryXP],
   }
 */
static void process_countries(json *countries)
{
    int   i = 0;
    char *old_income_level = NULL;
    json *country = NULL;
    json *income_level2countries_obj = NULL; // the dictionary described above
    json *income_level_countries_arr = NULL;
    char *income_level2countries_obj_str = NULL;

    if (!JSON_IS_ARRAY(countries))
    {
        printf("Expected an array here\n");
        return;
    }

    income_level2countries_obj = JSON_OBJECT_CREATE();

    // sort the elements according to their income levels
    qsort(json_array_get_elements(countries), json_get_size(countries), 
        sizeof(json *), country_compare);
    
    // now print the countries
    for (i = 0; i < json_get_size(countries); i++)
    {
        char *country_name = NULL;
        char *current_income_level = NULL;

        country = json_array_get(countries, i);
        json_object_get_string(country, "name", &country_name);
        json_object_get_string(json_object_get(country, "incomeLevel"), 
            "id", &current_income_level);

        if (!old_income_level || strcmp(old_income_level, current_income_level) != 0)
        {
            income_level_countries_arr = JSON_ARRAY_CREATE();
            json_object_put_complex_value(income_level2countries_obj, current_income_level, 
                income_level_countries_arr);

            printf("\n***************************\n%s\n***************************\n", 
                current_income_level);
            old_income_level = current_income_level;
        }
        json_array_append_string(income_level_countries_arr, country_name);
        printf("%s\n", country_name);

    }
    printf("\n\n");

    income_level2countries_obj_str = json2string(income_level2countries_obj, 0);
    printf("The income_level2countries_obj string rep is:\n%s\n",
        income_level2countries_obj_str);

exit:
    free(income_level2countries_obj_str);
    json_destroy(income_level2countries_obj); // takes care of the created arrays
}

int main(int argc, char const *argv[])
{
    const char  *file_name = "./countries.json";
    char        *country_data = NULL;
    json_output *output = NULL;
    json        *root = NULL;
    
    country_data = read_file(file_name);
    output = json_parse(country_data);
    free(country_data); // you are excused, thanks!

    root = output->root;

    if (output->error)
    {
        printf("json parsing failed Error code = %d\n", output->error);
        goto exit;
    }

    if (!root)
    {
        printf("The file is empty\n");
        goto exit;
    }

    process_countries(root);

exit:
    json_output_destroy(output);

    return 0;
}