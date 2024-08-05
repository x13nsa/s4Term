#include "error.h"
#include <stdio.h>
#include <stdlib.h>

void error_usage (void)
{
	const char *usage = "\ts4t-b - basic spreadsheet for terminal\n"
						"\tusage: s4tb [-s sheet] [arguments]\n"
						"\targuments:\n"
						"\t  -o <*>         write output in * (stdout by default)\n"
						"\t  -p <*>         decimal precision of *\n";

	fprintf(stderr, usage, __DATE__, __TIME__);
	exit(EXIT_SUCCESS);
}
