#include "ev.h"
#include "file.h"
#include "config.h"
#include "cutest.h"
#include "memcheck.h"
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#if defined(_WIN32)
#   include <windows.h>
#else
#   include <unistd.h>
#   include <dirent.h>
#endif

typedef struct test_file_ctx
{
    char exe_path[4096];
    char exe_dir[4096];
}test_file_ctx_t;

static test_file_ctx_t s_test_file;

extern int ev__translate_sys_error(int errcode);

static void _on_init_file(void)
{
    ASSERT_GT_U32(ev_exepath(s_test_file.exe_path, sizeof(s_test_file.exe_path)), 0);

    const char* p_exe_pos = test_file_name(s_test_file.exe_path);
    size_t exe_dir_len = p_exe_pos - s_test_file.exe_path;
    memcpy(s_test_file.exe_dir, s_test_file.exe_path, exe_dir_len);
    s_test_file.exe_dir[exe_dir_len - 1] = '\0';
}

static void _init_file_once(void)
{
    static ev_once_t once_token = EV_ONCE_INIT;
    ev_once_execute(&once_token, _on_init_file);
}

int test_write_file(const char* path, const void* data, size_t size)
{
    int ret = 0;
    FILE* file;
#if defined(_WIN32)
    ret = fopen_s(&file, path, "wb");
#else
    file = fopen(path, "wb");
#endif
    if (ret != 0 || file == NULL)
    {
        return EV_ENOENT;
    }

    size_t left_size = size;

    while (left_size > 0)
    {
        unsigned char* pos = (unsigned char*)data + size - left_size;
        size_t write_size = fwrite(pos, 1, left_size, file);
        left_size = left_size - write_size;
    }

    fclose(file);

    return EV_SUCCESS;
}

int test_access_dir(const char* path)
{
#if defined(_WIN32)

    DWORD ret = GetFileAttributesA(path);
    if (ret == INVALID_FILE_ATTRIBUTES)
    {
        ret = GetLastError();
        return ev__translate_sys_error(ret);
    }

    switch (ret)
    {
    case FILE_ATTRIBUTE_DIRECTORY:
        return EV_SUCCESS;

    default:
        return EV_ENOENT;
    }

#else
    int errcode;
    DIR* dir = opendir(path);
    if (dir == NULL)
    {
        errcode = errno;
        return ev__translate_sys_error(errcode);
    }

    closedir(dir);
    return EV_SUCCESS;
#endif
}

const char* test_get_self_exe(void)
{
    _init_file_once();
    return s_test_file.exe_path;
}

const char* test_get_self_dir(void)
{
    _init_file_once();
    return s_test_file.exe_dir;
}

const char* test_file_name(const char* file)
{
    const char* pos = file;

    for (; *file; ++file)
    {
        if (*file == '\\' || *file == '/')
        {
            pos = file + 1;
        }
    }
    return pos;
}

const char* test_self_exe_name(void)
{
    _init_file_once();
    return test_file_name(s_test_file.exe_path);
}

char* file_parrent_dir(const char* path)
{
    size_t i = 0;
    const char* last_slash = NULL;

    for (; path[i] != '\0'; i++)
    {
        if (path[i] == '\\' || path[i] == '/')
        {
            last_slash = &path[i];
        }
    }

    if (last_slash == NULL)
    {
        return mmc_strdup("");
    }

    size_t str_size = last_slash - path;
    char* buffer = mmc_malloc(str_size + 1);
    ASSERT_NE_PTR(buffer, NULL);

    memcpy(buffer, path, str_size);
    buffer[str_size] = '\0';

    return buffer;
}
