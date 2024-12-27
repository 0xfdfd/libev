#include "ev.h"
#include "file.h"
#include "test.h"
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#if defined(_WIN32)
#   include <windows.h>
#else
#   include <unistd.h>
#   include <dirent.h>
#   include <stdlib.h>
#endif

typedef struct test_file_ctx
{
    char exe_path[4096];
    char exe_dir[4096];
}test_file_ctx_t;

static test_file_ctx_t s_test_file;

static void _on_init_file(void)
{
    ASSERT_GT_INT64(ev_exepath(s_test_file.exe_path, sizeof(s_test_file.exe_path)), 0);

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

#if !defined(_WIN32)
static int fopen_s(FILE** stream, const char* path, const char* mode)
{
    if ((*stream = fopen(path, mode)) != NULL)
    {
        return 0;
    }
    return errno;
}
#endif

int test_write_file(const char* path, const void* data, size_t size)
{
    FILE* file;
    int ret = fopen_s(&file, path, "wb");
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

    return 0;
}

long test_read_file(const char* path, char** content)
{
    FILE* file = NULL;
    int ret = fopen_s(&file, path, "rb");
    if (ret != 0)
    {
        return EV_ENOENT;
    }

    fseek(file, 0, SEEK_END);
    size_t file_sz = ftell(file);
    rewind(file);

    *content = ev_malloc(file_sz + 1);
    if (fread(*content, file_sz, 1, file) != 1)
    {
        fclose(file);
        ev_free(*content);
        return EV_EPIPE;
    }
    (*content)[file_sz] = '\0';

    return (long)file_sz;
}

int test_access_dir(const char* path)
{
#if defined(_WIN32)

    DWORD ret = GetFileAttributesA(path);
    if (ret == INVALID_FILE_ATTRIBUTES)
    {
        return EV_ENOENT;
    }

    switch (ret)
    {
    case FILE_ATTRIBUTE_DIRECTORY:
        return 0;

    default:
        return EV_ENOENT;
    }

#else
    int errcode;
    DIR* dir = opendir(path);
    if (dir == NULL)
    {
        errcode = errno; (void)errcode;
        return EV_ENOENT;
    }

    closedir(dir);
    return 0;
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
        return strdup("");
    }

    size_t str_size = last_slash - path;
    char* buffer = malloc(str_size + 1);
    ASSERT_NE_PTR(buffer, NULL);

    memcpy(buffer, path, str_size);
    buffer[str_size] = '\0';

    return buffer;
}
