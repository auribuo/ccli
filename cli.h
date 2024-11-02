/**
 * @file cli.h
 * @brief POSIX compliant cli option parser
 * @author Aurelio Buonomo
 * @version 1.0.0
 */
#ifndef CCLI_H
#define CCLI_H
#define CCLI_IMPLEMENTATION

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @def ARG_NULL
 * @brief The NULL argument. Used for termination of the options array.
 */
#define ARG_NULL 0b0000000000000000

/**
 * @def ARG_REQ_MASK
 * @brief Bitmask for the bit controlling if a option is required.
 */
#define ARG_REQ_MASK 0b1000000000000000

/**
 * @def ARG_POS_MASK
 * @brief Bitmask for the bit controlling if a option is positional.
 */
#define ARG_POS_MASK 0b0100000000000000

/**
 * @def ARG_MAT_MASK
 * @brief Bitmask used internally to check if a option is matched.
 */
#define ARG_MAT_MASK 0b0010000000000000

/**
 * @def ARG_TYP_MASK
 * @brief Bitmask for the bits defining the type of the option. See @ref option_type_t.
 */
#define ARG_TYP_MASK 0b0000000000011111

/**
 * @def ARG_CMD_MASK
 * @brief Bitmask for the bits controlling the command of a option.
 */
#define ARG_CMD_MASK 0b0001111111100000

/**
 * @def ARG_MAKE(typ, req, pos, cmd)
 * @brief Evaluates to the params field of a @ref option_t.
 * @param typ The type of the option See @ref option_type_t
 * @param req 1 if the option is required, else 0
 * @param pos 1 if the option is positional, else 0
 * @param cmd 0 if the option is global. 1 if the flag is a flag of the root command. Else n + 2 where n is the index of the command in the commands array
 *
 * @section option params structure
 *
 * The params field of an @ref option_t has the following structure:
 *
 * Binary representation: 0000000000000000
 * Legend:                rpmccccccccttttt
 * r = required
 * p = positional
 * m = matched
 * c = command where 0 = global, 1 = root, n - 2 = index in commands
 * t = type
 */
#define ARG_MAKE(typ, req, pos, cmd) \
    ((req << 15) | (pos << 14) | (cmd << 5) | typ) & (~ARG_MAT_MASK)

/**
 * @def ARG_MAKE_GLOBAL(typ, req, pos)
 * @brief Evaluates to the params field of a global @ref option_t.
 * @param typ The type of the option See @ref option_type_t
 * @param req 1 if the option is required, else 0
 * @param pos 1 if the option is positional, else 0
 */
#define ARG_MAKE_GLOBAL(typ, req, pos) ARG_MAKE(typ, req, pos, 0)

/**
 * @def ARG_MAKE_ROOT(typ, req, pos)
 * @brief Evaluates to the params field of a root @ref option_t.
 * @param typ The type of the option See @ref option_type_t
 * @param req 1 if the option is required, else 0
 * @param pos 1 if the option is positional, else 0
 */
#define ARG_MAKE_ROOT(typ, req, pos) ARG_MAKE(typ, req, pos, 1)

/**
 * @def ARG_MAKE_CMD(typ, req, pos, cmd)
 * @brief Evaluates to the params field of a @ref option_t associated to the command with the given index.
 * @param typ The type of the option See @ref option_type_t
 * @param req 1 if the option is required, else 0
 * @param pos 1 if the option is positional, else 0
 * @param cmd The index of the command in the commands array
 */
#define ARG_MAKE_CMD(typ, req, pos, cmd) ARG_MAKE(typ, req, pos, (cmd + 2))

/**
 * @def ARG_TYPE(arg)
 * @brief Evaluates to the type of the given option.
 * @param arg The params field of the @ref option_t
 */
#define ARG_TYPE(arg) (arg & ARG_TYP_MASK)

/**
 * @def ARG_CMD(arg)
 * @brief Evaluates to the cmd part of the given option.
 * @param arg The params field of the @ref option_t
 */
#define ARG_CMD(arg) ((arg & ARG_CMD_MASK) >> 5)

/**
 * @def ARG_CMD_IDX(arg)
 * @brief Evaluates to the index in the command field of the given option. Using it on global or root options results in an invalid index.
 * @param arg The params field of the @ref option_t
 */
#define ARG_CMD_IDX(arg) ((arg & ARG_CMD_MASK) >> 5) - 2

/**
 * @def ARG_GLOBAL(arg)
 * @brief Evaluates whether the option is global or not.
 * @param arg The params field of the @ref option_t
 */
#define ARG_GLOBAL(arg) ((arg & ARG_CMD_MASK) >> 5) == 0

/**
 * @def ARG_ROOT(arg)
 * @brief Evaluates whether the option is a root flag or not.
 * @param arg The params field of the @ref option_t
 */
#define ARG_ROOT(arg) ((arg & ARG_CMD_MASK) >> 5) == 1

/**
 * @def ARG_REQUIRED(arg)
 * @brief Evaluates whether the option is required or not.
 * @param arg The params field of the @ref option_t
 */
#define ARG_REQUIRED(arg) (arg & ARG_REQ_MASK) == ARG_REQ_MASK

/**
 * @def ARG_POSITIONAL(arg)
 * @brief Evaluates whether the option is positional or not.
 * @param arg The params field of the @ref option_t
 */
#define ARG_POSITIONAL(arg) (arg & ARG_POS_MASK) == ARG_POS_MASK

/**
 * @def ARG_MATCHED(arg)
 * @brief Evaluates whether the option is matched or not.
 * @param arg The params field of the @ref option_t
 */
#define ARG_MATCHED(arg) (arg & ARG_MAT_MASK) == ARG_MAT_MASK

/**
 * @def ARG_SET_MATCHED(arg)
 * @brief Evaluates to the same params field but with the matched option bit set.
 * @param arg The params field of the @ref option_t
 */
#define ARG_SET_MATCHED(arg) (arg | ARG_MAT_MASK)

/**
 * @brief Union holding all possible data of a parsed option.
 */
typedef union opt_data_u {
    char *str_data;     /**< The string data of the option */
    int64_t num_data;   /**< The numer data of the option */
    uint64_t unum_data; /**< The unsigned data of the option */
    bool bool_data;     /**< The boolean data of the option */
} opt_data_t;

/**
 * @brief Represents a single option of the cli.
 */
typedef struct option_s {
    char short_arg;   /**< The shorthand version of the option. Set to 0 if not required */
    char *long_arg;   /**< The long version and name of the option. Required */
    uint16_t params;  /**< The params field of the option. See @ref ARG_MAKE */
    opt_data_t *data; /**< The data field of the option. After parsing holds the data passed down in the cli. Accessing fields not matching the type of the option is undefined behaviour */
    char *desc;       /**< Optional description to print in the help menu */
    char *arg_desc;   /**< Description/name of the parameter of the option. Only applicable to string and boolean options*/
} option_t;

/**
 * @brief Represents a cli command.
 */
typedef struct command_s {
    char *command; /**< The name of the command. Required */
    char *desc;    /**< Optional description to print in the help menu */
} command_t;

/**
 * @brief Represents a single exclusion between two options.
 *
 * Note: This system is a bit janky at the moment and only really works between two options that appear only once in all exclusions.
 */
typedef struct exclusion_s {
    char *one;      /**< Name of the first option */
    char *other;    /**< Name of the second option */
    bool satisfied; /**< Indicates whether the exclusion is satisfied */
} exclusion_t;

/**
 * @brief Represents all valid option types. The value of each field matches the value in the type portion in the params field of the option.
 */
typedef enum option_type_e {
    boolean = 1, /**< Indicates a boolean (on/off) option */
    string = 2,  /**< Indicates a string option */
    number = 4,  /**< Indicates a integer option */
    unumber = 8, /**< Indicates an unsigned integer option */
} option_type_t;

/**
 * @brief Represents an example to print in the help menu.
 */
typedef struct example_s {
    char *options;     /**< Only the options the cli has to be run with */
    char *description; /**< The description of the action performed */
} example_t;

#ifdef CCLI_IMPLEMENTATION
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

_Noreturn void cli_panic(const char *msg) {
    if (msg != NULL) {
        fprintf(stderr, "cli_panic: %s\n", msg);
    } else {
        fprintf(stderr, "cli_panic: Program cli_paniced.");
    }
    exit(1);
}

_Noreturn void cli_panicf(const char *msg, ...) {
    va_list argptr;
    va_start(argptr, msg);

    if (msg != NULL) {
        fprintf(stderr, "cli_panic: ");
        vfprintf(stderr, msg, argptr);
        putchar('\n');
    }
    va_end(argptr);
    exit(1);
}

_Noreturn void cli_fatal(const char *bin, const char *msg) {
    if (msg != NULL) {
        fprintf(stderr, "%s: %s\n", bin, msg);
    }
    exit(1);
}

_Noreturn void cli_fatalf(const char *bin, const char *format, ...) {
    va_list argptr;
    va_start(argptr, format);

    if (format != NULL) {
        fprintf(stderr, "%s: ", bin);
        vfprintf(stderr, format, argptr);
        putchar('\n');
    }
    va_end(argptr);
    exit(1);
}

_Noreturn void cli_fatalf_help(const char *bin, const char *format, ...) {
    va_list argptr;
    va_start(argptr, format);

    if (format != NULL) {
        fprintf(stderr, "%s: ", bin);
        vfprintf(stderr, format, argptr);
        fprintf(stderr, ". For more information see %s --help\n", bin);
    }
    va_end(argptr);
    exit(1);
}

void cli_check_alloc(void *ptr) {
    if (ptr == NULL) {
        cli_panic("Allocation error. Could not allocate memory");
    }
}

bool cli_try_parse_int(char *num, int64_t *data) {
    char *end_ptr;
    errno = 0;
    bool failed = false;

    int64_t parsed = strtol(num, &end_ptr, 10);

    if (end_ptr == num) {
        failed = true;
    } else if (errno == ERANGE && (parsed == LONG_MAX || parsed == LONG_MIN)) {
        failed = true;
    } else if (errno != 0 && num == 0) {
        failed = true;
    } else if (*end_ptr != '\0') {
        failed = true;
    }

    if (failed) {
        if (strlen(num) <= 2 || num[0] != '0' || num[1] != 'x') {
            failed = true;
        } else {
            failed = false;
            char *end_ptr_hex;
            parsed = strtol(num, &end_ptr_hex, 16);

            if (end_ptr_hex == num) {
                failed = true;
            } else if ((parsed == LONG_MAX || parsed == LONG_MIN)) {
                failed = true;
            } else if (errno != 0 && num == 0) {
                failed = true;
            } else if (*end_ptr_hex != '\0') {
                failed = true;
            }
        }
    }

    if (failed) {
        if (strlen(num) <= 2 || num[0] != '0' || num[1] != 'b') {
            return false;
        }

        num += 2;

        failed = false;
        char *end_ptr_bin;
        parsed = strtol(num, &end_ptr_bin, 2);

        if (end_ptr_bin == num) {
            failed = true;
        } else if ((parsed == LONG_MAX || parsed == LONG_MIN)) {
            failed = true;
        } else if (errno != 0 && num == 0) {
            failed = true;
        } else if (*end_ptr_bin != '\0') {
            failed = true;
        }
    }

    if (!failed) {
        *data = parsed;
    }

    return !failed;
}

bool cli_try_parse_uint(char *num, uint64_t *data) {
    char *end_ptr;
    errno = 0;
    bool failed = false;

    uint64_t parsed = strtoll(num, &end_ptr, 10);

    if (end_ptr == num) {
        failed = true;
    } else if (errno == ERANGE && (parsed == UINT64_MAX || parsed == UINT64_MAX)) {
        failed = true;
    } else if (errno != 0 && num == 0) {
        failed = true;
    } else if (*end_ptr != '\0') {
        failed = true;
    }

    if (failed) {
        if (strlen(num) <= 2 || num[0] != '0' || num[1] != 'x') {
            failed = true;
        } else {
            failed = false;
            char *end_ptr_hex;
            parsed = strtol(num, &end_ptr_hex, 16);

            if (end_ptr_hex == num) {
                failed = true;
            } else if ((parsed == UINT64_MAX || parsed == UINT64_MAX)) {
                failed = true;
            } else if (errno != 0 && num == 0) {
                failed = true;
            } else if (*end_ptr_hex != '\0') {
                failed = true;
            }
        }
    }

    if (failed) {
        if (strlen(num) <= 2 || num[0] != '0' || num[1] != 'b') {
            return false;
        }

        num += 2;

        failed = false;
        char *end_ptr_bin;
        parsed = strtol(num, &end_ptr_bin, 2);

        if (end_ptr_bin == num) {
            failed = true;
        } else if ((parsed == UINT64_MAX || parsed == UINT64_MAX)) {
            failed = true;
        } else if (errno != 0 && num == 0) {
            failed = true;
        } else if (*end_ptr_bin != '\0') {
            failed = true;
        }
    }

    if (!failed) {
        *data = parsed;
    }

    return !failed;
}

bool cli_streq(const char *s1, const char *s2) {
    if (s1 == NULL || s2 == NULL) {
        return s1 == s2;
    }
    if (s1 == s2) {
        return true;
    }
    return strcmp(s1, s2) == 0;
}

bool cli_strcontains(const char *s, char c) {
    if (s == NULL) {
        return false;
    }

    for (size_t i = 0; i < strlen(s); i++) {
        if (s[i] == c) {
            return true;
        }
    }

    return false;
}

int64_t cli_stridx(const char *s, char c) {
    int64_t idx = -1;
    if (s == NULL) {
        return idx;
    }

    for (size_t i = 0; i < strlen(s) && idx == -1; i++) {
        if (s[i] == c) {
            idx = i;
        }
    }

    return idx;
}

/**
 * @brief Help option. Always present.
 */
const option_t help_opt = {'h', "help", ARG_MAKE_GLOBAL(boolean, false, false), NULL, "Show this help menu", NULL};

/**
 * @brief Represents all possible types of short options.
 */
typedef enum short_opt_type_e {
    none,     /**< Not a short option */
    single,   /**< Single option */
    multiple, /**< Multiple in one */
} short_opt_type_t;

/**
 * @brief Returns whether opt is a null (terminating) option.
 * @param opt The option to check
 * @return True if the option is considered a terminating option else false
 * @note The function checks for the params field to be @ref ARG_NULL
 */
bool _is_opt_null(option_t opt) { return opt.params == ARG_NULL; }

/**
 * @brief Returns whether cmd is a null (terminating) command.
 * @param cmd The command to check
 * @return True if the command is considered a terminating option else false
 * @note The function checks for the command field to be NULL
 */
bool _is_cmd_null(command_t cmd) { return cmd.command == NULL; }

/**
 * @brief Calculates the length of a zero-terminated @ref option_t array.
 * @param options The zero-terminated array of @ref option_t
 * @return The length of the array excluding the zero-terminator
 * @note Not having a zero-terminator can lead to undefined behaviour
 */
size_t _opt_len(option_t *options) {
    if (options == NULL) {
        return 0;
    }
    size_t idx = 0;
    option_t opt;
    while (!_is_opt_null(opt = options[idx++]))
        ;
    return idx - 1;
}

/**
 * @brief Calculates the length of a zero-terminated @ref command_t array.
 * @param commands The zero-terminated array of @ref command_t
 * @return The length of the array excluding the zero-terminator
 * @note Not having a zero-terminator can lead to undefined behaviour
 */
size_t _cmd_len(command_t *commands) {
    if (commands == NULL) {
        return 0;
    }
    size_t idx = 0;
    command_t cmd;
    while (!_is_cmd_null(cmd = commands[idx++]))
        ;
    return idx - 1;
}

/**
 * @brief Validates a zero-terminated @ref option_t array. cli_panics if options are not valid
 * @param options The zero-terminated array of @ref option_t
 */
void _validate_options(option_t *options) {
    for (size_t i = 0; i < _opt_len(options); i++) {
        option_t opt = options[i];
        if (opt.long_arg == NULL) {
            cli_panicf("Invalid option at index %lu. Long option is always required!", i);
        }
        if (ARG_TYPE(opt.params) != boolean && !(ARG_POSITIONAL(opt.params)) && opt.arg_desc == NULL) {
            cli_panicf("Invalid option %s. If option is not boolean arg_desc is required!", opt.long_arg);
        }
    }
}

/**
 * @brief Return whether the given option is relevant in the context of the given command.
 * @param arg_opt The params of the option to check
 * @param commands The zero-terminated array of @ref command_t
 * @param command Name of the current command
 * @return True if the option is relevant, else false
 */
bool _arg_relevant(uint32_t arg_opt, command_t *commands, char *command) {
    if (ARG_GLOBAL(arg_opt)) {
        return true;
    } else if (ARG_ROOT(arg_opt)) {
        return command == NULL;
    }
    return cli_streq(commands[ARG_CMD_IDX(arg_opt)].command, command);
}

/**
 * @brief Calculates the max length of the long arg in all of the given options in the context of the given command.
 * @param options The zero-terminated array of @ref option_t
 * @param commands The zero-terminated array of @ref command_t
 * @param command Name of the current command
 * @return The length of the longest option name + arg_desc
 */
size_t _max_long_arg_len(option_t *options, command_t *commands, char *command) {
    size_t max = 4; // Hardcoded to the word "help"

    for (size_t i = 0; i < _opt_len(options); i++) {
        option_t opt = options[i];
        if (!_arg_relevant(opt.params, commands, command)) {
            continue;
        }
        if (opt.long_arg == NULL) {
            continue;
        }
        size_t len = strlen(opt.long_arg);
        if (opt.arg_desc != NULL) {
            len += 3 + strlen(opt.arg_desc);
        }
        if (len > max) {
            max = len;
        }
    }

    return max;
}

/**
 * @brief Calculates the amount of positional options in the context of the given command.
 * @param options The zero-terminated array of @ref option_t
 * @param commands The zero-terminated array of @ref command_t
 * @param command Name of the current command
 * @return The amount of positional options
 */
size_t _pos_args_len(option_t *options, command_t *commands, char *command) {
    size_t count = 0;
    for (size_t i = 0; i < _opt_len(options); i++) {
        if (ARG_POSITIONAL(options[i].params) && _arg_relevant(options[i].params, commands, command)) {
            count++;
        }
    }
    return count;
}

/**
 * @brief Prints the help menu.
 * @param commands All commands of the cli. Set to NULL if there are no commands else a zero-terminated array of @ref command_t
 * @param command The command to print the help menu of. Set to NULL to print help for the root command
 * @param options All options of the cli as a zero-terminated array of @ref option_t
 * @param argv The argv array
 * @param examples Optional zero-terminated array of examples
 */
void cli_help(command_t *commands, char *command, option_t *options, char *argv[], example_t *examples) {
    uint32_t max_len = _max_long_arg_len(options, commands, command);
    size_t num_options = _opt_len(options);
    size_t num_commands = _cmd_len(commands);
    printf("Usage: \n");
    if (num_commands > 0) {
        if (command == NULL) {
            printf("\t%s [command]\n", argv[0]);
        }
    }
    printf("\t%s ", argv[0]);
    if (num_commands > 0 && command != NULL) {
        printf("%s ", command);
    }
    printf("[options] ");
    for (size_t i = 0; i < num_options; i++) {
        option_t opt = options[i];
        if (ARG_POSITIONAL(opt.params) && _arg_relevant(opt.params, commands, command)) {
            printf("%s ", opt.long_arg);
        }
    }
    if (num_commands > 0 && command == NULL) {
        printf("\n\nAvailable commands:\n");
        for (size_t i = 0; i < num_commands; i++) {
            command_t cmd = commands[i];
            char *padded_cmd = (char *)malloc(sizeof(char) * max_len + 1);
            cli_check_alloc(padded_cmd);
            memset(padded_cmd, ' ', max_len);
            memcpy(padded_cmd, cmd.command, strlen(cmd.command));
            padded_cmd[max_len] = 0;
            printf("\t%s      %s\n", padded_cmd, cmd.desc);

            free(padded_cmd);
        }
    } else {
        putchar('\n');
    }
    printf("\nAvailable options:\n");
    for (size_t i = 0; i < num_options; i++) {
        option_t opt = options[i];
        if (ARG_POSITIONAL(opt.params) || !_arg_relevant(opt.params, commands, command)) {
            continue;
        }
        char *padded_long;
        if (opt.long_arg != NULL) {
            padded_long = (char *)malloc(sizeof(char) * max_len + 1);
            cli_check_alloc(padded_long);
            memset(padded_long, ' ', max_len);
            memcpy(padded_long, opt.long_arg, strlen(opt.long_arg));
            padded_long[max_len] = 0;
            if (opt.arg_desc != NULL) {
                padded_long[strlen(opt.long_arg) + 1] = '<';
                memcpy(padded_long + strlen(opt.long_arg) + 2, opt.arg_desc, strlen(opt.arg_desc));
                padded_long[strlen(opt.long_arg) + 1 + strlen(opt.arg_desc) + 1] = '>';
            }
        } else {
            padded_long = (char *)malloc(sizeof(char) * max_len + 3);
            cli_check_alloc(padded_long);
            memset(padded_long, ' ', max_len + 3);
        }

        if (opt.short_arg == 0) {
            printf("\t  ");
        } else {
            printf("\t-%c", opt.short_arg);
        }
        printf(" --%s %s\n", padded_long, opt.desc);

        free(padded_long);
    }

    char *padded_long;
    padded_long = (char *)malloc(sizeof(char) * max_len + 1);
    cli_check_alloc(padded_long);
    memset(padded_long, ' ', max_len);
    memcpy(padded_long, help_opt.long_arg, strlen(help_opt.long_arg));
    padded_long[max_len] = 0;

    printf("\t-%c", help_opt.short_arg);
    printf(" --%s %s\n", padded_long, help_opt.desc);

    free(padded_long);

    if (_pos_args_len(options, commands, command) > 0) {
        printf("\nPositional options:\n");
        for (size_t i = 0; i < num_options; i++) {
            option_t opt = options[i];
            if (!(ARG_POSITIONAL(opt.params)) || !_arg_relevant(opt.params, commands, command)) {
                continue;
            }
            char *padded_long;
            padded_long = (char *)malloc(sizeof(char) * max_len + 1);
            cli_check_alloc(padded_long);
            memset(padded_long, ' ', max_len);
            memcpy(padded_long, opt.long_arg, strlen(opt.long_arg));
            padded_long[max_len] = 0;

            printf("\t%s      %s\n", padded_long, opt.desc);

            free(padded_long);
        }
    }

    if (examples != NULL) {
        printf("\nExamples:\n");
        example_t example;
        size_t idx = 0;
        while ((example = examples[idx++]).options != NULL) {
            printf("%s %s\t%s\n", argv[0], example.options, example.description);
        }
    }

    printf("\n\nUse `%s [command] --help` to get help for a specific command\n", argv[0]);
}

/**
 * @brief Returns whether the given option is a long style option or not.
 * @param opt The string of the option to check
 * @return True if the option is a long option, false if not
 */
bool _is_long_opt(char *opt) {
    size_t len = strlen(opt);
    if (len < 3) {
        return false;
    }
    return opt[0] == '-' && opt[0] == opt[1];
}

/**
 * @brief Determines the type of short option for the given string.
 * @param opt The string of the option to check
 * @return The type of the short option as described in @ref short_opt_type_t
 */
short_opt_type_t _short_opt_type(char *opt) {
    size_t len = strlen(opt);
    if (len < 2) {
        return none;
    }

    if (opt[0] != '-') {
        return none;
    }

    if (len >= 3) {
        return multiple;
    }

    return opt[0] == '-' && opt[1] != '-' ? single : none;
}

/**
 * @brief Returns whether the given string is an option or not.
 * @param opt The string if the option to check
 * @return True if the string is an option, else false
 * @note See @ref _short_opt_type and @ref _is_long_opt
 */
bool _is_option(char *opt) {
    return _is_long_opt(opt) || _short_opt_type(opt) != none;
}

/**
 * @brief Parses all the remaining values in argv as positional options. Errors if the amount does not coincide with the amount of positional options in the option array in the context of the given command.
 * @param options The zero-terminated array of @ref option_t
 * @param commands The zero-terminated array of @ref command_t
 * @param command Name of the current command
 * @param argc_idx The index to start the parsing from
 * @param argc The total length of argv
 * @param argv The argv array
 */
void _cli_parse_remaining_positionals(option_t *options, command_t *commands, char *command, int argc_idx, int argc, char **argv) {
    size_t pos_arg_count = _pos_args_len(options, commands, command);
    if (argc - argc_idx - 1 > pos_arg_count) {
        cli_fatalf_help(argv[0], "Too many positional arguments: Expected %d got %d", pos_arg_count, argc - argc_idx - 1);
    }

    for (; argc_idx < argc; argc_idx++) {
        char *arg = argv[argc_idx];
        if (strcmp(arg, "--") == 0 || strcmp(arg, "-") == 0) {
            continue;
        }
        for (size_t opt_search = 0; opt_search < _opt_len(options); opt_search++) {
            option_t opt = options[opt_search];
            if (!(ARG_MATCHED(opt.params)) && ARG_POSITIONAL(opt.params)) {
                options[opt_search].params = ARG_SET_MATCHED(opt.params);
                options[opt_search].data->str_data = arg;
            }
        }
    }

    if (argc_idx != argc) {
        cli_fatalf_help(argv[0], "Too few positional arguments: Expected %d got %d", pos_arg_count, argc - argc_idx);
    }
}

/**
 * @brief Returns whether the given string is equal to the name of the given option.
 * @param argv_opt The string to test
 * @param long_opt The name of the option
 * @return True if the options match, else false
 * @note The function checks if the long_opt with prepended '--' is equal to the given argv_opt
 */
bool _long_opt_eq(char *argv_opt, char *long_opt) {
    if (argv_opt == NULL || long_opt == NULL) {
        return false;
    }
    char *long_opt_with_dash = (char *)malloc(sizeof(char) * strlen(long_opt) + 3);
    cli_check_alloc(long_opt_with_dash);
    memcpy(long_opt_with_dash + 2, long_opt, strlen(long_opt) + 1);
    long_opt_with_dash[0] = long_opt_with_dash[1] = '-';
    bool are_eq = strcmp(argv_opt, long_opt_with_dash) == 0;
    free(long_opt_with_dash);
    return are_eq;
}

/**
 * @brief Checks if any required options in the context of the given command are not set. Errors on the first violation.
 * @param cmd_idx The index of the command
 * @param options The zero-terminated array of @ref option_t
 * @param mutual_exclusions The zero-terminated array of @ref exclusion_t to know if two required options are mutually exclusive
 */
void _check_unmatched(uint8_t cmd_idx, option_t *options, exclusion_t mutual_exclusions[]) {
    for (size_t opt_search = 0; opt_search < _opt_len(options); opt_search++) {
        option_t opt = options[opt_search];
        if (!(ARG_GLOBAL(opt.params)) && ARG_CMD(opt.params) != cmd_idx) {
            continue;
        }
        if (!(ARG_MATCHED(opt.params))) {
            if (ARG_REQUIRED(opt.params)) {
                size_t idx = 0;
                exclusion_t ex;
                bool can_proceed = false;
                if (mutual_exclusions != NULL) {
                    while ((ex = mutual_exclusions[idx++]).one != NULL) {
                        if (cli_streq(ex.one, opt.long_arg) || cli_streq(ex.other, opt.long_arg)) {
                            can_proceed = true;
                            break;
                        }
                    }
                }
                if (can_proceed) {
                    continue;
                }
                cli_fatalf_help("Missing required argument `%s`", opt.long_arg);
            }
        }
    }
}

/**
 * @brief Finds the help command among the given options to instantly print the help menu.
 * @param commands All commands of the cli. Set to NULL if there are no commands else a zero-terminated array of @ref command_t
 * @param command Name of the current command
 * @param options All options of the cli as a zero-terminated array of @ref option_t
 * @param argc The argc value
 * @param argv The argv array
 * @param examples Optional zero-terminated array of examples
 */
void _find_help(command_t *commands, char *command, option_t *options, int argc, char *argv[], example_t *examples) {
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        if (strcmp(arg, "--") == 0 || strcmp(arg, "-") == 0) {
            return;
        }
        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            cli_help(commands, command, options, argv, examples);
            exit(0);
        }
    }
}

/**
 * @brief Checks if any mutual exclusions are violated. Errors on the first violation.
 * @param cmd_idx The index of the command
 * @param options The zero-terminated array of @ref option_t
 * @param mutual_exclusions The zero-terminated array of @ref exclusion_t to know if two required options are mutually exclusive
 */
void _check_mutual_exclusions(uint8_t cmd_idx, option_t *options, exclusion_t mutual_exclusions[]) {
    if (mutual_exclusions == NULL) {
        return;
    }
    size_t idx = 0;
    exclusion_t exclusion;
    while ((exclusion = mutual_exclusions[idx++]).one != NULL) {
        if (exclusion.other == NULL) {
            cli_panic("check_mutual_exclusions: NULL in non NULL exclusion!");
        }
        if (strlen(exclusion.one) == 0 || strlen(exclusion.other) == 0) {
            cli_panic("check_mutual_exclusions: Empty exclusion!");
        }
        bool one_matched = false;
        bool other_matched = false;
        bool both_required = true;
        bool irrelevant = false;
        for (int i = 0; i < _opt_len(options); i++) {
            option_t opt = options[i];
            if (!(ARG_GLOBAL(opt.params)) && ARG_CMD(opt.params) != cmd_idx && (cli_streq(exclusion.one, opt.long_arg) || cli_streq(exclusion.other, opt.long_arg))) {
                irrelevant = true;
                break;
            }
            if (cli_streq(exclusion.one, opt.long_arg)) {
                one_matched = ARG_MATCHED(opt.params);
                both_required &= ARG_REQUIRED(opt.params);
                continue;
            }
            if (cli_streq(exclusion.other, opt.long_arg)) {
                other_matched = ARG_MATCHED(opt.params);
                both_required &= ARG_REQUIRED(opt.params);
                continue;
            }
        }
        if (irrelevant) {
            continue;
        }
        if (one_matched == false && other_matched == false && both_required) {
            cli_fatalf_help("One of the options `%s` and `%s` is required because they are both required but mutually exclusive", exclusion.one, exclusion.other);
        }
        if (one_matched == true && other_matched == true) {
            cli_fatalf_help("Options `%s` and `%s` are mutually exclusive. Please provide only one of them", exclusion.one, exclusion.other);
        }
    }
}

/**
 * @brief Checks for which command is being run. Adheres to the specification in @ref ARG_MAKE.
 * @param commands The zero-terminated array of @ref command_t
 * @param argc The length of argv
 * @param argv The argv array
 * @returns A number <= 1 representing the command which is being run
 */
size_t _run_command(command_t *commands, int argc, char *argv[]) {
    if (argc == 1) {
        return 1;
    }
    for (size_t i = 0; i < _cmd_len(commands); i++) {
        command_t cmd = commands[i];
        if (cli_streq(cmd.command, argv[1])) {
            return i + 2;
        }
    }
    return 1;
}

/**
 * @brief Parses an option of type --opt=arg.
 * @param options The zero-terminated array of @ref option_t
 * @param arg The string argument passed down to the cli
 * @param cmd_idx The index of the command
 */
void _cli_parse_equals(option_t *options, char *arg, uint64_t cmd_idx) {
    size_t total_len = strlen(arg);
    size_t pre_len = cli_stridx(arg, '=');
    size_t post_len = total_len - pre_len - 1;
    char *opt_str = (char *)malloc((pre_len + 1) * sizeof(char));
    cli_check_alloc(opt_str);
    memcpy(opt_str, arg, pre_len);
    opt_str[pre_len] = 0;
    char *param = (char *)malloc((post_len + 1) * sizeof(char));
    cli_check_alloc(param);
    memcpy(param, arg + pre_len + 1, post_len);
    param[post_len] = 0;
    size_t opt_count = _opt_len(options);
    bool matched_arg = false;
    for (size_t opt_search = 0; opt_search < opt_count; opt_search++) {
        option_t opt = options[opt_search];
        if (!(ARG_GLOBAL(opt.params)) && !(ARG_CMD(opt.params) == cmd_idx)) {
            continue;
        }
        if (_long_opt_eq(opt_str, opt.long_arg) || opt_str[1] == opt.short_arg) {
            matched_arg = true;
            options[opt_search].params = ARG_SET_MATCHED(opt.params);

            if (ARG_TYPE(opt.params) == boolean) {
                free(opt_str);
                cli_fatalf_help("Invalid flag usage. Option `%s` does not expect an argument", opt.long_arg);
            } else {
                if (ARG_TYPE(opt.params) == string) {
                    options[opt_search].data->str_data = param;
                } else if (ARG_TYPE(opt.params) == number) {
                    char *arg_num_param = param;
                    int64_t arg_num_parse_res = 0;

                    if (cli_try_parse_int(arg_num_param, &arg_num_parse_res)) {
                        options[opt_search].data->num_data = arg_num_parse_res;
                    } else {
                        free(opt_str);
                        cli_fatalf("Invalid numerical sequence for option `%s`: %s", opt.long_arg, arg_num_param);
                    }
                } else if (ARG_TYPE(opt.params) == unumber) {
                    char *arg_unum_param = param;
                    uint64_t arg_unum_parse_res = 0;

                    if (cli_try_parse_uint(arg_unum_param, &arg_unum_parse_res)) {
                        options[opt_search].data->num_data = arg_unum_parse_res;
                    } else {
                        free(opt_str);
                        cli_fatalf("Invalid numerical sequence for option `%s`: %s", opt.long_arg, arg_unum_param);
                    }
                } else {
                    cli_panic("Unrecognized type of flag encountered!");
                }
            }
        }
    }
    if (!matched_arg) {
        free(opt_str);
        cli_fatalf_help("Unknown argument `%s`", arg);
    }
    free(opt_str);
}

/**
 * @brief Parses the values in argv into the options defined in options. Fails automatically if an error during parsing is encountered. If successful all the @ref opt_data_t in the options contain the respective values.
 * @param commands All commands of the cli. Set to NULL if there are no commands else a zero-terminated array of @ref command_t
 * @param options All options of the cli as a zero-terminated array of @ref option_t
 * @param argc The argc value
 * @param argv The argv array
 * @param exclusions Optional zero-terminated array of @ref exclusion_t to respect
 * @param examples Optional zero-terminated array of examples
 * @return The name of the command invoked or NULL if the root command was invoked
 */
char *cli_parse_opts(command_t *commands, option_t *options, int argc, char *argv[], exclusion_t mutual_exclusions[], example_t examples[]) {
    _validate_options(options);
    int cmd_idx = _run_command(commands, argc, argv);
    char *command = cmd_idx > 1 ? commands[cmd_idx - 2].command : NULL;
    _find_help(commands, command, options, argc, argv, examples);
    size_t opt_count = _opt_len(options);
    for (int argc_idx = 1 + (cmd_idx > 1); argc_idx < argc; argc_idx++) {
        char *arg = argv[argc_idx];

        bool matched_arg = false;

        bool is_long = _is_long_opt(arg);
        short_opt_type_t short_opt = _short_opt_type(arg);
        bool is_positional = false;

        if (!is_long && short_opt == none) {
            is_positional = true;
        }

        if (cli_streq(arg, "--") || cli_streq(arg, "-")) {
            _cli_parse_remaining_positionals(options, commands, command, argc_idx, argc, argv);
            _check_mutual_exclusions(cmd_idx, options, mutual_exclusions);
            _check_unmatched(cmd_idx, options, mutual_exclusions);
            return cmd_idx == 1 ? NULL : commands[cmd_idx - 2].command;
        }

        if (cli_strcontains(arg, '=')) {
            _cli_parse_equals(options, arg, cmd_idx);
            continue;
        }

        for (size_t opt_search = 0; opt_search < opt_count; opt_search++) {
            option_t opt = options[opt_search];
            if (!(ARG_GLOBAL(opt.params)) && !(ARG_CMD(opt.params) == cmd_idx)) {
                continue;
            }
            if (is_positional && ARG_POSITIONAL(opt.params)) {
                matched_arg = true;
                options[opt_search].params = ARG_SET_MATCHED(opt.params);
                options[opt_search].data->str_data = arg;
            } else if ((is_long && _long_opt_eq(arg, opt.long_arg)) || (short_opt == single && arg[1] == opt.short_arg)) {
                matched_arg = true;
                options[opt_search].params = ARG_SET_MATCHED(opt.params);

                if (ARG_TYPE(opt.params) == boolean) {
                    options[opt_search].data->bool_data = true;
                } else {
                    if (argc_idx + 1 >= argc || _is_option(argv[argc_idx + 1])) {
                        if (ARG_TYPE(opt.params) == number && argc_idx + 1 < argc) {
                            char *arg_num_param_maybe = argv[argc_idx + 1];
                            int64_t arg_num_parse_maybe = 0;

                            if (cli_try_parse_int(arg_num_param_maybe, &arg_num_parse_maybe)) {
                                options[opt_search].data->num_data = arg_num_parse_maybe;
                                argc_idx++;
                            } else {
                                cli_fatalf_help("Missing argument: Option `%s` requires an argument but none was given", opt.long_arg);
                            }
                        } else if (ARG_TYPE(opt.params) == unumber && argc_idx + 1 < argc) {
                            cli_fatalf_help("Invalid unsigned numerical value for option `%s`: %s", opt.long_arg, argv[argc_idx + 1]);
                        } else {
                            cli_fatalf_help("Missing argument: Option `%s` requires an argument but none was given", opt.long_arg);
                        }
                    } else if (ARG_TYPE(opt.params) == string) {
                        options[opt_search].data->str_data = argv[++argc_idx];
                    } else if (ARG_TYPE(opt.params) == number) {
                        char *arg_num_param = argv[(++argc_idx)];
                        int64_t arg_num_parse_res = 0;

                        if (cli_try_parse_int(arg_num_param, &arg_num_parse_res)) {
                            options[opt_search].data->num_data = arg_num_parse_res;
                        } else {
                            cli_fatalf("Invalid numerical sequence for option `%s`: %s", opt.long_arg, arg_num_param);
                        }
                    } else if (ARG_TYPE(opt.params) == unumber) {
                        char *arg_unum_param = argv[(++argc_idx)];
                        uint64_t arg_unum_parse_res = 0;

                        if (cli_try_parse_uint(arg_unum_param, &arg_unum_parse_res)) {
                            options[opt_search].data->num_data = arg_unum_parse_res;
                        } else {
                            cli_fatalf(argv[0], "Invalid numerical sequence for option `%s`: %s", opt.long_arg, arg_unum_param);
                        }
                    } else {
                        cli_panic("Unrecognized type of flag encountered!");
                    }
                }
            } else if (!is_long && short_opt == multiple) {
                cli_fatal(argv[0], "Multiple shorthand options at once are not yet supported");
            }
        }

        if (!matched_arg) {
            cli_fatalf_help(argv[0], "Unknown argument `%s`", arg);
        }
    }

    _check_mutual_exclusions(cmd_idx, options, mutual_exclusions);
    _check_unmatched(cmd_idx, options, mutual_exclusions);
    return cmd_idx == 1 ? NULL : commands[cmd_idx - 2].command;
}
#endif
#endif
