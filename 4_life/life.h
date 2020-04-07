#define SOCK "/tmp/life.sock"
#define INITFILE "init.txt"
#define FIELD_MAX_X 100
#define FIELD_MAX_Y 100

#define ERR_TOO_LARGE "Field is too big, try to reduce x or y in init file"
#define ERR_WRONG_CELL_VAL "Wrong cell value at (x:%d, y:%d)"

#define CELL 'o'
#define NO_CELL '.'

//#define _DEBUG

#ifdef _DEBUG
#define DBG(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define DBG(fmt, ...)
#endif

void err(char *msg, const char *arg, bool critical);
