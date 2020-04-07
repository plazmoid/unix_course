typedef struct {
    unsigned fx; // field sizes
    unsigned fy;
    char ** field; // 2D-field
} CellField;

typedef enum {
    SPAWN,
    REMOVE
} CellAction;

typedef struct {
    unsigned fx;
    unsigned fy;
    CellAction c_action;
} ActionTuple;


unsigned euclid_mod(int n, int mod);

unsigned count_neighbours(const CellField* cf, const int x, const int y);

void normalize_coords(const CellField* cf, int* x, int* y);

char get_cell(const CellField* cf, int x, int y);

void set_cell(CellField* cf, int x, int y, char cell);