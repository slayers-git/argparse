#include <argparse.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* This is a very simple implementation of a calculator 
 * usage:
 * 	example -o[+, -, /, *, %] op1 op2 
 * 	or
 * 	example --operator [+, -, /, *, %] op1 op2 */

/* define a very simple list of operators */
enum operator {
	plus,
	minus,
	multiply,
	divide,
	modulo,
	undef
};

/* convinient macro */
#define STREQ(a, b) (strcmp (a, b) == 0)

/* Usage info */
#define USAGE \
	 "Usage:\n" \
	 "\texample -o[+, -, /, *, %] op1 op2\n" \
	 "or\n" \
	 "\texample --operator [+, -, /, *, %] op1 op2"

static char op = undef;

/* the custom handler for our enumerator 
 * 
 * handlers modify the value of the argument
 * to write it in retval the right way */
arg_return custom_enum_handler (char * data, size_t size, void * retval) {
	if (STREQ (data, "+")) {
		op = plus;
	}
	else if (STREQ (data, "-")) {
		op = minus;
	}
	else if (STREQ (data, "/")) {
		op = divide;
	}
	else if  (STREQ (data, "*")) {
		op = multiply;
	}
	else if  (STREQ (data, "%")) {
		op = modulo;
	}
	else {
		/* The value is not an operator, so we return ARG_NMATCH */
		return ARG_NMATCH;
	}
	/* Everything is good, return ARG_SUCCESS */
	return ARG_SUCCESS;
}

int main (int argc, char ** argv) {
	if (argc == 1) {
		puts ("You haven't specified any arguments");
		return 0;
	}
	/* A description of the parser rules
	 * a full argument rule would look like this:
	 * { 'o', "operator", custom_enum_handler, &op, ARG_FLAG_DEFAULT }
	 *
	 * Where:
	 * 'o' is the short key
	 * "operator" is the long key
	 * custom_enum_handler is the handler
	 * &op is the return value for the handler
	 * ARG_FLAG_DEFAULT is flag, defining the parser's behaviour, when the key is met */
	arg_list list = {
		{ 'o', "operator", custom_enum_handler, &op }, /* C allows to drop ARG_FLAGS_DEFAULT */
		/* if the handler == NULL, the parser will consider the argument a flag and write
		 * 1 into the return value (unless ARG_FLAG_UNSET is passed, which will write 0 in the ret. val).
		 * If the return value passed == 0, nothing will happen.
		 * Here, because `flags` is set to ARG_FLAG_HALT, the parser will stop, when the key is met. */
		{ 'h', "help", NULL, NULL, ARG_FLAG_HALT }, 
		{ 0 } /* any argument list should end with 0, so the parser could count the number of rules */
	};

	/* this variable will store the faulty argument (if there are any) */
	char * err_arg;
	/* the return code of the parser (should be ARG_SUCCESS <0>) */
	arg_return code;

	/* the buffer for the *not-a-key* value (value, not preceeded by a key) */
	char * nk_buf[2];
	/* size of the written buffer */
	size_t nk_siz;

	/* call the parser
	 * if an error occured, the parser will stop and return the faulty argument and will write the error
	 * code to `code`. On success it will return NULL, and the return code will be ARG_SUCCESS. 
	 *
	 * here:
	 * &argc is a pointer to the argc variable
	 * &argv is a pointer to the argv variable
	 * list is the argument list
	 * nk_buf is the buffer for storing *not-a-key* values
	 * &nk_siz is the variable, that is going to store the size of the nk_buf
	 * ARG_PARSE_MERGED is a flag, that tells the parser, that we allow merged key-value arguments (i.e -o+)
	 * &code is a variable that is going to store the return code of the parser */
	if ((err_arg = arg_parse (&argc, &argv, list, nk_buf, &nk_siz, ARG_PARSE_MERGED, &code)) != NULL) {
		/* if --help or -h is met, the return code will be set to ARG_HALT as defined in the parser rules*/
		if (code == ARG_HALT) {
			puts (USAGE);
			return 0;
		}
		/* if an error occured */
		printf ("Could not parse argument: %s [%s]\n", err_arg, arg_geterror (code));
		return 1;
	}

	if (op == undef) {
		puts ("Specify an operator with -o [+, -, /, *, %]");
		return 1;
	}

	/* Check, if user passed 2 arguments */
	if (nk_siz != 2) {
		puts ("You have to specify two operands");
		return 2;
	}
	
	int64_t a, b;
	/* write them into a and b */
	a = atol (nk_buf[0]);
	b = atol (nk_buf[1]);

	switch (op) {
	case plus:
		a += b;
		break;
	case minus:
		a -= b;
		break;
	case divide:
		a /= b;
		break;
	case multiply:
		a *= b;
		break;
	case modulo:
		a %= b;
		break;
	}

	printf ("The result is %li\n", a);

	return 0;
}
