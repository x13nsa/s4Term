#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void error_print_usage (void)
{
	const char *usage = "\ts4t-b - basic spreadsheet for terminal\n"
			            "\tusage: s4tb [-s sheet] [arguments]\n"
						"\targuments:\n"
						"\t  -o <*>         display output in * (stdout by default)\n"
						"\t  -h             display this message\n";

	fprintf(stderr, usage, __DATE__, __TIME__);
	exit(EXIT_SUCCESS);
}

void error_bad_use (const char *const err, ...)
{
	va_list args;
	va_start(args, err);

	fputs("\t[s4tb:error]: fatal error\n\t", stderr);
	vfprintf(stderr, err, args);

	fputc(10, stderr);
	exit(EXIT_FAILURE);
}

void error_check_ptr (const void *const p)
{
	if (p) return;
	error_bad_use("out of memory");
}
