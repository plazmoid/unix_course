#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>

// function that can either show system (errno) error messages
// or custom with arguments
void err(char *msg, const char *arg, bool critical) {
    if(msg == NULL || strlen(msg) == 0) {
        extern int errno;
        msg = strerror(errno);
    }
    if (arg != NULL && strlen(arg) > 0) {
        syslog(LOG_ERR, "%s, '%s'\n", msg, arg);
    } else {
        syslog(LOG_ERR, "%s\n", msg);
    }
    if(critical) {
        exit(-1);
    }
}