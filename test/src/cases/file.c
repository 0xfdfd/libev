#include "ev.h"
#include "test.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
#   include  <io.h>
#   define unlink(x)        _unlink(x)
#   define access(a, b)     _access(a, b)
#   define F_OK             0
#else
#   include <unistd.h>
#endif

typedef struct test_file
{
    ev_loop_t       loop;           /**< Event loop */
    ev_file_t       file;           /**< File handle */
    ev_fs_req_t     token;          /**< Request token */

    struct
    {
        unsigned    file_init : 1;  /**< Flag for file init */
    }flags;

    ev_threadpool_t pool;           /**< Thread pool */
    ev_os_thread_t  threads[4];
}test_file_t;

test_file_t         g_test_file;    /**< Global test context */

static const char* s_sample_file = "sample_file";
static const char* s_test_buf = "abcdefghijklmnopqrstuvwxyz1234567890";

TEST_FIXTURE_SETUP(file)
{
    int ret;
    memset(&g_test_file, 0, sizeof(g_test_file));

    ret = ev_threadpool_init(&g_test_file.pool, NULL, g_test_file.threads,
        ARRAY_SIZE(g_test_file.threads));
    ASSERT_EQ_D32(ret, EV_SUCCESS);

    ret = ev_loop_init(&g_test_file.loop);
    ASSERT_EQ_D32(ret, EV_SUCCESS);

    ret = ev_loop_link_threadpool(&g_test_file.loop, &g_test_file.pool);
    ASSERT_EQ_D32(ret, EV_SUCCESS);

    ret = ev_file_init(&g_test_file.loop, &g_test_file.file);
    ASSERT_EQ_D32(ret, 0);

    g_test_file.flags.file_init = 1;
    unlink(s_sample_file);
}

TEST_FIXTURE_TEAREDOWN(file)
{
    if (g_test_file.flags.file_init)
    {
        ev_file_exit(&g_test_file.file, NULL);
        ASSERT_EQ_D32(ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT), EV_SUCCESS);
        g_test_file.flags.file_init = 0;
    }

    ASSERT_EQ_D32(ev_loop_exit(&g_test_file.loop), EV_SUCCESS);
    ev_threadpool_exit(&g_test_file.pool);
}

static void _test_file_on_open_nonexist_open(ev_file_t* file, ev_fs_req_t* req)
{
    ASSERT_EQ_PTR(file, &g_test_file.file);
    ASSERT_EQ_D32(req->result, EV_ENOENT);

    ASSERT_EQ_D32(access(s_sample_file, F_OK), -1);
}

TEST_F(file, open_nonexist)
{
    int ret;

    ret = ev_file_open(&g_test_file.file, &g_test_file.token, s_sample_file,
        EV_FS_O_RDWR, 0, _test_file_on_open_nonexist_open);
    ASSERT_EQ_D32(ret, EV_SUCCESS);

    ASSERT_EQ_D32(ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT), EV_SUCCESS);
}

static void _test_file_on_open_create_open(ev_file_t* file, ev_fs_req_t* req)
{
    ASSERT_EQ_PTR(file, &g_test_file.file);
    ASSERT_EQ_D32(req->result, EV_SUCCESS);

    ASSERT_EQ_D32(access(s_sample_file, F_OK), 0);
}

TEST_F(file, open_create)
{
    int ret;

    ret = ev_file_open(&g_test_file.file, &g_test_file.token, s_sample_file,
        EV_FS_O_RDWR | EV_FS_O_CREAT, EV_FS_S_IRUSR | EV_FS_S_IWUSR,
        _test_file_on_open_create_open);
    ASSERT_EQ_D32(ret, EV_SUCCESS);

    ASSERT_EQ_D32(ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT), EV_SUCCESS);
}

static void _test_file_read_write_on_open(ev_file_t* file, ev_fs_req_t* req)
{
    ASSERT_EQ_PTR(file, &g_test_file.file);
    ASSERT_EQ_D32(req->result, EV_SUCCESS);

    ASSERT_EQ_D32(access(s_sample_file, F_OK), 0);
}

static void _test_file_read_write_on_write_done(ev_file_t* file, ev_fs_req_t* req)
{
    ASSERT_EQ_PTR(file, &g_test_file.file);
    ASSERT_EQ_D32(req->result, strlen(s_test_buf));
}

static void _test_file_read_write_on_read_done(ev_file_t* file, ev_fs_req_t* req)
{
    ASSERT_EQ_PTR(file, &g_test_file.file);
    ASSERT_EQ_D32(req->result, strlen(s_test_buf));
}

TEST_F(file, read_write)
{
    int ret;

    ret = ev_file_open(&g_test_file.file, &g_test_file.token, s_sample_file,
        EV_FS_O_RDWR | EV_FS_O_CREAT, EV_FS_S_IRUSR | EV_FS_S_IWUSR,
        _test_file_read_write_on_open);
    ASSERT_EQ_D32(ret, EV_SUCCESS);

    ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
    ASSERT_EQ_D32(ret, EV_SUCCESS);

    ev_buf_t buf = ev_buf_make((void*)s_test_buf, strlen(s_test_buf));
    ret = ev_file_write(&g_test_file.file, &g_test_file.token, &buf, 1, 0,
        _test_file_read_write_on_write_done);
    ASSERT_EQ_D32(ret, EV_SUCCESS);

    ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
    ASSERT_EQ_D32(ret, EV_SUCCESS);

    char buffer[1024];
    buf = ev_buf_make(buffer, sizeof(buffer));
    ret = ev_file_read(&g_test_file.file, &g_test_file.token, &buf, 1, 0,
        _test_file_read_write_on_read_done);
    ASSERT_EQ_D32(ret, EV_SUCCESS);

    ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
    ASSERT_EQ_D32(ret, EV_SUCCESS);

    buffer[strlen(s_test_buf)] = '\0';
    ASSERT_EQ_STR(s_test_buf, buffer);
}

static void _test_file_stat_on_stat(ev_file_t* file, ev_fs_req_t* req)
{
    ASSERT_EQ_PTR(file, &g_test_file.file);

    ASSERT_EQ_D32(req->result, EV_SUCCESS);
    ev_file_stat_t* statbuf = ev_fs_get_statbuf(req);

    ASSERT_EQ_U64(statbuf->st_size, 0);
}

TEST_F(file, stat)
{
    int ret;

    ret = ev_file_open(&g_test_file.file, &g_test_file.token, s_sample_file,
        EV_FS_O_RDWR | EV_FS_O_CREAT, EV_FS_S_IRUSR | EV_FS_S_IWUSR,
        _test_file_read_write_on_open);
    ASSERT_EQ_D32(ret, EV_SUCCESS);

    ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
    ASSERT_EQ_D32(ret, EV_SUCCESS);

    ret = ev_file_stat(&g_test_file.file, &g_test_file.token, _test_file_stat_on_stat);
    ASSERT_EQ_D32(ret, EV_SUCCESS);

    ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
    ASSERT_EQ_D32(ret, EV_SUCCESS);
}
