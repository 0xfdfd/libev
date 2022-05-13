#include "test.h"

struct test_process
{
    char*           self_exe_path;  /**< Full path to self */
    ev_process_t    process;        /**< Process handle */
    ev_loop_t       loop;           /**< Event loop */

    ev_pipe_t       stdin_pipe;     /**< STDIN pipe for child process */
    ev_pipe_t       stdout_pipe;    /**< STDOUT pipe for child process */
    ev_pipe_t       stderr_pipe;    /**< STDERR pipe for child process */

    int             flag_stdin_init;
};

struct test_process g_test_process;
static const char* g_test_process_data = "abcdefghijklmnopqrstuvwxyz1234567890";

static void _close_stdin_pipe(void)
{
    if (g_test_process.flag_stdin_init)
    {
        g_test_process.flag_stdin_init = 0;
        ev_pipe_exit(&g_test_process.stdin_pipe, NULL);
    }
}

TEST_FIXTURE_SETUP(process)
{
    memset(&g_test_process, 0, sizeof(g_test_process));

    g_test_process.self_exe_path = strdup(test_get_self_exe());
    ASSERT_NE_PTR(g_test_process.self_exe_path, NULL);

    ASSERT_EQ_D32(ev_loop_init(&g_test_process.loop), EV_SUCCESS);
    ASSERT_EQ_D32(ev_pipe_init(&g_test_process.loop, &g_test_process.stdin_pipe, 0), EV_SUCCESS);
    ASSERT_EQ_D32(ev_pipe_init(&g_test_process.loop, &g_test_process.stdout_pipe, 0), EV_SUCCESS);
    ASSERT_EQ_D32(ev_pipe_init(&g_test_process.loop, &g_test_process.stderr_pipe, 0), EV_SUCCESS);

    g_test_process.flag_stdin_init = 1;
}

TEST_FIXTURE_TEAREDOWN(process)
{
    _close_stdin_pipe();
    ev_pipe_exit(&g_test_process.stdout_pipe, NULL);
    ev_pipe_exit(&g_test_process.stderr_pipe, NULL);
    ASSERT_EQ_D32(ev_loop_run(&g_test_process.loop, EV_LOOP_MODE_DEFAULT), 0);

    ASSERT_EQ_D32(ev_loop_exit(&g_test_process.loop), EV_SUCCESS);

    free(g_test_process.self_exe_path);
    g_test_process.self_exe_path = NULL;
}

TEST_F(process, spawn_stdout_ignore)
{
    int ret;

    char* argv[] = { g_test_process.self_exe_path, "--help", NULL };

    ev_process_options_t opt;
    memset(&opt, 0, sizeof(opt));
    opt.argv = argv;
    opt.stdios[1].flag = EV_PROCESS_STDIO_REDIRECT_NULL;

    ret = ev_process_spawn(&g_test_process.loop, &g_test_process.process, &opt);
    ASSERT_EQ_D32(ret, EV_SUCCESS);
}

static void _test_process_redirect_pipe_on_write_done(ev_pipe_write_req_t* req, size_t size, int stat)
{
    (void)req;
    ASSERT_EQ_SIZE(size, strlen(g_test_process_data));
    ASSERT_EQ_D32(stat, EV_SUCCESS);

    _close_stdin_pipe();
}

TEST_F(process, redirect_pipe)
{
    int ret;

    /* start as echo server */
    char* argv[] = { g_test_process.self_exe_path, "--stdio_echo_server", NULL };

    ev_process_options_t opt;
    memset(&opt, 0, sizeof(opt));
    opt.argv = argv;
    opt.stdios[0].flag = EV_PROCESS_STDIO_REDIRECT_PIPE;
    opt.stdios[0].data.pipe = &g_test_process.stdin_pipe;
    opt.stdios[1].flag = EV_PROCESS_STDIO_REDIRECT_PIPE;
    opt.stdios[1].data.pipe = &g_test_process.stdout_pipe;
    opt.stdios[2].flag = EV_PROCESS_STDIO_REDIRECT_PIPE;
    opt.stdios[2].data.pipe = &g_test_process.stderr_pipe;
    ret = ev_process_spawn(&g_test_process.loop, &g_test_process.process, &opt);
    ASSERT_EQ_D32(ret, EV_SUCCESS);

    static ev_pipe_write_req_t write_req;
    ev_buf_t write_buf = ev_buf_make((char*)g_test_process_data, strlen(g_test_process_data));
    ret = ev_pipe_write(&g_test_process.stdin_pipe, &write_req, &write_buf, 1,
            _test_process_redirect_pipe_on_write_done);
    ASSERT_EQ_D32(ret, EV_SUCCESS);

    ASSERT_EQ_D32(ev_loop_run(&g_test_process.loop, EV_LOOP_MODE_DEFAULT), 0);
}
