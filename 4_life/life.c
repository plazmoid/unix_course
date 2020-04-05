#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "life.h"

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

int main(int argc, char **argv) {
    char cell;
    CellField cfield = {
        .fx = 0,
        .fy = 0,
        .field = NULL
    };
    FILE *input = fopen(INITFILE, "r");
    if(input == NULL) {
        err(NULL, INITFILE, true);
    }
    if(fscanf(input, "%u %u\n", &cfield.fx, &cfield.fy) == EOF) {
        err(NULL, "x y", true);
    }
    DBG("scanned xy: %d %d\n", cfield.fx, cfield.fx);
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
        DBG("read row %d\n", row);
        for(int col = 0; col < cfield.fx; ) {
            cell = cfield.field[row][col];
            DBG("cell val: %c at %d %d\n", cell, row, col);
            if(cell == '0' || cell == '1') {
                cfield.field[row][col] = cell;
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

    for(int row = 0; row < cfield.fy; row++) {
        for(int col = 0; col < cfield.fx; col++) {
            printf("%c", cfield.field[row][col]);
        }
        printf("\n");
    }
    free(cfield.field);
    return 0;
}