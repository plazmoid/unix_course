#include <fcntl.h>

#define LCK_DIR "/tmp"
#define LCK_SUFFIX "lck"

#define ERR_LOCKD "This file is already locked, waiting for lock release"
#define ERR_UNK "Unknown lock type"
#define ERR_RELEASE "Failed to release lock"

typedef enum {
    L_WR = O_WRONLY,
    L_RD = O_RDONLY
} lock_t;

typedef enum {
    LE_PID_NE = 1,
    LE_LOCKD,
    LE_SOME
} lock_err;