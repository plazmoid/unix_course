#include "gz-sparse.h"

char* slice(const char* arr, int from, int to) {
	if(from > BUF_SIZE || to > BUF_SIZE || to < from) {
		printf("Index error: [%d; %d]\n", from, to);
		exit(1);
	}

	//inclusive slice
	int res_size = (to - from) * sizeof(char) + 1;
	char* res;
	if ((res = (char*)malloc(res_size)) == NULL) {
		printf(MALL_ERR);
		exit(1);
	}
	strncpy(res, arr+from, res_size);
	return res;
}

int main(int argc, char** argv) {
	if (argc == 1) {
		printf("Usage: gzip -cd arch.gz | %s <filename>\n", argv[0]);
		return 1;
	}
	
	// fildes of result file, amount of read bytes from current chunk
	int fd, nbytes;
	// indicates if currently reading zeroes
	bool zstart = false;
	unsigned long nonzr_pos = 0, // last pos of non-zero byte
				  zr_pos = 0,  // last pos of zero byte
				  write_len = 0, // length of data to write
				  pos; // current pos
	char chunk[BUF_SIZE]; // we read from stdin by chunks
	char* filename = argv[1];
	char* write_buf; // slices are stored here

	// open new file with removing if existed
	fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0660);
	if (fd == -1) {
		printf(FILE_CR_ERR);
		return 1;
	}
	while (1) {
		// read stdin until eof
		nbytes = read(STDIN_FILENO, chunk, BUF_SIZE);
		if(nbytes == EOF) {
			break;
		}
		zr_pos = 0;
		nonzr_pos = 0;
		// read current chunk
		for(int offset = 0; offset < nbytes; offset++) {
			if(chunk[offset] == '\0') {
				// if zeroes started
				if(!zstart) {
					// write non-zero part of chunk into out file
					zstart = true;
					zr_pos = offset;
					write_len = zr_pos - nonzr_pos;
					write_buf = slice(chunk, nonzr_pos, zr_pos);
					write(fd, write_buf, write_len);
					free(write_buf);
				}
			} else {
				// if non-zeroes started
				if(zstart) {
					// just move caret, skipping zeroes
					zstart = false;
					nonzr_pos = offset;
					write_len = nonzr_pos - zr_pos;	
					lseek(fd, write_len, SEEK_CUR);
				}
			}
		}
		// write the last part of chunk
		if(!zstart) {
			write_buf = slice(chunk, nonzr_pos, nbytes);
			write_len = nbytes - nonzr_pos;
			write(fd, write_buf, write_len);
			free(write_buf);
		} else {
			write_len = nbytes - zr_pos;
			pos = lseek(fd, 0, SEEK_CUR);
			if(ftruncate(fd, pos + write_len) == -1) {
				printf(TRUNC_ERR);
				return 1;
			}
			lseek(fd, write_len, SEEK_CUR);
		}
		// if read less bytes than BUF_SIZE, it means we read the last chunk, break
		if(nbytes < BUF_SIZE) {
			break;
		}
	}
	close(fd);
	return 0;
}
