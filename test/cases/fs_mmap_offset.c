#include "test.h"
#include "utils/file.h"
#include "utils/random.h"

static const char* s_file_path = "29cc199c-ded0-4402-b910-a670aa64fcc1";
static size_t s_mmap_offset_granularity = 0;
static uint8_t* s_file_data = NULL;
static size_t s_file_data_sz = 0;

TEST_FIXTURE_SETUP(fs)
{
    s_mmap_offset_granularity = ev_os_mmap_offset_granularity();
    ASSERT_GT_SIZE(s_mmap_offset_granularity, 0);

    s_file_data_sz = s_mmap_offset_granularity * 2;
    s_file_data = ev_malloc(s_file_data_sz);
    test_random(s_file_data, s_file_data_sz);

    ev_fs_remove(NULL, NULL, s_file_path, 0, NULL);
    test_write_file(s_file_path, s_file_data, s_file_data_sz);
}

TEST_FIXTURE_TEARDOWN(fs)
{
    if (s_file_data != NULL)
    {
        ev_free(s_file_data);
        s_file_data = NULL;
    }
    ev_fs_remove(NULL, NULL, s_file_path, 0, NULL);
}

TEST_F(fs, mmap_offset_half_size_0)
{
    ev_file_t* file = ev_malloc(sizeof(ev_file_t));
    ASSERT_EQ_INT(ev_file_open(NULL, file, NULL, s_file_path, EV_FS_O_RDONLY, 0, NULL), 0);

    ev_file_map_t* view = ev_malloc(sizeof(ev_file_map_t));
    ASSERT_EQ_INT(ev_file_mmap(view, file, s_mmap_offset_granularity, 0, EV_FS_S_IRUSR), 0);

    const char* data = view->addr;
    ASSERT_EQ_INT(memcmp(data, s_file_data + s_mmap_offset_granularity, s_mmap_offset_granularity), 0);

    ev_file_close(file, NULL);
    ev_free(file);

    ev_file_munmap(view);
    ev_free(view);
}

TEST_F(fs, mmap_offset_half_size_half)
{
    ev_file_t* file = ev_malloc(sizeof(ev_file_t));
    ASSERT_EQ_INT(ev_file_open(NULL, file, NULL, s_file_path, EV_FS_O_RDONLY, 0, NULL), 0);

    ev_file_map_t* view = ev_malloc(sizeof(ev_file_map_t));
    ASSERT_EQ_INT(ev_file_mmap(view, file, s_mmap_offset_granularity, s_mmap_offset_granularity, EV_FS_S_IRUSR), 0);

    const char* data = view->addr;
    ASSERT_EQ_INT(memcmp(data, s_file_data + s_mmap_offset_granularity, s_mmap_offset_granularity), 0);

    ev_file_close(file, NULL);
    ev_free(file);

    ev_file_munmap(view);
    ev_free(view);
}
