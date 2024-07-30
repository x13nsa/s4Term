#ifndef S4TB_S4TB_H
#define S4TB_S4TB_H
#include <stdio.h>
#include <stdbool.h>

struct Cell {
	int a;
};

struct SLexer {
	char	*content;
	size_t	csize;
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
