#ifndef __EV_SHARD_MEMORY_H__
#define __EV_SHARD_MEMORY_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_SHARED_MEMORY Shared memory
 * @{
 */

/**
 * @brief Shared memory type.
 */
typedef struct ev_shm
{
    void*                   addr;       /**< Shared memory address */
    size_t                  size;       /**< Shared memory size */
    EV_SHM_BACKEND          backend;    /**< Backend */
} ev_shm_t;
#define EV_SHM_INIT         { NULL, 0, EV_SHM_BACKEND_INVALID }

/**
 * @brief Create a new shared memory
 * @param[out] shm  Shared memory token
 * @param[in] key   Shared memory key
 * @param[in] size  Shared memory size
 * @return          #ev_errno_t
 */
EV_API int ev_shm_init(ev_shm_t* shm, const char* key, size_t size);

/**
 * @brief Open a existing shared memory
 * @param[out] shm  Shared memory token
 * @param[in] key   Shared memory key
 * @return          #ev_errno_t
 */
EV_API int ev_shm_open(ev_shm_t* shm, const char* key);

/**
 * @brief Close shared memory
 * @param[in] shm   Shared memory token
 */
EV_API void ev_shm_exit(ev_shm_t* shm);

/**
 * @brief Get shared memory address
 * @param[in] shm   Shared memory token
 * @return          Shared memory address
 */
EV_API void* ev_shm_addr(ev_shm_t* shm);

/**
 * @brief Get shared memory size
 * @param[in] shm   Shared memory token
 * @return          Shared memory size
 */
EV_API size_t ev_shm_size(ev_shm_t* shm);

/**
 * @} EV_SHARED_MEMORY
 */

#ifdef __cplusplus
}
#endif
#endif
