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

// static fields are required by sighup_ and exit_ handlers
static char cfg_path[PATH_MAX];

// only daemon needs to use exit_handler
static bool handle_atexit = false;

// we need to exit() from different places with the same behaviour
void exit_handler() {
    if(handle_atexit) {
        // protection from calling by forks
        cleanup();
        manage_pidfile(PIDFILE, false);
        syslog(LOG_INFO, "Exiting");
        closelog();
    }
}

void sighup_handler(int sig) {
    entry_t *task;
    if(tasks.ev != NULL) {
        syslog(LOG_WARNING, 
                "SIGHUP received, killing and reloading all tasks");
        // kill all active tasks
        for(int i = 0; i < tasks.ec; i++) {
            task = &tasks.ev[i];
            // give child a chance to stop gracefully
            if(kill(task->pid, SIGTERM) > 0) {
                kill(task->pid, SIGKILL);
            }
            DBG("Killing %s (%d)", task->full_cmd, task->pid);
            task->finished = true;
        }
        cleanup();
        // reread config and launch tasks again
        read_cfg(cfg_path, &tasks);
        syslog(LOG_INFO, "Tasks reloaded");
        run_tasks(&tasks);
    }
}

void daemonize() {
    pid_t pid;
    struct rlimit lim;
    // signals below are already ignored
    // if we're called by real init
    if(getppid() != 1) {
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
    }
    // create new session 
    // to prevent child killing
    // when killing parent
    setsid();
    pid = fork();
    if(pid == -1) {
        err(NULL, NULL, true);
    }
    // exit current process to release terminal
    if(pid != 0) {
        exit(0);
    }
    // only daemon needs exit handler
    handle_atexit = true;
    // close all fildes
    getrlimit(RLIMIT_NOFILE, &lim);
    for(int fd = 0; fd < lim.rlim_max; fd++) {
        close(fd);
    }
    chdir("/");
}

int manage_pidfile(const char* name, bool create) {
    char pidfile_path[512];
    sprintf(pidfile_path, "/tmp/%s.pid", name);
    if(!create) {
        // if not create, then delete
        if(access(pidfile_path, F_OK) != -1 ) {
            DBG("Removing %s", pidfile_path);
            unlink(pidfile_path);
        }
    } else {
        FILE *f;
        char pidstr[32];
        DBG("Creating pidfile %s", pidfile_path);
        if(access(pidfile_path, F_OK) != -1 ) {
            // if file already exists, show error and rewrite it
            err(ERRPIDEXS, pidfile_path, false);
        }
        f = fopen(pidfile_path, "w");
        if(f == NULL) {
            err(NULL, pidfile_path, false);
            return -1;
        }
        // write pid
        sprintf(pidstr, "%d", getpid());
        if(!fwrite(pidstr, strlen(pidstr), 1, f)) {
            err(ERRPIDWR, pidfile_path, false);
            fclose(f);
            return -1;
        }
        // finally this function was useful, without it was UB
        if(fclose(f)) {
            err(NULL, pidfile_path, false);
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
        // exit when all tasks are finished
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
                // child doesn't need parent's exit handler
                handle_atexit = false;
                // create child's pidfile
                manage_pidfile(task->argv[0], true);
                execv(task->exec_name, task->argv);
                // if we're here, there was an error, just exit
                exit(-1);
            } else {
                // remember pid to waitpid() it later
                task->pid = pid;
                DBG("Running %s (%d)", task->full_cmd, pid);
            }
        }

        // wait for all children
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
            DBG("%s (%d) returned %d", 
                    task->full_cmd, task->pid, status);
            // remove child's pidfile
            manage_pidfile(task->argv[0], false);
            if(status == 0) {
                if(!strcmp(task->action, "wait")) {
                    task->finished = true;        
                } else if(!strcmp(task->action, "respawn")) {
                    // if the last lauch was successful, 
                    // reset fails counter
                    if(task->fails > 0) {
                        task->fails = 0;
                    }
                }
            } else {
                if(task->fails < FAILS_LIMIT) {
                    task->fails++;
                } else {
                    // if child exited with non zero code more
                    // than FAILS_LIMIT times, don't launch it anymore
                    char errmsg[512];
                    sprintf(errmsg, ERRNONZR, task->pid, status);
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
        // removing all childs' pidfiles
        manage_pidfile(task->argv[0], false);
        for(int j = 0; j < task->argc; j++) {
            free(task->argv[j]);
        }
        free(task->exec_name);
        free(task->full_cmd);
    }
    free(tasks.ev);
    // zeroing counter because on SIGHUP we'll use
    // this struct again
    tasks.ec = 0;
}

int main() {
    /*
    // use sigaction() instead of signal() to handle SIGHUP
    // to prevent handler from running only single time
    struct sigaction sa;
    sa.sa_handler = sighup_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    
    if(sigaction(SIGHUP, &sa, NULL) == -1) {
        err(NULL, NULL, true);
    }*/

    signal(SIGHUP, sighup_handler);
    openlog(LOG_IDENT, LOG_PID | LOG_CONS, LOG_DAEMON);
    atexit(exit_handler);
    syslog(LOG_INFO, "Starting");
    // remember full path to config because
    // we'll be in / after daemonizing
    getcwd(cfg_path, PATH_MAX);
    strcat(cfg_path, "/");
    strcat(cfg_path, CFG_NAME);
    daemonize();
    if(manage_pidfile(PIDFILE, true) == -1) {
        exit(-1);
    }
    read_cfg(cfg_path, &tasks);
    run_tasks(&tasks);
    return 0;
}