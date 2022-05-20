#ifndef __EV_SHARED_MEMORY_H__
#define __EV_SHARED_MEMORY_H__

#include "ev/backend.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_SHARED_MEMORY Shared memory
 * @{
 */

struct ev_shm;

/**
 * @brief Typedef of #ev_shm.
 */
typedef struct ev_shm ev_shm_t;

/**
 * @brief Shared memory type.
 */
struct ev_shm
{
    void*                   addr;       /**< Shared memory address */
    size_t                  size;       /**< Shared memory size */
    ev_shm_backend_t        backend;    /**< Backend */
};
#define EV_SHM_INIT         { NULL, 0, EV_SHM_BACKEND_INVALID }

/**
 * @brief Create a new shared memory
 * @param[out] shm  Shared memory token
 * @param[in] key   Shared memory key
 * @param[in] size  Shared memory size
 * @return          #ev_errno_t
 */
int ev_shm_init(ev_shm_t* shm, const char* key, size_t size);

/**
 * @brief Open a existing shared memory
 * @param[out] shm  Shared memory token
 * @param[in] key   Shared memory key
 * @return          #ev_errno_t
 */
int ev_shm_open(ev_shm_t* shm, const char* key);

/**
 * @brief Close shared memory
 * @param[in] shm   Shared memory token
 */
void ev_shm_exit(ev_shm_t* shm);

/**
 * @brief Get shared memory address
 * @param[in] shm   Shared memory token
 * @return          Shared memory address
 */
void* ev_shm_addr(ev_shm_t* shm);

/**
 * @brief Get shared memory size
 * @param[in] shm   Shared memory token
 * @return          Shared memory size
 */
size_t ev_shm_size(ev_shm_t* shm);

/**
 * @} EV_SHARED_MEMORY
 */

#ifdef __cplusplus
}
#endif
#endif
