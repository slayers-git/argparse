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

#define ARG_LONG  0
#define ARG_SHORT 1

/* A simple implementation of some important "string.h" functions */
#ifdef ARG_STANDALONE
#	define ARG_ASSERT(x)
#	define ARG_STRLEN(str) arg_strlen(str)
#	define ARG_STRCPY(dest, src) arg_strcpy(dest, src)
#	define ARG_MEMCPY(dest, src, n) arg_memcpy(dest, src, n)
#	define ARG_STRCMP(a, b) arg_strcmp(a, b)

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

static int arg_strcmp (char * f, char * s) {
	while (*f && *s) {
		if (!(*f == *s)) {
			break;	
		}
		++f; ++s;
	}
	return *(unsigned char *)f - *(unsigned char *)s;
}

#else
#	include <string.h>
#	include <assert.h>
#	define ARG_ASSERT(x) assert(x)
#	define ARG_STRLEN(str) strlen(str)
#	define ARG_STRCPY(dest, src) strcpy(dest, src)
#	define ARG_MEMCPY(dest, src, n) memcpy(dest, src, n)
#	define ARG_STRCMP(a, b) strcmp(a, b)
#endif

#define ARG_STREQ(a, b) (ARG_STRCMP(a, b) == 0)

#if __STDC_VERSION__ >= 199903L
#	define ARG_INLINE inline
#else
#	define ARG_INLINE 
#endif

arg_return arg_string_handler (void * data_ptr, size_t blksize, void * retval) {
	void ** _retval = (void **)retval;
	*_retval = data_ptr;
	return 0;
}
arg_return arg_strcpy_handler (void * data_ptr, size_t blksize, void * retval) {
	if (blksize == 0) {
		retval = NULL;
		return 0;
	}
	ARG_STRCPY((char *)retval, (char *)data_ptr);
	return 0;
}

struct arg_state {
	size_t   len;
	int   * argc;
	int    flags;

	char ** argv;
	struct arg_argument * list;
	struct arg_argument * ptr;

	char type;
};

static ARG_INLINE size_t arg_list_len (arg_list list) {
	size_t r = 0;
	while (*(struct arg_argument **)list != NULL) {
		++r; ++list;
	}
	return r;
}

static ARG_INLINE void arg_parse_value (struct arg_state * state, void ** data, size_t * size) {
	*size = ARG_STRLEN (*state->argv);
	*data = *state->argv;
	/* We make so the pointer goes to the nullptr
	 * so if the argument is a short type, we 
	 * could end the iteration */	
	*state->argv += *size;
}

static ARG_INLINE arg_return arg_call_handler (struct arg_state * state) {
	if (state->ptr->flags & ARG_FLAG_OPTIONAL) {
		if (
				(state->type == ARG_SHORT && *(*state->argv + 1) != 0x0) || 
				*state->argc == 1 ||
				**(state->argv + 1) == '-'
		) {
			++(*state->argv);
			return ARG_SUCCESS;
		}
	}
	/* An argument with a mandatory value */
	if (state->ptr->handler) {
		void * data;
		size_t size;

		arg_return code;

		switch (state->type) {
		case ARG_SHORT:
			if (state->flags & ARG_PARSE_MERGED) {
				if (*(*state->argv + 1) == 0x0) {
					if (*state->argc == 1)
						return ARG_NVALUE;
					++state->argv;
					--(*state->argc);
					break;
				}
				++(*state->argv);
				break;
			}
			if (*(*state->argv + 1) != 0x0 || *state->argc == 1)
				return ARG_NVALUE;
			++state->argv;
			--(*state->argc);
			break;
		case ARG_LONG:
			if (*state->argc == 1) {
				return ARG_NVALUE;
			}
			++state->argv;
			--(*state->argc);
			break;
		}
		arg_parse_value (state, &data, &size);

		if ((code = state->ptr->handler (data, size, state->ptr->retval)) != ARG_SUCCESS) {
			return code;
		}
		return ARG_SUCCESS;
	}
	/* A flag */
	if (state->ptr->retval != NULL)
		*(char *)state->ptr->retval = state->ptr->flags & ARG_FLAG_UNSET ? 0 : 1;
	++(*state->argv);

	if (state->ptr->flags & ARG_FLAG_HALT)
		return ARG_HALT;

	return ARG_SUCCESS;
}

static ARG_INLINE arg_return arg_parse_short (struct arg_state * state) {
	size_t i;
	char found;

	arg_return code;

	while (**state->argv) {
		for (i = 0; i < state->len; ++i) {
			found = 0;
			if (**state->argv == state->list[i].short_arg) {
				state->ptr = &state->list[i];
				if ((code = arg_call_handler (state)) != ARG_SUCCESS) {
					return code;
				}
				found = 1;
				break;
			}
		}
		if (!found)
			return ARG_NMATCH;
	}

	return ARG_SUCCESS;
}

static ARG_INLINE arg_return arg_parse_long (struct arg_state * state) {
	size_t i;
	arg_return code;
	for (i = 0; i < state->len; ++i) {
		if (ARG_STREQ (state->list[i].long_arg, *state->argv)) {
			state->ptr = &state->list[i];
			if ((code = arg_call_handler (state)) != ARG_SUCCESS) {
				return code;
			}
			return ARG_SUCCESS;
		}
	}

	return ARG_NMATCH;
}

char * arg_parse (int * argc, char *** argv, arg_list list, char ** nk, size_t * nk_size, arg_flags flags, arg_return * code) {
	ARG_ASSERT (*argc > 1);
	ARG_ASSERT (argv != NULL);
	ARG_ASSERT (list != NULL);

	size_t len = arg_list_len (list);
	ARG_ASSERT (len > 0);

	if (nk_size)
		*nk_size = 0;

	struct arg_state state;
	state.argc   =    argc;
	state.argv   =   *argv;
	state.flags  =   flags;
	state.list   =    list;
	state.ptr    =    NULL;
	state.len    =     len;

	char accept_args = 1;

	arg_return _code;
	while (--(*state.argc)) {
		++state.argv;
		char * arg = *state.argv;
		/* Not a key */
		if ((*arg) != '-' || !accept_args) {
			if (!nk) {
				*code = ARG_UNEXP;
				*argv = state.argv;
				return arg;
			}
			++(*nk_size);
			*nk = *state.argv;
			++nk;
			continue;
		}

		/* long */
		if (*(arg + 1) == '-') {
			*state.argv += 2;
			if (!(**state.argv)) {
				accept_args = 0;
				continue;
			}
			state.type = ARG_LONG;
			if ((_code = arg_parse_long (&state)) != ARG_SUCCESS) {
				*code = _code;
				*argv = state.argv;
				return arg;
			}
			continue;
		}

		/* short */
		++(*state.argv);
		state.type = ARG_SHORT;
		if ((_code = arg_parse_short (&state)) != ARG_SUCCESS) {
			*code = _code;
			*argv = state.argv;
			return arg;
		}
	}

	return NULL;
}
