#include "test.h"
#include "utils/file.h"
#include <string.h>

struct test_process
{
    ev_os_pid_t pid;
};

struct test_process g_test_process;

TEST_FIXTURE_SETUP(process)
{
    memset(&g_test_process, 0, sizeof(g_test_process));
    g_test_process.pid = EV_OS_PID_INVALID;
}

TEST_FIXTURE_TEAREDOWN(process)
{
    if (g_test_process.pid != EV_OS_PID_INVALID)
    {
        ASSERT_EQ_D32(ev_waitpid(g_test_process.pid, EV_INFINITE_TIMEOUT), EV_SUCCESS);
        g_test_process.pid = EV_OS_PID_INVALID;
    }
}

TEST_F(process, exec)
{
    int ret;

    const char* path = test_get_self_exe();
    ASSERT_NE_PTR(path, NULL);

    char* argv[] = { (char*)path, "--help", NULL };

    ev_exec_opt_t opt;
    memset(&opt, 0, sizeof(opt));
    opt.argv = argv;

    ret = ev_exec(&g_test_process.pid, &opt);
    ASSERT_EQ_D32(ret, EV_SUCCESS);
}
