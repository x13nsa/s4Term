#include "error.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>

void e_print_usage (void)
{
	fprintf(stderr, "\ts4Term [basic version] - compiled at %s %s\n", __DATE__, __TIME__);
	fputs("\tusage: s4Term [filename]\n", stderr);
	exit(EXIT_SUCCESS);
}

void e_system (const char *const msg)
{
	fprintf(stderr, "\t[s4Term:error]: %s\n", msg);
	perror("\tsystem error");
	exit(EXIT_FAILURE);
}

void e_check_ptr (const void *const ptr, const char *const where)
{
	if (ptr) return;
	e_system(where);
}

void e_at_lexing (const char *const fmt, const char* tok, const unsigned short nline, const unsigned short loff, ...)
{
	va_list args;
	va_start(args, loff);

	unsigned short len = 0;
	unsigned char stop = 0;
	fprintf(stderr, "\t[s4Term:error]: error at lexing at (%d:%d)\n\t", nline, loff);

	while (*tok != '\n')
	{
		if (*tok == ' ') stop = 1;
		if (!stop) len++;

		fputc(*tok++, stderr);
	}

	fprintf(stderr, "\n\t%.*s\n\t", len, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	fprintf(stderr, fmt, args);

	fputc(10, stderr);
	va_end(args);
	exit(EXIT_FAILURE);
}
