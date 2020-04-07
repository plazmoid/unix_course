#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "life.h"
#include "datatypes.h"

static CellField cfield = {
    .fx = 0,
    .fy = 0,
    .field = NULL
};

void err(char *msg, const char *arg, bool critical) {
    if(msg == NULL || strlen(msg) == 0) {
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

void game_of_life(CellField *cfield) {
    ActionTuple *action, *fld_actions;
    int neighbours = 0;
    int fld_actions_len = 0;
    fld_actions = malloc(cfield->fx * cfield->fy);
    DBG("mallocd actions\n");
    while(1) {
        fld_actions_len = 0;
        ////////////////
        system("clear");
        for(int y = 0; y < cfield->fy; y++) {
            for(int x = 0; x < cfield->fx; x++) {
                char c = get_cell(cfield, x, y);
                printf("%c", c);
            }
            printf(" %d\n", y);
        }
        sleep(2);
        ////////////////
        for(int x = 0; x < cfield->fx; x++) {
            for(int y = 0; y < cfield->fy; y++) {
                neighbours = count_neighbours(cfield, x, y);
                if(neighbours <= 3 && neighbours >= 2) {
                    if(get_cell(cfield, x, y) == NO_CELL) {
                        fld_actions[fld_actions_len] = (ActionTuple){
                            .c_action = SPAWN,
                            .fx = x,
                            .fy = y
                        };
                        fld_actions_len++;
                    }
                } else {
                    if(get_cell(cfield, x, y) == CELL) {
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
        for(; fld_actions_len >= 0; fld_actions_len--) {
            action = &fld_actions[fld_actions_len];
            if(action->c_action == SPAWN) {
                set_cell(cfield, action->fx, action->fy, true);
            } else if(action->c_action == REMOVE) {
                set_cell(cfield, action->fx, action->fy, false);
            }
        }
    }
    free(fld_actions);
}

int main(int argc, char **argv) {
    char cell;
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
    cfield.field = (char**)malloc(cfield.fy);
    if(cfield.field == NULL) {
        err(NULL, "field", true);
    }
    for(int row = 0; row < cfield.fy; row++) {
        cfield.field[row] = (char*)malloc(cfield.fx);
        fscanf(input, "%s\n", cfield.field[row]);
        if(ferror(input) > 0) {
            err(NULL, NULL, true);
        }
        //DBG("read row %d\n", row);
        for(int col = 0; col < cfield.fx; ) {
            cell = get_cell(&cfield, col, row);
            if(cell == NO_CELL || cell == CELL) {
                set_cell(&cfield, col, row, cell);
                col++;
                printf("%c", get_cell(&cfield, col, row));
            } else if(cell != '\n') {
                char errmsg[64], cellstr[2];
                sprintf(errmsg, ERR_WRONG_CELL_VAL, col+1, row+1);
                cellstr[0] = cell;
                cellstr[1] = '\0';
                err(errmsg, cellstr, true);
            }
        }
        printf("\n");
    }/*
    for(int y = 0; y < cfield.fy; y++) {
        for(int x = 0; x < cfield.fx; x++) {
            printf("%c", get_cell(&cfield, x, y));
        }
        printf(" %d\n", y);
    }*/
    sleep(3);
    game_of_life(&cfield);

    for(int row = 0; row < cfield.fy; row++) {
        free(cfield.field[row]);
    }
    free(cfield.field);
    return 0;
}