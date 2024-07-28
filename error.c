#include "error.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

void error_usage (void)
{
	static const char *usage = "\ts4term - spreadsheet (no TUI) - %s %s\n"
			                   "\tusage: s4term [-s sheet] [arguments]\n"
							   "\targuments:\n"
							   "\t  -o <*>         output\n"
							   "\t  -h             display this message\n";
	fprintf(stderr, usage, __DATE__, __TIME__);
	exit(EXIT_SUCCESS);
}

void error_fatal (const char *const fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	fputs("\t[s4term:error]: cannot continue with the execution of the program\n\t", stderr);
	vfprintf(stderr, fmt, args);

	fputc(10, stderr);
	exit(EXIT_FAILURE);
}

void error_check_ptr (const void *const p)
{
	if (p) return;
	error_fatal("your ran out of memory :(");
}

void error_at_lexer (const char *con, const char *const msg, unsigned short nline, unsigned short lpos, ...)
{
	va_list args;
	va_start(args, lpos);
	fprintf(stderr, "\t[s4term:error]: error while lexing at (%d:%d)\n\t\x1b[5;31m", nline, lpos);

	while (*con != '\n') {
		const char a = *con++;
		if (isspace(a))
			fprintf(stderr, "\x1b[0m");
		fputc(a, stderr);
	}
	fputs("\n\n\t", stderr);

	vfprintf(stderr, msg, args);
	fputc(10, stderr);
	exit(EXIT_FAILURE);
}
