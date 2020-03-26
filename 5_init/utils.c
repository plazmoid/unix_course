#include <stdlib.h>
#include <string.h>

char* join_str(char* strings[], char* separator, int count) {
    size_t total_length = 0;
    int i = 0;

    for (i = 0; i < count; i++) {
        total_length += strlen(strings[i]);
    }
    total_length++;
    total_length += strlen(separator) * (count - 1);

    char *str = (char*)malloc(total_length);
    str[0] = '\0';

    for (i = 0; i < count; i++) {
        strcat(str, strings[i]);
        if (i < (count - 1)) {
            strcat(str, separator);
        }
    }

    return str;
}