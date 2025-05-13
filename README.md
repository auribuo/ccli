# </> ccli - A simple header-only cli framework

## Usage

```c
#define CCLI_IMPLEMENTATION // Needed for the implementation
#define CCLI_STRIP_PREFIX   // Optional to remove prefixes
#include "ccli.h"

// Just declare your flags here. You can also construct the options array in main if you want
bool warn = false;
long timeout = 0;
char iface[CCLI_MAX_STR_LEN] = "eth0";

bool silent = false;

// Here you can declare the subcommands
// The array must be {0} terminated, this macro does that for you
commands(commands, {"run", "Run it"});

// Define here you options. The first parameter is the name of the array
// The array must be {0} terminated, this macro does that for you
options(options,
        option_bool_var(warn, "Enable warnings", false, false, scope_global()),
        option_int_var_p(timeout, "Set timeout", "sec", true, false, scope_global()),
        option_string_var_p(iface, "Set interface", "name", false, false, scope_global()),
        option_bool_var_pc('q', silent, "Enable silent output", false, false, scope_subcmd(0)));

examples(examples,
         {"-t 10 --interface wlo1", "Set the timeout to 10s and override the default interface with wlo1"},
         {"-t 10 --warn", "Set the timeout to 10s and enable warnings"});

int main(int argc, char **argv) {
    // This parses the arguments and writes the parsed values into the given pointers and returns the subcommand or null if none was provided
    // The function exits with an error message if something went wrong.
    // You can control the output streams using CCLI_OK_STREAM for the help menu and CCLI_ERR_STREAM for all the error messages
    // If the first argument is not a flag it will be treated as a subcommand. This will be returned by parse_opts
    const char *subcmd = parse_opts(commands, options, argc, argv, examples);

    printf("subcmd = %s\n", subcmd);
    printf("warn (%s)\n", warn ? "true" : "false");
    printf("timeout (%ld)\n", timeout);
    printf("interface (%s)\n", iface);
}
```

## License

The project is licenced under the MIT license.

See [LICENSE](./LICENSE)

## Contributing

If you want to contribute to this project open an issue.
