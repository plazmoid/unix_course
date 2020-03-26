#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void err(char *msg, const char *arg, bool critical) {
    if(msg == NULL || strlen(msg) == 0) {
        extern int errno;
        msg = strerror(errno);
    }
    fprintf(stderr, "Error: %s\n", msg);
    if (arg != NULL && strlen(arg) > 0) {
        fprintf(stderr, "Arg: '%s'\n", arg);
    }
    if(critical) {
        exit(1);
    }
}