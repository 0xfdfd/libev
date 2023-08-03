#define _CRTDBG_MAP_ALLOC
#ifdef NDEBUG
#undef NDEBUG
#endif

#include "memcheck.h"
#include "cutest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define container_of(ptr, type, member) \
    ((type *) ((char *) (ptr) - offsetof(type, member)))

#include <stdlib.h>

typedef struct mmc_runtime_s
{
    ev_list_t       snapshot_queue;
    ev_mutex_t      mutex;
    mmc_snapshot_t* curr_snapshot;  /**< Current snapshot */
    uint64_t        index;
}mmc_runtime_t;

static mmc_runtime_t s_mmc_rt = {
    EV_LIST_INIT,
    EV_MUTEX_INVALID,
    NULL,
    0,
};

static void _mmc_create_new_snapshot_layer(void)
{
    mmc_snapshot_t* snapshot = malloc(sizeof(mmc_snapshot_t));
    if (snapshot == NULL)
    {
        cutest_porting_abort("out of memory");
        exit(EXIT_FAILURE);
    }

    ev_list_init(&snapshot->mmlist);
    ev_mutex_init(&snapshot->guard, 0);
    snapshot->refcnt = 0;

    ev_mutex_enter(&s_mmc_rt.mutex);
    {
        snapshot->idx = s_mmc_rt.index++;
        ev_list_push_back(&s_mmc_rt.snapshot_queue, &snapshot->node);
        s_mmc_rt.curr_snapshot = snapshot;
    }
    ev_mutex_leave(&s_mmc_rt.mutex);
}

static void _memcheck_init(void)
{
    ev_list_init(&s_mmc_rt.snapshot_queue);
    ev_mutex_init(&s_mmc_rt.mutex, 0);
    _mmc_create_new_snapshot_layer();
}

static void _mmc_destroy_snapshot(mmc_snapshot_t* snapshot)
{
    ev_mutex_exit(&snapshot->guard);
    free(snapshot);
}

/**
 * @brief remove empty snapshot
 */
static void _mmc_snapshot_compression(void)
{
    ev_mutex_enter(&s_mmc_rt.mutex);
    {
        ev_list_node_t* it = ev_list_begin(&s_mmc_rt.snapshot_queue);
        ev_list_node_t* it_end = ev_list_end(&s_mmc_rt.snapshot_queue);
        while (it != it_end)
        {
            mmc_snapshot_t* snapshot = container_of(it, mmc_snapshot_t, node);
            it = ev_list_next(it);

            if (ev_list_size(&snapshot->mmlist) != 0 || snapshot->refcnt != 0)
            {
                continue;
            }

            ev_list_erase(&s_mmc_rt.snapshot_queue, &snapshot->node);
            _mmc_destroy_snapshot(snapshot);
        }
    }
    ev_mutex_leave(&s_mmc_rt.mutex);
}

static void _mmc_do_cb(mmc_snapshot_t* snapshot, mmc_cmp_cb cb, void* arg)
{
    ev_mutex_enter(&snapshot->guard);
    {
        ev_list_node_t* it = ev_list_begin(&snapshot->mmlist);
        for (; it != NULL; it = ev_list_next(it))
        {
            memblock_t* block = container_of(it, memblock_t, node);
            cb(block, arg);
        }
    }
    ev_mutex_leave(&snapshot->guard);
}

static void _mmc_do_compare(mmc_snapshot_t* snap1, mmc_snapshot_t* snap2,
    mmc_cmp_cb cb, void* arg)
{
    ev_list_node_t* it = ev_list_next(&snap1->node);
    for (; it != &snap2->node; it = ev_list_next(it))
    {
        mmc_snapshot_t* snapshot = container_of(it, mmc_snapshot_t, node);
        _mmc_do_cb(snapshot, cb, arg);
    }
    _mmc_do_cb(snap2, cb, arg);
}

void* mmc_malloc(size_t size)
{
    size_t malloc_size = sizeof(memblock_t) + size;
    memblock_t* memblock = malloc(malloc_size);
    if (memblock == NULL)
    {
        return NULL;
    }

    memblock->size = size;
    memblock->layer = s_mmc_rt.curr_snapshot;

    ev_mutex_enter(&memblock->layer->guard);
    {
        ev_list_push_back(&memblock->layer->mmlist, &memblock->node);
    }
    ev_mutex_leave(&memblock->layer->guard);

    return memblock->payload;
}

void* mmc_calloc(size_t nmemb, size_t size)
{
    size_t calloc_size = nmemb * size;

    void* ptr = mmc_malloc(calloc_size);
    if (ptr != NULL)
    {
        memset(ptr, 0, calloc_size);
    }

    return ptr;
}

void mmc_free(void* ptr)
{
    if (ptr == NULL)
    {
        return;
    }

    memblock_t* memblock = container_of(ptr, memblock_t, payload);

    ev_mutex_enter(&memblock->layer->guard);
    {
        ev_list_erase(&memblock->layer->mmlist, &memblock->node);
    }
    ev_mutex_leave(&memblock->layer->guard);

    free(memblock);
}

void* mmc_realloc(void* ptr, size_t size)
{
    if (ptr == NULL)
    {
        return mmc_malloc(size);
    }
    if (size == 0)
    {
        mmc_free(ptr);
        return NULL;
    }

    memblock_t* memblock = container_of(ptr, memblock_t, payload);
    size_t new_memblock_size = sizeof(memblock_t) + size;

    ev_mutex_enter(&memblock->layer->guard);
    {
        ev_list_erase(&memblock->layer->mmlist, &memblock->node);
    }
    ev_mutex_leave(&memblock->layer->guard);

    memblock_t* dst_memblock = memblock;
    memblock_t* new_memblock = realloc(memblock, new_memblock_size);
    if (new_memblock != NULL)
    {
        dst_memblock = new_memblock;
        dst_memblock->size = size;
        dst_memblock->layer = s_mmc_rt.curr_snapshot;
    }

    ev_mutex_enter(&dst_memblock->layer->guard);
    {
        ev_list_push_back(&dst_memblock->layer->mmlist, &dst_memblock->node);
    }
    ev_mutex_leave(&dst_memblock->layer->guard);

    return dst_memblock != new_memblock ? NULL : dst_memblock;
}

char* mmc_strdup(const char* str)
{
    size_t len = strlen(str) + 1;
    char* m = mmc_malloc(len);
    if (m == NULL)
    {
        return NULL;
    }
    return memcpy(m, str, len);
}

static void _mmc_dump_add_snapshot(mmc_info_t* info, mmc_snapshot_t* snapshot)
{
    ev_mutex_enter(&snapshot->guard);
    ev_list_node_t* it = ev_list_begin(&snapshot->mmlist);
    for (; it != NULL; it = ev_list_next(it))
    {
        memblock_t* block = EV_CONTAINER_OF(it, memblock_t, node);
        info->bytes += block->size;
    }
    ev_mutex_leave(&snapshot->guard);
}

static char _mmc_ascii_to_char(unsigned char c)
{
	if (c >= 32 && c <= 126)
	{
		return c;
	}
	return '.';
}

void mmc_dump(mmc_info_t* info)
{
    memset(info, 0, sizeof(*info));

    ev_mutex_enter(&s_mmc_rt.mutex);
    {
        ev_list_node_t* it = ev_list_begin(&s_mmc_rt.snapshot_queue);
        for (; it != NULL; it = ev_list_next(it))
        {
            mmc_snapshot_t* snapshot = container_of(it, mmc_snapshot_t, node);
            info->blocks += ev_list_size(&snapshot->mmlist);
            _mmc_dump_add_snapshot(info, snapshot);
        }
    }
    ev_mutex_leave(&s_mmc_rt.mutex);
}

void mmc_init(void)
{
    _memcheck_init();

    ASSERT_EQ_INT(
        ev_replace_allocator(mmc_malloc, mmc_calloc, mmc_realloc, mmc_free),
        0
    );
}

void mmc_exit(void)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&s_mmc_rt.snapshot_queue)) != NULL)
    {
        mmc_snapshot_t* snapshot = container_of(it, mmc_snapshot_t, node);
        _mmc_destroy_snapshot(snapshot);
    }
    s_mmc_rt.curr_snapshot = NULL;
    ev_mutex_exit(&s_mmc_rt.mutex);
}

void mmc_snapshot_take(mmc_snapshot_t** snapshot)
{
    _mmc_snapshot_compression();

    ev_mutex_enter(&s_mmc_rt.mutex);
    {
        *snapshot = s_mmc_rt.curr_snapshot;
        ev_mutex_enter(&s_mmc_rt.curr_snapshot->guard);
        {
            s_mmc_rt.curr_snapshot->refcnt++;
        }
        ev_mutex_leave(&s_mmc_rt.curr_snapshot->guard);
    }
    ev_mutex_leave(&s_mmc_rt.mutex);

    _mmc_create_new_snapshot_layer();
}

void mmc_snapshot_free(mmc_snapshot_t** snapshot)
{
    ev_mutex_enter(&(*snapshot)->guard);
    {
        (*snapshot)->refcnt--;
    }
    ev_mutex_leave(&(*snapshot)->guard);

    *snapshot = NULL;
    _mmc_snapshot_compression();
}

void mmc_snapshot_compare(mmc_snapshot_t* snap1, mmc_snapshot_t* snap2,
    mmc_cmp_cb cb, void* arg)
{
    if (snap1->idx == snap2->idx)
    {
        return;
    }

    /* snap1 should be the earlier one */
    if (snap1->idx > snap2->idx)
    {
        mmc_snapshot_t* tmp = snap1;
        snap1 = snap2;
        snap2 = tmp;
    }

    _mmc_do_compare(snap1, snap2, cb, arg);
}

void mmc_dump_hex(const void* data, size_t size, size_t width)
{
	const unsigned char* pdat = (unsigned char*)data;

	size_t idx_line;
	for (idx_line = 0; idx_line < size; idx_line += width)
	{
        fprintf(stdout, "%p: ", pdat + idx_line);

		/* printf hex */
        size_t idx_colume;
		for (idx_colume = 0; idx_colume < width; idx_colume++)
		{
			const char* postfix = (idx_colume < width - 1) ? "" : "|";

			if (idx_colume + idx_line < size)
			{
				fprintf(stdout, "%02x %s", pdat[idx_colume + idx_line], postfix);
			}
			else
			{
				fprintf(stdout, "   %s", postfix);
			}
		}
		fprintf(stdout, " ");
		/* printf char */
		for (idx_colume = 0; (idx_colume < width) && (idx_colume + idx_line < size); idx_colume++)
		{
			fprintf(stdout, "%c", _mmc_ascii_to_char(pdat[idx_colume + idx_line]));
		}
		fprintf(stdout, "\n");
	}
}
