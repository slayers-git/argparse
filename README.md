# Introduction

argparse is a simple parser for the command line options, that implements the parser for the standart Unix-like options

## Example

```c
/* <this_example> --verbose -cj --move "/some/where/" test_project */
#include "argparse.h"
#include <stdio.h>

static char is_verbose = 0;
static char jump = 0;
static char jump_last = 0;
static char should_create = 0;
static char project_name [5000];

int main (int argc, char ** argv) {
	if (argc == 1) {
		return 0;
	}
	char buffer[5000] = { 0x0 };
	arg_list list = {
		{ 'v', "verbose", NULL, &is_verbose },
		{ 'c', "create", NULL, &should_create },
		{ 'j', "jump", NULL, &jump },
		{ 'J', "jump-last", NULL, &jump_last },
		{ 'm', "move", arg_string_handler, buffer },
		{ 0 }
	};

	arg_return code;
	char ** ptr;

	/* buffer for elements, that are not keys */
	char * nk_buffer[3];
	/* size of the written buffer */
	size_t buf_size;

	if ((ptr = arg_parse (argc, argv, list, nk_buffer, &buf_size, &code)) != NULL) {
		printf ("An error occured here: %s", *ptr);
		return code;
	}

	if (nk_buffer[0]) {
		ARG_STRCPY(project_name, nk_buffer[0]);
	}

	printf ("Name: %s\nCreate? %i\nVerbose? %i\nJump? %i\nJump-last? %i\nMove to: %s\n", (const char *)project_name, should_create, is_verbose, jump, jump_last, buffer);

	return 0;
}
```

## Installation

To install argparse on a unix system just type `make && doas make install`
