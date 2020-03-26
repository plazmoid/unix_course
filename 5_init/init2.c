#include <errno.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include "init2.h"

typedef struct {
    char* prog;
    char** argv;
    char* action;
} runnable;

void err(const char* arg, bool critical) {
    extern int errno;
    char* msg = strerror(errno);
    fprintf(stderr, "Error: %s \nArg: '%s'\n", msg, arg);
    if(critical) {
        exit(1);
    }
}

void read_cfg(char* path) {
    FILE* f = fopen(CFG_NAME, "r");
    if(f == NULL) {
        err(CFG_NAME, true);
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* cfg_raw = malloc(fsize+1);
    fread(cfg_raw, 1, fsize, f);
    fclose(f);
    cfg_raw[fsize] = '\0';

    int cfg_entry_count = 0;
    for(int i = 0; cfg_raw[i]; i++) {
        cfg_entry_count += (cfg_raw[i] == '\n');
    }
    printf("entries found: %d\n", cfg_entry_count);
    free(cfg_raw);
}

void daemonize() {
    struct rlimit lim;
    //signals below are already ignored
    //if we're called by real init
    if(getppid() != 1) {
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
    }
    //create new session 
    //to prevent child killing
    //when killing parent
    setsid();
    //exit current process to release terminal
    if(fork() != 0) {
        exit(0);
    }
    //close all fildes
    getrlimit(RLIMIT_NOFILE, &lim);
    for(int fd = 0; fd < lim.rlim_max; fd++) {
        close(fd);
    }
    chdir("/");
}

int main() {
    read_cfg(CFG_NAME);
    /*daemonize();
    openlog(LOG_IDENT, LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Started lolling");
    sleep(1);
    syslog(LOG_INFO, "Lolling");
    sleep(1);
    syslog(LOG_INFO, "Lolling");
    sleep(1);
    syslog(LOG_INFO, "End lolling");
    closelog();*/
    return 0;
}