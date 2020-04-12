#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <time.h>
#include "life.h"
#include "datatypes.h"

// field with cells
static CellField cfield = {
    .fx = 0,
    .fy = 0,
    .field = NULL
};

static int listen_socket_fd;
static struct sockaddr_in addr;


void err(char *msg, const char *arg, bool critical) {
    if(msg == NULL || strlen(msg) == 0) {
        msg = strerror(errno);
    }
    if (arg != NULL) {
        fprintf(stderr, "%s, '%s'\n", msg, arg);
    } else {
        fprintf(stderr, "%s\n", msg);
    }
    if(critical) {
        exit(-1);
    }
}

// handy error checker
int errwrap(int ret) {
    if(ret == -1) {
        err(NULL, NULL, true);
        return -1;
    } else {
        return ret;
    }
}

void sock_init() {
	int port = 31338;
    int opt = 1;
    listen_socket_fd = errwrap(socket(AF_INET, SOCK_STREAM, 0));
    int flags = errwrap(fcntl(listen_socket_fd, F_GETFL));
    // make i/o with socket non-blocking
    errwrap(fcntl(listen_socket_fd, F_SETFL, flags | O_NONBLOCK)); 
    errwrap(setsockopt(listen_socket_fd, SOL_SOCKET, SO_REUSEADDR, 
            &opt, sizeof(int)));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    errwrap(bind(listen_socket_fd, (struct sockaddr*)&addr, sizeof(addr)));
    errwrap(listen(listen_socket_fd, 5));
	fprintf(stderr, "Listening on localhost:%d\n", port);
}

void client_handler(CellField *cfield) {
    // we need to send the whole field with \n
    char buf[(cfield->fx + 1)*cfield->fy];
    int bytes_written = 0;
    int client_socket_fd = accept(listen_socket_fd, NULL, NULL);
    if(client_socket_fd == -1) {
        if(errno != EWOULDBLOCK) {
            err(NULL, NULL, true);
        }
    } else {
        // fill send buffer line by line
        for(int y = 0; y < cfield->fy; y++) {
            memcpy(buf+bytes_written, cfield->field[y], cfield->fx);
            bytes_written += cfield->fx;
            buf[bytes_written] = '\n';
            bytes_written++;
        }
        errwrap(send(client_socket_fd, buf, bytes_written, 0));
        close(client_socket_fd);
    }
}

void game_of_life(CellField *cfield) {
    // &ActionTuple; array of actions
    ActionTuple *action, *fld_actions;
	// current cell
	char cell;
    int fld_actions_len = 0;
    int neighbours = 0;
    // if true, then we can make next life iteration
    bool sync_ready = true;
    // fildes of sync file
    int sync_fd;
    time_t real_time = 0, delta = 0;
    struct stat file_stat;
    fld_actions = malloc(cfield->fx * cfield->fy);
    // game cycle
    while(1) {
        fld_actions_len = 0;
        // lookup for every cell
        for(int x = 0; x < cfield->fx; x++) {
            for(int y = 0; y < cfield->fy; y++) {
                // implementing game rules
                neighbours = count_neighbours(cfield, x, y);
                cell = get_cell(cfield, x, y);
				if(neighbours == 3 || neighbours == 2) {
                    if(cell == NO_CELL && neighbours == 3) {
                        // its better to perform all changes after full field scan
                        // so just store them
                        fld_actions[fld_actions_len] = (ActionTuple){
                            .c_action = SPAWN,
                            .fx = x,
                            .fy = y
                        };
                        fld_actions_len++;
                    }
                } else {
                    if(cell == CELL) {
                        fld_actions[fld_actions_len] = (ActionTuple){
                            .c_action = REMOVE,
                            .fx = x,
                            .fy = y
                        };
                        fld_actions_len++;
                    }
                }
            }
        }
        fld_actions_len--;
        // apply all changes
        for(; fld_actions_len >= 0; fld_actions_len--) {
            action = &fld_actions[fld_actions_len];
            if(action->c_action == SPAWN) {
                set_cell(cfield, action->fx, action->fy, CELL);
            } else if(action->c_action == REMOVE) {
                set_cell(cfield, action->fx, action->fy, NO_CELL);
            }
        }
        // now we have to wait for sync permit
        sync_ready = false;
        // touch sync file
        sync_fd = creat(SYNCFILE, 0666);
        if(sync_fd == -1) {
            err(NULL, SYNCFILE, true);
        }
		close(sync_fd);
        while(!sync_ready) {
            // handle connections while waiting
            client_handler(cfield);
            errwrap(stat(SYNCFILE, &file_stat));
            time(&real_time);
            delta = real_time - file_stat.st_mtime; // seconds
            if(delta == 1) {
                sync_ready = true;
            } else if(delta > 1) { // print warn message if sync was too long
                char delta_val[32];
                sprintf(delta_val, "%ld", delta);
                err(ERR_LONG_TICK, delta_val, false);
                sync_ready = true;
            }
            usleep(500); // just to not overload processor
        }
    }
    // actually this string is never gonna execute, because program could be interrupted only by ^C
    // but whatever
    free(fld_actions);
}

int main(int argc, char **argv) {
    char cell;
    sock_init();
    FILE *input = fopen(INITFILE, "r");
    if(input == NULL) {
        err(NULL, INITFILE, true);
    }
    if(fscanf(input, "%u %u\n", &cfield.fx, &cfield.fy) == EOF) {
        err(NULL, "x y", true);
    }
    if(cfield.fx > FIELD_MAX_X || cfield.fy > FIELD_MAX_Y) {
        err(ERR_TOO_LARGE, NULL, true);
    }
    cfield.field = calloc(cfield.fy, sizeof(char*));
    if(cfield.field == NULL) {
        err(NULL, "field", true);
    }
    for(int row = 0; row < cfield.fy; row++) {
        cfield.field[row] = (char*)calloc(1, cfield.fx);
        // read from initfile line by line
        fscanf(input, "%s\n", cfield.field[row]);
        if(ferror(input) > 0) {
            err(NULL, NULL, true);
        }
        // check loaded string for errors
        for(int col = 0; col < cfield.fx; ) {
            cell = get_cell(&cfield, col, row);
            if(cell == NO_CELL || cell == CELL) {
                set_cell(&cfield, col, row, cell);
                col++;
            } else if(cell != '\n') {
                char errmsg[64], cellstr[2];
                sprintf(errmsg, ERR_WRONG_CELL_VAL, col+1, row+1);
                cellstr[0] = cell;
                cellstr[1] = '\0';
                err(errmsg, cellstr, true);
            }
        }
    }
    game_of_life(&cfield);

    for(int row = 0; row < cfield.fy; row++) {
        free(cfield.field[row]);
    }
    free(cfield.field);
    close(listen_socket_fd);
    return 0;
}
