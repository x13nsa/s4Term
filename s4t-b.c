#include "s4t-b.h"
#include <err.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <getopt.h>
#include <string.h>

enum LexErr {
	lexerr_unknown_token	= 0,
	lexerr_string_ovrflow	= 1,
	lexerr_multiline_str	= 2,
	lexerr_malformed_ref	= 3,
	lexerr_ref_outtabnds	= 4,
	lexerr_number_ovrflow	= 5,
	lexerr_max_cap_reached	= 6
};

struct ErrorCell {
	char	*error;
	width_t	width;
};

static void print_usage (void);
static void parse_arguments (const uint32_t, char**, struct Program *const);

static void read_file (const char *const, struct SheetLexer *const);
static void calculate_sheet_dimensions (const char*, struct SheetDimensions *const);

static void lex_tables_content (struct Program *const);
static enum TokenClass initializate_token_if_any (struct Token *const, struct SheetLexer *const, const struct SheetDimensions *const);

static void init_token_as_string (struct Token *const, struct SheetLexer *const);
static void init_token_as_reference (struct Token *const, struct SheetLexer *const, const struct SheetDimensions *const);

static void init_token_as_number (struct Token *const, struct SheetLexer *const);
static void solve_cell (struct Cell *const, struct Token *const, const width_t, width_t*);

static void fatal__ (const enum LexErr, const struct SheetLexer *const);
static void write_err_on_cell (const enum CellClass, struct Cell *const);

static void display_table (const struct Program *const);

int main (int argc, char **argv)
{
	if (argc == 1) print_usage();

	struct Program P;
	parse_arguments(argc, argv, &P);

	read_file(P.args.in_name, &P.slex);
	calculate_sheet_dimensions(P.slex.src, &P.sdim);

	P.grid = (struct Cell*) calloc(P.sdim.total_of_cells, sizeof(struct Cell));
	CHECK_PTR(P.grid);

	/* +---------------------------------------+
	 * +    |    |    |    |    |    |    |  *.|
	 * +---------------------------------------` Top right cell
	 * +    |    |    |    |    |    |    |    |
	 * +---------------------------------------+
	 * Clone operations (^) cannot be performed before this cell.
	 */
	P.sdim.toprightcell = &P.grid[P.sdim.rows - 1];

	lex_tables_content(&P);
	display_table(&P);

	free(P.slex.src);
	return 0;
}

static void print_usage (void)
{
	static const char* usage = "\tspreadsheet for terminal basic version - %s %s\n"
							   "\tusage: s4t-b [-s sheet] [arguments]\n"
							   "\targuments:\n"
							   "\t  -o <*>		write output to <*>\n"
							   "\t  -p <*>		decimal precision of <*>\n";
	fprintf(stderr, usage, __DATE__, __TIME__);
	exit(EXIT_SUCCESS);
}

static void parse_arguments (const uint32_t nargs, char **vargs, struct Program *const P)
{
	memset(P, 0, sizeof(struct Program));

	int32_t op;
	opterr = 0;

	while ((op = getopt(nargs, vargs, ":s:o:p:")) != -1) {
		switch (op) {
			case 's': P->args.in_name = optarg; break;
			case 'o': P->args.out_name = optarg; break;
			case 'p': P->args.precision = (width_t) atoi(optarg); break;
			case ':': errx(EXIT_FAILURE, "[fatal]: the '-%c' option requires an argument", optopt); break;
			case '?': errx(EXIT_FAILURE, "[fatal]: the '-%c' option is unknown", optopt); break;
			default:  print_usage();
		}
	}

	if (!P->args.in_name)
		errx(EXIT_FAILURE, "[fatal]: cannot proceed without sheet.");

	if (P->args.precision >= UCHAR_MAX)
		P->args.precision = 1;

	P->sdim.cellwidth = 10;
}

static void read_file (const char *const filename, struct SheetLexer *const slex)
{
	FILE *sheet = fopen(filename, "r");
	if (!sheet)
		err(EXIT_FAILURE, "[fatal]: %s won't work due to", filename);

	fseek(sheet, 0, SEEK_END);
	slex->srcsz = ftell(sheet);
	fseek(sheet, 0, SEEK_SET);

	slex->src = (char*) calloc(slex->srcsz + 1, sizeof(char));
	CHECK_PTR(slex->src);

	const size_t read = fread(slex->src, 1, slex->srcsz, sheet);
	if (read != slex->srcsz)
		errx(EXIT_FAILURE, "[fatal]: not all file was read: %ld/%ld bytes", read, slex->srcsz);
	fclose(sheet);
}

static void calculate_sheet_dimensions (const char *src, struct SheetDimensions *const sdim)
{
	uint16_t columns = 0;

	while (*src) {
		const char a = *src++;
		if (a == '\n') {
			sdim->rows++;
			columns = MAX_OF(columns, sdim->columns);
			sdim->columns = 0;
		} else if (a == '|') sdim->columns++;
	}

	sdim->columns = columns;
	sdim->total_of_cells = (uint32_t) (sdim->columns * sdim->rows);
}


static void lex_tables_content (struct Program *const P)
{
	struct Token stream[TOKEN_CONTAINTER_LEN];
	memset(&stream, 0, sizeof(struct Token) * TOKEN_CONTAINTER_LEN);

	struct Cell  *ths_cell = &P->grid[0];
	struct Token *ths_tokn = &stream[0];

	while (P->slex.at < P->slex.srcsz) {
		ths_tokn->class = initializate_token_if_any(ths_tokn, &P->slex, &P->sdim);

		if (ths_cell->exprlen == TOKEN_CONTAINTER_LEN)
			fatal__(lexerr_max_cap_reached, &P->slex);

		if (ths_tokn->class == TClass_space)
			continue;

		if (ths_tokn->class == TClass_nextcell) {
			solve_cell(ths_cell++, stream, P->args.precision, &P->sdim.cellwidth);
			ths_tokn = &stream[0];
			continue;
		}
		if (ths_tokn->class == TClass_newline) {
			ths_cell = &P->grid[++P->slex.current_row * P->sdim.columns];
			ths_tokn = &stream[0];
			P->slex.l_off = 0;
			continue;
		}
		if (ths_tokn->class == TClass_reference)
			ths_tokn->as.ref.ref = &P->grid[ths_tokn->as.ref.at];

		ths_tokn++;
		ths_cell->exprlen++;
	}
}

static enum TokenClass initializate_token_if_any (struct Token *const tok, struct SheetLexer *const slex, const struct SheetDimensions *const sdim)
{
	const char this = slex->src[slex->at++];
	slex->l_off++;

	switch (this) {
		case '^':
		case '=':
		case '+':
		case '/':
		case '*':
		case '(':
		case ')':	return this;
		case '|':	return TClass_nextcell;
		case '\n':	return TClass_newline;

		case '"':
			init_token_as_string(tok, slex);
			return TClass_string;
		case '@':
			init_token_as_reference(tok, slex, sdim);
			return TClass_reference;
		case ':':
			SET_TODO("parse commands");
			return TClass_command;
	}

	if (isspace(this)) return TClass_space;
	if (isdigit(this)) goto __token_is_num;

	const char next = slex->src[slex->at];

	if (this == '-') {
		if (isdigit(next)) goto __token_is_num;
		return TClass_sub_sign;
	}

	fatal__(lexerr_unknown_token, slex);
	return TClass_unknown;

	__token_is_num:
	{
		init_token_as_number(tok, slex);
		return TClass_number;
	}
}

static void init_token_as_string (struct Token *const tok, struct SheetLexer *const slex)
{
	tok->as.txt.src   = slex->src + slex->at;
	tok->as.txt.width = 0;

	width_t *w = &tok->as.txt.width;

	while (tok->as.txt.src[*w] != '"') {
		if (*w == MAX_STRING_LENGTH)
			fatal__(lexerr_string_ovrflow, slex);
		if (tok->as.txt.src[*w] == '\n')
			fatal__(lexerr_multiline_str, slex);
		*w += 1;
	}

	slex->at    += *w + 1;
	slex->l_off += *w + 1;
}

static void init_token_as_reference (struct Token *const tok, struct SheetLexer *const slex, const struct SheetDimensions *const sdim)
{
	char *bgns = slex->src + slex->at, *ends;

	if (!isalpha(*bgns))
		fatal__(lexerr_malformed_ref, slex);

	uint16_t _row = 0, _col = 0, depth = 0;
	while (isalpha(*bgns))
		_col += (depth++ * 26) + (tolower(*bgns++) - 'a');

	if (!isdigit(*bgns) || (_col >= USHRT_MAX))
		fatal__(lexerr_malformed_ref, slex);

	_row = (uint16_t) strtoul(bgns, &ends, 10);
	if ((tok->as.ref.at = _row * sdim->columns + _col) >= sdim->total_of_cells)
		fatal__(lexerr_ref_outtabnds, slex);

	const size_t diff = ends - (slex->src + slex->at);
	slex->l_off += diff;
	slex->at    += diff;
}

static void init_token_as_number (struct Token *const tok, struct SheetLexer *const slex)
{
	long double *num = &tok->as.num.val;
	char *ends, *bgns = slex->src + slex->at - 1;

	*num = strtold(bgns, &ends);
	if ((*num >= LLONG_MAX) || (*num <= LLONG_MIN))
		fatal__(lexerr_number_ovrflow, slex);

	const size_t inc = ends - bgns - 1;
	slex->l_off += inc;
	slex->at    += inc;
	tok->as.num.width = inc + 1;
}

static void solve_cell (struct Cell *const cell, struct Token *const stream, width_t precision, width_t *finalwidth)
{
	if (cell->exprlen == 0) {
		write_err_on_cell(CClass_empty, cell);
		return;
	}

	switch (stream[0].class) {
		case TClass_number:
			cell->as.num.val = stream[0].as.num.val;
			cell->width      = stream[0].as.num.width + precision;
			cell->class		 = CClass_number;
			break;
		case TClass_string:
			cell->as.txt.src = stream[0].as.txt.src;
			cell->width		 = stream[0].as.txt.width;
			cell->class		 = CClass_string;
			break;
		case TClass_reference:
			break;
		case TClass_expression:
			break;
		case TClass_clone:
			break;
		default:
			write_err_on_cell(CClass_unknonwn_op, cell);
			break;
	}

	*finalwidth = MAX_OF(*finalwidth, cell->width);
}

static void fatal__ (const enum LexErr wh, const struct SheetLexer *const slex)
{
	static const char *const fmts[] = {
		"[fatal:%d:%d]: unknown token\n\x1b[5;31m%.*s\x1b[0m",
		"[fatal:%d:%d]: string is too long\n\x1b[5;31m%.*s\x1b[0m",
		"[fatal:%d:%d]: multi-line string is not allowed\n\x1b[5;31m%.*s\x1b[0m",
		"[fatal:%d:%d]: malformed reference\n\x1b[5;31m%.*s\x1b[0m",
		"[fatal:%d:%d]: reference outta bounds\n\x1b[5;31m%.*s\x1b[0m",
		"[fatal:%d:%d]: number overflow\n\x1b[5;31m%.*s\x1b[0m",
		"[fatal:%d:%d]: maximum capacity reached\n\x1b[5;31m%.*s\x1b[0m",
	};

	char *bgns = slex->src + slex->at - 1;
	uint16_t _show = 0;

	while (!isspace(bgns[_show]) && bgns[_show])
		_show++;
	errx(EXIT_FAILURE, fmts[wh], slex->current_row, slex->l_off - 1, _show, bgns);
}

static void write_err_on_cell (const enum CellClass wh, struct Cell *const cell)
{
	static const struct ErrorCell errs[] = {
		{"![empty-cell]",	13},
		{"![unknown-op]",	13},
	};

	const struct ErrorCell *const e = &errs[wh];

	cell->as.txt.src   = e->error;
	cell->as.txt.width = e->width;
	cell->width        = e->width;
	cell->class		   = wh;
}

static void display_table (const struct Program *const P)
{
	const width_t width = P->sdim.cellwidth;

	for (uint16_t row = 0; row < P->sdim.rows; row++) {
		for (uint16_t col = 0; col < P->sdim.columns; col++) {
			struct Cell *cell = &P->grid[row * P->sdim.columns + col];

			if (cell->class == CClass_empty) {
				printf("%*s  ", width, " ");
			}
			else if (cell->class == CClass_number) {
				printf("%*.*Lf  ", width, P->args.precision, cell->as.num.val);
			}
			else {
				printf("%-*.*s  ", cell->width, width, cell->as.txt.src);
			}
		}
		putchar(10);
	}
}
