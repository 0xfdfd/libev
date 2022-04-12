#include "ev.h"
#include "file.h"
#include <stdio.h>

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
