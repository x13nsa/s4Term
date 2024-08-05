#include "s4tb.h"
#include "error.h"
#include <err.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#define	MAX_OF(a, b)	((a) > (b) ? (a) : (b))

enum LexErr {
	lexr_unknonwn_token	= 0,
};

static void parse_args (const uint32_t, char**, struct Program *const);
static void read_file (struct Lexer *const, const char *const);

static void det_table_size (const struct Lexer *const, uint16_t*, uint16_t*);
static void lex_table (struct Program*);

static enum TokenType find_type_out (struct Lexer *const);
static void fatal_error_while_lex (const enum LexErr, struct Lexer *const);

static void solve_cell (struct Cell *const);

int main (int argc, char **argv)
{
	if (argc == 1) error_usage();

	struct Program prgm;
	parse_args(argc, argv, &prgm);

	read_file(&prgm.lex, prgm.in_filename);
	det_table_size(&prgm.lex, &prgm.rows, &prgm.columns);

	prgm.grid_size = prgm.rows * prgm.columns;
	prgm.grid      = (struct Cell*) calloc(prgm.grid_size, sizeof(struct Cell));
	CHECK_PTR(prgm.grid);

	lex_table(&prgm);

	return 0;
}

static void parse_args (const uint32_t nargs, char **vargs, struct Program *const p)
{
	memset(p, 0, sizeof(struct Program));
	int32_t op;

	opterr = 0;

	while ((op = getopt(nargs, vargs, ":s:o:d:")) != -1) {
		switch (op) {
			case 's': p->in_filename  = optarg; break;
			case 'o': p->out_filename = optarg; break;
			case 'd': p->precision    = (uint8_t) atoi(optarg); break;
			case ':': errx(EXIT_FAILURE, "[error]: missing argument for `%c`", optopt); break;
			case '?': errx(EXIT_FAILURE, "[error]: unknonwn option `%c`", optopt); break;
			default:  error_usage();
		}
	}

	if (p->precision == 0)
		p->precision = 1;

	if (!p->in_filename)
		errx(EXIT_FAILURE, "[error]: cannot continue if no sheet is provied");
}

static void read_file (struct Lexer *const lex, const char *const filename)
{
	FILE *file = fopen(filename, "r");
	if (!file)
		err(EXIT_FAILURE, "[error]: `%s` will not work", filename);

	fseek(file, 0, SEEK_END);
	lex->bytes = ftell(file);
	fseek(file, 0, SEEK_SET);

	lex->content = (char*) calloc(lex->bytes + 1, sizeof(char));
	CHECK_PTR(lex->content);

	const size_t read = fread(lex->content, 1, lex->bytes, file);
	if (read != lex->bytes)
		warnx("not whole file was read: %ld/%ld bytes", read, lex->bytes);
	fclose(file);
}

static void det_table_size (const struct Lexer *const lex, uint16_t *rows, uint16_t *columns)
{
	uint16_t cols = 0;

	for (register size_t k = 0; k < lex->bytes; k++) {
		const char a = lex->content[k];
		if (a == '\n') {
			cols = MAX_OF(cols, *columns);
			*columns = 0;
			*rows += 1;
		} else if (a == '|') { *columns += 1; }
	}

	*columns = cols;
}

static void lex_table (struct Program *prgm)
{
	struct Token thisexpr[MAX_TOKEN_PER_CELL];
	memset(&thisexpr, 0, sizeof(struct Token) * MAX_TOKEN_PER_CELL);

	struct Token *tkn = &thisexpr[0];
	struct Cell *cell = &prgm->grid[0];
	struct Lexer *lex = &prgm->lex;

	while (lex->pos < lex->bytes) {
		tkn->type = find_type_out(lex);

		switch (tkn->type) {
			case t_type_space: continue;

			case t_type_next_row: {
				cell = &prgm->grid[++lex->row * prgm->columns];
				lex->loff = 0;
				tkn = &thisexpr[0];
				continue;
			}
			case t_type_next_col: {
				solve_cell(cell++);
				tkn = &thisexpr[0];
				continue;
			}

		}

		tkn++;
	}
}

static enum TokenType find_type_out (struct Lexer *const lex)
{
	lex->loff++;
	const char this = lex->content[lex->pos++];

	if (isspace(this))
		return t_type_space;

	switch (this) {
		case '\n':
		case '|':
		case '@':
		case '"':
		case '^':
		case '=':
		case '+':
		case '*':
		case '/':
		case '(':
		case ')':
			return this;
	}

	const char next = lex->content[lex->pos];
	if (this == '-')
		return isdigit(next) ? t_type_dec_num : t_type_sub_sign;

	if (isdigit(this))
		return t_type_dec_num;

	fatal_error_while_lex(lexr_unknonwn_token, lex);
	return t_type_unknown;
}

static void fatal_error_while_lex (const enum LexErr wh, struct Lexer *const lex)
{
	static const char *const fmts[] = {
		"[error]: (%d: %d) unknown token\n\x1b[5;31m%.*s\x1b[0m",
	};

	char *off = lex->content + lex->pos - 1;
	uint16_t _long = 1;

	while (!isspace(lex->content[lex->pos])) {
		lex->pos++;
		_long++;
	}

	errx(EXIT_FAILURE, fmts[wh], lex->row, lex->loff, _long, off);
}

static void solve_cell (struct Cell *const cell)
{

}
