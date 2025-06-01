# </> ccli - A simple header-only cli framework

## Example

```c
#define CCLI_IMPLEMENTATION       // Needed for the implementation
#define CCLI_STRIP_PREFIX         // Optional to remove prefixes. Redefinable macros are not stripped
#define CCLI_DISALLOW_HEX_NUMBERS // Disallow 0x4f format for numbers
#define CCLI_DISALLOW_BIN_NUMBERS // Disallow 0b001010 format for numbers
#include "ccli.h"

// Just declare your flags here. You can also construct the options array in main if you want
bool warn = false;
long timeout = 10;                     // Number expects a 64-bit (un)signed integer. You can redefine the types with `#define ccli_num int` for exaple
char iface[CCLI_MAX_STR_LEN] = "eth0"; // String expects a mutable buffer. You could also malloc the string. The max length is still limited by CCLI_MAX_STR_LEN (redefinable)
bool silent = false;

// Here you can declare the subcommands
// The array must be {0} terminated, this macro does that for you
commands(commands, {"run", "Run it"}, {"debug", "Debug it"});

// Define here your options. The first parameter is the name of the array
// The array must be {0} terminated, this macro does that for you
options(options,
        option_bool_var(warn, "Enable warnings", false, false, scope_global()),                       // No short option
        option_int_var_p(timeout, "Set timeout", "sec", false, true, scope_global()),                 // Auto generate short option from var name
        option_string_p("interface", iface, "Set interface", "name", false, false, scope_global()),   // Given long name with generated short from long
        option_bool_pc("quiet", 'q', silent, "Enable silent output", false, false, scope_subcmd(0))); // Given long and short name. Scope it to the 0th subcommand (run)

// Define here your examples. The first parameter are the flags added to the invocation and the second a short description.
// The array must be {0} terminated, this macro does that for you
examples(examples,
         {"-t 10 --interface wlo1", "Set the timeout to 10s and override the default interface with wlo1"},
         {"-t 10 --warn", "Set the timeout to 10s and enable warnings"});

int main(int argc, char **argv) {
    // This parses the arguments and writes the parsed values into the given pointers and returns the subcommand or null if none was provided
    // The function exits with an error message if something went wrong.
    // You can control the output stream using CCLI_STREAM 
    // commands and examples are optional
    const char *subcmd = parse_opts(commands, options, argc, argv, examples);

    printf("subcmd = %s\n", subcmd);
    printf("warn (%s)\n", warn ? "true" : "false");
    printf("timeout (%ld)\n", timeout);
    printf("interface (%s)\n", iface);
}
```
