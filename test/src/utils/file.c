#include "ev.h"
#include "file.h"
#include <stdio.h>
#include <errno.h>

#if !defined(_WIN32)
#   include <dirent.h>
#endif

extern int ev__translate_sys_error(int errcode);

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
