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
#include <wordexp.h>
#include "init2.h"
#include "errors.h"


entry_t parse_config_line(const char *line) {
    entry_t entry;
    wordexp_t result;
    if(wordexp(line, &result, 0) != 0) {
        err(ERRLNFMT, line, true);
    }
    entry.action = strdup(result.we_wordv[result.we_wordc-1]);
    result.we_wordv[result.we_wordc-1] = "";
    entry.cmd = result.we_wordv;
    return entry;
}

void read_cfg(char *path, entries_t* parsed_entries) {
    entry_t entry;
    char *cfg_raw, *tok_ptr;
    int cfg_lines_count = 0;
    long fsize;
    FILE *f;

    f = fopen(CFG_NAME, "r");
    if(f == NULL) {
        err(NULL, CFG_NAME, true);
    }
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    cfg_raw = malloc(fsize+1);
    if(cfg_raw == NULL) {
        err(NULL, NULL, true);
    }
    fread(cfg_raw, 1, fsize, f);
    fclose(f);
    cfg_raw[fsize] = '\0';

    for(int i = 0; cfg_raw[i]; i++) {
        cfg_lines_count += (cfg_raw[i] == *(char*)CFG_DELIM);
    }
    #ifdef _DEBUG
    printf("entries found: %d\n", cfg_lines_count);
    #endif
    //TODO: check empty lines
    parsed_entries->ev = malloc(sizeof(entry_t) * cfg_lines_count);
    if(parsed_entries->ev == NULL) {
        err(NULL, NULL, true);
    }

    tok_ptr = strtok(cfg_raw, CFG_DELIM);
    if(tok_ptr == NULL) {
        err(ERRFMT, NULL, true);
    }
    while(tok_ptr != NULL) {
        #ifdef _DEBUG
        printf("Entry #%d: %s\n", parsed_entries->ec, tok_ptr);
        #endif
        if(strlen(tok_ptr) > 0) {
            entry = parse_config_line(tok_ptr);
            unsigned *cnt = &parsed_entries->ec;
            parsed_entries->ev[*cnt] = entry;
            *cnt += 1;
        }
        tok_ptr = strtok(NULL, CFG_DELIM);
    }
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
    entries_t entries = {
        .ec = 0,
        .ev = NULL
    };
    read_cfg(CFG_NAME, &entries);
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
    for(int i = 0; i < entries.ec; i++) {
        #ifdef _DEBUG
        printf("***** \ncmd: ");
        for(int j = 0; entries.ev[i].cmd[j]; j++) {
            printf("%s ", entries.ev[i].cmd[j]);
        }
        printf("\naction: %s\n", entries.ev[i].action);
        #endif

        free(entries.ev[i].cmd);
    }
    free(entries.ev);
    return 0;
}