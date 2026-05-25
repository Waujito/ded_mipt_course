/**
 * @file 
 *
 * @brief Declarations for argument parsing machine
 */

#ifndef ARGPARSE_H
#define ARGPARSE_H


enum has_arg_vals {
	REQUIRED_ARGUMENT,
	OPTIONAL_ARGUMENT
};

/**
 * @brief Option unit describing an argument
 */
struct option {
	/**
	 * @brief Argument name, e.g. `help` for `--help`
	 */
	const char *name;

	/**
	 * @brief Should the value follow argument?
	 *
	 * Possible values are enumerated in has_arg_vals
	 */
	int has_arg;

	/**
	 * @brief A numeric interpretation of argument.
	 *
	 * Typically, here is a large enum for each argument
	 */
	int val;
};

/**
 * @brief A context given to callback.
 */
struct args_context {
	/**
	 * @brief option parsed
	 */
	struct option opt;
	/**
	 * @brief A value of option, or NULL if it is not passed
	 */
	const char *value;
	/**
	 * @brief A pointer to some context by user
	 */
	void *context;
};

/**
 * @brief A callback
 */
typedef int (*args_callback)(struct args_context ctx);

/**
 * @brief Parses args from argc and argv
 *
 * Parses argc/argv for command line arguments. 
 * It is like a handwritten replacement for getopt_long.
 * But instead of global variable it works through callback functions and context pointers
 *
 * @param opts A list of option structs describing option-value mapping
 * @param callback A pointer to args_callback function. The function should take
 * struct args_context and return zero on success and anything else on failure.
 * @param context A unspecified pointer to something like settings. It will be passed to args_context
 *
 * @return 0 if all arguments parsed successfully, or negative index of failed argument if the error happened. Negative index of argument is ok because parse_args does not parse the first (i = 0) argument which is typically a command name.
 *
 */
int parse_args(int argc, const char *argv[],
	       struct option opts[],
	       args_callback callback, void *context);

#endif /* ARGPARSE_H */
