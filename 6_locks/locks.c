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
    // lock fd, split index
    int lfd, n_idx;
    // path to lockfile, pid read from lockfile
    // raw buffer to read in, lock char (w/r), for tmp vals
    char lck_path[512], read_pid[16], buf[64], lc, *tmp;
    // pid of the current process
    char pid[16];
    // if file is already locked
    bool locked; 
    
    sprintf(lck_path, "%s/%s.%s", LCK_DIR, file, LCK_SUFFIX);
    locked = access(lck_path, F_OK) == 0;
    if(locked) {
        // if file has lock, parse it
        lfd = errwrap(open(lck_path, O_RDONLY));
        errwrap(read(lfd, buf, 63));
        close(lfd);
        // poor analogue of split
        tmp = strchr(buf, '\n');
        if(tmp == NULL) {
            err(ERR_FMT, lck_path, true);
        }
        n_idx = (int)(tmp - buf);
        // read pid from lockfile
        strncpy(read_pid, buf, n_idx);
        // and the type of lock
        lc = buf[n_idx+1];
        // check lock type (r/w)
        if(lc != *(char*)LCKF_WR && lc != *(char*)LCKF_RD) {
            err(ERR_FMT, lck_path, true);
        }
        if(acq_lock) {
            // and we want to lock it again
            // in case of existing exclusive lock
            // fail to lock immediately
            if(lc == *(char*)LCKF_WR) {
                return LE_LOCKD;
            }
            // if lock is shared
            switch (l_type) {
                // can't lock exclusively
                case L_WR:
                    return LE_LOCKD;
                // but can create infinite shared locks
                case L_RD:
                    return 0;

                default:
                    err(ERR_UNK, NULL, false);
                    return LE_SOME;
            }
        } else {
            // if want to unlock
            sprintf(pid, "%d", (int)getpid());
            // compare pids to ensure that we've created a lock
            if(strcmp(pid, read_pid) == 0) {
                return unlink(lck_path);
            }
            // or realise that we're 
            // trying to remove foreign lock
            return LE_PID_NE;
        }
    } else {
        // if want to lock and there are no lockfiles
        if(acq_lock) {
            lfd = errwrap(open(lck_path, O_CREAT | O_WRONLY, 0600));
            sprintf(pid, "%d\n", (int)getpid());
            // write pid
            if(write(lfd, pid, strlen(pid)) == -1) {
                close(lfd);
                return LE_SOME;
            }
            // and lock type
            switch (l_type) {
                case L_WR:
                    if(write(lfd, LCKF_WR, 1) == -1) {
                        close(lfd);
                        return LE_SOME;
                    }
                    break;
                case L_RD:
                    if(write(lfd, LCKF_RD, 1) == -1) {
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
    // text editor, file to edit
    char *editor, *file;
    // to show blocking error only once
    bool errord = false;
    if(argc != 2) {
        printf("Usage: %s <file_to edit>\n", argv[0]);
        exit(1);
    }
    file = argv[1];
    while(manage_lock(file, L_WR, true) != 0) {
        if(!errord) {
            err(ERR_LOCKD, NULL, false);
            errord = true;
        }
        // wait for lock release
        usleep(1000);
    }
    editor = getenv("EDITOR");
    if(editor == NULL) {
        editor = "nano";
    }
    // let user edit file
    pid = errwrap(fork());
    if(pid == 0) {
        execlp(editor, editor, file, (char*)NULL);
        exit(-1);
    } else {
        waitpid(pid, NULL, 0);
        // release lock
        if(manage_lock(file, L_WR, false) != 0) {
            err(ERR_RELEASE, file, false);
        }
    }
    return 0;
}