#include <fcntl.h>

#define LCK_DIR "/tmp"
#define LCK_SUFFIX "lck"

#define ERR_LOCKD "This file is already locked, waiting for lock release"

typedef enum {
    L_WR = O_WRONLY,
    L_RD = O_RDONLY
} lock_t;