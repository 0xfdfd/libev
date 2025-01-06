#ifndef __EV_SHARED_MEMORY_WIN32_H__
#define __EV_SHARED_MEMORY_WIN32_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Windows backend for #ev_shm_t.
 */
typedef struct ev_shm_backend
{
    HANDLE                              map_file;
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
