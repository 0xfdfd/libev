#define _CRTDBG_MAP_ALLOC
#ifdef NDEBUG
#undef NDEBUG
#endif

#if !defined(__has_feature)
#   define __has_feature(x)    0
#endif

#if defined(__clang__) && __has_feature(address_sanitizer)
/* Clang with address sanitizer */
#   define HAVE_SANS   1
#elif defined(__GNUC__) && defined(__SANITIZE_ADDRESS__) && (__SANITIZE_ADDRESS__ == 1)
/* GCC with address sanitizer */
#   define HAVE_SANS   1
#elif defined(_MSC_VER) && defined(__SANITIZE_ADDRESS__) && (__SANITIZE_ADDRESS__ == 1)
/* MSVC with address sanitizer */
#   define HAVE_SANS   1
#else
#   define HAVE_SANS   0
#endif

#include "ev.h"
#include "memcheck.h"
#include "cutest.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define container_of(ptr, type, member) \
    ((type *) ((char *) (ptr) - offsetof(type, member)))

#if HAVE_SANS

void setup_memcheck(void)
{
}

void dump_memcheck(void)
{
}

#else

#if defined(_MSC_VER)
#include <crtdbg.h>

static void* _memcheck_malloc(size_t size)
{
    return malloc(size);
}

static void* _memcheck_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

static void* _memcheck_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

static void _memcheck_free(void* ptr)
{
    free(ptr);
}

void dump_memcheck(void)
{
    _CrtDumpMemoryLeaks();
}

#else

#include <stdlib.h>

typedef struct memblock_s
{
    ev_list_node_t  node;
    size_t          size;
    uint8_t         payload[];
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

static void* _memcheck_malloc(size_t size)
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

static void* _memcheck_calloc(size_t nmemb, size_t size)
{
    size_t calloc_size = nmemb * size;

    void* ptr = _memcheck_malloc(calloc_size);
    if (ptr != NULL)
    {
        memset(ptr, 0, calloc_size);
    }

    return ptr;
}

static void _memcheck_free(void* ptr)
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

static void* _memcheck_realloc(void* ptr, size_t size)
{
    _memcheck_init();

    if (ptr == NULL)
    {
        return _memcheck_malloc(size);
    }
    if (size == 0)
    {
        _memcheck_free(ptr);
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

void dump_memcheck(void)
{
    assert(ev_list_size(&s_runtime.mem_queue) == 0);
}

#endif

void setup_memcheck(void)
{
    ASSERT_EQ_D32(
        ev_replace_allocator(_memcheck_malloc, _memcheck_calloc, _memcheck_realloc, _memcheck_free),
        EV_SUCCESS
    );
}

#endif