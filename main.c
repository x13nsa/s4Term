#include "s4tb.h"
#include "error.h"
#include <math.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

static void read_sheet (const char *const, struct SLexer *const);
static void det_table_sz (struct Sheet *const);

static void collect_cells (struct Sheet *const);
static enum TokenType find_out_type (struct SLexer *const);

static void get_number_token (struct SLexer *const, struct Token *const);
static void get_string_token (struct SLexer *const, struct Token *const);

static unsigned int get_reference_token (struct SLexer *const, const unsigned int, const unsigned short);

int main (int argc, char **argv)
{
	if (argc == 1) error_print_usage();

	opterr = 0;
	signed int op;

	struct Sheet sheet;
	memset(&sheet, 0, sizeof(struct Sheet));

	while ((op = getopt(argc, argv, ":s:o:h")) != -1) {
		switch (op) {
			case 's': sheet.filename_in  = optarg; break;
			case 'o': sheet.filename_out = optarg; break;
			default: error_print_usage();
		}
	}

	read_sheet(sheet.filename_in, &sheet.slexer);
	det_table_sz(&sheet);

	sheet.gridsize = sheet.columns * sheet.rows;
	sheet.grid = (struct Cell*) calloc(sheet.gridsize, sizeof(struct Cell));
	error_check_ptr(sheet.grid);

	collect_cells(&sheet);
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
			maxncols = (maxncols > sheet->columns) ? maxncols : sheet->columns;
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

		switch (this_token->type) {
			case t_type_space: continue;

			case t_type_unknown: {
				char *offset = lex->content + lex->cpos - 1;
				error_at_lexer("unknown token", offset, lex->nline, lex->cpos - 1);
				break;
			}

			case t_type_newline: {
				this_cell  = &sheet->grid[++lex->nline * sheet->columns];
				this_token = &thisexpr[0];
				lex->loff  = 0;
				continue;
			}

			case t_type_cell: {
				this_cell++;
				break;
			}

			case t_type_number: {
				get_number_token(lex, this_token);
				printf("token: <%Lf>\n", this_token->as.number);
				break;
			}


			case t_type_string: {
				get_string_token(lex, this_token);
				printf("token: <%.*s>\n", this_token->as.text.len, this_token->as.text.src);
				break;
			}

			case t_type_reference: {
				const unsigned int pos = get_reference_token(lex, sheet->gridsize, sheet->columns);
				this_token->as.reference = &sheet->grid[pos];
				printf("token: <POS=%d>\n", pos);
				break;
			}


			case t_type_command:
				MARK_TODO("commands");
				break;
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
	char *off = slex->content + slex->cpos - 1;

	char *ends;
	token->as.number = strtold(off, &ends);

	if ((token->as.number >= LLONG_MAX) || (token->as.number <= LLONG_MIN))
		error_at_lexer("number overflow", off, slex->nline, --slex->loff);

	const size_t inc = ends - off - 1;
	slex->cpos += inc;
	slex->loff += inc;
}

static void get_string_token (struct SLexer *const slex, struct Token *const token)
{
	token->as.text.src = slex->content + slex->cpos;
	token->as.text.len = 0;

	do {
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


