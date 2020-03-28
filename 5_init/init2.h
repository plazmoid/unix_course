#include <stdbool.h>
#include <sys/types.h>

#define LOG_IDENT "init2"
#define CFG_NAME "init2.cfg"
#define CFG_DELIM "\n"
#define PIDFILE "init2"

//#define _DEBUG

#ifdef _DEBUG
#define DBG(fmt, ...) syslog(LOG_DEBUG, fmt, ##__VA_ARGS__)
#else
#define DBG(fmt, ...)
#endif

// holds one valid config entry
typedef struct {
    pid_t pid;
    // failed launches counter
    unsigned fails;
    // like arguments in main() function
    unsigned argc;
    char **argv;
    // full path to executable
    char *exec_name;
    // command with args in one line
    char *full_cmd;
    // wait/respawn
    char *action;
    // indicates that command was already ran
    bool finished;
} entry_t;

// holds all entries parsed from config and their amount
typedef struct {
    unsigned ec; //config entries count
    entry_t* ev; //array of entries
} entries_t;

// creates or removes pidfiles
int manage_pidfile(const char* name, bool create);

void run_tasks(entries_t *tasklist);

// kind of a destructor
void cleanup();