/* This file is part of the argparse library.
 *
 * Copyright (C) 2021-2022 by Sergey Lafin
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory. */

#include "argparse.h"

#define ARG_LONG  0
#define ARG_SHORT 1

#if __STDC_VERSION__ >= 199903L
#	define ARG_INLINE inline
#else
#	define ARG_INLINE 
#endif

#ifdef ARGPARSE_ERRORS
#	define _inc_aerr
	/* a map with descriptions of all error codes */
	const char * arg_err_map [] = {
		[ARG_SUCCESS] = "",

		[ARG_HALT]   = "parsing was halted",
		[ARG_INVAL]  = "invalid argument",
		[ARG_UNEXP]  = "unexpected characters are met",
		[ARG_NMATCH] = "not an argument",
		[ARG_NVALUE] = "argument does not have a value"
	};
#endif

/* A simple implementation of some important "string.h" functions */
#ifdef ARG_STANDALONE
#   define ARG_ASSRT(str) /* Pretty much impossible to implement in standalone */
#	define ARG_STRLEN(str) arg_strlen(str)
#	define ARG_STRCPY(dest, src) arg_strcpy(dest, src)
#	define ARG_MEMCPY(dest, src, n) arg_memcpy(dest, src, n)
#	define ARG_STRCMP(a, b) arg_strcmp(a, b)

static ARG_INLINE size_t arg_strlen (const char * str) {
	size_t len = 0;
	while (*str) {
		++str; ++len;
	}
	return len;
}

static ARG_INLINE char * arg_strcpy (char * dest, const char * src) {
	while (*src) {
		*dest++ = *src++;
	}
	dest = 0x0;
	return dest;
}

static ARG_INLINE void * arg_memcpy (void * dest, const void * src, size_t n) {
	while (*(char *)dest) {
		*(char *)dest++ = *(char *)src++;
	}
	return dest;
}

static ARG_INLINE int arg_strcmp (const char * f, const char * s) {
	while (*f && *s) {
		if (!(*f++ == *s++)) {
			break;	
		}
	}
	return *(unsigned char *)f - *(unsigned char *)s;
}

#else
#	include <string.h>
#	define ARG_STRLEN(str) strlen(str)
#	define ARG_STRCPY(dest, src) strcpy(dest, src)
#	define ARG_MEMCPY(dest, src, n) memcpy(dest, src, n)
#	define ARG_STRCMP(a, b) strcmp(a, b)

/* Uses my own simplified assert */

#   include <stdio.h>
#   include <stdlib.h>
#   define ARG_ASSERT(x) { if (!(x)) arg_assert (#x); }
static ARG_INLINE void arg_assert (const char *expr) {
    fprintf (stderr, "argparse: assertion failed: %s\n", expr);
    abort ();
}

#endif

#define ARG_STREQ(a, b) (ARG_STRCMP(a, b) == 0)
#define RETPTR(p) ((void **)p)

arg_return arg_string_handler (char * data_ptr, size_t blksize, void * retval) {
	void ** _retval = RETPTR (retval);
	*_retval = data_ptr;
	return 0;
}
arg_return arg_strcpy_handler (char * data_ptr, size_t blksize, void * retval) {
	if (blksize == 0) {
		retval = NULL;
		return 0;
	}
	ARG_STRCPY((char *)retval, data_ptr);
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

char *arg_parse (int *argc, char ***argv, arg_list list, char **nk_buf, size_t *nk_size, arg_flags flags, arg_return *code) {
	ARG_ASSERT (argv != NULL);
	ARG_ASSERT (list != NULL);

	size_t len = arg_list_len (list);
	ARG_ASSERT (len > 0);

    /* if nk_buf is specified, so should be the nk_size */
    if (nk_buf)
        ARG_ASSERT (nk_buf != NULL && nk_size != NULL);

    size_t nk_lim = nk_buf ? *nk_size : 0;
    if (nk_buf)
        *nk_size = 0;

    ++(*argv);


	struct arg_state state;
	state.argc   =    argc;
	state.argv   =   *argv;
	state.flags  =   flags;
	state.list   =    list;
	state.ptr    =    NULL;
	state.len    =     len;

	char accept_args = 1;

    arg_return retcode;
#define arg (*state.argv)
    for ((*state.argc)--; *state.argc; ++state.argv, --(*state.argc)) {
        if (*arg != '-' || !accept_args) {
            /* if nkbuf is null, we considered the argument malformed */
            if (!nk_buf) {
                *code = ARG_UNEXP;
                *argv = state.argv;

                return arg;
            }

            if ((*nk_size)++ < nk_lim) {
                *nk_buf++ = arg;
            }
            continue;
        }

        if (*arg == '-') {
            ++arg;
            if (*arg == '-') {
                ++arg;
                /* just "--" */
                if (!(*arg)) {
                    accept_args = 0;
                } else {
                    state.type = ARG_LONG;
                    if ((*code = arg_parse_long (&state)) != ARG_SUCCESS) {
                        *argv = state.argv;
                        return arg - 2;
                    }
                }

                continue;
            }
            else {
                state.type = ARG_SHORT;
                if ((*code = arg_parse_short (&state)) != ARG_SUCCESS) {
                    *argv = state.argv;

                    return arg - 1;
                }
            }
        }
    }

    return NULL;
}

const char * arg_geterror (arg_return code) {
#ifdef _inc_aerr 
	return arg_err_map [code];
#else
	return NULL;
#endif
}
