#include <stdlib.h>
#include <stdio.h>

#include "types.h"

#include "onegin.h"
#include "argparse.h"

#define DEFAULT_INPUT_FILENAME	"textonegin.txt"
#define DEFAULT_OUTPUT_FILENAME	".out.onegin_english.txt"

enum arg_options {
	OPT_HELP		= 0,
	OPT_IN_FILENAME		= 1,
	OPT_OUT_FILENAME	= 2,
};

static struct option opts[] = {
	{ "help",	OPTIONAL_ARGUMENT,	OPT_HELP	},
	{ "input",	REQUIRED_ARGUMENT,	OPT_IN_FILENAME	},
	{ "output",	REQUIRED_ARGUMENT,	OPT_OUT_FILENAME},
	{ 0,		0,		0		},
};

struct program_context {
	const char *in_filename;
	const char *out_filename;
};

static void init_program_ctx(struct program_context *ctx) {
	ctx->in_filename = DEFAULT_INPUT_FILENAME;
	ctx->out_filename = DEFAULT_OUTPUT_FILENAME;
}

const char *help_string =	"Sorts strings in poems\n"
				"\n"			
				"Possible flags:\n"			
				"--input filename - An input filename. Defaults to "	
				DEFAULT_INPUT_FILENAME "\n"
				"--output filename - An output filename. Defaults to "	
				DEFAULT_OUTPUT_FILENAME "\n";
				

static int arg_callback(struct args_context ctx) {
	log_debug("\nOption %s ", ctx.opt.name);
	if (ctx.value) {
		log_debug("with value %s", ctx.value);
	}
	log_debug("\n\n");

	struct program_context *pg_ctx = (struct program_context *)ctx.context;

	enum arg_options opt_val = (enum arg_options) ctx.opt.val;
	switch (opt_val) {
		case OPT_HELP:
			printf("%s\n", help_string);
			exit(EXIT_SUCCESS);
			break;
		case OPT_IN_FILENAME:
			pg_ctx->in_filename = ctx.value;
			break;
		case OPT_OUT_FILENAME:
			pg_ctx->out_filename = ctx.value;
			break;
		default:
			log_perror("Undefined argument\n");
			return -1;
	}

	return 0;
}

int main(int argc, const char *argv[]) {
	int ret = 0;

	printf("Hello, onegin!\n");

	struct program_context ctx = {0};
	init_program_ctx(&ctx);

	ret = parse_args(argc, argv, opts, arg_callback, &ctx);
	if (ret) {
		return EXIT_FAILURE;
	}

	ret = process_text(ctx.in_filename, ctx.out_filename);

	return 0;
}
