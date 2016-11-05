
/*
 * Tests the JSON parser
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// #include "../../parser.h"
#include "../../json.h"

static char *read_file(const char *file_name)
{
    int     written = 0;
    int     buf_sz = 300000; // the file is 97.1 KB
    char    *file_data = (char *) malloc(buf_sz * sizeof(char));

    FILE    *fp    = fopen(file_name, "r");
    char    *line  = NULL;
    size_t  n      = 0;
    ssize_t read;

    // read the file data into memory (wish we had a file iterator in iterator.c)
    file_data[0] = '\0';
    while ((read = getline(&line, &n, fp)) != -1)
    {
        strcat(file_data + written, line);
        written += read;
    }

    free(line);
    fclose(fp);
    return file_data;
}

int main(int argc, char const *argv[])
{
    const char  *file_name = "";
    char        *file_data = NULL;
    json_output *output = NULL;
    json        *root = NULL;

    if (argc < 2)
    {
	exit(EXIT_FAILURE);
    }

    file_name = argv[1];

    printf("Reading input...\n");
    file_data = read_file(file_name);
    printf("Parsing input...\n");
    output = json_parse(file_data);
    free(file_data); // you are excused, thanks!
    
    root = output->root;

    if (json_parser_found_error(output))
    {
	printf("Error: %s, Near character: %d\n",
	       json_parser_get_error(output),
	       json_parser_get_error_loc(output));
        //printf("json parsing failed Error code = %d\n", output->error);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

