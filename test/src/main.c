#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

static void _at_exit(void)
{
    /* ensure all output was done. */
    fflush(NULL);
}

int main(int argc, char* argv[])
{
#if defined(SIGPIPE)
    signal(SIGPIPE, SIG_IGN);
#endif
    atexit(_at_exit);
    return cutest_run_tests(argc, argv, &test_hook);
}
