#include "error.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>

void error_usage (void)
{
	static const char *usage = "\ts4Term - Spreadsheet for terminal, compiled %s %s\n"
	                           "\tUsage: s4term [args] [-s sheetname]\n"
							   "\tArgs:\n"
							   "\t  -o <!>    output file\n"
							   "\t  -h        display this message\n";
	fprintf(stderr, usage, __DATE__, __TIME__);
	exit(EXIT_SUCCESS);
}

void error_fatal (const char *err, ...)
{
	fputs("\t[s4Term:error]: fatal error\n\t", stderr);

	va_list args;
	va_start(args, err);
	vfprintf(stderr, err, args);

	fputc(10, stderr);
	if (errno) perror("\n\tsystem message");
	va_end(args);
	exit(EXIT_FAILURE);
}

void error_check_ptr (const void *const a)
{
	if (a) return;
	error_fatal("memory troubles");
}
