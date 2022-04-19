#include "config.h"
#include <string.h>

test_config_t test_config;

void test_config_setup(int argc, char* argv[])
{
    int i;
    const char* opt;
    memset(&test_config, 0, sizeof(test_config));

    test_config.argc = argc;
    test_config.argv = argv;

    for (i = 0; i < argc; i++)
    {
        opt = "--no_memcheck";
        if (strcmp(argv[i], opt) == 0)
        {
            test_config.flag_no_memcheck=1;
            continue;
        }

        opt = "--stdio_echo_server";
        if (strcmp(argv[i], opt) == 0)
        {
            test_config.flag_as_stdio_echo_server = 1;
            continue;
        }
    }
}

void test_config_cleanup(void)
{

}
