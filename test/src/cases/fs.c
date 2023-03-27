#include "ev.h"
#include "test.h"
#include "utils/file.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
#   include  <io.h>
#   include <direct.h>
#   define unlink(x)        _unlink(x)
#   define rmdir(p)         _rmdir(p)
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
        unsigned    file_init : 1;  /**< Flag for file initialize */
    }flags;

    ev_threadpool_t pool;           /**< Thread pool */
    ev_os_thread_t  threads[4];     /**< Thread storage */
}test_file_t;

test_file_t         g_test_file;    /**< Global test context */

static const char* s_sample_file = "sample_file";
static const char* s_sample_data = "abcdefghijklmnopqrstuvwxyz1234567890";
static const char* s_sample_path = "./dir1/dir2/dir3/dir4";

static void _test_fs_cleanup(void)
{
    static char buffer[1024];

    /* delete sample file */
    unlink(s_sample_file);

    /* delete sample dir */
    snprintf(buffer, sizeof(buffer), "%s", s_sample_path);
    (void)rmdir(buffer);

    char* p;
    for (p = strrchr(buffer, '/'); p != NULL; p = strrchr(buffer, '/'))
    {
        *p = '\0';
        (void)rmdir(buffer);
    }
}

static void _test_fs_close_file(void)
{
	if (g_test_file.flags.file_init)
	{
		ev_file_exit(&g_test_file.file, NULL);
		ASSERT_EQ_INT(ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_NOWAIT), EV_SUCCESS);
		g_test_file.flags.file_init = 0;
	}
}

static void _test_fs_init_file(void)
{
	int ret = ev_file_init(&g_test_file.loop, &g_test_file.file);
	ASSERT_EQ_INT(ret, 0);
	g_test_file.flags.file_init = 1;
}

TEST_FIXTURE_SETUP(fs)
{
    int ret;
    memset(&g_test_file, 0, sizeof(g_test_file));

    ret = ev_threadpool_init(&g_test_file.pool, NULL, g_test_file.threads,
        ARRAY_SIZE(g_test_file.threads));
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    ret = ev_loop_init(&g_test_file.loop);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    ret = ev_loop_link_threadpool(&g_test_file.loop, &g_test_file.pool);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    _test_fs_init_file();
    _test_fs_cleanup();
}

TEST_FIXTURE_TEARDOWN(fs)
{
    _test_fs_close_file();

    ASSERT_EQ_EVLOOP(&g_test_file.loop, &empty_loop);
    ASSERT_EQ_INT(ev_loop_exit(&g_test_file.loop), EV_SUCCESS);
    ev_threadpool_exit(&g_test_file.pool);

    _test_fs_cleanup();
}

//////////////////////////////////////////////////////////////////////////
// fs.open_nonexist
//////////////////////////////////////////////////////////////////////////

static void _test_file_on_open_nonexist_open(ev_fs_req_t* req)
{
    ASSERT_EQ_PTR(ev_fs_get_file(req), &g_test_file.file);
    ASSERT_EQ_SSIZE(req->result, EV_ENOENT);

    ASSERT_EQ_INT(access(s_sample_file, F_OK), -1);
    ev_fs_req_cleanup(req);
}

TEST_F(fs, open_nonexist)
{
    int ret;

    ret = ev_file_open(&g_test_file.file, &g_test_file.token, s_sample_file,
        EV_FS_O_RDWR, 0, _test_file_on_open_nonexist_open);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    ASSERT_EQ_INT(ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT), EV_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
// fs.open_create
//////////////////////////////////////////////////////////////////////////

static void _test_file_on_open_create_open(ev_fs_req_t* req)
{
    ASSERT_EQ_PTR(ev_fs_get_file(req), &g_test_file.file);
    ASSERT_EQ_SSIZE(req->result, EV_SUCCESS);

    ASSERT_EQ_INT(access(s_sample_file, F_OK), 0);
    ev_fs_req_cleanup(req);
}

TEST_F(fs, open_create)
{
    int ret;

    ret = ev_file_open(&g_test_file.file, &g_test_file.token, s_sample_file,
        EV_FS_O_RDWR | EV_FS_O_CREAT, EV_FS_S_IRUSR | EV_FS_S_IWUSR,
        _test_file_on_open_create_open);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    ASSERT_EQ_INT(ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT), EV_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
// fs.pread_pwrite
//////////////////////////////////////////////////////////////////////////

static void _test_file_pread_pwrite_on_open(ev_fs_req_t* req)
{
    ASSERT_EQ_PTR(ev_fs_get_file(req), &g_test_file.file);
    ASSERT_EQ_SSIZE(req->result, EV_SUCCESS);

    ASSERT_EQ_INT(access(s_sample_file, F_OK), 0);
    ev_fs_req_cleanup(req);
}

static void _test_file_pread_pwrite_on_write_done(ev_fs_req_t* req)
{
    ASSERT_EQ_PTR(ev_fs_get_file(req), &g_test_file.file);
    ASSERT_EQ_SSIZE(req->result, strlen(s_sample_data));
    ev_fs_req_cleanup(req);
}

static void _test_file_pread_pwrite_on_read_done(ev_fs_req_t* req)
{
    ASSERT_EQ_PTR(ev_fs_get_file(req), &g_test_file.file);
    ASSERT_EQ_SSIZE(req->result, strlen(s_sample_data));
    ev_fs_req_cleanup(req);
}

TEST_F(fs, pread_pwrite)
{
    int ret;
    char buffer[1024];

    ret = ev_file_open(&g_test_file.file, &g_test_file.token, s_sample_file,
        EV_FS_O_RDWR | EV_FS_O_CREAT, EV_FS_S_IRUSR | EV_FS_S_IWUSR,
        _test_file_pread_pwrite_on_open);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    ev_buf_t buf = ev_buf_make((void*)s_sample_data, strlen(s_sample_data));
    ret = ev_file_pwrite(&g_test_file.file, &g_test_file.token, &buf, 1, 0,
        _test_file_pread_pwrite_on_write_done);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    buf = ev_buf_make(buffer, sizeof(buffer));
    ret = ev_file_pread(&g_test_file.file, &g_test_file.token, &buf, 1, 0,
        _test_file_pread_pwrite_on_read_done);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    buffer[strlen(s_sample_data)] = '\0';
    ASSERT_EQ_STR(s_sample_data, buffer);
}

//////////////////////////////////////////////////////////////////////////
// fs.read_write
//////////////////////////////////////////////////////////////////////////

static void _test_file_read_write_on_open(ev_fs_req_t* req)
{
	ASSERT_EQ_PTR(ev_fs_get_file(req), &g_test_file.file);
	ASSERT_EQ_SSIZE(req->result, EV_SUCCESS);

	ASSERT_EQ_INT(access(s_sample_file, F_OK), 0);
	ev_fs_req_cleanup(req);
}

static void _test_file_read_write_on_write_done(ev_fs_req_t* req)
{
	ASSERT_EQ_PTR(ev_fs_get_file(req), &g_test_file.file);
	ASSERT_EQ_SSIZE(req->result, strlen(s_sample_data));
	ev_fs_req_cleanup(req);
}

static void _test_file_read_write_on_read_done(ev_fs_req_t* req)
{
	ASSERT_EQ_PTR(ev_fs_get_file(req), &g_test_file.file);
    ASSERT_EQ_SSIZE(req->result, 4);
	ev_fs_req_cleanup(req);
}

static void _test_file_read_write_on_seek_done(ev_fs_req_t* req)
{
    ev_fs_req_cleanup(req);
}

TEST_F(fs, read_write)
{
    int ret;
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

	ret = ev_file_open(&g_test_file.file, &g_test_file.token, s_sample_file,
		EV_FS_O_RDWR | EV_FS_O_CREAT, EV_FS_S_IRUSR | EV_FS_S_IWUSR,
        _test_file_read_write_on_open);
	ASSERT_EQ_INT(ret, EV_SUCCESS);

	ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
	ASSERT_EQ_INT(ret, EV_SUCCESS);

	ev_buf_t buf = ev_buf_make((void*)s_sample_data, strlen(s_sample_data));
	ret = ev_file_pwrite(&g_test_file.file, &g_test_file.token, &buf, 1, 0,
        _test_file_read_write_on_write_done);
	ASSERT_EQ_INT(ret, EV_SUCCESS);

	ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
	ASSERT_EQ_INT(ret, EV_SUCCESS);

    ret = ev_file_seek(&g_test_file.file, &g_test_file.token, EV_FS_SEEK_BEG, 0,
        _test_file_read_write_on_seek_done);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

	ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
	ASSERT_EQ_INT(ret, EV_SUCCESS);

	buf = ev_buf_make(buffer, 4);
	ret = ev_file_read(&g_test_file.file, &g_test_file.token, &buf, 1,
        _test_file_read_write_on_read_done);
	ASSERT_EQ_INT(ret, EV_SUCCESS);

	ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
	ASSERT_EQ_INT(ret, EV_SUCCESS);
    ret = memcmp(buffer, "abcd", 4);
    ASSERT_EQ_INT(ret, 0);

	buf = ev_buf_make(buffer, 4);
	ret = ev_file_read(&g_test_file.file, &g_test_file.token, &buf, 1,
		_test_file_read_write_on_read_done);
	ASSERT_EQ_INT(ret, EV_SUCCESS);

	ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
	ASSERT_EQ_INT(ret, EV_SUCCESS);
	ret = memcmp(buffer, "efgh", 4);
	ASSERT_EQ_INT(ret, 0);
}

//////////////////////////////////////////////////////////////////////////
// fs.fstat
//////////////////////////////////////////////////////////////////////////

static void _test_file_stat_on_stat(ev_fs_req_t* req)
{
    ASSERT_EQ_PTR(ev_fs_get_file(req), &g_test_file.file);

    ASSERT_EQ_SSIZE(req->result, EV_SUCCESS);
    ev_fs_stat_t* statbuf = ev_fs_get_statbuf(req);

    ASSERT_EQ_UINT64(statbuf->st_size, 0);
    ev_fs_req_cleanup(req);
}

TEST_F(fs, fstat)
{
    int ret;

    ret = ev_file_open(&g_test_file.file, &g_test_file.token, s_sample_file,
        EV_FS_O_RDWR | EV_FS_O_CREAT, EV_FS_S_IRUSR | EV_FS_S_IWUSR,
        _test_file_pread_pwrite_on_open);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    ret = ev_file_stat(&g_test_file.file, &g_test_file.token, _test_file_stat_on_stat);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
    ASSERT_EQ_INT(ret, EV_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
// fs.readdir
//////////////////////////////////////////////////////////////////////////

static void _test_file_readdir_on_readdir(ev_fs_req_t* req)
{
    int flag_have_exe = 0;
    ASSERT_GT_SSIZE(req->result, 0);

    ev_dirent_t* info = ev_fs_get_first_dirent(req);
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

    const char* exe_dir = test_get_self_dir();

    ret = ev_fs_readdir(&g_test_file.loop, &g_test_file.token, exe_dir,
        _test_file_readdir_on_readdir);
    ASSERT_EQ_INT(ret, 0);

    ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
    ASSERT_EQ_INT(ret, EV_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
// fs.readdir_nonexist
//////////////////////////////////////////////////////////////////////////

static void _test_file_readdir_nonexist_on_readdir(ev_fs_req_t* req)
{
    ASSERT_EQ_SSIZE(req->result, EV_ENOENT);
    ev_fs_req_cleanup(req);
}

TEST_F(fs, readdir_nonexist)
{
    int ret;
    const char* path = "./non-exist-dir";

    ret = ev_fs_readdir(&g_test_file.loop, &g_test_file.token, path,
        _test_file_readdir_nonexist_on_readdir);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
    ASSERT_EQ_INT(ret, EV_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
// fs.readfile
//////////////////////////////////////////////////////////////////////////

static void _test_file_readfile_on_readfile(ev_fs_req_t* req)
{
    ev_buf_t* buf = ev_fs_get_filecontent(req);
    ASSERT_EQ_SIZE(buf->size, strlen(s_sample_data));

    int ret = memcmp(buf->data, s_sample_data, buf->size);
    ASSERT_EQ_INT(ret, 0);

    ev_fs_req_cleanup(req);
}

TEST_F(fs, readfile)
{
    int ret;

    ret = test_write_file(s_sample_file, s_sample_data, strlen(s_sample_data));
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    ret = ev_fs_readfile(&g_test_file.loop, &g_test_file.token, s_sample_file,
        _test_file_readfile_on_readfile);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
    ASSERT_EQ_INT(ret, EV_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////
// fs.mkdir
//////////////////////////////////////////////////////////////////////////

static void _test_fs_mkdir_on_mkdir(ev_fs_req_t* req)
{
    ASSERT_EQ_SSIZE(req->result, EV_SUCCESS);
    ev_fs_req_cleanup(req);
}

TEST_F(fs, mkdir)
{
    int ret;
    ASSERT_EQ_INT(test_access_dir(s_sample_path), EV_ENOENT);

    ret = ev_fs_mkdir(&g_test_file.loop, &g_test_file.token, s_sample_path,
        EV_FS_S_IRWXU, _test_fs_mkdir_on_mkdir);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    ret = ev_loop_run(&g_test_file.loop, EV_LOOP_MODE_DEFAULT);
    ASSERT_EQ_INT(ret, EV_SUCCESS);

    ASSERT_EQ_INT(test_access_dir(s_sample_path), EV_SUCCESS);
}
