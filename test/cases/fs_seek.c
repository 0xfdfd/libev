#include "test.h"
#include "utils/random.h"

static const char* s_filename = "dfb7ae4b-3674-43b9-b907-d2842c6f60c5";

TEST_FIXTURE_SETUP(fs)
{
    ev_fs_remove(NULL, NULL, s_filename, 0, NULL);
}

TEST_FIXTURE_TEARDOWN(fs)
{
    ev_fs_remove(NULL, NULL, s_filename, 0, NULL);
}

TEST_F(fs, seek_offset)
{
    {
        ev_file_t* file = ev_malloc(sizeof(ev_file_t));
        ASSERT_EQ_INT(ev_file_open(NULL, file, NULL, s_filename, EV_FS_O_CREAT | EV_FS_O_WRONLY, EV_FS_S_IRWXU, NULL), 0);

        static char buffer[4096];
        test_random(buffer, sizeof(buffer));

        ev_buf_t buf = ev_buf_make((void*)buffer, sizeof(buffer));
        ASSERT_EQ_SSIZE(ev_file_write(file, NULL, &buf, 1, NULL), sizeof(buffer));

        ev_file_close(file, NULL);
        ev_free(file);
    }
    
    {
        ev_file_t* file = ev_malloc(sizeof(ev_file_t));
        ASSERT_EQ_INT(ev_file_open(NULL, file, NULL, s_filename, EV_FS_O_RDONLY, 0, NULL), 0);

        char buffer[128];
        ev_buf_t buf = ev_buf_make((void*)buffer, sizeof(buffer));
        ASSERT_EQ_SSIZE(ev_file_read(file, NULL, &buf, 1, NULL), sizeof(buffer));

        ASSERT_EQ_SSIZE(ev_file_seek(file, NULL, EV_FS_SEEK_CUR, 0, NULL), sizeof(buffer));

        ev_file_close(file, NULL);
        ev_free(file);
    }
}
