#ifndef S4TB_EXPR_H
#define S4TB_EXPR_H
#include "s4tb.h"

enum CellType expr_solve_expression (struct Cell *const, struct Token*);
void expr_solve_cloning (struct Cell *const, const unsigned short);

#endif
