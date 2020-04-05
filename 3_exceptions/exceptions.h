#define BUF_SIZE 1024
#define SINGL_INT_BUF_SIZE 2048

//#define _DEBUG

#ifdef _DEBUG
#define DBG(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#else
#define DBG(fmt, ...)
#endif