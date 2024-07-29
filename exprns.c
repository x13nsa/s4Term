#include "exprns.h"
#include <string.h>

/* This is an implementation of the Shutting Yard Algorithm:
 * https://mathcenter.oxford.emory.edu/site/cs171/shuntingYardAlgorithm/
 *
 * +------------------------------------------------+
 * +                        |                       +
 * +------------------------------------------------+
 *  ` queue starts here		` stack starts here
 *  				\		  /
 *  		MAX_TOKENS_PER_CELL / 2 consecutive
 *  		values whatever they are.
 * */
#define	MAX_CONTAINER_CAP	MAX_TOKENS_PER_CELL / 2
#define	QUEUE_SIZE			MAX_CONTAINER_CAP
#define	STACK_SIZE			MAX_CONTAINER_CAP

struct Output {
	struct Token	output[MAX_TOKENS_PER_CELL];
	long double		*final;
	unsigned int	que_i;
	unsigned int	stk_i;
};

static enum CellType push_queue (struct Output *const, const struct Token *const);
static enum CellType push_stack (struct Output *const, const struct Token *const);

static enum CellType right_par_found (struct Output *const);
static bool_t exchange_operators (const enum TokenType, const enum TokenType);

static enum CellType solve (struct Output *const);
static enum CellType perform_operation (long double*, unsigned short*, const enum TokenType);

enum CellType expr_solve_expr (struct Cell *const cell, const struct Token *expr)
{
	struct Output output;
	memset(&output, 0, sizeof(struct Output));

	output.stk_i = QUEUE_SIZE;
	output.final = &cell->as.number;

	for (size_t k = 1; k < cell->expr_len; k++) {
		expr++;

		switch (expr->type) {
			case ttype_is_number:
			case ttype_reference:				// TODO
				push_queue(&output, expr);
				break;

			case ttype_right_p:  case ttype_left_p:
			case ttype_sub_sign: case ttype_div_sign:
			case ttype_mul_sign: case ttype_add_sign:
				push_stack(&output, expr);
				break;

			default:
				return ctype_error_malformedex;
		}
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
	if (out->stk_i == MAX_TOKENS_PER_CELL)
		return ctype_error_expr_ovrflw;

	enum CellType status = ctype_number;
	if ((out->stk_i == QUEUE_SIZE) || (tok->type == ttype_left_p))
		goto push;

	if (tok->type == ttype_right_p)
		return right_par_found(out);

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

static enum CellType right_par_found (struct Output *const out)
{
	enum CellType status = ctype_number;
	struct Token *top;

	do {
		top = &out->output[--out->stk_i];
		if (top->type == ttype_left_p)
			break;
		status = push_queue(out, top);
	} while ((out->stk_i > QUEUE_SIZE) && !CELL_IS_ERR(status));

	return status;
}

static bool_t exchange_operators (const enum TokenType prev, const enum TokenType new)
{
	if (prev == ttype_left_p) return false;
	if (prev == new) return true;

	static const unsigned short same[] = {
		(unsigned short) '*' * '/',
		(unsigned short) '-' * '+'
	};

	const unsigned short this = prev * new;
	if ((this == same[0]) || (this == same[1]))
		return true;

	const bool_t newslow = ((new == '+') || (new == '-'));
	return newslow ? true : false;
}

static enum CellType solve (struct Output *const out)
{
	enum CellType status = ctype_number;
	register unsigned short k;

	for (k = --out->stk_i; (k >= QUEUE_SIZE) && !CELL_IS_ERR(status); k--)
		status = push_queue(out, &out->output[k]);

	long double numstack[MAX_CONTAINER_CAP] = {0};
	unsigned short num_i = 0;

	for (k = 0; (k < out->que_i) && !CELL_IS_ERR(status); k++) {
		struct Token *tok = &out->output[k];
		if (num_i == MAX_CONTAINER_CAP)
			return ctype_error_expr_ovrflw;

		if (tok->type == ttype_is_number) {
			numstack[num_i++] = tok->as.number;
			continue;
		}

		status = perform_operation(numstack, &num_i, tok->type);
	}

	*out->final = numstack[0];
	return status;
}

static enum CellType perform_operation (long double *stacknum, unsigned short *num_i, const enum TokenType wh)
{
	if (*num_i == 1)
		return ctype_error_malformedex;

	long double *op1 = &stacknum[*num_i - 2];
	long double op2  = stacknum[*num_i - 1];

	switch (wh) {
		case ttype_add_sign:
			*op1 = *op1 + op2;
			break;
		case ttype_sub_sign:
			*op1 = *op1 - op2;
			break;
		case ttype_mul_sign:
			*op1 = *op1 * op2;
			break;
		case ttype_div_sign:
			if (!op2) return ctype_error_div_by_zero;
			*op1 = *op1 / op2;
			break;
	}

	*num_i -= 1;
	return ctype_number;
}

