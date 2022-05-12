#include "test.h"
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int main(int argc, char* argv[])
{
#if defined(SIGPIPE)
    signal(SIGPIPE, SIG_IGN);
#endif
    return cutest_run_tests(argc, argv, &test_hook);
}
