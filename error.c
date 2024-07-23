#include "error.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

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

void e_check_ptr (const void *const ptr)
{
	if (ptr) return;
	e_system("out of RAM");
}
