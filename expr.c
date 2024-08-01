#include "expr.h"
#include <string.h>
#include <stdlib.h>

#define	HALF_CONTAINTER_SZ	TOKENSTREAM_SIZE / 2
#define	MAX_NUMSTACK_SZ		16

#define	HAS_LOW_PREC(a)		(((a) == t_type_add_sign) || ((a) == t_type_sub_sign))
#define	HAS_HIGH_PREC(a)	(((a) == t_type_mul_sign) || ((a) == t_type_div_sign))

struct Formula {
	struct Token	output[HALF_CONTAINTER_SZ * 2];
	long double		*save;
	unsigned short	beg_i;
	unsigned short	end_i;
	unsigned short	npars;
};

static enum CellType push_at_beginning (struct Formula *const, const long double, const enum TokenType);
static enum CellType push_at_end (struct Formula *const, struct Token *const);

static bool pop_top_operator (const enum TokenType, const enum TokenType);
static enum CellType right_par_found (struct Formula *const);

static enum CellType solve (struct Formula *const);
static enum CellType realizar (long double*, unsigned short*, const enum TokenType);

enum CellType expr_solve_expression (struct Cell *const cell, struct Token *stream)
{
	struct Formula fx;
	memset(&fx, 0, sizeof(struct Formula));

	fx.save  = &cell->as.number.value;
	fx.end_i = HALF_CONTAINTER_SZ;

	enum CellType status = c_type_unsolved;
	stream++;

	for (unsigned short i = 1; i < (cell->exprsz) && !CELLS_ERROR(status); i++) {
		switch (stream->type) {
			case t_type_number: {
				status = push_at_beginning(&fx, stream->as.number.value, t_type_number);
				break;
			}

			case t_type_reference: {
				struct Cell *ref = stream->as.reference;

				if (ref >  cell) 				return c_type_further_cln;
				if (ref == cell)				return c_type_self_ref;
				if (ref->type != c_type_number)	return c_type_illegal_val;

				status = push_at_beginning(&fx, ref->as.number.value, t_type_number);
				break;
			}

			case t_type_add_sign:
			case t_type_sub_sign:
			case t_type_mul_sign:
			case t_type_div_sign:
			case t_type_left_par:
			case t_type_right_par: {
				status = push_at_end(&fx, stream);
				break;
			}

			default:
				return c_type_bad_expr;
		}

		cell->type = c_type_number;
		stream++;
	}

	/*
	 * TODO: clone the final ouput to the expression of the current cell.
	 */

	return CELLS_ERROR(status) ? status : solve(&fx);
}

static enum CellType push_at_beginning (struct Formula *const fx, const long double n, const enum TokenType t)
{
	/* +----------------------------------------+
	 * +   OPERANDS         |   OPERATORS       +  } fx->output
	 * +--------------------`-------------------o~~~~~~~~~~~~~~~~~~~~~> TOKENSTREAM_SIZE
	 *                      |
	 *                      `HALF_CONTAINTER_SZ
	 *                      ` beg_i
	 * In the first half operands are saved, the another
	 * half is used for the symbols and precedence stuff, thus
	 * they cannot be together until the expression has been
	 * completely read.
	 */
	if (fx->beg_i == HALF_CONTAINTER_SZ)
		return c_type_expr_ovrflow;

	struct Token *this = &fx->output[fx->beg_i++];
	this->type = t;

	if (t == t_type_number) {
		this->as.number.value = n;
	}

	return c_type_number;
}

static enum CellType push_at_end (struct Formula *const fx, struct Token *const t)
{
	if (fx->end_i == TOKENSTREAM_SIZE)
		return c_type_expr_ovrflow;

	enum CellType status = c_type_number;

	if ((fx->end_i == HALF_CONTAINTER_SZ) || (t->type == t_type_left_par)) goto push;
	if (t->type == t_type_right_par) {
		status = right_par_found(fx);
		return status;
	}

	do {
		struct Token *top = &fx->output[--fx->end_i];
		if (!pop_top_operator(top->type, t->type)) {
			fx->end_i++;
			break;
		}

		status = push_at_beginning(fx, 0, top->type);
	} while (!CELLS_ERROR(status) && (fx->end_i > HALF_CONTAINTER_SZ));

	push:
	{
		if (t->type == t_type_left_par) fx->npars++;

		fx->output[fx->end_i++].type = t->type;
		return status;
	}
}

static bool pop_top_operator (const enum TokenType top, const enum TokenType new)
{
	if (top == new) return true;
	if (top == t_type_left_par) return false;

	static const unsigned short same[] = {
		t_type_add_sign * t_type_sub_sign,
		t_type_mul_sign * t_type_div_sign
	};

	const unsigned short a = new * top;
	if ((a == same[0]) || (a == same[1])) return true;
	if (HAS_LOW_PREC(new) && HAS_HIGH_PREC(top)) return true;

	return false;
}

static enum CellType right_par_found (struct Formula *const fx)
{
	if (fx->npars == 0) return c_type_bad_expr;
	enum CellType status = c_type_number;

	fx->end_i--;

	while (fx->output[fx->end_i].type != t_type_left_par && !CELLS_ERROR(status)) {
		struct Token *top = &fx->output[fx->end_i--];
		status = push_at_beginning(fx, 0, top->type);
	}

	fx->npars--;
	return status;
}

static enum CellType solve (struct Formula *const fx)
{
	enum CellType status = c_type_number;
	for (unsigned short k = --fx->end_i; (k >= HALF_CONTAINTER_SZ) && !CELLS_ERROR(status); k--)
		status = push_at_beginning(fx, 0, fx->output[k].type);

	long double numstack[MAX_NUMSTACK_SZ] = {0};
	unsigned short nums_i = 0;

	for (unsigned short k = 0; (k < fx->beg_i) && !CELLS_ERROR(status); k++) {
		struct Token *t = &fx->output[k];

		if (t->type == t_type_number) {
			if (nums_i == MAX_NUMSTACK_SZ) return c_type_expr_ovrflow;
			numstack[nums_i++] = t->as.number.value;
			continue;
		}
		status = realizar(numstack, &nums_i, t->type);
	}

	*fx->save = numstack[0];
	return status;
}

static enum CellType realizar (long double *nums, unsigned short *at, const enum TokenType wh)
{
	if (*at < 2) return c_type_bad_expr;

	long double *a = &nums[*at - 2];
	long double b = nums[*at - 1];

	switch (wh) {
		case t_type_add_sign: *a += b; break;
		case t_type_sub_sign: *a -= b; break;
		case t_type_mul_sign: *a *= b; break;
		case t_type_div_sign:
			if (b == 0) return c_type_div_by_zero;
			*a /= b;
			break;
	}

	*at -= 1;
	return c_type_number;
}
