#include <stdbool.h>
#define LOG_IDENT "init2"
#define CFG_NAME "init2.cfg"
#define CFG_DELIM "\n"
//#define _DEBUG

typedef struct {
    unsigned argc;
    unsigned fails;
    int pid;
    char **cmd;
    char *action;
    bool finished;
} entry_t;

typedef struct {
    unsigned ec; //entries count
    entry_t* ev; //array of entries
} entries_t;