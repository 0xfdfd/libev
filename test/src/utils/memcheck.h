#ifndef __TEST_MEMCHECK_H__
#define __TEST_MEMCHECK_H__

#include "ev.h"

#ifdef __cplusplus
extern "C" {
#endif

struct mmc_snapshot_s;
typedef struct mmc_snapshot_s mmc_snapshot_t;

typedef struct memblock_s
{
    ev_list_node_t  node;   /**< List node */
    size_t          size;   /**< Size of payload in bytes */

    mmc_snapshot_t* layer;  /**< Snapshot layer */

#if defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4200)
#endif
    /**
     * Data payload
     */
    uint8_t         payload[];
#if defined(_MSC_VER)
#    pragma warning(pop)
#endif
} memblock_t;

typedef struct mmc_snapshot_s
{
    ev_list_node_t  node;   /**< Layer node */
    ev_list_t       mmlist; /**< Memory block list */
    ev_mutex_t      guard;  /**< Snapshot mutex */
    size_t          refcnt; /**< Reference count */
    uint64_t        idx;    /**< Index */
} mmc_snapshot_t;

typedef void (*mmc_cmp_cb)(memblock_t*, void*);

/**
 * @brief Setup memory leak check.
 */
void mmc_init(void);

/**
 * @brief Cleanup mmc runtime.
 */
void mmc_exit(void);

/**
 * @brief Dump memory leak check result.
 */
void mmc_dump_exit(void);

/**
 * @brief same as malloc.
 */
void* mmc_malloc(size_t size);

/**
 * @brief Same as calloc.
 */
void* mmc_calloc(size_t nmemb, size_t size);

/**
 * @brief Same as free.
 */
void mmc_free(void* ptr);

/**
 * @brief Same as realloc.
 */
void* mmc_realloc(void* ptr, size_t size);

/**
 * @brief Same as [strdup(3)](https://man7.org/linux/man-pages/man3/strdup.3.html).
 * Use #mmc_free() to free duplicated string.
 */
char* mmc_strdup(const char* str);

/**
 * @brief Take a snapshot of current managed memory.
 * @param[out] snapshot     A pointer to store snapshot
 */
void mmc_snapshot_take(mmc_snapshot_t** snapshot);

/**
 * @brief Release a snapshot.
 * @param[in] snapshot      A snapshot.
 */
void mmc_snapshot_free(mmc_snapshot_t** snapshot);

/**
 * @brief Compare two snapshots.
 * @param[in] snap1         A snapshot.
 * @param[in] snap2         A snapshot.
 */
void mmc_snapshot_compare(mmc_snapshot_t* snap1, mmc_snapshot_t* snap2,
    mmc_cmp_cb cb, void* arg);

#ifdef __cplusplus
}
#endif
#endif
