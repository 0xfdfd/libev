#include "ev.h"
#include "misc_win.h"
#include "loop.h"
#include "shmem.h"

int ev_shm_init(ev_shm_t* shm, const char* key, size_t size)
{
    int err;

    shm->size = size;

    DWORD d_high = 0;
    DWORD d_low = (DWORD)size;
#if defined(_WIN64)
    if (d_low != size)
    {
        d_high = size >> 32;
    }
#endif

    shm->backend.map_file = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, d_high, d_low, key);
    if (shm->backend.map_file == NULL)
    {
        err = GetLastError();
        return ev__translate_sys_error(err);
    }

    shm->addr = MapViewOfFile(shm->backend.map_file, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (shm->addr == NULL)
    {
        err = GetLastError();
        CloseHandle(shm->backend.map_file);
        return ev__translate_sys_error(err);
    }

    return EV_SUCCESS;
}

int ev_shm_open(ev_shm_t* shm, const char* key)
{
    int err;

    shm->backend.map_file = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, key);
    if (shm->backend.map_file == NULL)
    {
        err = GetLastError();
        return ev__translate_sys_error(err);
    }

    shm->addr = MapViewOfFile(shm->backend.map_file, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (shm->addr == NULL)
    {
        err = GetLastError();
        CloseHandle(shm->backend.map_file);
        return ev__translate_sys_error(err);
    }

    MEMORY_BASIC_INFORMATION info;
    if (VirtualQuery(shm->addr, &info, sizeof(info)) == 0)
    {
        err = GetLastError();
        UnmapViewOfFile(shm->addr);
        CloseHandle(shm->backend.map_file);
        return ev__translate_sys_error(err);
    }
    shm->size = info.RegionSize;

    return EV_SUCCESS;
}

void ev_shm_exit(ev_shm_t* shm)
{
    if (!UnmapViewOfFile(shm->addr))
    {
        EV_ABORT("GetLastError:%lu", (unsigned long)GetLastError());
    }

    if (!CloseHandle(shm->backend.map_file))
    {
        EV_ABORT("GetLastError:%lu", (unsigned long)GetLastError());
    }
}
