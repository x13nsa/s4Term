#ifndef S4TBH_S4TB_H
#define S4TBH_S4TB_H

#define	TOKEN_CONTAINTER_LEN	32
#define	MAX_STRING_LENGTH		UCHAR_MAX

#define	CHECK_PTR(p)			do { if (p) break; err(EXIT_FAILURE, "[fatal]: cannot continue"); } while (0)
#define	MAX_OF(a, b)			((a) > (b) ? (a) : (b))
#define	SET_TODO(a)				printf("\x1b[5mTODO: %s: %d : [%s]\x1b[0m\n", a, __LINE__, __FILE__)

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef	uint8_t	width_t;

enum TokenClass {
	TClass_unknown		= 0,
	TClass_space		= 1,
	TClass_newline		= 10,
	TClass_nextcell		= '|',
	TClass_string		= '"',
	TClass_reference	= '@',
	TClass_clone		= '^',
	TClass_expression	= '=',
	TClass_add_sign		= '+',
	TClass_sub_sign		= '-',
	TClass_div_sign		= '/',
	TClass_mul_sign		= '*',
	TClass_l_par		= '(',
	TClass_r_par		= ')',
	TClass_command		= ':',
	TClass_number		= 128,
};

enum CellClass {
	CClass_empty		= 0,
	CClass_unknonwn_op	= 1,

	CClass_number		= 100,
	CClass_string		= 101
};

union Value {
	struct	{ long double val; width_t width; }	num;
	struct	{ struct Cell *ref; uint32_t at; }	ref;
	struct	{ char *src; width_t width; } 		txt;
};

struct Token {
	union Value		as;
	enum TokenClass	class;
};

struct Cell {
	union Value		as;
	uint16_t		exprlen;
	enum CellClass	class;
	width_t			width;
};

struct SheetLexer {
	char		*src;
	size_t		srcsz, l_off, at;
	uint16_t	current_row;
};

struct ExecArgs {
	char	*in_name;
	char	*out_name;
	width_t	precision;
};

struct SheetDimensions {
	struct Cell	*toprightcell;
	uint32_t	total_of_cells;
	uint16_t	columns, rows;
	width_t		cellwidth;
};

struct Program {
	struct ExecArgs			args;
	struct SheetLexer		slex;
	struct SheetDimensions	sdim;
	struct Cell				*grid;
};

#endif
