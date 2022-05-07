#include "test.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
    return cutest_run_tests(argc, argv, &test_hook);
}
