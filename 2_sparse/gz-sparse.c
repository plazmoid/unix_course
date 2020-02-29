#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define BUF_SIZE 100

int main(int argc, char** argv) {
	if (argc == 1) {
		printf("Usage: gzip -cd arch.gz | %s <filename>\n", argv[0]);
		return -1;
	}
	
	int fd, nbytes;
	bool zstart = false;
	unsigned long nonzr_pos = 0, zr_pos = 0, write_len = 0;
	char chunk[BUF_SIZE];
	char* filename = argv[1];
	char* write_buf;

	fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd == -1) {
		printf("Can't create new file");
		return -1;
	}
	while (1) {
		nbytes = read(STDIN_FILENO, chunk, BUF_SIZE);
		if(nbytes == EOF) {
			break;
		}
		for(int offset = 0; offset < nbytes; offset++) {
			if(chunk[offset] == '\0') {
				if(!zstart) {
					zstart = true;
					zr_pos = lseek(STDIN_FILENO, offset, SEEK_CUR);
					write_len = zr_pos - nonzr_pos - 1;
					write_buf = (char*)malloc(write_len);
					lseek(STDIN_FILENO, nonzr_pos, SEEK_SET);
					read(STDIN_FILENO, write_buf, write_len);
					write(fd, write_buf, write_len);
					free(write_buf);
				}
			} else {
				if(zstart) {
					zstart = false;
					nonzr_pos = lseek(STDIN_FILENO, offset, SEEK_CUR);
					lseek(fd, nonzr_pos, SEEK_SET);
				}
			}
		}
		if(nbytes < BUF_SIZE) {
			if(!zstart) {
				write_buf = (char*)malloc(nbytes);
				lseek(STDIN_FILENO, nonzr_pos, SEEK_SET);
				read(STDIN_FILENO, write_buf, nbytes);
				write(fd, write_buf, nbytes);
				free(write_buf);
			} else {
				write_len = lseek(STDIN_FILENO, 0, SEEK_END) - zr_pos;
				lseek(fd, write_len, SEEK_CUR);
			}
			break;
		}
	}
	close(fd);
	return 0;
}
