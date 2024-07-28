#ifndef S4TERM_S4TERM_H
#define S4TERM_S4TERM_H
#include <stdio.h>

#define	MAX_TEXT_LENGTH		128
#define	MAX_TOKENS_PER_CELL	32

#define	true	1
#define	false	0
typedef unsigned char	bool_t;

enum TokenType {
	ttype_next_cell 	= '|',
	ttype_text			= '"',
	ttype_reference 	= '@',
	ttype_expression	= '=',
	ttype_clone_up		= '^',
	ttype_clone_left	= '<',
	ttype_clone_right	= '>',
	ttype_clone_down	= 'v',
	ttype_sub_sign		= '-',
	ttype_add_sign		= '+',
	ttype_mul_sign		= '*',
	ttype_div_sign		= '/',
	ttype_left_p		= '(',
	ttype_right_p		= ')',

	ttype_is_space		= 128,
	ttype_is_newline	= 129,
	ttype_is_number		= 130,
	ttype_is_unknonw	= 131
};

enum CellType {
	ctype_error_unsolved	= 0,
	ctype_error_nosense		= 1,
	ctype_error_selfref		= 2,
	ctype_error_further_ref	= 3,
	ctype_error_expr_ovrflw	= 4,

	ctype_number			= 50,
	ctype_text				= 51,
	ctype_clone_up			= 52,
	ctype_clone_down		= 53,
	ctype_clone_left		= 54,
	ctype_clone_right		= 55
};

struct Cell;

union Value {
	struct			{ char *src; unsigned short len; } text;
	struct Cell		*reference;
	long double		number;
};

struct Token {
	union Value		as;
	enum TokenType	type;
	bool_t			is_hex;
};

struct Cell {
	union Value		as;
	struct Token	*expression;
	unsigned short	expr_len;
	enum CellType	type;
	bool_t			is_hex;
};

struct Lexer {
	char			*src;
	size_t			t_bytes;
	size_t			at;
	unsigned short	numline;
	unsigned short	linepos;
};

struct SheetInfo {
	struct Lexer	lexer;
	struct Cell		*grid;
	char			*in_filename;
	char			*out_filename;
	unsigned int	t_cells;
	unsigned short	rows;
	unsigned short	cols;
};

#endif
