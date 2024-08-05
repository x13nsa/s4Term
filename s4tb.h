#ifndef S4TB_S4TB_H
#define S4TB_S4TB_H
#include <stdint.h>
#include <stdlib.h>

#define	MAX_TOKEN_PER_CELL	64
#define	CHECK_PTR(a)		do { if (a) break; errx(EXIT_FAILURE, "[error]: cannot continue"); } while (0)

enum TokenType {
	t_type_next_row = '\n',
	t_type_next_col	= '|',
	t_type_referenz = '@',
	t_type_string   = '"',
	t_type_clone	= '^',
	t_type_exprssn	= '=',
	t_type_add_sign	= '+',
	t_type_sub_sign	= '-',
	t_type_mul_sign	= '*',
	t_type_div_sign	= '/',
	t_type_L_par	= '(',
	t_type_R_par	= ')',

	t_type_dec_num	= 129,
	t_type_unknown	= 130,
	t_type_space	= 131
};

struct Cell;

union Value {
	struct	{ long double val; uint16_t width; } num;
	struct	{ char *src; uint16_t len; } str;
	struct	Cell *ref;
};

struct Token {
	union Value		as;
	enum TokenType	type;
};

struct Cell {
	union Value	as;
	uint16_t	exprsz, width;
};

struct Lexer {
	char		*content;
	size_t		pos, bytes;
	uint16_t	loff, row;
};

struct Program {
	struct Lexer	lex;
	struct Cell		*grid;
	char			*in_filename, *out_filename;
	uint32_t		grid_size;
	uint16_t		rows, columns;
	uint8_t			precision;
};

#endif
