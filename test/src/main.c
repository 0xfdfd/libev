#include "cutest.h"
#include "tools/memcheck.h"

static void _before_all_test(int argc, char* argv[])
{
    (void)argc; (void)argv;
    setup_memcheck();
}

static void _after_all_test(void)
{
    dump_memcheck();
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
