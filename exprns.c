#include "exprns.h"
#include <string.h>

/* +------------------------------------------------+
 * +                               |                +
 * +------------------------------------------------+
 *  ` queue starts here				` stack starts here
 * */
#define	QUEUE_SIZE		MAX_TOKENS_PER_CELL
#define	STACK_SIZE		MAX_TOKENS_PER_CELL / 2
#define	MAX_EXPR_SIZE	QUEUE_SIZE + STACK_SIZE

struct Output {
	struct Token	output[MAX_EXPR_SIZE];
	unsigned int	que_i;
	unsigned int	stk_i;
};

/* These functions will return `ctype_number' to indicate
 * success.
 * */
static enum CellType push_number (struct Output *const, const struct Token *const);

enum CellType expr_solve_expr (struct Cell *const cell, const struct Token *expr)
{
	struct Output output = {
		.que_i = 0,
		.stk_i = QUEUE_SIZE
	};

	expr++;
	for (size_t k = 0; k < cell->expr_len; k++) {
		const enum TokenType kind = expr->type;
		if (kind == ttype_is_number)
			push_number(&output, expr);
	}

	return ctype_number;
}

static enum CellType push_number (struct Output *const out, const struct Token *const tok)
{
	if (out->que_i == QUEUE_SIZE)
		return ctype_error_expr_ovrflw;

	struct Token *this = &out->output[out->que_i++];
	this->as.number = tok->as.number;
	this->type = ttype_is_number;

	return ctype_number;
}

int main ()
{
	struct Token tokens[] = {
		{ .as.number = 454, .type = ttype_is_number, .is_hex = false },
		{ .as.number = 454, .type = ttype_is_number, .is_hex = false },
		{ .as.number = 454, .type = ttype_is_number, .is_hex = false },
		{ .as.number = 454, .type = ttype_is_number, .is_hex = false },
		{ .as.number = 454, .type = ttype_is_number, .is_hex = false },
		{ .as.number = 454, .type = ttype_is_number, .is_hex = false },
	};

	return 0;
}
