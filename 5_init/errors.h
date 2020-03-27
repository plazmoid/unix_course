#define ERR_WRDE_BADCHAR "Illegal char"
#define ERR_WRDE_SYNTAX "Syntax error"
#define ERR_WRDE_NOSPACE "Out of memory"

#define ERRFMT "Wrong config format"
#define ERRNONZR "Process exited with code %d too much times, giving up"
#define ERRACT "Unknown action"

//cmd args action
#define ERRLNFMT "Wrong line format, see example in task 5"

#define FAILS_LIMIT 50

//print system or custom error
void err(char *msg, const char *arg, bool critical);