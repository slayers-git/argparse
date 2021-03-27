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

static char * project_name;

static char * move_to = NULL;
static char * not_moving = "not moving anywhere";

int main (int argc, char ** argv) {
	/* argparse does not check if a program had recieved any arguments */
	if (argc == 1) {
		return 0;
	}

	/* An array, describing the options, that this program accepts */
	arg_list list = {
		/* s ,  long opt  ,      handler      ,  value buffer  */ 
		{ 'v', "verbose"  , NULL              , &is_verbose    },
		{ 'c', "create"   , NULL              , &should_create },
		{ 'j', "jump"     , NULL              , &jump          },
		{ 'J', "jump-last", NULL              , &jump_last     },
		{ 'm', "move"     , arg_string_handler, &move_to       },
		{ 0 }
		/* NOTE */
		/* 's' stands for "short option" here
		 * "handler" is a function that accepts a parsed value
		 * of the argument, modifying and writing it into the 
		 * value buffer */
	};

	/* return code of the parser */
	arg_return code;
	/* in case of an error, this is going to point to the program's argument
	 * that was the cause of it */
	char * ptr;

	/* buffer for the elements, that are not keys */
	char * nk_buffer = NULL;
	/* size of the nk_buffer */
	size_t buf_size;

	/* parse the program's arguments */
	if ((ptr = arg_parse (&argc, &argv, list, &nk_buffer, &buf_size, ARG_PARSE_DEFAULT, &code)) != NULL) {
		printf ("An error occured here: %s", ptr);
		return code;
	}

	/* if the user entered an argument, that does not belong to any key */
	if (nk_buffer)
		project_name = nk_buffer;

	if (move_to == NULL)
		move_to = not_moving;

	printf ("Name: %s\nCreate? %i\nVerbose? %i\nJump? %i\nJump-last? %i\nMove to: %s\n", (const char *)project_name, should_create, is_verbose, jump, jump_last, move_to);

	return 0;
}

```

## Installation

To install argparse on a unix system just type `make && sudo make install`
