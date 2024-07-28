#include "exprns.h"
#include <string.h>

/* This is an implementation of the Shutting Yard Algorithm:
 * https://mathcenter.oxford.emory.edu/site/cs171/shuntingYardAlgorithm/
 * */

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
	long double		ans;
};

static enum CellType push_queue (struct Output *const, const struct Token *const);
static enum CellType push_stack (struct Output *const, const struct Token *const);

static bool_t exchange_operators (const enum TokenType, const enum TokenType);
static enum CellType solve (struct Output *const);

enum CellType expr_solve_expr (struct Cell *const cell, const struct Token *expr)
{
	struct Output output = {
		.que_i = 0,
		.stk_i = QUEUE_SIZE
	};

	expr++;
	for (size_t k = 1; k < 10; k++) {
		const enum TokenType kind = expr->type;

		switch (expr->type) {
			case ttype_is_number:
				push_queue(&output, expr);
				break;

			case ttype_left_p:
			case ttype_right_p:
			case ttype_sub_sign:
			case ttype_div_sign:
			case ttype_mul_sign:
			case ttype_add_sign:
				push_stack(&output, expr);
				break;

			case ttype_reference:
				break;


			default:
				return ctype_error_malformedex;
		}

		expr++;
	}

	return solve(&output);
}

static enum CellType push_queue (struct Output *const out, const struct Token *const tok)
{
	if (out->que_i == QUEUE_SIZE)
		return ctype_error_expr_ovrflw;

	struct Token *this = &out->output[out->que_i++];
	this->as.number = tok->as.number;
	this->type = tok->type;

	return ctype_number;
}

static enum CellType push_stack (struct Output *const out, const struct Token *const tok)
{
	if (out->stk_i == MAX_EXPR_SIZE)
		return ctype_error_expr_ovrflw;

	enum CellType status = ctype_number;
	if (out->stk_i == QUEUE_SIZE)
		goto push;

	do {
		const struct Token *top = &out->output[out->stk_i - 1];

		if (!exchange_operators(top->type, tok->type))
			break;

		status = push_queue(out, top);
		out->stk_i--;
	} while ((out->stk_i > QUEUE_SIZE) && !CELL_IS_ERR(status));

	push: {
		out->output[out->stk_i++].type = tok->type;
		return status;
	}
}

static bool_t exchange_operators (const enum TokenType prev, const enum TokenType new)
{
	if (prev == new) return true;

	const bool_t newslow = ((new == '+') || (new == '-'));
	return newslow ? true : false;
}

static enum CellType solve (struct Output *const out)
{
	enum CellType status = ctype_number;
	register unsigned short k;

	for (k = --out->stk_i; (k >= QUEUE_SIZE) && !CELL_IS_ERR(status); k--)
		status = push_queue(out, &out->output[k]);

	for (k = 0; k < out->que_i; k++) {
		struct Token* a = &out->output[k];

		if (a->type == ttype_is_number)
			printf("%Lf ", a->as.number);
		else
			printf("%c ", a->type);
	}

	return status;
}

int main ()
{
	struct Token tokens[] = {
		{ .as.number = 0,	.type = ttype_expression,	.is_hex = false },

		{ .as.number = 4,	.type = ttype_is_number,	.is_hex = false },
		{ .as.number = 0,	.type = ttype_add_sign,		.is_hex = false },

		{ .as.number = 5,	.type = ttype_is_number,	.is_hex = false },
		{ .as.number = 0,	.type = ttype_div_sign,		.is_hex = false },

		{ .as.number = 6,	.type = ttype_is_number,	.is_hex = false },
		{ .as.number = 0,	.type = ttype_sub_sign,		.is_hex = false },

		{ .as.number = 7,	.type = ttype_is_number,	.is_hex = false },
		{ .as.number = 0,	.type = ttype_add_sign,		.is_hex = false },

		{ .as.number = 8,	.type = ttype_is_number,	.is_hex = false },
	};

	printf("\nstatus: %d\n", expr_solve_expr(NULL, tokens));
	return 0;
}

