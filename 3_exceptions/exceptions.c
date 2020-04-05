#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>
#include "exceptions.h"

void err(char *msg, const char *arg, bool critical) {
    if(msg == NULL || strlen(msg) == 0) {
        extern int errno;
        msg = strerror(errno);
    }
    if (arg != NULL && strlen(arg) > 0) {
        fprintf(stderr, "%s, '%s'\n", msg, arg);
    } else {
        fprintf(stderr, "%s\n", msg);
    }
    if(critical) {
        exit(-1);
    }
}

int main(int argc, char** argv) {
	int fd, nbytes;
	// indicates if currently reading zeroes
	bool zstart = false;
	char *parsed_int;
	char chunk[BUF_SIZE];
	if (argc < 3) {
		char *usage;
		sprintf(usage, "Usage: %s <in_1>, ..., <in_N>, <out>\n", argv[0]);
		err(usage, NULL, true);
	}
	for(int i = 1; i < argc - 1; i++) {
		int fd;
		if((fd = open(argv[i], O_RDONLY)) == -1) {
			printf("src %d: %s\n", i, argv[i]);
		} else {
			err(NULL, argv[i], false);
		}
		while (1) {
			// read stdin until eof
			nbytes = read(STDIN_FILENO, chunk, BUF_SIZE);
			if(nbytes == EOF) {
				break;
			}
			// read current chunk
			for(int offset = 0; offset < nbytes; offset++) {
			}
		}

		close(fd);
	}
	printf("into: %s\n", argv[argc-1]);
}
