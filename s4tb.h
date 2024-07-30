#ifndef S4TB_S4TB_H
#define S4TB_S4TB_H
#include <stdio.h>
#include <stdbool.h>

#define	TOKENSTREAM_SIZE	64

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

	t_type_space		= 128,
	t_type_number		= 129,
	t_type_unknown		= 130,
};

struct Token {
	enum TokenType	type;
};

struct Cell {
	unsigned short	exprsz;
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
};

#endif
