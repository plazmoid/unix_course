#include <fcntl.h>

#define LCK_DIR "/tmp"
#define LCK_SUFFIX "lck"
#define LCKF_WR "w"
#define LCKF_RD "r"

#define ERR_LOCKD "This file is already locked, waiting for lock release"
#define ERR_UNK "Unknown lock type"
#define ERR_RELEASE "Failed to release lock"
#define ERR_FMT "Wrong lockfile format"

typedef enum {
    L_WR = O_WRONLY,
    L_RD = O_RDONLY
} lock_t;

typedef enum {
    // current pid and parsed from pidfile are not equal
    LE_PID_NE = 1, 
    // file is locked
    LE_LOCKD,
    // general error, see more in errno
    LE_SOME
} lock_err;