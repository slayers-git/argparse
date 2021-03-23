/* argparse - fast argument parsing tool 
 * Copyright (C) 2021 Sergey Lafin
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. */

#include "argparse.h"

/* Sub to parse the value of the argument
 * IN int * argc: - 
 * IN char *** argv: array pointer
 *
 * RETURN: return code */
static arg_return arg_parse_value (int * argc, char *** argv, void ** data, size_t * size) {
	if (!(***argv)) {
		if (*argc == 1)
			return ARG_NVALUE;
		else {
			--(*argc);
			++(*argv);
		}
	}
	*size = ARG_STRLEN (**argv);
	if (*size == 0) {
		*data = NULL;
		return ARG_NVALUE;
	}
	*data = **argv;
	**argv += *size;
	return ARG_SUCCESS;
}

static ARG_INLINE arg_return arg_parse_call_handler (int * argc, char *** argv, struct arg_argument * ptr) {
	void * data = NULL;
	size_t size = 0;

	/* TODO: Add a freaking check for non-value arguments */
	if (ptr->handler) {
		arg_return ret_code;
		++(**argv);
		ret_code = arg_parse_value(argc, argv, &data, &size);
		if (ret_code == ARG_NVALUE) {
			return ret_code;
		}
		if ((ret_code = ptr->handler(data, size, ptr->retval)) != 0)
			return ret_code;
	} else {
		++(**argv);
		*(char *)ptr->retval = 1;
	}
	return ARG_SUCCESS;
}

/* The author is very lazy and, apparentely, also has very doubtful
 * knowledge of C, so, to avoid using all these fancy things like
 * __thread, he came up with the stupidest idea ever: "why don't I
 * pass the state of the parser to every single function?" */

/* Sub to parse long arguments 
 *   IN size_t * argc: -
 *   IN char *** argv: pointer to the array
 *   IN arg_list list: the list to parse from
 *   
 *   RETURN: Parse code */
static arg_return arg_parse_long (int * argc, char *** argv, arg_list list, size_t len) {
	size_t i;
	for (i = 0; i < len; ++i) {
		if (ARG_STREQ(list[i].long_arg, **argv)) {
			
			arg_parse_call_handler (argc, argv, &list[i]);
		}
	}
	return ARG_SUCCESS;	
}

/* Sub to parse short arguments 
 *   IN size_t * argc: -
 *   IN char *** argv: pointer to the array
 *   IN arg_list list: the list to parse from
 *   
 *   RETURN: Parse code */
static arg_return arg_parse_short (int * argc, char *** argv, arg_list list, size_t len) {
	size_t i;
	for (i = 0; i < len; ++i) {
		if (list[i].short_arg == ***argv) {
			return arg_parse_call_handler (argc, argv, &list[i]);
		}
	}
	return ARG_NMATCH;	
}

static arg_return arg_parse_short_a (int * argc, char *** argv, arg_list list, size_t len) {
	arg_return code;
	while (***argv) {
		if (***argv == '-') return ARG_UNEXP;
		if ((code = arg_parse_short (argc, argv, list, len)) != ARG_SUCCESS) {
			return code;
		}
	}

	return ARG_SUCCESS;
}

/* Sub to parse non-arguments 
 *   IN size_t * argc: -
 *   IN char *** argv: pointer to the array
 *   IN arg_list list: the list to parse from
 *   
 *   RETURN: Parse code */
static arg_return arg_parse_non (int * argc, char *** argv, arg_list list, size_t len) {
	return ARG_SUCCESS;
}

/* Sub to determine the size of an arg_list
 *   IN const arg_list list: the list
 *
 *   RETURN: the size of the list */
static ARG_INLINE size_t arg_list_len (const arg_list list) {
	size_t size = 0;
	while (*(struct arg_argument **)list) {
		++list; ++size;
	}
	return size;
}

/* arg_parse implementation */
char ** arg_parse (int argc, char ** argv, arg_list list, arg_return * code) {
	/* The idea of spliting these lines, is to keep track
	 * of the exact cause */
	ARG_ASSERT(argc > 1);
	ARG_ASSERT(argv != NULL);
	ARG_ASSERT(list != NULL);

	size_t list_len = arg_list_len (list);
	assert (list_len > 0);

	if (argc == 1) {
		*code = ARG_ZERO;
		return argv;
	}

	++argv;
	arg_return ret_code = 0;
	while (--argc) {
		if (**argv == '-') {
			if (*(++*argv) == '-' && **argv) { /* If the "--" is passed we have to stop accepting the arguments */ 
				if ((ret_code = arg_parse_long (&argc, &argv, list, list_len)) != 0) {
					*code = ret_code;
					return argv;
				}
			}

			if (*(*argv) == 0x0) {
				*code = ARG_UNEXP;
				return argv;
			}
			if ((ret_code = arg_parse_short_a (&argc, &argv, list, list_len)) != 0) {
				*code = ret_code;
				return argv;
			}
			if (argc != 1)
				++argv;

			/* Not a key */
			arg_parse_non (&argc, &argv, list, list_len);
		}
	}
	*code = ARG_SUCCESS;
	return NULL;
}
