#include "allocator.h"
#include "ev/errno.h"
#include <stdlib.h>

typedef struct ev_allocator
{
    ev_malloc_fn    malloc_func;
    ev_calloc_fn    calloc_func;
    ev_realloc_fn   realloc_func;
    ev_free_fn      free_func;
}ev_allocator_t;

static void* _ev_malloc(size_t size)
{
    return malloc(size);
}

static void* _ev_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

static void* _ev_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

static void _ev_free(void* ptr)
{
    free(ptr);
}

static ev_allocator_t ev__allocator = {
    _ev_malloc,     /* .malloc_func */
    _ev_calloc,     /* .calloc_func */
    _ev_realloc,    /* .realloc_func */
    _ev_free,       /* ._ev_free */
};

int ev_replace_allocator(ev_malloc_fn malloc_func, ev_calloc_fn calloc_func,
    ev_realloc_fn realloc_func, ev_free_fn free_func)
{
    if (malloc_func == NULL || calloc_func == NULL || realloc_func == NULL || free_func == NULL)
    {
        return EV_EINVAL;
    }

    ev__allocator.malloc_func = malloc_func;
    ev__allocator.calloc_func = calloc_func;
    ev__allocator.realloc_func = realloc_func;
    ev__allocator.free_func = free_func;

    return EV_SUCCESS;
}

void* ev__calloc(size_t nmemb, size_t size)
{
    return ev__allocator.calloc_func(nmemb, size);
}

void* ev__malloc(size_t size)
{
    return ev__allocator.malloc_func(size);
}

void* ev__realloc(void* ptr, size_t size)
{
    return ev__allocator.realloc_func(ptr, size);
}

void ev__free(void* ptr)
{
    ev__allocator.free_func(ptr);
}
