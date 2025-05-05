// https://github.com/auribuo/ccli
//
// Copyright (c) 2025 Aurelio Buonomo
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef CCLI_H
#define CCLI_H

#ifndef CCLI_OK_STREAM
#define CCLI_OK_STREAM stdout
#endif // !CCLI_OK_STREAM

#ifndef CCLI_ERR_STREAM
#define CCLI_ERR_STREAM stderr
#endif // !CCLI_ERR_STREAM

// Binary representation: 0000000000000000
// Legend:                rpmccccccccttttt
// r = required
// p = positional
// m = matched
// c = command where 0 = global, 1 = root, n - 2 = index in commands
// t = type
#define CCLI_ARG_NULL 0
#define CCLI_ARG_REQ_MASK 32768 // 0b1000000000000000
#define CCLI_ARG_POS_MASK 16384 // 0b0100000000000000
#define CCLI_ARG_MAT_MASK 8192  // 0b0010000000000000
#define CCLI_ARG_TYP_MASK 31    // 0b0000000000011111
#define CCLI_ARG_CMD_MASK 8160  // 0b0001111111100000

#define ccli_params(typ, req, pos, cmd) (((req) << 15) | ((pos) << 14) | ((cmd) << 5) | (typ)) & (~CCLI_ARG_MAT_MASK)

#define ccli_option_type(opt) ((opt).params & CCLI_ARG_TYP_MASK)
#define ccli_option_subcmd(opt) (((opt).params & CCLI_ARG_CMD_MASK) >> 5)
#define ccli_option_subcmd_idx(opt) ((((opt).params & CCLI_ARG_CMD_MASK) >> 5) - 2)

#define ccli_option_is_global(opt) ((((opt).params & CCLI_ARG_CMD_MASK) >> 5) == 0)
#define ccli_option_is_root(opt) ((((opt).params & CCLI_ARG_CMD_MASK) >> 5) == 1)
#define ccli_option_is_required(opt) (((opt).params & CCLI_ARG_REQ_MASK) == CCLI_ARG_REQ_MASK)
#define ccli_option_is_positional(opt) (((opt).params & CCLI_ARG_POS_MASK) == CCLI_ARG_POS_MASK)
#define ccli_option_is_null(opt) ((opt).params == CCLI_ARG_NULL)
#define ccli_option_is_matched(opt) (((opt).params & CCLI_ARG_MAT_MASK) == CCLI_ARG_MAT_MASK)

#define ccli_option_set_matched(opt)       \
    do {                                   \
        (opt).params |= CCLI_ARG_MAT_MASK; \
    } while (0)

#define ccli_scope_global() 0
#define ccli_scope_root() 1
#define ccli_scope_subcmd(x) ((x) + 2)

#define ccli_options(var_name, ...) \
    ccli_option var_name[] = {__VA_ARGS__, {0}}

#define ccli_commands(var_name, ...) \
    ccli_command var_name[] = {__VA_ARGS__, {0}}

#define ccli_short_name_helper(name) (#name[0])
#define ccli_short_name(var) (ccli_short_name_helper(var))

#define ccli_option_bool_var_p(var, desc, req, pos, scope) \
    {ccli_short_name(var), #var, ccli_params(ccli_boolean, req, pos, scope), &var, desc, NULL}
#define ccli_option_bool_var_pc(short, var, desc, req, pos, scope) \
    {short, #var, ccli_params(ccli_boolean, req, pos, scope), &var, desc, NULL}
#define ccli_option_bool_var(var, desc, req, pos, scope) \
    {0, #var, ccli_params(ccli_boolean, req, pos, scope), &var, desc, NULL}

#define ccli_option_string_var_p(var, desc, var_desc, req, pos, scope) \
    {ccli_short_name(var), #var, ccli_params(ccli_string, req, pos, scope), &var, desc, var_desc}
#define ccli_option_string_var_pc(short, var, desc, var_desc, req, pos, scope) \
    {short, #var, ccli_params(ccli_string, req, pos, scope), &var, desc, var_desc}
#define ccli_option_string_var(var, desc, var_desc, req, pos, scope) \
    {0, #var, ccli_params(ccli_string, req, pos, scope), &var, desc, var_desc}

#define ccli_option_int_var_p(var, desc, var_desc, req, pos, scope) \
    {ccli_short_name(var), #var, ccli_params(ccli_number, req, pos, scope), &var, desc, var_desc}
#define ccli_option_int_var_pc(short, var, desc, var_desc, req, pos, scope) \
    {short, #var, ccli_params(ccli_number, req, pos, scope), &var, desc, var_desc}
#define ccli_option_int_var(var, desc, var_desc, req, pos, scope) \
    {0, #var, ccli_params(ccli_number, req, pos, scope), &var, desc, var_desc}

#define ccli_option_uint_var_p(var, desc, var_desc, req, pos, scope) \
    {ccli_short_name(var), #var, ccli_params(ccli_unumber, req, pos, scope), &var, desc, var_desc}
#define ccli_option_uint_var_pc(short, var, desc, var_desc, req, pos, scope) \
    {short, #var, ccli_params(ccli_unumber, req, pos, scope), &var, desc, var_desc}
#define ccli_option_uint_var(var, desc, var_desc, req, pos, scope) \
    {0, #var, ccli_params(ccli_unumber, req, pos, scope), &var, desc, var_desc}

#define ccli_options_cmd_is_null(opt) ((opt).command == NULL)

typedef struct {
    char short_arg;
    const char *long_arg;
    uint16_t params;
    void *data;
    const char *desc;
    const char *arg_desc;
} ccli_option;

typedef struct {
    const char *command;
    const char *desc;
} ccli_command;

typedef enum {
    ccli_boolean = 1,
    ccli_string = 2,
    ccli_number = 4,
    ccli_unumber = 8,
} ccli_option_kind;

typedef enum {
    ccli_short_none,
    ccli_short_single,
    ccli_short_multiple,
} ccli_short_opt_kind;

typedef struct {
    const char *options;
    const char *description;
} ccli_example;

_Noreturn void ccli_panic_loc(const char *file, int line, const char *msg);
_Noreturn void ccli_panicf_loc(const char *file, int line, const char *msg, ...);
#define ccli_panic(msg) ccli_panic_loc(__FILE__, __LINE__, msg)
#define ccli_panicf(msg, ...) ccli_panicf_loc(__FILE__, __LINE__, msg, __VA_ARGS__)

_Noreturn void ccli_fatal(const char *bin, const char *msg);
_Noreturn void ccli_fatalf(const char *bin, const char *format, ...);
_Noreturn void ccli_fatalf_help(const char *bin, const char *format, ...);

#define ccli_check_alloc(ptr)          \
    do {                               \
        if (ptr == NULL) {             \
            ccli_panic("Out of RAM."); \
        }                              \
    } while (0)

bool ccli_try_parse_int(const char *num, int64_t *data);
bool ccli_try_parse_uint(const char *num, uint64_t *data);
bool ccli_streq(const char *s1, const char *s2);
bool ccli_strcontains(const char *s, char c);
int64_t ccli_stridx(const char *s, char c);

void ccli_help(ccli_command *subcommands, const char *subcommand, ccli_option *options, char *argv[], ccli_example *examples);
const char *ccli_parse_opts(ccli_command *subcommands, ccli_option *options, int argc, char *argv[], ccli_example examples[]);

size_t ccli__opt_len(ccli_option *options);
size_t ccli__cmd_len(ccli_command *commands);
void ccli__validate_options(ccli_option *options);
bool ccli__arg_relevant(ccli_option opt, ccli_command *subcommands, const char *subcommand);
size_t ccli__max_long_arg_len(ccli_option *options, ccli_command *subcommands, const char *subcommand);
size_t ccli__pos_args_len(ccli_option *options, ccli_command *subcommands, const char *subcommand);
bool ccli__is_long_opt(char *opt);
ccli_short_opt_kind ccli__short_opt_kind(char *opt);
bool ccli__is_option(char *opt);
void ccli__parse_remaining_positionals(ccli_option *options, ccli_command *subcommands, const char *subcommand, int argc_idx, int argc, char **argv);
bool ccli__long_opt_eq(const char *argv_opt, const char *long_opt);
void ccli__check_unmatched(const char *bin, uint8_t cmd_idx, ccli_option *options);
void ccli__find_help(ccli_command *subcommands, const char *subcommand, ccli_option *options, int argc, char *argv[], ccli_example *examples);
size_t ccli__run_command(ccli_command *subcommands, int argc, char *argv[]);
void ccli__parse_equals(const char *bin, ccli_option *options, char *arg, uint64_t cmd_idx);

const ccli_option help_opt = {'h', "help", ccli_params(ccli_boolean, false, false, ccli_scope_global()), NULL, "Show this help menu", NULL};

#ifdef CCLI_IMPLEMENTATION
#include <errno.h>
#include <limits.h>
#include <stdarg.h>

_Noreturn void ccli_panic_loc(const char *file, int line, const char *msg) {
    if (msg != NULL) {
        fprintf(CCLI_ERR_STREAM, "%s:%d: cli_panic: %s\n", file, line, msg);
    } else {
        fprintf(CCLI_ERR_STREAM, "%s:%d: cli_panic: Program cli_paniced.", file, line);
    }
    exit(1);
}

_Noreturn void ccli_panicf_loc(const char *file, int line, const char *msg, ...) {
    va_list argptr;
    va_start(argptr, msg);

    if (msg != NULL) {
        fprintf(CCLI_ERR_STREAM, "%s:%d: cli_panic: ", file, line);
        vfprintf(CCLI_ERR_STREAM, msg, argptr);
        fputc('\n', CCLI_ERR_STREAM);
    }
    va_end(argptr);
    exit(1);
}

_Noreturn void ccli_fatal(const char *bin, const char *msg) {
    if (msg != NULL) {
        fprintf(CCLI_ERR_STREAM, "%s: %s\n", bin, msg);
    }
    exit(1);
}

_Noreturn void ccli_fatalf(const char *bin, const char *format, ...) {
    va_list argptr;
    va_start(argptr, format);

    if (format != NULL) {
        fprintf(CCLI_ERR_STREAM, "%s: ", bin);
        vfprintf(CCLI_ERR_STREAM, format, argptr);
        fputc('\n', CCLI_ERR_STREAM);
    }
    va_end(argptr);
    exit(1);
}

_Noreturn void ccli_fatalf_help(const char *bin, const char *format, ...) {
    va_list argptr;
    va_start(argptr, format);

    if (format != NULL) {
        fprintf(CCLI_ERR_STREAM, "%s: ", bin);
        vfprintf(CCLI_ERR_STREAM, format, argptr);
        fprintf(CCLI_ERR_STREAM, ". For more information see %s --help\n", bin);
    }
    va_end(argptr);
    exit(1);
}
size_t ccli__opt_len(ccli_option *options) {
    if (options == NULL) {
        return 0;
    }
    size_t idx = 0;
    ccli_option opt;
    while (!ccli_option_is_null(opt = options[idx++]))
        ;
    return idx - 1;
}

size_t ccli__cmd_len(ccli_command *commands) {
    if (commands == NULL) {
        return 0;
    }
    size_t idx = 0;
    ccli_command cmd;
    while (!ccli_options_cmd_is_null(cmd = commands[idx++]))
        ;
    return idx - 1;
}

bool ccli_try_parse_int(const char *num, int64_t *data) {
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

bool ccli_try_parse_uint(const char *num, uint64_t *data) {
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

bool ccli_streq(const char *s1, const char *s2) {
    if (s1 == NULL || s2 == NULL) {
        return s1 == s2;
    }
    if (s1 == s2) {
        return true;
    }
    return strcmp(s1, s2) == 0;
}

bool ccli_strcontains(const char *s, char c) {
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

int64_t ccli_stridx(const char *s, char c) {
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

void ccli__validate_options(ccli_option *options) {
    for (size_t i = 0; i < ccli__opt_len(options); i++) {
        ccli_option opt = options[i];
        if (opt.long_arg == NULL) {
            ccli_panicf("Invalid option at index %lu. Long option is always required!", i);
        }
        if (ccli_option_type(opt) != ccli_boolean && !(ccli_option_is_positional(opt)) && opt.arg_desc == NULL) {
            ccli_panicf("Invalid option %s. If option is not boolean arg_desc is required!", opt.long_arg);
        }
    }
}

bool ccli__arg_relevant(ccli_option opt, ccli_command *subcommands, const char *subcommand) {
    if (ccli_option_is_global(opt)) {
        return true;
    } else if (ccli_option_is_root(opt)) {
        return subcommand == NULL;
    }
    return ccli_streq(subcommands[ccli_option_subcmd_idx(opt)].command, subcommand);
}

size_t ccli__max_long_arg_len(ccli_option *options, ccli_command *subcommands, const char *subcommand) {
    size_t max = 4; // Hardcoded to the word "help"
    size_t len = ccli__opt_len(options);

    for (size_t i = 0; i < len; i++) {
        ccli_option opt = options[i];
        if (!ccli__arg_relevant(opt, subcommands, subcommand)) {
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

size_t ccli__pos_args_len(ccli_option *options, ccli_command *subcommands, const char *subcommand) {
    size_t count = 0;
    size_t len = ccli__opt_len(options);

    for (size_t i = 0; i < len; i++) {
        if (ccli_option_is_positional(options[i]) && ccli__arg_relevant(options[i], subcommands, subcommand)) {
            count++;
        }
    }
    return count;
}

void ccli_help(ccli_command *subcommands, const char *subcommand, ccli_option *options, char *argv[], ccli_example *examples) {
    uint32_t max_len = ccli__max_long_arg_len(options, subcommands, subcommand);
    size_t num_options = ccli__opt_len(options);
    size_t num_commands = ccli__cmd_len(subcommands);
    fprintf(CCLI_OK_STREAM, "Usage: \n");
    if (num_commands > 0) {
        if (subcommand == NULL) {
            fprintf(CCLI_OK_STREAM, "\t%s [command]\n", argv[0]);
        }
    }
    fprintf(CCLI_OK_STREAM, "\t%s ", argv[0]);
    if (num_commands > 0 && subcommand != NULL) {
        fprintf(CCLI_OK_STREAM, "%s ", subcommand);
    }
    fprintf(CCLI_OK_STREAM, "[options] ");
    for (size_t i = 0; i < num_options; i++) {
        ccli_option opt = options[i];
        if (ccli_option_is_positional(opt) && ccli__arg_relevant(opt, subcommands, subcommand)) {
            fprintf(CCLI_OK_STREAM, "%s ", opt.long_arg);
        }
    }
    if (num_commands > 0 && subcommand == NULL) {
        fprintf(CCLI_OK_STREAM, "\n\nAvailable commands:\n");
        for (size_t i = 0; i < num_commands; i++) {
            ccli_command cmd = subcommands[i];
            char *padded_cmd = (char *)malloc(sizeof(char) * max_len + 1);
            ccli_check_alloc(padded_cmd);
            memset(padded_cmd, ' ', max_len);
            memcpy(padded_cmd, cmd.command, strlen(cmd.command));
            padded_cmd[max_len] = 0;
            fprintf(CCLI_OK_STREAM, "\t%s      %s\n", padded_cmd, cmd.desc);

            free(padded_cmd);
        }
    } else {
        fputc('\n', CCLI_OK_STREAM);
    }
    fprintf(CCLI_OK_STREAM, "\nAvailable options:\n");
    for (size_t i = 0; i < num_options; i++) {
        ccli_option opt = options[i];
        if (ccli_option_is_positional(opt) || !ccli__arg_relevant(opt, subcommands, subcommand)) {
            continue;
        }
        char *padded_long;
        if (opt.long_arg != NULL) {
            padded_long = (char *)malloc(sizeof(char) * max_len + 1);
            ccli_check_alloc(padded_long);
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
            ccli_check_alloc(padded_long);
            memset(padded_long, ' ', max_len + 3);
        }

        if (opt.short_arg == 0) {
            fprintf(CCLI_OK_STREAM, "\t  ");
        } else {
            fprintf(CCLI_OK_STREAM, "\t-%c", opt.short_arg);
        }
        fprintf(CCLI_OK_STREAM, " --%s %s\n", padded_long, opt.desc);

        free(padded_long);
    }

    char *padded_long;
    padded_long = (char *)malloc(sizeof(char) * max_len + 1);
    ccli_check_alloc(padded_long);
    memset(padded_long, ' ', max_len);
    memcpy(padded_long, help_opt.long_arg, strlen(help_opt.long_arg));
    padded_long[max_len] = 0;

    fprintf(CCLI_OK_STREAM, "\t-%c", help_opt.short_arg);
    fprintf(CCLI_OK_STREAM, " --%s %s\n", padded_long, help_opt.desc);

    free(padded_long);

    if (ccli__pos_args_len(options, subcommands, subcommand) > 0) {
        fprintf(CCLI_OK_STREAM, "\nPositional options:\n");
        for (size_t i = 0; i < num_options; i++) {
            ccli_option opt = options[i];
            if (!(ccli_option_is_positional(opt)) || !ccli__arg_relevant(opt, subcommands, subcommand)) {
                continue;
            }
            char *padded_long;
            padded_long = (char *)malloc(sizeof(char) * max_len + 1);
            ccli_check_alloc(padded_long);
            memset(padded_long, ' ', max_len);
            memcpy(padded_long, opt.long_arg, strlen(opt.long_arg));
            padded_long[max_len] = 0;

            fprintf(CCLI_OK_STREAM, "\t%s      %s\n", padded_long, opt.desc);

            free(padded_long);
        }
    }

    if (examples != NULL) {
        fprintf(CCLI_OK_STREAM, "\nExamples:\n");
        ccli_example example;
        size_t idx = 0;
        while ((example = examples[idx++]).options != NULL) {
            fprintf(CCLI_OK_STREAM, "%s %s\t%s\n", argv[0], example.options, example.description);
        }
    }

    fprintf(CCLI_OK_STREAM, "\n\nUse `%s [command] --help` to get help for a specific command\n", argv[0]);
}

bool ccli__is_long_opt(char *opt) {
    size_t len = strlen(opt);
    if (len < 3) {
        return false;
    }
    return opt[0] == '-' && opt[0] == opt[1];
}

ccli_short_opt_kind ccli__short_opt_kind(char *opt) {
    size_t len = strlen(opt);
    if (len < 2) {
        return ccli_short_none;
    }

    if (opt[0] != '-') {
        return ccli_short_none;
    }

    if (len >= 3) {
        return ccli_short_multiple;
    }

    return opt[0] == '-' && opt[1] != '-' ? ccli_short_single : ccli_short_none;
}

bool ccli__is_option(char *opt) {
    return ccli__is_long_opt(opt) || ccli__short_opt_kind(opt) != ccli_short_none;
}

void ccli__parse_remaining_positionals(ccli_option *options, ccli_command *subcommands, const char *subcommand, int argc_idx, int argc, char **argv) {
    size_t pos_arg_count = ccli__pos_args_len(options, subcommands, subcommand);
    if ((size_t)(argc - argc_idx - 1) > pos_arg_count) {
        ccli_fatalf_help(argv[0], "Too many positional arguments: Expected %d got %d", pos_arg_count, argc - argc_idx - 1);
    }

    for (; argc_idx < argc; argc_idx++) {
        char *arg = argv[argc_idx];
        if (strcmp(arg, "--") == 0 || strcmp(arg, "-") == 0) {
            continue;
        }
        size_t len = ccli__opt_len(options);
        for (size_t opt_search = 0; opt_search < len; opt_search++) {
            ccli_option opt = options[opt_search];
            if (!(ccli_option_is_matched(opt)) && ccli_option_is_positional(opt)) {
                ccli_option_set_matched(options[opt_search]);
                strcpy(options[opt_search].data, arg);
            }
        }
    }

    if (argc_idx != argc) {
        ccli_fatalf_help(argv[0], "Too few positional arguments: Expected %d got %d", pos_arg_count, argc - argc_idx);
    }
}

bool ccli__long_opt_eq(const char *argv_opt, const char *long_opt) {
    if (argv_opt == NULL || long_opt == NULL) {
        return false;
    }
    char *long_opt_with_dash = (char *)alloca(sizeof(char) * strlen(long_opt) + 3);
    ccli_check_alloc(long_opt_with_dash);
    memcpy(long_opt_with_dash + 2, long_opt, strlen(long_opt) + 1);
    long_opt_with_dash[0] = long_opt_with_dash[1] = '-';
    bool are_eq = strcmp(argv_opt, long_opt_with_dash) == 0;
    return are_eq;
}

void ccli__check_unmatched(const char *bin, uint8_t cmd_idx, ccli_option *options) {
    size_t len = ccli__opt_len(options);
    for (size_t opt_search = 0; opt_search < len; opt_search++) {
        ccli_option opt = options[opt_search];
        if (!(ccli_option_is_global(opt)) && ccli_option_subcmd(opt) != cmd_idx) {
            continue;
        }
        if (!(ccli_option_is_matched(opt))) {
            if (ccli_option_is_required(opt)) {
                ccli_fatalf_help(bin, "Missing required argument `%s`", opt.long_arg);
            }
        }
    }
}

void ccli__find_help(ccli_command *subcommands, const char *subcommand, ccli_option *options, int argc, char *argv[], ccli_example *examples) {
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        if (strcmp(arg, "--") == 0 || strcmp(arg, "-") == 0) {
            return;
        }
        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            ccli_help(subcommands, subcommand, options, argv, examples);
            exit(0);
        }
    }
}

size_t ccli__run_command(ccli_command *subcommands, int argc, char *argv[]) {
    if (argc == 1) {
        return 1;
    }
    size_t len = ccli__cmd_len(subcommands);
    for (size_t i = 0; i < len; i++) {
        ccli_command cmd = subcommands[i];
        if (ccli_streq(cmd.command, argv[1])) {
            return i + 2;
        }
    }
    return 1;
}

void ccli__parse_equals(const char *bin, ccli_option *options, char *arg, uint64_t cmd_idx) {
    size_t total_len = strlen(arg);
    size_t pre_len = ccli_stridx(arg, '=');
    size_t post_len = total_len - pre_len - 1;
    char *opt_str = (char *)malloc((pre_len + 1) * sizeof(char));
    ccli_check_alloc(opt_str);
    memcpy(opt_str, arg, pre_len);
    opt_str[pre_len] = 0;
    char *param = (char *)malloc((post_len + 1) * sizeof(char));
    ccli_check_alloc(param);
    memcpy(param, arg + pre_len + 1, post_len);
    param[post_len] = 0;
    size_t opt_count = ccli__opt_len(options);
    bool matched_arg = false;
    for (size_t opt_search = 0; opt_search < opt_count; opt_search++) {
        ccli_option opt = options[opt_search];
        if (!(ccli_option_is_global(opt)) && !(ccli_option_subcmd(opt) == cmd_idx)) {
            continue;
        }
        if (ccli__long_opt_eq(opt_str, opt.long_arg) || opt_str[1] == opt.short_arg) {
            matched_arg = true;
            ccli_option_set_matched(options[opt_search]);

            if (ccli_option_type(opt) == ccli_boolean) {
                free(opt_str);
                ccli_fatalf_help(bin, "Invalid flag usage. Option `%s` does not expect an argument", opt.long_arg);
            } else {
                if (ccli_option_type(opt) == ccli_string) {
                    strcpy(options[opt_search].data, param);
                } else if (ccli_option_type(opt) == ccli_number) {
                    char *arg_num_param = param;
                    int64_t arg_num_parse_res = 0;

                    if (ccli_try_parse_int(arg_num_param, &arg_num_parse_res)) {
                        *((int64_t *)options[opt_search].data) = arg_num_parse_res;
                    } else {
                        free(opt_str);
                        ccli_fatalf(bin, "Invalid numerical sequence for option `%s`: %s", opt.long_arg, arg_num_param);
                    }
                } else if (ccli_option_type(opt) == ccli_unumber) {
                    char *arg_unum_param = param;
                    uint64_t arg_unum_parse_res = 0;

                    if (ccli_try_parse_uint(arg_unum_param, &arg_unum_parse_res)) {
                        *((uint64_t *)options[opt_search].data) = arg_unum_parse_res;
                    } else {
                        free(opt_str);
                        ccli_fatalf(bin, "Invalid numerical sequence for option `%s`: %s", opt.long_arg, arg_unum_param);
                    }
                } else {
                    ccli_panic("Unrecognized type of flag encountered!");
                }
            }
        }
    }
    if (!matched_arg) {
        free(opt_str);
        ccli_fatalf_help(bin, "Unknown argument `%s`", arg);
    }
    free(opt_str);
}

const char *ccli_parse_opts(ccli_command *subcommands, ccli_option *options, int argc, char *argv[], ccli_example examples[]) {
    if (argc == 0 || argv == NULL) {
        ccli_panic("argc and argv are required");
    }

    const char *bin = argv[0];
    ccli__validate_options(options);
    int cmd_idx = ccli__run_command(subcommands, argc, argv);
    const char *subcommand = cmd_idx > 1 ? subcommands[cmd_idx - 2].command : NULL;
    ccli__find_help(subcommands, subcommand, options, argc, argv, examples);
    size_t opt_count = ccli__opt_len(options);
    for (int argc_idx = 1 + (cmd_idx > 1); argc_idx < argc; argc_idx++) {
        char *arg = argv[argc_idx];

        bool matched_arg = false;

        bool is_long = ccli__is_long_opt(arg);
        ccli_short_opt_kind short_opt = ccli__short_opt_kind(arg);
        bool is_positional = false;

        if (!is_long && short_opt == ccli_short_none) {
            is_positional = true;
        }

        if (ccli_streq(arg, "--") || ccli_streq(arg, "-")) {
            ccli__parse_remaining_positionals(options, subcommands, subcommand, argc_idx, argc, argv);
            ccli__check_unmatched(bin, cmd_idx, options);
            return cmd_idx == 1 ? NULL : subcommands[cmd_idx - 2].command;
        }

        if (ccli_strcontains(arg, '=')) {
            ccli__parse_equals(bin, options, arg, cmd_idx);
            continue;
        }

        for (size_t opt_search = 0; opt_search < opt_count; opt_search++) {
            ccli_option opt = options[opt_search];
            if (!(ccli_option_is_global(opt)) && !(ccli_option_subcmd(opt) == cmd_idx)) {
                continue;
            }
            if (is_positional && ccli_option_is_positional(opt)) {
                matched_arg = true;
                ccli_option_set_matched(options[opt_search]);
                strcpy(options[opt_search].data, arg);
            } else if ((is_long && ccli__long_opt_eq(arg, opt.long_arg)) || (short_opt == ccli_short_single && arg[1] == opt.short_arg)) {
                matched_arg = true;
                ccli_option_set_matched(options[opt_search]);

                if (ccli_option_type(opt) == ccli_boolean) {
                    *((bool *)options[opt_search].data) = true;
                } else {
                    if (argc_idx + 1 >= argc || ccli__is_option(argv[argc_idx + 1])) {
                        if (ccli_option_type(opt) == ccli_number && argc_idx + 1 < argc) {
                            char *arg_num_param_maybe = argv[argc_idx + 1];
                            int64_t arg_num_parse_maybe = 0;

                            if (ccli_try_parse_int(arg_num_param_maybe, &arg_num_parse_maybe)) {
                                *((int64_t *)options[opt_search].data) = arg_num_parse_maybe;
                                argc_idx++;
                            } else {
                                ccli_fatalf_help(bin, "Missing argument: Option `%s` requires an argument but none was given", opt.long_arg);
                            }
                        } else if (ccli_option_type(opt) == ccli_unumber && argc_idx + 1 < argc) {
                            ccli_fatalf_help(bin, "Invalid unsigned numerical value for option `%s`: %s", opt.long_arg, argv[argc_idx + 1]);
                        } else {
                            ccli_fatalf_help(bin, "Missing argument: Option `%s` requires an argument but none was given", opt.long_arg);
                        }
                    } else if (ccli_option_type(opt) == ccli_string) {
                        strcpy(options[opt_search].data, argv[++argc_idx]);
                    } else if (ccli_option_type(opt) == ccli_number) {
                        char *arg_num_param = argv[(++argc_idx)];
                        int64_t arg_num_parse_res = 0;

                        if (ccli_try_parse_int(arg_num_param, &arg_num_parse_res)) {
                            *((int64_t *)options[opt_search].data) = arg_num_parse_res;
                        } else {
                            ccli_fatalf(bin, "Invalid numerical sequence for option `%s`: %s", opt.long_arg, arg_num_param);
                        }
                    } else if (ccli_option_type(opt) == ccli_unumber) {
                        char *arg_unum_param = argv[(++argc_idx)];
                        uint64_t arg_unum_parse_res = 0;

                        if (ccli_try_parse_uint(arg_unum_param, &arg_unum_parse_res)) {
                            *((uint64_t *)options[opt_search].data) = arg_unum_parse_res;
                        } else {
                            ccli_fatalf(bin, "Invalid numerical sequence for option `%s`: %s", opt.long_arg, arg_unum_param);
                        }
                    } else {
                        ccli_panic("Unrecognized type of flag encountered!");
                    }
                }
            } else if (!is_long && short_opt == ccli_short_multiple) {
                ccli_fatal(bin, "Multiple shorthand options at once are not yet supported"); // TODO
            }
        }

        if (!matched_arg) {
            ccli_fatalf_help(bin, "Unknown argument `%s`", arg);
        }
    }

    ccli__check_unmatched(bin, cmd_idx, options);
    return cmd_idx == 1 ? NULL : subcommands[cmd_idx - 2].command;
}
#ifndef _CCLI_STRIP_PREFIX_GUARD
#define _CCLI_STRIP_PREFIX_GUARD
#ifdef CCLI_STRIP_PREFIX
// TODO: prefix stripping
#endif // CCLI_STRIP_PREFIX
#endif // !_CCLI_STRIP_PREFIX_GUARD
#endif // CCLI_IMPLEMENTATION
#endif // CCLI_H
