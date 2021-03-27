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

/* Parsing:
 * 	check whether or not the argument belongs in line
 * 	check if its long or short type
 * 	parse accordingly:
 * 		short, meaning that the data can be inserted after a whitespace
 * 		long, meaning that the data can be inserted after a space char 
 *
 * 	keys can be merged together, if they are flags (i.e: tar -xzvf ...) or,
 * 	if ARG_PARSE_MERGED is passed to the arg_parse function, merged key-value
 * 	arguments can be used (i.e objdump -Mintel ...) */


#ifndef __SL_ARGPARSE_H__
#define __SL_ARGPARSE_H__

#include <stddef.h>

#define ARG_BUFSIZ 20

#define ARG_SUCCESS 0

#define ARG_ZERO   -1
#define ARG_INVAL  -2
#define ARG_NFOUND -3
#define ARG_HALT   -4
#define ARG_UNEXP  -5
#define ARG_NMATCH -6
#define ARG_NVALUE -7

/* Type for the return codes */
typedef signed char arg_return;

typedef arg_return p_arg_handler (void * data_ptr, size_t blksize, void * retval);

/* The handler is the parser for the value of the argument
 * if NULL is passed instead of a valid handler function,
 * the argument will be considered as a flag.
 * Otherwise it will wait for the data to be passed. */
typedef p_arg_handler * arg_handler;

/* A handler for a generic string. */
p_arg_handler arg_string_handler;
/* A handler for a generic string, copies the data
 * to the static buffer */
p_arg_handler arg_strcpy_handler;

/* This flag tells the parser to set the flag argument value to 0 instead of 1 */
#define ARG_FLAG_UNSET    0x01
/* This flag tells the parser that the value for this argument is optional */
#define ARG_FLAG_OPTIONAL 0x04
/* This flag tells the parser to stop when the argument is met and parsed */
#define ARG_FLAG_HALT     0x08

/* Global parser flags */
/* Defaults */
#define ARG_PARSE_DEFAULT   0x0
/* Allow merged arguments (i.e: objdump -Mintel ...) */
#define ARG_PARSE_MERGED   0x01

/* Flags define the behaviour of the arguments parser 
 * As example, ARG_FLAG_HALT will cause parser to be stopped when 
 * the specified argument is met. */
typedef unsigned char arg_flags;

/* struct arg_argument
 *   Contains a basic description of the argument list
 *   char short_arg: a short variant of the argument's key (i.e: -r)
 *   char * long_arg: a long variant of the argument's key (i.e: --recursive)
 *   arg_handler handler: a handler for the value to be parsed
 *   void * retval: the variable, which accepts the parsed value 
 *   arg_flags flags: flags, which define the argument's parsing behavior */
struct arg_argument {
	char short_arg;
	char * long_arg;
	arg_handler handler;
	void * retval;
	arg_flags flags;
};

/* Contains a list of the arguments, defining their descriptions.
 * Put a {0} to mark the end of the list */
typedef struct arg_argument arg_list[];

/* char * arg_parse:
 *   @param argc
 *     argument count 
 *   @param argv
 *     arguments array 
 *   @param list
 *     list, defining the accepted arguments 
 *   @param not_keys
 *     a buffer for random values
 *   @param not_keys_size
 *     size of the not_keys buffer
 *   @param flags
 *     flags, defining the behaviour of the parser
 *   @param return_code
 *     return code of the parser
 *     NOTE: ARG_SUCCESS [0] is returned on success
 *
 *   RETURN char * arg: 
 *     - NULL on a complete success.
 *     - A pointer to the argv list's element which failed the parsing on
 *     a failure.
 *
 *   NOTES:
 *   1 this function is re-entrant, meaning, that if an error occured, you
 *     can call this function again with the same set of variables and
 *     continue parsing.
 *   2 this function changes the addresses of the pointers, to save them
 *     as they were, consider writing them into another set of variables. */
char * arg_parse (int * argc, char *** argv, arg_list list, char ** not_keys, size_t * not_keys_size, arg_flags flags, arg_return * return_code);

#endif
