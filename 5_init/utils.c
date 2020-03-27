#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
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

char* str_slice(const char* arr, int from, int to) {
    int slen = strlen(arr);
    if(from >= slen || to < from) {
        char errmsg[64];
        sprintf(errmsg, "%s, [%d; %d]", arr, from, to);
        err("Index error", errmsg, true);
    }

    //inclusive slice
    int res_size = (to - from) * sizeof(char) + 1;
    char* res = (char*)malloc(res_size);
    if (res == NULL) {
        err(NULL, "slice", true);
    }
    strncpy(res, arr+from, res_size);
    return res;
}

char* get_exec_from_abspath(char* path) {
    int i, slen = strlen(path);
    for(i = slen-1; i > 0 && path[i] != '/'; i--);

    if(i == 0) {
        return path;
    } else {
        return str_slice(path, i, slen-1);
    }
}