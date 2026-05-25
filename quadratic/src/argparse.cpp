/**
 * @file
 *
 * @brief argparse machine
 */
#include <assert.h>

#include "argparse.h"

enum lookup_option_status {
	LOOKUP_ERROR = -1,
	LOOKUP_CONTINUE = 0,
	LOOKUP_FOUND = 1,
};

/**
 * @brief Compares an option with an argument.
 *
 * @param [in, out] argi A pointer to index being processed. 
 * May be modified by this function if parameter for argument was used.
 *
 * @param [in] opt A pointer to option to be processed
 *
 * @param [out] actx A pointer to args_context. If the argument is compared,
 * it is filled up with opt and value. 
 * __Note__ a context to a callback should be filled manually
 *
 */
static enum lookup_option_status lookup_for_option(
			int argc, int *argi, 
			const char *argv[], struct option *opt,
			struct args_context *actx) {
	assert(argi);
	assert(argv);
	assert(opt);
	assert(actx);

	// TODO allow option without an argument
	const char *optname	= opt->name;
	const char *arg_name	= argv[*argi];
	const char *value	= 0;

	// read dashes
	int nDashes = 0;
	while (*arg_name == '-') nDashes++, arg_name++;

	int cmp_len = 0;
	while (*optname && *arg_name && *optname == *arg_name)
		optname++, arg_name++, cmp_len++;

	if (*arg_name && *arg_name != '=') {
		return LOOKUP_CONTINUE;
	}

	if (	(nDashes == 1 && cmp_len != 1) || 
		nDashes != 2)
		return LOOKUP_ERROR;

	if (*arg_name == '=') {
		value = ++arg_name;
	} else if (opt->has_arg == REQUIRED_ARGUMENT) {
		if (*argi + 1 < argc) {
			value = argv[++(*argi)];
		} else {
			return LOOKUP_ERROR;
		}
	}

	if (opt->has_arg == REQUIRED_ARGUMENT && !value) {
		return LOOKUP_ERROR;
	}

	actx->opt = *opt;
	actx->value = value;

	return LOOKUP_FOUND;
}

int parse_args(int argc, const char *argv[], 
	       struct option opts[],
	       args_callback callback, void *context) {
	assert(argv);
	assert(opts);
	assert(callback);

	for (int argi = 1; argi < argc; argi++) {
		struct args_context ctx = {0};
		
		for (int opti = 0; opts[opti].name; opti++) {
			enum lookup_option_status ret = lookup_for_option(
				argc, &argi, argv,
				opts + opti, &ctx
			);

			if (ret == LOOKUP_CONTINUE) {
				continue;
			} else if (ret == LOOKUP_FOUND) {
				break;
			} else {
				return -argi;
			}
		}

		if (ctx.opt.name != 0) {
			ctx.context = context;
			int ret = callback(ctx);
			if (ret) {
				return -argi;
			}
		} else {
			return -argi;
		}

	}

	return 0;
}
