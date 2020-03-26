#define LOG_IDENT "init2"
#define CFG_NAME "init2.cfg"
#define CFG_DELIM "\n"
#define _DEBUG

typedef struct {
    char **cmd;
    char *action;
} entry_t;

typedef struct {
    unsigned ec; //entries count
    entry_t* ev; //array of entries
} entries_t;