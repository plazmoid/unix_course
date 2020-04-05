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
	// fildes for input files; for output; amount of read bytes; sort pid; sort return code
	int fdi, fdo, nbytes, pid, status;
	// chunk to store data from files; current char
	char chunk[BUF_SIZE], ch;
	// digits in current int
	unsigned long long parsed_int_size = 0;
	// i'm storing int in char* because it may be really big
	char *parsed_int = calloc(1, SINGL_INT_BUF_SIZE);
	if (argc < 3) {
		char usage[128];
		sprintf(usage, "Usage: %s <in_1>, ..., <in_N>, <out>\n", argv[0]);
		err(usage, NULL, true);
	}
	// write in temp file first because we can't $ sort qwe > qwe
	if((fdo = creat(TMP_UNSORTED, 0660)) == -1) {
		err(NULL, TMP_UNSORTED, true);
	}
	// loop over every input file
	for(int i = 1; i < argc - 1; i++) {
		if((fdi = open(argv[i], O_RDONLY)) == -1) {
			// can't open? just skip
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
					// if int is too big for our buffer (with \0), realloc it
					if(parsed_int_size > 0 && parsed_int_size % (SINGL_INT_BUF_SIZE - 1) == 0) {
						parsed_int = realloc(parsed_int, 
							SINGL_INT_BUF_SIZE * (parsed_int_size / (SINGL_INT_BUF_SIZE - 1) + 1));
					}
					parsed_int[parsed_int_size] = ch;
					parsed_int[++parsed_int_size] = '\0';
				} else {
					if(parsed_int_size > 0) {
						// if scanned whole number, write it into tmp file
						if(write(fdo, parsed_int, parsed_int_size) == -1) {
							err(NULL, argv[i], false);
						} else {
							if(write(fdo, "\n", 1) == -1) {
								err(NULL, argv[i], false);
							}
						}
						// free space for the next number (instead of free/calloc)
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

	// sort
	pid = fork();
    if(pid == -1) {
        err(NULL, NULL, true);
    } else if(pid == 0) {
		// we'll sort into outfile parsed from args
		if((fdo = creat(argv[argc-1], 0660)) == -1) {
			err(NULL, argv[argc-1], true);
		}
		// redirect into outfile
		dup2(fdo, STDOUT_FILENO);
		close(fdo);
		execl("/bin/sort", "sort", "-n", TMP_UNSORTED, (char*)NULL);
		// sort exited with error, print it
		err(NULL, NULL, true);
	} else {
		if(waitpid(pid, &status, 0) == -1) {
			err(NULL, NULL, false);
		}
		unlink(TMP_UNSORTED);
		// print exit status of sort
		if(status != 0) {
			char errmsg[64];
			sprintf(errmsg, "Sort exited with status %d", status);
			err(errmsg, NULL, false);
		}
	}
	return 0;
}