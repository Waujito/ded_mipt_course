#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <execinfo.h>
#include <signal.h>
#include <colors.h>

#include "argparse.h"
#include "equation_solvers.h"

/*

	   |\---/|
	   | ,_, |
	    \_`_/-..----.
	 ___/ `   ' ,""+ \
	(__...'   __\    |`.___.';
	  (_,...'(_,.`__)/'.....+

*/

enum {
	OPT_HELP
};

static struct option opts[] = {
	{ "help",	OPTIONAL_ARGUMENT,	OPT_HELP	},
	{ 0,		0,		0		},
};


int arg_callback(struct args_context ctx);

int arg_callback(struct args_context ctx) {
	printf("Option %s ", ctx.opt.name);
	if (ctx.value) {
		printf("with value %s", ctx.value);
	}
	printf("\n");

	return 0;
}

/*
int aaa (char* s)             { s++; s = "aa");

int bbb (char  s[])           { s++; s = "aa");
int bbb (char* const  s)      { s++; s = "aa");

int aaa (const char* s)       { s++; s = "aa");

int bbb (const char  s[])     { s++; s = "aa");
int bbb (const char* const s) { s++; s = "aa");
*/


/**
 * @brief Parses coefficients from stdin
 *
 * Ensures the coefficients are separated by spaces or newlines.
 *
 * @param [in, out] pol A pointer to the polynom structure. 
 * The structure should be filled up with nCoeffs so 
 * this function will determine a number of coefficients to be read.
 *
 * @return 0 if parsed successfully, -1 in case of an error
 */
int read_coefficients(struct polynom *pol);

/**
 * @brief Writes found roots to stdout
 *
 * @param roots Polynom roots
 *
 * @return 0 if written successfully, -1 in case of an error
 */
int print_roots(struct polynom_roots roots);


static void sigsegv_handler(int signo) {
	assert (signo == SIGSEGV);
	void *backtrace_ptrs[128];
	int ret = backtrace(backtrace_ptrs, 128);
	fprintf(stderr, COLOR_RED
	"!!! NEW ENTRY DETECTED !!!\n"
	"\n"
	"Poltorashka's Paw-ndora Log: Entry #9\n"
	"\n"
	"The sun was warm, the nap was deep.\n"
	"But a Vibe disturbed my sleep.\n"
	"A low hum, not-a-bird, not-a-bee,\n"
	"was coming from beneath the tree.\n"
	"\n"
	"My ear did twitch. My tail did sway.\n"
	"I simply had to dig and play.\n"
	"I pawed the earth, a frantic dance,\n"
	"and saw a faint and eerie GLANCE.\n"
	"\n"
	"Not a mouse. Not a toy. Not a treat.\n"
	"But something pulsing with a FEAT.\n"
	"A shard of rock, both dark and bright,\n"
	"with runes of ominous, crimson light:\n"
	"\n"
	"#######################################\n"
	"# !! S I G S E G V !! #\n"
	"#######################################\n"
	"\n"
	"\n"
	"\n"
	"HISSSS! I know this name from The Old Tome,\n"
	"(The one I nap on, back at home).\n"
	"It speaks of places Memory-forgot,\n"
	"and Pointer-things that HIT THE SPOT.\n"
	"\n"
	"A place you can't *quite* reach or see...\n"
	"It smells like $ sudo rm -rf / --no-purge-purr\n"
	"It is the Forbidden Address! The Ultimate segfault !\n"
	"\n"
	"I boop it with a careful paw.\n"
	"The world glitches. Static crawls.\n"
	"My whiskers twitch. The screen goes grey...\n"
	"...system reboot? Okay. Now pray.\n"
	"\n"
	"THE SIGSEGV IS FOUND.\n"
	"(This looks... interesting.)\n"
	"(Must find the System Wizard Human. They have the Dreamies.)\n"
	COLOR_CLEAR
	"\n\n"
	"A backtrace log: \n");

	backtrace_symbols_fd(backtrace_ptrs, ret, fileno(stderr));

	exit(EXIT_SUCCESS);
}

int main(int argc, const char *argv[]) {
	struct polynom pol = (struct polynom){0};
	int ret = 0;

	if (signal(SIGSEGV, sigsegv_handler) == SIG_ERR) {
		perror("SIGSEGV callback register");
		return 0;
	}

	ret = parse_args(argc, argv, opts, arg_callback, NULL);
	if (ret) {
		printf("What the f... error ^_^ %d\n", ret);
		return EXIT_FAILURE;
	}
	printf("\n");

	printf(	"Write coefficients a, b, c "
	       	"separated with spaces so they form "
		"a quadratic equation a*x^2 + b*x + c = 0 :\n");

	pol.nCoeffs = QUADRATIC_COEFFICIENTS_LEN;
	ret = read_coefficients(&pol);

	if (ret) {
		fprintf(stderr, "RTFM\n");
		return EXIT_FAILURE;
	}

	struct polynom_roots roots;
	ret = solve_polynomial(pol, &roots);
	if (ret) {
		fprintf(stderr, "Something went wrong while solving an equation: "
				"A solver exited with exit code %d\n", ret);
		return EXIT_FAILURE;
	}

	print_roots(roots);

	return EXIT_SUCCESS;
}

int read_coefficients(struct polynom *pol) { // TODO several tries
	assert (pol != NULL);
	assert (pol->nCoeffs <= MAX_COEFFICIENTS);

	int c = 0;

	for (int i = 0; i < pol->nCoeffs; i++) {

		int ret = scanf(" %lg", pol->coeffs + i);

		if (errno) {
			fprintf(stderr, "scanf() returned %d: %s",
				ret, strerror (errno));
			return -1;
		}

		if (ret != 1) {
			return -1;
		}

		c = getchar();
		if (c != ' ' && c != '\n') {
			return -1;
		}
	}

	if (c != '\n') {
		return -1;
	}

	return 0;
}

int print_roots(struct polynom_roots roots) {
	assert (roots.nRoots <= MAX_ROOTS);

	if (roots.nRoots == SQ_INF_ROOTS) {
		printf("The equation is valid for every rational, yoo-hoo! x\n");
		return 0;
	}

	printf("Found a total of %d solutions of the given equation", roots.nRoots);

	if (roots.nRoots != 0) {
		printf(" : ");

		for (int i = 0; i < roots.nRoots; i++) {
			printf("%lg ", roots.roots[i]);
		}
	}
	printf("\n");

	return 0;
}
