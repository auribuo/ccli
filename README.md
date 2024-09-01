# </> ccli - A simple header-only cli framework

## Usage

To use the header just include it where you want to use it.

By default only the struct and macro definitions are exposed.
To include the argument parser define the `CCLI_IMPLEMENTATION` macro
before the include:

```c
#define CCLI_IMPLEMENTATION
#include "cli.h"
```

Then create you can define your options in the following way:

```c
static command_t commands[] = {
    {"test", "Run the test command"},
    {"version", "Display the version of the app"},
    {0}
};

static opt_data_t echo_data;
static opt_data_t stderr_data = {.bool_data = false};

static option_t options[] = {
    {0, "stderr", ARG_MAKE_GLOBAL(boolean, 0, 0), &stderr_data, "Print to stderr instead of stdout", NULL},
    {'e', "echo", ARG_MAKE_CMD(string, 1, 0, 0), &echo_data, "Echo the value given to this flag", "message"},
    {0}
};
```

The array `commands` hold all commands of the application.
The unions of type `opt_data_t` hold the values
the user passes to all the later defined flags.
You can set default values by assigning a value to the correct field in the union.

Then you are ready to define all the options. The `options` array holds all the options
across all the commands in the application.

An option is a struct holding the following information, in order:
- The short form of the flag
- The long form and name of the flag. Required
- The parameters of the flag. Check out the documentation of the macros
- A pointer to the union to save the data to
- An optional description of the flag.
- The name of the argument to a string|number|unumber flag. Required or `NULL` if the flag is `boolean`

Once the options are defined you can call the `cli_parse_opts` funtion in your program.
The function returns the string value of the command that has been called or `NULL` if no command was invoked.

## Documentation

Currently the only available documentation is the code itself.

All of the function in the header are documented.

More in-depth docuementation might come in the future.

## Api changes

The project is fairly new. It started as a part of an university project
of mine. I decied to extrapolate the code and make it available on GitHub.

I will change some of the apis to make working with the library easier
and to avoid potential naming conflicts.

## License

The project is licenced under the MIT license.

See [LICENSE](./LICENSE)

## Contributing

If you want to contribute to this project open an issue.
