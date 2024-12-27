#include "ev.h"
#include "test.h"
#include "utils/file.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <io.h>
#include <direct.h>
#define rmdir(p) _rmdir(p)
#define access(a, b) _access(a, b)
#define F_OK 0
#else
#include <unistd.h>
#endif

typedef struct test_file
{
    ev_loop_t   *loop;  /**< Event loop */
    ev_file_t   *file;  /**< File handle */
    ev_fs_req_t  token; /**< Request token */
    ev_fs_stat_t stat;
} test_file_t;

test_file_t g_test_file; /**< Global test context */

static const char *s_sample_file = "sample_file";
static const char *s_sample_data = "abcdefghijklmnopqrstuvwxyz1234567890";
static const char *s_sample_path = "./dir1/dir2/dir3/dir4";

static void _test_fs_cleanup(void)
{
    static char buffer[1024];

    /* delete sample file */
    ev_fs_remove(NULL, NULL, s_sample_file, 1, NULL);

    /* delete sample dir */
    snprintf(buffer, sizeof(buffer), "%s", s_sample_path);
    (void)rmdir(buffer);

    char *p;
    for (p = strrchr(buffer, '/'); p != NULL; p = strrchr(buffer, '/'))
    {
        *p = '\0';
        (void)rmdir(buffer);
    }
}

static void _test_fs_close_file(void)
{
    if (g_test_file.file != NULL)
    {
        ev_file_close(g_test_file.file, NULL);
        ASSERT_EQ_INT(ev_loop_run(g_test_file.loop, EV_LOOP_MODE_NOWAIT, 0), 0);

        ev_free(g_test_file.file);
        g_test_file.file = NULL;
    }
}

TEST_FIXTURE_SETUP(fs)
{
    int ret;
    memset(&g_test_file, 0, sizeof(g_test_file));

    ret = ev_loop_init(&g_test_file.loop);
    ASSERT_EQ_INT(ret, 0);

    _test_fs_cleanup();
}

TEST_FIXTURE_TEARDOWN(fs)
{
    _test_fs_close_file();

    ASSERT_EQ_INT(ev_loop_exit(g_test_file.loop), 0);

    _test_fs_cleanup();
}

//////////////////////////////////////////////////////////////////////////
// fs.open_nonexist
//////////////////////////////////////////////////////////////////////////

static void _test_file_on_open_nonexist_open(ev_fs_req_t *req)
{
    ASSERT_EQ_PTR(ev_fs_get_file(req), g_test_file.file);
    ASSERT_EQ_SSIZE(req->result, EV_ENOENT);

    ASSERT_EQ_INT(access(s_sample_file, F_OK), -1);
    ev_fs_req_cleanup(req);
}

TEST_F(fs, open_nonexist)
{
    int ret;

    g_test_file.file = ev_malloc(sizeof(ev_file_t));

    ret = ev_file_open(g_test_file.loop, g_test_file.file, &g_test_file.token,
                       s_sample_file, EV_FS_O_RDWR, 0,
                       _test_file_on_open_nonexist_open);
    ASSERT_EQ_INT(ret, 0);

    ASSERT_EQ_INT(ev_loop_run(g_test_file.loop, EV_LOOP_MODE_DEFAULT,
                              EV_INFINITE_TIMEOUT),
                  0);
}

//////////////////////////////////////////////////////////////////////////
// fs.open_create
//////////////////////////////////////////////////////////////////////////

static void _test_file_on_open_create_open(ev_fs_req_t *req)
{
    ASSERT_EQ_PTR(ev_fs_get_file(req), g_test_file.file);
    ASSERT_EQ_SSIZE(req->result, 0);

    ASSERT_EQ_INT(access(s_sample_file, F_OK), 0);
    ev_fs_req_cleanup(req);
}

TEST_F(fs, open_create)
{
    int ret;

    g_test_file.file = ev_malloc(sizeof(ev_file_t));

    ret = ev_file_open(g_test_file.loop, g_test_file.file, &g_test_file.token,
                       s_sample_file, EV_FS_O_RDWR | EV_FS_O_CREAT,
                       EV_FS_S_IRUSR | EV_FS_S_IWUSR,
                       _test_file_on_open_create_open);
    ASSERT_EQ_INT(ret, 0);

    ASSERT_EQ_INT(ev_loop_run(g_test_file.loop, EV_LOOP_MODE_DEFAULT,
                              EV_INFINITE_TIMEOUT),
                  0);
}

//////////////////////////////////////////////////////////////////////////
// fs.pread_pwrite
//////////////////////////////////////////////////////////////////////////

static void _test_file_pread_pwrite_on_open(ev_fs_req_t *req)
{
    ASSERT_EQ_PTR(ev_fs_get_file(req), g_test_file.file);
    ASSERT_EQ_SSIZE(req->result, 0);

    ASSERT_EQ_INT(access(s_sample_file, F_OK), 0);
    ev_fs_req_cleanup(req);
}

static void _test_file_pread_pwrite_on_write_done(ev_fs_req_t *req)
{
    ASSERT_EQ_PTR(ev_fs_get_file(req), g_test_file.file);
    ASSERT_EQ_SSIZE(req->result, strlen(s_sample_data));
    ev_fs_req_cleanup(req);
}

static void _test_file_pread_pwrite_on_read_done(ev_fs_req_t *req)
{
    ASSERT_EQ_PTR(ev_fs_get_file(req), g_test_file.file);
    ASSERT_EQ_SSIZE(req->result, strlen(s_sample_data));
    ev_fs_req_cleanup(req);
}

TEST_F(fs, pread_pwrite)
{
    int  ret;
    char buffer[1024];

    g_test_file.file = ev_malloc(sizeof(ev_file_t));

    ret = ev_file_open(g_test_file.loop, g_test_file.file, &g_test_file.token,
                       s_sample_file, EV_FS_O_RDWR | EV_FS_O_CREAT,
                       EV_FS_S_IRUSR | EV_FS_S_IWUSR,
                       _test_file_pread_pwrite_on_open);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_loop_run(g_test_file.loop, EV_LOOP_MODE_DEFAULT,
                      EV_INFINITE_TIMEOUT);
    ASSERT_EQ_INT(ret, 0);

    ev_buf_t buf = ev_buf_make((void *)s_sample_data, strlen(s_sample_data));
    ret = (int)ev_file_pwritev(g_test_file.file, &g_test_file.token, &buf, 1, 0,
                               _test_file_pread_pwrite_on_write_done);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_loop_run(g_test_file.loop, EV_LOOP_MODE_DEFAULT,
                      EV_INFINITE_TIMEOUT);
    ASSERT_EQ_INT(ret, 0);

    buf = ev_buf_make(buffer, sizeof(buffer));
    ret = (int)ev_file_preadv(g_test_file.file, &g_test_file.token, &buf, 1, 0,
                              _test_file_pread_pwrite_on_read_done);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_loop_run(g_test_file.loop, EV_LOOP_MODE_DEFAULT,
                      EV_INFINITE_TIMEOUT);
    ASSERT_EQ_INT(ret, 0);

    buffer[strlen(s_sample_data)] = '\0';
    ASSERT_EQ_STR(s_sample_data, buffer);
}

//////////////////////////////////////////////////////////////////////////
// fs.read_write
//////////////////////////////////////////////////////////////////////////

static void _test_file_read_write_on_open(ev_fs_req_t *req)
{
    ASSERT_EQ_PTR(ev_fs_get_file(req), g_test_file.file);
    ASSERT_EQ_SSIZE(req->result, 0);

    ASSERT_EQ_INT(access(s_sample_file, F_OK), 0);
    ev_fs_req_cleanup(req);
}

static void _test_file_read_write_on_write_done(ev_fs_req_t *req)
{
    ASSERT_EQ_PTR(ev_fs_get_file(req), g_test_file.file);
    ASSERT_EQ_SSIZE(req->result, strlen(s_sample_data));
    ev_fs_req_cleanup(req);
}

static void _test_file_read_write_on_read_done(ev_fs_req_t *req)
{
    ASSERT_EQ_PTR(ev_fs_get_file(req), g_test_file.file);
    ASSERT_EQ_SSIZE(req->result, 4);
    ev_fs_req_cleanup(req);
}

static void _test_file_read_write_on_seek_done(ev_fs_req_t *req)
{
    ev_fs_req_cleanup(req);
}

TEST_F(fs, read_write)
{
    int  ret;
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    g_test_file.file = ev_malloc(sizeof(ev_file_t));

    ret = ev_file_open(g_test_file.loop, g_test_file.file, &g_test_file.token,
                       s_sample_file, EV_FS_O_RDWR | EV_FS_O_CREAT,
                       EV_FS_S_IRUSR | EV_FS_S_IWUSR,
                       _test_file_read_write_on_open);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_loop_run(g_test_file.loop, EV_LOOP_MODE_DEFAULT,
                      EV_INFINITE_TIMEOUT);
    ASSERT_EQ_INT(ret, 0);

    ev_buf_t buf = ev_buf_make((void *)s_sample_data, strlen(s_sample_data));
    ret = (int)ev_file_pwritev(g_test_file.file, &g_test_file.token, &buf, 1, 0,
                               _test_file_read_write_on_write_done);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_loop_run(g_test_file.loop, EV_LOOP_MODE_DEFAULT,
                      EV_INFINITE_TIMEOUT);
    ASSERT_EQ_INT(ret, 0);

    ret =
        (int)ev_file_seek(g_test_file.file, &g_test_file.token, EV_FS_SEEK_BEG,
                          0, _test_file_read_write_on_seek_done);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_loop_run(g_test_file.loop, EV_LOOP_MODE_DEFAULT,
                      EV_INFINITE_TIMEOUT);
    ASSERT_EQ_INT(ret, 0);

    buf = ev_buf_make(buffer, 4);
    ret = (int)ev_file_readv(g_test_file.file, &g_test_file.token, &buf, 1,
                             _test_file_read_write_on_read_done);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_loop_run(g_test_file.loop, EV_LOOP_MODE_DEFAULT,
                      EV_INFINITE_TIMEOUT);
    ASSERT_EQ_INT(ret, 0);
    ret = memcmp(buffer, "abcd", 4);
    ASSERT_EQ_INT(ret, 0);

    buf = ev_buf_make(buffer, 4);
    ret = (int)ev_file_readv(g_test_file.file, &g_test_file.token, &buf, 1,
                             _test_file_read_write_on_read_done);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_loop_run(g_test_file.loop, EV_LOOP_MODE_DEFAULT,
                      EV_INFINITE_TIMEOUT);
    ASSERT_EQ_INT(ret, 0);
    ret = memcmp(buffer, "efgh", 4);
    ASSERT_EQ_INT(ret, 0);
}

//////////////////////////////////////////////////////////////////////////
// fs.fstat
//////////////////////////////////////////////////////////////////////////

static void _test_file_stat_on_stat(ev_fs_req_t *req)
{
    ASSERT_EQ_PTR(ev_fs_get_file(req), g_test_file.file);

    ASSERT_EQ_SSIZE(req->result, 0);
    ev_fs_stat_t *statbuf = ev_fs_get_statbuf(req);

    ASSERT_EQ_UINT64(statbuf->st_size, 0);
    ev_fs_req_cleanup(req);
}

TEST_F(fs, fstat)
{
    int ret;

    g_test_file.file = ev_malloc(sizeof(ev_file_t));

    ret = ev_file_open(g_test_file.loop, g_test_file.file, &g_test_file.token,
                       s_sample_file, EV_FS_O_RDWR | EV_FS_O_CREAT,
                       EV_FS_S_IRUSR | EV_FS_S_IWUSR,
                       _test_file_pread_pwrite_on_open);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_loop_run(g_test_file.loop, EV_LOOP_MODE_DEFAULT,
                      EV_INFINITE_TIMEOUT);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_file_stat(g_test_file.file, &g_test_file.token, &g_test_file.stat,
                       _test_file_stat_on_stat);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_loop_run(g_test_file.loop, EV_LOOP_MODE_DEFAULT,
                      EV_INFINITE_TIMEOUT);
    ASSERT_EQ_INT(ret, 0);
}

//////////////////////////////////////////////////////////////////////////
// fs.readdir
//////////////////////////////////////////////////////////////////////////

static void _test_file_readdir_on_readdir(ev_fs_req_t *req)
{
    int flag_have_exe = 0;
    ASSERT_GT_SSIZE(req->result, 0);

    ev_dirent_t *info = ev_fs_get_first_dirent(req);
    ASSERT_NE_PTR(info, NULL);

    for (; info != NULL; info = ev_fs_get_next_dirent(info))
    {
        if (strcmp(info->name, test_self_exe_name()) == 0)
        {
            flag_have_exe = 1;
            break;
        }
    }

    ASSERT_EQ_INT(flag_have_exe, 1);
    ev_fs_req_cleanup(req);
}

TEST_F(fs, readdir)
{
    int ret;

    const char *exe_dir = test_get_self_dir();

    ret = (int)ev_fs_readdir(g_test_file.loop, &g_test_file.token, exe_dir,
                             _test_file_readdir_on_readdir);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_loop_run(g_test_file.loop, EV_LOOP_MODE_DEFAULT,
                      EV_INFINITE_TIMEOUT);
    ASSERT_EQ_INT(ret, 0);
}

//////////////////////////////////////////////////////////////////////////
// fs.readdir_nonexist
//////////////////////////////////////////////////////////////////////////

static void _test_file_readdir_nonexist_on_readdir(ev_fs_req_t *req)
{
    ASSERT_EQ_SSIZE(req->result, EV_ENOENT);
    ev_fs_req_cleanup(req);
}

TEST_F(fs, readdir_nonexist)
{
    int         ret;
    const char *path = "./non-exist-dir";

    ret = (int)ev_fs_readdir(g_test_file.loop, &g_test_file.token, path,
                             _test_file_readdir_nonexist_on_readdir);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_loop_run(g_test_file.loop, EV_LOOP_MODE_DEFAULT,
                      EV_INFINITE_TIMEOUT);
    ASSERT_EQ_INT(ret, 0);
}

//////////////////////////////////////////////////////////////////////////
// fs.readfile
//////////////////////////////////////////////////////////////////////////

static void _test_file_readfile_on_readfile(ev_fs_req_t *req)
{
    ev_buf_t *buf = ev_fs_get_filecontent(req);
    ASSERT_EQ_SIZE(buf->size, strlen(s_sample_data));

    int ret = memcmp(buf->data, s_sample_data, buf->size);
    ASSERT_EQ_INT(ret, 0);

    ev_fs_req_cleanup(req);
}

TEST_F(fs, readfile)
{
    int ret;

    ret = test_write_file(s_sample_file, s_sample_data, strlen(s_sample_data));
    ASSERT_EQ_INT(ret, 0);

    ret = (int)ev_fs_readfile(g_test_file.loop, &g_test_file.token,
                              s_sample_file, _test_file_readfile_on_readfile);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_loop_run(g_test_file.loop, EV_LOOP_MODE_DEFAULT,
                      EV_INFINITE_TIMEOUT);
    ASSERT_EQ_INT(ret, 0);
}

//////////////////////////////////////////////////////////////////////////
// fs.mkdir
//////////////////////////////////////////////////////////////////////////

static void _test_fs_mkdir_on_mkdir(ev_fs_req_t *req)
{
    ASSERT_EQ_SSIZE(req->result, 0);
    ev_fs_req_cleanup(req);
}

TEST_F(fs, mkdir)
{
    int ret;
    ASSERT_EQ_INT(test_access_dir(s_sample_path), EV_ENOENT);

    ret = ev_fs_mkdir(g_test_file.loop, &g_test_file.token, s_sample_path,
                      EV_FS_S_IRWXU, _test_fs_mkdir_on_mkdir);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_loop_run(g_test_file.loop, EV_LOOP_MODE_DEFAULT,
                      EV_INFINITE_TIMEOUT);
    ASSERT_EQ_INT(ret, 0);

    ASSERT_EQ_INT(test_access_dir(s_sample_path), 0);
}
