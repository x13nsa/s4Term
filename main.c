#include "s4tb.h"
#include "error.h"
#include "expr.h"
#include <math.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define	MAX_OF(a, b)		((a) > (b) ? (a) : (b))

struct CellError {
	char	*err;
	size_t	len;
};

static void read_sheet (const char *const, struct SLexer *const);
static void det_table_sz (struct Sheet *const);

static void collect_cells (struct Sheet *const);
static enum TokenType find_out_type (struct SLexer *const);

static void get_number_token (struct SLexer *const, struct Token *const);
static void get_string_token (struct SLexer *const, struct Token *const);

static unsigned int get_reference_token (struct SLexer *const, const unsigned int, const unsigned short);
static void solve_fucking_cell (struct Sheet *const, struct Cell *const, struct Token *const);

static void set_error_on_cell (struct Cell *const, const enum CellType);
static void print_sheet (const struct Sheet *const);

int main (int argc, char **argv)
{
	if (argc == 1) error_print_usage();

	opterr = 0;
	int op;

	struct Sheet sheet;
	memset(&sheet, 0, sizeof(struct Sheet));

	while ((op = getopt(argc, argv, ":s:o:p:h")) != -1) {
		switch (op) {
			case 's': sheet.filename_in  = optarg; break;
			case 'o': sheet.filename_out = optarg; break;
			case 'p': sheet.dprecision   = atoi(optarg); break;
			default: error_print_usage();
		}
	}

	read_sheet(sheet.filename_in, &sheet.slexer);
	det_table_sz(&sheet);

	sheet.gridsize = sheet.columns * sheet.rows;
	sheet.grid = (struct Cell*) calloc(sheet.gridsize, sizeof(struct Cell));

	error_check_ptr(sheet.grid);
	collect_cells(&sheet);

	print_sheet(&sheet);
	return 0;
}


static void read_sheet (const char *const filename, struct SLexer *const slex)
{
	if (!filename)
		error_bad_use("no sheet provided");

	FILE *file = fopen(filename, "r");
	if (!file)
		error_bad_use("'%s' sheet does not work; check its existence and permissions", filename);

	fseek(file, 0, SEEK_END);
	slex->t_bytes = ftell(file);
	fseek(file, 0, SEEK_SET);

	slex->content = (char*) calloc(slex->t_bytes + 1, sizeof(char));
	error_check_ptr(slex->content);

	const size_t read = fread(slex->content, 1, slex->t_bytes, file);
	if (read != slex->t_bytes)
		error_bad_use("not whole sheet was read: %ld/%ld bytes", read, slex->t_bytes);
	fclose(file);
}

static void det_table_sz (struct Sheet *const sheet)
{
	unsigned short maxncols = 0;
	const size_t til = sheet->slexer.t_bytes;

	for (register size_t k = 0; k < til; k++) {
		const char c = sheet->slexer.content[k];
		if (c == '|')
			sheet->columns++;
		else if (c == '\n') {
			maxncols = MAX_OF(maxncols, sheet->columns);
			sheet->rows++;
			sheet->columns = 0;
		}
	}

	sheet->columns = maxncols;
}

static void collect_cells (struct Sheet *const sheet)
{
	struct Token thisexpr[TOKENSTREAM_SIZE] = {0};

	struct Token *this_token = &thisexpr[0];
	struct Cell *this_cell   = &sheet->grid[0];

	struct SLexer *lex = &sheet->slexer;
	lex->nline = 1;

	while (lex->cpos < lex->t_bytes) {
		this_token->type = find_out_type(lex);

		if (this_cell->exprsz == TOKENSTREAM_SIZE)
			error_at_lexer("maximum number of tokens per cell reached: %d", lex->content + lex->cpos, lex->nline, lex->loff, TOKENSTREAM_SIZE);

		switch (this_token->type) {
			case t_type_space: continue;

			case t_type_unknown: {
				char *offset = lex->content + (--lex->cpos);
				error_at_lexer("unknown token", offset, lex->nline, lex->cpos);
				break;
			}

			case t_type_newline: {
				this_cell  = &sheet->grid[lex->nline++ * sheet->columns];
				this_token = &thisexpr[0];
				lex->loff  = 0;
				continue;
			}

			case t_type_cell: {
				solve_fucking_cell(sheet, this_cell, thisexpr);
				this_token = &thisexpr[0];
				this_cell++;
				continue;
			}

			case t_type_number: {
				get_number_token(lex, this_token);
				break;
			}

			case t_type_string: {
				get_string_token(lex, this_token);
				break;
			}

			case t_type_reference: {
				const unsigned int pos = get_reference_token(lex, sheet->gridsize, sheet->columns);
				this_token->as.reference = &sheet->grid[pos];
				break;
			}

			case t_type_command: {
				MARK_TODO("commands");
				break;
			}
		}

		this_cell->exprsz++;
		this_token++;
	}
}

static enum TokenType find_out_type (struct SLexer *const slex)
{
	slex->loff++;
	const char a = slex->content[slex->cpos++];

	switch (a) {
		case '|': case '"': case ':':
		case '(': case ')': case '+':
		case '/': case '*': case '=':
		case '@': case '\n': return a;
	}

	if (isspace(a))
		return t_type_space;

	const char b = slex->content[slex->cpos];
	if (a == t_type_sub_sign)
		return isdigit(b) ? t_type_number : t_type_sub_sign;

	return isdigit(a) ? t_type_number : t_type_unknown;
}

static void get_number_token (struct SLexer *const slex, struct Token *const token)
{
	char *off = slex->content + slex->cpos - 1, *ends;
	long double *number = &token->as.number.value;

	*number = strtold(off, &ends);

	if ((*number >= LLONG_MAX) || (*number <= LLONG_MIN))
		error_at_lexer("number overflow", off, slex->nline, --slex->loff);

	const size_t inc = ends - off - 1;
	slex->cpos += inc;
	slex->loff += inc;
	token->as.number.width = (unsigned short) inc + 1;
}

static void get_string_token (struct SLexer *const slex, struct Token *const token)
{
	token->as.text.src = slex->content + slex->cpos;
	token->as.text.len = 0;

	do {
		if (slex->content[slex->cpos] == '\n')
			error_at_lexer("invalid string", --token->as.text.src, slex->nline, slex->loff);

		token->as.text.len++;
		slex->loff++;
	} while (slex->content[slex->cpos++] != '"');

	token->as.text.len--;
}

static unsigned int get_reference_token (struct SLexer *const slex, const unsigned int gsize, const unsigned short columns)
{
	char *start = slex->content + slex->cpos - 1;
	const unsigned short off = slex->loff;

	if (!isalpha(slex->content[slex->cpos])) goto bad_ref;
	unsigned short col = 0, depth = 0;

	do {
		col += (depth++ * 26) + (tolower(slex->content[slex->cpos++]) - 'a');
		slex->loff++;
	} while (isalpha(slex->content[slex->cpos]));

	if (!isdigit(slex->content[slex->cpos])) goto bad_ref;

	char *ends;
	unsigned int pos = col + (columns * ((unsigned short) strtold(slex->content + slex->cpos, &ends)));

	if (pos >= UINT_MAX) goto bad_ref;
	if (pos >= gsize) error_at_lexer("reference outta bounds", start, slex->nline, off);

	const size_t inc = ends - (slex->content + slex->cpos);
	slex->loff += inc;
	slex->cpos += inc;

	return pos;

	bad_ref:
	{
		error_at_lexer("malformed reference", start, slex->nline, off);
		return 0;
	}
}

static void solve_fucking_cell (struct Sheet *const sheet, struct Cell *const cell, struct Token *const stream)
{
	if (cell->exprsz == 0) {
		set_error_on_cell(cell, c_type_unsolved);
		return;
	}

	switch (stream[0].type) {
		case t_type_number: {
			cell->as.number = stream[0].as.number;
			cell->width     = stream[0].as.number.width;
			cell->type      = c_type_number;
			break;
		}
		case t_type_string: {
			cell->as.text.src = stream[0].as.text.src;
			cell->as.text.len = stream[0].as.text.len;
			cell->type        = c_type_string;
			cell->width       = cell->as.text.len;
			break;
		}
		case t_type_reference: {
			const struct Cell *const ref = stream[0].as.reference;
			if (ref >  cell) { set_error_on_cell(cell, c_type_further_cln); break; }
			if (ref == cell) { set_error_on_cell(cell, c_type_self_ref); break; }

			memcpy(cell, ref, sizeof(struct Cell));
			break;
		}
		case t_type_expressions: {
			const enum CellType ret = expr_solve_expression(cell, stream);
			if (CELLS_ERROR(ret)) {
				set_error_on_cell(cell, ret);
				break;
			}
			cell->type = c_type_number;
			break;
		}
		default: {
			set_error_on_cell(cell, c_type_unknonwn_op);
			break;
		}
	}

	sheet->cell_width = MAX_OF(sheet->cell_width, cell->width);
}

static void set_error_on_cell (struct Cell *const cell, const enum CellType wh)
{
	static const struct CellError errors[] = {
		{NULL, 0},
		{"![unknonwn-op]",		14},
		{"![further-clone]",	16},
		{"![self-reference]",	17},
		{"![malformed-expr]",	17},
		{"![expr-overflow]",	16}
	};

	cell->as.text.src = errors[wh].err;
	cell->as.text.len = errors[wh].len;
	cell->width       = errors[wh].len;
	cell->type = wh;
}

static void print_sheet (const struct Sheet *const sheet)
{
	FILE *outw = stdout;
	if (sheet->filename_out)
		outw = fopen(sheet->filename_out, "a+");

	for (unsigned short row = 0; row < sheet->rows; row++) {
		fprintf(outw, "\n\t| ");

		for (unsigned short col = 0; col < sheet->columns; col++) {
			struct Cell *t = &sheet->grid[row * sheet->columns + col];

			if (t->type == c_type_number)
				fprintf(outw, "%-*Lf  | ", sheet->cell_width, t->as.number.value);
			else if (t->type == c_type_unsolved)
				fprintf(outw, "%*s| ", sheet->cell_width + 2, " ");
			else
				fprintf(outw, "%-*.*s  | ", sheet->cell_width, t->as.text.len, t->as.text.src);
		}
	}

	fprintf(outw, "\n\n");
}
