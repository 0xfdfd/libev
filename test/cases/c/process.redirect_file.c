#include "test.h"

#define TEST_PROCESS_TMPFILE_PATH   "56267b38-5b28-4819-84d8-142eb29ce949"

typedef struct test_process
{
    char*           self_exe_path;
    ev_process_t*   process;

    ev_file_t*      file;
    ev_loop_t*      loop;
    ev_fs_req_t     token;
} test_process_t;

static test_process_t g_test_process;

TEST_FIXTURE_SETUP(process)
{
    ev_fs_remove_sync(TEST_PROCESS_TMPFILE_PATH, 1);
    memset(&g_test_process, 0, sizeof(g_test_process));

    g_test_process.loop = ev_calloc(1, sizeof(ev_loop_t));
    ASSERT_EQ_INT(ev_loop_init(g_test_process.loop), 0);

    g_test_process.file = ev_calloc(1, sizeof(ev_file_t));
    ASSERT_EQ_INT(ev_file_init(g_test_process.loop, g_test_process.file), 0);

    g_test_process.self_exe_path = mmc_strdup(test_get_self_exe());
    g_test_process.process = ev_calloc(1, sizeof(ev_process_t));
}

TEST_FIXTURE_TEARDOWN(process)
{
    if (g_test_process.file != NULL)
    {
        ev_file_exit(g_test_process.file, NULL);
        ev_loop_run(g_test_process.loop, EV_LOOP_MODE_NOWAIT);

        ev_free(g_test_process.file);
        g_test_process.file = NULL;
    }

    ev_loop_exit(g_test_process.loop);
    ev_free(g_test_process.loop);
    g_test_process.loop = NULL;

    ev_free(g_test_process.process);
    g_test_process.process = NULL;

    ev_free(g_test_process.self_exe_path);
    g_test_process.self_exe_path = NULL;

    ev_fs_remove_sync(TEST_PROCESS_TMPFILE_PATH, 1);
}

static void _test_process_redirect_file_on_open(ev_fs_req_t* req)
{
    ASSERT_EQ_SSIZE(req->result, 0);
    ev_fs_req_cleanup(req);
}

static void _test_process_redirect_file_on_chld(ev_process_t* handle,
    ev_process_exit_status_t exit_status, int exit_code)
{
    ASSERT_EQ_INT(exit_status, EV_PROCESS_EXIT_NORMAL);
    ASSERT_EQ_INT(exit_code, 0);

    ev_process_exit(handle, NULL);
    ev_loop_run(g_test_process.loop, EV_LOOP_MODE_NOWAIT);
}

TEST_F(process, redirect_file)
{
    int ret;
    
    ret = ev_file_open(g_test_process.file,
        &g_test_process.token,
        TEST_PROCESS_TMPFILE_PATH,
        EV_FS_O_CREAT | EV_FS_O_WRONLY,
        EV_FS_S_IRWXU,
        _test_process_redirect_file_on_open);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_loop_run(g_test_process.loop, EV_LOOP_MODE_DEFAULT);
    ASSERT_EQ_INT(ret, 0);

    char* argv[] = { g_test_process.self_exe_path, "--", "help", NULL };

    ev_process_options_t opt;
    memset(&opt, 0, sizeof(opt));
    opt.argv = argv;
    opt.on_exit = _test_process_redirect_file_on_chld;
    opt.stdios[1].flag = EV_PROCESS_STDIO_REDIRECT_FD;
    opt.stdios[1].data.fd = g_test_process.file->file;
    ret = ev_process_spawn(g_test_process.loop, g_test_process.process, &opt);
    ASSERT_EQ_INT(ret, 0);

	ret = ev_loop_run(g_test_process.loop, EV_LOOP_MODE_DEFAULT);
	ASSERT_EQ_INT(ret, 0);
}
