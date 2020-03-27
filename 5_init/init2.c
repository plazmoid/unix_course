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
#include <sys/wait.h>
#include <unistd.h>
#include "init2.h"
#include "errors.h"
#include "utils.h"
#include "configs.h"

/*static entries_t tasks = {
    .ec = 0,
    .ev = NULL
};

void sighup(int sig) {
    if(tasks.ev != NULL) {

    }
}*/

void daemonize() {
    int pid;
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
    pid = fork();
    if(pid == -1) {
        err(NULL, NULL, true);
    }
    if(pid != 0) {
        exit(0);
    }
    //close all fildes
    getrlimit(RLIMIT_NOFILE, &lim);
    for(int fd = 0; fd < lim.rlim_max; fd++) {
        close(fd);
    }
    chdir("/");
}

void run_tasks(entries_t *tasklist) {
    int pid, pid_cnt = 0, status;
    bool any_runnable = true;
    char **argv, *action, *full_cmd;
    syslog(LOG_INFO, "Running tasks");
    while(any_runnable) {
        any_runnable = false;
        for(int i = 0; i < tasklist->ec; i++) {
            entry_t *task = &tasklist->ev[i];
            if(task->finished) {
                continue;
            }
            any_runnable = true;
            argv = task->cmd;
            full_cmd = join_str(argv, " ", task->argc);
            pid = fork();
            if(pid == -1) {
                err(NULL, NULL, true);
            } else if(pid == 0) {
                execv(argv[0], argv);
                err(NULL, full_cmd, true);
            } else {
                task->pid = pid;
                syslog(LOG_INFO, "Running %s (%d)", full_cmd, pid);
                //TODO: wait in other cycle
                if(waitpid(pid, &status, 0) == -1) {
                    char errmsg[512];
                    sprintf(errmsg, "%s (%d)", full_cmd, pid);
                    err(NULL, errmsg, false);
                }
                action = task->action;
                syslog(LOG_INFO, "%d returned %d", pid, status);
                if(!strcmp(action, "wait")) {
                    task->finished = true;
                } else if(!strcmp(action, "respawn")) {

                }
            }
            free(full_cmd);
        }
    }
}

int main() {
    //signal(SIGHUP, sighup);
    entries_t tasks = {
        .ec = 0,
        .ev = NULL
    };
    openlog(LOG_IDENT, LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Starting");
    read_cfg(CFG_NAME, &tasks);
    daemonize();
    run_tasks(&tasks);
    for(int i = 0; i < tasks.ec; i++) {
        /*#ifdef _DEBUG
        syslog(LOG_DEBUG, "***** \ncmd: ");
        for(int j = 0; tasks.ev[i].cmd[j]; j++) {
            syslog(LOG_DEBUG, "%s ", tasks.ev[i].cmd[j]);
        }
        syslog(LOG_DEBUG, "\naction: %s\n", tasks.ev[i].action);
        #endif*/

        free(tasks.ev[i].cmd);
    }

    syslog(LOG_INFO, "Exiting");
    closelog();
    free(tasks.ev);
    return 0;
}