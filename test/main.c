#include "test.h"
#include <signal.h>

int main(int argc, char* argv[])
{
#if defined(SIGPIPE)
    signal(SIGPIPE, SIG_IGN);
#endif

    return cutest_run_tests(argc, argv, stdout, &test_hook);
}
