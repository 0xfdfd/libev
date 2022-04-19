#include "ev.h"
#include "file.h"
#include "config.h"
#include "cutest.h"
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#if defined(_WIN32)
#   include <windows.h>
#else
#   include <unistd.h>
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

const char* test_get_self_exe(void)
{
#if defined(_WIN32)

    char* path;
    errno_t errcode = _get_pgmptr(&path);
    ASSERT_EQ_D32(errcode, 0);

    return path;
#else

    static char buffer[4096];
    const char* path = "/proc/self/exe";
#   if defined(__FreeBSD__)
    path = "/proc/curproc/file";
#elif defined(__sun)
    path = "/proc/self/path/a.out";
#   endif

    ssize_t ret = readlink(path, buffer, sizeof(buffer));
    if (ret == -1 && errno == ENOENT)
    {
        /* argv[0] is not a reliable executable path */
        if(test_config.argv != NULL && test_config.argv[0][0] == '\\')
        {
            return test_config.argv[0];
        }
        assert(0);
        return NULL;
    }

    return buffer;
#endif
}
