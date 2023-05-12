#include "test.h"
#include "type/__init__.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

static void _at_exit(void)
{
    ev_library_shutdown();

    /* ensure all output was done. */
    fflush(NULL);

    mmc_exit();
}

int main(int argc, char* argv[])
{
#if defined(SIGPIPE)
    signal(SIGPIPE, SIG_IGN);
#endif

    atexit(_at_exit);
    register_types();

    return cutest_run_tests(argc, argv, stdout, &test_hook);
}
