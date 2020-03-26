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
    int pids[tasklist->ec];
    char **argv, *action, *msg;
    syslog(LOG_INFO, "Running tasks");
    for(int i = 0; i < tasklist->ec; i++) {
        entry_t task = tasklist->ev[i];
        if(task.finished) {
            continue;
        }
        pid = fork();
        if(pid == -1) {
            err(NULL, NULL, true);
        } else if(pid == 0) {
            argv = task.cmd;
            msg = join_str(argv, " ", task.argc);
            syslog(LOG_INFO, "Running %s", msg);
            free(msg);
            execv(argv[0], argv);
            err(NULL, argv[0], true);
        } else {
            pids[pid_cnt] = pid;
            pid_cnt++;
            if(waitpid(pid, &status, 0) == -1) {
                char errmsg[512];
                msg = join_str(argv, " ", task.argc);
                sprintf(errmsg, "%s (%d)", msg, pid);
                err(NULL, errmsg, false);
                free(msg);
            }
            action = task.action;
            if(!strcmp(action, "wait")) {
                task.finished = true;
            } else if(!strcmp(action, "respawn")) {

            }
        }
    }
}

int main() {
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