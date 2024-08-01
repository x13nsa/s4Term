#include "error.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>

void error_print_usage (void)
{
	const char *usage = "\ts4t-b - basic spreadsheet for terminal\n"
			            "\tusage: s4tb [-s sheet] [arguments]\n"
						"\targuments:\n"
						"\t  -o <*>         display output in * (stdout by default)\n"
						"\t  -p <*>         decimal precision\n"
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

void error_at_lexer (const char *const fmt, char *err, const unsigned short line, const unsigned short pos, ...)
{
	fprintf(stderr, "\t[s4tb:error]: error at lexing at (%d: %d)\n\t\x1b[5;31m", line, pos);

	do {
		fputc(*err, stderr);
		if (isspace(*err)) fprintf(stderr, "\x1b[0m");
	} while (*err++ != '\n');

	fputc('\t', stderr);

	va_list args;
	va_start(args, pos);
	vfprintf(stderr, fmt, args);

	fputc('\n', stderr);
	exit(EXIT_FAILURE);
}
