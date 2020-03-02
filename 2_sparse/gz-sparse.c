#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1000

char* slice(const char* arr, int from, int to) {
	if(from > BUF_SIZE || to > BUF_SIZE || to < from) {
		printf("Index error: [%d; %d]\n", from, to);
		exit(-1);
	}

	int res_size = (to - from) * sizeof(char) + 1;
	char* res = (char*)malloc(res_size);
	strncpy(res, arr+from, res_size);
	return res;
}

int main(int argc, char** argv) {
	if (argc == 1) {
		printf("Usage: gzip -cd arch.gz | %s <filename>\n", argv[0]);
		return -1;
	}
	
	int fd, nbytes, wr;
	bool zstart = false;
	unsigned long nonzr_pos = 0, zr_pos = 0, write_len = 0;
	char chunk[BUF_SIZE];
	char* filename = argv[1];
	char* write_buf;

	fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd == -1) {
		printf("Can't create new file\n");
		return -1;
	}
	while (1) {
		nbytes = read(STDIN_FILENO, chunk, BUF_SIZE);
		if(nbytes == EOF) {
			break;
		}
		zr_pos = 0;
		nonzr_pos = 0;
		for(int offset = 0; offset < nbytes; offset++) {
			if(chunk[offset] == '\0') {
				if(!zstart) {
					zstart = true;
					zr_pos = offset;
					write_len = zr_pos - nonzr_pos;
					//printf("slice1: [%d; %d], zr_pos: %d\n", nonzr_pos, write_len, zr_pos);
					write_buf = slice(chunk, nonzr_pos, zr_pos);
					//printf("before z: %s\n", write_buf);
					write(fd, write_buf, write_len);
					//printf("Written %d bytes at %d\n", wr, pos);
					free(write_buf);
				}
			} else {
				if(zstart) {
					zstart = false;
					nonzr_pos = offset;
					write_len = nonzr_pos - zr_pos;	
					//printf("Skipped %d zs\n", write_len);
					lseek(fd, write_len, SEEK_CUR);
				}
			}
		}
		// the rest
		if(!zstart) {
			//printf("slice2: [%d; %d]\n", nonzr_pos, nbytes);
			write_buf = slice(chunk, nonzr_pos, nbytes);
			write_len = nbytes - nonzr_pos;
			/*int pos = lseek(fd, 0, SEEK_CUR);
			printf("before: %d, to write: %d bytes \n", pos, write_len);
			printf("slice2val: %s\n", write_buf);*/
			write(fd, write_buf, write_len);
			/*pos = lseek(fd, 0, SEEK_CUR);
			printf("after: %d\n", pos);*/
			//write(fd, "|", 1);
			free(write_buf);
		} else {
			write_len = nbytes - zr_pos;
			lseek(fd, write_len, SEEK_CUR);
		}
		if(nbytes < BUF_SIZE - 1) {
			break;
		}
	}
	close(fd);
	return 0;
}
