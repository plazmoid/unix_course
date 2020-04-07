#include <stdio.h>
#include <stdbool.h>
#include "datatypes.h"
#include "life.h"

unsigned euclid_mod(int n, int mod) {
    return ((n % mod) + mod) % mod;
}

void normalize_coords(const CellField* cf, int* x, int* y) {
    int dx = *x;
    int dy = *y;
    *x = (dx >= 0 && dx < cf->fx)? dx : euclid_mod(dx, cf->fx);
    *y = (dy >= 0 && dy < cf->fy)? dy : euclid_mod(dy, cf->fy);
}

// all functions below are safe to use even with negative arguments
// euclid_mod will correct wrong value

char get_cell(const CellField* cf, int x, int y) {
    normalize_coords(cf, &x, &y);
    char c = cf->field[y][x];
    return c;
}

void set_cell(CellField* cf, int x, int y, char cell) {
    normalize_coords(cf, &x, &y);
    if(cell == CELL || cell == NO_CELL) {
        cf->field[y][x] = cell;
    } else {
        char buf[2];
        buf[0] = cell;
        buf[1] = '\0';
        err("Wrong cell type", buf, true);
    }
}

unsigned count_neighbours(const CellField* cf, int x, int y) {
    unsigned cnt = 0;
    int nx, ny;
    for(int dx = -1; dx <= 1; dx++) {
        for(int dy = -1; dy <= 1; dy++) {
            if(dx == 0 && dy == 0) {
                continue; // don't count ourself
            }
            nx = x + dx;
            ny = y + dy;
            if(get_cell(cf, nx, ny) == CELL) {
                cnt++;
            }
        }
    }
    return cnt;
}