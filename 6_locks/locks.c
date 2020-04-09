#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "locks.h"

int errwrap(int ret) {
    if(ret == -1) {
        printf("%s\n", strerror(errno));
        exit(-1);
    }
    return ret;
}

int manage_lock(char *file, lock_t l_type, bool acq_lock) {
    int lfd;
    char *path[512];
    bool locked; 
    
    sprintf(path, "%s/%s.%s", LCK_DIR, file, LCK_SUFFIX);
    locked = access(path, F_OK) == 0;
    if(locked) {
        if(acq_lock) {
            switch (l_type) {

                case L_WR:
                    /* code */
                    break;
            }
            lfd = errwrap(open(path, O_CREAT));
        }
    } else {
        if(acq_lock) {

        } else {

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
    while(lock(file, L_WR, true) != 0) {
        if(!errord) {
            printf("%s\n", ERR_LOCKD);
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
        execlp(editor, editor, file);
        exit(-1);
    } else {
        waitpid(pid);
    }
    lock(file, L_WR, false);
    return 0;
}