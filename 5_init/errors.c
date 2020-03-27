#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include "init2.h" //atexit

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
        manage_pidfile(PIDFILE, false);
        syslog(LOG_ERR, "Exiting");
        exit(-1);
    }
}