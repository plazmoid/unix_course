#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "locks.h"

void err(char *msg, const char *arg, bool critical) {
    if(msg == NULL || strlen(msg) == 0) {
        msg = strerror(errno);
    }
    if (arg != NULL) {
        fprintf(stderr, "%s, '%s'\n", msg, arg);
    } else {
        fprintf(stderr, "%s\n", msg);
    }
    if(critical) {
        exit(-1);
    }
}

int errwrap(int ret) {
    if(ret == -1) {
        err(NULL, NULL, true);
    }
    return ret;
}

int manage_lock(char *file, lock_t l_type, bool acq_lock) {
    int lfd;
    char lck_path[512];
    char pid[16];
    bool locked; 
    
    sprintf(lck_path, "%s/%s.%s", LCK_DIR, file, LCK_SUFFIX);
    locked = access(lck_path, F_OK) == 0;
    if(locked) {
        if(acq_lock) {
            switch (l_type) {
                case L_WR:
                    return LE_LOCKD;

                case L_RD:
                    return 0;

                default:
                    err(ERR_UNK, NULL, false);
                    return LE_SOME;
            }
        } else {
            char read_pid[16];
            lfd = errwrap(open(lck_path, O_RDONLY));
            errwrap(read(lfd, read_pid, 15));
            close(lfd);
            sprintf(pid, "%d", (int)getpid());
            if(strncmp(pid, read_pid, strlen(pid)) == 0) {
                return unlink(lck_path);
            }
            return LE_PID_NE;
        }
    } else {
        if(acq_lock) {
            lfd = errwrap(open(lck_path, O_CREAT | O_WRONLY));
            sprintf(pid, "%d\n", (int)getpid());
            errwrap(write(lfd, pid, strlen(pid)));
            switch (l_type) {
                case L_WR:
                    if(write(lfd, "w", 1) == -1) {
                        close(lfd);
                        return LE_SOME;
                    }
                    break;
                case L_RD:
                    if(write(lfd, "r", 1) == -1) {
                        close(lfd);
                        return LE_SOME;
                    }
                    break;

                default:
                    err(ERR_UNK, NULL, false);
                    close(lfd);
                    return LE_SOME;
            }
            close(lfd);
            return 0;
        } else {
            return 0;
        }
    }
}

int main(int argc, char *argv[]) {
    int pid;
    char *editor, *file;
    bool errord = false;
    if(argc != 2) {
        printf("Usage: %s <file_to edit>", argv[0]);
        exit(1);
    }
    file = argv[1];
    while(manage_lock(file, L_WR, true) != 0) {
        if(!errord) {
            err(ERR_LOCKD, NULL, false);
            errord = true;
        }
        usleep(1000);
    }
    editor = getenv("EDITOR");
    if(editor == NULL) {
        editor = "nano";
    }
    pid = errwrap(fork());
    if(pid == 0) {
        execlp(editor, editor, file, (char*)NULL);
        exit(-1);
    } else {
        waitpid(pid, NULL, 0);
        if(manage_lock(file, L_WR, false) != 0) {
            err(ERR_RELEASE, file, false);
        }
    }
    return 0;
}