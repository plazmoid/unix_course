#define ERR_WRDE_BADCHAR "Illegal char"
#define ERR_WRDE_SYNTAX "Syntax error"
#define ERR_WRDE_NOSPACE "Out of memory"

#define ERRFMT "Wrong config format"
#define ERRACT "Unknown action"
#define ERRLNFMT "Wrong line format, see example in task 5"
//cmd args action

//print system or custom error
void err(char *msg, const char *arg, bool critical);