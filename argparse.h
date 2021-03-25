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
 * 	flags can be merged together, if they are flags (i.e: tar -xzvf ...) */


#ifndef __SL_ARGPARSE_H__
#define __SL_ARGPARSE_H__

/* A simple implementation of some important "string.h" functions */
#ifdef ARG_STANDALONE
#	define ARG_ASSERT(x)
#	define ARG_STRLEN(str) arg_strlen(str)
#	define ARG_STRCPY(dest, src) arg_strcpy(dest, src)
#	define ARG_MEMCPY(dest, src, n) arg_memcpy(dest, src, n)
static size_t arg_strlen (char * str) {
	size_t len = 0;
	while (*str) {
		++str; ++len;
	}
	return len;
}
static char * arg_strcpy (char * dest, char * src) {
	while (*src) {
		*dest = *src;
		++dest; ++src;
	}
	dest = 0x0;
	return dest;
}
static void * arg_memcpy (void * dest, void * src, size_t n) {
	while (*dest) {
		*(char)dest = *(char *)src;
		++dest; ++src;
	}
	return dest;
}

#else
#	include <string.h>
#	include <assert.h>
#	define ARG_ASSERT(x) assert(x)
#	define ARG_STRLEN(str) strlen(str)
#	define ARG_STRCPY(dest, src) strcpy(dest, src)
#	define ARG_MEMCPY(dest, src, n) memcpy(dest, src, n)
#endif

#if __STDC_VERSION__ >= 199903L
#	define ARG_INLINE inline
#else
#	define ARG_INLINE 
#endif

#define ARG_BUFSIZ 20

#define ARG_ZERO   -1
#define ARG_INVAL  -2
#define ARG_NFOUND -3
#define ARG_HALT   -4
#define ARG_UNEXP  -5
#define ARG_NMATCH -6
#define ARG_NVALUE -7

/* If you want to make this work, you have to recompile the library
 * with -DARG_TRUE_EQ_ONE (makes successful returns equal to 1) */
#ifndef ARG_TRUE_EQ_ONE
#	define ARG_SUCCESS 0
#else
#	define ARG_SUCCESS 1
#endif

#define ARG_STREQ(a, b) (arg_strcmp(a, b) == 0)

/* A slightly altered strcmp(), will move the pointer of the second
 * string, but will restore it to the former state, if strings don't
 * match
 *   IN char * f: first string
 *   IN char ** f: second string
 *
 *   RETURN int:
 *     non-zero, if the strings don't match */
static int arg_strcmp (char * f, char ** s) {
	char * rest_ptr = *s;
	while (*f && **s) {
		if (!(*f == **s)) {
			break;	
		}
		++f; ++(*s);
	}
	int diff;
	if ((diff = *(unsigned char *)f - *(unsigned char *)*s) != 0)
		*s = rest_ptr;

	return diff;
}

typedef int p_arg_handler (void * data_ptr, size_t blksize, void * retval);

/* The handler is the parser for the value of the argument
 * if NULL is passed instead of a valid handler function,
 * the argument will be considered as a flag.
 * Otherwise it will wait for the data to be passed. */
typedef p_arg_handler * arg_handler;

/* A handler for a generic string. */
static p_arg_handler arg_string_handler;
/* A handler for a generic string, copies the data
 * to the static buffer */
static p_arg_handler arg_strcpy_handler;

static int arg_string_handler (void * data_ptr, size_t blksize, void * retval) {
	void ** _retval = (void **)retval;
	*_retval = data_ptr;
	return 0;
}
static int arg_strcpy_handler (void * data_ptr, size_t blksize, void * retval) {
	if (blksize == 0) {
		retval = NULL;
		return 0;
	}
	ARG_STRCPY((char *)retval, (char *)data_ptr);
	return 0;
}

/* This flag tells the parser to set the flag argument value to 0 instead of 1 */
#define ARG_FLAG_UNSET    0x01
/* This flag tells the parser that the value for this argument is optional */
/* #define ARG_FLAG_OPTIONAL 0x04 */
/* This flag tells the parser to stop when the argument is met and parsed */
#define ARG_FLAG_HALT     0x08

/* Flags define the behaviour of the arguments parser 
 * As example, ARG_FLAG_HALT will cause parser to be stopped when 
 * the specified argument is met. */
typedef unsigned char arg_flags;

/* Type for the return codes */
typedef signed char arg_return;

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

/* char ** arg_parse:
 *   IN int argc: argument count 
 *   IN char ** argv: arguments array 
 *   IN arg_list list: list, defining the accepted arguments 
 *   IN char ** not_keys: a buffer for random values
 *   IN size_t * not_keys_size: size of the not_keys buffer
 *   OUT arg_return * return_code: a return code of the status
 *     ARG_SUCCESS [0] if the parsing is done without failures
 *
 *   RETURN char ** arg: 
 *     - NULL on a complete success.
 *     - A pointer to the argv list's element which failed the parsing on
 *     a failure.
 *     - in case of a halt, return the pointer
 *     to the last argument parsed.
 *
 *   NOTE:
 *     this function changes the addresses of the pointers, to save them
 *     as they were, consider writing them into another set of variables. */
char ** arg_parse (int argc, char ** argv, arg_list list, char ** not_keys, size_t * not_keys_size, arg_return * return_code);

#endif
