#include "config.h"
#include "tools/memcheck.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

        opt = "--";
        if (strcmp(argv[i], opt) == 0)
        {
            if (i == argc - 1)
            {
                fprintf(stderr, "missing argument after `--`, use `-- help` (pay attention to the space) to see what tools builtin.\n");
                exit(EXIT_FAILURE);
            }

            test_config.argct = argc - i - 1;
            test_config.argvt = argv + i + 1;
            return;
        }
    }
}

void test_config_cleanup(void)
{
}
