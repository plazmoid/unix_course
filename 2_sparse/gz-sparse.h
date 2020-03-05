#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 4096
#define TRUNC_ERR "Truncation error\n"
#define MALL_ERR "Memory allocation error\n"
#define FILE_CR_ERR "Can't create new file\n"

//slice with malloc
char* slice(const char* arr, int from, int to);
