#include "error.h"
#include <stdio.h>
#include <ctype.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

enum TokenKind {
	tkind_string		= '"',
	tkind_next_cell		= '|',
	tkind_reference		= '@',
	tkind_clone_top		= '^',
	tkind_clone_down	= 'v',
	tkind_clone_left	= '<',
	tkind_clone_right	= '>',
	tkind_minus			= '-',
	tkind_addition		= '+',
	tkind_division		= '/',
	tkind_mult			= '*',
	tkind_expression	= '=',

	tkind_number		= 256,
	tkind_unknown		= 257,
	tkind_whitespace	= 258
};

struct Cell {
		int a;
};

struct SheetLex {
	char			*src;
	size_t			sz;
	size_t			at;
	unsigned short	numline;
	unsigned short	l_off;
};

struct Sheet {
	struct SheetLex	lexer;
	struct Cell		*cells;
	char			*in_filename;
	char			*out_filename;
	unsigned short	rows;
	unsigned short	cols;
};

static void read_sheet (struct Sheet *const);
static void get_table_size (struct Sheet *const);

static void analyze_table (struct Sheet *const);
static enum TokenKind get_kind (struct SheetLex *const);

int main (int argc, char **argv)
{
	if (argc == 1) error_usage();

	struct Sheet sheet;
	memset(&sheet, 0, sizeof(struct Sheet));

	opterr = 0;
	int op;

	while ((op = getopt(argc, argv, ":s:o:h")) != -1) {
		switch (op) {
			case 's': sheet.in_filename = optarg; break;
			case 'o': sheet.out_filename = optarg; break;
			default: error_usage();
		}
	}

	if (!sheet.in_filename) error_fatal("no sheet provied");

	read_sheet(&sheet);
	get_table_size(&sheet);

	sheet.cells = (struct Cell*) calloc(sheet.rows * sheet.cols, sizeof(struct Cell));
	error_check_ptr(sheet.cells);

	analyze_table(&sheet);

	free(sheet.lexer.src);
	free(sheet.cells);
	return 0;
}

static void read_sheet (struct Sheet *const s)
{
	FILE *f = fopen(s->in_filename, "r");
	if (!f)
		error_fatal("sheet provied does not work: `%s'", s->in_filename);

	fseek(f, 0, SEEK_END);
	s->lexer.sz = ftell(f);
	fseek(f, 0, SEEK_SET);

	s->lexer.src = (char*) calloc(s->lexer.sz + 1, sizeof(char));
	error_check_ptr(s->lexer.src);

	const size_t read_B = fread(s->lexer.src, 1, s->lexer.sz, f);
	if (read_B != s->lexer.sz)
		error_fatal("only %ld bytes out of %ld were read", read_B, s->lexer.sz);
	fclose(f);
}

static void get_table_size (struct Sheet *const s)
{
	unsigned short maxcol = 0;

	for (register size_t b = 0; b < s->lexer.sz; b++) {
		const char a = s->lexer.src[b];
		if (a == '\n') {
			maxcol  = (maxcol > s->cols) ? maxcol : s->cols;
			s->cols = 0;
			s->rows++;
		} else if (a == '|') s->cols++;
	}

	s->cols = maxcol;
}

static void analyze_table (struct Sheet *const s)
{
	while (s->lexer.at < s->lexer.sz) {
		switch (get_kind(&s->lexer)) {
			case tkind_whitespace: continue;
			case tkind_unknown: break;

			case tkind_next_cell: {
				break;
			}

			case tkind_string: {
				break;
			}

			case tkind_reference: {
				break;
			}

			case tkind_number: {
				break;
			}
		}
	}
}

static enum TokenKind get_kind (struct SheetLex *const lex)
{
	const char c = lex->src[lex->at++];
	lex->l_off++;

	if (isspace(c)) {
		if (c == '\n') {
			lex->l_off = 1;
			lex->numline++;
		}
		return tkind_whitespace;
	}

	switch (c) {
		case '|': case '"':
		case '<': case '>':
		case '^': case 'v':
		case '+': case '*':
		case '/': case '@':
		case '=': return c;
	}

	if (c == '-')
		return isdigit(lex->src[lex->at + 1]) ? tkind_number : tkind_minus;
	return isdigit(c) ? tkind_number : tkind_unknown;
}

