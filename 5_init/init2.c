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
#include "configs.h"

static entries_t tasks = {
    .ec = 0,
    .ev = NULL
};

static char cfg_path[PATH_MAX];


void sighup_handler(int sig) {
    entry_t *task;
    if(tasks.ev != NULL) {
        syslog(LOG_WARNING, 
                "SIGHUP received, killing and reloading all tasks");
        for(int i = 0; i < tasks.ec; i++) {
            task = &tasks.ev[i];
            if(kill(task->pid, SIGTERM) == -1) {
                err(NULL, task->full_cmd, false);
            }
            task->finished = true;
        }
        cleanup();
        read_cfg(cfg_path, &tasks);
        syslog(LOG_INFO, "Tasks reloaded");
        run_tasks(&tasks);
    }
}

void daemonize() {
    pid_t pid;
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
    pid = fork();
    if(pid == -1) {
        err(NULL, NULL, true);
    }
    //exit current process to release terminal
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

int manage_pidfile(const char* name, bool create) {
    char pidfile_path[512];
    sprintf(pidfile_path, "/tmp/%s.pid", name);
    //strcpy(pidfile_path, "/tmp/");
    //strcat(pidfile_path, name);
    if(!create) {
        if(unlink(pidfile_path) == -1) {
            err(NULL, pidfile_path, false);
        }
    } else {
        FILE *f;
        char pidstr[32];
        if(access(pidfile_path, F_OK) != -1 ) {
            err(ERRPIDEXS, pidfile_path, false);
            return -1;
        }
        f = fopen(pidfile_path, "w");
        if(f == NULL) {
            err(NULL, pidfile_path, false);
            return -1;
        }
        sprintf(pidstr, "%d", getpid());
        if(!fwrite(pidstr, strlen(pidstr), 1, f)) {
            err(ERRPIDWR, pidfile_path, false);
            return -1;
        }
    }
    return 0;
}

void run_tasks(entries_t *tasklist) {
    entry_t *task;
    pid_t pid;
    int status;
    bool any_runnable = true;
    syslog(LOG_INFO, "Running tasks");
    while(any_runnable) {
        any_runnable = false;
        for(int i = 0; i < tasklist->ec; i++) {
            task = &tasklist->ev[i];
            if(task->finished) {
                continue;
            }
            any_runnable = true;
            pid = fork();
            if(pid == -1) {
                err(NULL, NULL, true);
            } else if(pid == 0) {
                manage_pidfile(task->argv[0], true);
                execv(task->exec_name, task->argv);
                exit(-1);
            } else {
                task->pid = pid;
                #ifdef _DEBUG
                syslog(LOG_DEBUG, "Running %s (%d)", task->full_cmd, pid);
                #endif
            }
        }

        for(int i = 0; i < tasklist->ec; i++) {
            task = &tasklist->ev[i];
            if(task->finished) {
                continue;
            }
            if(waitpid(task->pid, &status, 0) == -1) {
                char errmsg[64];
                sprintf(errmsg, "%d", task->pid);
                err(NULL, errmsg, false);
            }
            manage_pidfile(task->argv[0], false);
            #ifdef _DEBUG
            syslog(LOG_DEBUG, "%d (%s) returned %d", 
                    task->pid, task->full_cmd, status);
            #endif
            if(status == 0) {
                if(!strcmp(task->action, "wait")) {
                    task->finished = true;        
                } else if(!strcmp(task->action, "respawn")) {
                    if(task->fails > 0) {
                        task->fails = 0;
                    }
                }
            } else {
                if(task->fails < FAILS_LIMIT) {
                    task->fails++;
                } else {
                    char errmsg[512];
                    sprintf(errmsg, ERRNONZR, status);
                    err(errmsg, task->full_cmd, false);
                    task->finished = true;
                }
            }
        }
    }
}

void cleanup() {
    entry_t *task;
    for(int i = 0; i < tasks.ec; i++) {
        task = &tasks.ev[i];
        for(int j = 0; j < task->argc; j++) {
            free(task->argv[j]);
        }
        free(task->exec_name);
        free(task->full_cmd);
    }
    free(tasks.ev);
    manage_pidfile(PIDFILE, false);
}

int main() {
    getcwd(cfg_path, PATH_MAX);
    strcat(cfg_path, "/");
    strcat(cfg_path, CFG_NAME);
    if(manage_pidfile(PIDFILE, true) == -1) {
        return -1;
    }
    daemonize();
    signal(SIGHUP, sighup_handler);
    openlog(LOG_IDENT, LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Starting");
    read_cfg(cfg_path, &tasks);
    run_tasks(&tasks);
    cleanup();
    syslog(LOG_INFO, "Exiting");
    closelog();
    return 0;
}