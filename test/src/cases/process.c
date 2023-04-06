#include "test.h"

typedef struct test_process
{
    char*           self_exe_path;  /**< Full path to self */
    ev_process_t    process;        /**< Process handle */
    ev_loop_t       loop;           /**< Event loop */

    ev_pipe_t       stdin_pipe;     /**< STDIN pipe for child process */
    ev_pipe_t       stdout_pipe;    /**< STDOUT pipe for child process */
    ev_pipe_t       stderr_pipe;    /**< STDERR pipe for child process */

    int             flag_stdin_init;
    int             flag_exit;
}test_process_t;

test_process_t*     g_test_process;
static const char* g_test_process_data = "abcdefghijklmnopqrstuvwxyz1234567890";

static void _close_stdin_pipe(void)
{
    if (g_test_process->flag_stdin_init)
    {
        g_test_process->flag_stdin_init = 0;
        ev_pipe_exit(&g_test_process->stdin_pipe, NULL);
    }
}

TEST_FIXTURE_SETUP(process)
{
    g_test_process = mmc_calloc(1, sizeof(*g_test_process));

    g_test_process->self_exe_path = mmc_strdup(test_get_self_exe());
    ASSERT_NE_PTR(g_test_process->self_exe_path, NULL);

    ASSERT_EQ_INT(ev_loop_init(&g_test_process->loop), 0);
    ASSERT_EQ_INT(ev_pipe_init(&g_test_process->loop, &g_test_process->stdin_pipe, 0), 0);
    ASSERT_EQ_INT(ev_pipe_init(&g_test_process->loop, &g_test_process->stdout_pipe, 0), 0);
    ASSERT_EQ_INT(ev_pipe_init(&g_test_process->loop, &g_test_process->stderr_pipe, 0), 0);

    g_test_process->flag_stdin_init = 1;
}

TEST_FIXTURE_TEARDOWN(process)
{
    _close_stdin_pipe();
    ev_pipe_exit(&g_test_process->stdout_pipe, NULL);
    ev_pipe_exit(&g_test_process->stderr_pipe, NULL);

    ASSERT_EQ_INT(ev_loop_run(&g_test_process->loop, EV_LOOP_MODE_DEFAULT), 0);
    ASSERT_EQ_EVLOOP(&g_test_process->loop, &empty_loop);
    ASSERT_EQ_INT(ev_loop_exit(&g_test_process->loop), 0);

    mmc_free(g_test_process->self_exe_path);
    g_test_process->self_exe_path = NULL;

    mmc_free(g_test_process);
    g_test_process = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// process.spawn_stdout_ignore
///////////////////////////////////////////////////////////////////////////////

TEST_F(process, spawn_stdout_ignore)
{
    int ret;

    char* argv[] = { g_test_process->self_exe_path, "--help", NULL };

    ev_process_options_t opt;
    memset(&opt, 0, sizeof(opt));
    opt.argv = argv;
    opt.stdios[1].flag = EV_PROCESS_STDIO_REDIRECT_NULL;

    ret = ev_process_spawn(&g_test_process->loop, &g_test_process->process, &opt);
    ASSERT_EQ_INT(ret, 0);

    ev_process_exit(&g_test_process->process, NULL);
}

///////////////////////////////////////////////////////////////////////////////
// process.redirect_pipe
///////////////////////////////////////////////////////////////////////////////

static void _test_process_redirect_pipe_on_write_done(ev_pipe_write_req_t* req, ssize_t size)
{
    (void)req;
    ASSERT_EQ_SSIZE(size, strlen(g_test_process_data));

    _close_stdin_pipe();

    ev_process_exit(&g_test_process->process, NULL);
}

TEST_F(process, redirect_pipe)
{
    int ret;

    /* start as echo server */
    char* argv[] = { g_test_process->self_exe_path, "--", "echoserver", NULL };

    ev_process_options_t opt;
    memset(&opt, 0, sizeof(opt));
    opt.argv = argv;
    opt.stdios[0].flag = EV_PROCESS_STDIO_REDIRECT_PIPE;
    opt.stdios[0].data.pipe = &g_test_process->stdin_pipe;
    opt.stdios[1].flag = EV_PROCESS_STDIO_REDIRECT_PIPE;
    opt.stdios[1].data.pipe = &g_test_process->stdout_pipe;
    opt.stdios[2].flag = EV_PROCESS_STDIO_REDIRECT_PIPE;
    opt.stdios[2].data.pipe = &g_test_process->stderr_pipe;
    ret = ev_process_spawn(&g_test_process->loop, &g_test_process->process, &opt);
    ASSERT_EQ_INT(ret, 0);

    static ev_pipe_write_req_t write_req;
    ev_buf_t write_buf = ev_buf_make((char*)g_test_process_data, strlen(g_test_process_data));
    ret = ev_pipe_write(&g_test_process->stdin_pipe, &write_req, &write_buf, 1,
            _test_process_redirect_pipe_on_write_done);
    ASSERT_EQ_INT(ret, 0);

    ASSERT_EQ_INT(ev_loop_run(&g_test_process->loop, EV_LOOP_MODE_DEFAULT), 0);
}

///////////////////////////////////////////////////////////////////////////////
// process.exit_callback
///////////////////////////////////////////////////////////////////////////////

static void _test_process_exit_callback(ev_process_t* handle,
        ev_process_exit_status_t exit_status, int exit_code)
{
    ASSERT_EQ_PTR(&g_test_process->process, handle);
    ASSERT_EQ_INT(exit_status, EV_PROCESS_EXIT_NORMAL);
    ASSERT_EQ_INT(exit_code, 0);
    g_test_process->flag_exit = 1;

    ev_process_exit(&g_test_process->process, NULL);
}

TEST_F(process, exit_callback)
{
    int ret;
    char* argv[] = { g_test_process->self_exe_path, "--help", NULL };

    ev_process_options_t opt;
    memset(&opt, 0, sizeof(opt));
    opt.argv = argv;
    opt.on_exit = _test_process_exit_callback;
    opt.stdios[1].flag = EV_PROCESS_STDIO_REDIRECT_NULL;

    ret = ev_process_spawn(&g_test_process->loop, &g_test_process->process, &opt);
    ASSERT_EQ_INT(ret, 0);

    ASSERT_EQ_INT(ev_loop_run(&g_test_process->loop, EV_LOOP_MODE_DEFAULT), 0);
    ASSERT_EQ_INT(g_test_process->flag_exit, 1);
}

//////////////////////////////////////////////////////////////////////////
// process.getcwd
//////////////////////////////////////////////////////////////////////////
TEST_F(process, getcwd)
{
    char buffer[4096];

    ssize_t ret = ev_getcwd(buffer, sizeof(buffer));
    ASSERT_GT_INT64(ret, 0);

    ASSERT_NE_CHAR(buffer[ret - 1], '/');
    ASSERT_NE_CHAR(buffer[ret - 1], '\\');
}

//////////////////////////////////////////////////////////////////////////
// process.exepath
//////////////////////////////////////////////////////////////////////////
TEST_F(process, exepath)
{
    char buffer[4096];
    ssize_t ret = ev_exepath(buffer, sizeof(buffer));
    ASSERT_GT_INT64(ret, 0);
    ASSERT_LT_INT64(ret, sizeof(buffer));
    ASSERT_NE_CHAR(buffer[0], '\0');
    ASSERT_EQ_CHAR(buffer[ret], '\0');

    /* Path should not terminal with '/' or '\' */
    ASSERT_NE_CHAR(buffer[ret - 1], '/');
    ASSERT_NE_CHAR(buffer[ret - 1], '\\');

    ASSERT_EQ_INT64(ret, ev_exepath(NULL, 0));
}

//////////////////////////////////////////////////////////////////////////
// process.cwd
//////////////////////////////////////////////////////////////////////////

typedef struct process_cwd_path
{
    char buffer[4096];
    ev_pipe_read_req_t req;
}process_cwd_path_t;

static void _process_cwd_on_read(ev_pipe_read_req_t* req, ssize_t size)
{
    process_cwd_path_t* cwd_path = EV_CONTAINER_OF(req, process_cwd_path_t, req);

    cwd_path->buffer[size] = '\0';

    size--;
    while (cwd_path->buffer[size] == '\r' || cwd_path->buffer[size] == '\n')
    {
        cwd_path->buffer[size] = '\0';
        size--;
    }

    ev_process_exit(&g_test_process->process, NULL);
}

TEST_F(process, cwd)
{
    int ret;
    
    char* dir_path = file_parrent_dir(test_get_self_dir());
    char* argv[] = { g_test_process->self_exe_path, "--", "pwd", NULL };

    ev_process_options_t opt;
    memset(&opt, 0, sizeof(opt));
    opt.argv = argv;
    opt.cwd = dir_path;
    opt.stdios[1].flag = EV_PROCESS_STDIO_REDIRECT_PIPE;
    opt.stdios[1].data.pipe = &g_test_process->stdout_pipe;

    ret = ev_process_spawn(&g_test_process->loop, &g_test_process->process, &opt);
    ASSERT_EQ_INT(ret, 0);

    process_cwd_path_t cwd_path;
    memset(&cwd_path, 0, sizeof(cwd_path));
    ev_buf_t buf = ev_buf_make(cwd_path.buffer, sizeof(cwd_path.buffer));

    ret = ev_pipe_read(&g_test_process->stdout_pipe, &cwd_path.req, &buf, 1,
        _process_cwd_on_read);
    ASSERT_EQ_INT(ret, 0);

    ASSERT_EQ_INT(ev_loop_run(&g_test_process->loop, EV_LOOP_MODE_DEFAULT), 0);

    ASSERT_EQ_STR(cwd_path.buffer, dir_path);
    mmc_free(dir_path);
}
