#include "test.h"
#include "utils/file.h"
#include "utils/random.h"

static const char* s_file_name = "2e8b75e7-ec65-46a3-ba02-ba136c093eb4";
static const char* s_file_content =
"06af3c93-7002-4abb-8b14-1f03a175752b\n"
"c3962866-361e-40d8-a759-c735925f25f2\n";

TEST_FIXTURE_SETUP(fs)
{
    ev_fs_remove(NULL, NULL, s_file_name, 0, NULL);
    test_write_file(s_file_name, s_file_content, strlen(s_file_content) + 1);
}

TEST_FIXTURE_TEARDOWN(fs)
{
    ev_fs_remove(NULL, NULL, s_file_name, 0, NULL);
}

TEST_F(fs, mmap_size_0_rd)
{
    ev_file_t* file = ev_malloc(sizeof(ev_file_t));
    ASSERT_EQ_INT(ev_file_open(NULL, file, NULL, s_file_name, EV_FS_O_RDONLY, 0, NULL), 0);

    ev_file_map_t* view = ev_malloc(sizeof(ev_file_map_t));
    ASSERT_EQ_INT(ev_file_mmap(view, file, 0, EV_FS_S_IRUSR), 0);

    const char* content = view->addr;
    ASSERT_EQ_STR(content, s_file_content);

    ev_file_munmap(view);
    ev_free(view);

    ev_file_close(file, NULL);
    ev_free(file);
}

TEST_F(fs, mmap_size_0_rdwr)
{
    const char* overwrite_content = "hello world";
    const size_t overwrite_content_sz = strlen(overwrite_content);

    /* Check and overwrite file. */
    {
        ev_file_t* file = ev_malloc(sizeof(ev_file_t));
        ASSERT_EQ_INT(ev_file_open(NULL, file, NULL, s_file_name, EV_FS_O_RDWR, 0, NULL), 0);

        ev_file_map_t* view = ev_malloc(sizeof(ev_file_map_t));
        ASSERT_EQ_INT(ev_file_mmap(view, file, 0, EV_FS_S_IRUSR | EV_FS_S_IWUSR), 0);

        char* content = view->addr;
        ASSERT_EQ_STR(content, s_file_content);

        memcpy(content, overwrite_content, overwrite_content_sz);

        ev_file_munmap(view);
        ev_free(view);

        ev_file_close(file, NULL);
        ev_free(file);
    }

    /* Check file content. */
    {
        char* content = NULL;
        test_read_file(s_file_name, &content);

        ASSERT_EQ_INT(strncmp(content, overwrite_content, overwrite_content_sz), 0);
        ASSERT_EQ_INT(strcmp(content + overwrite_content_sz, s_file_content + overwrite_content_sz), 0);

        ev_free(content);
    }
}
