#include "s4tb.h"
#include "error.h"
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

static void read_sheet (const char *const, struct SLexer *const);
static void det_table_sz (struct Sheet *const);

static void collect_cells (struct Sheet *const);

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
	slex->csize = ftell(file);
	fseek(file, 0, SEEK_SET);

	slex->content = (char*) calloc(slex->csize + 1, sizeof(char));
	error_check_ptr(slex->content);

	const size_t read = fread(slex->content, 1, slex->csize, file);
	if (read != slex->csize)
		error_bad_use("not whole sheet was read: %ld/%ld bytes", read, slex->csize);
	fclose(file);
}

static void det_table_sz (struct Sheet *const sheet)
{
	unsigned short maxncols = 0;
	const size_t til = sheet->slexer.csize;

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

}
