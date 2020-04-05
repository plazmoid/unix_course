#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/wait.h>
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
	int fdi, fdo, nbytes, pid, status;
	char chunk[BUF_SIZE], ch;
	unsigned long long parsed_int_size = 0;
	char *parsed_int = calloc(1, SINGL_INT_BUF_SIZE);
	if (argc < 3) {
		char usage[128];
		sprintf(usage, "Usage: %s <in_1>, ..., <in_N>, <out>\n", argv[0]);
		err(usage, NULL, true);
	}
	if((fdo = creat(TMP_UNSORTED, 0660)) == -1) {
		err(NULL, TMP_UNSORTED, true);
	}
	for(int i = 1; i < argc - 1; i++) {
		if((fdi = open(argv[i], O_RDONLY)) == -1) {
			err(NULL, argv[i], false);
			continue;
		}
		parsed_int_size = 0;
		while (1) {
			// read file until eof
			nbytes = read(fdi, chunk, BUF_SIZE);
			if(nbytes == EOF) {
				break;
			}
			// read current chunk
			for(int offset = 0; offset < nbytes; offset++) {
				ch = chunk[offset];
				if(isdigit(ch)) {
					//don't catch first zero
					if(ch == 0 && parsed_int_size == 0) {
						continue;
					}
					// if int is too big for our buffer (with \0), realloc it
					if(parsed_int_size > 0 && parsed_int_size % (SINGL_INT_BUF_SIZE - 1) == 0) {
						DBG("int size is %llu, realloc\n", parsed_int_size);
						parsed_int = realloc(parsed_int, 
							SINGL_INT_BUF_SIZE * (parsed_int_size / (SINGL_INT_BUF_SIZE - 1) + 1));
					}
					parsed_int[parsed_int_size] = ch;
					parsed_int[++parsed_int_size] = '\0';
				} else {
					if(parsed_int_size > 0) {
						DBG("writing int with len %llu\n", parsed_int_size);
						if(write(fdo, parsed_int, parsed_int_size) == -1) {
							err(NULL, argv[i], false);
						} else {
							if(write(fdo, "\n", 1) == -1) {
								err(NULL, argv[i], false);
							}
						}
						memset(parsed_int, '\0', parsed_int_size);
						parsed_int_size = 0;
					}
				}
			}
			// if read less bytes than BUF_SIZE, it means we read the last chunk, break
			if(nbytes < BUF_SIZE) {
				break;
			}
		}
		close(fdi);
	}
	close(fdo);

	pid = fork();
    if(pid == -1) {
        err(NULL, NULL, true);
    } else if(pid == 0) {
		if((fdo = creat(argv[argc-1], 0660)) == -1) {
			err(NULL, argv[argc-1], true);
		}
		dup2(fdo, STDOUT_FILENO);
		close(fdo);
		execl("/bin/sort", "sort", "-n", TMP_UNSORTED, (char*)NULL);
		err(NULL, NULL, true);
	} else {
		if(waitpid(pid, &status, 0) == -1) {
			err(NULL, NULL, false);
		}
		unlink(TMP_UNSORTED);
	}
	return 0;
}