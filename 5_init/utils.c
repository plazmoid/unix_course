#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include "errors.h"

char* join_str(char *strings[], char *separator, int count) {
    size_t total_length = 0;
    int i = 0;

    for (i = 0; i < count; i++) {
        total_length += strlen(strings[i]);
    }
    total_length++;
    total_length += strlen(separator) * (count - 1);

    char *str = (char*)calloc(1, total_length);
    if(!str) {
        err(NULL, "join", true);
    }

    for (i = 0; i < count; i++) {
        strcat(str, strings[i]);
        if (i < (count - 1)) {
            strcat(str, separator);
        }
    }

    return str;
}

// '/bin/executable' -> 'executable'
char* get_exec_from_abspath(char* path) {
    char *slash = rindex(path, '/');

    if(slash == NULL) {
        return path;
    } else {
        char *ret = strdup(slash+1);
        free(path);
        return ret;
    }
}