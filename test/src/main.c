#include "test.h"
#include "tools/memcheck.h"
#include <stdlib.h>
#include <string.h>

typedef struct test_config
{
    int flag_no_memcheck;
}test_config_t;

test_config_t g_test_config;

static void _before_all_test(int argc, char* argv[])
{
    (void)argc; (void)argv;

    memset(&g_test_config, 0, sizeof(g_test_config));

    int i;
    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "--no_memcheck") == 0)
        {
            g_test_config.flag_no_memcheck = 1;
        }
    }

    if (!g_test_config.flag_no_memcheck)
    {
        setup_memcheck();
    }
}

static void _after_all_test(void)
{
    fflush(NULL);

    if (!g_test_config.flag_no_memcheck)
    {
        dump_memcheck();
    }
}

static cutest_hook_t test_hook = {
    _before_all_test,   /* .before_all_test */
    _after_all_test,    /* .after_all_test */
    NULL,               /* .before_fixture_setup */
    NULL,               /* .after_fixture_setup */
    NULL,               /* .before_fixture_teardown */
    NULL,               /* .after_fixture_teardown */
    NULL,               /* .before_fixture_test */
    NULL,               /* .after_fixture_test */
    NULL,               /* .before_parameterized_test */
    NULL,               /* .after_parameterized_test */
    NULL,               /* .before_simple_test */
    NULL,               /* .after_simple_test */
};

int main(int argc, char* argv[])
{
    return cutest_run_tests(argc, argv, &test_hook);
}
