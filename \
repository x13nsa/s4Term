#include "error.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

struct Sheet {
	char	*sheet;
	size_t	bytes;
};

static size_t read_file (FILE *const, char **const);

int main (int argc, char **argv)
{
	if (argc == 1) e_print_usage();

	FILE *file = fopen(argv[1], "r");
	if (!file) e_system("cannot read file");

	struct Sheet sheet;
	memset(&sheet, 0, sizeof(struct Sheet));
	sheet.bytes = read_file(file, &sheet.sheet);
	return 0;
}

static size_t read_file (FILE *const file, char **const mem)
{
	fseek(file, 0, SEEK_END);
	const size_t bytes = ftell(file);
	fseek(file, 0, SEEK_SET);

	*mem = (char*) calloc(bytes + 1, sizeof(char));
	e_check_ptr(mem);

	const size_t leidos = fread(*mem, 1, bytes, file);
	if (leidos != bytes)
		fprintf(stderr, "\t[s4Term:warning]: not whole file was read: %ld/%ld (B)\n", leidos, bytes);

	fclose(file);
	return bytes;
}
