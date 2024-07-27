#include "error.h"
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <getopt.h>
#include <stdlib.h>

#define	MAX_TEXT_LENGTH		32
#define	MAX_TOKENS_PER_CELL	64

enum ErrLexKind {
	errlex_text_overflow	= 0,
};

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

	ttype_is_space		= 128,
	ttype_is_number		= 129,
	ttype_is_unknonw	= 130
};

enum CellType {
	ctype_number,
	ctype_text,
	ctype_clone_up,
	ctype_clone_down,
	ctype_clone_left,
	ctype_clone_right,
	ctype_error
};

union Value {
	struct			{ char *src; unsigned short len; } text;
	union Value		*reference;
	long double		number;
};

struct Token {
	union Value		as;
	enum TokenType	type;
};

struct Cell {
	union Value		as;
	struct Token	*expression;
	unsigned short	expr_len;
	enum CellType	type;
};

struct Lexer {
	char			*src;
	size_t			t_bytes;
	size_t			at;
	unsigned short	numline;
	unsigned short	linepos;
	unsigned short	cell;
};

struct SheetInfo {
	struct Lexer	lexer;
	struct Cell		*grid;
	char			*in_filename;
	char			*out_filename;
	unsigned short	rows;
	unsigned short	cols;
	unsigned short	cell;
};

static void read_file (const char *const, struct Lexer *const);
static void get_table_dimensions (const char*, unsigned short*, unsigned short*);

static void analyze_table (struct SheetInfo *const);
static enum TokenType que_es_esto (struct Lexer *const);

static void get_token_as_a_string (union Value *const, struct Lexer *const);
static void get_token_as_a_number (long double *const, struct Lexer *const);

int main (int argc, char **argv) {
	if (argc == 1) error_usage();

	struct SheetInfo sheet;
	memset(&sheet, 0, sizeof(struct SheetInfo));

	int op;
	opterr = 0;

	while ((op = getopt(argc, argv, ":s:o:h")) != -1) {
		switch (op) {
			case 's': sheet.in_filename  = optarg; break;
			case 'o': sheet.out_filename = optarg; break;
			default: error_usage();
		}
	}

	read_file(sheet.in_filename, &sheet.lexer);
	get_table_dimensions(sheet.lexer.src, &sheet.rows, &sheet.cols);

	sheet.grid = (struct Cell*) calloc(sheet.rows * sheet.cols, sizeof(struct Cell));
	error_check_ptr(sheet.grid);

	analyze_table(&sheet);
	return 0;
}

static void read_file (const char *const filename, struct Lexer *const lexer)
{
	if (!filename) error_usage();
	FILE *file = fopen(filename, "r");

	if (!file)
		error_fatal("`%s' sheet given does not work", filename);

	fseek(file, 0, SEEK_END);
	lexer->t_bytes = ftell(file);
	fseek(file, 0, SEEK_SET);

	lexer->src = (char*) calloc(lexer->t_bytes + 1, sizeof(char));
	error_check_ptr(lexer->src);

	const size_t r_bytes = fread(lexer->src, 1, lexer->t_bytes, file);
	if (r_bytes != lexer->t_bytes)
		error_fatal("cannot read whole file, only %ld out of %ld bytes were read", r_bytes, lexer->t_bytes);
	fclose(file);
}

static void get_table_dimensions (const char *s, unsigned short *r, unsigned short *c)
{
	unsigned short maxc = 0;
	while (*s) {
		const char ch = *s++;
		if (ch == '\n') {
			maxc = (*c > maxc) ? *c : maxc;
			*r += 1;
			*c = 0;
		} else if (ch == '|') *c += 1;
	}

	*c = maxc;
}

static void analyze_table (struct SheetInfo *const sheet)
{
	struct Token tokens[MAX_TOKENS_PER_CELL];
	memset(tokens, 0, sizeof(struct Token) * MAX_TOKENS_PER_CELL);

	struct Lexer *lex = &sheet->lexer;
	lex->numline = 1;

	struct Cell  *thsc = &sheet->grid[0];
	struct Token *thsv = &tokens[0];

	while (lex->at < lex->t_bytes) {
		printf("with: %p\n", thsv);
		if (thsc->expr_len == MAX_TOKENS_PER_CELL)
			error_at_lexer(lex->src + lex->at, "maximum number of tokens reached (%d)", lex->numline, lex->linepos, MAX_TOKENS_PER_CELL);

		thsv->type = que_es_esto(lex);

		if (thsv->type == ttype_is_unknonw)
			error_at_lexer(lex->src + lex->at - 1, "unknown token", lex->numline, lex->linepos);
		if (thsv->type == ttype_is_space)
			continue;
		if (thsv->type == ttype_next_cell)
			continue;

		if (thsv->type == ttype_text)
			get_token_as_a_string(&thsv->as, lex);
		if (thsv->type == ttype_is_number)
			get_token_as_a_number(&thsv->as.number, lex);

		thsc->expr_len++;
		thsv++;
	}
}

static enum TokenType que_es_esto (struct Lexer *const lex)
{
	const char a = lex->src[lex->at++];
	lex->linepos++;

	if (isspace(a)) {
		if (a == 10) {
			lex->numline++;
			lex->cell = 0;
			lex->linepos = 0;
		}
		return ttype_is_space;
	}

	switch (a) {
		case '|': case '"': case '@': case '^':
		case '<': case '>': case 'v': case '=':
		case '+': case '*': case '/': return a;
	}

	const char b = lex->src[lex->at];

	if (a == '-') {
		const char c = ((lex->at + 1) < lex->t_bytes) ? lex->src[lex->at + 1] : 0;

		if ((b == '0') && (c == 'x'))
			return ttype_is_number;

		return isdigit(b) ?  ttype_is_number : ttype_sub_sign;
	}

	if ((a == '0') && (b == 'x'))
		return ttype_is_number;

	return isdigit(a) ? ttype_is_number : ttype_is_unknonw;
}

static void get_token_as_a_string (union Value *const str, struct Lexer *const lex)
{
	const unsigned starts_at = lex->linepos;
	str->text.src = lex->src + lex->at;

	while (lex->src[lex->at++] != '"') {
		if (str->text.len == MAX_TEXT_LENGTH)
			error_at_lexer(str->text.src, "text overflow, max length is %d", lex->numline, starts_at, MAX_TEXT_LENGTH);
		if (lex->src[lex->at] == '\n')
			error_at_lexer(str->text.src, "multiline string not allowed", lex->numline, starts_at);

		lex->linepos++;
		str->text.len++;
	}

	lex->linepos++;
	printf("string: <%.*s>\n", str->text.len, str->text.src);
}

static void get_token_as_a_number (long double *const num, struct Lexer *const lex)
{
	char *ends;
	char *from = lex->src + lex->at - 1;
	long double number = strtold(from, &ends);

	if (errno || (number >= LLONG_MAX))
		error_at_lexer(from, "number overflow", lex->numline, lex->linepos);

	const size_t diff = ends - from - 1;
	lex->linepos += diff;
	lex->at      += diff;

	*num = number;
	printf("number: <%Lf>\n", *num);
}
