#ifndef __EV_SHARED_MEMORY_UNIX_H__
#define __EV_SHARED_MEMORY_UNIX_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Unix backend for #ev_shm_t.
 */
typedef struct ev_shm_backend
{
    char name[256];
    int  map_file;
    struct
    {
        unsigned is_open : 1;
    } mask;
} ev_shm_backend_t;

struct ev_shmem
{
    void            *addr;    /**< Shared memory address */
    size_t           size;    /**< Shared memory size */
    ev_shm_backend_t backend; /**< Backend */
};

#ifdef __cplusplus
}
#endif
#endif
