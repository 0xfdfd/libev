#define _CRTDBG_MAP_ALLOC
#ifdef NDEBUG
#undef NDEBUG
#endif

#include "ev.h"
#include "memcheck.h"
#include "cutest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define container_of(ptr, type, member) \
    ((type *) ((char *) (ptr) - offsetof(type, member)))

#include <stdlib.h>

typedef struct memblock_s
{
    ev_list_node_t  node;
    size_t          size;
#if defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4200)
#endif
    uint8_t         payload[];
#if defined(_MSC_VER)
#    pragma warning(pop)
#endif
}memblock_t;

typedef struct memcheck_runtime_s
{
    ev_list_t       mem_queue;
    ev_mutex_t      mutex;
    ev_once_t       once_token;
}memcheck_runtime_t;

static memcheck_runtime_t s_runtime = {
    EV_LIST_INIT,
    EV_MUTEX_INVALID,
    EV_ONCE_INIT,
};

static void _memcheck_on_init(void)
{
    ev_mutex_init(&s_runtime.mutex, 0);
}

static void _memcheck_init(void)
{
    ev_once_execute(&s_runtime.once_token, _memcheck_on_init);
}

void* memcheck_malloc(size_t size)
{
    _memcheck_init();

    size_t malloc_size = sizeof(memblock_t) + size;
    memblock_t* memblock = malloc(malloc_size);
    if (memblock == NULL)
    {
        return NULL;
    }

    memblock->size = size;

    ev_mutex_enter(&s_runtime.mutex);
    {
        ev_list_push_back(&s_runtime.mem_queue, &memblock->node);
    }
    ev_mutex_leave(&s_runtime.mutex);

    return memblock->payload;
}

void* memcheck_calloc(size_t nmemb, size_t size)
{
    size_t calloc_size = nmemb * size;

    void* ptr = memcheck_malloc(calloc_size);
    if (ptr != NULL)
    {
        memset(ptr, 0, calloc_size);
    }

    return ptr;
}

void memcheck_free(void* ptr)
{
    _memcheck_init();

    if (ptr == NULL)
    {
        return;
    }

    memblock_t* memblock = container_of(ptr, memblock_t, payload);

    ev_mutex_enter(&s_runtime.mutex);
    {
        ev_list_erase(&s_runtime.mem_queue, &memblock->node);
    }
    ev_mutex_leave(&s_runtime.mutex);

    free(memblock);
}

void* memcheck_realloc(void* ptr, size_t size)
{
    _memcheck_init();

    if (ptr == NULL)
    {
        return memcheck_malloc(size);
    }
    if (size == 0)
    {
        memcheck_free(ptr);
        return NULL;
    }

    memblock_t* memblock = container_of(ptr, memblock_t, payload);
    size_t new_memblock_size = sizeof(memblock_t) + size;

    ev_mutex_enter(&s_runtime.mutex);
    {
        ev_list_erase(&s_runtime.mem_queue, &memblock->node);
    }
    ev_mutex_leave(&s_runtime.mutex);

    memblock_t* dst_memblock = memblock;
    memblock_t* new_memblock = realloc(memblock, new_memblock_size);
    if (new_memblock != NULL)
    {
        dst_memblock = new_memblock;
        dst_memblock->size = size;
    }

    ev_mutex_enter(&s_runtime.mutex);
    {
        ev_list_push_back(&s_runtime.mem_queue, &dst_memblock->node);
    }
    ev_mutex_leave(&s_runtime.mutex);

    return dst_memblock != new_memblock ? NULL : dst_memblock;
}

char* memcheck_strdup(const char* str)
{
    size_t len = strlen(str) + 1;
    char* m = memcheck_malloc(len);
    if (m == NULL)
    {
        return NULL;
    }
    return memcpy(m, str, len);
}

void dump_memcheck(void)
{
    size_t left_block = ev_list_size(&s_runtime.mem_queue);
    if (left_block != 0)
    {
        fprintf(stderr, "[  ERROR   ] memory leak detected: %zu block%s not free.\n",
            left_block, left_block == 1 ? "" : "s");
        exit(EXIT_FAILURE);
    }
}

void setup_memcheck(void)
{
    ASSERT_EQ_D32(
        ev_replace_allocator(memcheck_malloc, memcheck_calloc, memcheck_realloc, memcheck_free),
        EV_SUCCESS
    );
}
