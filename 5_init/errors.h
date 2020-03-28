// wordexp error messages
#define ERR_WRDE_BADCHAR "Illegal char"
#define ERR_WRDE_SYNTAX "Syntax error"
#define ERR_WRDE_NOSPACE "Out of memory"

#define ERRFMT "Wrong config format"
#define ERRNONZR "Process exited with code %d too much times, giving up"
#define ERRACT "Unknown action"
#define ERRLNFMT "Wrong line format, see example in config"
#define ERRPIDWR "Error writing in pidfile"
#define ERRPIDEXS "Pidfile already exists"

#define FAILS_LIMIT 50

//print system or custom error
void err(char *msg, const char *arg, bool critical);