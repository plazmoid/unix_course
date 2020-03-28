#include <stdbool.h>
#include <sys/types.h>

#define LOG_IDENT "init2"
#define CFG_NAME "init2.cfg"
#define CFG_DELIM "\n"
#define PIDFILE "init2"
#define _DEBUG

typedef struct {
    unsigned argc;
    unsigned fails;
    pid_t pid;
    char **argv;
    char *exec_name;
    char *full_cmd;
    char *action;
    bool finished;
} entry_t;

typedef struct {
    unsigned ec; //entries count
    entry_t* ev; //array of entries
} entries_t;


int manage_pidfile(const char* name, bool create);

void run_tasks(entries_t *tasklist);

void cleanup();