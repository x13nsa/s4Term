#ifndef S4TB_S4TB_H
#define S4TB_S4TB_H
#include <stdio.h>
#include <stdbool.h>

#define	TOKENSTREAM_SIZE	64
#define	CELLS_ERROR(a)		(((a) >= c_type_unknonwn_op) && ((a) <= c_type_illegal_val))
#define	MARK_TODO(s)		printf("TODO: %s (%s: %d)", s, __FILE__, __LINE__);

enum TokenType {
	t_type_cell			= '|',
	t_type_string		= '"',
	t_type_command		= ':',
	t_type_newline		= '\n',
	t_type_left_par		= '(',
	t_type_right_par	= ')',
	t_type_add_sign		= '+',
	t_type_sub_sign		= '-',
	t_type_div_sign		= '/',
	t_type_mul_sign		= '*',
	t_type_expressions	= '=',
	t_type_reference	= '@',
	t_type_clone_up		= '^',
	t_type_space		= 128,
	t_type_number		= 129,
	t_type_unknown		= 130,
};

enum CellType {
	c_type_unsolved		= 0,
	c_type_unknonwn_op	= 1,
	c_type_further_cln	= 2,
	c_type_self_ref		= 3,
	c_type_bad_expr		= 4,
	c_type_expr_ovrflow	= 5,
	c_type_div_by_zero	= 6,
	c_type_illegal_val	= 7,
	c_type_bad_clone	= 8,

	c_type_number		= 10,
	c_type_string		= 11,
};

struct Cell;

union Value {
	struct Cell	*reference;
	struct		{ long double value; unsigned short width; } number;
	struct 		{ char *src; unsigned short len; } text;
};

struct Token {
	union Value		as;
	enum TokenType	type;
	unsigned short	number_width;
};

struct Cell {
	struct Token	*expr;
	union Value		as;
	unsigned short	exprsz;
	unsigned short	width;
	enum CellType	type;
	bool 			is_expression;
};

struct SLexer {
	char			*content;
	size_t			t_bytes;
	size_t			cpos;
	unsigned short	nline;
	unsigned short	loff;
};

struct Sheet {
	struct SLexer	slexer;
	struct Cell		*grid;
	char			*filename_in;
	char			*filename_out;
	unsigned int	gridsize;
	unsigned short	columns;
	unsigned short	rows;
	unsigned short	cell_width;
	unsigned short	dprecision;
};

#endif
